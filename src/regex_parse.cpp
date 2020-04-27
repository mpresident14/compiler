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
 * Regex { Regex* } { delete #0; }
 * Regex := Alt                              { #0 }
 *        | Concats                          { new Concat(move(#0)) }
 *        | Regex STAR                       { new Star(#0) }
 *        | Not                              { #0 }
 *        | LBRACKET CHAR DASH CHAR RBRACKET { new Range(#1, #3) }
 *        | LPAREN Regex RPAREN              { #1 }
 *        | CHAR                             { new Character(#0) }
 *        | LBRACKET CHAR RBRACKET           { new Character(#1))) }
 *
 * Alt { Regex* } { delete #0; }
 * Alt := Alts                       { new Alt(move(#0)) }
 *      | LBRACKET Concats RBRACKET  { new Alt(move(#1)) }
 *
 * Not { Regex* } { delete #0; }
 * Not := LBRACKET CARET CHAR RBRACKET            { new Not(new Character(#2))) }
 *      | LBRACKET CARET Concats RBRACKET         { new Not(new Alt(move(#2))) }
 *      | LBRACKET CARET CHAR DASH CHAR RBRACKET  { new Not(new Range(#2, #4)) }
 *
 * Alts { RegexVector }
 * Alts := Regex BAR Regex { RegexVector(#0, #2) }
 *       | Regex BAR Alts  { RegexVector(#0, move(#2)) }
 *
 * Concats { RegexVector* }
 * Concats := Regex Regex   { RegexVector(#0, #1) }
 *          | Regex Concats { RegexVector(#0, move(#1)) }
 *
 *
 * */

namespace {

  /* Tokens */
  enum TokenEnum {
    BAR = -1,
    STAR = -2,
    CARET = -3,
    LBRACKET = -4,
    RBRACKET = -5,
    LPAREN = -6,
    RPAREN = -7,
    DASH = -8,
    CHAR = -9,
  };

  /* Variables */
  enum VarEnum {
    REGEX = 1,
    ALT,
    NOT,
    ALTS,
    CONCATS,
  };

  /* Concrete Types */
  enum ConcreteEnum {
    REGEX_ALT = 1,
    REGEX_CONCATS,
    REGEX_STAR,
    REGEX_NOT,
    REGEX_RANGE,
    REGEX_GROUP,
    REGEX_CHAR,
    REGEX_BRACKET_CHAR,
    ALT_ALTS,
    ALT_BRACKET,
    NOT_CHAR,
    NOT_CONCATS,
    NOT_RANGE,
    ALTS_REGEX,
    ALTS_ALTS,
    CONCATS_REGEX,
    CONCATS_CONCATS,
  };


  // NOTE: These must be in same order as the constants above.
  // TODO: Fix when we get {[<int>] = <value>} initialization
  struct GrammarData GRAMMAR_DATA = {
    /* tokens */ {
        { "BAR", "", 1, Assoc::LEFT, "", "", "" },
        { "STAR", "", 6, Assoc::LEFT, "", "", "" },
        { "CARET", "", NONE, Assoc::RIGHT, "", "", "" },
        { "LBRACKET", "", 3, Assoc::NONE, "", "", "" },
        { "RBRACKET", "", 5, Assoc::NONE, "", "", "" },
        { "LPAREN", "", 3, Assoc::NONE, "", "", "" },
        { "RPAREN", "", 3, Assoc::NONE, "", "", "" },
        { "DASH", "", NONE, Assoc::NONE, "", "", "" },
        { "CHAR", "", 3, Assoc::LEFT, "", "", "" },
    },

    /* concretes */
    { { "SCONC", S, NONE, { REGEX }, "" },
      { "REGEX_ALT", REGEX, NONE, { ALT }, "" },
      { "REGEX_CONCATS", REGEX, NONE, { CONCATS }, "" },
      { "REGEX_STAR", REGEX, NONE, { REGEX, STAR }, "" },
      { "REGEX_NOT", REGEX, NONE, { NOT }, "" },
      { "REGEX_RANGE", REGEX, NONE, { LBRACKET, CHAR, DASH, CHAR, RBRACKET }, "" },
      { "REGEX_GROUP", REGEX, NONE, { LPAREN, REGEX, RPAREN }, "" },
      { "REGEX_CHAR", REGEX, NONE, { CHAR }, "" },
      { "REGEX_BRACKET_CHAR", REGEX, NONE, { LBRACKET, CHAR, RBRACKET }, "" },
      { "ALT_ALTS", ALT, NONE, { ALTS }, "" },
      { "ALT_BRACKET", ALT, NONE, { LBRACKET, CONCATS, RBRACKET }, "" },
      { "NOT_CHAR", NOT, NONE, { LBRACKET, CARET, CHAR, RBRACKET }, "" },
      { "NOT_CONCATS", NOT, NONE, { LBRACKET, CARET, CONCATS, RBRACKET }, "" },
      { "NOT_RANGE", NOT, NONE, { LBRACKET, CARET, CHAR, DASH, CHAR, RBRACKET }, "" },
      { "ALTS_REGEX", ALTS, NONE, { REGEX, BAR, REGEX }, "" },
      { "ALTS_ALTS", ALTS, NONE, { ALTS, BAR, REGEX }, "" },
      { "CONCATS_REGEX", CONCATS, 4, { REGEX, REGEX }, "" },
      { "CONCATS_CONCATS", CONCATS, 4, { CONCATS, REGEX }, "" } },

    /* variables */
    { { "S", "", { SCONC }, "" },
      { "Regex",
        "",
        { REGEX_ALT, REGEX_CONCATS, REGEX_STAR, REGEX_NOT, REGEX_RANGE, REGEX_GROUP, REGEX_CHAR, REGEX_BRACKET_CHAR },
        "" },
      { "Alt",
        "",
        {
            ALT_ALTS,
            ALT_BRACKET,
        },
        "" },
      { "Not",
        "",
        {
            NOT_CHAR,
            NOT_CONCATS,
            NOT_RANGE,
        },
        "" },
      { "Alts",
        "",
        {
            ALTS_REGEX,
            ALTS_ALTS,
        },
        "" },
      { "Concats",
        "",
        {
            CONCATS_REGEX,
            CONCATS_CONCATS,
        },
        "" } }
  };

  /*********
   * LEXER *
   *********/

  StackObj datalessObj(int symbol) { return StackObj{ nullptr, symbol }; }

  vector<StackObj> lex(const string& input) {
    vector<StackObj> tokens;
    bool escaped = false;
    size_t leftBracket = 0;
    bool caret = false;
    tokens.reserve(input.size());

    for (char c : input) {

      // All characters within brackets are just literals except '-' for a range and the first '^'
      if (leftBracket > 0) {
        if (escaped) {
          tokens.push_back({ new char(c), CHAR });
          escaped = false;
          ++leftBracket;
          continue;
        }

        if (c == '^' && leftBracket == 1) {
          tokens.push_back(datalessObj(CARET));
          caret = true;
          leftBracket = 2;
        } else if (c == '-' && ((leftBracket == 2 && !caret) || (leftBracket == 3 && caret))) {
          tokens.push_back(datalessObj(DASH));
          leftBracket = 4;
        } else if (c == '\\') {
          escaped = true;
        } else if (c == ']') {
          tokens.push_back(datalessObj(RBRACKET));
          leftBracket = 0;
          caret = false;
        } else {
          tokens.push_back({ new char(c), CHAR });
          ++leftBracket;
        }
        continue;
      }


      if (escaped) {
        tokens.push_back({ new char(c), CHAR });
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
        case '[':
          tokens.push_back(datalessObj(LBRACKET));
          leftBracket = 1;
          break;
        case '(':
          tokens.push_back(datalessObj(LPAREN));
          break;
        case ')':
          tokens.push_back(datalessObj(RPAREN));
          break;
        case '\\':
          escaped = true;
          break;
        default:
          tokens.push_back({ new char(c), CHAR });
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

  void deleteObjPtr(const StackObj& stackObj) noexcept {
    switch (stackObj.symbol) {
      case REGEX:
      case ALT:
      case NOT:
        delete (Regex**)stackObj.obj;
        break;
      case ALTS:
      case CONCATS:
        delete (RegexVector*)stackObj.obj;
        break;
      case CHAR:
        delete (char*)stackObj.obj;
        break;
      default:
        return;
    }
  }


  void deleteObj(const StackObj& stackObj) noexcept {
    switch (stackObj.symbol) {
      case REGEX:
      case ALT:
      case NOT:
        delete (*(Regex**)stackObj.obj);
    }
    deleteObjPtr(stackObj);
  }

  // TODO: Remove throw and make noexcept when done
  /* Construct a concrete object */
  void* constructObj(int concrete, StackObj* args) {
    switch (concrete) {
      case REGEX_ALT:
        return new Regex*(*(Regex**) args[0].obj);
      case REGEX_CONCATS:
        return new Regex*(new Concat(move(*(RegexVector*)args[0].obj)));
      case REGEX_STAR:
        return new Regex*(new Star(*(Regex**)args[0].obj));
      case REGEX_NOT:
        return new Regex*(*(Regex**) args[0].obj);
      case REGEX_RANGE:
        return new Regex*(new Range(*(char*)args[1].obj, *(char*)args[3].obj));
      case REGEX_GROUP:
        return new Regex*(*(Regex**)args[1].obj);
      case REGEX_CHAR:
        return new Regex*(new Character(*(char*)args[0].obj));
      case REGEX_BRACKET_CHAR:
        return new Regex*(new Character(*(char*)args[1].obj));
      case ALT_ALTS:
        return new Regex*(new Alt(move(*(RegexVector*)args[0].obj)));
      case ALT_BRACKET:
        return new Regex*(new Alt(move(*(RegexVector*)args[1].obj)));
      case NOT_CHAR:
        return new Regex*(new Not(new Character(*(char*) args[2].obj)));
      case NOT_CONCATS:
        return new Regex*(new Not(new Alt(move(*(RegexVector*) args[2].obj))));
      case NOT_RANGE:
        return new Regex*(new Not(new Range(*(char*)args[2].obj, *(char*)args[4].obj)));
      case ALTS_REGEX:
        return new RegexVector(
            RegexVector(*(Regex**)args[0].obj, *(Regex**)args[2].obj));
      case ALTS_ALTS:
        return new RegexVector(RegexVector(
            move(*(RegexVector*)args[0].obj), *(Regex**)args[2].obj));
      case CONCATS_REGEX:
        return new RegexVector(
            RegexVector(*(Regex**)args[0].obj, *(Regex**)args[1].obj));
      case CONCATS_CONCATS:
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
