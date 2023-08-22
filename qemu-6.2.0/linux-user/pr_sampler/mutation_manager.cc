#include "mutation_manager.hh"
#include <cassert>
MutationManager::MutationManager() {}

MutationManager &MutationManager::operator+=(const MutationManager &another) {
  this->mutation.insert(another.mutation.begin(), another.mutation.end());
  return *this;
}

void MutationManager::addMutation(SamplerContext ctx, uint64_t counter,
                                  uint64_t guidance) {
  InstructionInstance i{ctx, counter};
  mutation[i] = guidance;
}

void MutationManager::addChangeState(SamplerContext ctx, uint64_t counter) {
  assert(changeState.empty());
  InstructionInstance i{ctx, counter};
  changeState.emplace(i);
}

bool MutationManager::shouldChangeState(SamplerContext ctx, uint64_t counter) {
  InstructionInstance i{ctx, counter};
  return changeState.count(i);
}

uint64_t MutationManager::guideBranch(SamplerContext ctx, uint64_t counter){
  InstructionInstance i{ctx, counter};
  if(mutation.count(i)){
    return mutation[i];
  }else{
    return 0;
  }
}

bool MutationManager::guideCondiMov(uint64_t branch, uint64_t counter){
  assert(false);
  InstructionInstance i{branch, counter};
  if(mutation.count(i)){
    return mutation[i];
  }else{
    return 0;
  }
}