#include <string>
#include <string_view>
#include <vector>

inline std::string newLabel() {
  static int i = 0;
  return "L" + std::to_string(i++);
}

std::vector<std::string> splitPath(std::string_view importPath);
std::string qualifiedName(std::vector<std::string> qualifiers, std::string_view name);
