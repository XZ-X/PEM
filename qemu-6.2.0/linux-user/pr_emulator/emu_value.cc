#include "emulator.hh"
#include "qemu_utils/mem_op.hh"
#include "qemu_utils/tci_utils.hh"
#include "pem_config.h"
#include "pem_load_info.h"
#include "pem_params.hh"
#include <cstring>
#include <iostream>

using namespace std;

bool MemoryManager::validRealMemory(uint64_t addr) {
  // return (addr >= load_info.data_start && addr <= load_info.data_end) ||
  //        (addr >= load_info.exec_start && addr <= load_info.exec_end);
  return (addr >= load_info.load_lo && addr <= load_info.load_hi);
}

void MemoryManager::clearLocalStack(uint64_t rsp) {
  TR_LOG << "MM: Clear stack " << std::hex << rsp << endl;
  if (!stackAddress(rsp)) {
    TR_LOG << "MM: Clear stack: Not a valid stack addr " << std::hex << rsp
           << endl;
    return;
  }
  auto page = getPage(rsp);
  auto &currentPageTable = memorySnapshots.back();
  const uint8_t CLEAR_STACK_MAGIC = 5;
  for (uint8_t i = 1; i <= CLEAR_STACK_MAGIC; i++) {
    auto currentPage = page - i;
    if (currentPageTable.count(currentPage)) {
      delete currentPageTable[currentPage];
      currentPageTable.erase(currentPage);
    }
  }
  auto ofs = getOfs(rsp);
  ofs = std::min(ofs, (uint64_t)PAGE_SIZE - sizeof(uint64_t));
  if (!currentPageTable.count(page)) {
    TR_LOG << "MM: Clear stack: No page for " << std::hex << rsp << endl;
    return;
  }
  auto pageToClear = (uint64_t *)currentPageTable[page];
  for (int i = (ofs / sizeof(uint64_t)) - 1; i >= 0; i--) {
    pageToClear[i] = EMU_MAGIC;
  }
}

bool MemoryManager::shouldRecordLastOperation() { return recordLast; }

void MemoryManager::snapshot() { memorySnapshots.emplace_back(); }

void MemoryManager::rollBack() {
  auto &pageTable = memorySnapshots.back();
  for (auto &pageMem : pageTable) {
    delete pageMem.second;
  }
  memorySnapshots.pop_back();
}

void MemoryManager::clear() {
  while (!memorySnapshots.empty()) {
    rollBack();
  }
  memorySnapshots.emplace_back();
  heapID = dummyID = 0;
  recordLast = false;
  dummyValue.dummyValueUInt64[0] = EMU_MAGIC;
  dummyValue.dummyValueUInt64[1] = EMU_MAGIC;
}

void MemoryManager::insertPage(uint64_t pageIdx) {  
  auto page = new uint8_t[PAGE_SIZE];
  // XXX: implement copy-on-write for real mem and probabilistic mem!
  auto addrBase = pageIdx;
  if (validRealMemory(addrBase)) {
    TR_LOG << "MM: Insert from real memory!" << endl;
    // copy from real memory    
    memcpy(page, (const void *)addrBase, PAGE_SIZE);
  } else if (!memAddress(addrBase)) {
    TR_LOG << "MM: Insert from prob memory!" << endl;
    // copy from prob memory
    auto probAddrBase = (addrBase & (PROB_MEM_SIZE - 1)) + randomMem;
    memcpy(page, (const void *)probAddrBase, PAGE_SIZE);
  } else {
    TR_LOG << "MM: Insert from EMU MAGIC!" << endl;
    auto integerPage = (uint64_t *)page;
    // XXX: Logic for clear stack also implictly initialize memory!
    for (int i = 0; i < PAGE_SIZE / sizeof(uint64_t); i++) {
      integerPage[i] = EMU_MAGIC;
    }
  }
  memorySnapshots.back()[pageIdx] = page;
}

