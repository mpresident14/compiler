/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */
#include "test/parser/generated/expr_parser.hpp"

#include <vector>
#include <cstddef>
#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>
#include <memory>
#include <sstream>
#include <climits>
#include <unordered_map>
#include <string_view>
#include <streambuf>
#include <prez/print_stuff.hpp>
using namespace std;
namespace {
  constexpr int NONE = INT_MIN;
  constexpr int SKIP_TOKEN = INT_MIN + 1;

  constexpr int S = 0;

  bool isToken(int symbol) { return symbol < 0; }

  int tokenToFromIndex(int token) { return -token - 1; }
  struct Token {
    string name;
    string type;
    int precedence;
  };
  struct Concrete {
    int varType;
  };
  struct Variable {
    string name;
    string type;
  };
  struct GrammarData {
    vector<Token> tokens;
    vector<Concrete> concretes;
    vector<Variable> variables;
  };
  GrammarData GRAMMAR_DATA = { {
                                   { "INT", "int", -2147483648 },
                                   { "PLUS", "", 1 },
                                   { "STAR", "", 2 },
                               },
                               {
                                   { 0 },
                                   { 1 },
                                   { 1 },
                                   { 1 },
                               },
                               {
                                   { "S", "Start" },
                                   { "Expr", "Expr*" },
                               } };
  struct StackObj {
    void* obj;
    int symbol;
    size_t line;
  };
  struct SObj {
    SObj(Expr** r) : r_(r) {}
    ~SObj() { delete r_; }
    Expr** r_;
  };
  static size_t currentLine = 1;
  void deleteObjPtr(const StackObj& s) {
    switch (s.symbol) {
      case -1:
        delete (int*)s.obj;
        break;
      case 1:
        delete (Expr**)s.obj;
        break;
      default:
        return;
    }
  }
  void deleteObj(const StackObj& s) {
    switch (s.symbol) {
      case -1:
        delete (int*)s.obj;
        break;
      case 1:
        delete *(Expr**)s.obj;
        delete (Expr**)s.obj;
        break;
      default:
        return;
    }
  }
  void* constructObj(int concrete, StackObj* args) {
    switch (concrete) {
      case 1:
        return new Expr*(new EInt(*(int*)args[0].obj));
      case 2:
        return new Expr*(
            new EBinOp(*(Expr**)args[0].obj, PLUS, *(Expr**)args[2].obj));
      case 3:
        return new Expr*(
            new EBinOp(*(Expr**)args[0].obj, TIMES, *(Expr**)args[2].obj));
      case 0:
        return new SObj((Expr**)args[0].obj);
      default:
        throw invalid_argument("Can't construct. Out of options.");
    }
  }
  StackObj construct(
      int concrete,
      vector<StackObj>& stk,
      size_t reduceStart,
      int varType) {
    size_t line = reduceStart == stk.size() ? 0 : stk[reduceStart].line;
    return StackObj{ constructObj(concrete, &stk[reduceStart]), varType, line };
  }
  StackObj constructTokenObj(int token, const string_view& str) {
    switch (token) {
      case -1:
        return { new int(stoi(string(str))), token, currentLine };
        break;
      default:
        return { nullptr, token, currentLine };
    }
  }
  namespace lexer {
    struct Node {
      Node(int&& v) : v_(std::move(v)) {}

      Node* step(const char& t) const {
        auto iter = ts_.find(t);
        if (iter == ts_.end()) {
          return nullptr;
        }
        return iter->second;
      }
      int v_;
      std::unordered_map<char, Node*> ts_;
    };

