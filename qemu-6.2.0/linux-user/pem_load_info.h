
#ifndef PEM_LOAD_INFO_H
#define PEM_LOAD_INFO_H
#include <stdint.h>

typedef struct _LoadInfo{
  int loaded;
  uint64_t load_bias;
  uint64_t exec_start;
  uint64_t exec_end;  
  uint64_t data_start;
  uint64_t data_end;
  uint64_t load_lo;
  uint64_t load_hi;
  uint64_t entry;
}LoadInfo;

extern LoadInfo load_info;

#endif