template <typename T> bool MemoryManager::isDummyValue(T value) {
  return value == *(T *)dummyValue.DUMMY_VALUE;
}

template <typename T> T MemoryManager::read(uint64_t addr) {
  recordLast = true;
  // to handle indirect lib function call
  if (libFunctions.count(addr) && !libData.count(addr)) {
    recordLast = false;
    return addr;
  }
  addr = addr & (~(sizeof(T) - 1));  
  auto page = getPage(addr);
  auto ofs = getOfs(addr);
  ofs = std::min(ofs, (uint64_t)PAGE_SIZE - sizeof(T));
  T ourMemoryValue;
  bool found = false;
  for (int i = memorySnapshots.size() - 1; i >= 0; i--) {
    if (memorySnapshots[i].count(page)) {
      ourMemoryValue = *(T *)(memorySnapshots[i][page] + ofs);
      found = true;
      break;
    }
  }
  if (found) {
    recordLast = isDummyValue<T>(ourMemoryValue);
    return ourMemoryValue;
  }
  // does not appear in any snapshot
  // whether it is a valid real mem address
  if (validRealMemory(addr)) {
    TR_LOG << "EMU_MM: REAL MEM READ" << endl;
    return *(T *)addr;
  } else if (!memAddress(addr)) {
    if(!PROB_MEM){
      TR_LOG << "EMU_MM: NO-PROB-MEM" << endl;
      return (T)rand();
    }
    TR_LOG << "EMU_MM: PROB MEM READ" << endl;
    // probabilistic memory
    auto probAddr = addr & (PROB_MEM_SIZE - 1);
    return *(T*)(probAddr + randomMem);
  }
  recordLast = false;
  // create a new memory chunk
  insertPage(page);
  return *(T *)(memorySnapshots.back()[page] + ofs);
}

uint64_t MemoryManager::allocNewHeapChunk() {
  auto chunkID = heapID++;
  return HEAP_BASE | ((chunkID << CHUNK_ID_SHIFT) & CHUNK_ID_MASK);
}

uint64_t MemoryManager::allocNewDummyChunk() {
  auto chunkID = dummyID++;
  return DUMMY_BASE | ((chunkID << CHUNK_ID_SHIFT) & CHUNK_ID_MASK);
}

uint64_t MemoryManager::readQ(uint64_t addr) { return read<uint64_t>(addr); }

uint32_t MemoryManager::readL(uint64_t addr) { return read<uint32_t>(addr); }

uint16_t MemoryManager::readW(uint64_t addr) { return read<uint16_t>(addr); }

uint8_t MemoryManager::readB(uint64_t addr) { return read<uint8_t>(addr); }

template <typename T> void MemoryManager::write(uint64_t addr, T value) {
  if(!memAddress(addr) && !PROB_MEM){
    return;
  }
  recordLast = true;
  auto page = getPage(addr);
  auto ofs = getOfs(addr);
  // align
  ofs = ofs & (~(sizeof(T) - 1));
  ofs = std::min(ofs, (uint64_t)PAGE_SIZE - sizeof(T));
  auto &currentPageTable = memorySnapshots.back();
  if (!currentPageTable.count(page)) {
    insertPage(page);
  }
  *(T *)(currentPageTable[page] + ofs) = value;
}

void MemoryManager::writeQ(uint64_t addr, uint64_t value) {
  return write<uint64_t>(addr, value);
}

void MemoryManager::writeL(uint64_t addr, uint32_t value) {
  return write<uint32_t>(addr, value);
}

void MemoryManager::writeW(uint64_t addr, uint16_t value) {
  return write<uint16_t>(addr, value);
}

void MemoryManager::writeB(uint64_t addr, uint8_t value) {
  return write<uint8_t>(addr, value);
}

