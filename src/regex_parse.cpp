#include "regex_parse.hpp"
#include "utils.hpp"
#include "dfa.hpp"
#include "rules.hpp"
#include "build_parser.hpp"

#include <iostream>
#include <cstddef>
#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>
#include <sstream>

using namespace std;


/***********
 * GRAMMAR *
 ***********/

/* CHAR { char :: str[0] } <DELETER OPTIONAL>
 *
 *
 * Regex { Regex* } { delete $; }
 * Regex := Alts                             { new Alt(move($0)) }
 *        | Concats                          { new Concat(move($0)) }
 *        | Regex STAR                       { new Star($0) }
 *        | CARET Regex                      { new Not($1) }
 *        | LBRACKET CHAR DASH CHAR RBRACKET { new Range($1, $3) }
 *        | LPAREN Regex RPAREN              { $1 }
 *        | CHAR                             { new Character($0) }
 *
 * Alts { RegexVector }
 * Alts := Regex BAR Regex { RegexVector($0, $2) }
 *       | Regex BAR Alts  { RegexVector($0, move($2)) }
 *
 * Concats { RegexVector* }
 * Concats := Regex Regex   { RegexVector($0, $1) }
 *          | Regex Concats { RegexVector($0, move($1)) }
 *
 *
 * */

namespace {

  /* Variables */
  constexpr int REGEX = 1;
  constexpr int ALTS = 2;
  constexpr int CONCATS = 3;

  /* Tokens */
  constexpr int BAR = -1;
  constexpr int STAR = -2;
  constexpr int CARET = -3;
  constexpr int LBRACKET = -4;
  constexpr int RBRACKET = -5;
  constexpr int LPAREN = -6;
  constexpr int RPAREN = -7;
  constexpr int DASH = -8;
  constexpr int CHAR = -9;

  /* Concrete Types */
  constexpr int RALT = 1;
  constexpr int RCONCAT = 2;
  constexpr int RSTAR = 3;
  constexpr int RNOTRANGE = 4;
  constexpr int RRANGE = 5;
  constexpr int RGROUP = 6;
  constexpr int RCHAR = 7;
  constexpr int AREGEX = 8;
  constexpr int AALT = 9;
  constexpr int CREGEX = 10;
  constexpr int CCONCAT = 11;


  struct GrammarData GRAMMAR_DATA = {
    /* tokens */ {
        { "BAR", "", 1, Assoc::LEFT, "", "", "" },
        { "STAR", "", 6, Assoc::LEFT, "", "", "" },
        { "CARET", "", NONE, Assoc::RIGHT, "", "", "" },
        { "LBRACKET", "", 4, Assoc::NONE, "", "", "" },
        { "RBRACKET", "", 4, Assoc::NONE, "", "", "" },
        { "LPAREN", "", 4, Assoc::NONE, "", "", "" },
        { "RPAREN", "", 4, Assoc::NONE, "", "", "" },
        { "DASH", "", NONE, Assoc::NONE, "", "", "" },
        { "CHAR", "", 4, Assoc::LEFT, "", "", "" },
    },

    /* concretes */
    { { "SCONC", S, NONE, { REGEX }, "" },
      { "RALT", REGEX, NONE, { ALTS }, "" },
      { "RCONCAT", REGEX, NONE, { CONCATS }, "" },
      { "RSTAR", REGEX, NONE, { REGEX, STAR }, "" },
      { "RNOTRANGE", REGEX, NONE, { LBRACKET, CARET, CHAR, DASH, CHAR, RBRACKET }, "" },
      { "RRANGE", REGEX, NONE, { LBRACKET, CHAR, DASH, CHAR, RBRACKET }, "" },
      { "RGROUP", REGEX, NONE, { LPAREN, REGEX, RPAREN }, "" },
      { "RCHAR", REGEX, NONE, { CHAR }, "" },
      { "AREGEX", ALTS, NONE, { REGEX, BAR, REGEX }, "" },
      { "AALT", ALTS, NONE, { ALTS, BAR, REGEX }, "" },
      { "CREGEX", CONCATS, 5, { REGEX, REGEX }, "" },
      { "CCONCAT", CONCATS, 5, { CONCATS, REGEX }, "" } },

    /* variables */
    { { "S", "", { SCONC }, "" },
      { "Regex",
        "",
        { RALT, RCONCAT, RSTAR, RNOTRANGE, RRANGE, RGROUP, RCHAR },
        "" },
      { "Alts",
        "",
        {
            AREGEX,
            AALT,
        },
        "" },
      { "Concats",
        "",
        {
            CREGEX,
            CCONCAT,
        },
        "" } }
  };

  /*********
   * LEXER *
   *********/

  StackObj<nullptr_t> datalessObj(int symbol) { return StackObj<nullptr_t>{ {symbol}, unique_ptr<nullptr_t>() }; }

