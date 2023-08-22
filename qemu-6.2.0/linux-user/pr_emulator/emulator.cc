#include "emulator.hh"
#include "pr_cfg/basic_block.hh"
#include "pr_cfg/pa_cfg.hh"
#include "pr_sampler/sampler.hh"
#include "qemu_utils/tci_utils.hh"
#include "pem_config.h"
#include <iostream>

extern "C" {
#include "pem_load_info.h"
}

#ifdef RANDOM_INIT
#define EMU_MAGIC rand()
#endif

#ifdef EMU_TARGET_I386
#define EMU_PC (emuCPUState->eip)
#define CPU_REG_SP CPU_REG_ESP
#endif

#ifdef EMU_TARGET_AARCH64
#define EMU_PC (emuCPUState->pc)
#define CPU_REG_SP CPU_REG_SP
#endif

using namespace std;

Emulator::Emulator() {
  emuCPUState = globalCPUState;
  bbGenerator = &BasicBlockGenerator::getInstance();
  sampler = new Sampler(paProgram);
  for (auto &p : paProgram->funcDefs) {
    codeAddr.insert(p.first + load_info.load_bias);
    for (auto &bAddr2Blk : p.second->beginAddr2Block) {
      codeAddr.insert(bAddr2Blk.first + load_info.load_bias);
    }
  }
}

void Emulator::initialize() {
  reinitialize();
  codeAddr.clear();
}
#ifdef EMU_TARGET_I386
void Emulator::reinitializeCPUState() {
  emuCPUState->eflags = 0;
  emuCPUState->cc_dst = 0;
  emuCPUState->cc_src = 0;
  emuCPUState->cc_src2 = 0;
  emuCPUState->cc_op = 0;
  emuCPUState->fpstt = 0;
  emuCPUState->fpuc = 0;
  for (auto &t : emuCPUState->fptags) {
    t = 0;
  }
  for (auto &freg : emuCPUState->fpregs) {
    for (auto &b : freg.mmx._q_MMXReg) {
      b = EMU_MAGIC;
    }
  }
  TR_LOG << "Initialize with number " << EMU_MAGIC << endl;
  emuCPUState->ft0.high = EMU_MAGIC;
  emuCPUState->ft0.low = EMU_MAGIC;
  for (auto &b : emuCPUState->xmm_t0._q_ZMMReg) {
    b = EMU_MAGIC;
  }
  for (auto &b : emuCPUState->mmx_t0._q_MMXReg) {
    b = EMU_MAGIC;
  }
  for (auto &reg : emuCPUState->regs) {
    reg = EMU_MAGIC;
  }
  for (auto &freg : emuCPUState->xmm_regs) {
    for (auto &b : freg._q_ZMMReg) {
      b = EMU_MAGIC;
    }
  }
  for (auto &freg : emuCPUState->ymmh_regs) {
    for (auto &b : freg._q) {
      b = EMU_MAGIC;
    }
  }
  for (auto &freg : emuCPUState->zmmh_regs) {
    for (auto &b : freg._q) {
      b = EMU_MAGIC;
    }
  }
  for (auto &freg : emuCPUState->hi16_zmm_regs) {
    for (auto &b : freg._q_ZMMReg) {
      b = EMU_MAGIC;
    }
  }
  emuCPUState->regs[CPU_REG_RSP] = emuMemory.getStackBaseAddr();
}
#endif
#ifdef EMU_TARGET_AARCH64
void Emulator::reinitializeCPUState() {
  // XXX: SP!
  for (auto &reg : emuCPUState->regs) {
    reg = EMU_MAGIC;
  }
  for (auto &reg : emuCPUState->xregs) {
    reg = EMU_MAGIC;
  }
  emuCPUState->pstate = 0;
  emuCPUState->aarch64 = 1;
  emuCPUState->uncached_cpsr = 0;
  emuCPUState->spsr = 0;
  emuCPUState->CF = 0;
  emuCPUState->VF = 0;
  emuCPUState->NF = 0;
  emuCPUState->ZF = 0;
  emuCPUState->QF = 0;
  emuCPUState->GE = 0;
  emuCPUState->thumb = 0;
  emuCPUState->condexec_bits = 0;
  emuCPUState->btype = 0;
  emuCPUState->daif = 0;
  emuCPUState->xregs[CPU_REG_SP] = emuMemory.getStackBaseAddr();
}
#endif

