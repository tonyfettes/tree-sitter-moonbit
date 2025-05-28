#include "tree_sitter/parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <wctype.h>

#if defined(__wasi__) || defined(__EMSCRIPTEN__)
#else
#endif

#ifdef DEBUG
#include <stdio.h>
#define trace(string) printf(string)
#define tracef(format, ...) printf(format, __VA_ARGS__)
#else
#define trace(string)
#define tracef(format, ...)
#endif

struct ScannerState {
  bool remove_semi;
  bool multiline_string;
};

#if __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(struct ScannerState) <=
                   TREE_SITTER_SERIALIZATION_BUFFER_SIZE,
               "Context too large");
#endif

enum TokenKind {
  TOKEN_WITH,
  TOKEN_WHILE,
  TOKEN_UNDERSCORE,
  TOKEN_UIDENT,
  TOKEN_TYPEALIAS,
  TOKEN_TYPE,
  TOKEN_TRY,
  TOKEN_TRUE,
  TOKEN_TRAITALIAS,
  TOKEN_TRAIT,
  TOKEN_THROW,
  TOKEN_THIN_ARROW,
  TOKEN_TEST,
  TOKEN_STRUCT,
  TOKEN_STRING,
  TOKEN_SEMI,
  TOKEN_RPAREN,
  TOKEN_RETURN,
  TOKEN_READONLY,
  TOKEN_RBRACKET,
  TOKEN_RBRACE,
  TOKEN_RANGE_INCLUSIVE,
  TOKEN_RANGE_EXCLUSIVE,
  TOKEN_RAISE,
  TOKEN_QUESTION,
  TOKEN_PUB,
  TOKEN_PRIV,
  TOKEN_POST_LABEL,
  TOKEN_PLUS,
  TOKEN_PIPE,
  TOKEN_PACKAGE_NAME,
  TOKEN_NEWLINE,
  TOKEN_MUTABLE,
  TOKEN_MULTILINE_STRING,
  TOKEN_MULTILINE_INTERP,
  TOKEN_MINUS,
  TOKEN_MATCH,
  TOKEN_LPAREN,
  TOKEN_LOOP,
  TOKEN_LIDENT,
  TOKEN_LET,
  TOKEN_LBRACKET,
  TOKEN_LBRACE,
  TOKEN_IS,
  TOKEN_INTERP,
  TOKEN_INT,
  TOKEN_INFIX4,
  TOKEN_INFIX3,
  TOKEN_INFIX2,
  TOKEN_INFIX1,
  TOKEN_IN,
  TOKEN_IMPORT,
  TOKEN_IMPL,
  TOKEN_IF,
  TOKEN_GUARD,
  TOKEN_FOR,
  TOKEN_FNALIAS,
  TOKEN_FN,
  TOKEN_FLOAT,
  TOKEN_FAT_ARROW,
  TOKEN_FALSE,
  TOKEN_EXTERN,
  TOKEN_EXCLAMATION,
  TOKEN_EQUAL,
  TOKEN_EOF,
  TOKEN_ENUM,
  TOKEN_ELSE,
  TOKEN_ELLIPSIS,
  TOKEN_DOT_UIDENT,
  TOKEN_DOT_LPAREN,
  TOKEN_DOT_LIDENT,
  TOKEN_DOT_INT,
  TOKEN_DOTDOT,
  TOKEN_DERIVE,
  TOKEN_CONTINUE,
  TOKEN_CONST,
  TOKEN_COMMENT,
  TOKEN_COMMA,
  TOKEN_COLONCOLON,
  TOKEN_COLON,
  TOKEN_CHAR,
  TOKEN_CATCH,
  TOKEN_CARET,
  TOKEN_BYTES,
  TOKEN_BYTE,
  TOKEN_BREAK,
  TOKEN_BARBAR,
  TOKEN_BAR,
  TOKEN_AUGMENTED_ASSIGNMENT,
  TOKEN_ATTRIBUTE,
  TOKEN_ASYNC,
  TOKEN_AS,
  TOKEN_AMPERAMPER,
  TOKEN_AMPER,
};

#ifdef DEBUG
static const char *const symbol_names[] = {
    [SCANNER_RESET] = "$._scanner_reset",
    [AUTOMATIC_SEMICOLON] = "$._automatic_semicolon",
    [SEMICOLON] = "\";\"",
    [MULTILINE_STRING_SEPARATOR] = "\"#|\"",
    [MULTILINE_INTERPOLATION_SEPARATOR] = "\"$|\"",
    [FLOAT_LITERAL] = "$.float_literal",
    [FOR_KEYWORD] = "\"for\"",
};
#endif

void tree_sitter_moonbit_external_scanner_reset(void *payload) {
  struct ScannerState *context = payload;
  context->remove_semi = false;
  context->multiline_string = false;
}

