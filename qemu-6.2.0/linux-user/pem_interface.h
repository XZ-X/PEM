#ifndef PEM_INTERFACE_H
#define PEM_INTERFACE_H

#include <stdint.h>

typedef uint32_t Instruction;

extern int foobar(void);

#ifdef EMU_TARGET_I386
struct CPUX86State;
#endif
#ifdef EMU_TARGET_AARCH64
struct CPUARMState;
#endif
struct TranslationBlock;

typedef struct _BasicBlockC{
    TranslationBlock* tb;
    Instruction* code;
    uint64_t size;
    uint64_t beginPC;
    uint64_t endPC;
} BasicBlockC;

#ifdef EMU_TARGET_I386
extern CPUX86State* globalCPUState;
extern CPUX86State* init_state(int argc, char **argv, char **envp);
extern TranslationBlock* translate(CPUX86State* state, uint64_t addr);
#endif

#ifdef EMU_TARGET_AARCH64
extern CPUARMState* globalCPUState;
extern CPUARMState* init_state(int argc, char **argv, char **envp);
extern TranslationBlock* translate(CPUARMState* state, uint64_t addr);
#endif




extern BasicBlockC* createNewBasicBlockC(TranslationBlock* tb);

extern const Instruction* print_tb(TranslationBlock* tb);

// my helpers
extern void helper_call_indicator(void);

extern void helper_ret_indicator(void);

// void exec_tb_with_tci(CPUX86State* state, TranslationBlock* tb);

#endif