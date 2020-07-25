#include <string>
#include <string_view>
#include <vector>

namespace lang_utils {

inline std::string newLabel() {
  static int i = 0;
  return "_L" + std::to_string(i++);
}

std::string qualifiedName(std::vector<std::string> qualifiers, std::string_view name);

const std::string THIS = "this";

}
