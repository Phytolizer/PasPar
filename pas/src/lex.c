#include "pas/lex.h"

#include <stdbool.h>
#include <stdint.h>
#include <uthash.h>

#include "pas/string.h"

const char* const kPasTokenTypeNames[] = {
#define X(x) #x,
    PAS_TOKEN_TYPE_VARIANTS_
#undef X
};

typedef struct {
  String text;
  uint64_t line;
  uint64_t column;
  uint64_t position;
} Lexer;

typedef struct {
  String text;
  PasTokenType type;
  UT_hash_handle hh;
} LexerKeyword;

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

static String LexerText(Lexer* lexer, PasToken* token);
static void LexExponent(Lexer* lexer);
static void AddKeyword(LexerKeyword** keywords, String text, PasTokenType type);
static void CleanupKeywords(LexerKeyword* keywords);

static bool IsIdentifierStart(char c);
static bool IsDigit(char c);
static bool IsSign(char c);
static bool IsWhiteSpace(char c);
static bool IsIdentifierPart(char c);

PasTokens PasLex(String text) {
  Lexer lexer = {
      .text = text,
      .line = 1,
      .column = 1,
      .position = 0,
  };
  LexerKeyword* keywords = NULL;
  AddKeyword(&keywords, StringMakeC("and"), kPasTokenTypeAnd);
  AddKeyword(&keywords, StringMakeC("array"), kPasTokenTypeArray);
  AddKeyword(&keywords, StringMakeC("begin"), kPasTokenTypeBegin);
  AddKeyword(&keywords, StringMakeC("boolean"), kPasTokenTypeBoolean);
  AddKeyword(&keywords, StringMakeC("case"), kPasTokenTypeCase);
  AddKeyword(&keywords, StringMakeC("char"), kPasTokenTypeChar);
  AddKeyword(&keywords, StringMakeC("chr"), kPasTokenTypeChr);
  AddKeyword(&keywords, StringMakeC("const"), kPasTokenTypeConst);
  AddKeyword(&keywords, StringMakeC("div"), kPasTokenTypeDiv);
  AddKeyword(&keywords, StringMakeC("do"), kPasTokenTypeDo);
  AddKeyword(&keywords, StringMakeC("downto"), kPasTokenTypeDownto);
  AddKeyword(&keywords, StringMakeC("else"), kPasTokenTypeElse);
  AddKeyword(&keywords, StringMakeC("end"), kPasTokenTypeEnd);
  AddKeyword(&keywords, StringMakeC("file"), kPasTokenTypeFile);
  AddKeyword(&keywords, StringMakeC("for"), kPasTokenTypeFor);
  AddKeyword(&keywords, StringMakeC("function"), kPasTokenTypeFunction);
  AddKeyword(&keywords, StringMakeC("goto"), kPasTokenTypeGoto);
  AddKeyword(&keywords, StringMakeC("if"), kPasTokenTypeIf);
  AddKeyword(&keywords, StringMakeC("in"), kPasTokenTypeIn);
  AddKeyword(&keywords, StringMakeC("integer"), kPasTokenTypeInteger);
  AddKeyword(&keywords, StringMakeC("label"), kPasTokenTypeLabel);
  AddKeyword(&keywords, StringMakeC("mod"), kPasTokenTypeMod);
  AddKeyword(&keywords, StringMakeC("nil"), kPasTokenTypeNil);
  AddKeyword(&keywords, StringMakeC("not"), kPasTokenTypeNot);
  AddKeyword(&keywords, StringMakeC("of"), kPasTokenTypeOf);
  AddKeyword(&keywords, StringMakeC("or"), kPasTokenTypeOr);
  AddKeyword(&keywords, StringMakeC("packed"), kPasTokenTypePacked);
  AddKeyword(&keywords, StringMakeC("procedure"), kPasTokenTypeProcedure);
  AddKeyword(&keywords, StringMakeC("program"), kPasTokenTypeProgram);
  AddKeyword(&keywords, StringMakeC("real"), kPasTokenTypeReal);
  AddKeyword(&keywords, StringMakeC("record"), kPasTokenTypeRecord);
  AddKeyword(&keywords, StringMakeC("repeat"), kPasTokenTypeRepeat);
  AddKeyword(&keywords, StringMakeC("set"), kPasTokenTypeSet);
  AddKeyword(&keywords, StringMakeC("then"), kPasTokenTypeThen);
  AddKeyword(&keywords, StringMakeC("to"), kPasTokenTypeTo);
  AddKeyword(&keywords, StringMakeC("type"), kPasTokenTypeType);
  AddKeyword(&keywords, StringMakeC("until"), kPasTokenTypeUntil);
  AddKeyword(&keywords, StringMakeC("var"), kPasTokenTypeVar);
  AddKeyword(&keywords, StringMakeC("while"), kPasTokenTypeWhile);
  AddKeyword(&keywords, StringMakeC("with"), kPasTokenTypeWith);
  AddKeyword(&keywords, StringMakeC("unit"), kPasTokenTypeUnit);
  AddKeyword(&keywords, StringMakeC("interface"), kPasTokenTypeInterface);
  AddKeyword(&keywords, StringMakeC("uses"), kPasTokenTypeUses);
  AddKeyword(&keywords, StringMakeC("string"), kPasTokenTypeString);
  AddKeyword(&keywords, StringMakeC("implementation"),
             kPasTokenTypeImplementation);
  AddKeyword(&keywords, StringMakeC("true"), kPasTokenTypeTrue);
  AddKeyword(&keywords, StringMakeC("false"), kPasTokenTypeFalse);
  PasTokens tokens = {0};
  while (lexer.position < lexer.text.size) {
    PasToken token = {
        .line = lexer.line,
        .column = lexer.column,
        .position = lexer.position,
    };
    if (IsIdentifierStart(LEXER_CUR(&lexer))) {
      while (IsIdentifierPart(LEXER_CUR(&lexer))) {
        LEXER_NEXT(&lexer);
      }
      token.type = kPasTokenTypeIdent;
      token.text = LexerText(&lexer, &token);
      String lookup_text = StringDuplicate(&token.text);
      StringDowncase(&lookup_text);
      VEC_PUSH(&lookup_text, '\0');
      LexerKeyword* kw;
      HASH_FIND_STR(keywords, lookup_text.data, kw);
      VEC_FREE(&lookup_text);
      if (kw != NULL) {
        token.type = kw->type;
      }
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
      token.text = LexerText(&lexer, &token);
    } else if (IsWhiteSpace(LEXER_CUR(&lexer))) {
      while (IsWhiteSpace(LEXER_CUR(&lexer))) {
        LEXER_NEXT(&lexer);
      }
      token.type = kPasTokenTypeWs;
      token.text = LexerText(&lexer, &token);
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
          token.text = LexerText(&lexer, &token);
        } break;
        case '}':
          token.type = kPasTokenTypeRCurly;
          token.text = LexerText(&lexer, &token);
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
            token.type = kPasTokenTypeComment2;
          } else if (LEXER_PEEK(&lexer) == '.') {
            LEXER_NEXT(&lexer);
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeLBracket2;
          } else {
            LEXER_NEXT(&lexer);
            token.type = kPasTokenTypeLParen;
          }
          token.text = LexerText(&lexer, &token);
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
          token.text = LexerText(&lexer, &token);
          break;
        case '@':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeAt;
          token.text = LexerText(&lexer, &token);
          break;
        case '^':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypePointer;
          token.text = LexerText(&lexer, &token);
          break;
        case '[':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeLBracket;
          token.text = LexerText(&lexer, &token);
          break;
        case ']':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeRBracket;
          token.text = LexerText(&lexer, &token);
          break;
        case ')':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeRParen;
          token.text = LexerText(&lexer, &token);
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
          token.text = LexerText(&lexer, &token);
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
          token.text = LexerText(&lexer, &token);
          break;
        case '=':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeEqual;
          token.text = LexerText(&lexer, &token);
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
          token.text = LexerText(&lexer, &token);
          break;
        case ';':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeSemi;
          token.text = LexerText(&lexer, &token);
          break;
        case ',':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeComma;
          token.text = LexerText(&lexer, &token);
          break;
        case '/':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeSlash;
          token.text = LexerText(&lexer, &token);
          break;
        case '*':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeStar;
          token.text = LexerText(&lexer, &token);
          break;
        case '+':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypePlus;
          token.text = LexerText(&lexer, &token);
          break;
        case '-':
          LEXER_NEXT(&lexer);
          token.type = kPasTokenTypeMinus;
          token.text = LexerText(&lexer, &token);
          break;
        default:
          LEXER_NEXT(&lexer);
          break;
      }
    }
    VEC_PUSH(&tokens, token);
  }
  CleanupKeywords(keywords);
  return tokens;
}

String LexerText(Lexer* lexer, PasToken* token) {
  return StringMake(lexer->text.data + token->position,
                    lexer->text.data + lexer->position);
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

void AddKeyword(LexerKeyword** keywords, String text, PasTokenType type) {
  LexerKeyword* keyword = (LexerKeyword*)malloc(sizeof(LexerKeyword));
  keyword->text = text;
  keyword->type = type;
  HASH_ADD_KEYPTR(hh, *keywords, keyword->text.data, keyword->text.size,
                  keyword);
}

void CleanupKeywords(LexerKeyword* keywords) {
  LexerKeyword* kw;
  LexerKeyword* tmp;
  HASH_ITER(hh, keywords, kw, tmp) {
    HASH_DEL(keywords, kw);
    VEC_FREE(&kw->text);
    free(kw);
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

bool IsWhiteSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
