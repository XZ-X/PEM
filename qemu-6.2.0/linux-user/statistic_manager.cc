#include "statistic_manager.hh"
#include <unordered_map>

using namespace std;
StatisticManager::StatisticManager(const PAProgram *program)
    : program(program) {
  coverageMap.clear();
  for (auto &addrFunc : program->funcDefs) {
    for (auto &addrBB : addrFunc.second->beginAddr2Block) {
      coverageMap[addrBB.first] = 0;
    }
  }
}

void StatisticManager::visitBB(uint64_t beginAddr) { coverageMap[beginAddr]++; }

ostream &operator<<(ostream &os, const StatisticManager &mngr) {
  for (auto &addrFunc : mngr.program->funcDefs) {
    double covered = 0, all = 0;
    for (auto &addrBB : addrFunc.second->beginAddr2Block) {
      all += 1;
      uint64_t addr = addrBB.first;
      auto coverageInformation = mngr.coverageMap.find(addr);
      if (coverageInformation != mngr.coverageMap.cend() &&
          coverageInformation->second > 0) {
        covered += 1;
      }
    }
    os << std::dec << addrFunc.first << "," << covered / (all+0.0001) << endl;
  }
#ifdef DBG_COV
  os << "====================DBG==================" << endl;
  double covered = 0, all = 0;
  for (auto iter = mngr.coverageMap.cbegin(); iter != mngr.coverageMap.cend();
       ++iter) {
    all++;
    if (iter->second != 0) {
      covered++;
    } else {
      os << "Uncov: " << std::hex << iter->first << endl;
    }
  }
  os << "Coverage: " << covered << "/" << all << " = " << covered / all << endl;
#endif
  return os;
}