  vector<StackObjBase> lex(const string& input) {
    vector<StackObjBase> tokens;
    bool escaped = false;
    tokens.reserve(input.size());

    for (char c : input) {
      if (escaped) {
        tokens.push_back(StackObj<char>{ {CHAR}, new char(c) });
        escaped = false;
        continue;
      }

      switch (c) {
        case '|':
          tokens.push_back(datalessObj(BAR));
          break;
        case '*':
          tokens.push_back(datalessObj(STAR));
          break;
        case '^':
          tokens.push_back(datalessObj(CARET));
          break;
        case '[':
          tokens.push_back(datalessObj(LBRACKET));
          break;
        case ']':
          tokens.push_back(datalessObj(RBRACKET));
          break;
        case '(':
          tokens.push_back(datalessObj(LPAREN));
          break;
        case ')':
          tokens.push_back(datalessObj(RPAREN));
          break;
        case '-':
          tokens.push_back(datalessObj(DASH));
          break;
        case '\\':
          escaped = true;
          break;
        default:
          tokens.push_back(StackObj<char>{ {CHAR}, new char(c) });
      }
    }

    return tokens;
  }

  /**********
   * PARSER *
   **********/

  using CondensedDFA = DFA<RuleData, int>;
  using CondensedNode = CondensedDFA::Node;

  const CondensedDFA PARSER_DFA =
      buildParserDFA(GRAMMAR_DATA)
          .convert<RuleData>([](const DFARuleSet& ruleSet) {
            return condenseRuleSet(ruleSet, GRAMMAR_DATA);
          });

  struct Start {
    Start(Regex** r) : r_(r) {}
    ~Start() { delete r_; }
    Regex** r_;
  };

  // void deleteObjPtr(const StackObj& stackObj) noexcept {
  //   switch (stackObj.symbol) {
  //     case REGEX:
  //       delete (Regex**)stackObj.obj;
  //       break;
  //     case ALTS:
  //       delete (RegexVector*)stackObj.obj;
  //       break;
  //     case CONCATS:
  //       delete (RegexVector*)stackObj.obj;
  //       break;
  //     case CHAR:
  //       delete (char*)stackObj.obj;
  //       break;
  //     default:
  //       return;
  //   }
  // }


  void deleteObj(const StackObj& stackObj) noexcept {
    if (stackObj.symbol == REGEX) {
      delete (*(Regex**)stackObj.obj);
    }
    // deleteObjPtr(stackObj);
  }

  // TODO: Remove throw and make noexcept when done
  /* Construct a concrete object */
  void* constructObj(int concrete, StackObj* args) {
    switch (concrete) {
      case RALT:
        return new Regex*(new Alt(move(*(RegexVector*)args[0].obj)));
      case RCONCAT:
        return new Regex*(new Concat(move(*(RegexVector*)args[0].obj)));
      case RSTAR:
        return new Regex*(new Star(*(Regex**)args[0].obj));
      case RNOTRANGE:
        return new Regex*(new Range(*(char*)args[2].obj, *(char*)args[4].obj, true));
      case RRANGE:
        return new Regex*(new Range(*(char*)args[1].obj, *(char*)args[3].obj, false));
      case RGROUP:
        return new Regex*(*(Regex**)args[1].obj);
      case RCHAR:
        return new Regex*(new Character(*(char*)args[0].obj));
      case AREGEX:
        return new RegexVector(
            RegexVector(*(Regex**)args[0].obj, *(Regex**)args[2].obj));
      case AALT:
        return new RegexVector(RegexVector(
            move(*(RegexVector*)args[0].obj), *(Regex**)args[2].obj));
      case CREGEX:
        return new RegexVector(
            RegexVector(*(Regex**)args[0].obj, *(Regex**)args[1].obj));
      case CCONCAT:
        return new RegexVector(RegexVector(
            move(*(RegexVector*)args[0].obj), *(Regex**)args[1].obj));
      case SCONC:
        return new Start((Regex**)args[0].obj);
      default:
        throw invalid_argument("Can't construct. Out of options.");
    }
  }

  StackObj construct(int concrete, StackObj* args, int varType) {
    return StackObj{ constructObj(concrete, args), varType };
  }


