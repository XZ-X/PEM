#include "pr_cfg/pa_cfg.hh"
#include "pr_emulator/emulator.hh"
#include "pr_emulator/visitor.hh"
#include "pr_semantic/branch_info.pb.h"
#include "pr_semantic/sem_features.pb.h"
#include "pr_semantic/semantic_collector.hh"
#include "statistic_manager.hh"
#include "pem_config.h"
#include "pem_params.hh"
#include <boost/filesystem.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/icl/right_open_interval.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <unordered_map>
// #define SELECT_FUNC_ONLY
extern "C" {
#include "pem_helpers.h"
#include "pem_interface.h"
#include "pem_load_info.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
}

using namespace std;
using namespace boost;
using namespace boost::icl;

unordered_map<const void *, const char *> emuIgnoreHelpers, emuUsefulHelpers,
    emuExitHelpers;
static void inline initializeHelpers() {
  for (int64_t i = 0; i < usefulHelperNumber; i++) {
    emuUsefulHelpers[usefulHelpers[i]] = usefulHelpersName[i];
  }
  for (int64_t i = 0; i < ignoreHelperNumber; i++) {
    emuIgnoreHelpers[ignoreHelpers[i]] = ignoreHelpersName[i];
  }
  for (int64_t i = 0; i < exitHelperNumber; i++) {
    emuExitHelpers[exitHelpers[i]] = exitHelpersName[i];
  }
}

unordered_map<uint64_t, string> libFunctions;
unordered_map<uint64_t, string> libData;
unordered_set<string> nonreturnLibFunctions;
static void inline initializeLibFunctions(const string &execName) {
  ifstream plt(execName + ".cxx.plt");
  uint64_t addr;
  string name;
  string type;
  while (!plt.eof()) {
    plt >> std::hex >> addr;
    plt >> type;
    plt >> name;
    libFunctions[load_info.load_bias + addr] = name;
    if (type == "O") {
      libData[load_info.load_bias + addr] = name;
    }
  }
  nonreturnLibFunctions.insert("exit");
  nonreturnLibFunctions.insert("abort");
  nonreturnLibFunctions.insert("_exit");
  nonreturnLibFunctions.insert("__stack_chk_fail");
  // nonreturnLibFunctions.insert("__cxa_atexit");
  nonreturnLibFunctions.insert("error");
}

interval_set<uint64_t, std::less, right_open_interval<uint64_t>::type>
    stringIntervals;
static void inline initializeStringLength(const string &execName) {
  ifstream strLen(execName + ".str");
  uint64_t addr;
  uint64_t len;
  while (!strLen.eof()) {
    strLen >> std::hex >> addr;
    strLen >> std::dec >> len;
    if (len == 0) {
      continue;
    }
    addr += load_info.load_bias;
    right_open_interval<uint64_t>::type strRange(addr, addr + len);
    stringIntervals.add(strRange);
  }
}

PAProgram *paProgram;
void initializeCXXEnv(const string &execName) {
  initializeHelpers();
  initializeLibFunctions(execName);
  initializeStringLength(execName);
  ifstream fin(execName + ".pa");
  paProgram = parsePAProgram(fin);
}

unordered_set<uint64_t> loadSelectFunc(const string &execName) {
  auto selectFin = ifstream(execName + ".select");
  unordered_set<uint64_t> ret;
  uint64_t addr;
  while (!selectFin.eof()) {
    selectFin >> std::hex >> addr;
    ret.insert(addr);
  }
  return ret;
}

