#include "DebugInfoProcessorImpl.hpp"
#include "debuginfo_predicate_groups.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;
using llvm::cast;
using llvm::dyn_cast;
using llvm::isa;
namespace pred = cclyzer::predicates;

//----------------------------------------------------------------------------
// Process Debug Info Locations
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_location::write(
    const llvm::DILocation& diloc, const refmode_t& nodeId, DIProc& proc) {
  const unsigned line = diloc.getLine();
  const unsigned column = diloc.getColumn();

  proc.writeFact(pred::di_location::id, nodeId);

  // Record coordinates
  proc.writeFact(pred::di_location::line, nodeId, line);
  proc.writeFact(pred::di_location::column, nodeId, column);

  if (const llvm::DILocalScope* discope = diloc.getScope()) {
    refmode_t scopeId = record_di_scope::record(*discope, proc);
    proc.writeFact(pred::di_location::scope, nodeId, scopeId);
  }

  if (const llvm::DILocation* inlinedAt = diloc.getInlinedAt()) {
    refmode_t inlineLocId = record_di_location::record(*inlinedAt, proc);
    proc.writeFact(pred::di_location::inlined_at, nodeId, inlineLocId);
  }
}
