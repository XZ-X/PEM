#ifndef PR_SAMPLER_SAMPLER_HH
#define PR_SAMPLER_SAMPLER_HH

#include "control_flow_record.hh"
#include "mutation_manager.hh"
#include "pr_cfg/pa_cfg.hh"
#include <deque>
#include <unordered_map>
#include <vector>
#include "pem_params.hh"
#include "sampler_context.hh"

using namespace std;

class Emulator;


/// Note that sampler does not care about addr translation
/// Manually map the addr before/after obtain guidance from sampler!
class Sampler {
  enum State {
    // real execution, record all branches
    FAITHFUL,
    // wait for the branch to mutate
    // do not record any branches since we should have already seen them
    GUIDING,
    // record branches, as they may be introduced by mutations
    GUIDED
  };
  string stateToString(const State &s) {
    switch (s) {
    case FAITHFUL:
      return "FAITHFUL";
    case GUIDING:
      return "GUIDING";
    case GUIDED:
      return "GUIDED";
    default:
      assert(false);
    }
  }
  // XXX: Global states introduce dependency between functions.
  // The sampler should be stateless among functions!
  // global information
  PAProgram *prog;
  unordered_map<uint64_t, PABasicBlock *> endAddr2BB, beginAddr2BB;
  unordered_map<PAEdge, PALoopInfo *> loops;
  // current function state
  SelectivityPathPlanner planner;
  int cnt;
  unordered_set<uint64_t> nonLoopBBs;
  unordered_map<uint64_t, PALoopInfo *> additionalLoops;
  // current round state
  unordered_map<SamplerContext, uint64_t> instructionOccurrence;
  MutationManager *mngr = nullptr;
  unordered_map<PAEdge, uint64_t> loopTimes;
  unordered_map<uint64_t, uint64_t> additionalLoopTimes;
  SelectivityInfo *selectivityInfo = nullptr;
  uint64_t selectivityBlockEnd = 0;

  Path *currentPath;
  State state;
  void releaseResources();

public:
  Sampler(PAProgram *prog);

  void recordSelectivityInfo(int64_t selectivity, uint64_t currentBlockEnd);

  void initFunction(PAFunction *func);

  bool nextRound(Emulator& emu);

  uint64_t guideBranch(uint64_t currentBlockEnd, uint64_t faithfulRIP, SamplerContext ctx);

  bool guideCondiMov(uint64_t condiMov, bool faithfulChoice);

  uint64_t bbContainsBackEdge(uint64_t start, uint64_t end);

  static const uint64_t LOOP_EXIT;
  
  static const uint64_t INVALID_CF;
};

#endif