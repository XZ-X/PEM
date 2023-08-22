#include "sampler.hh"
#include "pr_emulator/emulator.hh"
#include "pem_config.h"
#include <cassert>
#include <iostream>

using namespace std;

const uint64_t Sampler::LOOP_EXIT = ~0xaaabbb;
const uint64_t Sampler::INVALID_CF = ~0xcccddd;

Sampler::Sampler(PAProgram *prog) : prog(prog) {
  for (auto addrFunc : prog->funcDefs) {
    auto func = addrFunc.second;
    endAddr2BB.insert(func->endAddr2Block.begin(), func->endAddr2Block.end());
    beginAddr2BB.insert(func->beginAddr2Block.begin(),
                        func->beginAddr2Block.end());
    loops.insert(func->loopInfos.begin(), func->loopInfos.end());
  }
}

void Sampler::recordSelectivityInfo(int64_t selectivity,
                                    uint64_t currentBlockEnd) {
  // - Note that the recorded selectivity info may not be used
  // - For example, in the loop conditions, the selectivity is not recorded
  // or, in a seen branch, the selectivity is not recorded, either
  // - Thus we record the selectivity info with the block addr.
  // - When use the selectivity info, first check the related addr.
  if (selectivityInfo == nullptr) {
    selectivityInfo = new SelectivityInfo;
  }
  selectivityInfo->selectivity = selectivity;
  selectivityBlockEnd = currentBlockEnd;
}

uint64_t Sampler::guideBranch(uint64_t currentBlockEnd, uint64_t faithfulRIP,
                              SamplerContext ctx) {
  TR_LOG << stateToString(state) << "::";
  // we do not mutate loop edge
  PAEdge e{currentBlockEnd, faithfulRIP};

  if (loops.count(e)) {
    loopTimes[e]++;
    if (loopTimes[e] <= SAMPLER_LOOP_TIME_MAGIC) {
      TR_LOG << "SAMPLER:LP:NO_CHANGE: " << std::hex << currentBlockEnd << endl;
      return faithfulRIP;
    }
    loopTimes[e] = SAMPLER_LOOP_TIME_MAGIC - SECOND_LOOP_MAGIC;
    auto loopInfo = loops[e];
    if (loopInfo->optionalContinuation != 0) {

      TR_LOG << "SAMPLER:LP:CHANGE_TO: " << std::hex
             << loopInfo->optionalContinuation << endl;

      return loopInfo->optionalContinuation;
    } else {
      TR_LOG << "SAMPLER:LP:FAIL_GUIDE " << std::hex << currentBlockEnd << endl;
      return LOOP_EXIT;
    }
  }
  // whether next block contains a back edge
  if (additionalLoops.count(faithfulRIP)) {
    TR_LOG << "SAMPLER:LP:ADDITIONAL_LOOP_BB " << std::hex << faithfulRIP;
    additionalLoopTimes[faithfulRIP]++;
    if (additionalLoopTimes[faithfulRIP] <= SAMPLER_LOOP_TIME_MAGIC) {
      TR_LOG << " no change" << endl;
      return faithfulRIP;
    } else {
      additionalLoopTimes[faithfulRIP] =
          SAMPLER_LOOP_TIME_MAGIC - SECOND_LOOP_MAGIC;
      auto loopInfo = additionalLoops[faithfulRIP];
      if (loopInfo->optionalContinuation != 0) {
        TR_LOG << " break to " << loopInfo->optionalContinuation << endl;
        return loopInfo->optionalContinuation;
      } else {
        TR_LOG << " fail to guide " << endl;
        return LOOP_EXIT;
      }
    }
  }
  // no loop, we try to manipulate this branch
  // no mutate
  if ((!endAddr2BB.count(currentBlockEnd)) ||
      (endAddr2BB[currentBlockEnd]->succs.size() <= 1)) {
    TR_LOG << "SAMPLER:BR:NO_MUTATE " << std::hex << currentBlockEnd << endl;
    return faithfulRIP;
  }
  // record
  TR_LOG << "SAMPLER:BR: " << std::hex << currentBlockEnd;
  instructionOccurrence[ctx]++;
  auto brOccurrence = instructionOccurrence[ctx];
  if (brOccurrence <= SAMPLER_RECORD_MAGIC &&
      (state == FAITHFUL || state == GUIDED)) {
    auto rcd = new BranchRecord(endAddr2BB[currentBlockEnd], brOccurrence,
                                faithfulRIP, ctx);
    if (selectivityInfo != nullptr && selectivityBlockEnd == currentBlockEnd) {
      rcd->selectivityInfo = selectivityInfo;
      selectivityInfo->cfRecord = rcd;
      selectivityInfo = nullptr;
      selectivityBlockEnd = 0;
    }
    rcd->instr = ctx;
    rcd->instanceCnt = brOccurrence;
    rcd->context = currentPath;
    currentPath->branches.emplace_back(rcd);
  }
  // invalid CF
  if (
      // has record
      endAddr2BB.count(currentBlockEnd) &&
      // is a switch block
      (endAddr2BB[currentBlockEnd]->succs.size() > 1) &&
      // faithful succ is not in record
      (!endAddr2BB[currentBlockEnd]->succs.count(faithfulRIP)) &&
      // faithful succ is not the start addr of a valid block
      (!beginAddr2BB.count(faithfulRIP))) {
    TR_LOG << "SAMPLER:BR: Invalid cf at " << std::hex << currentBlockEnd
           << " to " << faithfulRIP << endl;
    return Sampler::INVALID_CF;
  }
  if (state == FAITHFUL || state == GUIDED) {
    TR_LOG << " no change " << faithfulRIP << endl;
    return faithfulRIP;
  } else if (state == GUIDING) {
    if (mngr->shouldChangeState(ctx, brOccurrence)) {
      TR_LOG << " MUTATE ";
      state = GUIDED;
    }
    auto next = mngr->guideBranch(ctx, brOccurrence);
    if (next) {
      TR_LOG << " to " << next << endl;
      return next;
    } else {
      TR_LOG << " keep " << faithfulRIP << endl;
      return faithfulRIP;
    }
  }
  assert(false);
}

