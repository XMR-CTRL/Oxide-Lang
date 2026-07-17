#include "Lexer.h"
#include <cctype>
#include <map>
#include <cstdio>

Lexer::Lexer(std::string src) : src_(std::move(src)) {}

void Lexer::error(const std::string& msg) {
  if (errs_) errs_->push_back({msg, line_, col_});
}

Tok Lexer::keyword(const std::string& s) {
  static const std::map<std::string, Tok> kws = {
    {"let", Tok::kw_let}, {"mut", Tok::kw_mut}, {"fn", Tok::kw_fn},
    {"return", Tok::kw_return}, {"if", Tok::kw_if}, {"else", Tok::kw_else},
    {"while", Tok::kw_while}, {"for", Tok::kw_for}, {"break", Tok::kw_break},
    {"continue", Tok::kw_continue}, {"in", Tok::kw_in},
    {"true", Tok::kw_true}, {"false", Tok::kw_false},
    {"as", Tok::kw_as}, {"const", Tok::kw_const},
    {"enum", Tok::kw_enum},
    {"match", Tok::kw_match}, {"null", Tok::kw_null},
    {"extern", Tok::kw_extern}, {"typedef", Tok::kw_typedef},
    {"impl", Tok::kw_impl}, {"self", Tok::kw_self}, {"private", Tok::kw_private},
    {"sizeof", Tok::kw_sizeof},
    {"asm", Tok::kw_asm},
    {"i64", Tok::kw_i64}, {"f64", Tok::kw_f64}, {"bool", Tok::kw_bool},
    {"void", Tok::kw_void}, {"print", Tok::kw_print}, {"struct", Tok::kw_struct},
    {"str", Tok::kw_str}, {"vec", Tok::kw_vec},
    {"map", Tok::kw_map}, {"set", Tok::kw_set},
    {"hmap", Tok::kw_hmap}, {"hset", Tok::kw_hset},
    {"char", Tok::kw_char},
    {"i8", Tok::kw_i8}, {"i16", Tok::kw_i16}, {"i32", Tok::kw_i32},
    {"u8", Tok::kw_u8}, {"u16", Tok::kw_u16}, {"u32", Tok::kw_u32},
    {"u64", Tok::kw_u64}, {"usize", Tok::kw_usize},
    {"f32", Tok::kw_f32},
  };
  auto it = kws.find(s);
  return it == kws.end() ? Tok::ident : it->second;
}

static bool isIdentStart(char c) {
  return std::isalpha((unsigned char)c) || c == '_';
}
static bool isIdentPart(char c) {
  return std::isalnum((unsigned char)c) || c == '_';
}

