#ifndef PR_SAMPLER_MUTATION_MANAGER_HH
#define PR_SAMPLER_MUTATION_MANAGER_HH

#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <iostream>
#include "sampler_context.hh"

using namespace std;

struct InstructionInstance {
  SamplerContext ctx;
  uint64_t occurrence;

  bool operator==(const InstructionInstance &another) const {
    return this->ctx == another.ctx &&
           this->occurrence == another.occurrence;
  }
};

template <> struct std::hash<InstructionInstance> {
  size_t operator()(const InstructionInstance &i) const {
    return std::hash<SamplerContext>()(i.ctx) + i.occurrence;
  }
};

class SelectivityPathPlanner;
class MutationManager {  
  unordered_set<InstructionInstance> changeState;
  unordered_map<InstructionInstance, uint64_t> mutation;

public:
  MutationManager();

  MutationManager &operator+=(const MutationManager &another);

  void addMutation(SamplerContext ctx, uint64_t counter, uint64_t guidance);

  void addChangeState(SamplerContext ctx, uint64_t counter);

  bool shouldChangeState(SamplerContext branch, uint64_t counter);

  uint64_t guideBranch(SamplerContext branch, uint64_t counter);

  bool guideCondiMov(uint64_t condiMov, uint64_t counter);

  const unordered_map<InstructionInstance, uint64_t> &getMutations() const{
    return mutation;
  }

  const InstructionInstance& getChangeState() const{
    // XXX: we only change state once!
    return *changeState.begin();
  }


  int level = 0;
};

#endif