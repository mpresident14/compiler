#ifndef SRC_FILE_HPP
#define SRC_FILE_HPP

#include "src/assembly/assembly.hpp"
#include "src/intermediate/intermediate.hpp"
#include "src/language/decl.hpp"
#include "src/language/ctx.hpp"
#include "src/language/type.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace language {

/***********
 * SrcFile *
 ***********/

class SrcFile {
public:
  struct Import {
    std::string filename;
    size_t line;
  };

  SrcFile(std::vector<Import>&& imports, std::vector<DeclPtr>&& decls);
  im::SrcFile toImProg() const;
  assem::SrcFile toAssemProg() const;
  void initContext(
      const std::filesystem::path& filename,
      std::unordered_map<std::string, std::unique_ptr<SrcFile>>& initializedProgs,
      const std::vector<SrcFile>& builtIns,
      const std::shared_ptr<std::unordered_map<int, Ctx::ClassInfo*>>& classIds);

  std::vector<Import> imports_;
  std::vector<std::unique_ptr<ClassDecl>> classes_;
  std::vector<std::unique_ptr<Func>> funcs_;
  std::unique_ptr<Ctx> ctx_;
};


}  // namespace language

#endif  // SRC_FILE_HPP