void *tree_sitter_moonbit_external_scanner_create(void) {
  struct ScannerState *context = malloc(sizeof(struct ScannerState));
  tree_sitter_moonbit_external_scanner_reset(context);
  return context;
}

void tree_sitter_moonbit_external_scanner_destroy(void *payload) {
  free(payload);
}

unsigned tree_sitter_moonbit_external_scanner_serialize(void *payload,
                                                        char *buffer) {
  trace("serializing\n");
  *(struct ScannerState *)buffer = *(struct ScannerState *)payload;
  return sizeof(struct ScannerState);
}

void tree_sitter_moonbit_external_scanner_deserialize(void *payload,
                                                      const char *buffer,
                                                      unsigned length) {
  tree_sitter_moonbit_external_scanner_reset(payload);
  if (length != sizeof(struct ScannerState)) {
    return;
  }
  *(struct ScannerState *)payload = *(struct ScannerState *)buffer;
}

static void advance(TSLexer *lexer) { lexer->advance(lexer, false); }
static void skip(TSLexer *lexer) { lexer->advance(lexer, true); }

static void skip_spaces(TSLexer *lexer, const bool *valid_symbols) {
  if (!valid_symbols[AUTOMATIC_SEMICOLON]) {
    while (iswspace(lexer->lookahead) && !lexer->eof(lexer)) {
      skip(lexer);
    }
  } else {
    while (iswblank(lexer->lookahead) && !lexer->eof(lexer)) {
      skip(lexer);
    }
  }
}

static void advance_blanks(TSLexer *lexer) {
  while (iswblank(lexer->lookahead) && !lexer->eof(lexer)) {
    advance(lexer);
  }
}

enum FloatLiteralResult {
  FLOAT_LITERAL_OK,
  FLOAT_LITERAL_NOT,
  FLOAT_LITERAL_ERR
};

static enum FloatLiteralResult
scan_decimal_float_literal_fractional_part(TSLexer *lexer) {
  if (lexer->lookahead == '.') {
    return FLOAT_LITERAL_ERR;
  }
  while (iswdigit(lexer->lookahead) || lexer->lookahead == '_') {
    advance(lexer);
  }
  if (lexer->lookahead == 'e' || lexer->lookahead == 'E') {
    advance(lexer);
    if (lexer->lookahead == '+' || lexer->lookahead == '-') {
      advance(lexer);
    }
    while (iswdigit(lexer->lookahead) || lexer->lookahead == '_') {
      advance(lexer);
    }
  }
  return FLOAT_LITERAL_OK;
}

static enum FloatLiteralResult scan_decimal_float_literal(TSLexer *lexer) {
  while (iswdigit(lexer->lookahead) || lexer->lookahead == '_') {
    advance(lexer);
  }
  if (lexer->lookahead != '.') {
    return FLOAT_LITERAL_ERR;
  }
  advance(lexer);
  return scan_decimal_float_literal_fractional_part(lexer);
}

static enum FloatLiteralResult scan_float_literal(TSLexer *lexer,
                                                  const bool *valid_symbols) {
  skip_spaces(lexer, valid_symbols);
  tracef("scan_float_literal: lookahead: %c @ %d\n", lexer->lookahead,
         lexer->get_column(lexer));
  if (!iswdigit(lexer->lookahead)) {
    return FLOAT_LITERAL_NOT;
  }
  if (lexer->lookahead == '0') {
    advance(lexer);
    if (lexer->lookahead == '.') {
      advance(lexer);
      return scan_decimal_float_literal_fractional_part(lexer);
    }
    if (iswdigit(lexer->lookahead)) {
      advance(lexer);
      return scan_decimal_float_literal(lexer);
    }
    if (lexer->lookahead != 'x' && lexer->lookahead != 'X') {
      return FLOAT_LITERAL_ERR;
    }
    advance(lexer);
    while (iswxdigit(lexer->lookahead) || lexer->lookahead == '_') {
      advance(lexer);
    }
    if (lexer->lookahead != '.') {
      return FLOAT_LITERAL_ERR;
    }
    advance(lexer);
    if (lexer->lookahead == '.') {
      return FLOAT_LITERAL_ERR;
    }
    while (iswxdigit(lexer->lookahead) || lexer->lookahead == '_') {
      advance(lexer);
    }
    if (lexer->lookahead == 'p' || lexer->lookahead == 'P') {
      advance(lexer);
      if (lexer->lookahead == '+' || lexer->lookahead == '-') {
        advance(lexer);
      }
      while (iswxdigit(lexer->lookahead)) {
        advance(lexer);
      }
    }
    return FLOAT_LITERAL_OK;
  } else {
    advance(lexer);
    return scan_decimal_float_literal(lexer);
  }
}

enum AsiResult {
  ASI_REMOVE,
  ASI_INSERT,
  ASI_SKIP,
};

