#include "regex_eval.hpp"

using namespace std;


/*************
 * Regex DFA *
 *************/
namespace {

  constexpr char alphabet[] =
      " !\"#$%&\'()*+,-./"
      "\\0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`"
      "abcdefghijklmnopqrstuvwxyz{|}~";

  using RgxDFA = DFA<RgxPtr, char>;

  /* BFS with regex derivative expansion */
  RgxDFA buildRegexDFA(const RgxPtr rgx) {
    RgxDFA dfa(rgx);
    queue<RgxDFA::Node*> q;
    q.push(dfa.getRoot());

    while (!q.empty()) {
      RgxDFA::Node* node = q.front();
      RgxPtr rgx = node->getValue();
      q.pop();

      char c;
      for (size_t i = 0; (c = alphabet[i]) != '\0'; ++i) {
        RgxDFA::Node* successor = dfa.addTransition(node, c, rgx->getDeriv(c));
        if (successor) {
          q.push(successor);
        }
      }
    }

    return dfa;
  }


  /********************
   * Merged Regex DFA *
   *******************/

  /* Maps regex string to token enum type (Symbol::NONE defined to be 0) */
  using TokenPattern = pair<string, int>;

  struct MergeData {
    vector<pair<RgxDFA::Node*, int>> states;
    int token;
  };
}  // namespace

namespace std {
  template <>
  struct hash<MergeData> {
    size_t operator()(const MergeData& mergeData) const noexcept {
      const vector<pair<RgxDFA::Node*, int>>& states = mergeData.states;
      // mergeData will not be empty
      return (size_t)accumulate(
          states.cbegin() + 1,
          states.cend(),
          (uintptr_t)states[0].first,
          [](uintptr_t n, pair<const RgxDFA::Node*, int> node) {
            return n ^ (uintptr_t)node.first;
          });
    }
  };
}  // namespace std

namespace {

  inline bool operator==(const MergeData& md1, const MergeData& md2) noexcept {
    return md1.states == md2.states;
  }

  using MergedRgxDFA = DFA<MergeData, char>;


  MergedRgxDFA buildMergedRgxDFA(const GrammarData& grammarData) {
    // We need the regex DFAs to be live long enough to create the merged DFA,
    // so we store them in this vector. Afterwards, all the pointers inside the
    // merged DFA will be deleted, but we will no longer need them.
    size_t numTokens = grammarData.tokens.size();
    vector<RgxDFA> rgxDfas;
    rgxDfas.reserve(numTokens);

    // Initialize the root of the merged DFA by creating DFAs for each regex
    // and mapping their roots to the appropriate token value
    vector<pair<RgxDFA::Node*, int>> initialStates;
    int initialToken = NONE;
    for (size_t i = 0; i < numTokens; ++i) {
      const Token& token = grammarData.tokens[i];
      RgxPtr rgx = parse(token.regex);
      // Invalid regex
      if (!rgx) {
        throw runtime_error("Invalid regex " + token.regex);
      }
      RgxDFA rgxDfa = buildRegexDFA(move(rgx));
      initialStates.push_back({ rgxDfa.getRoot(), tokenToFromIndex(i) });
      if (rgxDfa.getRoot()->getValue()->isNullable()) {
        // Accepting the empty string will likely result in an infinite loop
        cerr << "WARNING: The regex \"" << token.regex
             << "\" accepts the empty string" << endl;
        // Multiple regex DFAs accept the empty string. We pick the regex that
        // was listed first.
        if (initialToken == NONE) {
          initialToken = tokenToFromIndex(i);
        }
      }
      rgxDfas.push_back(move(rgxDfa));
    }

    MergedRgxDFA mergedDfa(MergeData{ move(initialStates), initialToken });

    // BFS to build the merged DFA
    queue<MergedRgxDFA::Node*> q;
    q.push(mergedDfa.getRoot());

    while (!q.empty()) {
      MergedRgxDFA::Node* mergedNode = q.front();
      q.pop();
      char c;
      for (size_t i = 0; (c = alphabet[i]) != '\0'; ++i) {
        vector<pair<RgxDFA::Node*, int>> newStates;
        int newToken = NONE;
        for (auto& nodeAndToken : mergedNode->getValue().states) {
          RgxDFA::Node* node = nodeAndToken.first;
          int token = nodeAndToken.second;
          // Since Regex DFAs are actual DFAs (they have a transition for every
          // symbol in the alphabet), each node always has a successor for every
          // transition.
          RgxDFA::Node* successor = RgxDFA::step(node, c);
          newStates.push_back({ successor, token });
          if (successor->getValue()->isNullable()) {
            // Multiple regex DFAs accept the same string. We pick the regex
            // that was listed first.
            if (newToken == NONE) {
              newToken = token;
            }
          }
        }
        // Add transition to the merged node and add it to the queue if it did
        // not already exist in the merged DFA. Again, since Regex DFAs are
        // actual DFAs, we are guaranteed to have a valid state in newStates for
        // each Regex DFA.
        MergedRgxDFA::Node* mergedSuccessor =
            mergedDfa.addTransition(mergedNode, c, { newStates, newToken });
        if (mergedSuccessor) {
          q.push(mergedSuccessor);
        }
      }
    }

    return mergedDfa;
  }


  /**********************************
   * Write Merged Regex DFA to file *
   **********************************/

  /* Value string representation */
  string charToString(char c) {
    string str(1, '\'');
    str.reserve(4);
    if (c == '\\' || c == '\'') {
      str.append(1, '\\');
    }
    str.append(1, c);
    str.append(1, '\'');
    return str;
  }

}  // namespace


void rgxDFAToCode(ostream& out, const GrammarData& grammarData) {
  buildMergedRgxDFA(grammarData).streamAsCode(
      out,
      "int",
      "char",
      [](const MergeData& mergedData) { return mergedData.token; },
      [](int n) { return to_string(n); },
      charToString);
}
