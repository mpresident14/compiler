#include "config_parse.hpp"

#include <vector>
#include <string_view>
#include <stdexcept>
#include <cctype>
#include <fstream>
#include <iostream>

using namespace std;

namespace {

  string addlHdrIncludes;
  string addlCode;
  GrammarData grammarData;
  vector<Token>& tokens = grammarData.tokens;
  vector<Concrete>& concretes = grammarData.concretes;
  vector<Variable>& variables = grammarData.variables;


  /*********
   * UTILS *
   *********/

  void parseError(const string& expected) {
    throw invalid_argument("Expected: " + expected);
  }

  void consume(char c, string_view& input) {
    if (!input.empty() && input[0] == c) {
      input = input.substr(1);
    } else {
      parseError(string(1, c));
    }
  }

  void consumeWhitespace(string_view& input) {
    size_t len = input.size();
    size_t i = 0;
    while (i < len && isspace(input[i])) {
      ++i;
    }
    input = input.substr(i);
  }

  string_view parseWord(string_view& input) {
    size_t len = input.size();
    size_t i = 0;
    while (i < len && isalpha(input[i])) {
      ++i;
    }
    string_view word = input.substr(0, i);
    input = input.substr(i);
    return word;
  }


  // TODO: What about brackets in quotes
  string_view parseBracketed(string_view& input) {
    consume('{', input);
    int count = 1;
    size_t len = input.size();

    for (size_t i = 0; i < len; ++i) {
      char c = input[i];
      if (c == '{') {
        ++count;
      } else if (c == '}') {
        --count;
        if (count == 0) {
          string_view contents = input.substr(0, i - 1);
          input = input.substr(i);
          return contents;
        }
      }
      ++i;
    }

    // Reached the end of the input without a matching '}'
    parseError("}");
  }


  string_view parseQuoted(string_view& input) {
    consume('"', input);
    bool escaped = false;
    size_t len = input.size();
    size_t i = 0;
    while (i < len) {
      char c = input[i];
      if (escaped) {
        escaped = false;
        continue;
      }
      if (c == '\\') {
        escaped = true;
      } else if (c == '}') {
        --count;
      }
      ++i;
    }

    // Reached the end of the input without a matching }
    parseError("\"");

    string_view contents = input.substr(0, i - 1);
    input = input.substr(i);
    return contents;
  }

  /**********
   * TOKENS *
   **********/


  /************
   * SECTIONS *
   ************/


  void parseHeaderSection(string_view& input) {
    consume('%', input);
    if (parseWord(input) != "header") {
      parseError("header");
    }
    consumeWhitespace(input);
    addlHdrIncludes = parseBracketed(input);
  }

  void parseSourceSection(string_view& input) {
    consume('%', input);
    if (parseWord(input) != "source") {
      parseError("source");
    }
    consumeWhitespace(input);
    addlCode = parseBracketed(input);
  }
}


GrammarData parseConfig(const string& fileName) {
  ifstream config;
  config.open(fileName);
  string inputStr(istreambuf_iterator<char>{config}, istreambuf_iterator<char>{});
  string_view input(inputStr);
  config.close();

  parseHeaderSection(input);
  consumeWhitespace(input);
  parseSourceSection(input);
  return grammarData;
}

int main() {
  parseConfig("test/expr_config.txt");
  cout << addlHdrIncludes << endl;
  cout << addlCode << endl;
}
