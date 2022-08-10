#if LLVM_VERSION_MAJOR >= 7
#include <llvm/IR/Constants.h>
#endif
#include "DebugInfoProcessorImpl.hpp"
#include "debuginfo_predicate_groups.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;
namespace pred = cclyzer::predicates;

//----------------------------------------------------------------------------
// Process Debug Info Subranges
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_subrange::write(
    const llvm::DISubrange& disubrange, const refmode_t& nodeId, DIProc& proc) {
  proc.writeFact(pred::di_subrange::id, nodeId);

  int64_t count = 0;
#if LLVM_VERSION_MAJOR < 7
  count = disubrange.getCount();
#else
  auto* CI = disubrange.getCount().dyn_cast<llvm::ConstantInt*>();
  if (CI != nullptr) {
    count = CI->getSExtValue();
  }
#endif

#if LLVM_VERSION_MAJOR < 13
  const int64_t lowerBound = disubrange.getLowerBound();
  if (lowerBound)
    proc.writeFact(pred::di_subrange::lower_bound, nodeId, lowerBound);
#endif

  proc.writeFact(pred::di_subrange::count, nodeId, count);
}
