#include "src/language/utils.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;

namespace lang_utils {

string qualifiedName(vector<string> qualifiers, string_view name) {
  if (qualifiers.empty()) {
    return string(name);
  }
  return boost::join(qualifiers, "::").append("::").append(name);
}

}  // namespace lang_utils
