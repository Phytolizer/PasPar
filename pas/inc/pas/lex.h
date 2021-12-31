#pragma once

#include <stdint.h>
#include <vec/vec.h>

#include "pas/string.h"

#define PAS_TOKEN_TYPE_VARIANTS_ \
  X(Zero)                        \
                                 \
  X(And)                         \
  X(Array)                       \
  X(Begin)                       \
  X(Boolean)                     \
  X(Case)                        \
  X(Char)                        \
  X(Chr)                         \
  X(Const)                       \
  X(Div)                         \
  X(Do)                          \
  X(Downto)                      \
  X(Else)                        \
  X(End)                         \
  X(File)                        \
  X(For)                         \
  X(Function)                    \
  X(Goto)                        \
  X(If)                          \
  X(In)                          \
  X(Integer)                     \
  X(Label)                       \
  X(Mod)                         \
  X(Nil)                         \
  X(Not)                         \
  X(Of)                          \
  X(Or)                          \
  X(Packed)                      \
  X(Procedure)                   \
  X(Program)                     \
  X(Real)                        \
  X(Record)                      \
  X(Repeat)                      \
  X(Set)                         \
  X(Then)                        \
  X(To)                          \
  X(Type)                        \
  X(Until)                       \
  X(Var)                         \
  X(While)                       \
  X(With)                        \
                                 \
  X(Plus)                        \
  X(Minus)                       \
  X(Star)                        \
  X(Slash)                       \
  X(Assign)                      \
  X(Comma)                       \
  X(Semi)                        \
  X(Colon)                       \
  X(Equal)                       \
  X(NotEqual)                    \
  X(Lt)                          \
  X(Le)                          \
  X(Ge)                          \
  X(Gt)                          \
  X(LParen)                      \
  X(RParen)                      \
  X(LBracket)                    \
  X(LBracket2)                   \
  X(RBracket)                    \
  X(RBracket2)                   \
  X(Pointer)                     \
  X(At)                          \
  X(Dot)                         \
  X(DotDot)                      \
  X(LCurly)                      \
  X(RCurly)                      \
  X(Unit)                        \
  X(Interface)                   \
  X(Uses)                        \
  X(String)                      \
  X(Implementation)              \
  X(True)                        \
  X(False)                       \
  X(Ws)                          \
  X(Comment1)                    \
  X(Comment2)                    \
  X(Ident)                       \
  X(StringLiteral)               \
  X(NumInt)                      \
  X(NumReal)

typedef enum {
#define X(x) kPasTokenType##x,
  PAS_TOKEN_TYPE_VARIANTS_
#undef X
} PasTokenType;

extern const char* const kPasTokenTypeNames[];

typedef struct {
  uint64_t line;
  uint64_t column;
  uint64_t position;
  PasTokenType type;
  String text;
} PasToken;

typedef VEC_TYPE(PasToken) PasTokens;

PasTokens PasLex(String text);
