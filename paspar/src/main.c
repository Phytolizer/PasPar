#include <pas/lex.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  String source = {0};
  FILE* fp = fopen("test.pas", "r");
  if (!fp) {
    fprintf(stderr, "Could not open test.pas\n");
    return 1;
  }
  char buf[1024];
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    VEC_APPEND(&source, buf, strlen(buf));
  }
  PasTokens tokens = PasLex(source);
  for (uint64_t i = 0; i < tokens.size; ++i) {
    printf("%10s: %.*s\n", kPasTokenTypeNames[tokens.data[i].type],
           (int)tokens.data[i].text.size, tokens.data[i].text.data);
  }
  return 0;
}
