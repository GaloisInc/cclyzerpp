#include <llvm/IR/ModuleSlotTracker.h>

#include <memory>
#include <sstream>

#include "ContextManager.hpp"
#include "RefmodeEngine.hpp"

// Forward declaration
namespace llvm {
class raw_string_ostream;
}

// Refmode Policy implementation
class cclyzer::RefmodeEngine::Impl {
 public:
  // Compute refmode for obj, given some context state
  template <typename T>
  auto refmode(const T &obj) -> refmode_t;  // const;

  //-------------------------------------------------
  // Context management
  //-------------------------------------------------

  void enterContext(const llvm::Value &val) { ctx->pushContext(val); }

  void exitContext() { ctx->popContext(); }

  void enterModule(const llvm::Module &module, const std::string &path) {
    slotTracker = std::make_unique<llvm::ModuleSlotTracker>(&module);
    ctx = std::make_unique<ContextManager>(module, path);
  }

  void exitModule() {}

  auto functionContext() -> const llvm::Function * {
    if (ctx->functionContext() != nullptr) {
      return llvm::dyn_cast<llvm::Function>(ctx->functionContext()->anchor);
    }
    return nullptr;
  }

  auto moduleContext() -> const llvm::Module * { return &ctx->module(); }

 protected:
  // Methods that compute refmodes for various LLVM types
  auto refmodeOf(const llvm::Value *Val) -> refmode_t;

  void appendMetadataId(llvm::raw_string_ostream &, const llvm::Metadata &);

  // Compute variable numberings
  static void computeNumbering(
      const llvm::Function *, std::map<const llvm::Value *, unsigned> &);

  // Compute all metadata slots
  void parseMetadata(const llvm::Module *module);

  template <typename T, typename S>
  auto withContext(S &stream) const -> S & {
    for (const auto &it : *ctx) {
      const llvm::Value *anchor = it.anchor;

      // Skip basic blocks
      if (anchor && llvm::isa<llvm::BasicBlock>(*anchor) &&
          !llvm::isa<T>(*anchor)) {
        continue;
}

      stream << it.prefix << ':';

      if (anchor && llvm::isa<T>(*anchor)) { break;
}
    }
    return stream;
  }

  template <typename S>
  auto withGlobalContext(S &stream) const -> S & {
    auto firstCtxt = ctx->begin();

    assert(firstCtxt != ctx->end());
    assert(firstCtxt->anchor == nullptr);
    stream << firstCtxt->prefix << ':';
    return stream;
  }

 private:
  // Slot tracker and context manager
  std::unique_ptr<llvm::ModuleSlotTracker> slotTracker;
  std::unique_ptr<ContextManager> ctx;
};
