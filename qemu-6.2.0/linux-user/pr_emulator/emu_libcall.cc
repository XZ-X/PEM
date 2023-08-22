#include "pem_config.h"
#include "emulator.hh"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <unordered_map>

#ifdef EMU_TARGET_I386
#define PARAM_ONE (emuCPUState->regs[CPU_REG_RDI])
#define PARAM_TWO (emuCPUState->regs[CPU_REG_RSI])
#define PARAM_THREE (emuCPUState->regs[CPU_REG_RDX])
#define RETURN_REG (emuCPUState->regs[CPU_REG_RAX])
#endif
#ifdef EMU_TARGET_AARCH64
#define PARAM_ONE (emuCPUState->xregs[CPU_REG_X0])
#define PARAM_TWO (emuCPUState->xregs[CPU_REG_X1])
#define PARAM_THREE (emuCPUState->xregs[CPU_REG_X2])
#define RETURN_REG (emuCPUState->xregs[CPU_REG_X0])
#endif

using namespace std;
// XXX: record the position of param?
static inline void recordLibParam(MemoryManager &mem,
                                  PathSemanticCollector &collector,
                                  uint64_t value, uint64_t addr) {
  if (libFunctions.count(value)) {
    TR_LOG << "SEMV:Libcall, record libcall " << std::hex << value << endl;
    collector.addLibCall(libFunctions[value], addr);
  } else if (stringIntervalContains(value)) {
    string literal((char *)value);
    TR_LOG << "SEMV:Libcall, record literal " << literal << endl;
    collector.addStringLiteral(literal, addr);
  } else {
    if (auto normalizedAddr = mem.normalizeMemoryAddress(value)) {
      collector.addValue(normalizedAddr, addr);
      TR_LOG << "SEMV: MemAddr [" << normalizedAddr << "(" << std::hex
             << normalizedAddr << ")" << std::dec << "]" << endl;
    }
    auto toRecord = getValueOrReadValue(value, mem);
    TR_LOG << "SEMV:Libcall, record value " << std::hex << toRecord << " ;;; "
           << value << endl;
    collector.addValue(toRecord, addr);
  }
}

void Emulator::libcall(const string &rawName) {
  string name = normalizeLibName(rawName);
  const auto currentBlock = ecStack.back().currentBlock->getBeginPC();
  static unordered_map<string, LibFunction> libCallLookupTable = {
      {"malloc", &Emulator::emu_malloc},
      {"calloc", &Emulator::emu_calloc},
      {"realloc", &Emulator::emu_realloc},
      {"__errno_location", &Emulator::emu___errno_location},
      {"strcmp", &Emulator::emu_strcmp},
      {"strncmp", &Emulator::emu_strncmp},
      {"strlen", &Emulator::emu_strlen}};
  collector->addLibCall(name, currentBlock);

  if (libCallLookupTable.count(name)) {
#ifdef PEM_TR
    cout << "Emulated Libcall: " << name << endl;
#endif
    auto func = libCallLookupTable[name];
    (this->*func)();
  } else {
#ifdef PEM_TR
    cout << "Ignored Libcall: " << name << endl;
#endif
    RETURN_REG = EMU_MAGIC;
  }
  auto s1 = PARAM_ONE;
  auto s2 = PARAM_TWO;
  recordLibParam(emuMemory, *collector, s1, currentBlock);
  recordLibParam(emuMemory, *collector, s2, currentBlock);
}

void Emulator::emu_malloc() {
  auto addr = emuMemory.allocNewHeapChunk();
  RETURN_REG = addr;
}

void Emulator::emu_calloc() {
  auto addr = emuMemory.allocNewHeapChunk();
  RETURN_REG = addr;
}

void Emulator::emu_realloc() {
  auto addr = PARAM_ONE;
  RETURN_REG = addr;
}

void Emulator::emu_strcmp() {
  auto s1 = PARAM_ONE;
  auto s2 = PARAM_TWO;
  if (stringIntervalContains(s1) && stringIntervalContains(s2)) {
    RETURN_REG = strcmp((char *)s1, (char *)s2);
  } else if (emuMemory.readW(s1) == emuMemory.readW(s2)) {
    RETURN_REG = 0;
  } else {
    RETURN_REG = EMU_MAGIC;
  }
}

void Emulator::emu_strncmp() {
  auto s1 = PARAM_ONE;
  auto s2 = PARAM_TWO;
  if (stringIntervalContains(s1) && stringIntervalContains(s2)) {
    auto n = PARAM_THREE;
    RETURN_REG = strncmp((char *)s1, (char *)s2, n);
  } else {
    emu_strcmp();
  }
}

void Emulator::emu_strlen() { RETURN_REG = 5; }

void Emulator::emu___errno_location() {
  if (errnoAddr == 0) {
    errnoAddr = emuMemory.allocNewDummyChunk();
  }
  RETURN_REG = errnoAddr;
}