bool Lexer::lex(std::vector<Token>& out, std::vector<LexError>& errs) {
  errs_ = &errs;
  auto peekc = [&]() -> char { return (i_ + 1 < src_.size()) ? src_[i_ + 1] : 0; };
  auto peekc2 = [&]() -> char { return (i_ + 2 < src_.size()) ? src_[i_ + 2] : 0; };
  while (i_ < src_.size()) {
    char c = src_[i_];


    if (c == ' ' || c == '\t' || c == '\r') { i_++; col_++; continue; }
    if (c == '\n') { i_++; line_++; col_ = 1; continue; }


    if (c == '/' && i_ + 1 < src_.size() && src_[i_ + 1] == '/') {
      while (i_ < src_.size() && src_[i_] != '\n') { i_++; col_++; }
      continue;
    }

    if (c == '/' && i_ + 1 < src_.size() && src_[i_ + 1] == '*') {
      i_ += 2; col_ += 2;
      while (i_ + 1 < src_.size() &&
             !(src_[i_] == '*' && src_[i_ + 1] == '/')) {
        if (src_[i_] == '\n') { line_++; col_ = 1; }
        else col_++;
        i_++;
      }
      if (i_ + 1 >= src_.size()) {
        error("unterminated block comment");
        i_ = src_.size();
        continue;
      }
      i_ += 2; col_ += 2;
      continue;
    }

    Token t;
    t.line = line_;
    t.col = col_;


    if (isIdentStart(c)) {
      size_t start = i_;
      while (i_ < src_.size() && isIdentPart(src_[i_])) { i_++; col_++; }
      t.text = src_.substr(start, i_ - start);
      t.kind = keyword(t.text);
      out.push_back(t);
      continue;
    }


    if (std::isdigit((unsigned char)c)) {
      size_t start = i_;
      bool isFloat = false;


      int base = 10;
      if (c == '0' && i_ + 1 < src_.size() &&
          (src_[i_ + 1] == 'x' || src_[i_ + 1] == 'X')) {
        i_ += 2; col_ += 2;
        base = 16;
        while (i_ < src_.size() && std::isxdigit((unsigned char)src_[i_])) { i_++; col_++; }
        t.text = src_.substr(start, i_ - start);
        t.kind = Tok::int_lit;
        try { t.u64 = std::stoull(t.text, nullptr, 16); }
        catch (...) { error("invalid hex literal"); t.u64 = 0; }
        out.push_back(t);
        continue;
      }
      if (c == '0' && i_ + 1 < src_.size() &&
          (src_[i_ + 1] == 'b' || src_[i_ + 1] == 'B')) {
        i_ += 2; col_ += 2;
        base = 2;
        while (i_ < src_.size() && (src_[i_] == '0' || src_[i_] == '1')) { i_++; col_++; }
        t.text = src_.substr(start, i_ - start);
        t.kind = Tok::int_lit;
        try { t.u64 = std::stoull(t.text, nullptr, 2); }
        catch (...) { error("invalid binary literal"); t.u64 = 0; }
        out.push_back(t);
        continue;
      }
      while (i_ < src_.size() && std::isdigit((unsigned char)src_[i_])) { i_++; col_++; }
      if (i_ < src_.size() && src_[i_] == '.' &&
          i_ + 1 < src_.size() && std::isdigit((unsigned char)src_[i_ + 1])) {
        isFloat = true;
        i_++; col_++;
        while (i_ < src_.size() && std::isdigit((unsigned char)src_[i_])) { i_++; col_++; }
      }
      t.text = src_.substr(start, i_ - start);
      if (isFloat) {
        t.kind = Tok::float_lit;
        t.f64 = std::stod(t.text);
      } else {
        t.kind = Tok::int_lit;
        t.u64 = std::stoull(t.text);
      }
      out.push_back(t);
      continue;
    }


    if (c == '\'') {
      i_++; col_++;
      unsigned char ch;
      bool ok = false, closing = false;
      while (i_ < src_.size()) {
        char cur = src_[i_];
        if (cur == '\'') { i_++; col_++; closing = true; break; }
        if (cur == '\\') {
          i_++; col_++;
          if (i_ >= src_.size()) { error("unterminated char"); break; }
          char esc = src_[i_++];
          col_++;
          switch (esc) {
            case 'n': ch = '\n'; break;
            case 't': ch = '\t'; break;
            case 'r': ch = '\r'; break;
            case '0': ch = '\0'; break;
            case 'a': ch = '\a'; break;
            case 'b': ch = '\b'; break;
            case 'f': ch = '\f'; break;
            case 'v': ch = '\v'; break;
            case '\'': ch = '\''; break;
            case '\\': ch = '\\'; break;
            case '"': ch = '"'; break;
            case 'x': {

              auto hexval = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
              };
              int hi = (i_ < src_.size()) ? hexval(src_[i_]) : -1;
              if (hi < 0) { ch = (unsigned char)'x'; }
              else {
                i_++; col_++;
                int lo = (i_ < src_.size()) ? hexval(src_[i_]) : -1;
                int val = hi;
                if (lo >= 0) { val = (hi << 4) | lo; i_++; col_++; }
                ch = (unsigned char)(val & 0xFF);
              }
              break;
            }
            default: ch = (unsigned char)esc; break;
          }
          ok = true;
          if (i_ < src_.size() && src_[i_] == '\'') { i_++; col_++; closing = true; break; }
          else { error("char literal must be a single character"); break; }
        } else {
          if (cur == '\n') { line_++; col_ = 1; }
          else col_++;
          ch = (unsigned char)cur;
          ok = true;
          i_++;

          if (i_ < src_.size() && src_[i_] == '\'') { i_++; col_++; closing = true; break; }
          else { error("char literal must be a single character"); break; }
        }
      }
      if (!closing) error("unterminated char");
      t.kind = Tok::char_lit;
      t.u64 = ok ? (uint64_t)ch : 0;
      out.push_back(t);
      continue;
    }


    if (c == '"') {
      i_++; col_++;
      std::string s;
      bool closing = false;
      while (i_ < src_.size()) {
        char ch = src_[i_];
        if (ch == '"') { i_++; col_++; closing = true; break; }
        if (ch == '\\') {
          i_++; col_++;
          if (i_ >= src_.size()) {
            error("unterminated string");
            break;
          }
          char esc = src_[i_++];
          col_++;
          switch (esc) {
            case 'n': s += '\n'; break;
            case 't': s += '\t'; break;
            case 'r': s += '\r'; break;
            case '0': s += '\0'; break;
            case 'a': s += '\a'; break;
            case 'b': s += '\b'; break;
            case 'f': s += '\f'; break;
            case 'v': s += '\v'; break;
            case '"': s += '"'; break;
            case '\\': s += '\\'; break;
            case 'x': {


              auto hexval = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
              };
              int hi = (i_ < src_.size()) ? hexval(src_[i_]) : -1;
              if (hi >= 0) {
                i_++; col_++;
                int lo = (i_ < src_.size()) ? hexval(src_[i_]) : -1;
                int val = hi;
                if (lo >= 0) { val = (hi << 4) | lo; i_++; col_++; }
                s += static_cast<char>(val & 0xFF);
              } else {
                s += 'x';
              }
              break;
            }
            default: s += esc; break;
          }
        } else {
          if (ch == '\n') { line_++; col_ = 1; }
          else col_++;
          s += ch;
          i_++;
        }
      }
      if (!closing) { error("unterminated string"); }
      t.kind = Tok::str_lit;
      t.text = std::move(s);
      out.push_back(t);
      continue;
    }


    auto two = [&](char first, char second) -> bool {
      if (i_ + 1 < src_.size() && src_[i_] == first && src_[i_ + 1] == second) {
        i_ += 2; col_ += 2; return true;
      }
      return false;
    };
    auto one = [&]() { i_++; col_++; };

    switch (c) {
      case '+':
        if (two('+', '+')) t.kind = Tok::inc;
        else if (two('+', '=')) t.kind = Tok::pluseq;
        else { t.kind = Tok::plus; one(); }
        break;
      case '-':
        if (two('-', '-')) t.kind = Tok::dec;
        else if (two('-', '>')) t.kind = Tok::arrow;
        else if (two('-', '=')) t.kind = Tok::minuseq;
        else { t.kind = Tok::minus; one(); }
        break;
      case '*': t.kind = two('*', '=') ? Tok::stareq : Tok::star; if (t.kind == Tok::star) one(); break;
      case '/': t.kind = two('/', '=') ? Tok::lasheq : Tok::slash; if (t.kind == Tok::slash) one(); break;
      case '%': t.kind = two('%', '=') ? Tok::percenteq : Tok::percent; if (t.kind == Tok::percent) one(); break;
      case '=': t.kind = two('=', '=') ? Tok::eqeq : (two('=', '>') ? Tok::fatarrow : Tok::eq); if (t.kind == Tok::eq) one(); break;
      case '!': t.kind = two('!', '=') ? Tok::bangeq : Tok::bang; if (t.kind == Tok::bang) one(); break;
      case '<': {
        char n = peekc();
        if (n == '<') { char m = peekc2(); t.kind = (m == '=') ? Tok::shleq : Tok::shl; i_ += (m=='=')?3:2; col_ += (m=='=')?3:2; }
        else if (n == '=') { t.kind = Tok::lteq; i_+=2; col_+=2; }
        else { t.kind = Tok::lt; one(); }
        break;
      }
      case '>': {
        char n = peekc();
        if (n == '>') { char m = peekc2(); t.kind = (m == '=') ? Tok::shreq : Tok::shr; i_ += (m=='=')?3:2; col_ += (m=='=')?3:2; }
        else if (n == '=') { t.kind = Tok::gteq; i_+=2; col_+=2; }
        else { t.kind = Tok::gt; one(); }
        break;
      }
      case '&': t.kind = two('&', '&') ? Tok::ampamp : (two('&', '=') ? Tok::ampeq : Tok::amp); if (t.kind == Tok::amp) one(); break;
      case '|': t.kind = two('|', '|') ? Tok::barbar : (two('|', '=') ? Tok::bareq : Tok::bar); if (t.kind == Tok::bar) one(); break;
      case '^': t.kind = two('^', '=') ? Tok::careteq : Tok::caret; if (t.kind == Tok::caret) one(); break;
      case '~': t.kind = Tok::tilde; one(); break;
      case '?': t.kind = Tok::question; one(); break;
      case '(': t.kind = Tok::lparen; one(); break;
      case ')': t.kind = Tok::rparen; one(); break;
      case '{': t.kind = Tok::lbrace; one(); break;
      case '}': t.kind = Tok::rbrace; one(); break;
      case '[': t.kind = Tok::lbracket; one(); break;
      case ']': t.kind = Tok::rbracket; one(); break;
      case ',': t.kind = Tok::comma; one(); break;
      case ';': t.kind = Tok::semicolon; one(); break;
      case ':': t.kind = two(':', ':') ? Tok::coloncolon : Tok::colon; if (t.kind == Tok::colon) one(); break;
      case '.': t.kind = two('.', '.') ? Tok::dotdot : Tok::dot; if (t.kind == Tok::dot) one(); break;
      default:
        error(std::string("unexpected character '") + c + "'");
        i_++; col_++;
        t.kind = Tok::err;
        t.text = std::string(1, c);
        break;
    }
    out.push_back(t);
  }

  Token eof;
  eof.kind = Tok::end;
  eof.line = line_;
  eof.col = col_;
  out.push_back(eof);
  return errs.empty();
}
