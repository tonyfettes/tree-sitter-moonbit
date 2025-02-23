#include "tree_sitter/parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <wctype.h>
#if defined(__wasi__) || defined(__EMSCRIPTEN__)
#define STANDALONE
#else
#define STANDALONE
#endif
#ifndef STANDALONE
#include <stdio.h>
#endif

struct ScannerState {
  bool remove_semi;
  bool multiline_string;
};

_Static_assert(
  sizeof(struct ScannerState) <= TREE_SITTER_SERIALIZATION_BUFFER_SIZE,
  "Context too large"
);

enum TokenType {
  AUTOMATIC_NEWLINE,
  AUTOMATIC_SEMICOLON,
  MULTILINE_STRING_SEPARATOR,
  FLOAT_LITERAL,
  __END__,
};

static const char *const symbol_names[] = {
  [AUTOMATIC_NEWLINE] = "",
  [AUTOMATIC_SEMICOLON] = ";",
  [MULTILINE_STRING_SEPARATOR] = "#|",
  [FLOAT_LITERAL] = "float"
};

void tree_sitter_moonbit_external_scanner_reset(void *payload) {
  struct ScannerState *context = payload;
  context->remove_semi = false;
  context->multiline_string = false;
}

void *tree_sitter_moonbit_external_scanner_create() {
  struct ScannerState *context = malloc(sizeof(struct ScannerState));
  tree_sitter_moonbit_external_scanner_reset(context);
  return context;
}

void tree_sitter_moonbit_external_scanner_destroy(void *payload) {
  free(payload);
}

unsigned
tree_sitter_moonbit_external_scanner_serialize(void *payload, char *buffer) {
#ifndef STANDALONE
  printf("serializing\n");
#endif
  *(struct ScannerState *)buffer = *(struct ScannerState *)payload;
  return sizeof(struct ScannerState);
}
void tree_sitter_moonbit_external_scanner_deserialize(
  void *payload,
  const char *buffer,
  unsigned length
) {
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
    while (iswspace(lexer->lookahead)) {
      skip(lexer);
    }
  } else {
    while (iswblank(lexer->lookahead)) {
      skip(lexer);
    }
  }
}

