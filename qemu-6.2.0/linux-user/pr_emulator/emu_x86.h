#include "qemu_utils/tci_utils.hh"

#define CPU_NB_REGS 16

typedef enum {
  CPU_REG_EAX = 0,
  CPU_REG_ECX,
  CPU_REG_EDX,
  CPU_REG_EBX,
  CPU_REG_ESP,
  CPU_REG_EBP,
  CPU_REG_ESI,
  CPU_REG_EDI,

  /* 64-bit registers; always define the symbols to avoid
     too much if-deffing.  */
  CPU_REG_R8,
  CPU_REG_R9,
  CPU_REG_R10,
  CPU_REG_R11,
  CPU_REG_R12,
  CPU_REG_R13,
  CPU_REG_R14,
  CPU_REG_R15,

  CPU_REG_XMM0,
  CPU_REG_XMM1,
  CPU_REG_XMM2,
  CPU_REG_XMM3,
  CPU_REG_XMM4,
  CPU_REG_XMM5,
  CPU_REG_XMM6,
  CPU_REG_XMM7,

  /* 64-bit registers; likewise always define.  */
  CPU_REG_XMM8,
  CPU_REG_XMM9,
  CPU_REG_XMM10,
  CPU_REG_XMM11,
  CPU_REG_XMM12,
  CPU_REG_XMM13,
  CPU_REG_XMM14,
  CPU_REG_XMM15,

  CPU_REG_RAX = CPU_REG_EAX,
  CPU_REG_RCX = CPU_REG_ECX,
  CPU_REG_RDX = CPU_REG_EDX,
  CPU_REG_RBX = CPU_REG_EBX,
  CPU_REG_RSP = CPU_REG_ESP,
  CPU_REG_RBP = CPU_REG_EBP,
  CPU_REG_RSI = CPU_REG_ESI,
  CPU_REG_RDI = CPU_REG_EDI,

  CPU_AREG0 = CPU_REG_EBP,
  CPU_REG_CALL_STACK = CPU_REG_ESP
} CPUReg;

/*
This struct only contains the beginning parts of the CPUX86 State.
We need to use C interface to create such states.
And we can do pointer casting to read values of GPRs and eflags.
*/
typedef struct CPUX86State {
  /* standard registers */
  // 0x00
  target_ulong regs[CPU_NB_REGS];
  // 0x80
  target_ulong eip;
  // 0x88
  target_ulong eflags; /* eflags register. During CPU emulation, CC
                      flags and DF are set to zero because they are
                      stored elsewhere */
                       /* emulator internal eflags handling */
  // 0x90
  target_ulong cc_dst;
  // 0x98
  target_ulong cc_src;
  // 0xa0
  target_ulong cc_src2;
  // 0xa8
  uint32_t cc_op;  
  int32_t df;       /* D flag : 1 if D = 0, -1 if D = 1 */
  uint32_t hflags;  /* TB flags, see HF_xxx constants. These flags
                       are known at translation time. */
  uint32_t hflags2; /* various other flags, see HF2_xxx constants. */

  /* segments */
  SegmentCache segs[6]; /* selector values */
  SegmentCache ldt;
  SegmentCache tr;
  SegmentCache gdt; /* only base and limit are used */
  SegmentCache idt; /* only base and limit are used */

  target_ulong cr[5]; /* NOTE: cr1 is unused */
  int32_t a20_mask;

  BNDReg bnd_regs[4];
  BNDCSReg bndcs_regs;
  uint64_t msr_bndcfgs;
  uint64_t efer;

  /* Beginning of state preserved by INIT (dummy marker).  */
  struct {
  } start_init_save;

  /* FPU state */
  unsigned int fpstt; /* top of stack index */
  uint16_t fpus;
  uint16_t fpuc;
  uint8_t fptags[8]; /* 0 = valid, 1 = empty */
  FPReg fpregs[8];
  /* KVM-only so far */
  uint16_t fpop;
  uint16_t fpcs;
  uint16_t fpds;
  uint64_t fpip;
  uint64_t fpdp;

  /* emulator internal variables */
  float_status fp_status;
  floatx80 ft0;

  float_status mmx_status; /* for 3DNow! float ops */
  float_status sse_status;
  uint32_t mxcsr;
  ZMMReg xmm_regs[CPU_NB_REGS == 8 ? 8 : 32];
  ZMMReg xmm_t0;
  MMXReg mmx_t0;

  XMMReg ymmh_regs[CPU_NB_REGS];

  uint64_t opmask_regs[NB_OPMASK_REGS];
  YMMReg zmmh_regs[CPU_NB_REGS];
  ZMMReg hi16_zmm_regs[CPU_NB_REGS];
} CPUX86State;

