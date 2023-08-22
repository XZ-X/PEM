#include "emulator.hh"
#include "qemu_utils/bit_op.hh"
#include "qemu_utils/tci_utils.hh"
#include <byteswap.h>

static inline uint64_t getCondiValue(const uint64_t u0, const uint64_t u1,
                                     const TCGCond condition) {
  bool result = false;
  int64_t i0 = u0;
  int64_t i1 = u1;
  switch (condition) {
  case TCG_COND_EQ:
  case TCG_COND_NE:
    if (u0 < u1) {
      return u1 - u0;
    } else {
      return u0 - u1;
    }
  case TCG_COND_LT:
    // a < b
    return i0 - i1;
  case TCG_COND_GE:
    // a >= b <=> not(a < b)
    return i0 - i1;
  case TCG_COND_LE:
    // a <= b <=> not(a > b)
    return i1 - i0;
  case TCG_COND_GT:
    // a > b
    return i1 - i0;
  case TCG_COND_LTU:
    return u0 - u1;
  case TCG_COND_GEU:
    return u0 - u1;
  case TCG_COND_LEU:
    return u1 - u0;
  case TCG_COND_GTU:
    return u1 - u0;
  default:
    assert(false);
  }
}

void Emulator::visit_setcond_i32(const Instruction *code) {
  tci_args_rrrc(*code, &tciState.r0, &tciState.r1, &tciState.r2,
                &tciState.condition);
  auto &regs = tciState.regs;
  auto compareValue =
      getCondiValue(regs[tciState.r1], regs[tciState.r2], tciState.condition);
  sampler->recordSelectivityInfo(compareValue,
                                 ecStack.back().currentBlock->getEndPC() -
                                     load_info.load_bias);
  TR_LOG << "SEMV: Unstable " << compareValue << " ~ " << regs[tciState.r1]
         << " " << regs[tciState.r2] << endl;
  collector->addUnstableValue(compareValue,
                              ecStack.back().currentBlock->getBeginPC());
  regs[tciState.r0] =
      tci_compare32(regs[tciState.r1], regs[tciState.r2], tciState.condition);
}

void Emulator::visit_setcond2_i32(const Instruction *code) { assert(false); }

void Emulator::visit_movcond_i32(const Instruction *code) {
  tci_args_rrrrrc(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3,
                  &tciState.r4, &tciState.condition);
  auto &regs = tciState.regs;
  auto result =
      tci_compare32(regs[tciState.r1], regs[tciState.r2], tciState.condition);
  regs[tciState.r0] = regs[result ? tciState.r3 : tciState.r4];
}

void Emulator::visit_setcond_i64(const Instruction *code) {
  tci_args_rrrc(*code, &tciState.r0, &tciState.r1, &tciState.r2,
                &tciState.condition);
  auto &regs = tciState.regs;
  auto compareValue =
      getCondiValue(regs[tciState.r1], regs[tciState.r2], tciState.condition);
  sampler->recordSelectivityInfo(compareValue,
                                 ecStack.back().currentBlock->getEndPC() -
                                     load_info.load_bias);
  TR_LOG << "SEMV: Unstable " << compareValue << " ~ " << regs[tciState.r1]
         << " " << regs[tciState.r2] << endl;
  collector->addUnstableValue(compareValue,
                              ecStack.back().currentBlock->getBeginPC());
  regs[tciState.r0] =
      tci_compare64(regs[tciState.r1], regs[tciState.r2], tciState.condition);
}

void Emulator::visit_movcond_i64(const Instruction *code) {
  tci_args_rrrrrc(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3,
                  &tciState.r4, &tciState.condition);
  auto &regs = tciState.regs;
  auto result =
      tci_compare64(regs[tciState.r1], regs[tciState.r2], tciState.condition);
  regs[tciState.r0] = regs[result ? tciState.r3 : tciState.r4];
}

void Emulator::visit_mov_i32(const Instruction *code) {
  tci_args_rr(*code, &tciState.r0, &tciState.r1);
  tciState.regs[tciState.r0] = tciState.regs[tciState.r1];
}

void Emulator::visit_mov_i64(const Instruction *code) { visit_mov_i32(code); }

