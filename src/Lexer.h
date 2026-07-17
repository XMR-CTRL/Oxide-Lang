#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class Tok {
  end,

  ident,
  int_lit,
  float_lit,
  str_lit,
  char_lit,

  kw_let,
  kw_mut,
  kw_fn,
  kw_return,
  kw_if,
  kw_else,
  kw_while,
  kw_for,
  kw_break,
  kw_continue,
  kw_in,
  kw_true,
  kw_false,
  kw_as,
  kw_const,
  kw_enum,
  kw_match,
  kw_null,
  kw_extern,
  kw_typedef,
  kw_impl,
  kw_self,
  kw_private,
  kw_sizeof,
  kw_asm,

  kw_i64,
  kw_f64,
  kw_bool,
  kw_void,
  kw_print,
  kw_struct,
  kw_str,
  kw_vec,
  kw_map,
  kw_set,
  kw_hmap,
  kw_hset,
  kw_char,
  kw_i8, kw_i16, kw_i32,
  kw_u8, kw_u16, kw_u32, kw_u64, kw_usize,
  kw_f32,

  plus, minus, star, slash, percent,
  eq, eqeq, bang, bangeq,
  lt, gt, lteq, gteq,
  ampamp, barbar,
  amp, bar, caret, tilde, shl, shr,
  pluseq, minuseq, stareq, lasheq, percenteq,
  ampeq, bareq, careteq, shleq, shreq,
  inc, dec,
  question,

  lparen, rparen, lbrace, rbrace,
  lbracket, rbracket, dot,
  comma, semicolon, colon, arrow,
  dotdot, coloncolon, fatarrow,

  err,
};

struct Token {
  Tok kind;
  std::string text;
  uint64_t u64 = 0;
  double f64 = 0;
  int line = 1;
  int col = 1;
};

struct LexError {
  std::string msg;
  int line;
  int col;
};

class Lexer {
public:
  explicit Lexer(std::string src);
  bool lex(std::vector<Token>& out, std::vector<LexError>& errs);

private:
  std::string src_;
  size_t i_ = 0;
  int line_ = 1;
  int col_ = 1;
  std::vector<LexError>* errs_ = nullptr;

  void error(const std::string& msg);
  Tok keyword(const std::string& s);
};
