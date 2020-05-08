#ifndef SRC_PARSER_CONFIG_LEXER_HPP
#define SRC_PARSER_CONFIG_LEXER_HPP

/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */

#include <vector>
#include <string>
#include <iostream>
namespace src::parser::config_lexer {
  class StackObj {
  public:
    StackObj(int symbol, void* obj, size_t line)
        : symbol_(symbol), obj_(obj), line_(line) {}
    StackObj(StackObj&& other)
        : symbol_(other.symbol_),
          obj_(other.obj_),
          line_(other.line_),
          released_(other.released_) {
      other.obj_ = nullptr;
    }
    StackObj(const StackObj& other) = delete;
    StackObj& operator=(const StackObj& other) = delete;
    StackObj& operator=(StackObj&& other) = delete;

    ~StackObj() {
      if (!obj_) {
        return;
      }

      switch (symbol_) {
        case -14:
          delete static_cast<std::string*>(obj_);
          break;
        case -15:
          delete static_cast<std::string*>(obj_);
          break;
        case -16:
          delete static_cast<std::string*>(obj_);
          break;
        default:
          return;
      }
    }
    void* releaseObj() noexcept {
      released_ = true;
      return obj_;
    }
    int getSymbol() const noexcept { return symbol_; }
    size_t getLine() const noexcept { return line_; }
    void* getObj() const noexcept { return obj_; }
    void unrelease() noexcept { released_ = false; }

  private:
    const int symbol_;
    void* obj_;
    size_t line_;
    bool released_ = false;
  };

  std::vector<StackObj> tokenize(const std::string& input);
  std::vector<StackObj> tokenize(std::istream& input);
}  // namespace src::parser::config_lexer
#endif