#ifndef CONTEXT_MANAGER_HPP_
#define CONTEXT_MANAGER_HPP_

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include <map>
#include <sstream>
#include <utility>
#include <vector>

namespace cclyzer {
// Class that encapsulates any state that the refmode engine needs
class ContextManager {
  // Single context item
  struct Context {
    Context(const llvm::Value& v, std::string prefix)
        : anchor(&v),
          prefix(std::move(prefix)),
          isFunction(llvm::isa<llvm::Function>(v)) {}

    Context(std::string prefix)
        : anchor(nullptr), prefix(std::move(prefix)), isFunction(false) {}

    // Container of local context. Can be global variable,
    // function, block, instruction, etc.
    const llvm::Value* anchor;

    // Mapping numbers to unnamed values
    std::map<const llvm::Value*, unsigned> numbering;

    std::string prefix;

    bool isFunction;
  };

 public:
  // Type definitions
  using context = Context;
  using iterator = std::vector<context>::iterator;
  using const_iterator = std::vector<context>::const_iterator;
  using reverse_iterator = std::vector<context>::reverse_iterator;
  using const_reverse_iterator = std::vector<context>::const_reverse_iterator;

  ContextManager(const llvm::Module& module, const std::string& path)
      : mod(module) {
    // Compute global prefix for this module
    std::stringstream prefix;
    prefix << '<' << path << '>' << std::flush;

    // Add context
    contexts.emplace_back(prefix.str());
  }

  virtual ~ContextManager() { contexts.pop_back(); }

  /// Record that a local context has been entered.  ctx is an IR
  /// "container" of some sort which is being considered for
  /// structural equivalence: global variables, functions, blocks,
  /// instructions, etc.
  void pushContext(const llvm::Value& ctx) {
    std::string prefix;

    // Compute prefix for fully qualified value names under given
    // context

    if (const auto* fctx = llvm::dyn_cast<llvm::Function>(&ctx)) {
      prefix = fctx->getName();
      instrIndex = 0;
      iFunctionCtx = contexts.size();
    } else if (const auto* bbctx = llvm::dyn_cast<llvm::BasicBlock>(&ctx)) {
      prefix = bbctx->getName();
    } else if (llvm::isa<llvm::Instruction>(ctx)) {
      prefix = std::to_string(instrIndex++);
      constantIndex = 0;
    }

    contexts.emplace_back(ctx, prefix);
  }

  /// Record that a local context has been exited.
  void popContext() {
    if (contexts.back().isFunction) {
      iFunctionCtx = SIZE_MAX;
    }

    contexts.pop_back();
  }

  auto begin() -> iterator { return contexts.begin(); }
  auto end() -> iterator { return contexts.end(); }

  [[nodiscard]] auto begin() const -> const_iterator {
    return contexts.begin();
  }
  [[nodiscard]] auto end() const -> const_iterator { return contexts.end(); }

  auto rbegin() -> reverse_iterator { return contexts.rbegin(); }
  auto rend() -> reverse_iterator { return contexts.rend(); }

  [[nodiscard]] auto rbegin() const -> const_reverse_iterator {
    return contexts.rbegin();
  }
  [[nodiscard]] auto rend() const -> const_reverse_iterator {
    return contexts.rend();
  }

  [[nodiscard]] inline auto instrCount() const -> unsigned {
    return instrIndex;
  }
  inline auto constantCount() -> unsigned { return constantIndex++; }
  [[nodiscard]] inline auto module() const -> const llvm::Module& {
    return mod;
  }

  [[nodiscard]] inline auto functionContext() const -> const context* {
    if (iFunctionCtx == SIZE_MAX) {
      return nullptr;
    }

    const context& functionCtx = contexts[iFunctionCtx];
    return functionCtx.isFunction ? &functionCtx : nullptr;
  }

 private:
  // Tracking local contexts
  std::vector<Context> contexts;

  // The vector index containing a function context
  std::size_t iFunctionCtx{SIZE_MAX};

  // Current module and path
  const llvm::Module& mod;

  // Instruction and constant indices
  unsigned instrIndex{0};
  unsigned constantIndex{0};
};
}  // end of namespace cclyzer

#endif /* CONTEXT_MANAGER_HPP_ */