    auto n0x93c080 = std::make_unique<Node>(-2147483648);
    auto n0x8c0c70 = std::make_unique<Node>(-2147483648);
    auto n0x8c33a0 = std::make_unique<Node>(-1);
    auto n0x8c3790 = std::make_unique<Node>(-2);
    auto n0x8c1770 = std::make_unique<Node>(-3);
    auto makeDFA() {
      n0x93c080->ts_ = {
        { '~', n0x8c0c70.get() }, { '}', n0x8c0c70.get() },
        { '|', n0x8c0c70.get() }, { '{', n0x8c0c70.get() },
        { 'z', n0x8c0c70.get() }, { 'y', n0x8c0c70.get() },
        { 'x', n0x8c0c70.get() }, { 'w', n0x8c0c70.get() },
        { 'v', n0x8c0c70.get() }, { 'u', n0x8c0c70.get() },
        { 't', n0x8c0c70.get() }, { 's', n0x8c0c70.get() },
        { 'r', n0x8c0c70.get() }, { 'q', n0x8c0c70.get() },
        { 'p', n0x8c0c70.get() }, { 'o', n0x8c0c70.get() },
        { 'n', n0x8c0c70.get() }, { 'm', n0x8c0c70.get() },
        { 'l', n0x8c0c70.get() }, { '\n', n0x8c0c70.get() },
        { 'k', n0x8c0c70.get() }, { '\t', n0x8c0c70.get() },
        { 'j', n0x8c0c70.get() }, { 'i', n0x8c0c70.get() },
        { 'h', n0x8c0c70.get() }, { 'g', n0x8c0c70.get() },
        { 'f', n0x8c0c70.get() }, { 'e', n0x8c0c70.get() },
        { 'd', n0x8c0c70.get() }, { 'c', n0x8c0c70.get() },
        { 'b', n0x8c0c70.get() }, { 'a', n0x8c0c70.get() },
        { '`', n0x8c0c70.get() }, { '_', n0x8c0c70.get() },
        { '^', n0x8c0c70.get() }, { ']', n0x8c0c70.get() },
        { '[', n0x8c0c70.get() }, { 'Z', n0x8c0c70.get() },
        { 'Y', n0x8c0c70.get() }, { 'X', n0x8c0c70.get() },
        { 'W', n0x8c0c70.get() }, { 'V', n0x8c0c70.get() },
        { 'U', n0x8c0c70.get() }, { 'T', n0x8c0c70.get() },
        { 'S', n0x8c0c70.get() }, { 'R', n0x8c0c70.get() },
        { 'Q', n0x8c0c70.get() }, { 'P', n0x8c0c70.get() },
        { 'O', n0x8c0c70.get() }, { 'N', n0x8c0c70.get() },
        { '5', n0x8c33a0.get() }, { '4', n0x8c33a0.get() },
        { '3', n0x8c33a0.get() }, { '2', n0x8c33a0.get() },
        { '1', n0x8c33a0.get() }, { '0', n0x8c0c70.get() },
        { '/', n0x8c0c70.get() }, { '\\', n0x8c0c70.get() },
        { '-', n0x8c0c70.get() }, { '.', n0x8c0c70.get() },
        { ',', n0x8c0c70.get() }, { '+', n0x8c3790.get() },
        { '$', n0x8c0c70.get() }, { '#', n0x8c0c70.get() },
        { '"', n0x8c0c70.get() }, { ' ', n0x8c0c70.get() },
        { '!', n0x8c0c70.get() }, { '%', n0x8c0c70.get() },
        { '&', n0x8c0c70.get() }, { '\'', n0x8c0c70.get() },
        { '(', n0x8c0c70.get() }, { ')', n0x8c0c70.get() },
        { '*', n0x8c1770.get() }, { '6', n0x8c33a0.get() },
        { '7', n0x8c33a0.get() }, { '8', n0x8c33a0.get() },
        { '9', n0x8c33a0.get() }, { ':', n0x8c0c70.get() },
        { ';', n0x8c0c70.get() }, { '<', n0x8c0c70.get() },
        { '=', n0x8c0c70.get() }, { '>', n0x8c0c70.get() },
        { '?', n0x8c0c70.get() }, { '@', n0x8c0c70.get() },
        { 'A', n0x8c0c70.get() }, { 'B', n0x8c0c70.get() },
        { 'C', n0x8c0c70.get() }, { 'D', n0x8c0c70.get() },
        { 'E', n0x8c0c70.get() }, { 'F', n0x8c0c70.get() },
        { 'G', n0x8c0c70.get() }, { 'H', n0x8c0c70.get() },
        { 'I', n0x8c0c70.get() }, { 'J', n0x8c0c70.get() },
        { 'K', n0x8c0c70.get() }, { 'L', n0x8c0c70.get() },
        { 'M', n0x8c0c70.get() },
      };
      n0x8c0c70->ts_ = {
        { '~', n0x8c0c70.get() }, { '}', n0x8c0c70.get() },
        { '|', n0x8c0c70.get() }, { '{', n0x8c0c70.get() },
        { 'z', n0x8c0c70.get() }, { 'y', n0x8c0c70.get() },
        { 'x', n0x8c0c70.get() }, { 'w', n0x8c0c70.get() },
        { 'v', n0x8c0c70.get() }, { 'u', n0x8c0c70.get() },
        { 't', n0x8c0c70.get() }, { 's', n0x8c0c70.get() },
        { 'r', n0x8c0c70.get() }, { 'q', n0x8c0c70.get() },
        { 'p', n0x8c0c70.get() }, { 'o', n0x8c0c70.get() },
        { 'n', n0x8c0c70.get() }, { 'm', n0x8c0c70.get() },
        { 'l', n0x8c0c70.get() }, { '\n', n0x8c0c70.get() },
        { 'k', n0x8c0c70.get() }, { '\t', n0x8c0c70.get() },
        { 'j', n0x8c0c70.get() }, { 'i', n0x8c0c70.get() },
        { 'h', n0x8c0c70.get() }, { 'g', n0x8c0c70.get() },
        { 'f', n0x8c0c70.get() }, { 'e', n0x8c0c70.get() },
        { 'd', n0x8c0c70.get() }, { 'c', n0x8c0c70.get() },
        { 'b', n0x8c0c70.get() }, { 'a', n0x8c0c70.get() },
        { '`', n0x8c0c70.get() }, { '_', n0x8c0c70.get() },
        { '^', n0x8c0c70.get() }, { ']', n0x8c0c70.get() },
        { '[', n0x8c0c70.get() }, { 'Z', n0x8c0c70.get() },
        { 'Y', n0x8c0c70.get() }, { 'X', n0x8c0c70.get() },
        { 'W', n0x8c0c70.get() }, { 'V', n0x8c0c70.get() },
        { 'U', n0x8c0c70.get() }, { 'T', n0x8c0c70.get() },
        { 'S', n0x8c0c70.get() }, { 'R', n0x8c0c70.get() },
        { 'Q', n0x8c0c70.get() }, { 'P', n0x8c0c70.get() },
        { 'O', n0x8c0c70.get() }, { 'N', n0x8c0c70.get() },
        { '5', n0x8c0c70.get() }, { '4', n0x8c0c70.get() },
        { '3', n0x8c0c70.get() }, { '2', n0x8c0c70.get() },
        { '1', n0x8c0c70.get() }, { '0', n0x8c0c70.get() },
        { '/', n0x8c0c70.get() }, { '\\', n0x8c0c70.get() },
        { '-', n0x8c0c70.get() }, { '.', n0x8c0c70.get() },
        { ',', n0x8c0c70.get() }, { '+', n0x8c0c70.get() },
        { '$', n0x8c0c70.get() }, { '#', n0x8c0c70.get() },
        { '"', n0x8c0c70.get() }, { ' ', n0x8c0c70.get() },
        { '!', n0x8c0c70.get() }, { '%', n0x8c0c70.get() },
        { '&', n0x8c0c70.get() }, { '\'', n0x8c0c70.get() },
        { '(', n0x8c0c70.get() }, { ')', n0x8c0c70.get() },
        { '*', n0x8c0c70.get() }, { '6', n0x8c0c70.get() },
        { '7', n0x8c0c70.get() }, { '8', n0x8c0c70.get() },
        { '9', n0x8c0c70.get() }, { ':', n0x8c0c70.get() },
        { ';', n0x8c0c70.get() }, { '<', n0x8c0c70.get() },
        { '=', n0x8c0c70.get() }, { '>', n0x8c0c70.get() },
        { '?', n0x8c0c70.get() }, { '@', n0x8c0c70.get() },
        { 'A', n0x8c0c70.get() }, { 'B', n0x8c0c70.get() },
        { 'C', n0x8c0c70.get() }, { 'D', n0x8c0c70.get() },
        { 'E', n0x8c0c70.get() }, { 'F', n0x8c0c70.get() },
        { 'G', n0x8c0c70.get() }, { 'H', n0x8c0c70.get() },
        { 'I', n0x8c0c70.get() }, { 'J', n0x8c0c70.get() },
        { 'K', n0x8c0c70.get() }, { 'L', n0x8c0c70.get() },
        { 'M', n0x8c0c70.get() },
      };
      n0x8c33a0->ts_ = {
        { '~', n0x8c0c70.get() }, { '}', n0x8c0c70.get() },
        { '|', n0x8c0c70.get() }, { '{', n0x8c0c70.get() },
        { 'z', n0x8c0c70.get() }, { 'y', n0x8c0c70.get() },
        { 'x', n0x8c0c70.get() }, { 'w', n0x8c0c70.get() },
        { 'v', n0x8c0c70.get() }, { 'u', n0x8c0c70.get() },
        { 't', n0x8c0c70.get() }, { 's', n0x8c0c70.get() },
        { 'r', n0x8c0c70.get() }, { 'q', n0x8c0c70.get() },
        { 'p', n0x8c0c70.get() }, { 'o', n0x8c0c70.get() },
        { 'n', n0x8c0c70.get() }, { 'm', n0x8c0c70.get() },
        { 'l', n0x8c0c70.get() }, { '\n', n0x8c0c70.get() },
        { 'k', n0x8c0c70.get() }, { '\t', n0x8c0c70.get() },
        { 'j', n0x8c0c70.get() }, { 'i', n0x8c0c70.get() },
        { 'h', n0x8c0c70.get() }, { 'g', n0x8c0c70.get() },
        { 'f', n0x8c0c70.get() }, { 'e', n0x8c0c70.get() },
        { 'd', n0x8c0c70.get() }, { 'c', n0x8c0c70.get() },
        { 'b', n0x8c0c70.get() }, { 'a', n0x8c0c70.get() },
        { '`', n0x8c0c70.get() }, { '_', n0x8c0c70.get() },
        { '^', n0x8c0c70.get() }, { ']', n0x8c0c70.get() },
        { '[', n0x8c0c70.get() }, { 'Z', n0x8c0c70.get() },
        { 'Y', n0x8c0c70.get() }, { 'X', n0x8c0c70.get() },
        { 'W', n0x8c0c70.get() }, { 'V', n0x8c0c70.get() },
        { 'U', n0x8c0c70.get() }, { 'T', n0x8c0c70.get() },
        { 'S', n0x8c0c70.get() }, { 'R', n0x8c0c70.get() },
        { 'Q', n0x8c0c70.get() }, { 'P', n0x8c0c70.get() },
        { 'O', n0x8c0c70.get() }, { 'N', n0x8c0c70.get() },
        { '5', n0x8c33a0.get() }, { '4', n0x8c33a0.get() },
        { '3', n0x8c33a0.get() }, { '2', n0x8c33a0.get() },
        { '1', n0x8c33a0.get() }, { '0', n0x8c33a0.get() },
        { '/', n0x8c0c70.get() }, { '\\', n0x8c0c70.get() },
        { '-', n0x8c0c70.get() }, { '.', n0x8c0c70.get() },
        { ',', n0x8c0c70.get() }, { '+', n0x8c0c70.get() },
        { '$', n0x8c0c70.get() }, { '#', n0x8c0c70.get() },
        { '"', n0x8c0c70.get() }, { ' ', n0x8c0c70.get() },
        { '!', n0x8c0c70.get() }, { '%', n0x8c0c70.get() },
        { '&', n0x8c0c70.get() }, { '\'', n0x8c0c70.get() },
        { '(', n0x8c0c70.get() }, { ')', n0x8c0c70.get() },
        { '*', n0x8c0c70.get() }, { '6', n0x8c33a0.get() },
        { '7', n0x8c33a0.get() }, { '8', n0x8c33a0.get() },
        { '9', n0x8c33a0.get() }, { ':', n0x8c0c70.get() },
        { ';', n0x8c0c70.get() }, { '<', n0x8c0c70.get() },
        { '=', n0x8c0c70.get() }, { '>', n0x8c0c70.get() },
        { '?', n0x8c0c70.get() }, { '@', n0x8c0c70.get() },
        { 'A', n0x8c0c70.get() }, { 'B', n0x8c0c70.get() },
        { 'C', n0x8c0c70.get() }, { 'D', n0x8c0c70.get() },
        { 'E', n0x8c0c70.get() }, { 'F', n0x8c0c70.get() },
        { 'G', n0x8c0c70.get() }, { 'H', n0x8c0c70.get() },
        { 'I', n0x8c0c70.get() }, { 'J', n0x8c0c70.get() },
        { 'K', n0x8c0c70.get() }, { 'L', n0x8c0c70.get() },
        { 'M', n0x8c0c70.get() },
      };
      n0x8c3790->ts_ = {
        { '~', n0x8c0c70.get() }, { '}', n0x8c0c70.get() },
        { '|', n0x8c0c70.get() }, { '{', n0x8c0c70.get() },
        { 'z', n0x8c0c70.get() }, { 'y', n0x8c0c70.get() },
        { 'x', n0x8c0c70.get() }, { 'w', n0x8c0c70.get() },
        { 'v', n0x8c0c70.get() }, { 'u', n0x8c0c70.get() },
        { 't', n0x8c0c70.get() }, { 's', n0x8c0c70.get() },
        { 'r', n0x8c0c70.get() }, { 'q', n0x8c0c70.get() },
        { 'p', n0x8c0c70.get() }, { 'o', n0x8c0c70.get() },
        { 'n', n0x8c0c70.get() }, { 'm', n0x8c0c70.get() },
        { 'l', n0x8c0c70.get() }, { '\n', n0x8c0c70.get() },
        { 'k', n0x8c0c70.get() }, { '\t', n0x8c0c70.get() },
        { 'j', n0x8c0c70.get() }, { 'i', n0x8c0c70.get() },
        { 'h', n0x8c0c70.get() }, { 'g', n0x8c0c70.get() },
        { 'f', n0x8c0c70.get() }, { 'e', n0x8c0c70.get() },
        { 'd', n0x8c0c70.get() }, { 'c', n0x8c0c70.get() },
        { 'b', n0x8c0c70.get() }, { 'a', n0x8c0c70.get() },
        { '`', n0x8c0c70.get() }, { '_', n0x8c0c70.get() },
        { '^', n0x8c0c70.get() }, { ']', n0x8c0c70.get() },
        { '[', n0x8c0c70.get() }, { 'Z', n0x8c0c70.get() },
        { 'Y', n0x8c0c70.get() }, { 'X', n0x8c0c70.get() },
        { 'W', n0x8c0c70.get() }, { 'V', n0x8c0c70.get() },
        { 'U', n0x8c0c70.get() }, { 'T', n0x8c0c70.get() },
        { 'S', n0x8c0c70.get() }, { 'R', n0x8c0c70.get() },
        { 'Q', n0x8c0c70.get() }, { 'P', n0x8c0c70.get() },
        { 'O', n0x8c0c70.get() }, { 'N', n0x8c0c70.get() },
        { '5', n0x8c0c70.get() }, { '4', n0x8c0c70.get() },
        { '3', n0x8c0c70.get() }, { '2', n0x8c0c70.get() },
        { '1', n0x8c0c70.get() }, { '0', n0x8c0c70.get() },
        { '/', n0x8c0c70.get() }, { '\\', n0x8c0c70.get() },
        { '-', n0x8c0c70.get() }, { '.', n0x8c0c70.get() },
        { ',', n0x8c0c70.get() }, { '+', n0x8c0c70.get() },
        { '$', n0x8c0c70.get() }, { '#', n0x8c0c70.get() },
        { '"', n0x8c0c70.get() }, { ' ', n0x8c0c70.get() },
        { '!', n0x8c0c70.get() }, { '%', n0x8c0c70.get() },
        { '&', n0x8c0c70.get() }, { '\'', n0x8c0c70.get() },
        { '(', n0x8c0c70.get() }, { ')', n0x8c0c70.get() },
        { '*', n0x8c0c70.get() }, { '6', n0x8c0c70.get() },
        { '7', n0x8c0c70.get() }, { '8', n0x8c0c70.get() },
        { '9', n0x8c0c70.get() }, { ':', n0x8c0c70.get() },
        { ';', n0x8c0c70.get() }, { '<', n0x8c0c70.get() },
        { '=', n0x8c0c70.get() }, { '>', n0x8c0c70.get() },
        { '?', n0x8c0c70.get() }, { '@', n0x8c0c70.get() },
        { 'A', n0x8c0c70.get() }, { 'B', n0x8c0c70.get() },
        { 'C', n0x8c0c70.get() }, { 'D', n0x8c0c70.get() },
        { 'E', n0x8c0c70.get() }, { 'F', n0x8c0c70.get() },
        { 'G', n0x8c0c70.get() }, { 'H', n0x8c0c70.get() },
        { 'I', n0x8c0c70.get() }, { 'J', n0x8c0c70.get() },
        { 'K', n0x8c0c70.get() }, { 'L', n0x8c0c70.get() },
        { 'M', n0x8c0c70.get() },
      };
      n0x8c1770->ts_ = {
        { '~', n0x8c0c70.get() }, { '}', n0x8c0c70.get() },
        { '|', n0x8c0c70.get() }, { '{', n0x8c0c70.get() },
        { 'z', n0x8c0c70.get() }, { 'y', n0x8c0c70.get() },
        { 'x', n0x8c0c70.get() }, { 'w', n0x8c0c70.get() },
        { 'v', n0x8c0c70.get() }, { 'u', n0x8c0c70.get() },
        { 't', n0x8c0c70.get() }, { 's', n0x8c0c70.get() },
        { 'r', n0x8c0c70.get() }, { 'q', n0x8c0c70.get() },
        { 'p', n0x8c0c70.get() }, { 'o', n0x8c0c70.get() },
        { 'n', n0x8c0c70.get() }, { 'm', n0x8c0c70.get() },
        { 'l', n0x8c0c70.get() }, { '\n', n0x8c0c70.get() },
        { 'k', n0x8c0c70.get() }, { '\t', n0x8c0c70.get() },
        { 'j', n0x8c0c70.get() }, { 'i', n0x8c0c70.get() },
        { 'h', n0x8c0c70.get() }, { 'g', n0x8c0c70.get() },
        { 'f', n0x8c0c70.get() }, { 'e', n0x8c0c70.get() },
        { 'd', n0x8c0c70.get() }, { 'c', n0x8c0c70.get() },
        { 'b', n0x8c0c70.get() }, { 'a', n0x8c0c70.get() },
        { '`', n0x8c0c70.get() }, { '_', n0x8c0c70.get() },
        { '^', n0x8c0c70.get() }, { ']', n0x8c0c70.get() },
        { '[', n0x8c0c70.get() }, { 'Z', n0x8c0c70.get() },
        { 'Y', n0x8c0c70.get() }, { 'X', n0x8c0c70.get() },
        { 'W', n0x8c0c70.get() }, { 'V', n0x8c0c70.get() },
        { 'U', n0x8c0c70.get() }, { 'T', n0x8c0c70.get() },
        { 'S', n0x8c0c70.get() }, { 'R', n0x8c0c70.get() },
        { 'Q', n0x8c0c70.get() }, { 'P', n0x8c0c70.get() },
        { 'O', n0x8c0c70.get() }, { 'N', n0x8c0c70.get() },
        { '5', n0x8c0c70.get() }, { '4', n0x8c0c70.get() },
        { '3', n0x8c0c70.get() }, { '2', n0x8c0c70.get() },
        { '1', n0x8c0c70.get() }, { '0', n0x8c0c70.get() },
        { '/', n0x8c0c70.get() }, { '\\', n0x8c0c70.get() },
        { '-', n0x8c0c70.get() }, { '.', n0x8c0c70.get() },
        { ',', n0x8c0c70.get() }, { '+', n0x8c0c70.get() },
        { '$', n0x8c0c70.get() }, { '#', n0x8c0c70.get() },
        { '"', n0x8c0c70.get() }, { ' ', n0x8c0c70.get() },
        { '!', n0x8c0c70.get() }, { '%', n0x8c0c70.get() },
        { '&', n0x8c0c70.get() }, { '\'', n0x8c0c70.get() },
        { '(', n0x8c0c70.get() }, { ')', n0x8c0c70.get() },
        { '*', n0x8c0c70.get() }, { '6', n0x8c0c70.get() },
        { '7', n0x8c0c70.get() }, { '8', n0x8c0c70.get() },
        { '9', n0x8c0c70.get() }, { ':', n0x8c0c70.get() },
        { ';', n0x8c0c70.get() }, { '<', n0x8c0c70.get() },
        { '=', n0x8c0c70.get() }, { '>', n0x8c0c70.get() },
        { '?', n0x8c0c70.get() }, { '@', n0x8c0c70.get() },
        { 'A', n0x8c0c70.get() }, { 'B', n0x8c0c70.get() },
        { 'C', n0x8c0c70.get() }, { 'D', n0x8c0c70.get() },
        { 'E', n0x8c0c70.get() }, { 'F', n0x8c0c70.get() },
        { 'G', n0x8c0c70.get() }, { 'H', n0x8c0c70.get() },
        { 'I', n0x8c0c70.get() }, { 'J', n0x8c0c70.get() },
        { 'K', n0x8c0c70.get() }, { 'L', n0x8c0c70.get() },
        { 'M', n0x8c0c70.get() },
      };
      return std::move(n0x93c080);
    }
    auto root = makeDFA();
  }  // namespace lexer
  enum class Assoc { LEFT, RIGHT, NOT, NONE };
  struct DFARule {
    int concrete;
    vector<int> symbols;
    size_t pos;
    mutable vector<bool> lookahead;
  };
  struct RuleData {
    optional<DFARule> reducibleRule;
    int precedence;
    Assoc assoc;
  };
  namespace parser {
    struct Node {
      Node(RuleData&& v) : v_(std::move(v)) {}

