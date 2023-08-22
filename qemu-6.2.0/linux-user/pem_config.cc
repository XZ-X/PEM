#include "pem_config.h"

const int32_t SAMPLER_ROUND_MAGIC = 400;
const uint32_t SAMPLER_LOOP_TIME_MAGIC = 20;
const uint32_t SAMPLER_RECORD_MAGIC = 1;
const int32_t MUTATION_LEVEL_MAGIC = 99999;
const int32_t MAX_BB_NUMBER = 6000;
const int32_t SECOND_LOOP_MAGIC = 10;
const uint32_t STRING_LEN_MAGIC = 23;
// const uint32_t STRING_LEN_MAGIC = 5;
const int32_t MUTATION_UPPER = 10;
const int32_t MUTATION_LOWER = 10;
const char* STRING_LIB_MAGIC = "str";
const char* MEM_LIB_MAGIC = "mem";
const uint32_t MAX_OVERALL_SEMANTICS = 24000;
const int32_t SEED_ROUND = 2;
#ifdef ABL_STABLE_EMU_SEED
uint64_t EMU_DEFAULT = 0xadadadad'adadadad;
#else
const uint64_t EMU_DEFAULT = 0xadadadad'adadadad;
#endif
const uint32_t PROB_MEM_SIZE = 256 * 1024;
// const uint32_t PROB_MEM_SIZE = 16 * 1024;
const uint32_t SAMPLE_SIZE = 20000;
const bool DETERMINISTIC = false;
const bool PROB_MEM = true;
