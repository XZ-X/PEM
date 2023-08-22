#include "semantic_collector.hh"
#include "pr_emulator/emulator.hh"
#include "sem_features.pb.h"
#include "pem_config.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iterator>
using namespace std;
#define NO_DYN_CFG

static string inline clearInvisible(const string &s) {
  string ret = s;
  for (int i = 0; i < s.length(); i++) {
    if (!iswprint(s[i])) {
      ret[i] = '?';
    }
  }
  return ret;
}

ostream &operator<<(ostream &os, const SemanticFeature &feature) {
  switch (feature.type) {
  case SemanticFeature::NUMBER:
    os << (uint64_t)feature.num;
    break;
  case SemanticFeature::STRING:
    os << "\"" << feature.str << "\"";
    break;
  }
  return os;
}

template <typename InputIterator>
static inline void dumpSeq(ofstream &fout, const string &name,
                           const InputIterator begin, const InputIterator end,
                           string wrap = "") {
  fout << "'" << name << "':[";
  for (auto i = begin; i != end; i++) {
    fout << wrap << *i << wrap << ",";
  }

  fout << "],";
}

template <typename T>
static inline void dumpVector(ofstream &fout, const string &name,
                              const vector<T> &vec, string wrap = "") {
  fout << "'" << name << "':[";
  for (auto &featureBB : vec) {
    fout << wrap << featureBB.first << wrap << ",";
  }

  fout << "],";
}

static void inline dumpRound(ofstream &fout, const Round &rnd) {
  fout << "{" << endl;
  fout << "'seed':" << rnd.seedValue << "," << endl;
  fout << "'stable':{" << endl;
  for (auto &vo : rnd.stableOcc) {
    fout << vo.first << ":" << vo.second << ",";
  }
  fout << "}," << endl;
  fout << "'unstable':{" << endl;
  for (auto &vo : rnd.unstableOcc) {
    fout << vo.first << ":" << vo.second << ",";
  }
  fout << "}" << endl;
  fout << "}," << endl;
}

void FunctionSemanticCollector::dumpSemanticFeatures(ofstream &fout) {
#ifdef DUMP_PB
  return;
#endif
  fout << funcAddr;
  fout << ':';
  fout << "{'pathwise':[\n";
  for (auto p : pathSemantics) {
    fout << "{";
    // dumpVector(fout, "value", p->value);
    // fout << endl;
    // dumpVector(fout, "vdiff", p->valueDiff);
    // fout << endl;
    dumpVector(fout, "libcall", p->libCall, "'");
    fout << endl;
    dumpVector(fout, "literal", p->stringLiterals, "\"");
    fout << endl;
    // dumpVector(fout, "unstable", p->unstableValue, "\"");
    // fout << endl;
    // dumpVector(fout, "overall", p->feature);
    dumpSeq(fout, "overall", p->feature.begin(), p->feature.end());
    fout << "},\n";
  }
  fout << "],";
  fout << "'aggregated':[";
  for (auto &rnd : rounds) {
    dumpRound(fout, rnd);
  }
  fout << "]";
  fout << "}," << endl;
}

void FunctionSemanticCollector::dumpDynamicCFG(ofstream &fout) {
#ifdef NO_DYN_CFG
  return;
#endif
  fout << funcAddr;
  fout << ':';
  fout << "{\n" << std::dec;
  for (auto &bbSuccs : dynamicCFG) {
    fout << bbSuccs.first;
    fout << ":{";
    dumpSeq(fout, "succs", bbSuccs.second.succ.begin(),
            bbSuccs.second.succ.end());
    fout << endl;
    dumpSeq(fout, "value", bbSuccs.second.value.begin(),
            bbSuccs.second.value.end());
    fout << endl;
    dumpSeq(fout, "libcall", bbSuccs.second.libCall.begin(),
            bbSuccs.second.libCall.end(), "'");
    fout << "},";
    fout << endl;
  }
  fout << "}";
}

