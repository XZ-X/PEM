#include "pa_cfg.hh"
#include <cassert>
#include <string>

using namespace std;

// FUNCTION_BEGIN
//  startAddr
//  endAddr
//  doesReturn
//  bbNumber
//  BB_BEGIN
//   begin
//   end
//   exceptionExit
//   SUCC_BEGIN
//    succ1 succ2 ...
//   SUCC_END
//   PREDS_BEGIN
//    pred1 pred2 ...
//   PREDS_END
//  BB_END
//  loopNumber
//  LOOP_BEGIN
//    backEdgeFrom
//    backEdgeTo
//    optionalContinuation
//  LOOP_END
// FUNCTION_END

const string FUNCTION_BEGIN = "FBEGIN";
const string FUNCTION_END = "FEND";
const string BB_BEGIN = "BBEGIN";
const string BB_END = "BEND";
const string SUCC_BEGIN = "VECBE";
const string SUCC_END = "VECEN";
const string PRED_BEGIN = SUCC_BEGIN;
const string PRED_END = SUCC_END;
const string LOOP_BEGIN = "LBEGIN";
const string LOOP_END = "LEND";

static inline PALoopInfo *parsePALoopInfo(ifstream &fin) {
  auto ret = new PALoopInfo;
  fin >> ret->backEdge.from;
  fin >> ret->backEdge.to;
  fin >> ret->optionalContinuation;
  string buffer;
  fin >> buffer;
  assert(buffer == LOOP_END);
  return ret;
}

static inline PABasicBlock *parsePABB(ifstream &fin) {
  auto ret = new PABasicBlock;
  fin >> ret->begin;
  fin >> ret->end;
  fin >> ret->exceptionExit;
  string buffer;
  fin >> buffer;
  assert(buffer == SUCC_BEGIN);
  // parse successors
  fin >> buffer;
  while (buffer != SUCC_END) {
    ret->succs.insert(stol(buffer));
    fin >> buffer;
  }
  fin >> buffer;
  assert(buffer == PRED_BEGIN);
  // parse predecessors
  fin >> buffer;
  while (buffer != PRED_END) {
    ret->preds.insert(stol(buffer));
    fin >> buffer;
  }
  fin >> buffer;
  assert(buffer == BB_END);
  return ret;
}

static inline PAFunction *parsePAFunction(ifstream &fin) {
  auto ret = new PAFunction;
  fin >> ret->funcBegin;
  fin >> ret->funcEnd;
  fin >> ret->doesReturn;
  int bbNum;
  fin >> bbNum;
  string buffer;
  for (int i = 0; i < bbNum; i++) {
    fin >> buffer;
    assert(buffer == BB_BEGIN);
    auto bb = parsePABB(fin);
    bb->parent = ret;
    ret->beginAddr2Block[bb->begin] = bb;
    ret->endAddr2Block[bb->end] = bb;
    if (bb->exceptionExit) {
      ret->exceptionExits.insert(bb);
    }
  }
  int loopNum;
  fin >> loopNum;
  for (int i = 0; i < loopNum; i++) {
    fin >> buffer;
    assert(buffer == LOOP_BEGIN);
    auto loop = parsePALoopInfo(fin);
    loop->parent = ret;
    ret->loopInfos[loop->backEdge] = loop;
    ret->backEdges.insert(loop->backEdge);
  }
  fin >> buffer;
  assert(buffer == FUNCTION_END);
  return ret;
}

PAProgram *parsePAProgram(ifstream &fin) {
  auto ret = new PAProgram;
  string buffer;
  fin >> skipws >> buffer;
  while (!fin.eof()) {        
    assert(buffer == FUNCTION_BEGIN);
    auto func = parsePAFunction(fin);
    ret->funcDefs[func->funcBegin] = func;
    fin >> skipws >> buffer;
  }
  return ret;
}