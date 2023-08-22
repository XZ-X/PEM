#include "pem_helpers.h"

#include "qemu/osdep.h"

#include "cpu.h"
#include "disas/disas.h"
#include "exec/cpu_ldst.h"
#include "exec/exec-all.h"
// #include "exec/helper-gen.h"
// #include "exec/helper-proto.h"
// #include "exec/log.h"
// #include "exec/translator.h"
#include "qemu/host-utils.h"
#include "tcg/tcg-op.h"

#ifdef TARGET_I386
#include "target/i386/tcg/helper-tcg.h"
#define USEFUL_HELPER "x86_useful_helper.txt"
#define IGNORE_HELPER "x86_ignore_helper.txt"
#endif

#ifdef TARGET_AARCH64
// #include "target/arm/helper.h"
#define USEFUL_HELPER "aarch64_useful_helper.txt"
#define IGNORE_HELPER "aarch64_ignore_helper.txt"
#endif

const void *usefulHelpers[] = {
#define PEM_HELPER(name) &name,
#include USEFUL_HELPER
#undef PEM_HELPER
};

const char *usefulHelpersName[] = {
#define PEM_HELPER(name) #name,
#include USEFUL_HELPER
#undef PEM_HELPER
};

const int usefulHelperNumber = sizeof(usefulHelpers) / sizeof(void *);

const void *ignoreHelpers[] = {
#define PEM_HELPER(name) &name,
#include IGNORE_HELPER
#undef PEM_HELPER
};

const char *ignoreHelpersName[] = {
#define PEM_HELPER(name) #name,
#include IGNORE_HELPER
#undef PEM_HELPER
};

const int ignoreHelperNumber = sizeof(ignoreHelpers) / sizeof(void *);

#ifdef TARGET_I386
const void *exitHelpers[] = {&helper_raise_exception, &helper_raise_interrupt};
const char *exitHelpersName[] = {"helper_raise_exception",
                                 "helper_raise_interrupt"};
#endif

#ifdef TARGET_AARCH64
const void *exitHelpers[] = {&helper_exception_with_syndrome,
                             &helper_exception_internal,
                             &helper_exception_bkpt_insn};
const char *exitHelpersName[] = {"helper_exception_with_syndrome",
                                 "helper_exception_internal",
                                 "helper_exception_bkpt_insn"};

#endif

const int exitHelperNumber = sizeof(exitHelpers) / sizeof(void *);