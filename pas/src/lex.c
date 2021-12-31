#include "pas/lex.h"

#include <stdbool.h>
#include <stdint.h>
#include <uthash.h>

typedef struct {
  String text;
  uint64_t line;
  uint64_t column;
  uint64_t position;
} Lexer;

#define LEXER_LOOK(L, Offset)                 \
  ((L)->position + (Offset) >= (L)->text.size \
       ? '\0'                                 \
       : (L)->text.data[(L)->position + (Offset)])
#define LEXER_CUR(L) LEXER_LOOK(L, 0)
#define LEXER_PEEK(L) LEXER_LOOK(L, 1)
#define LEXER_NEXT(L)           \
  do {                          \
    if (LEXER_CUR(L) == '\n') { \
      (L)->line++;              \
      (L)->column = 1;          \
    } else {                    \
      (L)->column++;            \
    }                           \
    (L)->position++;            \
  } while (0)
#define LEXER_TEXT(L)                                         \
  StringMake((L)->text.data + (L)->position - token.position, \
             (L)->text.data + (L)->position)

static void LexExponent(Lexer* lexer);

static bool IsIdentifierStart(char c);
static bool IsDigit(char c);
static bool IsSign(char c);
static bool IsIdentifierPart(char c);

PasTokens PasLex(String text) {
  Lexer lexer = {
      .text = text,
      .line = 1,
      .column = 1,
      .position = 0,
  };
  PasTokens tokens = {0};
  while (lexer.position < lexer.text.size) {
    PasToken token = {
        .line = lexer.line,
        .column = lexer.column,
        .position = lexer.position,
    };
    // loop will handle comments and whitespace
    while (token.type == kPasTokenTypeZero) {
      if (IsIdentifierStart(LEXER_CUR(&lexer))) {
        while (IsIdentifierPart(LEXER_CUR(&lexer))) {
          LEXER_NEXT(&lexer);
        }
        token.type = kPasTokenTypeIdent;
        token.text = LEXER_TEXT(&lexer);
      } else if (IsDigit(LEXER_CUR(&lexer))) {
        while (IsDigit(LEXER_CUR(&lexer))) {
          LEXER_NEXT(&lexer);
        }
        if (LEXER_CUR(&lexer) == '.') {
          LEXER_NEXT(&lexer);
          while (IsDigit(LEXER_CUR(&lexer))) {
            LEXER_NEXT(&lexer);
          }
          LexExponent(&lexer);
          token.type = kPasTokenTypeNumReal;
        } else if (LEXER_CUR(&lexer) == 'e') {
          LexExponent(&lexer);
          token.type = kPasTokenTypeNumReal;
        } else {
          token.type = kPasTokenTypeNumInt;
        }
        token.text = LEXER_TEXT(&lexer);
      } else {
        switch (LEXER_CUR(&lexer)) {
          case '{': {
            bool found = false;
            for (uint64_t i = 0;
                 LEXER_LOOK(&lexer, i) != '\n' && LEXER_LOOK(&lexer, i) != '\0';
                 ++i) {
              if (LEXER_LOOK(&lexer, i) == '}') {
                for (uint64_t j = 0; j <= i; ++j) {
                  LEXER_NEXT(&lexer);
                }
                found = true;
                break;
              }
            }
            if (found) {
              token.type = kPasTokenTypeComment1;
            } else {
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeLCurly;
            }
            token.text = LEXER_TEXT(&lexer);
          } break;
          case '}':
            token.type = kPasTokenTypeRCurly;
            token.text = LEXER_TEXT(&lexer);
            LEXER_NEXT(&lexer);
            break;
          case '(':
            if (LEXER_PEEK(&lexer) == '*') {
              LEXER_NEXT(&lexer);
              LEXER_NEXT(&lexer);
              while (LEXER_CUR(&lexer) != '*' || LEXER_PEEK(&lexer) != ')') {
                if (LEXER_PEEK(&lexer) == '\0') {
                  break;
                }
                LEXER_NEXT(&lexer);
              }
              LEXER_NEXT(&lexer);
              LEXER_NEXT(&lexer);
            } else if (LEXER_PEEK(&lexer) == '.') {
              LEXER_NEXT(&lexer);
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeLBracket2;
              token.text = LEXER_TEXT(&lexer);
            } else {
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeLParen;
              token.text = LEXER_TEXT(&lexer);
            }
            break;
          case ' ':
          case '\t':
          case '\r':
          case '\n':
            LEXER_NEXT(&lexer);
            break;
          case '.':
            if (LEXER_PEEK(&lexer) == '.') {
              LEXER_NEXT(&lexer);
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeDotDot;
            } else if (LEXER_PEEK(&lexer) == ')') {
              LEXER_NEXT(&lexer);
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeRBracket2;
            } else {
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeDot;
            }
            token.text = LEXER_TEXT(&lexer);
            break;
          case '@':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeAt;
            token.text = LEXER_TEXT(&lexer);
            break;
          case '^':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypePointer;
            token.text = LEXER_TEXT(&lexer);
            break;
          case '[':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeLBracket;
            token.text = LEXER_TEXT(&lexer);
            break;
          case ']':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeRBracket;
            token.text = LEXER_TEXT(&lexer);
            break;
          case ')':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeRParen;
            token.text = LEXER_TEXT(&lexer);
            break;
          case '>':
            if (LEXER_PEEK(&lexer) == '=') {
              LEXER_NEXT(&lexer);
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeGe;
            } else {
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeGt;
            }
            token.text = LEXER_TEXT(&lexer);
            break;
          case '<':
            if (LEXER_PEEK(&lexer) == '=') {
              LEXER_NEXT(&lexer);
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeLe;
            } else if (LEXER_PEEK(&lexer) == '>') {
              LEXER_NEXT(&lexer);
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeNotEqual;
            } else {
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeLt;
            }
            token.text = LEXER_TEXT(&lexer);
            break;
          case '=':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeEqual;
            token.text = LEXER_TEXT(&lexer);
            break;
          case ':':
            if (LEXER_PEEK(&lexer) == '=') {
              LEXER_NEXT(&lexer);
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeAssign;
            } else {
              LEXER_NEXT(&lexer);
              token.type = kPasTokenTypeColon;
            }
            token.text = LEXER_TEXT(&lexer);
            break;
          case ';':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeSemi;
            token.text = LEXER_TEXT(&lexer);
            break;
          case ',':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeComma;
            token.text = LEXER_TEXT(&lexer);
            break;
          case '/':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeSlash;
            token.text = LEXER_TEXT(&lexer);
            break;
          case '*':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeStar;
            token.text = LEXER_TEXT(&lexer);
            break;
          case '+':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypePlus;
            token.text = LEXER_TEXT(&lexer);
            break;
          case '-':
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeMinus;
            token.text = LEXER_TEXT(&lexer);
            break;
          default:
            LEXER_NEXT(&lexer);
            break;
        }
      }
    }
    VEC_PUSH(&tokens, token);
  }
  return tokens;
}

void LexExponent(Lexer* lexer) {
  LEXER_NEXT(lexer);
  if (IsSign(LEXER_CUR(lexer))) {
    LEXER_NEXT(lexer);
  }
  while (IsDigit(LEXER_CUR(lexer))) {
    LEXER_NEXT(lexer);
  }
}

bool IsIdentifierStart(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool IsDigit(char c) {
  return c >= '0' && c <= '9';
}

bool IsIdentifierPart(char c) {
  return IsIdentifierStart(c) || IsDigit(c) || c == '_';
}

bool IsSign(char c) {
  return c == '+' || c == '-';
}
