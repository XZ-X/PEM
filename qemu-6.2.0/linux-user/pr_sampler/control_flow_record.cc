#include "control_flow_record.hh"
#include "mutation_manager.hh"
#include "pem_config.h"
#include <algorithm>
#include <cassert>
#ifdef PEM_DBG
#include "pr_emulator/emulator.hh"
#endif

using namespace std;

BranchRecord::BranchRecord(PABasicBlock *bb, uint64_t cnt,
                           uint64_t faithfulOutcome, SamplerContext ctx)
    : ControlFlowRecord(ctx, cnt, faithfulOutcome), branchBB(bb) {
  for (auto succ : branchBB->succs) {
    if (succ != faithfulOutcome) {
      unvisited.insert(succ);
    }
  }
}

bool BranchRecord::done() { return unvisited.empty(); }

void BranchRecord::mutate(MutationManager *mngr, bool last) {
  if (last) {
    mngr->addChangeState(instr, instanceCnt);
  }
  if (!unvisited.empty()) {
    auto mutatedTarget = *unvisited.begin();
    mngr->addMutation(instr, instanceCnt, mutatedTarget);
    unvisited.erase(mutatedTarget);
  }
  // mngr->addMutation(instAddr, instanceCnt, )
}

void CondiMovRecord::mutate(MutationManager *mngr, bool last) {
  if (last) {
    mngr->addChangeState(instr, instanceCnt);
  }
  mutated = true;
  mngr->addMutation(instr, instanceCnt, 1);
}

bool CondiMovRecord::done() { return mutated; }

void Path::dumpRecord() {
  TR_LOG << "PATH RECORDS:" << endl;
#ifdef PEM_TR
  int i = 0;
  for (auto rcd : this->branches) {
    TR_LOG << std::dec << i++ << ": ";
    TR_LOG << std::hex << rcd->instr.instructionAddr << "@"
           << rcd->instr.callingContext << "," << std::dec << rcd->instanceCnt
           << "," << std::hex << rcd->realOutcome << "   ";
    if (rcd->selectivityInfo != nullptr) {
      if (rcd->selectivityInfo->cfRecord != rcd) {
        TR_LOG << "ERROR: selectivityInfo->cfRecord != rcd" << endl;
        assert(false);
      }
      TR_LOG << std::dec << rcd->selectivityInfo->selectivity
             << ":::" << std::hex << rcd->selectivityInfo->selectivity;
    }
    TR_LOG << endl;
    if (rcd->context != this) {
      TR_LOG << "ERROR: context is not correct" << endl;
      assert(false);
    }
  }
#endif
}