FunctionSemanticCollector::~FunctionSemanticCollector() {
  for (auto p : pathSemantics) {
    delete p;
  }
  pathSemantics.clear();
}

void FunctionSemanticCollector::startRound(uint64_t seed) {
  rounds.emplace_back();
  rounds.back().seedValue = seed;
}

void FunctionSemanticCollector::dumpPB(pr::Function &func) {
  func.set_addr(funcAddr);
  for (auto round : rounds) {
    auto pbRound = func.add_rounds();
    pbRound->set_seed(round.seedValue);
    for (auto path : round.paths) {
      auto pbPath = pbRound->add_paths();
      for (const auto &v : path->value) {
        pbPath->add_value(v.first);
      }
      for (const auto &libcall : path->libCall) {
        pbPath->add_libcall(libcall.first);
      }
      for (const auto &literal : path->stringLiterals) {
        pbPath->add_literal(literal.first);
      }
      for (const auto &unstable : path->unstableValue) {
        pbPath->add_unstable(unstable.first);
      }
      for (const auto &feature : path->feature) {
        auto pbFeature = pbPath->add_overall();
        switch (feature.type) {
        case SemanticFeature::NUMBER:
          pbFeature->set_num(feature.num);
          pbFeature->set_type(pr::DataType::NUMBER);
          break;
        case SemanticFeature::STRING:
          pbFeature->set_str(feature.str);
          pbFeature->set_type(pr::DataType::STRING);
          break;
        }
      }
    }
  }
}

void FunctionSemanticCollector::dumpBranchInfo(binfo::Function& func){
  func.set_addr(funcAddr);
  for (auto& round:rounds){
    auto pbRound = func.add_rounds();
    pbRound->set_seed(round.seedValue);
    assert(round.paths.size() == 1);
    auto path = round.paths.front();
    for(auto& branchInfo:path->branches){
      auto pbBranchInfo = pbRound->add_branches();
      pbBranchInfo->set_addr(branchInfo.addr);
      pbBranchInfo->set_selectivity(branchInfo.selectivity);
    }
  }
}

void FunctionSemanticCollector::aggregate(PathSemanticCollector *pathSemantic) {

  const auto &path = pathSemantic->path;
  unordered_set<uint64_t> localStableOcc{}, localUnstableOcc{};
  for (auto &valueAddr : pathSemantic->value) {
    localStableOcc.insert(valueAddr.first);
  }
  for (auto &usAddr : pathSemantic->unstableValue) {
    localUnstableOcc.insert(usAddr.first);
  }
  for (auto v : localStableOcc) {
    rounds.back().stableOcc[v] += 1;
  }
  for (auto v : localUnstableOcc) {
    rounds.back().unstableOcc[v] += 1;
  }

#ifndef NO_DYN_CFG
  auto pathLen = path.size();
  if (pathLen > 1) {
    auto prev = path[0];
    for (int i = 1; i < pathLen; i++) {
      auto current = path[i];
      dynamicCFG[prev];
      dynamicCFG[prev].succ.insert(current);
      prev = current;
    }
  }
  for (auto valueBB : pathSemantic->value) {
    dynamicCFG[valueBB.second].value.push_back(valueBB.first);
  }
  for (auto callBB : pathSemantic->libCall) {
    dynamicCFG[callBB.second].libCall.push_back(callBB.first);
  }
#endif
  pathSemantics.emplace_back(pathSemantic);
  rounds.back().paths.emplace_back(pathSemantic);
#ifndef DUMP_PB
  // keep path information for ablation study
  if (execMode != ExecMode::ABLATION) {
    pathSemantic->path.clear();
  }
#endif
}

