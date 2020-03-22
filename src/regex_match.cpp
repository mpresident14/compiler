#include "regex_match.hpp"

using namespace std;


using RgxDFA = DFA<RgxPtr, char>;

static constexpr char alphabet[] =
    R"( !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)";

/* BFS with regex derivative expansion */
RgxDFA buildRegexDFA(const RgxPtr rgx) {
  RgxDFA dfa(rgx);
  queue<const RgxDFA::Node*> q;
  q.push(dfa.getRoot());

  while (!q.empty()) {
    const RgxDFA::Node* node = q.front();
    RgxPtr rgx = node->getValue();
    q.pop();
    for (const char c : alphabet) {
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


struct MergeData {
  vector<pair<const RgxDFA::Node*, Symbol>> states;
  Symbol token;
};


namespace std {
  template <>
  struct hash<MergeData> {
    size_t operator()(const MergeData& mergeData) const noexcept {
      const vector<pair<const RgxDFA::Node*, Symbol>>& states = mergeData.states;
      // mergeData will not be empty
      return (size_t)accumulate(states.cbegin() + 1,
          states.cend(),
          (uintptr_t)states[0].first,
          [](uintptr_t n, pair<const RgxDFA::Node*, Symbol> node) {
            return n ^ (uintptr_t)node.first;
          });
    }
  };
}  // namespace std

bool operator==(const MergeData& md1, const MergeData& md2) noexcept {
  return md1.states == md2.states;
}


using MergedRgxDFA = DFA<MergeData, char>;

MergedRgxDFA buildMergedRgxDFA(const vector<TokenPattern>& tokenPatterns) {
  DFA_t parserDfa = buildParserDFA();
  // We need the regex DFAs to be live long enough to create the merged DFA,
  // so we store them in this vector. Afterwards, all the pointers inside the
  // merged DFA will be deleted, but we will no longer need them.
  vector<RgxDFA> rgxDfas;
  rgxDfas.reserve(tokenPatterns.size());

  // Initialize the root of the merged DFA by creating DFAs for each regex
  // and mapping their roots to the appropriate token value
  vector<pair<const RgxDFA::Node*, Symbol>> initialStates;
  Symbol initialToken = Symbol::EPSILON;
  for (const TokenPattern& tokenPattern : tokenPatterns) {
    RgxPtr rgx = RgxPtr(parse(parserDfa, lex(tokenPattern.first)));
    // Invalid regex
    if (!rgx) {
      throw runtime_error("Invalid regex + " + tokenPattern.first);
    }
    RgxDFA rgxDfa = buildRegexDFA(move(rgx));
    initialStates.push_back({ rgxDfa.getRoot(), tokenPattern.second });
    if (rgxDfa.getRoot()->getValue()->isNullable()) {
      // Multiple regex DFAs accept the empty string
      if (initialToken != Symbol::EPSILON) {
        cerr << "WARNING: Overlapping regexes for tokens " << initialToken << " and "
             << tokenPattern.second << endl;
      }
      initialToken = tokenPattern.second;
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
    for (const char c : alphabet) {
      vector<pair<const RgxDFA::Node*, Symbol>> newStates;
      Symbol newToken = Symbol::EPSILON;
      for (const auto& nodeAndSymbol : mergedNode->getValue().states) {
        const RgxDFA::Node* node = nodeAndSymbol.first;
        const Symbol token = nodeAndSymbol.second;
        const RgxDFA::Node* successor = RgxDFA::step(node, c);
        if (successor) {
          newStates.push_back({ successor, token });
          if (successor->getValue()->isNullable()) {
            // Multiple regex DFAs accept the same string
            if (newToken != Symbol::EPSILON) {
              cerr << "WARNING: Overlapping regexes for tokens " << newToken << " and " << token
                   << endl;
            }
            newToken = token;
          }
        }
      }
      // Add transition to the merged node and add it to the queue if it did not already
      // exist in the merged DFA
      const MergedRgxDFA::Node* mergedSuccessor =
          mergedDfa.addTransition(mergedNode, c, { newStates, newToken });
      if (mergedSuccessor) {
        q.push(mergedSuccessor);
      }
    }
  }

  return mergedDfa;
}

/* Step through the merged regex DFA and return the token corresponding
 * to the longest matching prefix */
optional<StackObj> getToken(string_view& input, const MergedRgxDFA& dfa) {
  size_t i = 0;
  const size_t len = input.size();
  size_t lastAcceptingPos;
  Symbol lastAcceptingToken = Symbol::EPSILON;
  const MergedRgxDFA::Node* currentNode = dfa.getRoot();

  while (currentNode) {
    // Check if this is an accepting state, and if so,
    // record the token type and the position in the input
    Symbol token = currentNode->getValue().token;
    if (token != Symbol::EPSILON) {
      lastAcceptingToken = token;
      lastAcceptingPos = i;
    }

    // No more input
    if (i == len) {
      break;
    }
    // Advance to the next state
    currentNode = MergedRgxDFA::step(currentNode, input[i++]);
  }

  // Never reached an accepting state
  if (lastAcceptingToken == Symbol::EPSILON) {
    return {};
  }

  // Grab matching prefix
  StackObj stackObj = constructTokenObj(lastAcceptingToken, input.substr(0, lastAcceptingPos));
  // Discard matching prefix so we can reuse this string_view for the next token
  input = input.substr(lastAcceptingPos);
  return { move(stackObj) };
}


vector<StackObj> tokenize(const string& input, const vector<TokenPattern>& tokenPatterns) {
  const MergedRgxDFA dfa = buildMergedRgxDFA(tokenPatterns);
  vector<StackObj> tokens;
  string_view inputView = input;

  while (!inputView.empty()) {
    optional<StackObj> optionalObj = getToken(inputView, dfa);
    if (optionalObj.has_value()) {
      tokens.push_back(*optionalObj);
    } else {
      stringstream error;
      vector<Symbol> prevTokens;
      auto startIter = tokens.size() < 25 ? tokens.cbegin() : tokens.cend() - 25;
      transform(
          move(startIter), tokens.cend(), back_inserter(prevTokens), [](const StackObj& stackObj) {
            return stackObj.symbol;
          });
      error << "Lexer error at: " << inputView.substr(0, 25) << '\n'
            << "Previous tokens were: " << prevTokens;
      throw runtime_error(error.str());
    }
  }

  return tokens;
}
