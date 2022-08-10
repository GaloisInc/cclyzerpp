#include "DebugInfoProcessorImpl.hpp"

#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

#include <list>
#include <map>

#include "debuginfo_predicate_groups.hpp"
#include "predicate_groups.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;
using llvm::cast;
using llvm::dyn_cast;
using llvm::raw_string_ostream;
using std::list;
using std::string;
namespace pred = cclyzer::predicates;
namespace dwarf = llvm::dwarf;

//------------------------------------------------------------------------------
// Actual Implementation
//------------------------------------------------------------------------------

void DebugInfoProcessor::Impl::generateDebugInfo(
    const llvm::Module& m, const string& path) {
  // iterate over global variables
  for (const auto* divarex : debugInfoFinder.global_variables()) {
    record_di_variable::record(*divarex->getVariable(), *this);
  }

  // iterate over subprogram and record each one
  for (const auto* subprogram : debugInfoFinder.subprograms()) {
    record_di_subprogram::record(*subprogram, *this);
  }

  // iterate over subprogram and record each one
  for (const auto* scope : debugInfoFinder.scopes()) {
    record_di_scope::record(*scope, *this);
  }

  // iterate over compile unit and record each one
  for (const auto* compUnit : debugInfoFinder.compile_units()) {
    // iterate over imported entities
    auto importedEntities = compUnit->getImportedEntities();

    // record each imported entity entry
    for (const auto* diimport : importedEntities) {
      record_di_imported_entity::record(*diimport, *this);
    }
  }

  // iterate over types
  for (const auto* dbgType : debugInfoFinder.types()) {
    record_di_type::record(*dbgType, *this);
  }
  write_local_var_assocs();
}