template <typename T>
static inline void loadRegs(TCIState &tciState, const Instruction *code) {
  tci_args_rrs(*code, &tciState.r0, &tciState.r1, &tciState.ofs);
  tciState.ptr = (void *)(tciState.regs[tciState.r1] + tciState.ofs);
  tciState.regs[tciState.r0] = *(T *)tciState.ptr;
}

void Emulator::visit_ld8u_i32(const Instruction *code) {
  loadRegs<uint8_t>(tciState, code);
}

void Emulator::visit_ld8u_i64(const Instruction *code) { visit_ld8u_i32(code); }

void Emulator::visit_ld8s_i32(const Instruction *code) {
  loadRegs<int8_t>(tciState, code);
}

void Emulator::visit_ld8s_i64(const Instruction *code) { visit_ld8s_i32(code); }

void Emulator::visit_ld16u_i32(const Instruction *code) {
  loadRegs<uint16_t>(tciState, code);
}

void Emulator::visit_ld16u_i64(const Instruction *code) {
  visit_ld16u_i32(code);
}

void Emulator::visit_ld16s_i32(const Instruction *code) {
  loadRegs<int16_t>(tciState, code);
}

void Emulator::visit_ld16s_i64(const Instruction *code) {
  visit_ld16s_i32(code);
}

void Emulator::visit_ld32u_i64(const Instruction *code) {
  loadRegs<uint32_t>(tciState, code);
}

void Emulator::visit_ld_i32(const Instruction *code) { visit_ld32s_i64(code); }

void Emulator::visit_ld32s_i64(const Instruction *code) {
  loadRegs<int32_t>(tciState, code);
}

void Emulator::visit_ld_i64(const Instruction *code) {
  loadRegs<int64_t>(tciState, code);
}

template <typename T>
static inline void storeRegs(TCIState &tciState, const Instruction *code) {
  tci_args_rrs(*code, &tciState.r0, &tciState.r1, &tciState.ofs);
  tciState.ptr = (void *)(tciState.regs[tciState.r1] + tciState.ofs);
  *(T *)tciState.ptr = tciState.regs[tciState.r0];
}

void Emulator::visit_st8_i32(const Instruction *code) {
  storeRegs<uint8_t>(tciState, code);
}

void Emulator::visit_st8_i64(const Instruction *code) { visit_st8_i32(code); }

void Emulator::visit_st16_i32(const Instruction *code) {
  storeRegs<uint16_t>(tciState, code);
}

void Emulator::visit_st16_i64(const Instruction *code) { visit_st16_i32(code); }

void Emulator::visit_st_i32(const Instruction *code) {
  storeRegs<uint32_t>(tciState, code);
}

void Emulator::visit_st32_i64(const Instruction *code) { visit_st_i32(code); }

void Emulator::visit_st_i64(const Instruction *code) {
  storeRegs<uint64_t>(tciState, code);
}

