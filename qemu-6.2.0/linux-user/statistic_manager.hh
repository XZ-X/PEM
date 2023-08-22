#ifndef PEM_STATS_MNGR
#define PEM_STATS_MNGR

#include "pr_cfg/pa_cfg.hh"
#include <unordered_map>
#include <iostream>

using namespace std;
class StatisticManager{
  const PAProgram* program;
  unordered_map<uint64_t, double> coverageMap;
public:
  StatisticManager(const PAProgram* program);

  void visitBB(uint64_t beginAddr);
  
friend ostream& operator<<(ostream& os, const StatisticManager& mngr);

};


#endif