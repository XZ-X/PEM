#include <cstdint>

#include "control_flow_record.hh"
#include "mutation_manager.hh"
#include "pr_cfg/pa_cfg.hh"
#include "selectivity_information.hh"
#include "pem_config.h"
#include "pem_params.hh"
#include <cstdlib>

using namespace std;

bool SelectivityInfo::operator<(const SelectivityInfo &other) const {
  if (selectivity != other.selectivity) {
    return selectivity < other.selectivity;
  } else {
    return cfRecord->instr < other.cfRecord->instr;
  }
}

void SelectivityPathPlanner::reset() {
  selectivityAllCandidates.clear();
  if (DETERMINISTIC) {
    deterministicCandidates.clear();
  }
  seenCFRecords.clear();
  mustMutate.clear();
  currentMutationTarget = nullptr;
  fromLargest = false;
  // delete all paths
  for (auto path : paths) {
    delete path;
  }
  paths.clear();
  for (auto rcd : utilCFRecords) {
    delete rcd;
  }
  utilCFRecords.clear();
  mutations.clear();
  srand(0x2333);
}

bool SelectivityPathPlanner::seeRecordBefore(ControlFlowRecord *rcd) {
  if (!seenCFRecords.count(rcd->instr)) {
    return false;
  }
  auto range = seenCFRecords.equal_range(rcd->instr);
  for (auto it = range.first; it != range.second; ++it) {
    if (it->second->realOutcome == rcd->realOutcome) {
      return true;
    }
  }
  return false;
}

struct UtilControlFlowRecord : public ControlFlowRecord {
  UtilControlFlowRecord(SamplerContext instr, uint64_t cnt,
                        uint64_t realOutcome)
      : ControlFlowRecord(instr, cnt, realOutcome) {}
  void mutate(MutationManager *mngr, bool last) override { assert(false); }
  bool done() override { return true; }
};

