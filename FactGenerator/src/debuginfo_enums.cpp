#include "DebugInfoProcessorImpl.hpp"
#include "debuginfo_predicate_groups.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;
namespace pred = cclyzer::predicates;

//----------------------------------------------------------------------------
// Process Debug Info Enumerators
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_enumerator::write(
    const llvm::DIEnumerator& dienum, const refmode_t& nodeId, DIProc& proc) {
  proc.writeFact(pred::di_enumerator::id, nodeId);

#if LLVM_VERSION_MAJOR > 12
  const std::string name = dienum.getName().str();
  // TODO Assert that this is in range:
  const int64_t value = dienum.getValue().getLimitedValue();
#else
  const std::string name = dienum.getName();
  const int64_t value = dienum.getValue();
#endif

  proc.writeFact(pred::di_enumerator::name, nodeId, name);
  proc.writeFact(pred::di_enumerator::value, nodeId, value);
}
