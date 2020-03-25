#include "tokenize.hpp"
#include "test/gen_regex_dfa.hpp"

using namespace std;

static void consumeWhiteSpace(string_view& input) {
  size_t i = 0;
  while (isspace(input[i])){
    ++i;
  }
  input = input.substr(i);
}

/* Step through the merged regex DFA and return the token corresponding
 * to the longest matching prefix */
static optional<StackObj> getToken(string_view& input, const regex_dfa::Node* dfaRoot) {
  size_t i = 0;
  const size_t len = input.size();
  size_t lastAcceptingPos;
  Symbol lastAcceptingToken = Symbol::NONE;
  const regex_dfa::Node* currentNode = dfaRoot;

  while (currentNode) {
    // Check if this is an accepting state, and if so,
    // record the token type and the position in the input
    Symbol token = static_cast<Symbol>(currentNode->v_);
    if (token != Symbol::NONE) {
      lastAcceptingToken = token;
      lastAcceptingPos = i;
    }

    // No more input
    if (i == len) {
      break;
    }
    // Advance to the next state
    currentNode = currentNode->step(input[i++]);
  }

  // Never reached an accepting state
  if (lastAcceptingToken == Symbol::NONE) {
    return {};
  }

  // Construct object from the matching prefix
  StackObj stackObj = constructTokenObj(lastAcceptingToken, input.substr(0, lastAcceptingPos));
  // Discard matching prefix so we can reuse this string_view for the next token
  input = input.substr(lastAcceptingPos);
  return { move(stackObj) };
}



vector<StackObj> tokenize(const string& input) {
  if (input.empty()) {
    return {};
  }

  vector<StackObj> tokens;
  string_view inputView = input;

  const regex_dfa::Node* dfaRoot = regex_dfa::root.get();

  consumeWhiteSpace(inputView);
  while (!inputView.empty()) {
    optional<StackObj> optionalObj = getToken(inputView, dfaRoot);
    if (optionalObj.has_value()) {
      tokens.push_back(move(*optionalObj));
    } else {
      ostringstream error;
      vector<Symbol> prevTokens;
      auto startIter = tokens.size() < 25 ? tokens.cbegin() : tokens.cend() - 25;
      transform(
          move(startIter), tokens.cend(), back_inserter(prevTokens), [](const StackObj& stackObj) {
            return stackObj.symbol;
          });
      error << "Lexer error at: " << inputView.substr(0, 25) << '\n'
            << "Previous tokens were: " << prevTokens;
      for_each(tokens.cbegin(), tokens.cend(), mem_fun_ref(&StackObj::deleteObj));
      throw runtime_error(error.str());
    }

    consumeWhiteSpace(inputView);
  }

  return tokens;
}
