#ifndef POINTERANALYSIS_H
#define POINTERANALYSIS_H

#include <boost/flyweight.hpp>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/CommandLine.h"

namespace cclyzer {

class PointerAnalysisAAResult
#if LLVM_VERSION_MAJOR > 15
    : public llvm::AAResultBase {
  friend llvm::AAResultBase;
#else
    : public llvm::AAResultBase<PointerAnalysisAAResult> {
  friend llvm::AAResultBase<PointerAnalysisAAResult>;
#endif

 public:
  explicit PointerAnalysisAAResult(
      const std::map<int, boost::flyweight<std::string>>&& context_to_string,
      const std::vector<std::tuple<
          int,
          boost::flyweight<std::string>,
          int,
          const llvm::Value*>>&& variable_points_to,
      const std::vector<std::tuple<
          int,
          boost::flyweight<std::string>,
          boost::flyweight<std::string>>>&& alloc_may_alias,
      const std::vector<std::tuple<
          int,
          boost::flyweight<std::string>,
          boost::flyweight<std::string>>>&& alloc_must_alias,
      const std::vector<std::tuple<
          int,
          boost::flyweight<std::string>,
          boost::flyweight<std::string>>>&& alloc_subregion,
      const std::vector<std::tuple<
          int,
          boost::flyweight<std::string>,
          boost::flyweight<std::string>>>&& alloc_contains,
      const std::vector<std::tuple<
          int,
          boost::flyweight<std::string>,
          int,
          boost::flyweight<std::string>>>&& pointer_points_to,
      const std::vector<std::tuple<
          int,
          boost::flyweight<std::string>,
          int,
          const llvm::Value*>>&& operand_points_to,
      const std::vector<
          std::tuple<const llvm::Value*, boost::flyweight<std::string>>>&&
          global_allocations,
      const std::vector<std::tuple<int, boost::flyweight<std::string>, int>>&&
          allocation_sizes,
      const std::vector<std::tuple<
          int,
          const llvm::Value*,
          int,
          boost::flyweight<std::string>>>&& allocation_sites,
      const std::set<const llvm::Value*>&& null_ptr_set,
      const std::multimap<
          const llvm::Value*,
          std::tuple<int, int, const llvm::Value*>>&& callgraph)
      : context_to_string_(context_to_string),
        variable_points_to_(variable_points_to),
        alloc_may_alias_(alloc_may_alias),
        alloc_must_alias_(alloc_must_alias),
        alloc_subregion_(alloc_subregion),
        alloc_contains_(alloc_contains),
        pointer_points_to_(pointer_points_to),
        operand_points_to_(operand_points_to),
        global_allocations_(global_allocations),
        allocation_sizes_(allocation_sizes),
        allocation_sites_(allocation_sites),
        null_ptr_set_(null_ptr_set),
        callgraph_(callgraph) {}

  auto alias(
      const llvm::MemoryLocation&,
      const llvm::MemoryLocation&,
      llvm::AAQueryInfo&) -> llvm::AliasResult;

  auto getContextToString()
      -> const std::map<int, boost::flyweight<std::string>>& {
    return context_to_string_;
  }

  auto getVariablePointsTo() -> const std::vector<
      std::
          tuple<int, boost::flyweight<std::string>, int, const llvm::Value*>>& {
    return variable_points_to_;
  }

  auto getPointerPointsTo() -> const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      int,
      boost::flyweight<std::string>>>& {
    return pointer_points_to_;
  }

  auto getAllocMayAlias() -> const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      boost::flyweight<std::string>>>& {
    return alloc_may_alias_;
  }

  auto getAllocMustAlias() -> const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      boost::flyweight<std::string>>>& {
    return alloc_must_alias_;
  }

  auto getAllocSubregion() -> const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      boost::flyweight<std::string>>>& {
    return alloc_subregion_;
  }

  auto getAllocContains() -> const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      boost::flyweight<std::string>>>& {
    return alloc_contains_;
  }

  auto getOperandPointsTo() -> const std::vector<
      std::
          tuple<int, boost::flyweight<std::string>, int, const llvm::Value*>>& {
    return operand_points_to_;
  }

  auto getGlobalAllocations() -> const std::vector<
      std::tuple<const llvm::Value*, boost::flyweight<std::string>>>& {
    return global_allocations_;
  }

  auto getAllocationSizes() -> const
      std::vector<std::tuple<int, boost::flyweight<std::string>, int>>& {
    return allocation_sizes_;
  }

  auto getAllocationSites() -> const std::vector<
      std::
          tuple<int, const llvm::Value*, int, boost::flyweight<std::string>>>& {
    return allocation_sites_;
  }

  auto getNullPtrSet() -> const std::set<const llvm::Value*>& {
    return null_ptr_set_;
  }

  auto getCallGraph() -> const std::
      multimap<const llvm::Value*, std::tuple<int, int, const llvm::Value*>>& {
    return callgraph_;
  }

 private:
  const std::map<int, boost::flyweight<std::string>> context_to_string_;
  const std::vector<
      std::tuple<int, boost::flyweight<std::string>, int, const llvm::Value*>>
      variable_points_to_;
  const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      boost::flyweight<std::string>>>
      alloc_may_alias_;
  const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      boost::flyweight<std::string>>>
      alloc_must_alias_;
  const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      boost::flyweight<std::string>>>
      alloc_subregion_;
  const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      boost::flyweight<std::string>>>
      alloc_contains_;
  const std::vector<std::tuple<
      int,
      boost::flyweight<std::string>,
      int,
      boost::flyweight<std::string>>>
      pointer_points_to_;
  const std::vector<
      std::tuple<int, boost::flyweight<std::string>, int, const llvm::Value*>>
      operand_points_to_;
  const std::vector<
      std::tuple<const llvm::Value*, boost::flyweight<std::string>>>
      global_allocations_;
  const std::vector<std::tuple<int, boost::flyweight<std::string>, int>>
      allocation_sizes_;
  const std::vector<
      std::tuple<int, const llvm::Value*, int, boost::flyweight<std::string>>>
      allocation_sites_;
  const std::set<const llvm::Value*> null_ptr_set_;
  const std::
      multimap<const llvm::Value*, std::tuple<int, int, const llvm::Value*>>
          callgraph_;
};

class PointerAnalysis : public llvm::AnalysisInfoMixin<PointerAnalysis> {
 public:
  static llvm::AnalysisKey Key;
  using Result = PointerAnalysisAAResult;
  auto run(llvm::Module&, llvm::ModuleAnalysisManager&) -> Result;
  static auto name() -> llvm::StringRef { return "cclyzer"; }
};

class LegacyPointerAnalysis : public llvm::ModulePass {
 public:
  static char ID;

  LegacyPointerAnalysis() : llvm::ModulePass(ID) {}

  auto runOnModule(llvm::Module&) -> bool override;

  auto getResult() -> PointerAnalysisAAResult& { return *result_; }
  [[nodiscard]] auto getResult() const -> const PointerAnalysisAAResult& {
    return *result_;
  }

 private:
  std::unique_ptr<PointerAnalysisAAResult> result_;
};

}  // namespace cclyzer

#endif  // POINTERANALYSIS_H
