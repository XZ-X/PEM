#ifndef PR_EMULATOR_BASIC_BLOCK_HH
#define PR_EMULATOR_BASIC_BLOCK_HH
#include "pem_config.h"
extern "C" {
#include "pem_interface.h"
}

#include <cstdint>
#include <deque>
#include <unordered_map>
#include <vector>

using namespace std;

// TranslationBlock* tb;
//     Instruction* code;
//     uint64_t size;
//     uint64_t beginPC;
//     uint64_t endPC;

struct BasicBlock {
  BasicBlock(BasicBlockC *bb);
  ~BasicBlock();
  Instruction *getEntry() { return bb->code; }

  uint64_t getSize() const { return bb->size; }

  uint64_t getBeginPC() const { return bb->beginPC; }

  uint64_t getEndPC() const { return bb->endPC; }

private:
  BasicBlockC *bb;
};

struct BasicBlockGenerator {
  static BasicBlockGenerator &getInstance() {
    static BasicBlockGenerator instance;
    return instance;
  }

  BasicBlock *getBlock(uint64_t pc);

private:
  BasicBlockGenerator();
  unordered_map<uint64_t, BasicBlock *> cache;
#ifdef EMU_TARGET_I386
  CPUX86State *cpuState;
#endif
#ifdef EMU_TARGET_AARCH64
  CPUARMState *cpuState;
#endif
};

#endif