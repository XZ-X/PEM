#ifndef PR_EMULATOR_EMULATOR_HH
#define PR_EMULATOR_EMULATOR_HH

#include "pr_cfg/basic_block.hh"
#include "pr_sampler/sampler.hh"
#include "pr_semantic/semantic_collector.hh"
#include "qemu_utils/mem_op.hh"
#include "qemu_utils/tci_utils.hh"
#include "statistic_manager.hh"
#include "visitor.hh"
#include "pem_config.h"
#include "pem_load_info.h"
#include <algorithm>
#include <unordered_map>
#include <vector>
extern "C" {
#ifdef EMU_TARGET_I386
#include "emu_x86.h"
#endif
#ifdef EMU_TARGET_AARCH64
#include "emu_aarch64.h"
#endif
}

extern ofstream ablationStream;

#define ABL_OUT                                                                \
  if (true) {                                                                  \
  } else                                                                       \
    ablationStream

#ifndef RANDOM_INIT
extern uint64_t EMU_MAGIC;
#else
#define EMU_MAGIC rand()
#endif

struct TCIState {
  static const uint8_t STACK_SIZE = 16;
  static const uint8_t REG_SIZE = 16;
  // uint8_t opc;
  TCGReg r0, r1, r2, r3, r4, r5;
  void *callSlots[STACK_SIZE];
  uint64_t stack[STACK_SIZE + 128];
  uint64_t regs[REG_SIZE];
  uint64_t t1;
  TCGCond condition;
  uint64_t taddr;
  uint8_t pos, len;
  uint32_t tmp32;
  uint64_t tmp64;
  uint64_t T1, T2;
  MemOpIdx oi;
  int32_t ofs;
  void *ptr;
};

struct ExecutionContext {
  const BasicBlock *currentBlock = nullptr;
  const Instruction *currentInst = nullptr;
  const Instruction *nextInst = nullptr;
};

struct ControlFlags {
  bool exitBB = false;
  bool finish = false;
  bool endWithCall = false;
  bool endWithRet = false;
  void reset() {
    exitBB = false;
    finish = false;
    endWithCall = false;
    endWithRet = false;
  }
};

class MemoryManager {
  using PageTable = unordered_map<uint64_t, uint8_t *>;

private:
  const uint16_t PAGE_SIZE = 1024;
  const uint64_t STACK_BASE = 0x001F'0000'0000'0000;
  const uint64_t HEAP_BASE = 0x003F'0000'0000'0000;
  const uint64_t DUMMY_BASE = 0x007F'0000'0000'0000;
  const uint8_t CHUNK_ID_SHIFT = 32;
  const uint64_t CHUNK_ID_MASK = 0x0000'FFFF'0000'0000;
  union DummyValue {
    uint8_t DUMMY_VALUE[10];
    uint64_t dummyValueUInt64[2] = {EMU_MAGIC, EMU_MAGIC};
  } dummyValue;
  bool recordLast = false;
  uint64_t heapID = 0;
  uint64_t dummyID = 0;
  vector<PageTable> memorySnapshots;
  uint64_t getPage(uint64_t addr) { return addr & (~(PAGE_SIZE - 1)); }
  uint64_t getOfs(uint64_t addr) { return addr & (PAGE_SIZE - 1); }
  void insertPage(uint64_t pageIdx);

  template <typename T> T read(uint64_t addr);

  template <typename T> void write(uint64_t addr, T value);

  template <typename T> bool isDummyValue(T value);

public:
  static const uint64_t STACK_ADDR = 4266;
  static const uint64_t GLOBAL_ADDR = 8416;
  static const uint64_t HEAP_ADDR = 8858;
  static const uint64_t CODE_ADDR = 6468;
  MemoryManager() {
    memorySnapshots.emplace_back();
    TR_LOG << "Real Mem Range: " << std::hex << load_info.load_lo << " - "
           << load_info.load_hi << endl;
  }

  uint64_t readQ(uint64_t addr);
  uint32_t readL(uint64_t addr);
  uint16_t readW(uint64_t addr);
  uint8_t readB(uint64_t addr);

  void writeQ(uint64_t addr, uint64_t value);
  void writeL(uint64_t addr, uint32_t value);
  void writeW(uint64_t addr, uint16_t value);
  void writeB(uint64_t addr, uint8_t value);
  bool shouldRecordLastOperation();

  uint64_t getStackBaseAddr() { return STACK_BASE; }

  void clearLocalStack(uint64_t rsp);

  uint64_t allocNewHeapChunk();
  uint64_t allocNewDummyChunk();

  bool validRealMemory(uint64_t addr);

  bool stackAddress(uint64_t addr) {
    const uint64_t STACK_MASK = 0x00F0'0000'0000'0000;
    const uint64_t STACK_SIG = 0x0010'0000'0000'0000;
    return (addr & STACK_MASK) == STACK_SIG;
  }

  bool heapAddress(uint64_t addr) {
    const uint64_t HEAP_MASK = 0x00FF'0000'0000'0000;
    const uint64_t HEAP_SIG = 0x003F'0000'0000'0000;
    return (addr & HEAP_MASK) == HEAP_SIG;
  }