#ifdef DEBUG
static const char *asi_result_to_string[] = {
    [ASI_REMOVE] = "ASI_REMOVE",
    [ASI_INSERT] = "ASI_INSERT",
    [ASI_SKIP] = "ASI_SKIP",
};
#endif

static bool test_symbol_end(TSLexer *lexer) {
  if (iswalpha(lexer->lookahead) || lexer->lookahead == '_') {
    return false;
  } else {
    return true;
  }
}

static enum AsiResult asi_symbol_end(TSLexer *lexer) {
  if (iswalpha(lexer->lookahead) || lexer->lookahead == '_') {
    return ASI_INSERT;
  } else {
    return ASI_REMOVE;
  }
}

static bool test_symbol(TSLexer *lexer, const char *expected) {
  for (size_t i = 0; expected[i]; i++) {
    if (lexer->lookahead != expected[i]) {
      return false;
    }
    advance(lexer);
  }
  return test_symbol_end(lexer);
}

static enum AsiResult asi_symbol(TSLexer *lexer, const char *expected) {
  for (size_t i = 0; expected[i]; i++) {
    if (lexer->lookahead != expected[i]) {
      return ASI_INSERT;
    }
    advance(lexer);
  }
  return asi_symbol_end(lexer);
}

static enum AsiResult can_insert_semi(TSLexer *lexer,
                                      struct ScannerState *state) {
  if (state->multiline_string) {
    switch (lexer->lookahead) {
    case '#':
      advance(lexer);
      switch (lexer->lookahead) {
      case '|':
        return ASI_REMOVE;
      default:
        break;
      }
      break;
    case '$':
      advance(lexer);
      switch (lexer->lookahead) {
      case '|':
        return ASI_REMOVE;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
  switch (lexer->lookahead) {
  case '+':
    advance(lexer);
    switch (lexer->lookahead) {
    case '=':
      return ASI_REMOVE;
    default:
      return ASI_INSERT;
    }
  case '-':
    advance(lexer);
    switch (lexer->lookahead) {
    case '=':
      return ASI_REMOVE;
    default:
      return ASI_INSERT;
    }
  case '*':
    advance(lexer);
    switch (lexer->lookahead) {
    case '=':
      return ASI_REMOVE;
    default:
      return ASI_INSERT;
    }
  case '/':
    advance(lexer);
    switch (lexer->lookahead) {
    case '/':
      return ASI_SKIP;
    case '=':
      return ASI_REMOVE;
    default:
      return ASI_INSERT;
    }
  case '|':
    advance(lexer);
    switch (lexer->lookahead) {
    case '>': // PIPE
      return ASI_REMOVE;
    case '|': // BARBAR
      return ASI_INSERT;
    default: // BAR
      return ASI_REMOVE;
    }
  case '%':
    advance(lexer);
    switch (lexer->lookahead) {
    case '=':
      return ASI_REMOVE;
    default:
      return ASI_INSERT;
    }
  case '.':
    advance(lexer);
    if (lexer->lookahead == '.') {
      advance(lexer);
      if (lexer->lookahead == '.') {
        advance(lexer);
        return ASI_INSERT;
      } else {
        return ASI_REMOVE;
      }
    } else {
      return ASI_REMOVE;
    }
  case ':':
  case ',':
  case ')':
  case ']':
  case ';':
  case '=':
  case '?':
  case '!':
    return ASI_REMOVE;
  case 'a':
    advance(lexer);
    switch (lexer->lookahead) {
    case 's':
      advance(lexer);
      return asi_symbol_end(lexer);
    default:
      return ASI_INSERT;
    }
  case 'i':
    advance(lexer);
    switch (lexer->lookahead) {
    case 's':
    case 'n':
      advance(lexer);
      return asi_symbol_end(lexer);
    default:
      return ASI_INSERT;
    }
  case 'e':
    advance(lexer);
    return asi_symbol(lexer, "lse");
  case 'c':
    advance(lexer);
    return asi_symbol(lexer, "atch");
  case 'd':
    advance(lexer);
    return asi_symbol(lexer, "erive");
  case 'w':
    advance(lexer);
    return asi_symbol(lexer, "ith");
  default:
    return ASI_INSERT;
  }
}

#ifdef DEBUG
static bool trace_valid_symbols(const bool *valid_symbols) {
  for (int i = SCANNER_RESET; i < ERROR_SENTINEL; i++) {
    printf("valid_symbols[%s]: %s\n", symbol_names[i],
           valid_symbols[i] ? "true" : "false");
  }
  return false;
}
#define trace_valid_symbols(...) trace_valid_symbols(valid_symbols)
#else
#define trace_valid_symbols(...)
#endif

static inline bool scan_string(TSLexer *lexer, const char *expected) {
  for (size_t i = 0; expected[i]; i++) {
    if (lexer->lookahead != expected[i]) {
      return false;
    }
    advance(lexer);
  }
  return true;
}

static inline bool scan_char(TSLexer *lexer, char expected) {
  if (lexer->lookahead != expected) {
    return false;
  }
  advance(lexer);
  return true;
}

static inline bool scan_word_end(TSLexer *lexer) {
  if (iswalnum(lexer->lookahead) || lexer->lookahead == '_') {
    return false;
  } else {
    return true;
  }
}

static inline bool scan_symbol(TSLexer *lexer, const char *expected) {
  return scan_string(lexer, expected) && scan_word_end(lexer);
}

bool tree_sitter_moonbit_external_scanner_scan(void *payload, TSLexer *lexer,
                                               const bool *valid_symbols) {
  switch (lexer->lookahead) {
  case 'a':
    advance(lexer);
    if (!scan_string(lexer, "s")) {
      return false;
    }
    switch (lexer->lookahead) {
    case 'y':
      advance(lexer);
      if (!scan_symbol(lexer, "nc")) {
        return false;
      }
      lexer->result_symbol = TOKEN_ASYNC;
      lexer->mark_end(lexer);
      return true;
    default:
      if (!scan_word_end(lexer)) {
        return false;
      }
      lexer->result_symbol = TOKEN_AS;
      lexer->mark_end(lexer);
      return true;
    }
  case 'c':
    advance(lexer);
    switch (lexer->lookahead) {
    case 'a':
      advance(lexer);
      if (!scan_symbol(lexer, "tch")) {
        return false;
      }
      lexer->result_symbol = TOKEN_CATCH;
      lexer->mark_end(lexer);
      return true;
    case 'o':
      advance(lexer);
      scan_string(lexer, "n");
      switch (lexer->lookahead) {
      case 't':
        advance(lexer);
        if (!scan_symbol(lexer, "inue")) {
          return false;
        }
        lexer->result_symbol = TOKEN_CONTINUE;
        lexer->mark_end(lexer);
        return true;
      case 's':
        advance(lexer);
        if (!scan_symbol(lexer, "t")) {
          return false;
        }
        lexer->result_symbol = TOKEN_CONST;
        lexer->mark_end(lexer);
        return true;
      }
    }
  case 'd':
    advance(lexer);
    if (!scan_symbol(lexer, "erive")) {
      return false;
    }
    lexer->result_symbol = TOKEN_DERIVE;
    lexer->mark_end(lexer);
    return true;
  case 'e':
    advance(lexer);
    switch (lexer->lookahead) {
    case 'l':
      advance(lexer);
      if (!scan_symbol(lexer, "se")) {
        return false;
      }
      lexer->result_symbol = TOKEN_ELSE;
      lexer->mark_end(lexer);
      return true;
    case 'x':
      advance(lexer);
      if (!scan_string(lexer, "tern")) {
        return false;
      }
      lexer->result_symbol = TOKEN_EXTERN;
      lexer->mark_end(lexer);
      return true;
    case 'n':
      advance(lexer);
      if (!scan_symbol(lexer, "um")) {
        return false;
      }
      lexer->result_symbol = TOKEN_ENUM;
      lexer->mark_end(lexer);
      return true;
    default:
      return false;
    }
  case 'f':
    advance(lexer);
    switch (lexer->lookahead) {
    case 'a':
      advance(lexer);
      if (!scan_string(lexer, "lse")) {
        return false;
      }
      lexer->result_symbol = TOKEN_FALSE;
      lexer->mark_end(lexer);
      return true;
    case 'n':
      advance(lexer);
      switch (lexer->lookahead) {
      case 'a':
        advance(lexer);
        if (!scan_symbol(lexer, "lias")) {
          return false;
        }
        lexer->result_symbol = TOKEN_FNALIAS;
        lexer->mark_end(lexer);
        return true;
      default:
        if (!scan_word_end(lexer)) {
          return false;
        }
        lexer->result_symbol = TOKEN_FN;
        lexer->mark_end(lexer);
        return true;
      }
    case 'o':
      advance(lexer);
      if (scan_symbol(lexer, "r")) {
        lexer->result_symbol = TOKEN_FOR;
        lexer->mark_end(lexer);
        return true;
      } else {
        return false;
      }
    default:
      return false;
    }
  case 'w':
    advance(lexer);
    switch (lexer->lookahead) {
    case 'i':
      advance(lexer);
      if (scan_symbol(lexer, "th")) {
        lexer->result_symbol = TOKEN_WITH;
        lexer->mark_end(lexer);
        return true;
      } else {
        return false;
      }
    case 'h':
      advance(lexer);
      if (scan_symbol(lexer, "ile")) {
        lexer->result_symbol = TOKEN_WHILE;
        lexer->mark_end(lexer);
        return true;
      } else {
        return false;
      }
    default:
      return false;
    }
    break;
  }
}