void Emulator::visit_tci_movi(const Instruction *code) {
  tci_args_ri(*code, &tciState.r0, &tciState.t1);
  tciState.regs[tciState.r0] = tciState.t1;
}

static bool inline isCode(uint64_t addr) {
  return load_info.exec_start <= addr && load_info.exec_end > addr;
}

void Emulator::visit_tci_movl(const Instruction *code) {
  tci_args_rl(*code, ecStack.back().nextInst, &tciState.r0, &tciState.ptr);
  auto value = *(uint64_t *)tciState.ptr;
  tciState.regs[tciState.r0] = value;
  logCurrentInst();
  TR_LOG << "SEMV: movl " << std::hex << value << endl;
  if (libFunctions.count(value)) {
    TR_LOG << "Addr of libcall " << libFunctions[value] << endl;
    collector->addLibCall(libFunctions[value],
                          ecStack.back().currentBlock->getBeginPC());
  } else if (stringIntervalContains(value)) {
    string literal((char *)value);
    TR_LOG << "Addr of string literal " << literal << endl;
    collector->addStringLiteral(literal,
                                ecStack.back().currentBlock->getBeginPC());
  } else if (codeAddr.count(value) || isCode(value)) {
    TR_LOG << "Code in record, ignore!" << endl;
  } else if (emuMemory.validRealMemory(value)) {
    // XXX: use hacking to rule out code sections!
    auto interProceduralCF =
        emuControlFlags.endWithCall || emuControlFlags.endWithRet;
    auto currentPC = ecStack.back().currentBlock->getBeginPC();
    auto ofs = (int64_t)(value - currentPC);
    auto intralProcedurualCF = ofs > -0x1000 && ofs < 0x1000;
    if (interProceduralCF || intralProcedurualCF) {
      TR_LOG << " CF, ignore!" << endl;
    } else {
      auto toRecord = getValueOrReadValue(value, emuMemory);
      TR_LOG << "Record: " << std::hex << toRecord << endl;
      collector->addValue(toRecord, ecStack.back().currentBlock->getBeginPC());
    }
  }
}

template <typename T>
static inline void extType(TCIState &tciState, const Instruction *code) {
  tci_args_rr(*code, &tciState.r0, &tciState.r1);
  tciState.regs[tciState.r0] = (T)tciState.regs[tciState.r1];
}

void Emulator::visit_ext8s_i32(const Instruction *code) {
  extType<int8_t>(tciState, code);
}

void Emulator::visit_ext8s_i64(const Instruction *code) {
  extType<int8_t>(tciState, code);
}

void Emulator::visit_ext8u_i32(const Instruction *code) {
  extType<uint8_t>(tciState, code);
}

void Emulator::visit_ext8u_i64(const Instruction *code) {
  extType<uint8_t>(tciState, code);
}

void Emulator::visit_ext16s_i32(const Instruction *code) {
  extType<int16_t>(tciState, code);
}

void Emulator::visit_ext16s_i64(const Instruction *code) {
  extType<int16_t>(tciState, code);
}

void Emulator::visit_ext16u_i32(const Instruction *code) {
  extType<uint16_t>(tciState, code);
}

void Emulator::visit_ext16u_i64(const Instruction *code) {
  extType<uint16_t>(tciState, code);
}

void Emulator::visit_ext32s_i64(const Instruction *code) {
  extType<int32_t>(tciState, code);
}

void Emulator::visit_ext_i32_i64(const Instruction *code) {
  extType<int32_t>(tciState, code);
}

void Emulator::visit_ext32u_i64(const Instruction *code) {
  extType<uint32_t>(tciState, code);
}

void Emulator::visit_extu_i32_i64(const Instruction *code) {
  extType<uint32_t>(tciState, code);
}

#define UNARY_OP(OP)                                                           \
  tci_args_rr(*code, &tciState.r0, &tciState.r1);                              \
  tciState.regs[tciState.r0] = OP(tciState.regs[tciState.r1]);

