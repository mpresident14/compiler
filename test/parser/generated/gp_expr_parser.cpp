/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */
#include "test/parser/generated/gp_expr_parser.hpp"

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

char c[] = "Just for testing %";
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
                                   { "MINUS", "", 1 },
                                   { "STAR", "", 2 },
                                   { "FSLASH", "", 2 },
                                   { "TILDE", "", 3 },
                                   { "LPAREN", "", -2147483648 },
                                   { "RPAREN", "", -2147483648 },
                                   { "", "", -2147483647 },
                               },
                               {
                                   { 0 },
                                   { 1 },
                                   { 1 },
                                   { 1 },
                                   { 1 },
                                   { 1 },
                                   { 1 },
                                   { 1 },
                                   { 1 },
                                   { 2 },
                               },
                               {
                                   { "S", "Start" },
                                   { "Expr", "Expr*" },
                                   { "ParenExpr", "Expr*" },
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
      case 2:
        delete (Expr**)s.obj;
        break;
      default:
        return;
    }
  }
  void deleteObj(const StackObj& s) {
    switch (s.symbol) {
      case -1:
        cerr << "INT deleter called" << endl;
        delete (int*)s.obj;
        break;
      case 1:
        delete *(Expr**)s.obj;
        delete (Expr**)s.obj;
        break;
      case 2:
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
      case 4:
        return new Expr*(
            new EBinOp(*(Expr**)args[0].obj, MINUS, *(Expr**)args[2].obj));
      case 5:
        return new Expr*(
            new EBinOp(*(Expr**)args[0].obj, DIVIDE, *(Expr**)args[2].obj));
      case 6:
        return new Expr*(new EUOp(*(Expr**)args[1].obj, BITNEG));
      case 7:
        return new Expr*(new EUOp(*(Expr**)args[1].obj, NEG));
      case 8:
        return new Expr*(*(Expr**)args[0].obj);
      case 9:
        return new Expr*(*(Expr**)args[1].obj);
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

    auto n0xa1cfe0 = std::make_unique<Node>(-2147483648);
    auto n0xa2e880 = std::make_unique<Node>(-6);
    auto n0xa1dae0 = std::make_unique<Node>(-2147483648);
    auto n0xa2d810 = std::make_unique<Node>(-1);
    auto n0xa2d5c0 = std::make_unique<Node>(-5);
    auto n0xa2d390 = std::make_unique<Node>(-3);
    auto n0xa2d1a0 = std::make_unique<Node>(-2);
    auto n0xa1fb00 = std::make_unique<Node>(-7);
    auto n0xa1f710 = std::make_unique<Node>(-8);
    auto n0xa1cb30 = std::make_unique<Node>(-4);
    auto n0xa2dbc0 = std::make_unique<Node>(-2147483648);
    auto n0xa317d0 = std::make_unique<Node>(-2147483648);
    auto n0xa33820 = std::make_unique<Node>(-9);
    auto makeDFA() {
      n0xa1cfe0->ts_ = {
        { '~', n0xa2e880.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa2d810.get() }, { '4', n0xa2d810.get() },
        { '3', n0xa2d810.get() }, { '2', n0xa2d810.get() },
        { '1', n0xa2d810.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa2d5c0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa2d390.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa2d1a0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1fb00.get() }, { ')', n0xa1f710.get() },
        { '*', n0xa1cb30.get() }, { '6', n0xa2d810.get() },
        { '7', n0xa2d810.get() }, { '8', n0xa2d810.get() },
        { '9', n0xa2d810.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa2dbc0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa2e880->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa1dae0->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa2d810->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa2d810.get() }, { '4', n0xa2d810.get() },
        { '3', n0xa2d810.get() }, { '2', n0xa2d810.get() },
        { '1', n0xa2d810.get() }, { '0', n0xa2d810.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa2d810.get() },
        { '7', n0xa2d810.get() }, { '8', n0xa2d810.get() },
        { '9', n0xa2d810.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa2d5c0->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa2d390->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa2d1a0->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa1fb00->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa1f710->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa1cb30->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa2dbc0->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa317d0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa317d0->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa33820.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      n0xa33820->ts_ = {
        { '~', n0xa1dae0.get() }, { '}', n0xa1dae0.get() },
        { '|', n0xa1dae0.get() }, { '{', n0xa1dae0.get() },
        { 'z', n0xa1dae0.get() }, { 'y', n0xa1dae0.get() },
        { 'x', n0xa1dae0.get() }, { 'w', n0xa1dae0.get() },
        { 'v', n0xa1dae0.get() }, { 'u', n0xa1dae0.get() },
        { 't', n0xa1dae0.get() }, { 's', n0xa1dae0.get() },
        { 'r', n0xa1dae0.get() }, { 'q', n0xa1dae0.get() },
        { 'p', n0xa1dae0.get() }, { 'o', n0xa1dae0.get() },
        { 'n', n0xa1dae0.get() }, { 'm', n0xa1dae0.get() },
        { 'l', n0xa1dae0.get() }, { '\n', n0xa1dae0.get() },
        { 'k', n0xa1dae0.get() }, { '\t', n0xa1dae0.get() },
        { 'j', n0xa1dae0.get() }, { 'i', n0xa1dae0.get() },
        { 'h', n0xa1dae0.get() }, { 'g', n0xa1dae0.get() },
        { 'f', n0xa1dae0.get() }, { 'e', n0xa1dae0.get() },
        { 'd', n0xa1dae0.get() }, { 'c', n0xa1dae0.get() },
        { 'b', n0xa1dae0.get() }, { 'a', n0xa1dae0.get() },
        { '`', n0xa1dae0.get() }, { '_', n0xa1dae0.get() },
        { '^', n0xa1dae0.get() }, { ']', n0xa1dae0.get() },
        { '[', n0xa1dae0.get() }, { 'Z', n0xa1dae0.get() },
        { 'Y', n0xa1dae0.get() }, { 'X', n0xa1dae0.get() },
        { 'W', n0xa1dae0.get() }, { 'V', n0xa1dae0.get() },
        { 'U', n0xa1dae0.get() }, { 'T', n0xa1dae0.get() },
        { 'S', n0xa1dae0.get() }, { 'R', n0xa1dae0.get() },
        { 'Q', n0xa1dae0.get() }, { 'P', n0xa1dae0.get() },
        { 'O', n0xa1dae0.get() }, { 'N', n0xa1dae0.get() },
        { '5', n0xa1dae0.get() }, { '4', n0xa1dae0.get() },
        { '3', n0xa1dae0.get() }, { '2', n0xa1dae0.get() },
        { '1', n0xa1dae0.get() }, { '0', n0xa1dae0.get() },
        { '/', n0xa1dae0.get() }, { '\\', n0xa1dae0.get() },
        { '-', n0xa1dae0.get() }, { '.', n0xa1dae0.get() },
        { ',', n0xa1dae0.get() }, { '+', n0xa1dae0.get() },
        { '$', n0xa1dae0.get() }, { '#', n0xa1dae0.get() },
        { '"', n0xa1dae0.get() }, { ' ', n0xa1dae0.get() },
        { '!', n0xa1dae0.get() }, { '%', n0xa1dae0.get() },
        { '&', n0xa1dae0.get() }, { '\'', n0xa1dae0.get() },
        { '(', n0xa1dae0.get() }, { ')', n0xa1dae0.get() },
        { '*', n0xa1dae0.get() }, { '6', n0xa1dae0.get() },
        { '7', n0xa1dae0.get() }, { '8', n0xa1dae0.get() },
        { '9', n0xa1dae0.get() }, { ':', n0xa1dae0.get() },
        { ';', n0xa1dae0.get() }, { '<', n0xa1dae0.get() },
        { '=', n0xa1dae0.get() }, { '>', n0xa1dae0.get() },
        { '?', n0xa1dae0.get() }, { '@', n0xa1dae0.get() },
        { 'A', n0xa1dae0.get() }, { 'B', n0xa1dae0.get() },
        { 'C', n0xa1dae0.get() }, { 'D', n0xa1dae0.get() },
        { 'E', n0xa1dae0.get() }, { 'F', n0xa1dae0.get() },
        { 'G', n0xa1dae0.get() }, { 'H', n0xa1dae0.get() },
        { 'I', n0xa1dae0.get() }, { 'J', n0xa1dae0.get() },
        { 'K', n0xa1dae0.get() }, { 'L', n0xa1dae0.get() },
        { 'M', n0xa1dae0.get() },
      };
      return std::move(n0xa1cfe0);
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

    auto n0xa25540 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa8f560 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa23160 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 0,
                                                              {
                                                                  1,
                                                              },
                                                              1,
                                                              {
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         -2147483648,
                                         Assoc::NONE });
    auto n0xa28b00 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 1,
                                                              {
                                                                  -1,
                                                              },
                                                              1,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         -2147483648,
                                         Assoc::NONE });
    auto n0xa1ea80 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa278a0 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 8,
                                                              {
                                                                  2,
                                                              },
                                                              1,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         -2147483648,
                                         Assoc::NONE });
    auto n0xa28bd0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa41730 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa27ef0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa28390 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 1,
                                                              {
                                                                  -1,
                                                              },
                                                              1,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  1,
                                                                  0,
                                                              } } },
                                         -2147483648,
                                         Assoc::NONE });
    auto n0xa3c5b0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa282c0 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 8,
                                                              {
                                                                  2,
                                                              },
                                                              1,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  1,
                                                                  0,
                                                              } } },
                                         -2147483648,
                                         Assoc::NONE });
    auto n0xa3b7a0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa93360 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa26a50 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa33ad0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa3ff60 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa2e4f0 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 6,
                                                              {
                                                                  -6,
                                                                  1,
                                                              },
                                                              2,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         3,
                                         Assoc::NOT });
    auto n0xa93c10 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 7,
                                                              {
                                                                  -3,
                                                                  1,
                                                              },
                                                              2,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         3,
                                         Assoc::NONE });
    auto n0xa3b020 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa8ec10 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa8de90 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa947c0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa39770 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 9,
                                                              {
                                                                  -7,
                                                                  1,
                                                                  -8,
                                                              },
                                                              3,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         -2147483648,
                                         Assoc::NONE });
    auto n0xa955d0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xa3a790 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 6,
                                                              {
                                                                  -6,
                                                                  1,
                                                              },
                                                              2,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  1,
                                                                  0,
                                                              } } },
                                         3,
                                         Assoc::NOT });
    auto n0xa39840 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 7,
                                                              {
                                                                  -3,
                                                                  1,
                                                              },
                                                              2,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  1,
                                                                  0,
                                                              } } },
                                         3,
                                         Assoc::NONE });
    auto n0xa22640 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 5,
                                                              {
                                                                  1,
                                                                  -5,
                                                                  1,
                                                              },
                                                              3,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         2,
                                         Assoc::LEFT });
    auto n0xa21df0 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 3,
                                                              {
                                                                  1,
                                                                  -4,
                                                                  1,
                                                              },
                                                              3,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         2,
                                         Assoc::LEFT });
    auto n0xa23f00 =
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
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         1,
                                         Assoc::LEFT });
    auto n0xa24cd0 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 4,
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
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                              } } },
                                         1,
                                         Assoc::LEFT });
    auto n0xa2f490 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 9,
                                                              {
                                                                  -7,
                                                                  1,
                                                                  -8,
                                                              },
                                                              3,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  1,
                                                                  0,
                                                              } } },
                                         -2147483648,
                                         Assoc::NONE });
    auto n0xa335d0 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 5,
                                                              {
                                                                  1,
                                                                  -5,
                                                                  1,
                                                              },
                                                              3,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  1,
                                                                  0,
                                                              } } },
                                         2,
                                         Assoc::LEFT });
    auto n0xa32d40 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 3,
                                                              {
                                                                  1,
                                                                  -4,
                                                                  1,
                                                              },
                                                              3,
                                                              {
                                                                  0,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  1,
                                                                  0,
                                                              } } },
                                         2,
                                         Assoc::LEFT });
    auto n0xa31cd0 =
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
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  1,
                                                                  0,
                                                              } } },
                                         1,
                                         Assoc::LEFT });
    auto n0xa324b0 =
        std::make_unique<Node>(RuleData{ optional<DFARule>{ { 4,
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
                                                                  1,
                                                                  1,
                                                                  0,
                                                                  0,
                                                                  1,
                                                                  0,
                                                              } } },
                                         1,
                                         Assoc::LEFT });
    auto makeDFA() {
      n0xa25540->ts_ = {
        { -7, n0xa8f560.get() }, { 1, n0xa23160.get() },
        { -1, n0xa28b00.get() }, { -6, n0xa1ea80.get() },
        { 2, n0xa278a0.get() },  { -3, n0xa28bd0.get() },
      };
      n0xa8f560->ts_ = {
        { -7, n0xa41730.get() }, { 1, n0xa27ef0.get() },
        { -1, n0xa28390.get() }, { -6, n0xa3c5b0.get() },
        { 2, n0xa282c0.get() },  { -3, n0xa3b7a0.get() },
      };
      n0xa23160->ts_ = {
        { -5, n0xa93360.get() },
        { -4, n0xa26a50.get() },
        { -2, n0xa33ad0.get() },
        { -3, n0xa3ff60.get() },
      };
      n0xa28b00->ts_ = {};
      n0xa1ea80->ts_ = {
        { -7, n0xa8f560.get() }, { 1, n0xa2e4f0.get() },
        { -1, n0xa28b00.get() }, { -6, n0xa1ea80.get() },
        { 2, n0xa278a0.get() },  { -3, n0xa28bd0.get() },
      };
      n0xa278a0->ts_ = {};
      n0xa28bd0->ts_ = {
        { -7, n0xa8f560.get() }, { 1, n0xa93c10.get() },
        { -1, n0xa28b00.get() }, { -6, n0xa1ea80.get() },
        { 2, n0xa278a0.get() },  { -3, n0xa28bd0.get() },
      };
      n0xa41730->ts_ = {
        { -7, n0xa41730.get() }, { 1, n0xa3b020.get() },
        { -1, n0xa28390.get() }, { -6, n0xa3c5b0.get() },
        { 2, n0xa282c0.get() },  { -3, n0xa3b7a0.get() },
      };
      n0xa27ef0->ts_ = {
        { -5, n0xa8ec10.get() }, { -4, n0xa8de90.get() },
        { -2, n0xa947c0.get() }, { -8, n0xa39770.get() },
        { -3, n0xa955d0.get() },
      };
      n0xa28390->ts_ = {};
      n0xa3c5b0->ts_ = {
        { -7, n0xa41730.get() }, { 1, n0xa3a790.get() },
        { -1, n0xa28390.get() }, { -6, n0xa3c5b0.get() },
        { 2, n0xa282c0.get() },  { -3, n0xa3b7a0.get() },
      };
      n0xa282c0->ts_ = {};
      n0xa3b7a0->ts_ = {
        { -7, n0xa41730.get() }, { 1, n0xa39840.get() },
        { -1, n0xa28390.get() }, { -6, n0xa3c5b0.get() },
        { 2, n0xa282c0.get() },  { -3, n0xa3b7a0.get() },
      };
      n0xa93360->ts_ = {
        { -7, n0xa8f560.get() }, { 1, n0xa22640.get() },
        { -1, n0xa28b00.get() }, { -6, n0xa1ea80.get() },
        { 2, n0xa278a0.get() },  { -3, n0xa28bd0.get() },
      };
      n0xa26a50->ts_ = {
        { -7, n0xa8f560.get() }, { 1, n0xa21df0.get() },
        { -1, n0xa28b00.get() }, { -6, n0xa1ea80.get() },
        { 2, n0xa278a0.get() },  { -3, n0xa28bd0.get() },
      };
      n0xa33ad0->ts_ = {
        { -7, n0xa8f560.get() }, { 1, n0xa23f00.get() },
        { -1, n0xa28b00.get() }, { -6, n0xa1ea80.get() },
        { 2, n0xa278a0.get() },  { -3, n0xa28bd0.get() },
      };
      n0xa3ff60->ts_ = {
        { -7, n0xa8f560.get() }, { 1, n0xa24cd0.get() },
        { -1, n0xa28b00.get() }, { -6, n0xa1ea80.get() },
        { 2, n0xa278a0.get() },  { -3, n0xa28bd0.get() },
      };
      n0xa2e4f0->ts_ = {
        { -5, n0xa93360.get() },
        { -4, n0xa26a50.get() },
        { -2, n0xa33ad0.get() },
        { -3, n0xa3ff60.get() },
      };
      n0xa93c10->ts_ = {
        { -5, n0xa93360.get() },
        { -4, n0xa26a50.get() },
        { -2, n0xa33ad0.get() },
        { -3, n0xa3ff60.get() },
      };
      n0xa3b020->ts_ = {
        { -5, n0xa8ec10.get() }, { -4, n0xa8de90.get() },
        { -2, n0xa947c0.get() }, { -8, n0xa2f490.get() },
        { -3, n0xa955d0.get() },
      };
      n0xa8ec10->ts_ = {
        { -7, n0xa41730.get() }, { 1, n0xa335d0.get() },
        { -1, n0xa28390.get() }, { -6, n0xa3c5b0.get() },
        { 2, n0xa282c0.get() },  { -3, n0xa3b7a0.get() },
      };
      n0xa8de90->ts_ = {
        { -7, n0xa41730.get() }, { 1, n0xa32d40.get() },
        { -1, n0xa28390.get() }, { -6, n0xa3c5b0.get() },
        { 2, n0xa282c0.get() },  { -3, n0xa3b7a0.get() },
      };
      n0xa947c0->ts_ = {
        { -7, n0xa41730.get() }, { 1, n0xa31cd0.get() },
        { -1, n0xa28390.get() }, { -6, n0xa3c5b0.get() },
        { 2, n0xa282c0.get() },  { -3, n0xa3b7a0.get() },
      };
      n0xa39770->ts_ = {};
      n0xa955d0->ts_ = {
        { -7, n0xa41730.get() }, { 1, n0xa324b0.get() },
        { -1, n0xa28390.get() }, { -6, n0xa3c5b0.get() },
        { 2, n0xa282c0.get() },  { -3, n0xa3b7a0.get() },
      };
      n0xa3a790->ts_ = {
        { -5, n0xa8ec10.get() },
        { -4, n0xa8de90.get() },
        { -2, n0xa947c0.get() },
        { -3, n0xa955d0.get() },
      };
      n0xa39840->ts_ = {
        { -5, n0xa8ec10.get() },
        { -4, n0xa8de90.get() },
        { -2, n0xa947c0.get() },
        { -3, n0xa955d0.get() },
      };
      n0xa22640->ts_ = {
        { -5, n0xa93360.get() },
        { -4, n0xa26a50.get() },
        { -2, n0xa33ad0.get() },
        { -3, n0xa3ff60.get() },
      };
      n0xa21df0->ts_ = {
        { -5, n0xa93360.get() },
        { -4, n0xa26a50.get() },
        { -2, n0xa33ad0.get() },
        { -3, n0xa3ff60.get() },
      };
      n0xa23f00->ts_ = {
        { -5, n0xa93360.get() },
        { -4, n0xa26a50.get() },
        { -2, n0xa33ad0.get() },
        { -3, n0xa3ff60.get() },
      };
      n0xa24cd0->ts_ = {
        { -5, n0xa93360.get() },
        { -4, n0xa26a50.get() },
        { -2, n0xa33ad0.get() },
        { -3, n0xa3ff60.get() },
      };
      n0xa2f490->ts_ = {};
      n0xa335d0->ts_ = {
        { -5, n0xa8ec10.get() },
        { -4, n0xa8de90.get() },
        { -2, n0xa947c0.get() },
        { -3, n0xa955d0.get() },
      };
      n0xa32d40->ts_ = {
        { -5, n0xa8ec10.get() },
        { -4, n0xa8de90.get() },
        { -2, n0xa947c0.get() },
        { -3, n0xa955d0.get() },
      };
      n0xa31cd0->ts_ = {
        { -5, n0xa8ec10.get() },
        { -4, n0xa8de90.get() },
        { -2, n0xa947c0.get() },
        { -3, n0xa955d0.get() },
      };
      n0xa324b0->ts_ = {
        { -5, n0xa8ec10.get() },
        { -4, n0xa8de90.get() },
        { -2, n0xa947c0.get() },
        { -3, n0xa955d0.get() },
      };
      return std::move(n0xa25540);
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
namespace test::parser::generated::gp_expr_parser {
  Expr* parse(const string& input) {
    vector<StackObj> stackObjs = tokenize(input);
    return shiftReduce(stackObjs);
  }
  Expr* parse(istream& input) {
    return parse(string(
        istreambuf_iterator<char>{ input }, istreambuf_iterator<char>{}));
  }
}  // namespace test::parser::generated::gp_expr_parser