#include "qemu_utils/tci_utils.hh"

typedef struct CPUARMTBFlags {
  uint32_t flags;
  target_ulong flags2;
} CPUARMTBFlags;

/*
This struct only contains the beginning parts of the CPUARM State.
We need to use C interface to create such states.
And we can do pointer casting to read values of GPRs and eflags.
*/
typedef struct CPUARMState {
  /* Regs for current mode.  */
  uint32_t regs[16];

  /* 32/64 switch only happens when taking and returning from
   * exceptions so the overlap semantics are taken care of then
   * instead of having a complicated union.
   */
  /* Regs for A64 mode.  */
  uint64_t xregs[32];
  uint64_t pc;
  /* PSTATE isn't an architectural register for ARMv8. However, it is
   * convenient for us to assemble the underlying state into a 32 bit format
   * identical to the architectural format used for the SPSR. (This is also
   * what the Linux kernel's 'pstate' field in signal handlers and KVM's
   * 'pstate' register are.) Of the PSTATE bits:
   *  NZCV are kept in the split out env->CF/VF/NF/ZF, (which have the same
   *    semantics as for AArch32, as described in the comments on each field)
   *  nRW (also known as M[4]) is kept, inverted, in env->aarch64
   *  DAIF (exception masks) are kept in env->daif
   *  BTYPE is kept in env->btype
   *  all other bits are stored in their correct places in env->pstate
   */
  uint32_t pstate;
  uint32_t aarch64; /* 1 if CPU is in aarch64 state; inverse of PSTATE.nRW */
  /* Cached TBFLAGS state.  See below for which bits are included.  */
  CPUARMTBFlags hflags;

  /* Frequently accessed CPSR bits are stored separately for efficiency.
     This contains all the other bits.  Use cpsr_{read,write} to access
     the whole CPSR.  */
  uint32_t uncached_cpsr;
  uint32_t spsr;

  /* Banked registers.  */
  uint64_t banked_spsr[8];
  uint32_t banked_r13[8];
  uint32_t banked_r14[8];

  /* These hold r8-r12.  */
  uint32_t usr_regs[5];
  uint32_t fiq_regs[5];

  /* cpsr flag cache for faster execution */
  uint32_t CF;            /* 0 or 1 */
  uint32_t VF;            /* V is the bit 31. All other bits are undefined */
  uint32_t NF;            /* N is bit 31. All other bits are undefined.  */
  uint32_t ZF;            /* Z set if zero.  */
  uint32_t QF;            /* 0 or 1 */
  uint32_t GE;            /* cpsr[19:16] */
  uint32_t thumb;         /* cpsr[5]. 0 = arm mode, 1 = thumb mode. */
  uint32_t condexec_bits; /* IT bits.  cpsr[15:10,26:25].  */
  uint32_t btype;         /* BTI branch type.  spsr[11:10].  */
  uint64_t daif;          /* exception masks, in the bits they are in PSTATE */

  uint64_t elr_el[4]; /* AArch64 exception link regs  */
  uint64_t sp_el[4];  /* AArch64 banked stack pointers */

} CPUARMState;

typedef enum {
  CPU_REG_X0,
  CPU_REG_X1,
  CPU_REG_X2,
  CPU_REG_X3,
  CPU_REG_X4,
  CPU_REG_X5,
  CPU_REG_X6,
  CPU_REG_X7,
  CPU_REG_X8,
  CPU_REG_X9,
  CPU_REG_X10,
  CPU_REG_X11,
  CPU_REG_X12,
  CPU_REG_X13,
  CPU_REG_X14,
  CPU_REG_X15,
  CPU_REG_X16,
  CPU_REG_X17,
  CPU_REG_X18,
  CPU_REG_X19,
  CPU_REG_X20,
  CPU_REG_X21,
  CPU_REG_X22,
  CPU_REG_X23,
  CPU_REG_X24,
  CPU_REG_X25,
  CPU_REG_X26,
  CPU_REG_X27,
  CPU_REG_X28,
  CPU_REG_X29,
  CPU_REG_X30,

  /* X31 is either the stack pointer or zero, depending on context.  */
  CPU_REG_SP = 31,
  CPU_REG_XZR = 31,

  // CPU_REG_V0 = 32, CPU_REG_V1, CPU_REG_V2, CPU_REG_V3,
  // CPU_REG_V4, CPU_REG_V5, CPU_REG_V6, CPU_REG_V7,
  // CPU_REG_V8, CPU_REG_V9, CPU_REG_V10, CPU_REG_V11,
  // CPU_REG_V12, CPU_REG_V13, CPU_REG_V14, CPU_REG_V15,
  // CPU_REG_V16, CPU_REG_V17, CPU_REG_V18, CPU_REG_V19,
  // CPU_REG_V20, CPU_REG_V21, CPU_REG_V22, CPU_REG_V23,
  // CPU_REG_V24, CPU_REG_V25, CPU_REG_V26, CPU_REG_V27,
  // CPU_REG_V28, CPU_REG_V29, CPU_REG_V30, CPU_REG_V31,

  // /* Aliases.  */
  // CPU_REG_FP = CPU_REG_X29,
  // CPU_REG_LR = CPU_REG_X30,
  // TCG_AREG0  = CPU_REG_X19,
} CPUReg;

#define CPU_NB_REGS 32