void Emulator::visit_bswap16_i32(const Instruction *code) { UNARY_OP(bswap_16) }
void Emulator::visit_bswap16_i64(const Instruction *code) { UNARY_OP(bswap_16) }
void Emulator::visit_bswap32_i32(const Instruction *code) { UNARY_OP(bswap_32) }
void Emulator::visit_bswap32_i64(const Instruction *code) { UNARY_OP(bswap_32) }
void Emulator::visit_bswap64_i64(const Instruction *code) { UNARY_OP(bswap_64) }
void Emulator::visit_not_i32(const Instruction *code) { UNARY_OP(~) }
void Emulator::visit_not_i64(const Instruction *code) { visit_not_i32(code); }
void Emulator::visit_neg_i32(const Instruction *code) { UNARY_OP(-) }
void Emulator::visit_neg_i64(const Instruction *code) { visit_neg_i32(code); }
void Emulator::visit_ctpop_i32(const Instruction *code) { UNARY_OP(ctpop32) }
void Emulator::visit_ctpop_i64(const Instruction *code) { UNARY_OP(ctpop64) }

#undef UNARY_OP

#define BINARY_OP(OP)                                                          \
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);               \
  tciState.regs[tciState.r0] =                                                 \
      (tciState.regs[tciState.r1] OP tciState.regs[tciState.r2]);

void Emulator::visit_add_i32(const Instruction *code) { BINARY_OP(+) }
void Emulator::visit_add_i64(const Instruction *code) { BINARY_OP(+) }
void Emulator::visit_sub_i32(const Instruction *code) { BINARY_OP(-) }
void Emulator::visit_sub_i64(const Instruction *code) { BINARY_OP(-) }
void Emulator::visit_mul_i32(const Instruction *code) { BINARY_OP(*) }
void Emulator::visit_mul_i64(const Instruction *code) { BINARY_OP(*) }
void Emulator::visit_and_i32(const Instruction *code) { BINARY_OP(&) }
void Emulator::visit_and_i64(const Instruction *code) { BINARY_OP(&) }
void Emulator::visit_or_i32(const Instruction *code) { BINARY_OP(|) }
void Emulator::visit_or_i64(const Instruction *code) { BINARY_OP(|) }
void Emulator::visit_xor_i32(const Instruction *code) { BINARY_OP(^) }
void Emulator::visit_xor_i64(const Instruction *code) { BINARY_OP(^) }
void Emulator::visit_andc_i32(const Instruction *code) { BINARY_OP(&~) }
void Emulator::visit_andc_i64(const Instruction *code) { BINARY_OP(&~) }
void Emulator::visit_orc_i32(const Instruction *code) { BINARY_OP(| ~) }
void Emulator::visit_orc_i64(const Instruction *code) { BINARY_OP(| ~) }

#undef BINARY_OP

void Emulator::visit_eqv_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      ~(tciState.regs[tciState.r1] ^ tciState.regs[tciState.r2]);
}

void Emulator::visit_eqv_i64(const Instruction *code) { visit_eqv_i32(code); }

void Emulator::visit_nand_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      ~(tciState.regs[tciState.r1] & tciState.regs[tciState.r2]);
}

void Emulator::visit_nand_i64(const Instruction *code) { visit_nand_i32(code); }

void Emulator::visit_nor_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      ~(tciState.regs[tciState.r1] | tciState.regs[tciState.r2]);
}

void Emulator::visit_nor_i64(const Instruction *code) { visit_nor_i32(code); }

void Emulator::visit_div_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      (int32_t)tciState.regs[tciState.r1] / (int32_t)tciState.regs[tciState.r2];
}

void Emulator::visit_divu_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] = (uint32_t)tciState.regs[tciState.r1] /
                               (uint32_t)tciState.regs[tciState.r2];
}

void Emulator::visit_rem_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      (int32_t)tciState.regs[tciState.r1] % (int32_t)tciState.regs[tciState.r2];
}

void Emulator::visit_remu_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] = (uint32_t)tciState.regs[tciState.r1] %
                               (uint32_t)tciState.regs[tciState.r2];
}

void Emulator::visit_div_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      (int64_t)tciState.regs[tciState.r1] / (int64_t)tciState.regs[tciState.r2];
}

void Emulator::visit_divu_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] = (uint64_t)tciState.regs[tciState.r1] /
                               (uint64_t)tciState.regs[tciState.r2];
}