void Emulator::reinitialize() {
#ifdef RANDOM_INIT
  srand(2333);
#endif
  if(!PROB_MEM){
    srand(2333);
  }
  tciState.callSlots[0] = nullptr;
  errnoAddr = 0;
  emuMemory.clear();
  emuControlFlags.reset();
  reinitializeCPUState();
  tciState.regs[TCG_AREG0] = (uint64_t)emuCPUState;
  tciState.regs[TCG_REG_CALL_STACK] = (uint64_t)tciState.stack;
  bbNumber = MAX_BB_NUMBER;
  bbOccurrence.clear();
  callStack.clear();
  ecStack.clear();
}

void Emulator::run() {
  auto &currentEC = ecStack.back();
  collector->recordNewBlock(currentEC.currentBlock->getBeginPC());
  statsMngr->visitBB(currentEC.currentBlock->getBeginPC() -
                     load_info.load_bias);
#ifdef PEM_TR
  TR_LOG << "========BEFORE " << std::hex << EMU_PC << "=====" << endl;
#ifdef EMU_TARGET_I386
  for (int i = 0; i < CPU_NB_REGS; i++) {
    TR_LOG << i * 8 << ":" << emuCPUState->regs[i] << endl;
  }
#endif
#ifdef EMU_TARGET_AARCH64
  for (int i = 0; i < 16; i++) {
    TR_LOG << i * 4 << ":" << emuCPUState->regs[i] << endl;
  }
  const auto base = 16 * 4;
  for (int i = 0; i < 32; i++) {
    TR_LOG << i * 8 + base << ":" << emuCPUState->xregs[i] << endl;
  }
#endif
  TR_LOG << "==================" << endl;
#endif
  while (!emuControlFlags.finish) {
    if (!emuControlFlags.exitBB) {
      currentEC.currentInst = currentEC.nextInst++;
      visitInstr(currentEC.currentInst);
    } else {
      if (collector->full()) {
        emuControlFlags.finish = true;
        continue;
      }

      switchToNewBasicBlock(EMU_PC);

      if (!emuControlFlags.finish) {
        collector->recordNewBlock(currentEC.currentBlock->getBeginPC());
        statsMngr->visitBB(currentEC.currentBlock->getBeginPC() -
                           load_info.load_bias);
      }
#ifdef PEM_TR
      TR_LOG << "========BEFORE " << std::hex << EMU_PC << "=====" << endl;
#ifdef EMU_TARGET_I386
      for (int i = 0; i < CPU_NB_REGS; i++) {
        TR_LOG << i * 8 << ":" << emuCPUState->regs[i] << endl;
      }
#endif
#ifdef EMU_TARGET_AARCH64
      for (int i = 0; i < 16; i++) {
        TR_LOG << i * 4 << ":" << emuCPUState->regs[i] << endl;
      }
      const auto base = 16 * 4;
      for (int i = 0; i < 32; i++) {
        TR_LOG << i * 8 + base << ":" << emuCPUState->xregs[i] << endl;
      }
#endif
      TR_LOG << "==================" << endl;

#endif
      bbNumber--;
    }
  }
}

