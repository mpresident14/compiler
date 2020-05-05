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

    auto n0xd48400 = std::make_unique<Node>(-2147483648);
    auto n0xd670a0 = std::make_unique<Node>(-6);
    auto n0xd48f00 = std::make_unique<Node>(-2147483648);
    auto n0xd57970 = std::make_unique<Node>(-1);
    auto n0xd57720 = std::make_unique<Node>(-5);
    auto n0xd574f0 = std::make_unique<Node>(-3);
    auto n0xd57300 = std::make_unique<Node>(-2);
    auto n0xd4af20 = std::make_unique<Node>(-7);
    auto n0xd4ab30 = std::make_unique<Node>(-8);
    auto n0xd47f50 = std::make_unique<Node>(-4);
    auto n0xd57d20 = std::make_unique<Node>(-2147483648);
    auto n0xdbf7d0 = std::make_unique<Node>(-2147483648);
    auto n0xd5a200 = std::make_unique<Node>(-9);
    auto makeDFA() {
      n0xd48400->ts_ = {
        { '~', n0xd670a0.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd57970.get() }, { '4', n0xd57970.get() },
        { '3', n0xd57970.get() }, { '2', n0xd57970.get() },
        { '1', n0xd57970.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd57720.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd574f0.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd57300.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd4af20.get() }, { ')', n0xd4ab30.get() },
        { '*', n0xd47f50.get() }, { '6', n0xd57970.get() },
        { '7', n0xd57970.get() }, { '8', n0xd57970.get() },
        { '9', n0xd57970.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd57d20.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd670a0->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd48f00->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd57970->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd57970.get() }, { '4', n0xd57970.get() },
        { '3', n0xd57970.get() }, { '2', n0xd57970.get() },
        { '1', n0xd57970.get() }, { '0', n0xd57970.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd57970.get() },
        { '7', n0xd57970.get() }, { '8', n0xd57970.get() },
        { '9', n0xd57970.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd57720->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd574f0->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd57300->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd4af20->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd4ab30->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd47f50->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd57d20->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xdbf7d0.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xdbf7d0->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd5a200.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      n0xd5a200->ts_ = {
        { '~', n0xd48f00.get() }, { '}', n0xd48f00.get() },
        { '|', n0xd48f00.get() }, { '{', n0xd48f00.get() },
        { 'z', n0xd48f00.get() }, { 'y', n0xd48f00.get() },
        { 'x', n0xd48f00.get() }, { 'w', n0xd48f00.get() },
        { 'v', n0xd48f00.get() }, { 'u', n0xd48f00.get() },
        { 't', n0xd48f00.get() }, { 's', n0xd48f00.get() },
        { 'r', n0xd48f00.get() }, { 'q', n0xd48f00.get() },
        { 'p', n0xd48f00.get() }, { 'o', n0xd48f00.get() },
        { 'n', n0xd48f00.get() }, { 'm', n0xd48f00.get() },
        { 'l', n0xd48f00.get() }, { '\n', n0xd48f00.get() },
        { 'k', n0xd48f00.get() }, { '\t', n0xd48f00.get() },
        { 'j', n0xd48f00.get() }, { 'i', n0xd48f00.get() },
        { 'h', n0xd48f00.get() }, { 'g', n0xd48f00.get() },
        { 'f', n0xd48f00.get() }, { 'e', n0xd48f00.get() },
        { 'd', n0xd48f00.get() }, { 'c', n0xd48f00.get() },
        { 'b', n0xd48f00.get() }, { 'a', n0xd48f00.get() },
        { '`', n0xd48f00.get() }, { '_', n0xd48f00.get() },
        { '^', n0xd48f00.get() }, { ']', n0xd48f00.get() },
        { '[', n0xd48f00.get() }, { 'Z', n0xd48f00.get() },
        { 'Y', n0xd48f00.get() }, { 'X', n0xd48f00.get() },
        { 'W', n0xd48f00.get() }, { 'V', n0xd48f00.get() },
        { 'U', n0xd48f00.get() }, { 'T', n0xd48f00.get() },
        { 'S', n0xd48f00.get() }, { 'R', n0xd48f00.get() },
        { 'Q', n0xd48f00.get() }, { 'P', n0xd48f00.get() },
        { 'O', n0xd48f00.get() }, { 'N', n0xd48f00.get() },
        { '5', n0xd48f00.get() }, { '4', n0xd48f00.get() },
        { '3', n0xd48f00.get() }, { '2', n0xd48f00.get() },
        { '1', n0xd48f00.get() }, { '0', n0xd48f00.get() },
        { '/', n0xd48f00.get() }, { '\\', n0xd48f00.get() },
        { '-', n0xd48f00.get() }, { '.', n0xd48f00.get() },
        { ',', n0xd48f00.get() }, { '+', n0xd48f00.get() },
        { '$', n0xd48f00.get() }, { '#', n0xd48f00.get() },
        { '"', n0xd48f00.get() }, { ' ', n0xd48f00.get() },
        { '!', n0xd48f00.get() }, { '%', n0xd48f00.get() },
        { '&', n0xd48f00.get() }, { '\'', n0xd48f00.get() },
        { '(', n0xd48f00.get() }, { ')', n0xd48f00.get() },
        { '*', n0xd48f00.get() }, { '6', n0xd48f00.get() },
        { '7', n0xd48f00.get() }, { '8', n0xd48f00.get() },
        { '9', n0xd48f00.get() }, { ':', n0xd48f00.get() },
        { ';', n0xd48f00.get() }, { '<', n0xd48f00.get() },
        { '=', n0xd48f00.get() }, { '>', n0xd48f00.get() },
        { '?', n0xd48f00.get() }, { '@', n0xd48f00.get() },
        { 'A', n0xd48f00.get() }, { 'B', n0xd48f00.get() },
        { 'C', n0xd48f00.get() }, { 'D', n0xd48f00.get() },
        { 'E', n0xd48f00.get() }, { 'F', n0xd48f00.get() },
        { 'G', n0xd48f00.get() }, { 'H', n0xd48f00.get() },
        { 'I', n0xd48f00.get() }, { 'J', n0xd48f00.get() },
        { 'K', n0xd48f00.get() }, { 'L', n0xd48f00.get() },
        { 'M', n0xd48f00.get() },
      };
      return std::move(n0xd48400);
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

    auto n0xd50960 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd56f60 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd4e580 =
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
    auto n0xd53f20 =
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
    auto n0xd49ea0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd52cc0 =
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
    auto n0xd53ff0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd4f3a0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd567c0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd53040 =
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
    auto n0xd6c720 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd52f70 =
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
    auto n0xd53850 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd5a770 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xdbaa00 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd6bbf0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xdc1a30 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd55f70 =
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
    auto n0xd522a0 =
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
    auto n0xdb9cb0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd54610 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xdc0b70 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd65f50 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd54ec0 =
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
    auto n0xdbfde0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xdb9430 =
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
    auto n0xd54f90 =
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
    auto n0xd4e410 =
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
    auto n0xd4dbe0 =
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
    auto n0xd4fc60 =
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
    auto n0xd4d390 =
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
    auto n0xd58dc0 =
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
    auto n0xd5c7f0 =
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
    auto n0xd5bf60 =
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
    auto n0xd5aef0 =
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
    auto n0xd5b6d0 =
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
      n0xd50960->ts_ = {
        { -7, n0xd56f60.get() }, { 1, n0xd4e580.get() },
        { -1, n0xd53f20.get() }, { -6, n0xd49ea0.get() },
        { 2, n0xd52cc0.get() },  { -3, n0xd53ff0.get() },
      };
      n0xd56f60->ts_ = {
        { -7, n0xd4f3a0.get() }, { 1, n0xd567c0.get() },
        { -1, n0xd53040.get() }, { -6, n0xd6c720.get() },
        { 2, n0xd52f70.get() },  { -3, n0xd53850.get() },
      };
      n0xd4e580->ts_ = {
        { -5, n0xd5a770.get() },
        { -4, n0xdbaa00.get() },
        { -2, n0xd6bbf0.get() },
        { -3, n0xdc1a30.get() },
      };
      n0xd53f20->ts_ = {};
      n0xd49ea0->ts_ = {
        { -7, n0xd56f60.get() }, { 1, n0xd55f70.get() },
        { -1, n0xd53f20.get() }, { -6, n0xd49ea0.get() },
        { 2, n0xd52cc0.get() },  { -3, n0xd53ff0.get() },
      };
      n0xd52cc0->ts_ = {};
      n0xd53ff0->ts_ = {
        { -7, n0xd56f60.get() }, { 1, n0xd522a0.get() },
        { -1, n0xd53f20.get() }, { -6, n0xd49ea0.get() },
        { 2, n0xd52cc0.get() },  { -3, n0xd53ff0.get() },
      };
      n0xd4f3a0->ts_ = {
        { -7, n0xd4f3a0.get() }, { 1, n0xdb9cb0.get() },
        { -1, n0xd53040.get() }, { -6, n0xd6c720.get() },
        { 2, n0xd52f70.get() },  { -3, n0xd53850.get() },
      };
      n0xd567c0->ts_ = {
        { -5, n0xd54610.get() }, { -4, n0xdc0b70.get() },
        { -2, n0xd65f50.get() }, { -8, n0xd54ec0.get() },
        { -3, n0xdbfde0.get() },
      };
      n0xd53040->ts_ = {};
      n0xd6c720->ts_ = {
        { -7, n0xd4f3a0.get() }, { 1, n0xdb9430.get() },
        { -1, n0xd53040.get() }, { -6, n0xd6c720.get() },
        { 2, n0xd52f70.get() },  { -3, n0xd53850.get() },
      };
      n0xd52f70->ts_ = {};
      n0xd53850->ts_ = {
        { -7, n0xd4f3a0.get() }, { 1, n0xd54f90.get() },
        { -1, n0xd53040.get() }, { -6, n0xd6c720.get() },
        { 2, n0xd52f70.get() },  { -3, n0xd53850.get() },
      };
      n0xd5a770->ts_ = {
        { -7, n0xd56f60.get() }, { 1, n0xd4e410.get() },
        { -1, n0xd53f20.get() }, { -6, n0xd49ea0.get() },
        { 2, n0xd52cc0.get() },  { -3, n0xd53ff0.get() },
      };
      n0xdbaa00->ts_ = {
        { -7, n0xd56f60.get() }, { 1, n0xd4dbe0.get() },
        { -1, n0xd53f20.get() }, { -6, n0xd49ea0.get() },
        { 2, n0xd52cc0.get() },  { -3, n0xd53ff0.get() },
      };
      n0xd6bbf0->ts_ = {
        { -7, n0xd56f60.get() }, { 1, n0xd4fc60.get() },
        { -1, n0xd53f20.get() }, { -6, n0xd49ea0.get() },
        { 2, n0xd52cc0.get() },  { -3, n0xd53ff0.get() },
      };
      n0xdc1a30->ts_ = {
        { -7, n0xd56f60.get() }, { 1, n0xd4d390.get() },
        { -1, n0xd53f20.get() }, { -6, n0xd49ea0.get() },
        { 2, n0xd52cc0.get() },  { -3, n0xd53ff0.get() },
      };
      n0xd55f70->ts_ = {
        { -5, n0xd5a770.get() },
        { -4, n0xdbaa00.get() },
        { -2, n0xd6bbf0.get() },
        { -3, n0xdc1a30.get() },
      };
      n0xd522a0->ts_ = {
        { -5, n0xd5a770.get() },
        { -4, n0xdbaa00.get() },
        { -2, n0xd6bbf0.get() },
        { -3, n0xdc1a30.get() },
      };
      n0xdb9cb0->ts_ = {
        { -5, n0xd54610.get() }, { -4, n0xdc0b70.get() },
        { -2, n0xd65f50.get() }, { -8, n0xd58dc0.get() },
        { -3, n0xdbfde0.get() },
      };
      n0xd54610->ts_ = {
        { -7, n0xd4f3a0.get() }, { 1, n0xd5c7f0.get() },
        { -1, n0xd53040.get() }, { -6, n0xd6c720.get() },
        { 2, n0xd52f70.get() },  { -3, n0xd53850.get() },
      };
      n0xdc0b70->ts_ = {
        { -7, n0xd4f3a0.get() }, { 1, n0xd5bf60.get() },
        { -1, n0xd53040.get() }, { -6, n0xd6c720.get() },
        { 2, n0xd52f70.get() },  { -3, n0xd53850.get() },
      };
      n0xd65f50->ts_ = {
        { -7, n0xd4f3a0.get() }, { 1, n0xd5aef0.get() },
        { -1, n0xd53040.get() }, { -6, n0xd6c720.get() },
        { 2, n0xd52f70.get() },  { -3, n0xd53850.get() },
      };
      n0xd54ec0->ts_ = {};
      n0xdbfde0->ts_ = {
        { -7, n0xd4f3a0.get() }, { 1, n0xd5b6d0.get() },
        { -1, n0xd53040.get() }, { -6, n0xd6c720.get() },
        { 2, n0xd52f70.get() },  { -3, n0xd53850.get() },
      };
      n0xdb9430->ts_ = {
        { -5, n0xd54610.get() },
        { -4, n0xdc0b70.get() },
        { -2, n0xd65f50.get() },
        { -3, n0xdbfde0.get() },
      };
      n0xd54f90->ts_ = {
        { -5, n0xd54610.get() },
        { -4, n0xdc0b70.get() },
        { -2, n0xd65f50.get() },
        { -3, n0xdbfde0.get() },
      };
      n0xd4e410->ts_ = {
        { -5, n0xd5a770.get() },
        { -4, n0xdbaa00.get() },
        { -2, n0xd6bbf0.get() },
        { -3, n0xdc1a30.get() },
      };
      n0xd4dbe0->ts_ = {
        { -5, n0xd5a770.get() },
        { -4, n0xdbaa00.get() },
        { -2, n0xd6bbf0.get() },
        { -3, n0xdc1a30.get() },
      };
      n0xd4fc60->ts_ = {
        { -5, n0xd5a770.get() },
        { -4, n0xdbaa00.get() },
        { -2, n0xd6bbf0.get() },
        { -3, n0xdc1a30.get() },
      };
      n0xd4d390->ts_ = {
        { -5, n0xd5a770.get() },
        { -4, n0xdbaa00.get() },
        { -2, n0xd6bbf0.get() },
        { -3, n0xdc1a30.get() },
      };
      n0xd58dc0->ts_ = {};
      n0xd5c7f0->ts_ = {
        { -5, n0xd54610.get() },
        { -4, n0xdc0b70.get() },
        { -2, n0xd65f50.get() },
        { -3, n0xdbfde0.get() },
      };
      n0xd5bf60->ts_ = {
        { -5, n0xd54610.get() },
        { -4, n0xdc0b70.get() },
        { -2, n0xd65f50.get() },
        { -3, n0xdbfde0.get() },
      };
      n0xd5aef0->ts_ = {
        { -5, n0xd54610.get() },
        { -4, n0xdc0b70.get() },
        { -2, n0xd65f50.get() },
        { -3, n0xdbfde0.get() },
      };
      n0xd5b6d0->ts_ = {
        { -5, n0xd54610.get() },
        { -4, n0xdc0b70.get() },
        { -2, n0xd65f50.get() },
        { -3, n0xdbfde0.get() },
      };
      return std::move(n0xd50960);
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