void Emulator::visit_rem_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      (int64_t)tciState.regs[tciState.r1] % (int64_t)tciState.regs[tciState.r2];
}

void Emulator::visit_remu_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] = (uint64_t)tciState.regs[tciState.r1] %
                               (uint64_t)tciState.regs[tciState.r2];
}

void Emulator::visit_shl_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] = (uint32_t)tciState.regs[tciState.r1]
                               << (tciState.regs[tciState.r2] & 31);
}

void Emulator::visit_shl_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] = tciState.regs[tciState.r1]
                               << (tciState.regs[tciState.r2] & 63);
}

void Emulator::visit_shr_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      (uint32_t)tciState.regs[tciState.r1] >> (tciState.regs[tciState.r2] & 31);
}

void Emulator::visit_shr_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      tciState.regs[tciState.r1] >> (tciState.regs[tciState.r2] & 63);
}

void Emulator::visit_sar_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      (int32_t)tciState.regs[tciState.r1] >> (tciState.regs[tciState.r2] & 31);
}

void Emulator::visit_sar_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      (int64_t)tciState.regs[tciState.r1] >> (tciState.regs[tciState.r2] & 63);
}

void Emulator::visit_rotl_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      rol32(tciState.regs[tciState.r1], (tciState.regs[tciState.r2] & 31));
}

void Emulator::visit_rotl_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      rol64(tciState.regs[tciState.r1], (tciState.regs[tciState.r2] & 31));
}

void Emulator::visit_rotr_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      ror32(tciState.regs[tciState.r1], (tciState.regs[tciState.r2] & 31));
}

void Emulator::visit_rotr_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.regs[tciState.r0] =
      ror64(tciState.regs[tciState.r1], (tciState.regs[tciState.r2] & 31));
}

void Emulator::visit_deposit_i32(const Instruction *code) {
  tci_args_rrrbb(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.pos,
                 &tciState.len);
  tciState.regs[tciState.r0] =
      deposit32(tciState.regs[tciState.r1], tciState.pos, tciState.len,
                tciState.regs[tciState.r2]);
}

void Emulator::visit_deposit_i64(const Instruction *code) {
  tci_args_rrrbb(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.pos,
                 &tciState.len);
  tciState.regs[tciState.r0] =
      deposit64(tciState.regs[tciState.r1], tciState.pos, tciState.len,
                tciState.regs[tciState.r2]);
}

void Emulator::visit_extract_i32(const Instruction *code) {
  tci_args_rrbb(*code, &tciState.r0, &tciState.r1, &tciState.pos,
                &tciState.len);
  tciState.regs[tciState.r0] =
      extract32(tciState.regs[tciState.r1], tciState.pos, tciState.len);
}

void Emulator::visit_extract_i64(const Instruction *code) {
  tci_args_rrbb(*code, &tciState.r0, &tciState.r1, &tciState.pos,
                &tciState.len);
  tciState.regs[tciState.r0] =
      extract64(tciState.regs[tciState.r1], tciState.pos, tciState.len);
}

void Emulator::visit_sextract_i32(const Instruction *code) {
  tci_args_rrbb(*code, &tciState.r0, &tciState.r1, &tciState.pos,
                &tciState.len);
  tciState.regs[tciState.r0] =
      sextract32(tciState.regs[tciState.r1], tciState.pos, tciState.len);
}

void Emulator::visit_sextract_i64(const Instruction *code) {
  tci_args_rrbb(*code, &tciState.r0, &tciState.r1, &tciState.pos,
                &tciState.len);
  tciState.regs[tciState.r0] =
      sextract64(tciState.regs[tciState.r1], tciState.pos, tciState.len);
}

void Emulator::visit_clz_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.tmp32 = tciState.regs[tciState.r1];
  tciState.regs[tciState.r0] =
      tciState.tmp32 ? clz32(tciState.tmp32) : tciState.regs[tciState.r2];
}

void Emulator::visit_clz_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.tmp64 = tciState.regs[tciState.r1];
  tciState.regs[tciState.r0] =
      tciState.tmp64 ? clz64(tciState.tmp64) : tciState.regs[tciState.r2];
}

