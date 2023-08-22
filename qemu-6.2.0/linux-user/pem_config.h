#ifndef PEM_CONFIG_H
#define PEM_CONFIG_H

// #define PEM_DBG
// #define PEM_TR
// #define RANDOM_INIT
// #define ABL_STABLE_EMU_SEED
#define DUMP_PB
#include "pr_cfg/pa_cfg.hh"
#include "target_config.hh"
#include <boost/icl/interval_set.hpp>
#include <boost/icl/right_open_interval.hpp>
#include <unordered_map>
#include <unordered_set>
using namespace boost;
using namespace boost::icl;

extern std::unordered_map<const void *, const char *> emuIgnoreHelpers,
    emuUsefulHelpers, emuExitHelpers;
extern std::unordered_map<uint64_t, std::string> libFunctions;
extern std::unordered_map<uint64_t, std::string> libData;
extern std::unordered_set<std::string> nonreturnLibFunctions;
// extern std::unordered_map<uint64_t, uint64_t> stringLen;
extern interval_set<uint64_t, std::less, right_open_interval<uint64_t>::type>
    stringIntervals;

inline bool stringIntervalContains(uint64_t addr) {
  if(addr == (~0ULL)){
      return false;
  }
  right_open_interval<uint64_t> tmpRange(addr, addr + 1);
  return stringIntervals.find(tmpRange) != stringIntervals.end();
}
extern PAProgram *paProgram;
// max semantic length
extern const uint32_t MAX_OVERALL_SEMANTICS;
// MAGICs
extern const int32_t SAMPLER_ROUND_MAGIC;
extern const uint32_t SAMPLER_LOOP_TIME_MAGIC;
extern const uint32_t SAMPLER_RECORD_MAGIC;
extern const int32_t MUTATION_LEVEL_MAGIC;
extern const int32_t MAX_BB_NUMBER;
extern const int32_t SECOND_LOOP_MAGIC;
extern const uint32_t STRING_LEN_MAGIC;
extern const char *STRING_LIB_MAGIC;
extern const char *MEM_LIB_MAGIC;
extern const int32_t SEED_ROUND;
#ifdef ABL_STABLE_EMU_SEED
extern uint64_t EMU_DEFAULT;
#else
extern const uint64_t EMU_DEFAULT;
#endif
extern const int32_t MUTATION_UPPER;
extern const int32_t MUTATION_LOWER;
extern const bool DETERMINISTIC;
extern const bool PROB_MEM;

// execution mode
enum ExecMode{
 NORMAL, ABLATION, BRANCH_INFO
};
extern ExecMode execMode;
// probabilistic memory
extern const uint32_t PROB_MEM_SIZE;
extern const uint32_t SAMPLE_SIZE;


#ifdef PEM_TR
#define TR_LOG cout
#else
#define TR_LOG                                                                 \
  if (true) {                                                                  \
  } else                                                                       \
    cout
#endif


#endif