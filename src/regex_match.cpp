#include "regex_match.hpp"

using namespace std;


/*************
 * Regex DFA *
 *************/

static int NONE = 0;
static constexpr char alphabet[] =
    " !\"#$%&\'()*+,-./"
    "\\0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~";

/* BFS with regex derivative expansion */
RgxDFA buildRegexDFA(const RgxPtr rgx) {
  RgxDFA dfa(rgx);
  queue<const RgxDFA::Node*> q;
  q.push(dfa.getRoot());

  while (!q.empty()) {
    const RgxDFA::Node* node = q.front();
    RgxPtr rgx = node->getValue();
    q.pop();

    char c;
    for (size_t i = 0; (c = alphabet[i]) != '\0'; ++i) {
      const RgxDFA::Node* successor = dfa.addTransition(node, c, rgx->getDeriv(c));
      if (successor) {
        q.push(successor);
      }
    }
  }

  return dfa;
}


/* Accepting states in DFA are those for which the regex is nullable */
bool matches(const string& pattern, const string& input) {
  RgxPtr rgx = RgxPtr(parse(buildParserDFA(), lex(pattern)));
  RgxDFA dfa = buildRegexDFA(move(rgx));
  const RgxDFA::Node* node = dfa.run(vector<char>(input.cbegin(), input.cend()));
  return node->getValue()->isNullable();
}


/********************
 * Merged Regex DFA *
 *******************/

MergedRgxDFA buildMergedRgxDFA(const vector<TokenPattern>& tokenPatterns) {
  DFA_t parserDfa = buildParserDFA();
  // We need the regex DFAs to be live long enough to create the merged DFA,
  // so we store them in this vector. Afterwards, all the pointers inside the
  // merged DFA will be deleted, but we will no longer need them.
  vector<RgxDFA> rgxDfas;
  rgxDfas.reserve(tokenPatterns.size());

  // Initialize the root of the merged DFA by creating DFAs for each regex
  // and mapping their roots to the appropriate token value
  vector<pair<const RgxDFA::Node*, int>> initialStates;
  int initialToken = NONE;
  for (const TokenPattern& tokenPattern : tokenPatterns) {
    RgxPtr rgx = RgxPtr(parse(parserDfa, lex(tokenPattern.first)));
    // Invalid regex
    if (!rgx) {
      throw runtime_error("Invalid regex " + tokenPattern.first);
    }
    RgxDFA rgxDfa = buildRegexDFA(move(rgx));
    initialStates.push_back({ rgxDfa.getRoot(), tokenPattern.second });
    if (rgxDfa.getRoot()->getValue()->isNullable()) {
      // Accepting the empty string will likely result in an infinite loop
      cerr << "WARNING: The regex \"" << tokenPattern.first << "\" accepts the empty string" << endl;
      // Multiple regex DFAs accept the empty string. We pick the regex that was listed first.
      if (initialToken == NONE) {
        initialToken = tokenPattern.second;
      }
    }
    rgxDfas.push_back(move(rgxDfa));
  }

  MergedRgxDFA mergedDfa({ move(initialStates), initialToken });

  // BFS to build the merged DFA
  queue<const MergedRgxDFA::Node*> q;
  q.push(mergedDfa.getRoot());

  while (!q.empty()) {
    const MergedRgxDFA::Node* mergedNode = q.front();
    q.pop();
    char c;
    for (size_t i = 0; (c = alphabet[i]) != '\0'; ++i) {
      vector<pair<const RgxDFA::Node*, int>> newStates;
      int newToken = NONE;
      for (const auto& nodeAndToken : mergedNode->getValue().states) {
        const RgxDFA::Node* node = nodeAndToken.first;
        int token = nodeAndToken.second;
        // Since Regex DFAs are actual DFAs (they have a transition for every symbol
              // in the alphabet), each node always has a successor for every transition.
        const RgxDFA::Node* successor = RgxDFA::step(node, c);
        newStates.push_back({ successor, token });
        if (successor->getValue()->isNullable()) {
          // Multiple regex DFAs accept the same string. We pick the regex that was listed first.
          if (newToken == NONE) {
            newToken = token;
          }
        }
      }
      // Add transition to the merged node and add it to the queue if it did not already
      // exist in the merged DFA.
      // Again, since Regex DFAs are actual DFAs, we are guaranteed to have a valid
      // state in newStates for each Regex DFA.
      const MergedRgxDFA::Node* mergedSuccessor =
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

/* Value conversion function: V1 -> V2 */
int mergedDataToToken(const MergeData& mergedData) { return mergedData.token; }

/* Value string representation */
// string tokenToString(int token) {
//   ostringstream out;
//   out << token;
//   return out.str();
// }

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


void writeRegexDFA(const vector<TokenPattern> patterns, const string& grammarFile, const string& outFile, const string& namespaceName) {
  buildMergedRgxDFA(patterns).writeToFile(
      outFile,
      namespaceName,
      "#include \"" + grammarFile + "\"",
      "int",
      "char",
      mergedDataToToken,
      [](int n) { return to_string(n); },
      charToString);
}