void Emulator::visit_ctz_i32(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.tmp32 = tciState.regs[tciState.r1];
  tciState.regs[tciState.r0] =
      tciState.tmp32 ? ctz32(tciState.tmp32) : tciState.regs[tciState.r2];
}

void Emulator::visit_ctz_i64(const Instruction *code) {
  tci_args_rrr(*code, &tciState.r0, &tciState.r1, &tciState.r2);
  tciState.tmp64 = tciState.regs[tciState.r1];
  tciState.regs[tciState.r0] =
      tciState.tmp64 ? ctz32(tciState.tmp64) : tciState.regs[tciState.r2];
}

void Emulator::visit_add2_i32(const Instruction *code) {
  tci_args_rrrrrr(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3,
                  &tciState.r4, &tciState.r5);
  tciState.T1 =
      tci_uint64(tciState.regs[tciState.r3], tciState.regs[tciState.r2]);
  tciState.T2 =
      tci_uint64(tciState.regs[tciState.r5], tciState.regs[tciState.r4]);
  tci_write_reg64(tciState.regs, tciState.r1, tciState.r0,
                  tciState.T1 + tciState.T2);
}

void Emulator::visit_add2_i64(const Instruction *code) {
  tci_args_rrrrrr(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3,
                  &tciState.r4, &tciState.r5);
  tciState.T1 = tciState.regs[tciState.r2] + tciState.regs[tciState.r4];
  tciState.T2 = tciState.regs[tciState.r3] + tciState.regs[tciState.r5] +
                (tciState.T1 < tciState.regs[tciState.r2]);
  tciState.regs[tciState.r0] = tciState.T1;
  tciState.regs[tciState.r1] = tciState.T2;
}

void Emulator::visit_sub2_i32(const Instruction *code) {
  tci_args_rrrrrr(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3,
                  &tciState.r4, &tciState.r5);
  tciState.T1 =
      tci_uint64(tciState.regs[tciState.r3], tciState.regs[tciState.r2]);
  tciState.T2 =
      tci_uint64(tciState.regs[tciState.r5], tciState.regs[tciState.r4]);
  tci_write_reg64(tciState.regs, tciState.r1, tciState.r0,
                  tciState.T1 - tciState.T2);
}

void Emulator::visit_sub2_i64(const Instruction *code) {
  tci_args_rrrrrr(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3,
                  &tciState.r4, &tciState.r5);
  tciState.T1 = tciState.regs[tciState.r2] - tciState.regs[tciState.r4];
  tciState.T2 = tciState.regs[tciState.r3] - tciState.regs[tciState.r5] -
                (tciState.regs[tciState.r2] < tciState.regs[tciState.r4]);
  tciState.regs[tciState.r0] = tciState.T1;
  tciState.regs[tciState.r1] = tciState.T2;
}

void Emulator::visit_mulu2_i32(const Instruction *code) {
  tci_args_rrrr(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3);
  tciState.tmp64 = (uint64_t)(uint32_t)tciState.regs[tciState.r2] *
                   (uint32_t)tciState.regs[tciState.r3];
  tci_write_reg64(tciState.regs, tciState.r1, tciState.r0, tciState.tmp64);
}

void Emulator::visit_mulu2_i64(const Instruction *code) {
  tci_args_rrrr(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3);
  mulu64(&tciState.regs[tciState.r0], &tciState.regs[tciState.r1],
         tciState.regs[tciState.r2], tciState.regs[tciState.r3]);
}

void Emulator::visit_muls2_i32(const Instruction *code) {
  tci_args_rrrr(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3);
  tciState.tmp64 = (int64_t)(int32_t)tciState.regs[tciState.r2] *
                   (int32_t)tciState.regs[tciState.r3];
  tci_write_reg64(tciState.regs, tciState.r1, tciState.r0, tciState.tmp64);
}

void Emulator::visit_muls2_i64(const Instruction *code) {
  tci_args_rrrr(*code, &tciState.r0, &tciState.r1, &tciState.r2, &tciState.r3);
  muls64(&tciState.regs[tciState.r0], &tciState.regs[tciState.r1],
         tciState.regs[tciState.r2], tciState.regs[tciState.r3]);
}
