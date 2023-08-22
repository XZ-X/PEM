#ifndef PR_EMU_TCI_UTILS_HH
#define PR_EMU_TCI_UTILS_HH

#include "pem_config.h"
#include "bit_op.hh"
#include "mem_op.hh"
#include <cassert>
#include <cstdint>
extern "C"{
  #include "fpu/softfloat-types.h"
}

typedef uint64_t target_ulong;


#define NB_OPMASK_REGS 8

#define TCG_TARGET_REG_BITS 64
#define TCG_TARGET_MAYBE_vec 0
#define TCG_TARGET_INTERPRETER
typedef enum TCGOpcode {
#define DEF(name, oargs, iargs, cargs, flags) CXX_INDEX_op_##name,
#include "include/tcg/tcg-opc.h"
#undef DEF
  NB_OPS,
} TCGOpcode;
#undef TCG_TARGET_INTERPRETER
#undef TCG_TARGET_REG_BITS
#undef TCG_TARGET_MAYBE_vec

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"
#endif


typedef enum {
  TCG_REG_R0 = 0,
  TCG_REG_R1,
  TCG_REG_R2,
  TCG_REG_R3,
  TCG_REG_R4,
  TCG_REG_R5,
  TCG_REG_R6,
  TCG_REG_R7,
  TCG_REG_R8,
  TCG_REG_R9,
  TCG_REG_R10,
  TCG_REG_R11,
  TCG_REG_R12,
  TCG_REG_R13,
  TCG_REG_R14,
  TCG_REG_R15,

  TCG_REG_TMP = TCG_REG_R13,
  TCG_AREG0 = TCG_REG_R14,
  TCG_REG_CALL_STACK = TCG_REG_R15,
} TCGReg;


typedef struct SegmentCache {
    uint32_t selector;
    target_ulong base;
    uint32_t limit;
    uint32_t flags;
} SegmentCache;

typedef union {
    uint8_t _b[16];
    uint16_t _w[8];
    uint32_t _l[4];
    uint64_t _q[2];
} XMMReg;

typedef union {
    uint8_t _b[32];
    uint16_t _w[16];
    uint32_t _l[8];
    uint64_t _q[4];
} YMMReg;


#define MMREG_UNION(n, bits)        \
    union n {                       \
        uint8_t  _b_##n[(bits)/8];  \
        uint16_t _w_##n[(bits)/16]; \
        uint32_t _l_##n[(bits)/32]; \
        uint64_t _q_##n[(bits)/64]; \
        float32  _s_##n[(bits)/32]; \
        float64  _d_##n[(bits)/64]; \
    }


typedef MMREG_UNION(ZMMReg, 512) ZMMReg;
typedef MMREG_UNION(MMXReg, 64)  MMXReg;

typedef struct BNDReg {
    uint64_t lb;
    uint64_t ub;
} BNDReg;

typedef struct BNDCSReg {
    uint64_t cfgu;
    uint64_t sts;
} BNDCSReg;

typedef union {
    floatx80 d __attribute__((aligned(16)));
    MMXReg mmx;
} FPReg;

/*
 * Conditions.  Note that these are laid out for easy manipulation by
 * the functions below:
 *    bit 0 is used for inverting;
 *    bit 1 is signed,
 *    bit 2 is unsigned,
 *    bit 3 is used with bit 0 for swapping signed/unsigned.
 */
typedef enum {
  /* non-signed */
  TCG_COND_NEVER = 0 | 0 | 0 | 0,
  TCG_COND_ALWAYS = 0 | 0 | 0 | 1,
  TCG_COND_EQ = 8 | 0 | 0 | 0,
  TCG_COND_NE = 8 | 0 | 0 | 1,
  /* signed */
  TCG_COND_LT = 0 | 0 | 2 | 0,
  TCG_COND_GE = 0 | 0 | 2 | 1,
  TCG_COND_LE = 8 | 0 | 2 | 0,
  TCG_COND_GT = 8 | 0 | 2 | 1,
  /* unsigned */
  TCG_COND_LTU = 0 | 4 | 0 | 0,
  TCG_COND_GEU = 0 | 4 | 0 | 1,
  TCG_COND_LEU = 8 | 4 | 0 | 0,
  TCG_COND_GTU = 8 | 4 | 0 | 1,
} TCGCond;

