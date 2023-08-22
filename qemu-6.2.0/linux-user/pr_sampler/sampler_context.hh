#ifndef SAMPLER_CONTEXT_HH
#define SAMPLER_CONTEXT_HH

#include "pem_config.h"
#include <unordered_map>

struct SamplerContext {
  uint64_t instructionAddr;
  uint64_t callingContext;

  bool operator==(const SamplerContext &other) const {
    return (instructionAddr == other.instructionAddr) &&
           (callingContext == other.callingContext);
  }

  bool operator<(const SamplerContext &other) const {
    return (instructionAddr < other.instructionAddr) ||
           ((instructionAddr == other.instructionAddr) &&
            (callingContext < other.callingContext));
  }
};


inline ostream& operator<<(ostream& os, const SamplerContext& ctx){
  os << ctx.instructionAddr << "@" << ctx.callingContext;
  return os;
}

template <> struct std::hash<SamplerContext> {
  size_t operator()(const SamplerContext &sc) const {
    return ((sc.instructionAddr) << 24) ^
           ((sc.callingContext & 0xFFFF'FFFF) >> 4);
  }
};

#endif