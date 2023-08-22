#ifndef PR_SAMPLER_CONTROL_FLOW_RECORD_HH
#define PR_SAMPLER_CONTROL_FLOW_RECORD_HH
#include "mutation_manager.hh"
#include "pr_cfg/pa_cfg.hh"
#include "pem_config.h"
#include <cstdint>
#include <vector>
#include "selectivity_information.hh"
#include "sampler_context.hh"

using namespace std;

struct Path;
/// control flow records are identified by (addr, cnt, outcome)
struct ControlFlowRecord {
  SamplerContext instr;
  uint64_t instanceCnt = 0;
  uint64_t realOutcome = 0;
  // we own these selectivity information
  SelectivityInfo* selectivityInfo = nullptr;
  // the context of current control flow record 
  // (how can we reach the same predicate again)
  // we do not own the path information
  const Path* context = nullptr;

  ControlFlowRecord() = default;

  virtual void mutate(MutationManager *mngr, bool last) = 0;

  virtual bool done() = 0;

  virtual ~ControlFlowRecord(){
    delete selectivityInfo;
  }

protected:
  ControlFlowRecord(SamplerContext instr, uint64_t cnt, uint64_t realOutcome)
      : instr(instr), instanceCnt(cnt), realOutcome(realOutcome) {}
};

struct BranchRecord : public ControlFlowRecord {
  PABasicBlock *branchBB = nullptr;

  BranchRecord(PABasicBlock *bb, uint64_t cnt, uint64_t faithfulOutcome, SamplerContext ctx);

  void mutate(MutationManager *mngr, bool last) override;

  bool done() override;

  ~BranchRecord() = default;

private:
  unordered_set<uint64_t> unvisited;
};

struct CondiMovRecord : public ControlFlowRecord {

  void mutate(MutationManager *mngr, bool last) override;

  bool done() override;

  ~CondiMovRecord() = default;

private:
  bool mutated = false;
};

struct Path {
  // uint64_t length;
  vector<ControlFlowRecord *> branches;
  MutationManager *baseMutationContext;
  void setMetric(uint64_t metric) { this->metric = metric; }
  uint64_t getMetric() { return this->metric; }
  void setID(uint64_t id) { this->id = id; }
  uint64_t getID() { return this->id; }

// #ifdef PEM_DBG
  void dumpRecord();
// #endif

  ~Path() {
    delete baseMutationContext;
    baseMutationContext = nullptr;
    for (auto b : branches) {
      delete b;
    }
    branches.clear();
  }

private:
  uint64_t id = 0;
  uint64_t metric = 0;
};

// class MutationBunch {
// private:
//   // delete after finish this mutation
//   Path *const base;
//   bool valid = true;
//   int level = 0;
//   vector<Path *> followingPaths;

// public:
//   MutationBunch(Path *path) : base(path) {}

//   // switch ownership of `path` to `this`
//   void finishOnePath(Path *path);

//   vector<MutationBunch *> nextBunch();

//   MutationManager *nextPath();

//   uint64_t getBaseID() const {
//     if (base == nullptr) {
//       return -1;
//     } else {
//       return base->getID();
//     }
//   }

//   ~MutationBunch();
// };

#endif