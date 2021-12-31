#include <pas/lex.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
  PasTokens tokens = PasLex(StringMakeC("AND"));
  for (uint64_t i = 0; i < tokens.size; ++i) {
    printf("%10s: %.*s\n", kPasTokenTypeNames[tokens.data[i].type],
           (int)tokens.data[i].text.size, tokens.data[i].text.data);
  }
  return 0;
}
