#include <llvm/ADT/SmallVector.h>

#include "DebugInfoProcessorImpl.hpp"
#include "debuginfo_predicate_groups.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;
using llvm::cast;
using llvm::dyn_cast;
using llvm::SmallVector;
using std::string;
namespace pred = cclyzer::predicates;
namespace dwarf = llvm::dwarf;

//------------------------------------------------------------------------------
// Helper method to record union attributes
//------------------------------------------------------------------------------

template <typename P, typename W, typename T>
void DebugInfoProcessor::Impl::recordUnionAttribute(
    const refmode_t &nodeId, const T *attribute) {
  using pred = P;
  using recorder = di_recorder<T, W>;

  if (attribute != nullptr) {
    refmode_t attribId = recorder::record(cast<T>(*attribute), *this);
    writeFact(pred::node, nodeId, attribId);
  }
}

//------------------------------------------------------------------------------
// Helper method to record bit flags
//------------------------------------------------------------------------------

void DebugInfoProcessor::Impl::recordFlags(
    const Predicate &pred, const refmode_t &nodeId, unsigned flags) {
  using Flags = llvm::DINode::DIFlags;
  if (flags) {
    // Split flags inside vector
    using FlagVectorT = SmallVector<Flags, 8>;
    FlagVectorT flagsVector;
    llvm::DINode::splitFlags((Flags)flags, flagsVector);

    for (const auto &flg : flagsVector) {
      llvm::StringRef flag = llvm::DINode::getFlagString(flg);
      writeFact(pred, nodeId, flag.str());
    }
  }
}

//------------------------------------------------------------------------------
// Explicit template instantiations
//------------------------------------------------------------------------------

template void DebugInfoProcessor::Impl ::recordUnionAttribute<
    pred::di_subprogram::containing_type,
    DebugInfoProcessor::Impl::write_di_type>(
    const refmode_t &, const llvm::DIType *);

template void DebugInfoProcessor::Impl ::recordUnionAttribute<
    pred::di_subprogram::scope,
    DebugInfoProcessor::Impl::write_di_scope>(
    const refmode_t &, const llvm::DIScope *);

template void DebugInfoProcessor::Impl ::recordUnionAttribute<
    pred::di_template_param::type,
    DebugInfoProcessor::Impl::write_di_type>(
    const refmode_t &, const llvm::DIType *);

template void DebugInfoProcessor::Impl ::recordUnionAttribute<
    pred::di_variable::type,
    DebugInfoProcessor::Impl::write_di_type>(
    const refmode_t &, const llvm::DIType *);

template void DebugInfoProcessor::Impl ::recordUnionAttribute<
    pred::di_composite_type::basetype,
    DebugInfoProcessor::Impl::write_di_type>(
    const refmode_t &, const llvm::DIType *);

template void DebugInfoProcessor::Impl ::recordUnionAttribute<
    pred::di_composite_type::vtable,
    DebugInfoProcessor::Impl::write_di_type>(
    const refmode_t &, const llvm::DIType *);

template void DebugInfoProcessor::Impl ::recordUnionAttribute<
    pred::di_derived_type::basetype,
    DebugInfoProcessor::Impl::write_di_type>(
    const refmode_t &, const llvm::DIType *);

template void DebugInfoProcessor::Impl ::recordUnionAttribute<
    pred::di_type::scope,
    DebugInfoProcessor::Impl::write_di_scope>(
    const refmode_t &, const llvm::DIScope *);
