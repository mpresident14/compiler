#include "regex_eval.hpp"

using namespace std;


using RgxDFA = DFA<RgxPtr, char>;

static char alphabet[] = R"( !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)";

RgxDFA buildRegexDFA(const RgxPtr rgx) {
  RgxDFA dfa(rgx);
  queue<const RgxDFA::Node*> q;
  q.push(dfa.getRoot());

  while(!q.empty()) {
    const RgxDFA::Node* node = q.front();
    RgxPtr rgx = node->getValue();
    q.pop();
    for (char c : alphabet) {
      const RgxDFA::Node* succNode = dfa.addTransition(node, c, rgx->getDeriv(c));
      if (succNode) {
        q.push(succNode);
      }
    }
  }

  return dfa;
}


bool matches(const string& pattern, const string& input) {
  RgxPtr rgx = RgxPtr(parse(buildParserDFA(), lex(pattern)));
  RgxDFA dfa = buildRegexDFA(rgx);
  const RgxDFA::Node* node = dfa.run(vector<char>(input.cbegin(), input.cend()));
  return node->getValue()->isNullable();
}
