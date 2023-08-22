#include "pem_config.h"
#include "basic_block.hh"
#include "pem_load_info.h"
extern "C"{
  #include "pem_interface.h"
}
#include <iostream>

using namespace std;

BasicBlock::BasicBlock(BasicBlockC *bb) : bb(bb) {}

BasicBlock::~BasicBlock() {
  free(bb->tb);
  bb->tb = nullptr;
  free(bb);
  bb = nullptr;
}

BasicBlockGenerator::BasicBlockGenerator() {
  cache.reserve(8192);
  cpuState = globalCPUState;
}

BasicBlock *BasicBlockGenerator::getBlock(uint64_t pc) {
  if (cache.count(pc)) {
    return cache[pc];
  }
  if (!(pc <= load_info.exec_end && pc >= load_info.exec_start)) {
    cache[pc] = nullptr;
    return nullptr;
  }
  auto tb = translate(cpuState, pc);
#ifdef PEM_DBG
  cout << "Translate " << std::hex << pc << std::dec << endl;
  print_tb(tb);
#endif
  auto bbC = createNewBasicBlockC(tb);
  auto bb = new BasicBlock(bbC);
  cache[pc] = bb;
  return cache[pc];
}