static bool scan_decimal_float_literal_fractional_part(
  TSLexer *lexer,
  const bool *valid_symbols
) {
  if (lexer->lookahead == '.') {
    return false;
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
  return true;
}

static bool scan_decimal_float_literal(TSLexer *lexer, const bool *valid_symbols) {
  while (iswdigit(lexer->lookahead) || lexer->lookahead == '_') {
    advance(lexer);
  }
  if (lexer->lookahead != '.') {
    return false;
  }
  advance(lexer);
  return scan_decimal_float_literal_fractional_part(lexer, valid_symbols);
}

static bool scan_float_literal(TSLexer *lexer, const bool *valid_symbols) {
  skip_spaces(lexer, valid_symbols);
#ifndef STANDALONE
  printf("lookahead: %c @ %d\n", lexer->lookahead, lexer->get_column(lexer));
#endif
  if (!iswdigit(lexer->lookahead)) {
    return false;
  }
  if (lexer->lookahead == '0') {
    advance(lexer);
    if (lexer->lookahead == '.') {
      advance(lexer);
      return scan_decimal_float_literal_fractional_part(lexer, valid_symbols);
    }
    if (iswdigit(lexer->lookahead)) {
      advance(lexer);
      return scan_decimal_float_literal(lexer, valid_symbols);
    }
    if (lexer->lookahead != 'x' && lexer->lookahead != 'X') {
      return false;
    }
    advance(lexer);
    while (iswxdigit(lexer->lookahead) || lexer->lookahead == '_') {
      advance(lexer);
    }
    if (lexer->lookahead != '.') {
      return false;
    }
    advance(lexer);
    if (lexer->lookahead == '.') {
      return false;
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
    return true;
  } else {
    advance(lexer);
    return scan_decimal_float_literal(lexer, valid_symbols);
  }
}

static bool test_symbol(TSLexer *lexer, const char *expected) {
  for (size_t i = 0; expected[i]; i++) {
    if (lexer->lookahead != expected[i]) {
      return false;
    }
    advance(lexer);
  }
  return !iswalpha(lexer->lookahead);
}

static bool can_insert_semi(TSLexer *lexer, struct ScannerState *state) {
  if (state->multiline_string) {
    switch (lexer->lookahead) {
    case '#':
      advance(lexer);
      switch (lexer->lookahead) {
      case '|':
        return false;
      default:
        break;
      }
    default:
      break;
    }
  }
  switch (lexer->lookahead) {
  case '+':
    advance(lexer);
    switch (lexer->lookahead) {
    case '=':
      return false;
    default:
      return true;
    }
  case '-':
    advance(lexer);
    switch (lexer->lookahead) {
    case '=':
      return false;
    default:
      return true;
    }
  case '*':
    advance(lexer);
    switch (lexer->lookahead) {
    case '=':
      return false;
    default:
      return true;
    }
  case '/':
    advance(lexer);
    switch (lexer->lookahead) {
    case '/':
      return false;
    case '=':
      return false;
    default:
      return true;
    }
  case '|':
    advance(lexer);
    switch (lexer->lookahead) {
    case '>': // PIPE
      return false;
    case '|': // BARBAR
      return true;
    default: // BAR
      return false;
    }
  case '.':
  case ':':
  case ',':
  case ')':
  case ']':
  case ';':
  case '=':
  case '?':
  case '!':
    return false;
  case 'a':
    advance(lexer);
    switch (lexer->lookahead) {
    case 's':
      advance(lexer);
      return iswalpha(lexer->lookahead) || lexer->lookahead == '_';
    default:
      return true;
    }
  case 'i':
    advance(lexer);
    switch (lexer->lookahead) {
    case 's':
    case 'n':
      advance(lexer);
      return iswalpha(lexer->lookahead) || lexer->lookahead == '_';
    default:
      return true;
    }
  case 'e':
    advance(lexer);
    return !test_symbol(lexer, "lse");
  case 'c':
    advance(lexer);
    return !test_symbol(lexer, "atch");
  case 'd':
    advance(lexer);
    return !test_symbol(lexer, "erive");
  case 'w':
    advance(lexer);
    return !test_symbol(lexer, "ith");
  default:
    return true;
  }
}

static bool skip_line(TSLexer *lexer) {
  while (lexer->lookahead != '\n' && lexer->lookahead != 0 && !lexer->eof(lexer)
  ) {
    skip(lexer);
  }
  return true;
}

static bool scan_integer_suffix(TSLexer *lexer) {
  switch (lexer->lookahead) {
  case 'U':
    advance(lexer);
    switch (lexer->lookahead) {
    case 'L':
      advance(lexer);
      return true;
    default:
      return false;
    }
  case 'L':
  case 'N':
    advance(lexer);
    return true;
  default:
    return false;
  }
}

static bool scan_integer_literal(TSLexer *lexer, const bool *valid_symbols) {
  skip_spaces(lexer, valid_symbols);
  if (lexer->lookahead != '0') {
    return false;
  }
  advance(lexer);
  switch (lexer->lookahead) {
  case 'x':
  case 'X':
    advance(lexer);
    while (iswxdigit(lexer->lookahead) || lexer->lookahead == '_') {
      advance(lexer);
    }
    return scan_integer_suffix(lexer);
  case 'o':
  case 'O':
    advance(lexer);
    while (lexer->lookahead >= '0' && lexer->lookahead <= '7' ||
           lexer->lookahead == '_') {
      advance(lexer);
    }
    return scan_integer_suffix(lexer);
  default:
    if (!iswdigit(lexer->lookahead)) {
      return false;
    }
    advance(lexer);
    while (iswdigit(lexer->lookahead) || lexer->lookahead == '_') {
      advance(lexer);
    }
    return scan_integer_suffix(lexer);
  }
}

bool tree_sitter_moonbit_external_scanner_scan(
  void *payload,
  TSLexer *lexer,
  const bool *valid_symbols
) {
  struct ScannerState *context = (struct ScannerState *)payload;

#ifndef STANDALONE
  printf("==========\n");
  printf("lookahead: %c @ %d\n", lexer->lookahead, lexer->get_column(lexer));
  for (int i = AUTOMATIC_NEWLINE; i < __END__; i++) {
    printf("valid_symbols[%s]: %d\n", symbol_names[i], valid_symbols[i]);
  }
#endif

  if (lexer->eof(lexer)) {
    return false;
  }

  if (valid_symbols[FLOAT_LITERAL]) {
    if (scan_float_literal(lexer, valid_symbols)) {
      lexer->mark_end(lexer);
      lexer->result_symbol = FLOAT_LITERAL;
      return true;
    }
  }

  if (valid_symbols[MULTILINE_STRING_SEPARATOR]) {
    if (!valid_symbols[AUTOMATIC_SEMICOLON]) {
      while (iswspace(lexer->lookahead)) {
        skip(lexer);
      }
    } else {
      while (iswblank(lexer->lookahead)) {
        skip(lexer);
      }
    }
#ifndef STANDALONE
    printf("lookahead: %c @ %d\n", lexer->lookahead, lexer->get_column(lexer));
#endif
    if (lexer->lookahead == '#') {
      advance(lexer);
      if (lexer->lookahead == '|') {
        advance(lexer);
        lexer->result_symbol = MULTILINE_STRING_SEPARATOR;
        lexer->mark_end(lexer);
        context->multiline_string = true;
#ifndef STANDALONE
        printf("setting multiline_string to true\n");
#endif
        return true;
      }
    }
  }

  if (valid_symbols[AUTOMATIC_SEMICOLON]) {
    while (iswblank(lexer->lookahead)) {
      skip(lexer);
    }
#ifndef STANDALONE
    printf("lookahead: %c @ %d\n", lexer->lookahead, lexer->get_column(lexer));
#endif
    if (lexer->lookahead != '\n') {
      return false;
    }
    advance(lexer);
    lexer->result_symbol = AUTOMATIC_SEMICOLON;
    lexer->mark_end(lexer);
#ifndef STANDALONE
    printf("lookahead: %c @ %d\n", lexer->lookahead, lexer->get_column(lexer));
#endif
    while (iswspace(lexer->lookahead)) {
      advance(lexer);
    }
#ifndef STANDALONE
    printf("lookahead: %c @ %d\n", lexer->lookahead, lexer->get_column(lexer));
#endif
    bool insert_semi = can_insert_semi(lexer, context);
#ifndef STANDALONE
    printf("insert_semi: %s\n", insert_semi ? "true" : "false");
    printf("remove_semi: %s\n", context->remove_semi ? "true" : "false");
    printf("multiline  : %s\n", context->multiline_string ? "true" : "false");
#endif
    if (context->remove_semi) {
      insert_semi = false;
    }
    if (context->remove_semi || context->multiline_string) {
      tree_sitter_moonbit_external_scanner_reset(context);
      if (insert_semi == false) {
        lexer->result_symbol = AUTOMATIC_NEWLINE;
        return true;
      }
    }
    return insert_semi;
  }

  return false;
}