      Node* step(const int& t) const {
        auto iter = ts_.find(t);
        if (iter == ts_.end()) {
          return nullptr;
        }
        return iter->second;
      }
      RuleData v_;
      std::unordered_map<int, Node*> ts_;
    };

    auto n0x8c5500 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0x8c6df0 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 1,
                                                              {
                                                                  -1,
                                                              },
                                                              1,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                              } } },
                                         -2147483648,
                                         Assoc::NONE });
    auto n0x8c91d0 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 0,
                                                              {
                                                                  1,
                                                              },
                                                              1,
                                                              {
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         -2147483648,
                                         Assoc::NONE });
    auto n0x8cc790 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0x8cb530 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0x8c2710 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 3,
                                                              {
                                                                  1,
                                                                  -3,
                                                                  1,
                                                              },
                                                              3,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                              } } },
                                         2,
                                         Assoc::LEFT });
    auto n0x8cc860 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 2,
                                                              {
                                                                  1,
                                                                  -2,
                                                                  1,
                                                              },
                                                              3,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                              } } },
                                         1,
                                         Assoc::LEFT });
    auto makeDFA() {
      n0x8c5500->ts_ = {
        { -1, n0x8c6df0.get() },
        { 1, n0x8c91d0.get() },
      };
      n0x8c6df0->ts_ = {};
      n0x8c91d0->ts_ = {
        { -3, n0x8cc790.get() },
        { -2, n0x8cb530.get() },
      };
      n0x8cc790->ts_ = {
        { -1, n0x8c6df0.get() },
        { 1, n0x8c2710.get() },
      };
      n0x8cb530->ts_ = {
        { -1, n0x8c6df0.get() },
        { 1, n0x8cc860.get() },
      };
      n0x8c2710->ts_ = {
        { -3, n0x8cc790.get() },
        { -2, n0x8cb530.get() },
      };
      n0x8cc860->ts_ = {
        { -3, n0x8cc790.get() },
        { -2, n0x8cb530.get() },
      };
      return std::move(n0x8c5500);
    }
    auto root = makeDFA();
  }  // namespace parser
  void consumeWhiteSpace(string_view& input) {
    size_t i = 0;
    char c;
    while (isspace((c = input[i]))) {
      if (c == '\n') {
        ++currentLine;
      }
      ++i;
    }
    input = input.substr(i);
  }

  optional<StackObj> getToken(string_view& input) {
    size_t i = 0;
    const size_t len = input.size();
    size_t lastAcceptingPos;
    int lastAcceptingToken = NONE;
    size_t newlineCount = 0;
    size_t lastAcceptingNewlineCount = 0;
    lexer::Node* currentNode = lexer::root.get();

    while (currentNode) {
      int token = currentNode->v_;
      if (token != NONE) {
        lastAcceptingToken = token;
        lastAcceptingPos = i;
        lastAcceptingNewlineCount = newlineCount;
      }

      if (i == len) {
        break;
      }
      char c;
      currentNode = currentNode->step((c = input[i]));
      ++i;
      if (c == '\n') {
        ++newlineCount;
      }
    }

    if (lastAcceptingToken == NONE) {
      return {};
    }

    StackObj stackObj = constructTokenObj(
        lastAcceptingToken, input.substr(0, lastAcceptingPos));
    input = input.substr(lastAcceptingPos);
    currentLine += lastAcceptingNewlineCount;
    return { move(stackObj) };
  }


  vector<StackObj> tokenize(const string& input) {
    if (input.empty()) {
      return {};
    }

    vector<StackObj> tokens;
    string_view inputView = input;

    consumeWhiteSpace(inputView);
    while (!inputView.empty()) {
      optional<StackObj> optionalObj = getToken(inputView);
      if (optionalObj.has_value()) {
        if (GRAMMAR_DATA.tokens[tokenToFromIndex(optionalObj->symbol)]
                .precedence != SKIP_TOKEN) {
          tokens.push_back(move(*optionalObj));
        }
      } else {
        ostringstream error;
        vector<string> prevTokenNames;
        auto startIter =
            tokens.size() < 25 ? tokens.cbegin() : tokens.cend() - 25;
        transform(
            move(startIter),
            tokens.cend(),
            back_inserter(prevTokenNames),
            [](const StackObj& stackObj) {
              return GRAMMAR_DATA.tokens[tokenToFromIndex(stackObj.symbol)]
                  .name;
            });
        error << "Lexer error on line " << currentLine
              << " at: " << inputView.substr(0, 25) << '\n'
              << "Previous tokens were: " << prevTokenNames;
        for_each(tokens.cbegin(), tokens.cend(), deleteObj);
        throw runtime_error(error.str());
      }

      consumeWhiteSpace(inputView);
    }

    return tokens;
  }

  void cleanPtrsFrom(const vector<StackObj>& stackObjs, size_t i) {
    size_t size = stackObjs.size();
    for (; i < size; ++i) {
      deleteObj(stackObjs[i]);
    }
  }

  parser::Node* runDFA(const std::vector<int>& input) {
    parser::Node* currentNode = parser::root.get();
    for (int inputToken : input) {
      currentNode = currentNode->step(inputToken);
      if (currentNode == nullptr) {
        return nullptr;
      }
    }
    return currentNode;
  }

  void parseError(
      const vector<StackObj>& stk,
      const vector<StackObj>& inputTokens,
      size_t i) {
    ostringstream errMsg;
    vector<string> stkSymbolNames;
    vector<string> remainingTokenNames;
    auto stkObjToName = [](StackObj stkObj) {
      if (isToken(stkObj.symbol)) {
        return GRAMMAR_DATA.tokens[tokenToFromIndex(stkObj.symbol)].name;
      }
      return GRAMMAR_DATA.variables[stkObj.symbol].name;
    };

    transform(
        stk.begin(), stk.end(), back_inserter(stkSymbolNames), stkObjToName);
    transform(
        inputTokens.begin() + i,
        inputTokens.end(),
        back_inserter(remainingTokenNames),
        stkObjToName);

    errMsg << "Parse error on line " << stk[stk.size() - 1].line
           << ":\n\tStack: " << stkSymbolNames
           << "\n\tRemaining tokens: " << remainingTokenNames;
    throw invalid_argument(errMsg.str());
  }
  int tryReduce(
      const parser::Node* node,
      int nextToken,
      const vector<StackObj>& stk,
      const vector<Token>& tokens) {
    const RuleData& ruleData = node->v_;
    if (!ruleData.reducibleRule.has_value()) {
      return NONE;
    }
    const DFARule& rule = *ruleData.reducibleRule;
    if (nextToken != NONE && !rule.lookahead[tokenToFromIndex(nextToken)]) {
      return NONE;
    }
    if (!equal(
            rule.symbols.crbegin(),
            rule.symbols.crend(),
            stk.crbegin(),
            [](int symbol, const StackObj& stkObj) {
              return stkObj.symbol == symbol;
            })) {
      return NONE;
    }
    if (!node->ts_.contains(nextToken)) {
      return ruleData.reducibleRule->concrete;
    }
    int shiftPrecedence = tokens[tokenToFromIndex(nextToken)].precedence;
    if (ruleData.precedence == NONE && shiftPrecedence == NONE) {
      return NONE;
    }
    if (ruleData.precedence > shiftPrecedence) {
      return ruleData.reducibleRule->concrete;
    } else if (ruleData.precedence == shiftPrecedence) {
      if (ruleData.assoc == Assoc::LEFT) {
        return ruleData.reducibleRule->concrete;
      } else if (ruleData.assoc == Assoc::NONE) {
        return NONE;
      }
    }
    return NONE;
  }
  Expr* shiftReduce(vector<StackObj>& inputTokens) {
    vector<StackObj> stk = { move(inputTokens[0]) };
    size_t i = 1;
    size_t inputSize = inputTokens.size();
    while (!(i == inputSize && stk.size() == 1 && stk[0].symbol == S)) {
      vector<int> stkSymbols;
      transform(
          stk.begin(),
          stk.end(),
          back_inserter(stkSymbols),
          [](StackObj stkObj) { return stkObj.symbol; });
      parser::Node* currentNode = runDFA(stkSymbols);
      if (currentNode == nullptr) {
        cleanPtrsFrom(stk, 0);
        cleanPtrsFrom(inputTokens, i + 1);
        parseError(stk, inputTokens, i + 1);
      }
      int nextInputToken = i == inputSize ? NONE : inputTokens[i].symbol;
      int concrete =
          tryReduce(currentNode, nextInputToken, stk, GRAMMAR_DATA.tokens);
      if (concrete != NONE) {
        size_t reduceStart =
            stk.size() - currentNode->v_.reducibleRule->symbols.size();
        StackObj newObj = construct(
            concrete,
            stk,
            reduceStart,
            GRAMMAR_DATA.concretes[concrete].varType);
        if (newObj.symbol == S) {
          stk.pop_back();
        } else {
          size_t stkSize = stk.size();
          for (size_t j = 0; j < stkSize - reduceStart; ++j) {
            deleteObjPtr(stk.back());
            stk.pop_back();
          }
        }
        stk.push_back(newObj);
      } else {
        if (i == inputSize) {
          cleanPtrsFrom(stk, 0);
          parseError(stk, inputTokens, i);
        }
        StackObj token = inputTokens[i];
        currentNode = currentNode->step(token.symbol);
        stk.push_back(move(token));

        if (currentNode == nullptr) {
          cleanPtrsFrom(stk, 0);
          cleanPtrsFrom(inputTokens, i + 1);
          parseError(stk, inputTokens, i + 1);
        }
        ++i;
      }
    }

    SObj* start = (SObj*)(stk[0].obj);
    Expr* root = *start->r_;
    delete start;
    return root;
  }
}  // namespace
namespace test::parser::generated::expr_parser {
  Expr* parse(const string& input) {
    vector<StackObj> stackObjs = tokenize(input);
    return shiftReduce(stackObjs);
  }
  Expr* parse(istream& input) {
    return parse(string(
        istreambuf_iterator<char>{ input }, istreambuf_iterator<char>{}));
  }
}  // namespace test::parser::generated::expr_parser