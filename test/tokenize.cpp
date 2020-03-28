#include "test/tokenize.hpp"
#include "test/gen_lexer.hpp"

using namespace std;


void deleteObj(const StackObj& stackObj) noexcept {
  switch (stackObj.symbol) {
    case INT:
      delete (int*)stackObj.obj;
      break;
    case CHAR:
      delete (char*)stackObj.obj;
      break;
    case IDENTIFIER:
      delete (string*)stackObj.obj;
      break;
    default:
      return;
  }
}

StackObj constructTokenObj(int token, const std::string_view& str) {
  switch (token) {
    case INT:
      return { new int(atoi(str.data())), token };
      break;
    case CHAR:
      return { new char(str[0]), token };
    case IDENTIFIER:
      return { new std::string(str), token };
    default:
      return { nullptr, token };
  }
}

static void consumeWhiteSpace(string_view& input) {
  size_t i = 0;
  while (isspace(input[i])) {
    ++i;
  }
  input = input.substr(i);
}

/* Step through the merged regex DFA and return the token corresponding
 * to the longest matching prefix */
static optional<StackObj> getToken(string_view& input, Node* dfaRoot) {
  size_t i = 0;
  const size_t len = input.size();
  size_t lastAcceptingPos;
  int lastAcceptingToken = NONE;
  Node* currentNode = dfaRoot;

  while (currentNode) {
    // Check if this is an accepting state, and if so,
    // record the token type and the position in the input
    int token = currentNode->v_;
    if (token != NONE) {
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
  if (lastAcceptingToken == NONE) {
    return {};
  }

  // Construct object from the matching prefix
  StackObj stackObj =
      constructTokenObj(lastAcceptingToken, input.substr(0, lastAcceptingPos));
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

  Node* dfaRoot = root.get();

  consumeWhiteSpace(inputView);
  while (!inputView.empty()) {
    optional<StackObj> optionalObj = getToken(inputView, dfaRoot);
    if (optionalObj.has_value()) {
      tokens.push_back(move(*optionalObj));
    } else {
      ostringstream error;
      vector<int> prevTokens;
      auto startIter =
          tokens.size() < 25 ? tokens.cbegin() : tokens.cend() - 25;
      transform(
          move(startIter),
          tokens.cend(),
          back_inserter(prevTokens),
          [](const StackObj& stackObj) { return stackObj.symbol; });
      error << "Lexer error at: " << inputView.substr(0, 25) << '\n'
            << "Previous tokens were: " << prevTokens;
      for_each(tokens.cbegin(), tokens.cend(), deleteObj);
      throw runtime_error(error.str());
    }

    consumeWhiteSpace(inputView);
  }

  return tokens;
}
