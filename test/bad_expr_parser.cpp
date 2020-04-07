#include "test/bad_expr_parser.hpp"

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
  };
  struct SObj {
    SObj(Expr** r) : r_(r) {}
    ~SObj() { delete r_; }
    Expr** r_;
  };
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
        return new Expr*(new EPlus(*(Expr**)args[0].obj, *(Expr**)args[2].obj));
      case 3:
        return new Expr*(
            new ETimes(*(Expr**)args[0].obj, *(Expr**)args[2].obj));
      case 0:
        return new SObj((Expr**)args[0].obj);
      default:
        throw invalid_argument("Can't construct. Out of options.");
    }
  }
  StackObj construct(int concrete, StackObj* args, int varType) {
    return StackObj{ constructObj(concrete, args), varType };
  }
  StackObj constructTokenObj(int token, const string_view& str) {
    switch (token) {
      case -1:
        return { new int(stoi(string(str))), token };
        break;
      default:
        return { nullptr, token };
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

    auto n0x753d20 = std::make_unique<Node>(-2147483648);
    auto n0x753a30 = std::make_unique<Node>(-2147483648);
    auto n0x754050 = std::make_unique<Node>(-1);
    auto n0x754e10 = std::make_unique<Node>(-2);
    auto n0x754770 = std::make_unique<Node>(-3);
    auto makeDFA() {
      n0x753d20->ts_ = {
        { '~', n0x753a30.get() }, { '}', n0x753a30.get() },
        { '|', n0x753a30.get() }, { '{', n0x753a30.get() },
        { 'z', n0x753a30.get() }, { 'y', n0x753a30.get() },
        { 'x', n0x753a30.get() }, { 'w', n0x753a30.get() },
        { 'v', n0x753a30.get() }, { 'u', n0x753a30.get() },
        { 't', n0x753a30.get() }, { 's', n0x753a30.get() },
        { 'r', n0x753a30.get() }, { 'q', n0x753a30.get() },
        { 'p', n0x753a30.get() }, { 'o', n0x753a30.get() },
        { 'n', n0x753a30.get() }, { 'm', n0x753a30.get() },
        { 'l', n0x753a30.get() }, { 'k', n0x753a30.get() },
        { 'j', n0x753a30.get() }, { 'i', n0x753a30.get() },
        { 'h', n0x753a30.get() }, { 'g', n0x753a30.get() },
        { 'f', n0x753a30.get() }, { 'e', n0x753a30.get() },
        { 'd', n0x753a30.get() }, { 'c', n0x753a30.get() },
        { 'b', n0x753a30.get() }, { 'a', n0x753a30.get() },
        { '`', n0x753a30.get() }, { '_', n0x753a30.get() },
        { '^', n0x753a30.get() }, { ']', n0x753a30.get() },
        { '[', n0x753a30.get() }, { 'Z', n0x753a30.get() },
        { 'Y', n0x753a30.get() }, { 'X', n0x753a30.get() },
        { 'W', n0x753a30.get() }, { 'V', n0x753a30.get() },
        { 'U', n0x753a30.get() }, { 'T', n0x753a30.get() },
        { 'S', n0x753a30.get() }, { 'R', n0x753a30.get() },
        { 'Q', n0x753a30.get() }, { 'P', n0x753a30.get() },
        { 'O', n0x753a30.get() }, { 'N', n0x753a30.get() },
        { '5', n0x754050.get() }, { '4', n0x754050.get() },
        { '3', n0x754050.get() }, { '2', n0x754050.get() },
        { '1', n0x754050.get() }, { '0', n0x753a30.get() },
        { '/', n0x753a30.get() }, { '\\', n0x753a30.get() },
        { '-', n0x753a30.get() }, { '.', n0x753a30.get() },
        { ',', n0x753a30.get() }, { '+', n0x754e10.get() },
        { '$', n0x753a30.get() }, { '#', n0x753a30.get() },
        { '"', n0x753a30.get() }, { ' ', n0x753a30.get() },
        { '!', n0x753a30.get() }, { '%', n0x753a30.get() },
        { '&', n0x753a30.get() }, { '\'', n0x753a30.get() },
        { '(', n0x753a30.get() }, { ')', n0x753a30.get() },
        { '*', n0x754770.get() }, { '6', n0x754050.get() },
        { '7', n0x754050.get() }, { '8', n0x754050.get() },
        { '9', n0x754050.get() }, { ':', n0x753a30.get() },
        { ';', n0x753a30.get() }, { '<', n0x753a30.get() },
        { '=', n0x753a30.get() }, { '>', n0x753a30.get() },
        { '?', n0x753a30.get() }, { '@', n0x753a30.get() },
        { 'A', n0x753a30.get() }, { 'B', n0x753a30.get() },
        { 'C', n0x753a30.get() }, { 'D', n0x753a30.get() },
        { 'E', n0x753a30.get() }, { 'F', n0x753a30.get() },
        { 'G', n0x753a30.get() }, { 'H', n0x753a30.get() },
        { 'I', n0x753a30.get() }, { 'J', n0x753a30.get() },
        { 'K', n0x753a30.get() }, { 'L', n0x753a30.get() },
        { 'M', n0x753a30.get() },
      };
      n0x753a30->ts_ = {
        { '~', n0x753a30.get() }, { '}', n0x753a30.get() },
        { '|', n0x753a30.get() }, { '{', n0x753a30.get() },
        { 'z', n0x753a30.get() }, { 'y', n0x753a30.get() },
        { 'x', n0x753a30.get() }, { 'w', n0x753a30.get() },
        { 'v', n0x753a30.get() }, { 'u', n0x753a30.get() },
        { 't', n0x753a30.get() }, { 's', n0x753a30.get() },
        { 'r', n0x753a30.get() }, { 'q', n0x753a30.get() },
        { 'p', n0x753a30.get() }, { 'o', n0x753a30.get() },
        { 'n', n0x753a30.get() }, { 'm', n0x753a30.get() },
        { 'l', n0x753a30.get() }, { 'k', n0x753a30.get() },
        { 'j', n0x753a30.get() }, { 'i', n0x753a30.get() },
        { 'h', n0x753a30.get() }, { 'g', n0x753a30.get() },
        { 'f', n0x753a30.get() }, { 'e', n0x753a30.get() },
        { 'd', n0x753a30.get() }, { 'c', n0x753a30.get() },
        { 'b', n0x753a30.get() }, { 'a', n0x753a30.get() },
        { '`', n0x753a30.get() }, { '_', n0x753a30.get() },
        { '^', n0x753a30.get() }, { ']', n0x753a30.get() },
        { '[', n0x753a30.get() }, { 'Z', n0x753a30.get() },
        { 'Y', n0x753a30.get() }, { 'X', n0x753a30.get() },
        { 'W', n0x753a30.get() }, { 'V', n0x753a30.get() },
        { 'U', n0x753a30.get() }, { 'T', n0x753a30.get() },
        { 'S', n0x753a30.get() }, { 'R', n0x753a30.get() },
        { 'Q', n0x753a30.get() }, { 'P', n0x753a30.get() },
        { 'O', n0x753a30.get() }, { 'N', n0x753a30.get() },
        { '5', n0x753a30.get() }, { '4', n0x753a30.get() },
        { '3', n0x753a30.get() }, { '2', n0x753a30.get() },
        { '1', n0x753a30.get() }, { '0', n0x753a30.get() },
        { '/', n0x753a30.get() }, { '\\', n0x753a30.get() },
        { '-', n0x753a30.get() }, { '.', n0x753a30.get() },
        { ',', n0x753a30.get() }, { '+', n0x753a30.get() },
        { '$', n0x753a30.get() }, { '#', n0x753a30.get() },
        { '"', n0x753a30.get() }, { ' ', n0x753a30.get() },
        { '!', n0x753a30.get() }, { '%', n0x753a30.get() },
        { '&', n0x753a30.get() }, { '\'', n0x753a30.get() },
        { '(', n0x753a30.get() }, { ')', n0x753a30.get() },
        { '*', n0x753a30.get() }, { '6', n0x753a30.get() },
        { '7', n0x753a30.get() }, { '8', n0x753a30.get() },
        { '9', n0x753a30.get() }, { ':', n0x753a30.get() },
        { ';', n0x753a30.get() }, { '<', n0x753a30.get() },
        { '=', n0x753a30.get() }, { '>', n0x753a30.get() },
        { '?', n0x753a30.get() }, { '@', n0x753a30.get() },
        { 'A', n0x753a30.get() }, { 'B', n0x753a30.get() },
        { 'C', n0x753a30.get() }, { 'D', n0x753a30.get() },
        { 'E', n0x753a30.get() }, { 'F', n0x753a30.get() },
        { 'G', n0x753a30.get() }, { 'H', n0x753a30.get() },
        { 'I', n0x753a30.get() }, { 'J', n0x753a30.get() },
        { 'K', n0x753a30.get() }, { 'L', n0x753a30.get() },
        { 'M', n0x753a30.get() },
      };
      n0x754050->ts_ = {
        { '~', n0x753a30.get() }, { '}', n0x753a30.get() },
        { '|', n0x753a30.get() }, { '{', n0x753a30.get() },
        { 'z', n0x753a30.get() }, { 'y', n0x753a30.get() },
        { 'x', n0x753a30.get() }, { 'w', n0x753a30.get() },
        { 'v', n0x753a30.get() }, { 'u', n0x753a30.get() },
        { 't', n0x753a30.get() }, { 's', n0x753a30.get() },
        { 'r', n0x753a30.get() }, { 'q', n0x753a30.get() },
        { 'p', n0x753a30.get() }, { 'o', n0x753a30.get() },
        { 'n', n0x753a30.get() }, { 'm', n0x753a30.get() },
        { 'l', n0x753a30.get() }, { 'k', n0x753a30.get() },
        { 'j', n0x753a30.get() }, { 'i', n0x753a30.get() },
        { 'h', n0x753a30.get() }, { 'g', n0x753a30.get() },
        { 'f', n0x753a30.get() }, { 'e', n0x753a30.get() },
        { 'd', n0x753a30.get() }, { 'c', n0x753a30.get() },
        { 'b', n0x753a30.get() }, { 'a', n0x753a30.get() },
        { '`', n0x753a30.get() }, { '_', n0x753a30.get() },
        { '^', n0x753a30.get() }, { ']', n0x753a30.get() },
        { '[', n0x753a30.get() }, { 'Z', n0x753a30.get() },
        { 'Y', n0x753a30.get() }, { 'X', n0x753a30.get() },
        { 'W', n0x753a30.get() }, { 'V', n0x753a30.get() },
        { 'U', n0x753a30.get() }, { 'T', n0x753a30.get() },
        { 'S', n0x753a30.get() }, { 'R', n0x753a30.get() },
        { 'Q', n0x753a30.get() }, { 'P', n0x753a30.get() },
        { 'O', n0x753a30.get() }, { 'N', n0x753a30.get() },
        { '5', n0x754050.get() }, { '4', n0x754050.get() },
        { '3', n0x754050.get() }, { '2', n0x754050.get() },
        { '1', n0x754050.get() }, { '0', n0x754050.get() },
        { '/', n0x753a30.get() }, { '\\', n0x753a30.get() },
        { '-', n0x753a30.get() }, { '.', n0x753a30.get() },
        { ',', n0x753a30.get() }, { '+', n0x753a30.get() },
        { '$', n0x753a30.get() }, { '#', n0x753a30.get() },
        { '"', n0x753a30.get() }, { ' ', n0x753a30.get() },
        { '!', n0x753a30.get() }, { '%', n0x753a30.get() },
        { '&', n0x753a30.get() }, { '\'', n0x753a30.get() },
        { '(', n0x753a30.get() }, { ')', n0x753a30.get() },
        { '*', n0x753a30.get() }, { '6', n0x754050.get() },
        { '7', n0x754050.get() }, { '8', n0x754050.get() },
        { '9', n0x754050.get() }, { ':', n0x753a30.get() },
        { ';', n0x753a30.get() }, { '<', n0x753a30.get() },
        { '=', n0x753a30.get() }, { '>', n0x753a30.get() },
        { '?', n0x753a30.get() }, { '@', n0x753a30.get() },
        { 'A', n0x753a30.get() }, { 'B', n0x753a30.get() },
        { 'C', n0x753a30.get() }, { 'D', n0x753a30.get() },
        { 'E', n0x753a30.get() }, { 'F', n0x753a30.get() },
        { 'G', n0x753a30.get() }, { 'H', n0x753a30.get() },
        { 'I', n0x753a30.get() }, { 'J', n0x753a30.get() },
        { 'K', n0x753a30.get() }, { 'L', n0x753a30.get() },
        { 'M', n0x753a30.get() },
      };
      n0x754e10->ts_ = {
        { '~', n0x753a30.get() }, { '}', n0x753a30.get() },
        { '|', n0x753a30.get() }, { '{', n0x753a30.get() },
        { 'z', n0x753a30.get() }, { 'y', n0x753a30.get() },
        { 'x', n0x753a30.get() }, { 'w', n0x753a30.get() },
        { 'v', n0x753a30.get() }, { 'u', n0x753a30.get() },
        { 't', n0x753a30.get() }, { 's', n0x753a30.get() },
        { 'r', n0x753a30.get() }, { 'q', n0x753a30.get() },
        { 'p', n0x753a30.get() }, { 'o', n0x753a30.get() },
        { 'n', n0x753a30.get() }, { 'm', n0x753a30.get() },
        { 'l', n0x753a30.get() }, { 'k', n0x753a30.get() },
        { 'j', n0x753a30.get() }, { 'i', n0x753a30.get() },
        { 'h', n0x753a30.get() }, { 'g', n0x753a30.get() },
        { 'f', n0x753a30.get() }, { 'e', n0x753a30.get() },
        { 'd', n0x753a30.get() }, { 'c', n0x753a30.get() },
        { 'b', n0x753a30.get() }, { 'a', n0x753a30.get() },
        { '`', n0x753a30.get() }, { '_', n0x753a30.get() },
        { '^', n0x753a30.get() }, { ']', n0x753a30.get() },
        { '[', n0x753a30.get() }, { 'Z', n0x753a30.get() },
        { 'Y', n0x753a30.get() }, { 'X', n0x753a30.get() },
        { 'W', n0x753a30.get() }, { 'V', n0x753a30.get() },
        { 'U', n0x753a30.get() }, { 'T', n0x753a30.get() },
        { 'S', n0x753a30.get() }, { 'R', n0x753a30.get() },
        { 'Q', n0x753a30.get() }, { 'P', n0x753a30.get() },
        { 'O', n0x753a30.get() }, { 'N', n0x753a30.get() },
        { '5', n0x753a30.get() }, { '4', n0x753a30.get() },
        { '3', n0x753a30.get() }, { '2', n0x753a30.get() },
        { '1', n0x753a30.get() }, { '0', n0x753a30.get() },
        { '/', n0x753a30.get() }, { '\\', n0x753a30.get() },
        { '-', n0x753a30.get() }, { '.', n0x753a30.get() },
        { ',', n0x753a30.get() }, { '+', n0x753a30.get() },
        { '$', n0x753a30.get() }, { '#', n0x753a30.get() },
        { '"', n0x753a30.get() }, { ' ', n0x753a30.get() },
        { '!', n0x753a30.get() }, { '%', n0x753a30.get() },
        { '&', n0x753a30.get() }, { '\'', n0x753a30.get() },
        { '(', n0x753a30.get() }, { ')', n0x753a30.get() },
        { '*', n0x753a30.get() }, { '6', n0x753a30.get() },
        { '7', n0x753a30.get() }, { '8', n0x753a30.get() },
        { '9', n0x753a30.get() }, { ':', n0x753a30.get() },
        { ';', n0x753a30.get() }, { '<', n0x753a30.get() },
        { '=', n0x753a30.get() }, { '>', n0x753a30.get() },
        { '?', n0x753a30.get() }, { '@', n0x753a30.get() },
        { 'A', n0x753a30.get() }, { 'B', n0x753a30.get() },
        { 'C', n0x753a30.get() }, { 'D', n0x753a30.get() },
        { 'E', n0x753a30.get() }, { 'F', n0x753a30.get() },
        { 'G', n0x753a30.get() }, { 'H', n0x753a30.get() },
        { 'I', n0x753a30.get() }, { 'J', n0x753a30.get() },
        { 'K', n0x753a30.get() }, { 'L', n0x753a30.get() },
        { 'M', n0x753a30.get() },
      };
      n0x754770->ts_ = {
        { '~', n0x753a30.get() }, { '}', n0x753a30.get() },
        { '|', n0x753a30.get() }, { '{', n0x753a30.get() },
        { 'z', n0x753a30.get() }, { 'y', n0x753a30.get() },
        { 'x', n0x753a30.get() }, { 'w', n0x753a30.get() },
        { 'v', n0x753a30.get() }, { 'u', n0x753a30.get() },
        { 't', n0x753a30.get() }, { 's', n0x753a30.get() },
        { 'r', n0x753a30.get() }, { 'q', n0x753a30.get() },
        { 'p', n0x753a30.get() }, { 'o', n0x753a30.get() },
        { 'n', n0x753a30.get() }, { 'm', n0x753a30.get() },
        { 'l', n0x753a30.get() }, { 'k', n0x753a30.get() },
        { 'j', n0x753a30.get() }, { 'i', n0x753a30.get() },
        { 'h', n0x753a30.get() }, { 'g', n0x753a30.get() },
        { 'f', n0x753a30.get() }, { 'e', n0x753a30.get() },
        { 'd', n0x753a30.get() }, { 'c', n0x753a30.get() },
        { 'b', n0x753a30.get() }, { 'a', n0x753a30.get() },
        { '`', n0x753a30.get() }, { '_', n0x753a30.get() },
        { '^', n0x753a30.get() }, { ']', n0x753a30.get() },
        { '[', n0x753a30.get() }, { 'Z', n0x753a30.get() },
        { 'Y', n0x753a30.get() }, { 'X', n0x753a30.get() },
        { 'W', n0x753a30.get() }, { 'V', n0x753a30.get() },
        { 'U', n0x753a30.get() }, { 'T', n0x753a30.get() },
        { 'S', n0x753a30.get() }, { 'R', n0x753a30.get() },
        { 'Q', n0x753a30.get() }, { 'P', n0x753a30.get() },
        { 'O', n0x753a30.get() }, { 'N', n0x753a30.get() },
        { '5', n0x753a30.get() }, { '4', n0x753a30.get() },
        { '3', n0x753a30.get() }, { '2', n0x753a30.get() },
        { '1', n0x753a30.get() }, { '0', n0x753a30.get() },
        { '/', n0x753a30.get() }, { '\\', n0x753a30.get() },
        { '-', n0x753a30.get() }, { '.', n0x753a30.get() },
        { ',', n0x753a30.get() }, { '+', n0x753a30.get() },
        { '$', n0x753a30.get() }, { '#', n0x753a30.get() },
        { '"', n0x753a30.get() }, { ' ', n0x753a30.get() },
        { '!', n0x753a30.get() }, { '%', n0x753a30.get() },
        { '&', n0x753a30.get() }, { '\'', n0x753a30.get() },
        { '(', n0x753a30.get() }, { ')', n0x753a30.get() },
        { '*', n0x753a30.get() }, { '6', n0x753a30.get() },
        { '7', n0x753a30.get() }, { '8', n0x753a30.get() },
        { '9', n0x753a30.get() }, { ':', n0x753a30.get() },
        { ';', n0x753a30.get() }, { '<', n0x753a30.get() },
        { '=', n0x753a30.get() }, { '>', n0x753a30.get() },
        { '?', n0x753a30.get() }, { '@', n0x753a30.get() },
        { 'A', n0x753a30.get() }, { 'B', n0x753a30.get() },
        { 'C', n0x753a30.get() }, { 'D', n0x753a30.get() },
        { 'E', n0x753a30.get() }, { 'F', n0x753a30.get() },
        { 'G', n0x753a30.get() }, { 'H', n0x753a30.get() },
        { 'I', n0x753a30.get() }, { 'J', n0x753a30.get() },
        { 'K', n0x753a30.get() }, { 'L', n0x753a30.get() },
        { 'M', n0x753a30.get() },
      };
      return std::move(n0x753d20);
    }
    auto root = makeDFA();
  }  // namespace lexer
  enum class Assoc { LEFT, RIGHT, NOT, NONE };
  struct DFARule {
    int concrete;
    vector<int> symbols;
    size_t pos;
    mutable vector<bool> lookahead;

    friend std::ostream& operator<<(std::ostream& out, const DFARule& rule) {
      out << rule.concrete << " -> ";
      size_t len = rule.symbols.size();
      for (size_t i = 0; i < len; ++i) {
        if (i == rule.pos) {
          out << '.';
        }

        int symbol = rule.symbols[i];
        if (isToken(symbol)) {
          out << GRAMMAR_DATA.tokens[tokenToFromIndex(symbol)].name;
        } else {
          out << GRAMMAR_DATA.variables[symbol].name;
        }
        out << ' ';
      }
      if (rule.pos == len) {
        out << '.';
      }
      return out;
    }
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

    auto n0x7571e0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0x756180 =
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
    auto n0x752c60 =
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
    auto n0x756080 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0x755fb0 = std::make_unique<Node>(
        RuleData{ optional<DFARule>{}, -2147483648, Assoc::NONE });
    auto n0x756640 =
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
    auto n0x754a20 =
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
      n0x7571e0->ts_ = {
        { -1, n0x756180.get() },
        { 1, n0x752c60.get() },
      };
      n0x756180->ts_ = {};
      n0x752c60->ts_ = {
        { -3, n0x756080.get() },
        { -2, n0x755fb0.get() },
      };
      n0x756080->ts_ = {
        { -1, n0x756180.get() },
        { 1, n0x756640.get() },
      };
      n0x755fb0->ts_ = {
        { -1, n0x756180.get() },
        { 1, n0x754a20.get() },
      };
      n0x756640->ts_ = {
        { -3, n0x756080.get() },
        { -2, n0x755fb0.get() },
      };
      n0x754a20->ts_ = {
        { -3, n0x756080.get() },
        { -2, n0x755fb0.get() },
      };
      return std::move(n0x7571e0);
    }
    auto root = makeDFA();
  }  // namespace parser
  void consumeWhiteSpace(string_view& input) {
    size_t i = 0;
    while (isspace(input[i])) {
      ++i;
    }
    input = input.substr(i);
  }

  optional<StackObj> getToken(string_view& input) {
    size_t i = 0;
    const size_t len = input.size();
    size_t lastAcceptingPos;
    int lastAcceptingToken = NONE;
    lexer::Node* currentNode = lexer::root.get();

    while (currentNode) {
      int token = currentNode->v_;
      if (token != NONE) {
        lastAcceptingToken = token;
        lastAcceptingPos = i;
      }

      if (i == len) {
        break;
      }
      currentNode = currentNode->step(input[i++]);
    }

    if (lastAcceptingToken == NONE) {
      return {};
    }

    StackObj stackObj = constructTokenObj(
        lastAcceptingToken, input.substr(0, lastAcceptingPos));
    input = input.substr(lastAcceptingPos);
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
        tokens.push_back(move(*optionalObj));
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
        error << "Lexer error at: " << inputView.substr(0, 25) << '\n'
              << "Previous tokens were: " << prevTokenNames;
        for_each(tokens.cbegin(), tokens.cend(), deleteObj);
        throw runtime_error(error.str());
      }

      consumeWhiteSpace(inputView);
    }

    return tokens;
  }

  void conflictWarning(const DFARule& rule, int nextToken) {
    cerr << "WARNING: Shift reduce conflict for rule\n\t" << rule
         << "\n\tNext token: "
         << GRAMMAR_DATA.tokens[tokenToFromIndex(nextToken)].name << endl;
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

    errMsg << "No parse:\n\tStack: " << stkSymbolNames
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
      conflictWarning(*ruleData.reducibleRule, nextToken);
      return NONE;
    }
    if (ruleData.precedence > shiftPrecedence) {
      return ruleData.reducibleRule->concrete;
    } else if (ruleData.precedence == shiftPrecedence) {
      if (ruleData.assoc == Assoc::LEFT) {
        return ruleData.reducibleRule->concrete;
      } else if (ruleData.assoc == Assoc::NONE) {
        conflictWarning(*ruleData.reducibleRule, nextToken);
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
            &stk.data()[reduceStart],
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
namespace test::bad_expr_parser {
  Expr* parse(const string& input) {
    vector<StackObj> stackObjs = tokenize(input);
    return shiftReduce(stackObjs);
  }
  Expr* parse(istream& input) {
    return parse(string(
        istreambuf_iterator<char>{ input }, istreambuf_iterator<char>{}));
  }
}  // namespace test::bad_expr_parser