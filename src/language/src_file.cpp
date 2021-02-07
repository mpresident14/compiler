#include "src/language/src_file.hpp"
#include "src/language/parser.hpp"

using namespace std;
namespace fs = filesystem;

namespace language {

/***********
 * SrcFile *
 ***********/

SrcFile::SrcFile(vector<Import> &&imports, vector<DeclPtr> &&decls)
    : imports_(move(imports)), ctx_(nullptr) {
  for (DeclPtr &decl : decls) {
    if (decl->getCategory() == Decl::Category::CLASS) {
      classes_.emplace_back(static_cast<ClassDecl *>(decl.release()));
    } else {
      funcs_.emplace_back(static_cast<Func *>(decl.release()));
    }
  }
}

namespace {
fs::path toCanonical(const fs::path &parent, fs::path imported,
                     error_code &ec) {
  if (imported.is_relative()) {
    return fs::canonical(fs::path(parent).replace_filename(imported), ec);
  }
  return fs::canonical(imported, ec);
}
} // namespace

void SrcFile::initContext(
    const fs::path &filePath,
    unordered_map<string, unique_ptr<SrcFile>> &initializedProgs,
    const vector<SrcFile> &builtIns,
    const shared_ptr<unordered_map<int, Ctx::ClassInfo *>> &classIds) {
  // Create a new context
  ctx_ = make_unique<Ctx>(filePath.c_str(), classIds);

  // Add builtin declarations to the context
  for (const SrcFile &prog : builtIns) {
    ctx_->includeDecls(*prog.ctx_);
  }

  // Go through the imports and build the context tree so we have access
  // to all imported declarations.
  error_code ec;
  for (const Import &imported : imports_) {
    fs::path importPath = toCanonical(filePath, imported.filename, ec);
    if (ec) {
      ostream &err = ctx_->getLogger().logError(imported.line, ec.message());
      err << " \"" << imported.filename << "\"";
      continue;
    }

    auto progsIter = initializedProgs.find(importPath);
    SrcFile *prog = nullptr;
    if (progsIter == initializedProgs.end()) {
      // We haven't initialized this program yet
      // Mark as initialized before recursing to allow circular dependencies
      try {
        // We use a nullptr as a placeholder to mark that we already tried to
        // parse the program. This way, we don't try parsing it again if it is
        // imported somewhere else
        auto iter = initializedProgs.emplace(importPath, nullptr).first;
        iter->second = make_unique<SrcFile>(parser::parse(importPath));
        prog = iter->second.get();
        prog->initContext(importPath, initializedProgs, builtIns, classIds);
      } catch (const parser::ParseException &e) {
        cerr << e.what() << "\n(Imported at " << filePath << ", line "
             << imported.line << ")\n"
             << endl;
      }
    } else {
      prog = progsIter->second.get();
    }

    // If the program parsed, put the import's context in our context tree
    if (prog &&
        !ctx_->getCtxTree().addCtx(imported.filename, prog->ctx_.get())) {
      ctx_->getLogger().logNote(imported.line,
                                "Duplicate import '" + imported.filename + "'");
    }
  }

  // Add this program's declarations to its own context. We add class names
  // before functions/methods so that parameters and return types that are
  // classes will already be in the context.
  for (const unique_ptr<ClassDecl> &cls : classes_) {
    cls->addToCtx(*ctx_);
  }
  for (const unique_ptr<Func> &fn : funcs_) {
    fn->addToCtx(*ctx_);
  }
} // namespace language

assem::SrcFile SrcFile::toAssemProg() const { return toImProg().toAssemProg(); }

im::SrcFile SrcFile::toImProg() const {
  vector<im::DeclPtr> imDecls;
  for (const unique_ptr<ClassDecl> &cls : classes_) {
    cls->toImDecls(imDecls, *ctx_);
  }
  for (const unique_ptr<Func> &fn : funcs_) {
    fn->toImDecls(imDecls, *ctx_);
  }
  return im::SrcFile(move(imDecls));
}

} // namespace language
