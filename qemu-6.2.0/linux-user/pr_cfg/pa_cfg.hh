#ifndef PR_EMULATOR_PA_CFG_HH
#define PR_EMULATOR_PA_CFG_HH

#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using namespace std;

using PAAddr = uint64_t;

struct PAEdge {
  // XXX: from: end addr of basic block
  PAAddr from;
  // to: start addr of basic block
  PAAddr to;
  bool operator==(const PAEdge &e) const {
    return (this->from == e.from) && (this->to == e.to);
  }
};

template <> struct std::hash<PAEdge> {
  size_t operator()(const PAEdge &e) const {
    return ((e.from & 0xFFFF'FFF0) << 24) | ((e.to & 0xFFFF'FFF0) >> 4);
  }
};

struct PAFunction;

struct PABasicBlock {
  PAAddr begin;
  PAAddr end;
  unordered_set<PAAddr> succs, preds;
  bool exceptionExit;
  PAFunction* parent;
};

struct PALoopInfo {
  PAEdge backEdge;
  PAAddr optionalContinuation;
  PAFunction* parent;
};

struct PAFunction {
  // Function information
  PAAddr funcBegin;
  PAAddr funcEnd;
  unordered_map<PAAddr, PABasicBlock *> beginAddr2Block;
  // we map from **endAddr** to bb because QEMU potentially has different begin
  // addrs with IDA
  unordered_map<PAAddr, PABasicBlock *> endAddr2Block;

  // PreAnalysis Results
  bool doesReturn;
  unordered_set<PABasicBlock *> exceptionExits;
  unordered_set<PAEdge> backEdges;
  unordered_map<PAEdge, PALoopInfo *> loopInfos;
};

struct PAProgram {
  unordered_map<PAAddr, PAFunction *> funcDefs;
};

PAProgram *parsePAProgram(ifstream &fin);

#endif