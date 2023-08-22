#ifndef PR_SEMANTIC_COLLECTOR_HH
#define PR_SEMANTIC_COLLECTOR_HH

#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "sem_features.pb.h"
#include "branch_info.pb.h"

using namespace std;

struct PathSemanticCollector;

struct Round {
  uint64_t seedValue;
  unordered_map<uint64_t, uint64_t> stableOcc, unstableOcc;
  vector<PathSemanticCollector*> paths;
};

struct BranchInfo{
  uint64_t addr;
  int64_t selectivity;
};

struct SemanticFeature {
  enum Type { STRING, NUMBER } type;
  string str;
  uint64_t num;
  SemanticFeature(Type type, string str, uint64_t num)
      : type(type), str(str), num(num) {}

  SemanticFeature() {
    // default constructor to use vector
  }

  friend ostream &operator<<(ostream &os, const SemanticFeature &feature);
};

struct DynamicBasicBlock {
  unordered_set<uint64_t> succ;
  vector<uint64_t> value;
  vector<string> libCall;
};

struct FunctionSemanticCollector {
  FunctionSemanticCollector(uint64_t funcAddr) : funcAddr(funcAddr) {}
  void dumpSemanticFeatures(ofstream &fout);
  void dumpDynamicCFG(ofstream &fout);
  void aggregate(PathSemanticCollector *pathSemantics);
  void startRound(uint64_t seed);
  void dumpPB(pr::Function& func);
  void dumpBranchInfo(binfo::Function& func);
  ~FunctionSemanticCollector();

private:
  vector<Round> rounds;
  uint64_t funcAddr;
  unordered_map<uint64_t, DynamicBasicBlock> dynamicCFG;
  vector<PathSemanticCollector *> pathSemantics;
};

struct PathSemanticCollector {
  void addLibCall(const string &libName, const uint64_t addr);
  void addValue(const uint64_t v, const uint64_t addr);
  void addVDiff(const uint64_t vd, const uint64_t addr);
  void addUnstableValue(const uint64_t v, const uint64_t addr);
  void addStringLiteral(const string &literal, const uint64_t addr);
  void recordNewBlock(const uint64_t blockAddr);
  void addBranchInfo(const BranchInfo &branchInfo);
  bool full();
  uint64_t getPathLen() const;
  uint64_t getFeatureLen() const;
  void dumpAblation(ofstream &ablationOut);

private:
  vector<pair<uint64_t, uint64_t>> value;
  vector<pair<uint64_t, uint64_t>> unstableValue;
  unordered_map<uint64_t, uint64_t> unstableCounter;
  vector<pair<string, uint64_t>> libCall;
  vector<pair<uint64_t, uint64_t>> valueDiff;
  vector<pair<string, uint64_t>> stringLiterals;
  vector<uint64_t> path;
  vector<SemanticFeature> feature;
  vector<BranchInfo> branches;
  friend FunctionSemanticCollector;
};

#endif