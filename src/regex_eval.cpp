#include "regex_eval.hpp"

using namespace std;


// TODO: Allow passing in of an symbolToString function for better error messages

namespace {
  struct RuleData {
    optional<DFARule> reducibleRule;
    int precedence;
    Assoc assoc;
  };

  inline bool operator==(const RuleData& lhs, const RuleData rhs) noexcept {
    return lhs.reducibleRule == rhs.reducibleRule;
  }
}
// TODO: Fix these hash functions
namespace std {
  template<>
  struct hash<RuleData> {
    size_t operator()(const RuleData& ruleData) const noexcept {
      if (!ruleData.reducibleRule.has_value()) {
        return 0;
      }
      return DFARuleHash()(*ruleData.reducibleRule);
    }
  };
}


namespace {
  RuleData condenseRuleSet(const DFARuleSet& ruleSet, const GrammarData& grammarData) {
    auto setIter = find_if(ruleSet.cbegin(), ruleSet.cend(), mem_fun_ref(&DFARule::atEnd));
    // No reducible rules
    if (setIter == ruleSet.cend()) {
      return RuleData{ {}, NONE, Assoc::NONE };
    }

    const DFARule& rule = *setIter;
    int rulePrecedence = grammarData.overridePrecs[rule.concrete];

    auto ruleIter = find_if(rule.symbols.crbegin(), rule.symbols.crend(), isToken);
    // Reducible rule contains no tokens
    if (ruleIter == rule.symbols.crend()) {
      return RuleData{ optional(rule), rulePrecedence, Assoc::NONE };
    }

    int lastToken = *ruleIter;
    // If no override precedence, check precedence of token
    if (rulePrecedence == NONE) {
      rulePrecedence = grammarData.tokenPrecs[tokensIndex(lastToken)];
    }

    // If there is rule precedence, get associativity
    if (rulePrecedence == NONE) {
      return RuleData{ optional(rule), NONE, Assoc::NONE };
    } else {
      return RuleData{ optional(rule), rulePrecedence, grammarData.tokenAssoc[tokensIndex(lastToken)] };
    }
  }

  /***********
   * GRAMMAR *
   ***********/

  /* CHAR { char :: str[0] } <DELETER OPTIONAL>
   *
   *
   * Regex { Regex* } { delete $; }
   * Regex := Alts                             { new Alt(std::move($0)) }
   *        | Concats                          { new Concat(std::move($0)) }
   *        | Regex STAR                       { new Star($0) }
   *        | CARET Regex                      { new Not($1) }
   *        | LBRACKET CHAR DASH CHAR RBRACKET { new Range($1, $3) }
   *        | LPAREN Regex RPAREN              { $1 }
   *        | CHAR                             { new Character($0) }
   *
   * Alts { RegexVector }
   * Alts := Regex BAR Regex { RegexVector($0, $2) }
   *       | Alts BAR Regex  { RegexVector(std::move($0), $2) }
   *
   * Concats { RegexVector* }
   * Concats := Regex Regex   { RegexVector($0, $1) }
   *          | Concats Regex { RegexVector(std::move($0), $1) }
   *
   *
   * */

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
  constexpr int RNOT = 4;
  constexpr int RRANGE = 5;
  constexpr int RGROUP = 6;
  constexpr int RCHAR = 7;
  constexpr int AREGEX = 8;
  constexpr int AALT = 9;
  constexpr int CREGEX = 10;
  constexpr int CCONCAT = 11;


  GrammarData GRAMMAR_DATA = { { S,
                                REGEX,
                                REGEX,
                                REGEX,
                                REGEX,
                                REGEX,
                                REGEX,
                                REGEX,
                                ALTS,
                                ALTS,
                                CONCATS,
                                CONCATS },
                              { NONE /* SCONC */,
                                NONE,
                                NONE,
                                NONE,
                                NONE,
                                NONE,
                                NONE,
                                NONE,
                                NONE,
                                NONE,
                                4,
                                4 },
                              { 1, 5, 3, 7, 7, 8, 8, NONE, 4 },
                              { Assoc::LEFT,
                                Assoc::LEFT,
                                Assoc::NOT,
                                Assoc::NONE,
                                Assoc::NONE,
                                Assoc::NONE,
                                Assoc::NONE,
                                Assoc::NONE,
                                Assoc::LEFT } };