bool Sampler::guideCondiMov(uint64_t condiMov, bool faithfulChoice) {
  assert(false);
}

uint64_t Sampler::bbContainsBackEdge(uint64_t start, uint64_t end) {
  if (nonLoopBBs.count(start)) {
    return 0;
  }
  if (additionalLoops.count(start)) {
    auto conti = additionalLoops[start]->optionalContinuation;
    if (conti != 0) {
      return conti;
    } else {
      return LOOP_EXIT;
    }
  }
  for (auto &edgeInfo : loops) {
    if (start <= edgeInfo.first.from && end >= edgeInfo.first.to &&
        end > edgeInfo.first.from) {
      TR_LOG << "BB " << std::hex << start << "~" << end
             << " contains BE: " << edgeInfo.first.from << " -> "
             << edgeInfo.first.to << endl;
      additionalLoops[start] = edgeInfo.second;
      auto conti = edgeInfo.second->optionalContinuation;
      if (conti != 0) {
        return conti;
      } else {
        return LOOP_EXIT;
      }
    }
  }
  nonLoopBBs.insert(start);
  return 0;
}

void Sampler::releaseResources() {
  delete currentPath;
  currentPath = nullptr;
  planner.reset();
  delete selectivityInfo;
  selectivityInfo = nullptr;
  selectivityBlockEnd = 0;
}

bool Sampler::nextRound(Emulator &emu) {
  if (cnt == 0) {
    state = FAITHFUL;
  } else {
    state = GUIDING;
  }
  instructionOccurrence.clear();
  loopTimes.clear();
  additionalLoopTimes.clear();
  if (cnt == 0) {
    // before we run the first path
    currentPath = new Path;
    currentPath->baseMutationContext = nullptr;
    cnt++;
    currentPath->setID(cnt);
    TR_LOG << "********** Path " << currentPath->getID() << " ***********"
           << endl;
    return true;
  } else {
#ifdef PEM_DBG
    currentPath->dumpRecord();
#endif
    auto collector = emu.getPathSemCollector();
    if (collector == nullptr) {
      currentPath->setMetric(0);
    } else {
      currentPath->setMetric(collector->getFeatureLen());
    }
    if (execMode == ExecMode::BRANCH_INFO) {
      for (auto cfr : currentPath->branches) {
        if(cfr->selectivityInfo == nullptr){
          continue;
        }
        BranchInfo info;
        info.addr = cfr->instr.instructionAddr;
        info.selectivity = cfr->selectivityInfo->selectivity < 0
                               ? -cfr->selectivityInfo->selectivity
                               : cfr->selectivityInfo->selectivity;
        collector->addBranchInfo(info);
      }
    }
    planner.finishOnePath(currentPath);
    currentPath = nullptr;
  }  
  if (cnt >= SAMPLER_ROUND_MAGIC || execMode == ExecMode::BRANCH_INFO) {
    releaseResources();
    return false;
  }
  if (auto nextManager = planner.nextPath()) {
    currentPath = new Path;
    currentPath->baseMutationContext = nextManager;
    mngr = nextManager;
  } else {
    releaseResources();
    return false;
  }
  cnt++;
  currentPath->setID(cnt);
  // meta-info about the next path
  TR_LOG << "********** Path " << currentPath->getID() << " ***********"
         << endl;
  TR_LOG << "Mutation lvl:" << mngr->level << endl;
  TR_LOG << "Mutations:" << endl;
  for (auto &mutation : mngr->getMutations()) {
    TR_LOG << std::hex << mutation.first.ctx.instructionAddr << "@"
           << mutation.first.ctx.callingContext << " # " << std::dec
           << mutation.first.occurrence << ": " << std::hex << mutation.second
           << endl;
  }
  return true;
}

void Sampler::initFunction(PAFunction *func) {
  cnt = 0;
  mngr = nullptr;
  selectivityInfo = nullptr;
  planner.reset();
  nonLoopBBs.clear();
  additionalLoops.clear();
  instructionOccurrence.clear();
  loopTimes.clear();
  additionalLoopTimes.clear();
}