uint64_t Emulator::loadValueFromMem(uint64_t addr, MemOp &mop, bool log) {
  uint64_t ret;
  switch (mop) {
  case MO_UB:
    TR_LOG << "MEM:LDUB: ";
    ret = emuMemory.readB(addr);
    break;
  case MO_SB:
    TR_LOG << "MEM:LDSB: ";
    ret = (int8_t)emuMemory.readB(addr);
    break;
  case MO_LEUW:
    TR_LOG << "MEM:LDUW(LE): ";
    ret = emuMemory.readW(addr);
    break;
  case MO_LESW:
    TR_LOG << "MEM:LDSW(LE): ";
    ret = (int16_t)emuMemory.readW(addr);
    break;
  case MO_LEUL:
    TR_LOG << "MEM:LDUL(LE): ";
    ret = emuMemory.readL(addr);
    break;
  case MO_LESL:
    TR_LOG << "MEM:LDSL(LE): ";
    ret = (int32_t)emuMemory.readL(addr);
    break;
  case MO_LEQ:
    TR_LOG << "MEM:LDQ(LE): ";
    ret = emuMemory.readQ(addr);
    break;
  case MO_BEUW:
    TR_LOG << "MEM:LDUW(BE): ";
    ret = emuMemory.readW(addr);
    ret = be_bswap(ret, 16);
    break;
  case MO_BESW:
    TR_LOG << "MEM:LDSW(BE): ";
    ret = emuMemory.readW(addr);
    ret = (int16_t)be_bswap(ret, 16);
    break;
  case MO_BEUL:
    TR_LOG << "MEM:LDUL(BE): ";
    ret = emuMemory.readL(addr);
    ret = be_bswap(ret, 32);
    break;
  case MO_BESL:
    TR_LOG << "MEM:LDSL(BE): ";
    ret = emuMemory.readL(addr);
    ret = (int32_t)be_bswap(ret, 32);
    break;
  case MO_BEQ:
    TR_LOG << "MEM:LDQ(BE): ";
    ret = emuMemory.readQ(addr);
    ret = be_bswap(ret, 64);
    break;
  default:
    assert(false);
  }
  TR_LOG << std::dec << ret << " =[" << addr << "(" << std::hex << addr << ")"
         << std::dec << "]" << endl;
  auto currentBlock = ecStack.back().currentBlock->getBeginPC();
  if (auto normalizedAddr = emuMemory.normalizeMemoryAddress(addr)) {
    if (log) {
      collector->addValue(normalizedAddr, currentBlock);
      TR_LOG << "SEMV: MemAddr [" << normalizedAddr << "(" << std::hex
             << normalizedAddr << ")" << std::dec << "]" << endl;
    }
  }
  if (log && (!emuMemory.stackAddress(addr)) &&
      emuMemory.shouldRecordLastOperation()) {
    TR_LOG << "SEMV: Load value from memory " << std::dec << ret << " =["
           << addr << "(" << std::hex << addr << ")" << std::dec << "]" << endl;
    if (libFunctions.count(ret)) {
      auto &name = libFunctions[ret];
      TR_LOG << "Addr of lib function: " << name << endl;
      collector->addLibCall(name, currentBlock);
    } else if (stringIntervalContains(ret)) {
      string literal((char *)ret);
      TR_LOG << "Addr of string literal: " << literal << endl;
      collector->addStringLiteral(literal, currentBlock);
    } else {
      auto toRecord = getValueOrReadValue(ret, emuMemory);
      TR_LOG << "Record: " << toRecord << endl;
      collector->addValue(toRecord, currentBlock);
    }
  }
  return ret;
}