  const Grammar GRAMMAR = {
    /* S */ { GrammarRule{ SCONC, { REGEX } } },
    /* REGEX */
    {
        GrammarRule{ RALT, { ALTS } },
        GrammarRule{ RCONCAT, { CONCATS } },
        GrammarRule{ RSTAR, { REGEX, STAR } },
        GrammarRule{ RNOT, { CARET, REGEX } },
        GrammarRule{ RRANGE, { LBRACKET, CHAR, DASH, CHAR, RBRACKET } },
        GrammarRule{ RGROUP, { LPAREN, REGEX, RPAREN } },
        GrammarRule{ RCHAR, { CHAR } },
    },
    /* ALTS */
    {
        GrammarRule{ AREGEX, { REGEX, BAR, REGEX } },
        GrammarRule{ AALT, { ALTS, BAR, REGEX } },
    },
    /* CONCATS */
    {
        GrammarRule{ CREGEX, { REGEX, REGEX } },
        GrammarRule{ CCONCAT, { CONCATS, REGEX } },
    }
  };

  using CondensedDFA = DFA<RuleData, int>;
  using CondensedNode = CondensedDFA::Node;

  CondensedDFA buildRegexParser() {
    return buildParserDFA(GRAMMAR, 9).convert<RuleData>(
      [](const DFARuleSet& ruleSet) { return condenseRuleSet(ruleSet, GRAMMAR_DATA); }
    );
  }

  struct Start {
    Start(Regex** r) : r_(r) {}
    ~Start() { delete r_; }
    Regex** r_;
  };

  struct StackObj {
    // Can't delete from here since it is a void*, see constructObj
    void* obj;
    int symbol;

    void deleteObj() const noexcept;
    void deleteObjPtr() const noexcept;
  };


  void StackObj::deleteObj() const noexcept {
    switch (symbol) {
      case REGEX:
        delete (*(Regex**)obj);
        delete (Regex**)obj;
        break;
      case ALTS:
        delete (RegexVector*)obj;
        break;
      case CONCATS:
        delete (RegexVector*)obj;
        break;
      case CHAR:
        delete (char*)obj;
        break;
      default:
        return;
    }
  }

  void StackObj::deleteObjPtr() const noexcept {
    switch (symbol) {
      case REGEX:
        delete (Regex**)obj;
        break;
      case ALTS:
        delete (RegexVector*)obj;
        break;
      case CONCATS:
        delete (RegexVector*)obj;
        break;
      case CHAR:
        delete (char*)obj;
        break;
      default:
        return;
    }
  }

  // TODO: Remove throw and make noexcept when done
  /* Construct a concrete object */
  void* constructObj(int concrete, StackObj* args) {
    switch (concrete) {
      case RALT:
        return new Regex*(new Alt(std::move(*(RegexVector*)args[0].obj)));
      case RCONCAT:
        return new Regex*(new Concat(std::move(*(RegexVector*)args[0].obj)));
      case RSTAR:
        return new Regex*(new Star(*(Regex**)args[0].obj));
      case RNOT:
        return new Regex*(new Not(*(Regex**)args[1].obj));
      case RRANGE:
        return new Regex*(new Range(*(char*)args[1].obj, *(char*)args[3].obj));
      case RGROUP:
        return new Regex*(*(Regex**)args[1].obj);
      case RCHAR:
        return new Regex*(new Character(*(char*)args[0].obj));
      case AREGEX:
        return new RegexVector(RegexVector(*(Regex**)args[0].obj, *(Regex**)args[2].obj));
      case AALT:
        return new RegexVector(
            RegexVector(std::move(*(RegexVector*)args[0].obj), *(Regex**)args[2].obj));
      case CREGEX:
        return new RegexVector(RegexVector(*(Regex**)args[0].obj, *(Regex**)args[1].obj));
      case CCONCAT:
        return new RegexVector(
            RegexVector(std::move(*(RegexVector*)args[0].obj), *(Regex**)args[1].obj));
      case SCONC:
        return new Start((Regex**)args[0].obj);
      default:
        throw std::invalid_argument("Can't construct. Out of options.");
    }
  }

  StackObj construct(int concrete, StackObj* args, const vector<int>& concToSym) {
    return StackObj{ constructObj(concrete, args), concToSym[concrete] };
  }

  void conflictWarning(const DFARule& rule) {
    cerr << "WARNING: Shift reduce conflict for\n\t" << rule << endl;
  }