static void tci_write_reg64(uint64_t *regs, uint32_t high_index,
                            uint32_t low_index, uint64_t value) {
  regs[low_index] = (uint32_t)value;
  regs[high_index] = value >> 32;
}

/* Create a 64 bit value from two 32 bit values. */
static uint64_t tci_uint64(uint32_t high, uint32_t low) {
  return ((uint64_t)high << 32) + low;
}

/*
 * Load sets of arguments all at once.  The naming convention is:
 *   tci_args_<arguments>
 * where arguments is a sequence of
 *
 *   b = immediate (bit position)
 *   c = condition (TCGCond)
 *   i = immediate (uint32_t)
 *   I = immediate (tcg_target_ulong)
 *   l = label or pointer
 *   m = immediate (MemOpIdx)
 *   n = immediate (call return length)
 *   r = register
 *   s = signed ldst offset
 */

static inline void tci_args_l(uint32_t insn, const void *tb_ptr, void **l0) {
  int diff = sextract32(insn, 12, 20);
  *l0 = diff ? (char *)tb_ptr + diff : nullptr;
}

static inline void tci_args_r(uint32_t insn, TCGReg *r0) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
}

static inline void tci_args_nl(uint32_t insn, const void *tb_ptr, uint8_t *n0,
                               void **l1) {
  *n0 = extract32(insn, 8, 4);
  *l1 = sextract32(insn, 12, 20) + (char *)tb_ptr;
}

static inline void tci_args_rl(uint32_t insn, const void *tb_ptr, TCGReg *r0,
                               void **l1) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *l1 = sextract32(insn, 12, 20) + (char *)tb_ptr;
}

static inline void tci_args_rr(uint32_t insn, TCGReg *r0, TCGReg *r1) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
}

static inline void tci_args_ri(uint32_t insn, TCGReg *r0, uint64_t *i1) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *i1 = sextract32(insn, 12, 20);
}

static inline void tci_args_rrm(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                MemOpIdx *m2) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *m2 = static_cast<MemOpIdx>(extract32(insn, 20, 12));
}

static inline void tci_args_rrr(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                TCGReg *r2) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *r2 = static_cast<TCGReg>(extract32(insn, 16, 4));
}

static inline void tci_args_rrs(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                int32_t *i2) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *i2 = sextract32(insn, 16, 16);
}

static inline void tci_args_rrbb(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                 uint8_t *i2, uint8_t *i3) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *i2 = extract32(insn, 16, 6);
  *i3 = extract32(insn, 22, 6);
}

static inline void tci_args_rrrc(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                 TCGReg *r2, TCGCond *c3) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *r2 = static_cast<TCGReg>(extract32(insn, 16, 4));
  *c3 = static_cast<TCGCond>(extract32(insn, 20, 4));
}

static inline void tci_args_rrrm(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                 TCGReg *r2, MemOpIdx *m3) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *r2 = static_cast<TCGReg>(extract32(insn, 16, 4));
  *m3 = extract32(insn, 20, 12);
}

static inline void tci_args_rrrbb(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                  TCGReg *r2, uint8_t *i3, uint8_t *i4) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *r2 = static_cast<TCGReg>(extract32(insn, 16, 4));
  *i3 = extract32(insn, 20, 6);
  *i4 = extract32(insn, 26, 6);
}

static inline void tci_args_rrrrr(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                  TCGReg *r2, TCGReg *r3, TCGReg *r4) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *r2 = static_cast<TCGReg>(extract32(insn, 16, 4));
  *r3 = static_cast<TCGReg>(extract32(insn, 20, 4));
  *r4 = static_cast<TCGReg>(extract32(insn, 24, 4));
}

static inline void tci_args_rrrr(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                 TCGReg *r2, TCGReg *r3) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *r2 = static_cast<TCGReg>(extract32(insn, 16, 4));
  *r3 = static_cast<TCGReg>(extract32(insn, 20, 4));
}

static inline void tci_args_rrrrrc(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                   TCGReg *r2, TCGReg *r3, TCGReg *r4,
                                   TCGCond *c5) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *r2 = static_cast<TCGReg>(extract32(insn, 16, 4));
  *r3 = static_cast<TCGReg>(extract32(insn, 20, 4));
  *r4 = static_cast<TCGReg>(extract32(insn, 24, 4));
  *c5 = static_cast<TCGCond>(extract32(insn, 28, 4));
}