string outputBaseName;
uint8_t *randomMem = nullptr;
float *randomSamples = nullptr;
extern uint8_t randomIdx;
void parseCliOptions(int argc, char **argv) {
  int c = 0;
  while (-1 != (c = getopt(argc, argv, "P:A:M:BR:"))) {
    switch (c) {
    case 'R':{
      randomIdx = atoi(optarg);
      break;
    }
    case 'P': {
      assert(optarg != nullptr);
      cout << "Get probabilistic samples..." << endl;
      auto fd = open(optarg, O_RDONLY);
      if (fd == -1) {
        cout << "Cannot open probabilistic samples " << optarg << endl;
        exit(-1);
      }
      randomSamples = (float *)mmap(nullptr, sizeof(float) * SAMPLE_SIZE,
                                    PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
      cout << "Random samples loaded!" << endl;
      break;
    }
    case 'A':
      execMode = ExecMode::ABLATION;
      cout << "We are in ABLATION Mode!!" << endl;
      assert(optarg != nullptr);
      outputBaseName = string(optarg);
      break;
    case 'B':
      execMode = ExecMode::BRANCH_INFO;
      cout << "We are collecting branch info!" << endl;
      break;
    case 'M': {
      assert(optarg != nullptr);
      cout << "Load Probabilistic Memory Content..." << endl;
      auto fd = open(optarg, O_RDONLY);
      if (fd == -1) {
        cerr << "Cannot open probabilistic memory conent!!" << endl;
        exit(-1);
      }
      randomMem = (uint8_t *)mmap(nullptr, PROB_MEM_SIZE,
                                  PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
      cout << "Probabilistic Memory Loaded!" << endl;
      break;
    }
    }
  }
  if (randomMem == nullptr) {
    cout << "Specify the probabilistic memory!" << endl;
    exit(-1);
  }
  if (randomSamples == nullptr) {
    cout << "Specify the probabilistic samples!" << endl;
    exit(-1);
  }
}

#ifdef PEM_DBG
string addr;
#endif
#ifdef EMU_TARGET_I386
CPUX86State *globalCPUState;
#endif
#ifdef EMU_TARGET_AARCH64
CPUARMState *globalCPUState;
#endif
const string SEP = "######";
ExecMode execMode;
ofstream ablationStream;
int main(int argc, char **argv, char **envp) {  
  auto *state = init_state(argc, argv, envp);
  globalCPUState = state;
  string execName = argv[1];
  boost::filesystem::path fullPath(execName);
  string execBaseName = fullPath.leaf().string();
  cout << "Base file name: " << execBaseName << endl;
#ifdef PEM_DBG
  if (argc <= 2) {
    cerr << "Specify the interested function!" << endl;
    exit(-1);
  }
  addr = argv[2];
  uint64_t interestedAddr = stoul(addr);
#endif

  execMode = ExecMode::NORMAL;
  outputBaseName = execName;
  parseCliOptions(argc, argv);
  if (execMode == ExecMode::ABLATION) {
    outputBaseName = outputBaseName + string("/") + execBaseName;
  }

  initializeCXXEnv(execName);
#ifdef SELECT_FUNC_ONLY
  auto selectFunc = loadSelectFunc(execName);
#endif

#ifdef PEM_DBG
  ofstream result(execName + "." + addr + ".sem");
  ofstream dynamicCFGStream(execName + "." + addr + ".dyncfg");
  ofstream coverageStream(execName + "." + addr + ".cov");
#else
  ofstream result(outputBaseName + ".sem");
  ofstream dynamicCFGStream(outputBaseName + ".dyncfg");
  ofstream coverageStream(outputBaseName + ".cov");
  ablationStream = ofstream(outputBaseName + ".abl");
#endif
#ifdef DUMP_PB
  // to prevent memory execeed
  int pbCnt = 0;
  ofstream pbOutput(outputBaseName + to_string(pbCnt++) + ".pb",
                    ios::out | ios::trunc | ios::binary);
  int currentPbCnt = 0;
  pr::Result pbResult;
#endif
  ofstream branchInfoStream;
  binfo::Result pbBranchResult;
  if (execMode == ExecMode::BRANCH_INFO) {
    branchInfoStream = ofstream(outputBaseName + ".branch",
                                ios::out | ios::trunc | ios::binary);
  }
  Emulator emu;
  result << "{" << endl;
  dynamicCFGStream << "{" << endl;
  uint64_t all = paProgram->funcDefs.size();
  uint64_t i = 0;
  auto statsMngr = StatisticManager(paProgram);
  if (execMode == ExecMode::ABLATION) {
    // #ifndef SELECT_FUNC_ONLY
    //     cerr << "Ablation study without select function!!" << endl;
    //     exit(-1);
    // #endif
  } else {
    // #ifdef SELECT_FUNC_ONLY
    //     cerr << "Normal exection model with selected functions!!" << endl;
    //     exit(-1);
    // #endif
  }
  if(randomIdx != 0) {
    cout << "Random index: " << (int)randomIdx << endl;
  }
  for (auto &addrDef : paProgram->funcDefs) {
    auto funcAddr = addrDef.first;
    auto funcDef = addrDef.second;
    cout << i++ << "/" << all << "\t\t" << std::hex << funcAddr << std::dec
         << "\n";
    cout.flush();
#ifdef PEM_DBG
    if (funcAddr != interestedAddr) {
      continue;
    }
#endif
#ifdef SELECT_FUNC_ONLY
    if (!selectFunc.count(funcAddr)) {
      continue;
    }
#endif
    // if (i > 10) {
    //   break;
    // }
    FunctionSemanticCollector collector(funcAddr);
    emu.emulateFunction(funcDef, &collector, &statsMngr);
#ifdef DUMP_PB
    currentPbCnt++;
    auto pbFunc = pbResult.add_funcs();
    collector.dumpPB(*pbFunc);
    if (currentPbCnt % 200 == 0) {
      pbResult.SerializeToOstream(&pbOutput);
      pbOutput.flush();
      pbOutput.close();
      pbOutput = ofstream(outputBaseName + to_string(pbCnt++) + ".pb",
                          ios::out | ios::trunc | ios::binary);
      pbResult.Clear();
      currentPbCnt = 0;
    }

#endif
    if (execMode == ExecMode::BRANCH_INFO) {
      auto pbFunc = pbBranchResult.add_funcs();
      collector.dumpBranchInfo(*pbFunc);
    }
    collector.dumpSemanticFeatures(result);
    result << endl << SEP << endl;
    result.flush();
    collector.dumpDynamicCFG(dynamicCFGStream);
    dynamicCFGStream << "," << endl << SEP << endl;
    dynamicCFGStream.flush();
    ablationStream.flush();
  }

  result << "}";
  dynamicCFGStream << "}";
  coverageStream << statsMngr;
#ifdef DUMP_PB
  if (currentPbCnt > 0) {
    pbResult.SerializeToOstream(&pbOutput);
    pbOutput.flush();
    pbOutput.close();
  }
#endif
  if (execMode == ExecMode::BRANCH_INFO) {
    pbBranchResult.SerializeToOstream(&branchInfoStream);
    branchInfoStream.flush();
    branchInfoStream.close();
  }
  statsMngr.visitBB(0);
  result.close();
  dynamicCFGStream.close();
  coverageStream.close();
  ablationStream.close();
  cout << execBaseName << " done!" << endl;
}