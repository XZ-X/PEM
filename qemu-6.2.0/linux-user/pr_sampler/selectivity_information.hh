#ifndef SELECTIVITY_INFORMATION_HH
#define SELECTIVITY_INFORMATION_HH

#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <deque>
#include "pem_config.h"
#include "mutation_manager.hh"

using namespace std;

struct ControlFlowRecord;
struct Path;

// selectivity info is owned by the control flow record
// normally, we don't need to delete it explictly
// except for in the finalization period of an emulation, 
// we need to delete the unused instance.
struct SelectivityInfo {
  int64_t selectivity;
  ControlFlowRecord *cfRecord;
  bool operator<(const SelectivityInfo &other) const;
};

/**
 * Stage 0: Faithful path
 * The first seed path, i.e., the faithful path, is created by the sampler.
 * After a round, the sampler yield the ownership of that seed path to the
 * path planner.
 * 
 * Stage 1: Plan a new path
 * Path planner holds a bunch of paths and decide the next one path.
 * It delivers a mutation manager to the sampler. Mutation managers 
 * record information about when to mutate the branches.
 * 
 * Stage 2: Sample a path according to the mutation manager
 * Sampler uses the guidance from a mutation manager to sample a new path.
 * 
 * Stage 3: Return sampled path to the planner
 * Then it returns the path to the planner. The path owns the mutation manager,
 * which contains the information about how this path is created.
 * 
 * We iteratively do 1-3 until enough paths have been sampled.
 */
class SelectivityPathPlanner {
  multiset<SelectivityInfo> selectivityAllCandidates;
  vector<SelectivityInfo> deterministicCandidates;
  // switch-cases, and those withou selectivity info
  deque<ControlFlowRecord*> mustMutate;
  vector<Path*> paths;
  ControlFlowRecord* currentMutationTarget = nullptr;
  unordered_multimap<SamplerContext, ControlFlowRecord*> seenCFRecords;
  unordered_map<SamplerContext, uint8_t> mutations;
  // record this to manage resource
  // we need to delete these because all other cfrecords are managed by paths
  // but these records are created by us and thus have no related paths
  vector<ControlFlowRecord*> utilCFRecords;
  bool fromLargest = false;
  ControlFlowRecord* selectBySelectivity();

  bool seeRecordBefore(ControlFlowRecord *rcd);

public:
  // call this each time we start a new function
  void reset();
  // switch ownership of `path` to `this`
  void finishOnePath(Path *path);

  MutationManager *nextPath();

  ~SelectivityPathPlanner();

};

#endif