static inline void tci_args_rrrrrr(uint32_t insn, TCGReg *r0, TCGReg *r1,
                                   TCGReg *r2, TCGReg *r3, TCGReg *r4,
                                   TCGReg *r5) {
  *r0 = static_cast<TCGReg>(extract32(insn, 8, 4));
  *r1 = static_cast<TCGReg>(extract32(insn, 12, 4));
  *r2 = static_cast<TCGReg>(extract32(insn, 16, 4));
  *r3 = static_cast<TCGReg>(extract32(insn, 20, 4));
  *r4 = static_cast<TCGReg>(extract32(insn, 24, 4));
  *r5 = static_cast<TCGReg>(extract32(insn, 28, 4));
}

static bool tci_compare32(uint32_t u0, uint32_t u1, TCGCond condition) {
  bool result = false;
  int32_t i0 = u0;
  int32_t i1 = u1;
  switch (condition) {
  case TCG_COND_EQ:
    result = (u0 == u1);
    break;
  case TCG_COND_NE:
    result = (u0 != u1);
    break;
  case TCG_COND_LT:
    result = (i0 < i1);
    break;
  case TCG_COND_GE:
    result = (i0 >= i1);
    break;
  case TCG_COND_LE:
    result = (i0 <= i1);
    break;
  case TCG_COND_GT:
    result = (i0 > i1);
    break;
  case TCG_COND_LTU:
    result = (u0 < u1);
    break;
  case TCG_COND_GEU:
    result = (u0 >= u1);
    break;
  case TCG_COND_LEU:
    result = (u0 <= u1);
    break;
  case TCG_COND_GTU:
    result = (u0 > u1);
    break;
  default:
    assert(false);
  }
  return result;
}

static bool tci_compare64(uint64_t u0, uint64_t u1, TCGCond condition) {
  bool result = false;
  int64_t i0 = u0;
  int64_t i1 = u1;
  switch (condition) {
  case TCG_COND_EQ:
    result = (u0 == u1);
    break;
  case TCG_COND_NE:
    result = (u0 != u1);
    break;
  case TCG_COND_LT:
    result = (i0 < i1);
    break;
  case TCG_COND_GE:
    result = (i0 >= i1);
    break;
  case TCG_COND_LE:
    result = (i0 <= i1);
    break;
  case TCG_COND_GT:
    result = (i0 > i1);
    break;
  case TCG_COND_LTU:
    result = (u0 < u1);
    break;
  case TCG_COND_GEU:
    result = (u0 >= u1);
    break;
  case TCG_COND_LEU:
    result = (u0 <= u1);
    break;
  case TCG_COND_GTU:
    result = (u0 > u1);
    break;
  default:
    assert(false);
  }
  return result;
}

static inline void mulu64(uint64_t *plow, uint64_t *phigh, uint64_t a,
                          uint64_t b) {
  __uint128_t r = (__uint128_t)a * b;
  *plow = r;
  *phigh = r >> 64;
}

static inline void muls64(uint64_t *plow, uint64_t *phigh, int64_t a,
                          int64_t b) {
  __int128_t r = (__int128_t)a * b;
  *plow = r;
  *phigh = r >> 64;
}

/* compute with 96 bit intermediate result: (a*b)/c */
static inline uint64_t muldiv64(uint64_t a, uint32_t b, uint32_t c) {
  return (__int128_t)a * b / c;
}

static inline uint64_t divu128(uint64_t *plow, uint64_t *phigh,
                               uint64_t divisor) {
  __uint128_t dividend = ((__uint128_t)*phigh << 64) | *plow;
  __uint128_t result = dividend / divisor;

  *plow = result;
  *phigh = result >> 64;
  return dividend % divisor;
}

static inline int64_t divs128(uint64_t *plow, int64_t *phigh, int64_t divisor) {
  __int128_t dividend = ((__int128_t)*phigh << 64) | *plow;
  __int128_t result = dividend / divisor;

  *plow = result;
  *phigh = result >> 64;
  return dividend % divisor;
}

#endif
