#pragma once

#include <vec/vec.h>

typedef VEC_TYPE(char) String;

String StringMake(const char* left, const char* right);
String StringMakeC(const char* c);
