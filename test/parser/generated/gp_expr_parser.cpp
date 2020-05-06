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

    auto n0xd2f5f0 = std::make_unique<Node>(-2147483648);
    auto n0xd38e40 = std::make_unique<Node>(-6);
    auto n0xd2f590 = std::make_unique<Node>(-2147483648);
    auto n0xd37f70 = std::make_unique<Node>(-1);
    auto n0xd37df0 = std::make_unique<Node>(-5);
    auto n0xd37c90 = std::make_unique<Node>(-3);
    auto n0xd37b30 = std::make_unique<Node>(-2);
    auto n0xd32720 = std::make_unique<Node>(-7);
    auto n0xd378b0 = std::make_unique<Node>(-8);
    auto n0xd379f0 = std::make_unique<Node>(-4);
    auto n0xd38250 = std::make_unique<Node>(-2147483648);
    auto n0xd39060 = std::make_unique<Node>(-2147483648);
    auto n0xd2ebc0 = std::make_unique<Node>(-9);
    auto makeDFA() {
      n0xd2f5f0->ts_ = {
        { '~', n0xd38e40.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd37f70.get() }, { '4', n0xd37f70.get() },
        { '3', n0xd37f70.get() }, { '2', n0xd37f70.get() },
        { '1', n0xd37f70.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd37df0.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd37c90.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd37b30.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd32720.get() }, { ')', n0xd378b0.get() },
        { '*', n0xd379f0.get() }, { '6', n0xd37f70.get() },
        { '7', n0xd37f70.get() }, { '8', n0xd37f70.get() },
        { '9', n0xd37f70.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd38250.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd38e40->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd2f590->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd37f70->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd37f70.get() }, { '4', n0xd37f70.get() },
        { '3', n0xd37f70.get() }, { '2', n0xd37f70.get() },
        { '1', n0xd37f70.get() }, { '0', n0xd37f70.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd37f70.get() },
        { '7', n0xd37f70.get() }, { '8', n0xd37f70.get() },
        { '9', n0xd37f70.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd37df0->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd37c90->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd37b30->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd32720->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd378b0->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd379f0->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd38250->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd39060.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd39060->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2ebc0.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      n0xd2ebc0->ts_ = {
        { '~', n0xd2f590.get() }, { '}', n0xd2f590.get() },
        { '|', n0xd2f590.get() }, { '{', n0xd2f590.get() },
        { 'z', n0xd2f590.get() }, { 'y', n0xd2f590.get() },
        { 'x', n0xd2f590.get() }, { 'w', n0xd2f590.get() },
        { 'v', n0xd2f590.get() }, { 'u', n0xd2f590.get() },
        { 't', n0xd2f590.get() }, { 's', n0xd2f590.get() },
        { 'r', n0xd2f590.get() }, { 'q', n0xd2f590.get() },
        { 'p', n0xd2f590.get() }, { 'o', n0xd2f590.get() },
        { 'n', n0xd2f590.get() }, { 'm', n0xd2f590.get() },
        { 'l', n0xd2f590.get() }, { '\n', n0xd2f590.get() },
        { 'k', n0xd2f590.get() }, { '\t', n0xd2f590.get() },
        { 'j', n0xd2f590.get() }, { 'i', n0xd2f590.get() },
        { 'h', n0xd2f590.get() }, { 'g', n0xd2f590.get() },
        { 'f', n0xd2f590.get() }, { 'e', n0xd2f590.get() },
        { 'd', n0xd2f590.get() }, { 'c', n0xd2f590.get() },
        { 'b', n0xd2f590.get() }, { 'a', n0xd2f590.get() },
        { '`', n0xd2f590.get() }, { '_', n0xd2f590.get() },
        { '^', n0xd2f590.get() }, { ']', n0xd2f590.get() },
        { '[', n0xd2f590.get() }, { 'Z', n0xd2f590.get() },
        { 'Y', n0xd2f590.get() }, { 'X', n0xd2f590.get() },
        { 'W', n0xd2f590.get() }, { 'V', n0xd2f590.get() },
        { 'U', n0xd2f590.get() }, { 'T', n0xd2f590.get() },
        { 'S', n0xd2f590.get() }, { 'R', n0xd2f590.get() },
        { 'Q', n0xd2f590.get() }, { 'P', n0xd2f590.get() },
        { 'O', n0xd2f590.get() }, { 'N', n0xd2f590.get() },
        { '5', n0xd2f590.get() }, { '4', n0xd2f590.get() },
        { '3', n0xd2f590.get() }, { '2', n0xd2f590.get() },
        { '1', n0xd2f590.get() }, { '0', n0xd2f590.get() },
        { '/', n0xd2f590.get() }, { '\\', n0xd2f590.get() },
        { '-', n0xd2f590.get() }, { '.', n0xd2f590.get() },
        { ',', n0xd2f590.get() }, { '+', n0xd2f590.get() },
        { '$', n0xd2f590.get() }, { '#', n0xd2f590.get() },
        { '"', n0xd2f590.get() }, { ' ', n0xd2f590.get() },
        { '!', n0xd2f590.get() }, { '%', n0xd2f590.get() },
        { '&', n0xd2f590.get() }, { '\'', n0xd2f590.get() },
        { '(', n0xd2f590.get() }, { ')', n0xd2f590.get() },
        { '*', n0xd2f590.get() }, { '6', n0xd2f590.get() },
        { '7', n0xd2f590.get() }, { '8', n0xd2f590.get() },
        { '9', n0xd2f590.get() }, { ':', n0xd2f590.get() },
        { ';', n0xd2f590.get() }, { '<', n0xd2f590.get() },
        { '=', n0xd2f590.get() }, { '>', n0xd2f590.get() },
        { '?', n0xd2f590.get() }, { '@', n0xd2f590.get() },
        { 'A', n0xd2f590.get() }, { 'B', n0xd2f590.get() },
        { 'C', n0xd2f590.get() }, { 'D', n0xd2f590.get() },
        { 'E', n0xd2f590.get() }, { 'F', n0xd2f590.get() },
        { 'G', n0xd2f590.get() }, { 'H', n0xd2f590.get() },
        { 'I', n0xd2f590.get() }, { 'J', n0xd2f590.get() },
        { 'K', n0xd2f590.get() }, { 'L', n0xd2f590.get() },
        { 'M', n0xd2f590.get() },
      };
      return std::move(n0xd2f5f0);
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

    auto n0xd326a0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd706f0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd30d40 =
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
    auto n0xd33650 =
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
    auto n0xd2ea80 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd335d0 =
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
    auto n0xd336d0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd36b80 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd2d2d0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd2efa0 =
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
    auto n0xd36260 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd2d350 =
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
    auto n0xd71db0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd685e0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd32220 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd2e060 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd33220 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd2d6c0 =
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
    auto n0xd376f0 =
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
    auto n0xd69fd0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd52a40 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd38af0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd47640 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd52ac0 =
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
    auto n0xd38450 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0xd69ab0 =
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
    auto n0xd52f00 =
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
    auto n0xd6ea20 =
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
    auto n0xd6e520 =
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
    auto n0xd37020 =
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
    auto n0xd658f0 =
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
    auto n0xd486e0 =
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
    auto n0xd46dd0 =
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
    auto n0xd468f0 =
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
    auto n0xd46010 =
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
    auto n0xd46410 =
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
      n0xd326a0->ts_ = {
        { -7, n0xd706f0.get() }, { 1, n0xd30d40.get() },
        { -1, n0xd33650.get() }, { -6, n0xd2ea80.get() },
        { 2, n0xd335d0.get() },  { -3, n0xd336d0.get() },
      };
      n0xd706f0->ts_ = {
        { -7, n0xd36b80.get() }, { 1, n0xd2d2d0.get() },
        { -1, n0xd2efa0.get() }, { -6, n0xd36260.get() },
        { 2, n0xd2d350.get() },  { -3, n0xd71db0.get() },
      };
      n0xd30d40->ts_ = {
        { -5, n0xd685e0.get() },
        { -4, n0xd32220.get() },
        { -2, n0xd2e060.get() },
        { -3, n0xd33220.get() },
      };
      n0xd33650->ts_ = {};
      n0xd2ea80->ts_ = {
        { -7, n0xd706f0.get() }, { 1, n0xd2d6c0.get() },
        { -1, n0xd33650.get() }, { -6, n0xd2ea80.get() },
        { 2, n0xd335d0.get() },  { -3, n0xd336d0.get() },
      };
      n0xd335d0->ts_ = {};
      n0xd336d0->ts_ = {
        { -7, n0xd706f0.get() }, { 1, n0xd376f0.get() },
        { -1, n0xd33650.get() }, { -6, n0xd2ea80.get() },
        { 2, n0xd335d0.get() },  { -3, n0xd336d0.get() },
      };
      n0xd36b80->ts_ = {
        { -7, n0xd36b80.get() }, { 1, n0xd69fd0.get() },
        { -1, n0xd2efa0.get() }, { -6, n0xd36260.get() },
        { 2, n0xd2d350.get() },  { -3, n0xd71db0.get() },
      };
      n0xd2d2d0->ts_ = {
        { -5, n0xd52a40.get() }, { -4, n0xd38af0.get() },
        { -2, n0xd47640.get() }, { -8, n0xd52ac0.get() },
        { -3, n0xd38450.get() },
      };
      n0xd2efa0->ts_ = {};
      n0xd36260->ts_ = {
        { -7, n0xd36b80.get() }, { 1, n0xd69ab0.get() },
        { -1, n0xd2efa0.get() }, { -6, n0xd36260.get() },
        { 2, n0xd2d350.get() },  { -3, n0xd71db0.get() },
      };
      n0xd2d350->ts_ = {};
      n0xd71db0->ts_ = {
        { -7, n0xd36b80.get() }, { 1, n0xd52f00.get() },
        { -1, n0xd2efa0.get() }, { -6, n0xd36260.get() },
        { 2, n0xd2d350.get() },  { -3, n0xd71db0.get() },
      };
      n0xd685e0->ts_ = {
        { -7, n0xd706f0.get() }, { 1, n0xd6ea20.get() },
        { -1, n0xd33650.get() }, { -6, n0xd2ea80.get() },
        { 2, n0xd335d0.get() },  { -3, n0xd336d0.get() },
      };
      n0xd32220->ts_ = {
        { -7, n0xd706f0.get() }, { 1, n0xd6e520.get() },
        { -1, n0xd33650.get() }, { -6, n0xd2ea80.get() },
        { 2, n0xd335d0.get() },  { -3, n0xd336d0.get() },
      };
      n0xd2e060->ts_ = {
        { -7, n0xd706f0.get() }, { 1, n0xd37020.get() },
        { -1, n0xd33650.get() }, { -6, n0xd2ea80.get() },
        { 2, n0xd335d0.get() },  { -3, n0xd336d0.get() },
      };
      n0xd33220->ts_ = {
        { -7, n0xd706f0.get() }, { 1, n0xd658f0.get() },
        { -1, n0xd33650.get() }, { -6, n0xd2ea80.get() },
        { 2, n0xd335d0.get() },  { -3, n0xd336d0.get() },
      };
      n0xd2d6c0->ts_ = {
        { -5, n0xd685e0.get() },
        { -4, n0xd32220.get() },
        { -2, n0xd2e060.get() },
        { -3, n0xd33220.get() },
      };
      n0xd376f0->ts_ = {
        { -5, n0xd685e0.get() },
        { -4, n0xd32220.get() },
        { -2, n0xd2e060.get() },
        { -3, n0xd33220.get() },
      };
      n0xd69fd0->ts_ = {
        { -5, n0xd52a40.get() }, { -4, n0xd38af0.get() },
        { -2, n0xd47640.get() }, { -8, n0xd486e0.get() },
        { -3, n0xd38450.get() },
      };
      n0xd52a40->ts_ = {
        { -7, n0xd36b80.get() }, { 1, n0xd46dd0.get() },
        { -1, n0xd2efa0.get() }, { -6, n0xd36260.get() },
        { 2, n0xd2d350.get() },  { -3, n0xd71db0.get() },
      };
      n0xd38af0->ts_ = {
        { -7, n0xd36b80.get() }, { 1, n0xd468f0.get() },
        { -1, n0xd2efa0.get() }, { -6, n0xd36260.get() },
        { 2, n0xd2d350.get() },  { -3, n0xd71db0.get() },
      };
      n0xd47640->ts_ = {
        { -7, n0xd36b80.get() }, { 1, n0xd46010.get() },
        { -1, n0xd2efa0.get() }, { -6, n0xd36260.get() },
        { 2, n0xd2d350.get() },  { -3, n0xd71db0.get() },
      };
      n0xd52ac0->ts_ = {};
      n0xd38450->ts_ = {
        { -7, n0xd36b80.get() }, { 1, n0xd46410.get() },
        { -1, n0xd2efa0.get() }, { -6, n0xd36260.get() },
        { 2, n0xd2d350.get() },  { -3, n0xd71db0.get() },
      };
      n0xd69ab0->ts_ = {
        { -5, n0xd52a40.get() },
        { -4, n0xd38af0.get() },
        { -2, n0xd47640.get() },
        { -3, n0xd38450.get() },
      };
      n0xd52f00->ts_ = {
        { -5, n0xd52a40.get() },
        { -4, n0xd38af0.get() },
        { -2, n0xd47640.get() },
        { -3, n0xd38450.get() },
      };
      n0xd6ea20->ts_ = {
        { -5, n0xd685e0.get() },
        { -4, n0xd32220.get() },
        { -2, n0xd2e060.get() },
        { -3, n0xd33220.get() },
      };
      n0xd6e520->ts_ = {
        { -5, n0xd685e0.get() },
        { -4, n0xd32220.get() },
        { -2, n0xd2e060.get() },
        { -3, n0xd33220.get() },
      };
      n0xd37020->ts_ = {
        { -5, n0xd685e0.get() },
        { -4, n0xd32220.get() },
        { -2, n0xd2e060.get() },
        { -3, n0xd33220.get() },
      };
      n0xd658f0->ts_ = {
        { -5, n0xd685e0.get() },
        { -4, n0xd32220.get() },
        { -2, n0xd2e060.get() },
        { -3, n0xd33220.get() },
      };
      n0xd486e0->ts_ = {};
      n0xd46dd0->ts_ = {
        { -5, n0xd52a40.get() },
        { -4, n0xd38af0.get() },
        { -2, n0xd47640.get() },
        { -3, n0xd38450.get() },
      };
      n0xd468f0->ts_ = {
        { -5, n0xd52a40.get() },
        { -4, n0xd38af0.get() },
        { -2, n0xd47640.get() },
        { -3, n0xd38450.get() },
      };
      n0xd46010->ts_ = {
        { -5, n0xd52a40.get() },
        { -4, n0xd38af0.get() },
        { -2, n0xd47640.get() },
        { -3, n0xd38450.get() },
      };
      n0xd46410->ts_ = {
        { -5, n0xd52a40.get() },
        { -4, n0xd38af0.get() },
        { -2, n0xd47640.get() },
        { -3, n0xd38450.get() },
      };
      return std::move(n0xd326a0);
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