void Emulator::switchToNewBasicBlock(uint64_t eip) {
  auto currentBB = ecStack.back().currentBlock;
  auto currentEndPC = currentBB->getEndPC();
  TR_LOG << "TR:BB:" << std::hex << currentBB->getBeginPC() << "~"
         << currentEndPC << "  ->  " << eip << endl;
  //  operate call stack
  if (emuControlFlags.endWithCall) {
    emuControlFlags.endWithCall = false;
    TR_LOG << "CALL:" << std::hex << eip << endl;
    bool recursive = false;
    for (auto s : callStack) {
      if (s == eip) {
        TR_LOG << "Recursive call! Skip!" << endl;
        recursive = true;
        break;
      }
    }
    callStack.emplace_back(eip);
    if (recursive) {
      ret();
    }
  } else if (emuControlFlags.endWithRet) {
    emuControlFlags.endWithRet = false;
    if (callStack.size() > 0) {
      callStack.pop_back();
    }

#ifdef EMU_TARGET_I386
    emuMemory.clearLocalStack(emuCPUState->regs[CPU_REG_RSP]);
#endif
#ifdef EMU_TARGET_AARCH64
    emuMemory.clearLocalStack(emuCPUState->xregs[CPU_REG_SP]);
#endif
    TR_LOG << "RET:" << std::hex << eip << endl;
  }
  // handle libcall
  if (libFunctions.count(eip)) {
    if (nonreturnLibFunctions.count(libFunctions[eip])) {
      TR_LOG << "Non-Return libFUnc:" << libFunctions[eip] << endl;
      emuControlFlags.finish = true;
      return;
    }
    libcall(libFunctions[eip]);
    ret();

    eip = EMU_PC;

  } else {
    // normal block switch

    codeAddr.insert(EMU_PC);
    SamplerContext ctx;
    ctx.instructionAddr = currentEndPC - load_info.load_bias;
    if (callStack.size() > 1) {
      // TODO:FIX
      // ctx.callingContext = callStack[callStack.size() - 2];
      ctx.callingContext = 0x233;
    } else {
      ctx.callingContext = 0x233;
    }
    eip = sampler->guideBranch(currentEndPC - load_info.load_bias,
                               EMU_PC - load_info.load_bias, ctx);

    eip += load_info.load_bias;
    TR_LOG << "Look for guidance, guide to " << std::hex << eip << endl;
  }
re_generate:
  if (bbNumber <= 0) {
    TR_LOG << "Exit because too many blocks " << std::hex << eip << endl;
    emuControlFlags.finish = true;
    return;
  }
  if (eip == Sampler::LOOP_EXIT + load_info.load_bias ||
      eip == Sampler::INVALID_CF + load_info.load_bias) {
    TR_LOG << "Exit because LOOP/INVALID " << std::hex << eip << endl;
    emuControlFlags.finish = true;
    collector->addValue(eip, ecStack.back().currentBlock->getBeginPC());
    return;
  }
  auto bb = bbGenerator->getBlock(eip);
  if (bb == nullptr) {
    TR_LOG << "Exit because no valid bb " << std::hex << eip << endl;
    emuControlFlags.finish = true;
  } else {
    auto &ec = ecStack.back();
    ec.currentBlock = bb;
    ec.currentInst = nullptr;
    ec.nextInst = bb->getEntry();
    emuControlFlags.exitBB = false;
    bbOccurrence[bb]++;
    if (bbOccurrence[bb] > SAMPLER_LOOP_TIME_MAGIC + 2) {
      TR_LOG << "Potentially miss a loop! Analysis " << std::hex
             << bb->getBeginPC() << endl;

      auto next =
          sampler->bbContainsBackEdge(bb->getBeginPC() - load_info.load_bias,
                                      bb->getEndPC() - load_info.load_bias);
      if (next) {
        // break only one loop at a time
        bbOccurrence.clear();
        eip = next;
        eip += load_info.load_bias;
        goto re_generate;
      }
    }
    if (bbOccurrence[bb] > (SAMPLER_LOOP_TIME_MAGIC + 2) * SECOND_LOOP_MAGIC) {
      TR_LOG << "LOOP too many times, break! " << std::hex << bb->getBeginPC()
             << endl;
      eip = Sampler::LOOP_EXIT;
      eip += load_info.load_bias;
      goto re_generate;
    }
  }
}

