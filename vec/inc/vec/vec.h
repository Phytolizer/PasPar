#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t** data;
  uint64_t* size;
  uint64_t* capacity;
  uint64_t sizeof_t;
} VecUnpacked;

#define VEC_TYPE(T)    \
  struct {             \
    T* data;           \
    uint64_t size;     \
    uint64_t capacity; \
  }

#define VEC_UNPACK(V)        \
  ((VecUnpacked){            \
      (uint8_t**)&(V)->data, \
      &(V)->size,            \
      &(V)->capacity,        \
      sizeof(*(V)->data),    \
  })

#define VEC_FREE(V) free((V)->data)

#define VEC_PUSH(V, Value) \
  (VecExpand(VEC_UNPACK(V)) ? ((V)->data[(V)->size++] = (Value), true) : false)

#define VEC_POP(V) (V)->data[--(V)->size]

#define VEC_RESERVE(V, Amount) VecReserve(VEC_UNPACK(V), Amount)

bool VecExpand(VecUnpacked v);
bool VecReserve(VecUnpacked v, uint64_t amount);