void PathSemanticCollector::addLibCall(const string &libName,
                                       const uint64_t addr) {
  auto name = normalizeLibName(libName);
  TR_LOG << "SEM:Libcall " << libName << "::::" << name << endl;
  if (boost::starts_with(name, "str")) {
    name = STRING_LIB_MAGIC;
  } else if (boost::starts_with(name, "mem")) {
    name = MEM_LIB_MAGIC;
  } else if (boost::algorithm::contains(name, "print")) {
    name = string("print");
  }
  libCall.emplace_back(name, addr);
  feature.emplace_back(SemanticFeature::STRING, name, 0);
  TR_LOG << "size: " << std::dec << feature.size() << endl;
}

void PathSemanticCollector::addValue(const uint64_t v, const uint64_t addr) {
  TR_LOG << "SEM:Value " << std::hex << v << " " << std::dec << v << endl;
  value.emplace_back(v, addr);
  feature.emplace_back(SemanticFeature::NUMBER, "", v);
  TR_LOG << "size: " << std::dec << feature.size() << endl;
}

void PathSemanticCollector::addUnstableValue(const uint64_t v,
                                             const uint64_t addr) {
  if (unstableValue.size() > 2048) {
    TR_LOG << "SEM: Fail to add unstable value " << std::hex << v << " "
           << std::dec << v << endl;
    return;
  }
  unstableCounter[addr] += 1;
  if (unstableCounter[addr] > 3) {
    TR_LOG << "SEM: possible loop! don't add this unstable value!" << endl;
  } else {
    TR_LOG << "SEM: Add unstable value " << std::hex << v << " " << std::dec
           << v << endl;
    unstableValue.emplace_back(v, addr);
  }
}

void PathSemanticCollector::addVDiff(const uint64_t vd, const uint64_t addr) {
  assert(false);
  // // TODO: add to feature list
  // valueDiff.emplace_back(vd, addr);
}

void PathSemanticCollector::addBranchInfo(const BranchInfo &branchInfo) {
  branches.emplace_back(branchInfo);
}

void PathSemanticCollector::recordNewBlock(const uint64_t blockAddr) {
  path.emplace_back(blockAddr);
}

uint64_t PathSemanticCollector::getPathLen() const { return path.size(); }

uint64_t PathSemanticCollector::getFeatureLen() const { return feature.size(); }

void PathSemanticCollector::addStringLiteral(const string &literal,
                                             const uint64_t addr) {
  if (literal.length() > STRING_LEN_MAGIC) {
    TR_LOG << "SEM:Literal: " << literal.substr(0, STRING_LEN_MAGIC) << endl;
    auto toRecord = literal.substr(0, STRING_LEN_MAGIC);
    toRecord = clearInvisible(toRecord);
    boost::replace_all(toRecord, "\n", " ");
    boost::replace_all(toRecord, "\"", "'");
    boost::replace_all(toRecord, "\\", "\\\\");
    stringLiterals.emplace_back(toRecord, addr);
    feature.emplace_back(SemanticFeature::STRING, toRecord, 0);
    TR_LOG << "size: " << feature.size() << endl;
  } else {
    auto toRecord = clearInvisible(literal);
    TR_LOG << "SEM:Literal: " << toRecord << endl;
    boost::replace_all(toRecord, "\n", " ");
    boost::replace_all(toRecord, "\"", "'");
    boost::replace_all(toRecord, "\\", "\\\\");
    stringLiterals.emplace_back(toRecord, addr);
    feature.emplace_back(SemanticFeature::STRING, toRecord, 0);
    TR_LOG << "size: " << std::dec << feature.size() << endl;
  }
}

bool PathSemanticCollector::full() {
  return feature.size() > MAX_OVERALL_SEMANTICS;
}

void PathSemanticCollector::dumpAblation(ofstream &ablationStream) {
  ablationStream << "[";
  for (auto &valueAddr : value) {
    ablationStream << "(" << valueAddr.first << "," << valueAddr.second
                   << ",'V'"
                   << "),";
  }
  for (auto &valueAddr : unstableValue) {
    ablationStream << "(" << valueAddr.first << "," << valueAddr.second
                   << ",'U'"
                   << "),";
  }
  ablationStream << "]";
}
