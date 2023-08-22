#include "../pem_config.h"
#include "emulator.hh"
#include "qemu_utils/tci_utils.hh"
#include <boost/algorithm/string/predicate.hpp>
#include <ffi.h>
#include <iostream>
#include <vector>

extern "C" {
extern __thread uintptr_t tci_tb_ptr;
extern void *helper_cc_compute_all;
}

using namespace std;

// this br instruction should jump inside current bb
void Emulator::visit_br(const Instruction *code) {
  auto insn = *code;
  auto &currentEC = ecStack.back();
  tci_args_l(insn, ecStack.back().nextInst, (void **)&currentEC.nextInst);
}

void Emulator::visit_brcond_i32(const Instruction *code) {
  tci_args_rl(*code, ecStack.back().nextInst, &tciState.r0, &tciState.ptr);
  if ((uint32_t)tciState.regs[tciState.r0]) {
    ecStack.back().nextInst = (Instruction *)tciState.ptr;
  }
}

void Emulator::visit_brcond_i64(const Instruction *code) {
  tci_args_rl(*code, ecStack.back().nextInst, &tciState.r0, &tciState.ptr);
  if (tciState.regs[tciState.r0]) {
    ecStack.back().nextInst = (Instruction *)tciState.ptr;
  }
}

void Emulator::visit_exit_tb(const Instruction *code) {
  emuControlFlags.exitBB = true;
}

void Emulator::visit_goto_tb(const Instruction *code) { assert(false); }

void Emulator::visit_goto_ptr(const Instruction *code) { assert(false); }

void Emulator::visit_call(const Instruction *code) {
  tci_args_nl(*code, ecStack.back().nextInst, &tciState.len, &tciState.ptr);
  void **pptr = (void **)tciState.ptr;
  void *cif = pptr[1];
  void *fn = pptr[0];

  if (fn == &helper_call_indicator) {
    emuControlFlags.endWithCall = true;
    return;
  } else if (fn == &helper_ret_indicator) {
    emuControlFlags.endWithRet = true;
    return;
  }

  if (emuExitHelpers.count(fn)) {
    emuControlFlags.finish = true;
    return;
  }
  if (emuIgnoreHelpers.count(fn)) {
    return;
  }
  if(!emuUsefulHelpers.count(fn)){
    std::cerr << std::hex << (void*)helper_call_indicator << std::endl;
    std::cerr << std::hex << fn << std::endl;
    assert(emuUsefulHelpers.count(fn));    
  }
  auto name = emuUsefulHelpers[fn];
  if ("helper_str_indicator" == string(name)) {
    TR_LOG << "SEM Helper:Libcall: string indicator" << endl;
    collector->addLibCall(STRING_LIB_MAGIC,
                          ecStack.back().currentBlock->getBeginPC());
  } else {
    TR_LOG << "SEM Helper:Libcall: " << name << endl;
    collector->addLibCall(name, ecStack.back().currentBlock->getBeginPC());
  }
  // TODO PEM
#ifdef EMU_TARGET_I386
  if (fn == &helper_cc_compute_all) {
#endif
#ifdef EMU_TARGET_AARCH64
    if (true) {
#endif
      TR_LOG << "CALL HELPER: " << name << endl;
      if (tciState.callSlots[0] == nullptr) {
        for (int i = 0; i < tciState.STACK_SIZE; i++) {
          tciState.callSlots[i] = &tciState.stack[i];
        }
      }
      tci_tb_ptr = (uintptr_t) const_cast<Instruction *>(code);
      ffi_call((ffi_cif *)cif, (void (*)())fn, tciState.stack,
               tciState.callSlots);
      switch (tciState.len) {
      case 0: // void
        break;
      case 1: // u32
        if (sizeof(ffi_arg) == 4) {
          tciState.regs[TCG_REG_R0] = *(uint32_t *)tciState.stack;
          break;
        }
        // fall through
      case 2:
        tciState.regs[TCG_REG_R0] = *(uint64_t *)tciState.stack;
        break;
      default:
        assert(false);
      }
    }
    return;
    assert(false);
    auto helperName = emuUsefulHelpers[fn];
#ifdef PEM_TR
    cout << "CALL HELPER " << helperName << endl;
#endif
    // if (tciState.callSlots[0] == nullptr) {
    //   for (int i = 0; i < tciState.STACK_SIZE; i++) {
    //     tciState.callSlots[i] = &tciState.stack[i];
    //   }
    // }
    // tci_tb_ptr = (uintptr_t) const_cast<Instruction *>(code);
    // ffi_call((ffi_cif *)cif, (void (*)())fn, tciState.stack,
    // tciState.callSlots); switch (tciState.len) { case 0: // void
    //   break;
    // case 1: // u32
    //   if (sizeof(ffi_arg) == 4) {
    //     tciState.regs[TCG_REG_R0] = *(uint32_t *)tciState.stack;
    //     break;
    //   }
    //   // fall through
    // case 2:
    //   tciState.regs[TCG_REG_R0] = *(uint64_t *)tciState.stack;
    //   break;
    // default:
    //   assert(false);
    // }
  }