#ifdef EMU_TARGET_I386
// emulate the return operation
void Emulator::ret() {
  auto &rsp = emuCPUState->regs[CPU_REG_RSP];
  auto retAddr = emuMemory.readQ(rsp);
  rsp += 8;
  emuCPUState->eip = retAddr;
  TR_LOG << "RET:" << std::hex << retAddr << endl;
  if (callStack.size() > 0) {
    callStack.pop_back();
  }
}
#endif

#ifdef EMU_TARGET_AARCH64
// emulate the return operation
void Emulator::ret() {
  auto addr = emuCPUState->xregs[CPU_REG_X30];
  EMU_PC = addr;
  TR_LOG << "RET:" << std::hex << addr << endl;
  if (callStack.size() > 0) {
    callStack.pop_back();
  }
}
#endif
#ifndef RANDOM_INIT
uint64_t EMU_MAGIC = 0xadadadad'adadadad;
#endif

uint8_t randomIdx;
void Emulator::emulateFunction(PAFunction *func,
                               FunctionSemanticCollector *semCollector,
                               StatisticManager *statsMngr) {
  this->statsMngr = statsMngr;
  uint64_t magics[] = {
      EMU_DEFAULT,
      0x81818181'81818181,
      0xc1c1c1c1'c1c1c1c1,
      0x3e3e3e3e'3e3e3e3e,
      0x75757575'75757575,
      0x7a7a7a7a'7a7a7a7a,
      0x7f7f7f7f'7f7f7f7f,      
      0x89898989'89898989,
      0x8e8e8e8e'8e8e8e8e,
      0x93939393'93939393,
      0x98989898'98989898,
      0x9d9d9d9d'9d9d9d9d,
      0x4d4d4d4d'4d4d4d4d,
      0x0,
  };
  #ifdef ABL_STABLE_EMU_SEED
  EMU_MAGIC = magics[randomIdx];
  #endif
  // EMU_MAGIC = 0xadadadad'adadadad;
  // initialize values
  initialize();
  
  uint64_t len = 0;
  ABL_OUT << "PR_FUNC:" << func->funcBegin << endl;
  for (int k = 0; k < SEED_ROUND; k++) {
#ifndef RANDOM_INIT
    EMU_MAGIC = magics[k];
#endif
#ifdef ABL_STABLE_EMU_SEED
    EMU_MAGIC = magics[randomIdx+k];
#endif
    ABL_OUT << "PR_ROUND:" << (uint64_t)EMU_MAGIC << endl;
    semCollector->startRound(EMU_MAGIC);
    sampler->initFunction(func);
    int i = 0;
    while (sampler->nextRound(*this)) {
      reinitialize();
      collector = new PathSemanticCollector;
      TR_LOG << std::dec << "**************ROUND" << i++ << "  SEED " << k
             << "  *********" << endl;
      callFunction(func->funcBegin + load_info.load_bias);
      len = max(collector->getPathLen(), len);
      // aggregate
      if (execMode == ExecMode::ABLATION) {
        // collector->dumpAblation(ablationStream);
        ABL_OUT << endl;
        semCollector->aggregate(collector);
      } else if (execMode == ExecMode::NORMAL ||
                 execMode == ExecMode::BRANCH_INFO) {
        semCollector->aggregate(collector);
      } else {
        cout << "Emulation is not supported in current mode!" << endl;
        assert(false);
      }
    } // end of emulation (paths) loop
  }   // end of for loop
#ifndef RANDOM_INIT
  EMU_MAGIC = EMU_DEFAULT;
#endif
  cout << "Longest path: " << std::dec << len << endl;
}

void Emulator::callFunction(uint64_t functionStart) {
  ExecutionContext ec;
  auto bb = bbGenerator->getBlock(functionStart);
  ec.currentBlock = bb;
  ec.currentInst = nullptr;
  ec.nextInst = bb->getEntry();
  EMU_PC = functionStart;
  ecStack.push_back(ec);
  run();
}

void Emulator::logCurrentInst() {
  TR_LOG << "visit: " << std::hex << ecStack.back().currentInst << std::dec
         << endl;
}