  // TODO: Is it possible to have more than one rule able to be reduced? If so, handle it.
  int tryReduce(
      const CondensedNode* node,
      int nextToken,
      const std::vector<StackObj>& stk,
      const vector<int> tokenPrecs) {

    const RuleData& ruleData = node->getValue();

    // No reducible rule, so try shifting
    if (!ruleData.reducibleRule.has_value()) {
      return NONE;
    }

    const DFARule& rule = *ruleData.reducibleRule;

    // Check if rule matches the stack
    auto eqlLambda = [](int symbol, const StackObj& stkObj) {
      return stkObj.symbol == symbol;
    };
    if (!equal(rule.symbols.cbegin(), rule.symbols.cend(), stk.cbegin(), move(eqlLambda))) {
      return NONE;
    }

    // Can't shift, so reduce
    if (!node->getTransitions().contains(nextToken)) {
      return ruleData.reducibleRule->concrete;
    }

    // TODO: Handle nonassociativity
    // If both are options, then reduce if one of the following is true:
    // - precedence of rule is higher than that of next token
    // - precedence of rule is the same of that of next token and the rule's
    //   last token is left-associative
    int shiftPrecedence = tokenPrecs[tokensIndex(nextToken)];

    // Unspecified precedence -> conflict! (Resolve by shifting)
    if (ruleData.precedence == 0 && shiftPrecedence == 0) {
      conflictWarning(*ruleData.reducibleRule);
      return NONE;
    }

    if (ruleData.precedence > shiftPrecedence) {
      return ruleData.reducibleRule->concrete;
    } else if (ruleData.precedence == shiftPrecedence) {
      if (ruleData.assoc == Assoc::LEFT) {
        return ruleData.reducibleRule->concrete;
      } else if (ruleData.assoc == Assoc::NONE) {
        // Unspecified associativity -> conflict! (Resolve by shifting)
        conflictWarning(*ruleData.reducibleRule);
        return NONE;
      }
    }

    // shift precedence is higher, or same and right associative, so shift
    return NONE;
  }


  /* Deep delete StackObj::obj starting at index i */
  void cleanPtrsFrom(const std::vector<StackObj>& stackObjs, size_t i) {
    size_t size = stackObjs.size();
    for (; i < size; ++i) {
      stackObjs[i].deleteObj();
    }
  }

  Regex* parse(
      const CondensedDFA& dfa,
      std::vector<StackObj>& inputTokens,
      const GrammarData& grammarData) {

    using namespace std;

    StackObj& firstToken = inputTokens[0];
    const CondensedNode* currentNode = dfa.step(dfa.getRoot(), firstToken.symbol);
    if (currentNode == nullptr) {
      cleanPtrsFrom(inputTokens, 0);
      return nullptr;
    }

    vector<StackObj> stk = { move(firstToken) };
    size_t i = 1;
    size_t inputSize = inputTokens.size();

    // Stop when we have consumed all the input and the root of grammar
    // is the only thing on the stack
    while (!(i == inputSize && stk.size() == 1 && stk[0].symbol == S)) {
      int nextInputToken = i == inputSize ? NONE : inputTokens[i].symbol;
      int concrete = tryReduce(currentNode, nextInputToken, stk, grammarData.tokenPrecs);
      // Reduce
      if (concrete != NONE) {
        // Construct the new object, pop the arguments off the stack,
        // and push the new object onto it.
        size_t reduceStart = stk.size() - currentNode->getValue().reducibleRule->symbols.size();
        StackObj newObj = construct(
            concrete,
            &stk.data()[reduceStart],
            grammarData.concToSymb);

        // We always add the rule S -> <root_type>, so there is only one thing on
        // the stack if we reduced to S, and we don't want to delete the pointer
        // because this is the object we are returning to the caller
        if (newObj.symbol == S) {
          stk.pop_back();
        } else {
          size_t stkSize = stk.size();
          for (size_t j = 0; j < stkSize - reduceStart; ++j) {
            stk.back().deleteObjPtr();
            stk.pop_back();
          }
        }

        stk.push_back(newObj);

        // Restart the DFA.
        // TODO: Only backtrack as far as the reduction (store path)
        vector<int> stkSymbols;
        transform(stk.begin(), stk.end(), back_inserter(stkSymbols), [](StackObj stkObj) {
          return stkObj.symbol;
        });
        currentNode = dfa.run(stkSymbols);


      } else {  // Shift
        // No more tokens, didn't reduce to S
        if (i == inputSize) {
          cleanPtrsFrom(stk, 0);
          return nullptr;
        }
        StackObj token = inputTokens[i];
        currentNode = CondensedDFA::step(currentNode, token.symbol);
        stk.push_back(move(token));
        // No transition for this token
        if (currentNode == nullptr) {
          cleanPtrsFrom(stk, 0);
          cleanPtrsFrom(inputTokens, i + 1);
          return nullptr;
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
}