  bool dummyAddress(uint64_t addr) {
    const uint64_t DUMMY_MASK = 0x00FF'0000'0000'0000;
    const uint64_t DUMMY_SIG = 0x007F'0000'0000'0000;
    return (addr & DUMMY_MASK) == DUMMY_SIG;
  }

  bool memAddress(uint64_t addr) {
    return stackAddress(addr) || validRealMemory(addr) || heapAddress(addr) ||
           dummyAddress(addr);
  }

  uint64_t normalizeMemoryAddress(uint64_t addr) {
    if ((!memAddress(addr)) || stackAddress(addr) || validRealMemory(addr)) {
      return 0;
    } else {
      const uint64_t CLEAR_CHUNK_AND_REGION = 0x0000'0000'FFFF'FFFF;
      return addr & CLEAR_CHUNK_AND_REGION;
    }
  }

  void clear();
  void snapshot();
  void rollBack();
};

static inline uint64_t getValueOrReadValue(uint64_t value, MemoryManager &mem) {
  if (mem.memAddress(value)) {
    if (load_info.exec_start <= value && load_info.exec_end >= value) {
      return MemoryManager::CODE_ADDR;
    }
    auto v = mem.readQ(value);
    if (mem.shouldRecordLastOperation()) {
      if (mem.memAddress(v)) {
        if (mem.stackAddress(v)) {
          return MemoryManager::STACK_ADDR;
        } else if (mem.heapAddress(v) || mem.dummyAddress(v)) {
          return mem.normalizeMemoryAddress(v);
        } else if (load_info.exec_start <= v && load_info.exec_end >= v) {
          return MemoryManager::CODE_ADDR;
        } else {
          return MemoryManager::GLOBAL_ADDR;
        }
      }
      return v;
    }
    // unseen memory addr
    return mem.normalizeMemoryAddress(value);
  }
  return value;
}

class Emulator : public InstrVisitor<Emulator, void, const Instruction *> {
  typedef void (Emulator::*LibFunction)(void);
#ifdef EMU_TARGET_I386
  CPUX86State *emuCPUState;
#endif
#ifdef EMU_TARGET_AARCH64
  CPUARMState *emuCPUState;
#endif
  vector<ExecutionContext> ecStack;
  vector<uint64_t> callStack;
  BasicBlockGenerator *bbGenerator;
  ControlFlags emuControlFlags;
  TCIState tciState;
  MemoryManager emuMemory;
  Sampler *sampler;
  PathSemanticCollector *collector;
  // statistics
  unordered_map<BasicBlock *, uint64_t> bbOccurrence;
  unordered_set<uint64_t> codeAddr;
  int64_t bbNumber = 0;
  StatisticManager *statsMngr;
  void logCurrentInst();
  void initialize();
  void reinitialize();
  void reinitializeCPUState();
  void run();
  // control flow functions
  void switchToNewBasicBlock(uint64_t eip);
  void callFunction(uint64_t functionStart);
  void libcall(const string &name);
  void ret();
  // emulated lib calls
  void emu_malloc();
  void emu_calloc();
  void emu_realloc();
  void emu_strcmp();
  void emu_strncmp();
  void emu_strlen();
  uint64_t errnoAddr = 0;
  void emu___errno_location();

  // memory operations
  uint64_t loadValueFromMem(uint64_t addr, MemOp &mop, bool log);
  void storeValueToMem(uint64_t addr, uint64_t value, MemOp &mop);

public:
#define INST(name) void visit_##name(const Instruction *code);
#include "inst.txt"
#undef INST

  Emulator();

  void emulateFunction(PAFunction *func,
                       FunctionSemanticCollector *semCollector,
                       StatisticManager *statsMngr);

  PathSemanticCollector *getPathSemCollector() {
    return this->collector;
  }
  ~Emulator() { delete sampler; }
};

static inline const string normalizeLibName(const string &name) {
  static unordered_map<string, string> normalizeNameTable = {
      {"__stpcpy_chk", "stpcpy"},
      {"__openat_2", "openat"},
      {"__read_chk", "read"},
      {"__uflow", "uflow"},
      {"mempcpy", "mempcpy"},
      {"__fdelt_chk", "fdelt"},
      {"__strtoul_internal", "strtoul"},
      {"__memmove_chk", "memmove"},
      {"__getdelim", "getdelim"},
      {"__open_2", "open"},
      {"__fprintf_chk", "fprintf"},
      {"__strtol_internal", "strtol"},
      {"__ctype_tolower_loc", "tolower"},
      {"__strcpy_chk", "strcpy"},
      {"__fread_unlocked_chk", "fread_unlocked"},
      {"__strncat_chk", "strncat"},
      {"__sprintf_chk", "sprintf"},
      {"__explicit_bzero_chk", "explicit_bzero"},
      {"__ctype_toupper_loc", "toupper"},
      // {"__stack_chk_fail", ""},
      // {"__overflow", ""},
      {"__memcpy_chk", "memcpy"},
      // {"pthread_mutex_destroy", ""},
      {"__printf_chk", "printf"},
      // {"pthread_cond_destroy", ""},
      {"__fread_chk", "fread"},
      {"__snprintf_chk", "snprintf"},
      {"dcngettext", "gettext"},
      {"dcgettext", "gettext"},
      {"fwrite", "fputs"},
  };
  if (normalizeNameTable.count(name)) {
    return normalizeNameTable[name];
  } else {
    return name;
  }
}

#endif