void SelectivityPathPlanner::finishOnePath(Path *path) {
  TR_LOG << "Finish one path" << endl;
  paths.emplace_back(path);
  // find possible switch cases
  unordered_set<ControlFlowRecord *> possibleSwitchCases;
  // for each switch stmts
  vector<ControlFlowRecord *> indicators;
  // we assume the branches are in the execution order
  uint64_t ADDR_THRESHOLD = 0x100;
  int64_t SELECTIVITY_THRESHOLD = 0x100;
  int8_t CASE_NUM_THRESHOLD = 4;
  // add mutation to seen
  if (path->baseMutationContext != nullptr) {
    const auto &mutations = path->baseMutationContext->getMutations();
    const auto &changeState = path->baseMutationContext->getChangeState();
    const auto &target = *mutations.find(changeState);
    const auto cfRecord = new UtilControlFlowRecord(
        changeState.ctx, changeState.occurrence, target.second);
    seenCFRecords.emplace(changeState.ctx, cfRecord);
    utilCFRecords.emplace_back(cfRecord);
    TR_LOG << "Add seen record for mutation " << std::hex
           << changeState.ctx.instructionAddr << "@"
           << changeState.ctx.callingContext << ", " << cfRecord->realOutcome
           << endl;
  }

  for (auto rcd : path->branches) {
    auto seen = seeRecordBefore(rcd);
    if (seen) {
      TR_LOG << "Found a seen record with the same outcome at " << std::hex
             << rcd->instr << endl;
      continue;
    }
    TR_LOG << "Branch record: " << std::hex << rcd->instr << " ";
    if (rcd->selectivityInfo == nullptr) {
      TR_LOG << "No selectivity info" << endl;
      seenCFRecords.emplace(rcd->instr, rcd);
      mustMutate.emplace_back(rcd);
      continue;
    }
    if (indicators.empty()) {
      TR_LOG << "Indicator is empty" << endl;
      indicators.emplace_back(rcd);
      continue;
    }
    auto close = true;
    // close to indicators?
    for (auto indicator : indicators) {
      auto rcdAddr = rcd->instr.instructionAddr;
      auto indicatorAddr = indicator->instr.instructionAddr;
      auto addrClose = (rcdAddr - indicatorAddr < ADDR_THRESHOLD &&
                        rcdAddr >= indicatorAddr) ||
                       (indicatorAddr - rcdAddr < ADDR_THRESHOLD &&
                        rcdAddr <= indicatorAddr);
      if (!addrClose) {
        TR_LOG << "Addr not close to indicator " << indicatorAddr << endl;
        close = false;
        break;
      }
      auto rcdSelectivity = rcd->selectivityInfo->selectivity;
      auto indicatorSelectivity = indicator->selectivityInfo->selectivity;
      rcdSelectivity = rcdSelectivity < 0 ? -rcdSelectivity : rcdSelectivity;
      indicatorSelectivity = indicatorSelectivity < 0 ? -indicatorSelectivity
                                                      : indicatorSelectivity;
      auto selectivityClose =
          (rcdSelectivity - indicatorSelectivity < SELECTIVITY_THRESHOLD) &&
          (rcdSelectivity - indicatorSelectivity > -SELECTIVITY_THRESHOLD);

      if (!selectivityClose) {
        TR_LOG << "Selectivity not close to " << indicatorAddr << " we are "
               << rcdSelectivity << " indicator is " << indicatorSelectivity
               << endl;
        close = false;
        break;
      }
    }

    if (!close) {
      if (indicators.size() > CASE_NUM_THRESHOLD) {
        for (auto indicator : indicators) {
          possibleSwitchCases.emplace(indicator);
          TR_LOG << "Possible switch case " << indicator->instr << endl;
        }
      } else {
        TR_LOG << "Not enough indicators" << endl;
      }
      indicators.clear();
      indicators.emplace_back(rcd);
      continue;
    } else {
      TR_LOG << "Close to indicators" << endl;
      indicators.emplace_back(rcd);
    }
  } // end of for each branch record

  for (auto rcd : possibleSwitchCases) {
    TR_LOG << "Add to must mutate: " << std::hex << rcd->instr << endl;
    seenCFRecords.emplace(rcd->instr, rcd);
    mustMutate.emplace_back(rcd);
  }
  // for all others, simply add selectivity infos to the tree
  for (auto rcd : path->branches) {
    if (possibleSwitchCases.count(rcd) || rcd->selectivityInfo == nullptr) {
      continue;
    }
    if (seeRecordBefore(rcd)) {
      TR_LOG << "Found a seen record with the same outcome at " << std::hex
             << rcd->instr << endl;
      continue;
    }
    seenCFRecords.emplace(rcd->instr, rcd);
    if (DETERMINISTIC) {
      deterministicCandidates.emplace_back(*rcd->selectivityInfo);
    } else {
      selectivityAllCandidates.insert(*rcd->selectivityInfo);
    }
  }
}

