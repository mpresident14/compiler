/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */
#include "test/parser/generated/bad_expr_parser.hpp"

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
                                   { "PLUS", "", -2147483648 },
                                   { "STAR", "", -2147483648 },
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
    void* obj = s.obj;
    switch (s.symbol) {
      case -1:
        delete (int*)obj;
        break;
      case 1:
        delete (Expr**)s.obj;
        break;
      default:
        return;
    }
  }
  void deleteObj(const StackObj& s) {
    void* obj = s.obj;
    switch (s.symbol) {
      case -1:
        delete (int*)obj;
        break;
      case 1:
        delete *(Expr**)obj;
        delete (Expr**)obj;
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
  StackObj constructTokenObj(int token, string_view str) {
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

    auto n0x13605f0 = std::make_unique<Node>(-2147483648);
    auto n0x139ffc0 = std::make_unique<Node>(-2147483648);
    auto n0x1360590 = std::make_unique<Node>(-1);
    auto n0x13a21e0 = std::make_unique<Node>(-2);
    auto n0x13a0020 = std::make_unique<Node>(-3);
    auto makeDFA() {
      n0x13605f0->ts_ = {
        { '~', n0x139ffc0.get() }, { '}', n0x139ffc0.get() },
        { '|', n0x139ffc0.get() }, { '{', n0x139ffc0.get() },
        { 'z', n0x139ffc0.get() }, { 'y', n0x139ffc0.get() },
        { 'x', n0x139ffc0.get() }, { 'w', n0x139ffc0.get() },
        { 'v', n0x139ffc0.get() }, { 'u', n0x139ffc0.get() },
        { 't', n0x139ffc0.get() }, { 's', n0x139ffc0.get() },
        { 'r', n0x139ffc0.get() }, { 'q', n0x139ffc0.get() },
        { 'p', n0x139ffc0.get() }, { 'o', n0x139ffc0.get() },
        { 'n', n0x139ffc0.get() }, { 'm', n0x139ffc0.get() },
        { 'l', n0x139ffc0.get() }, { '\n', n0x139ffc0.get() },
        { 'k', n0x139ffc0.get() }, { '\t', n0x139ffc0.get() },
        { 'j', n0x139ffc0.get() }, { 'i', n0x139ffc0.get() },
        { 'h', n0x139ffc0.get() }, { 'g', n0x139ffc0.get() },
        { 'f', n0x139ffc0.get() }, { 'e', n0x139ffc0.get() },
        { 'd', n0x139ffc0.get() }, { 'c', n0x139ffc0.get() },
        { 'b', n0x139ffc0.get() }, { 'a', n0x139ffc0.get() },
        { '`', n0x139ffc0.get() }, { '_', n0x139ffc0.get() },
        { '^', n0x139ffc0.get() }, { ']', n0x139ffc0.get() },
        { '[', n0x139ffc0.get() }, { 'Z', n0x139ffc0.get() },
        { 'Y', n0x139ffc0.get() }, { 'X', n0x139ffc0.get() },
        { 'W', n0x139ffc0.get() }, { 'V', n0x139ffc0.get() },
        { 'U', n0x139ffc0.get() }, { 'T', n0x139ffc0.get() },
        { 'S', n0x139ffc0.get() }, { 'R', n0x139ffc0.get() },
        { 'Q', n0x139ffc0.get() }, { 'P', n0x139ffc0.get() },
        { 'O', n0x139ffc0.get() }, { 'N', n0x139ffc0.get() },
        { '5', n0x1360590.get() }, { '4', n0x1360590.get() },
        { '3', n0x1360590.get() }, { '2', n0x1360590.get() },
        { '1', n0x1360590.get() }, { '0', n0x139ffc0.get() },
        { '/', n0x139ffc0.get() }, { '\\', n0x139ffc0.get() },
        { '-', n0x139ffc0.get() }, { '.', n0x139ffc0.get() },
        { ',', n0x139ffc0.get() }, { '+', n0x13a21e0.get() },
        { '$', n0x139ffc0.get() }, { '#', n0x139ffc0.get() },
        { '"', n0x139ffc0.get() }, { ' ', n0x139ffc0.get() },
        { '!', n0x139ffc0.get() }, { '%', n0x139ffc0.get() },
        { '&', n0x139ffc0.get() }, { '\'', n0x139ffc0.get() },
        { '(', n0x139ffc0.get() }, { ')', n0x139ffc0.get() },
        { '*', n0x13a0020.get() }, { '6', n0x1360590.get() },
        { '7', n0x1360590.get() }, { '8', n0x1360590.get() },
        { '9', n0x1360590.get() }, { ':', n0x139ffc0.get() },
        { ';', n0x139ffc0.get() }, { '<', n0x139ffc0.get() },
        { '=', n0x139ffc0.get() }, { '>', n0x139ffc0.get() },
        { '?', n0x139ffc0.get() }, { '@', n0x139ffc0.get() },
        { 'A', n0x139ffc0.get() }, { 'B', n0x139ffc0.get() },
        { 'C', n0x139ffc0.get() }, { 'D', n0x139ffc0.get() },
        { 'E', n0x139ffc0.get() }, { 'F', n0x139ffc0.get() },
        { 'G', n0x139ffc0.get() }, { 'H', n0x139ffc0.get() },
        { 'I', n0x139ffc0.get() }, { 'J', n0x139ffc0.get() },
        { 'K', n0x139ffc0.get() }, { 'L', n0x139ffc0.get() },
        { 'M', n0x139ffc0.get() },
      };
      n0x139ffc0->ts_ = {
        { '~', n0x139ffc0.get() }, { '}', n0x139ffc0.get() },
        { '|', n0x139ffc0.get() }, { '{', n0x139ffc0.get() },
        { 'z', n0x139ffc0.get() }, { 'y', n0x139ffc0.get() },
        { 'x', n0x139ffc0.get() }, { 'w', n0x139ffc0.get() },
        { 'v', n0x139ffc0.get() }, { 'u', n0x139ffc0.get() },
        { 't', n0x139ffc0.get() }, { 's', n0x139ffc0.get() },
        { 'r', n0x139ffc0.get() }, { 'q', n0x139ffc0.get() },
        { 'p', n0x139ffc0.get() }, { 'o', n0x139ffc0.get() },
        { 'n', n0x139ffc0.get() }, { 'm', n0x139ffc0.get() },
        { 'l', n0x139ffc0.get() }, { '\n', n0x139ffc0.get() },
        { 'k', n0x139ffc0.get() }, { '\t', n0x139ffc0.get() },
        { 'j', n0x139ffc0.get() }, { 'i', n0x139ffc0.get() },
        { 'h', n0x139ffc0.get() }, { 'g', n0x139ffc0.get() },
        { 'f', n0x139ffc0.get() }, { 'e', n0x139ffc0.get() },
        { 'd', n0x139ffc0.get() }, { 'c', n0x139ffc0.get() },
        { 'b', n0x139ffc0.get() }, { 'a', n0x139ffc0.get() },
        { '`', n0x139ffc0.get() }, { '_', n0x139ffc0.get() },
        { '^', n0x139ffc0.get() }, { ']', n0x139ffc0.get() },
        { '[', n0x139ffc0.get() }, { 'Z', n0x139ffc0.get() },
        { 'Y', n0x139ffc0.get() }, { 'X', n0x139ffc0.get() },
        { 'W', n0x139ffc0.get() }, { 'V', n0x139ffc0.get() },
        { 'U', n0x139ffc0.get() }, { 'T', n0x139ffc0.get() },
        { 'S', n0x139ffc0.get() }, { 'R', n0x139ffc0.get() },
        { 'Q', n0x139ffc0.get() }, { 'P', n0x139ffc0.get() },
        { 'O', n0x139ffc0.get() }, { 'N', n0x139ffc0.get() },
        { '5', n0x139ffc0.get() }, { '4', n0x139ffc0.get() },
        { '3', n0x139ffc0.get() }, { '2', n0x139ffc0.get() },
        { '1', n0x139ffc0.get() }, { '0', n0x139ffc0.get() },
        { '/', n0x139ffc0.get() }, { '\\', n0x139ffc0.get() },
        { '-', n0x139ffc0.get() }, { '.', n0x139ffc0.get() },
        { ',', n0x139ffc0.get() }, { '+', n0x139ffc0.get() },
        { '$', n0x139ffc0.get() }, { '#', n0x139ffc0.get() },
        { '"', n0x139ffc0.get() }, { ' ', n0x139ffc0.get() },
        { '!', n0x139ffc0.get() }, { '%', n0x139ffc0.get() },
        { '&', n0x139ffc0.get() }, { '\'', n0x139ffc0.get() },
        { '(', n0x139ffc0.get() }, { ')', n0x139ffc0.get() },
        { '*', n0x139ffc0.get() }, { '6', n0x139ffc0.get() },
        { '7', n0x139ffc0.get() }, { '8', n0x139ffc0.get() },
        { '9', n0x139ffc0.get() }, { ':', n0x139ffc0.get() },
        { ';', n0x139ffc0.get() }, { '<', n0x139ffc0.get() },
        { '=', n0x139ffc0.get() }, { '>', n0x139ffc0.get() },
        { '?', n0x139ffc0.get() }, { '@', n0x139ffc0.get() },
        { 'A', n0x139ffc0.get() }, { 'B', n0x139ffc0.get() },
        { 'C', n0x139ffc0.get() }, { 'D', n0x139ffc0.get() },
        { 'E', n0x139ffc0.get() }, { 'F', n0x139ffc0.get() },
        { 'G', n0x139ffc0.get() }, { 'H', n0x139ffc0.get() },
        { 'I', n0x139ffc0.get() }, { 'J', n0x139ffc0.get() },
        { 'K', n0x139ffc0.get() }, { 'L', n0x139ffc0.get() },
        { 'M', n0x139ffc0.get() },
      };
      n0x1360590->ts_ = {
        { '~', n0x139ffc0.get() }, { '}', n0x139ffc0.get() },
        { '|', n0x139ffc0.get() }, { '{', n0x139ffc0.get() },
        { 'z', n0x139ffc0.get() }, { 'y', n0x139ffc0.get() },
        { 'x', n0x139ffc0.get() }, { 'w', n0x139ffc0.get() },
        { 'v', n0x139ffc0.get() }, { 'u', n0x139ffc0.get() },
        { 't', n0x139ffc0.get() }, { 's', n0x139ffc0.get() },
        { 'r', n0x139ffc0.get() }, { 'q', n0x139ffc0.get() },
        { 'p', n0x139ffc0.get() }, { 'o', n0x139ffc0.get() },
        { 'n', n0x139ffc0.get() }, { 'm', n0x139ffc0.get() },
        { 'l', n0x139ffc0.get() }, { '\n', n0x139ffc0.get() },
        { 'k', n0x139ffc0.get() }, { '\t', n0x139ffc0.get() },
        { 'j', n0x139ffc0.get() }, { 'i', n0x139ffc0.get() },
        { 'h', n0x139ffc0.get() }, { 'g', n0x139ffc0.get() },
        { 'f', n0x139ffc0.get() }, { 'e', n0x139ffc0.get() },
        { 'd', n0x139ffc0.get() }, { 'c', n0x139ffc0.get() },
        { 'b', n0x139ffc0.get() }, { 'a', n0x139ffc0.get() },
        { '`', n0x139ffc0.get() }, { '_', n0x139ffc0.get() },
        { '^', n0x139ffc0.get() }, { ']', n0x139ffc0.get() },
        { '[', n0x139ffc0.get() }, { 'Z', n0x139ffc0.get() },
        { 'Y', n0x139ffc0.get() }, { 'X', n0x139ffc0.get() },
        { 'W', n0x139ffc0.get() }, { 'V', n0x139ffc0.get() },
        { 'U', n0x139ffc0.get() }, { 'T', n0x139ffc0.get() },
        { 'S', n0x139ffc0.get() }, { 'R', n0x139ffc0.get() },
        { 'Q', n0x139ffc0.get() }, { 'P', n0x139ffc0.get() },
        { 'O', n0x139ffc0.get() }, { 'N', n0x139ffc0.get() },
        { '5', n0x1360590.get() }, { '4', n0x1360590.get() },
        { '3', n0x1360590.get() }, { '2', n0x1360590.get() },
        { '1', n0x1360590.get() }, { '0', n0x1360590.get() },
        { '/', n0x139ffc0.get() }, { '\\', n0x139ffc0.get() },
        { '-', n0x139ffc0.get() }, { '.', n0x139ffc0.get() },
        { ',', n0x139ffc0.get() }, { '+', n0x139ffc0.get() },
        { '$', n0x139ffc0.get() }, { '#', n0x139ffc0.get() },
        { '"', n0x139ffc0.get() }, { ' ', n0x139ffc0.get() },
        { '!', n0x139ffc0.get() }, { '%', n0x139ffc0.get() },
        { '&', n0x139ffc0.get() }, { '\'', n0x139ffc0.get() },
        { '(', n0x139ffc0.get() }, { ')', n0x139ffc0.get() },
        { '*', n0x139ffc0.get() }, { '6', n0x1360590.get() },
        { '7', n0x1360590.get() }, { '8', n0x1360590.get() },
        { '9', n0x1360590.get() }, { ':', n0x139ffc0.get() },
        { ';', n0x139ffc0.get() }, { '<', n0x139ffc0.get() },
        { '=', n0x139ffc0.get() }, { '>', n0x139ffc0.get() },
        { '?', n0x139ffc0.get() }, { '@', n0x139ffc0.get() },
        { 'A', n0x139ffc0.get() }, { 'B', n0x139ffc0.get() },
        { 'C', n0x139ffc0.get() }, { 'D', n0x139ffc0.get() },
        { 'E', n0x139ffc0.get() }, { 'F', n0x139ffc0.get() },
        { 'G', n0x139ffc0.get() }, { 'H', n0x139ffc0.get() },
        { 'I', n0x139ffc0.get() }, { 'J', n0x139ffc0.get() },
        { 'K', n0x139ffc0.get() }, { 'L', n0x139ffc0.get() },
        { 'M', n0x139ffc0.get() },
      };
      n0x13a21e0->ts_ = {
        { '~', n0x139ffc0.get() }, { '}', n0x139ffc0.get() },
        { '|', n0x139ffc0.get() }, { '{', n0x139ffc0.get() },
        { 'z', n0x139ffc0.get() }, { 'y', n0x139ffc0.get() },
        { 'x', n0x139ffc0.get() }, { 'w', n0x139ffc0.get() },
        { 'v', n0x139ffc0.get() }, { 'u', n0x139ffc0.get() },
        { 't', n0x139ffc0.get() }, { 's', n0x139ffc0.get() },
        { 'r', n0x139ffc0.get() }, { 'q', n0x139ffc0.get() },
        { 'p', n0x139ffc0.get() }, { 'o', n0x139ffc0.get() },
        { 'n', n0x139ffc0.get() }, { 'm', n0x139ffc0.get() },
        { 'l', n0x139ffc0.get() }, { '\n', n0x139ffc0.get() },
        { 'k', n0x139ffc0.get() }, { '\t', n0x139ffc0.get() },
        { 'j', n0x139ffc0.get() }, { 'i', n0x139ffc0.get() },
        { 'h', n0x139ffc0.get() }, { 'g', n0x139ffc0.get() },
        { 'f', n0x139ffc0.get() }, { 'e', n0x139ffc0.get() },
        { 'd', n0x139ffc0.get() }, { 'c', n0x139ffc0.get() },
        { 'b', n0x139ffc0.get() }, { 'a', n0x139ffc0.get() },
        { '`', n0x139ffc0.get() }, { '_', n0x139ffc0.get() },
        { '^', n0x139ffc0.get() }, { ']', n0x139ffc0.get() },
        { '[', n0x139ffc0.get() }, { 'Z', n0x139ffc0.get() },
        { 'Y', n0x139ffc0.get() }, { 'X', n0x139ffc0.get() },
        { 'W', n0x139ffc0.get() }, { 'V', n0x139ffc0.get() },
        { 'U', n0x139ffc0.get() }, { 'T', n0x139ffc0.get() },
        { 'S', n0x139ffc0.get() }, { 'R', n0x139ffc0.get() },
        { 'Q', n0x139ffc0.get() }, { 'P', n0x139ffc0.get() },
        { 'O', n0x139ffc0.get() }, { 'N', n0x139ffc0.get() },
        { '5', n0x139ffc0.get() }, { '4', n0x139ffc0.get() },
        { '3', n0x139ffc0.get() }, { '2', n0x139ffc0.get() },
        { '1', n0x139ffc0.get() }, { '0', n0x139ffc0.get() },
        { '/', n0x139ffc0.get() }, { '\\', n0x139ffc0.get() },
        { '-', n0x139ffc0.get() }, { '.', n0x139ffc0.get() },
        { ',', n0x139ffc0.get() }, { '+', n0x139ffc0.get() },
        { '$', n0x139ffc0.get() }, { '#', n0x139ffc0.get() },
        { '"', n0x139ffc0.get() }, { ' ', n0x139ffc0.get() },
        { '!', n0x139ffc0.get() }, { '%', n0x139ffc0.get() },
        { '&', n0x139ffc0.get() }, { '\'', n0x139ffc0.get() },
        { '(', n0x139ffc0.get() }, { ')', n0x139ffc0.get() },
        { '*', n0x139ffc0.get() }, { '6', n0x139ffc0.get() },
        { '7', n0x139ffc0.get() }, { '8', n0x139ffc0.get() },
        { '9', n0x139ffc0.get() }, { ':', n0x139ffc0.get() },
        { ';', n0x139ffc0.get() }, { '<', n0x139ffc0.get() },
        { '=', n0x139ffc0.get() }, { '>', n0x139ffc0.get() },
        { '?', n0x139ffc0.get() }, { '@', n0x139ffc0.get() },
        { 'A', n0x139ffc0.get() }, { 'B', n0x139ffc0.get() },
        { 'C', n0x139ffc0.get() }, { 'D', n0x139ffc0.get() },
        { 'E', n0x139ffc0.get() }, { 'F', n0x139ffc0.get() },
        { 'G', n0x139ffc0.get() }, { 'H', n0x139ffc0.get() },
        { 'I', n0x139ffc0.get() }, { 'J', n0x139ffc0.get() },
        { 'K', n0x139ffc0.get() }, { 'L', n0x139ffc0.get() },
        { 'M', n0x139ffc0.get() },
      };
      n0x13a0020->ts_ = {
        { '~', n0x139ffc0.get() }, { '}', n0x139ffc0.get() },
        { '|', n0x139ffc0.get() }, { '{', n0x139ffc0.get() },
        { 'z', n0x139ffc0.get() }, { 'y', n0x139ffc0.get() },
        { 'x', n0x139ffc0.get() }, { 'w', n0x139ffc0.get() },
        { 'v', n0x139ffc0.get() }, { 'u', n0x139ffc0.get() },
        { 't', n0x139ffc0.get() }, { 's', n0x139ffc0.get() },
        { 'r', n0x139ffc0.get() }, { 'q', n0x139ffc0.get() },
        { 'p', n0x139ffc0.get() }, { 'o', n0x139ffc0.get() },
        { 'n', n0x139ffc0.get() }, { 'm', n0x139ffc0.get() },
        { 'l', n0x139ffc0.get() }, { '\n', n0x139ffc0.get() },
        { 'k', n0x139ffc0.get() }, { '\t', n0x139ffc0.get() },
        { 'j', n0x139ffc0.get() }, { 'i', n0x139ffc0.get() },
        { 'h', n0x139ffc0.get() }, { 'g', n0x139ffc0.get() },
        { 'f', n0x139ffc0.get() }, { 'e', n0x139ffc0.get() },
        { 'd', n0x139ffc0.get() }, { 'c', n0x139ffc0.get() },
        { 'b', n0x139ffc0.get() }, { 'a', n0x139ffc0.get() },
        { '`', n0x139ffc0.get() }, { '_', n0x139ffc0.get() },
        { '^', n0x139ffc0.get() }, { ']', n0x139ffc0.get() },
        { '[', n0x139ffc0.get() }, { 'Z', n0x139ffc0.get() },
        { 'Y', n0x139ffc0.get() }, { 'X', n0x139ffc0.get() },
        { 'W', n0x139ffc0.get() }, { 'V', n0x139ffc0.get() },
        { 'U', n0x139ffc0.get() }, { 'T', n0x139ffc0.get() },
        { 'S', n0x139ffc0.get() }, { 'R', n0x139ffc0.get() },
        { 'Q', n0x139ffc0.get() }, { 'P', n0x139ffc0.get() },
        { 'O', n0x139ffc0.get() }, { 'N', n0x139ffc0.get() },
        { '5', n0x139ffc0.get() }, { '4', n0x139ffc0.get() },
        { '3', n0x139ffc0.get() }, { '2', n0x139ffc0.get() },
        { '1', n0x139ffc0.get() }, { '0', n0x139ffc0.get() },
        { '/', n0x139ffc0.get() }, { '\\', n0x139ffc0.get() },
        { '-', n0x139ffc0.get() }, { '.', n0x139ffc0.get() },
        { ',', n0x139ffc0.get() }, { '+', n0x139ffc0.get() },
        { '$', n0x139ffc0.get() }, { '#', n0x139ffc0.get() },
        { '"', n0x139ffc0.get() }, { ' ', n0x139ffc0.get() },
        { '!', n0x139ffc0.get() }, { '%', n0x139ffc0.get() },
        { '&', n0x139ffc0.get() }, { '\'', n0x139ffc0.get() },
        { '(', n0x139ffc0.get() }, { ')', n0x139ffc0.get() },
        { '*', n0x139ffc0.get() }, { '6', n0x139ffc0.get() },
        { '7', n0x139ffc0.get() }, { '8', n0x139ffc0.get() },
        { '9', n0x139ffc0.get() }, { ':', n0x139ffc0.get() },
        { ';', n0x139ffc0.get() }, { '<', n0x139ffc0.get() },
        { '=', n0x139ffc0.get() }, { '>', n0x139ffc0.get() },
        { '?', n0x139ffc0.get() }, { '@', n0x139ffc0.get() },
        { 'A', n0x139ffc0.get() }, { 'B', n0x139ffc0.get() },
        { 'C', n0x139ffc0.get() }, { 'D', n0x139ffc0.get() },
        { 'E', n0x139ffc0.get() }, { 'F', n0x139ffc0.get() },
        { 'G', n0x139ffc0.get() }, { 'H', n0x139ffc0.get() },
        { 'I', n0x139ffc0.get() }, { 'J', n0x139ffc0.get() },
        { 'K', n0x139ffc0.get() }, { 'L', n0x139ffc0.get() },
        { 'M', n0x139ffc0.get() },
      };
      return std::move(n0x13605f0);
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

    auto n0x1365360 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0x1365260 =
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
    auto n0x1360710 =
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
    auto n0x1364330 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0x13652e0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0x1362950 =
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
                                         -2147483648,
                                         Assoc::NONE });
    auto n0x13629d0 =
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
                                         -2147483648,
                                         Assoc::NONE });
    auto makeDFA() {
      n0x1365360->ts_ = {
        { -1, n0x1365260.get() },
        { 1, n0x1360710.get() },
      };
      n0x1365260->ts_ = {};
      n0x1360710->ts_ = {
        { -3, n0x1364330.get() },
        { -2, n0x13652e0.get() },
      };
      n0x1364330->ts_ = {
        { -1, n0x1365260.get() },
        { 1, n0x1362950.get() },
      };
      n0x13652e0->ts_ = {
        { -1, n0x1365260.get() },
        { 1, n0x13629d0.get() },
      };
      n0x1362950->ts_ = {
        { -3, n0x1364330.get() },
        { -2, n0x13652e0.get() },
      };
      n0x13629d0->ts_ = {
        { -3, n0x1364330.get() },
        { -2, n0x13652e0.get() },
      };
      return std::move(n0x1365360);
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
    if (ruleData.precedence > shiftPrecedence ||
        (ruleData.precedence == shiftPrecedence &&
         ruleData.assoc == Assoc::LEFT)) {
      return ruleData.reducibleRule->concrete;
    }
    return NONE;
  }
  Expr* shiftReduce(vector<StackObj>& inputTokens) {
    if (inputTokens.empty()) {
      parseError({}, inputTokens, 0);
    }
    vector<StackObj> stk = { move(inputTokens[0]) };
    vector<parser::Node*> dfaPath = { parser::root.get() };
    size_t i = 1;
    size_t inputSize = inputTokens.size();
    while (!(i == inputSize && stk.size() == 1 && stk[0].symbol == S)) {
      parser::Node* currentNode = dfaPath.back()->step(stk.back().symbol);
      if (currentNode == nullptr) {
        cleanPtrsFrom(stk, 0);
        cleanPtrsFrom(inputTokens, i);
        parseError(stk, inputTokens, i);
      }
      dfaPath.push_back(currentNode);

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
            dfaPath.pop_back();
          }
        }
        stk.push_back(newObj);
      } else {
        if (i == inputSize) {
          cleanPtrsFrom(stk, 0);
          parseError(stk, inputTokens, i);
        }
        StackObj token = inputTokens[i];
        stk.push_back(move(token));
        ++i;
      }
    }

    SObj* start = (SObj*)(stk[0].obj);
    Expr* root = *start->r_;
    delete start;
    return root;
  }
}  // namespace
namespace test::parser::generated::bad_expr_parser {
  Expr* parse(const string& input) {
    vector<StackObj> stackObjs = tokenize(input);
    return shiftReduce(stackObjs);
  }
  Expr* parse(istream& input) {
    return parse(string(
        istreambuf_iterator<char>{ input }, istreambuf_iterator<char>{}));
  }
}  // namespace test::parser::generated::bad_expr_parser