  int tryReduce(
      const CondensedNode* node,
      int nextToken,
      const vector<StackObj>& stk,
      const vector<Token>& tokens) {
    const RuleData& ruleData = node->getValue();

    // No reducible rule, so try shifting
    if (!ruleData.reducibleRule.has_value()) {
      return NONE;
    }

    const DFARule& rule = *ruleData.reducibleRule;

    // Check if next token is in the lookahead set
    if (nextToken != NONE && !rule.lookahead[tokenToFromIndex(nextToken)]) {
      return NONE;
    }

    // Check if rule matches the stack
    if (!equal(
            rule.symbols.crbegin(),
            rule.symbols.crend(),
            stk.crbegin(),
            [](int symbol, const StackObj& stkObj) {
              return stkObj.symbol == symbol;
            })) {
      return NONE;
    }

    // Can't shift, so reduce. After this point, nextToken
    // is guaranteed to not be NONE
    if (!node->getTransitions().contains(nextToken)) {
      return ruleData.reducibleRule->concrete;
    }

    // TODO: Handle nonassociativity
    // If both are options, then reduce if one of the following is true:
    // - precedence of rule is higher than that of next token
    // - precedence of rule is the same of that of next token and the rule's
    //   last token is left-associative
    int shiftPrecedence = tokens[tokenToFromIndex(nextToken)].precedence;

    // Unspecified precedence -> conflict! (Resolve by shifting)
    if (ruleData.precedence == NONE && shiftPrecedence == NONE) {
      return NONE;
    }

    if (ruleData.precedence > shiftPrecedence) {
      return ruleData.reducibleRule->concrete;
    } else if (ruleData.precedence == shiftPrecedence) {
      if (ruleData.assoc == Assoc::LEFT) {
        return ruleData.reducibleRule->concrete;
      } else if (ruleData.assoc == Assoc::NONE) {
        // Unspecified associativity -> conflict! (Resolve by shifting)
        return NONE;
      }
    }

    // shift precedence is higher, or same and right associative, so shift
    return NONE;
  }


  /* Deep delete StackObj::obj starting at index i */
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

    errMsg << "No parse:\n\tStack: " << stkSymbolNames
           << "\n\tRemaining tokens: " << remainingTokenNames;
    throw invalid_argument(errMsg.str());
  }


  Regex* shiftReduce(vector<StackObj>& inputTokens) {
    // TODO: Check for empty input
    vector<StackObj> stk = { move(inputTokens[0]) };
    size_t i = 1;
    size_t inputSize = inputTokens.size();

    // Stop when we have consumed all the input and the root of grammar
    // is the only thing on the stack
    while (!(i == inputSize && stk.size() == 1 && stk[0].symbol == S)) {
      // Run the DFA.
      // TODO: Only backtrack as far as the reduction (store path)
      vector<int> stkSymbols;
      transform(
          stk.begin(),
          stk.end(),
          back_inserter(stkSymbols),
          [](StackObj stkObj) { return stkObj.symbol; });

      // ----------DEBUG------------
      // vector<string> stkSymbolNames;
      // auto stkObjToName = [](StackObj stkObj) {
      //   if (isToken(stkObj.symbol)) {
      //     return GRAMMAR_DATA.tokens[tokenToFromIndex(stkObj.symbol)].name;
      //   }
      //   return GRAMMAR_DATA.variables[stkObj.symbol].name;
      // };
      // transform(
      //     stk.begin(), stk.end(), back_inserter(stkSymbolNames), stkObjToName);
      // cout << stkSymbolNames << endl;
      // ----------END-DEBUG----------


      CondensedNode* currentNode = PARSER_DFA.run(stkSymbols);
      if (currentNode == nullptr) {
        cleanPtrsFrom(stk, 0);
        cleanPtrsFrom(inputTokens, i + 1);
        parseError(stk, inputTokens, i + 1);
      }

      int nextInputToken = i == inputSize ? NONE : inputTokens[i].symbol;
      int concrete =
          tryReduce(currentNode, nextInputToken, stk, GRAMMAR_DATA.tokens);
      // Reduce
      if (concrete != NONE) {
        // Construct the new object, pop the arguments off the stack,
        // and push the new object onto it.
        size_t reduceStart =
            stk.size() - currentNode->getValue().reducibleRule->symbols.size();
        StackObj newObj = construct(
            concrete,
            &stk.data()[reduceStart],
            GRAMMAR_DATA.concretes[concrete].varType);

        // We always add the rule S -> <root_type>, so there is only one thing
        // on the stack if we reduced to S, and we don't want to delete the
        // pointer because this is the object we are returning to the caller
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

      } else {  // Shift
        // No more tokens, didn't reduce to S
        if (i == inputSize) {
          cleanPtrsFrom(stk, 0);
          parseError(stk, inputTokens, i);
        }
        StackObj token = inputTokens[i];
        currentNode = CondensedDFA::step(currentNode, token.symbol);
        stk.push_back(move(token));
        // No transition for this token
        if (currentNode == nullptr) {
          cleanPtrsFrom(stk, 0);
          cleanPtrsFrom(inputTokens, i + 1);
          parseError(stk, inputTokens, i + 1);
        }
        ++i;
      }
    }

    // Remove the actual grammar root from the fake root we encapsulated it with
    Start* start = (Start*)(stk[0].obj);
    Regex* root = *start->r_;
    delete start;
    return root;
  }
}  // namespace


RgxPtr parse(const string& input) {
  vector<StackObj> stackObjs = lex(input);
  return RgxPtr(shiftReduce(stackObjs));
}