ControlFlowRecord *SelectivityPathPlanner::selectBySelectivity() {
  if (DETERMINISTIC && deterministicCandidates.empty()) {
    TR_LOG << "Deterministic! We don't have a candidate" << endl;
    return nullptr;
  } else if (selectivityAllCandidates.empty()) {
    TR_LOG << "We don't have a candidate" << endl;
    return nullptr;
  }
#ifdef PEM_TR
  for (auto &candidate : selectivityAllCandidates) {
    TR_LOG << "Candidate: " << std::hex << candidate.cfRecord->instr << ", "
           << candidate.selectivity << endl;
  }
#endif
  if (DETERMINISTIC) {
    auto deterministic = deterministicCandidates.back();
    deterministicCandidates.pop_back();
    auto rcd = deterministic.cfRecord;
    TR_LOG << "Deterministic! Selected: " << std::hex << rcd->instr << ", "
           << rcd->selectivityInfo->selectivity << endl;
    return rcd;
  }
  auto randNum = rand();
  TR_LOG << "Random number: " << randNum;
  auto factor = randomSamples[(randNum % SAMPLE_SIZE)];
  TR_LOG << ", factor: " << factor;
  auto selectedIdx = (int)(factor * (selectivityAllCandidates.size() - 1));
  TR_LOG << ", idx:" << selectedIdx << endl;
  auto determCandidate = selectivityAllCandidates.end();
  if (selectedIdx != 0 && selectedIdx != selectivityAllCandidates.size() - 1) {
    TR_LOG << "PROBABILISTIC! Add deterministic candidate: ";
    if (factor >= 0.5) {
      determCandidate = selectivityAllCandidates.end();
      determCandidate--;
      mustMutate.push_back(determCandidate->cfRecord);
      TR_LOG << std::hex << determCandidate->cfRecord->instr
             << "::" << determCandidate->cfRecord->selectivityInfo->selectivity
             << endl;
    } else {
      determCandidate = selectivityAllCandidates.begin();
      mustMutate.push_back(determCandidate->cfRecord);
      TR_LOG << std::hex << determCandidate->cfRecord->instr
             << "::" << determCandidate->cfRecord->selectivityInfo->selectivity
             << endl;
    }
  }
  auto it = selectivityAllCandidates.begin();
  for (auto i = 0; i < selectedIdx; i++) {
    it++;
  }
  auto rcd = it->cfRecord;
  selectivityAllCandidates.erase(it);
  if(determCandidate != selectivityAllCandidates.end()) {
    selectivityAllCandidates.erase(determCandidate);
  }
  TR_LOG << "Selected: " << std::hex << rcd->instr << ", "
         << rcd->selectivityInfo->selectivity << endl;
  return rcd;
}

MutationManager *SelectivityPathPlanner::nextPath() {
  if (MUTATION_LEVEL_MAGIC == 0) {
    return nullptr;
  }
  uint8_t MUTATION_CNT_MAGIC = 1;
  // Currently we don't record who has been mutated.
  // We simply assume the branch records coming from paths are the first time
  // we see them.
  auto mngr = new MutationManager;
  TR_LOG << "Select next path" << endl;
  while (true) {
    if (currentMutationTarget != nullptr && !currentMutationTarget->done()) {
      mutations[currentMutationTarget->instr]++;
      TR_LOG << "Mutate current one: " << std::hex
             << currentMutationTarget->instr << endl;
      // first work on the current one!
      auto basePath = currentMutationTarget->context;
      if (basePath->baseMutationContext != nullptr) {
        if (basePath->baseMutationContext->level >= MUTATION_LEVEL_MAGIC) {
          TR_LOG << "Reach max mutation level" << endl;
          currentMutationTarget = nullptr;
          continue;
        }
        (*mngr) += *(basePath->baseMutationContext);
        mngr->level = basePath->baseMutationContext->level + 1;
      }
      currentMutationTarget->mutate(mngr, true);
      return mngr;
    }
    // select a new target
    if (!mustMutate.empty()) {
      currentMutationTarget = mustMutate.front();
      mustMutate.pop_front();
      TR_LOG << "Must mutate is not empty, pop one" << endl;
    } else {
      TR_LOG << "Now we select by selectivity" << endl;
      auto next = selectBySelectivity();
      if (next == nullptr) {
        TR_LOG << "No more selectivity candidates" << endl;
        delete mngr;
        return nullptr;
      }
      currentMutationTarget = next;
    }
    // if we have seen this one, we should skip it
    if (mutations.count(currentMutationTarget->instr)) {
      if (mutations[currentMutationTarget->instr] >= MUTATION_CNT_MAGIC) {
        TR_LOG << "We have already mutated enough times for this instruction: "
               << currentMutationTarget->instr << endl;
        currentMutationTarget = nullptr;
        continue;
      }
    }
  }
}

SelectivityPathPlanner::~SelectivityPathPlanner() {
  for (auto path : paths) {
    delete path;
  }
  paths.clear();
  for (auto rcd : utilCFRecords) {
    delete rcd;
  }
  utilCFRecords.clear();
}