void Emulator::storeValueToMem(uint64_t addr, uint64_t value, MemOp &mop) {
  if ((!emuMemory.stackAddress(addr))) {
    auto currentBlock = ecStack.back().currentBlock->getBeginPC();
    if (auto normalizedAddr = emuMemory.normalizeMemoryAddress(addr)) {
      TR_LOG << "SEMV: MemAddr [" << normalizedAddr << "(" << std::hex
             << normalizedAddr << ")" << std::dec << "]" << endl;
      collector->addValue(normalizedAddr, currentBlock);
    }
    TR_LOG << "SEMV: Store value to memory [" << addr << "(" << std::hex << addr
           << ")" << std::dec << "]=" << value << endl;
  }
  uint64_t valueToRecord = value;
  switch (mop) {
  case MO_UB:
    TR_LOG << "MEM:STB: ";
    emuMemory.writeB(addr, value);
    valueToRecord = value & 0xFF;
    break;
  case MO_BESW:
  case MO_BEUW:
    TR_LOG << "MEM:STW(BE): ";
    value = be_bswap(value, 16);
    // fall through
  case MO_LEUW:
    TR_LOG << "MEM:STW(LE): ";
    emuMemory.writeW(addr, value);
    valueToRecord = value & 0xFFFF;
    break;
  case MO_BESL:
  case MO_BEUL:
    TR_LOG << "MEM:STL(BE): ";
    value = be_bswap(value, 32);
    // fall through
  case MO_LEUL:
    TR_LOG << "MEM:STL(LE): ";
    emuMemory.writeL(addr, value);
    valueToRecord = value & 0xFFFF'FFFF;
    break;
  case MO_BEQ:
    TR_LOG << "MEM:STQ(BE): ";
    value = be_bswap(value, 32);
    // fall through
  case MO_LEQ:
    TR_LOG << "MEM:STQ(LE): ";
    emuMemory.writeQ(addr, value);
    valueToRecord = value;
    break;
  default:
    cout << std::hex << static_cast<int>(mop) << endl;
    assert(false);
  }
  TR_LOG << "[" << addr << "(" << std::hex << addr << ")" << std::dec
         << "]=" << value << endl;
  if (!emuMemory.stackAddress(addr)) {
    auto currentBlock = ecStack.back().currentBlock->getBeginPC();

    if (libFunctions.count(valueToRecord)) {
      auto &name = libFunctions[valueToRecord];
      TR_LOG << "Addr of lib function: " << name << endl;
      collector->addLibCall(name, currentBlock);
    } else if (stringIntervalContains(valueToRecord)) {
      TR_LOG << "Addr of string literal: " << (char *)valueToRecord << endl;
      string literal((char *)valueToRecord);
      collector->addStringLiteral(literal, currentBlock);
    } else {
      auto toRecord = getValueOrReadValue(valueToRecord, emuMemory);
      TR_LOG << "Record: " << toRecord << endl;
      collector->addValue(toRecord, currentBlock);
    }
  }
}

void Emulator::visit_qemu_ld_i32(const Instruction *code) {
  logCurrentInst();
  tci_args_rrm(*code, &tciState.r0, &tciState.r1, &tciState.oi);
  tciState.taddr = tciState.regs[tciState.r1];
  MemOp mop =
      static_cast<MemOp>(get_memop(tciState.oi) & (MO_BSWAP | MO_SSIZE));

  tciState.tmp32 = loadValueFromMem(tciState.taddr, mop, true);
  tciState.regs[tciState.r0] = tciState.tmp32;
}

void Emulator::visit_qemu_ld_i64(const Instruction *code) {
  logCurrentInst();
  tci_args_rrm(*code, &tciState.r0, &tciState.r1, &tciState.oi);
  tciState.taddr = tciState.regs[tciState.r1];
  MemOp mop =
      static_cast<MemOp>(get_memop(tciState.oi) & (MO_BSWAP | MO_SSIZE));

  tciState.tmp64 = loadValueFromMem(tciState.taddr, mop, true);
  tciState.regs[tciState.r0] = tciState.tmp64;
}

void Emulator::visit_qemu_st_i32(const Instruction *code) {
  TR_LOG << "visit: " << std::hex << ecStack.back().currentInst << std::dec
         << endl;
  tci_args_rrm(*code, &tciState.r0, &tciState.r1, &tciState.oi);
  tciState.taddr = tciState.regs[tciState.r1];
  MemOp mop =
      static_cast<MemOp>(get_memop(tciState.oi) & (MO_BSWAP | MO_SSIZE));

  tciState.tmp32 = tciState.regs[tciState.r0];
  storeValueToMem(tciState.taddr, tciState.tmp32, mop);
}

void Emulator::visit_qemu_st_i64(const Instruction *code) {
  TR_LOG << "visit: " << std::hex << ecStack.back().currentInst << std::dec
         << endl;
  tci_args_rrm(*code, &tciState.r0, &tciState.r1, &tciState.oi);
  tciState.taddr = tciState.regs[tciState.r1];
  MemOp mop =
      static_cast<MemOp>(get_memop(tciState.oi) & (MO_BSWAP | MO_SSIZE));

  tciState.tmp64 = tciState.regs[tciState.r0];
  storeValueToMem(tciState.taddr, tciState.tmp64, mop);
}