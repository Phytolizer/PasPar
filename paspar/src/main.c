#include <pas/lex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
  fclose(fp);
  PasTokens tokens = PasLex(source);
  VEC_FREE(&source);
  for (uint64_t i = 0; i < tokens.size; ++i) {
    printf("%20s: %.*s\n", kPasTokenTypeNames[tokens.data[i].type],
           (int)tokens.data[i].text.size, tokens.data[i].text.data);
  }
  for (uint64_t i = 0; i < tokens.size; ++i) {
    VEC_FREE(&tokens.data[i].text);
  }
  VEC_FREE(&tokens);
  return 0;
}
