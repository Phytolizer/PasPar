#include "pas/string.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

String StringMake(const char* left, const char* right) {
  String s = {0};
  VEC_RESERVE(&s, right - left);
  for (const char* c = left; c < right; c++) {
    VEC_PUSH(&s, *c);
  }
  return s;
}

String StringMakeC(const char* c) {
  String s = {0};
  VEC_RESERVE(&s, strlen(c));
  for (const char* p = c; *p != '\0'; p++) {
    VEC_PUSH(&s, *p);
  }
  return s;
}
