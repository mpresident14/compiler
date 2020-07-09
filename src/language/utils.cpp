#include "src/language/utils.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;

vector<string> splitPath(string_view importPath) {
  importPath = importPath.substr(0, importPath.size() - sizeof(".prez") + 1);
  vector<string> pathParts;
  boost::split(pathParts, importPath, [](char c) { return c == '/'; });
  return pathParts;
}


string qualifiedName(vector<string> qualifiers, string_view name) {
  if (qualifiers.empty()) {
    return string(name);
  }
  return boost::join(qualifiers, "::").append("::").append(name);
}
