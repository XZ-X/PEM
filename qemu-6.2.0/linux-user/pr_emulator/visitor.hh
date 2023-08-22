#ifndef PR_EMULATOR_VISITOR_HH
#define PR_EMULATOR_VISITOR_HH

#include "qemu_utils/tci_utils.hh"
#include <iostream>

typedef uint32_t Instruction;

template<typename SubClass, typename RetTy, typename Code=Instruction*>
class InstrVisitor{

public:
#define INST(name) \
    RetTy visit_##name(Code code){ }
  #include "inst.txt"
#undef INST

  RetTy visitInstr(Code code){
    uint32_t insn = *code;
    auto op = extract32(insn, 0, 8);
    switch (op)
    {        
        #define INST(name) \
        case CXX_INDEX_op_##name:\
          return static_cast<SubClass*>(this)->visit_##name(code);

        #include "inst.txt"
        #undef INST
        default:
        assert(false);      
    }
  }
  
};

struct PrintVisitor:public InstrVisitor<PrintVisitor, void, const Instruction*>{

#define INST(name)\
  void visit_##name(const Instruction* code){\
    std::cout<<#name<<std::endl;\
  }
  #include "inst.txt"
#undef INST

};
#endif