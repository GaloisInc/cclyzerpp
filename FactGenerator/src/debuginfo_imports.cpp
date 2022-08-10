#include "DebugInfoProcessorImpl.hpp"
#include "debuginfo_predicate_groups.hpp"

#if LLVM_VERSION_MAJOR < 5  // Support/Dwarf -> BinaryFormat/Dwarf
#include <llvm/Support/Dwarf.h>
#else
#include <llvm/BinaryFormat/Dwarf.h>
#endif

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;
using llvm::dyn_cast;
namespace pred = cclyzer::predicates;
namespace dwarf = llvm::dwarf;

//----------------------------------------------------------------------------
// Process Debug Info Imported Entities
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_imported_entity::write(
    const llvm::DIImportedEntity& diimport,
    const refmode_t& nodeId,
    DIProc& proc) {
  proc.writeFact(pred::di_imported_entity::id, nodeId);

#if LLVM_VERSION_MAJOR > 12
  const std::string name = diimport.getName().str();
#else
  const std::string name = diimport.getName();
#endif

  proc.writeFact(pred::di_imported_entity::name, nodeId, name);

  const unsigned line = diimport.getLine();
  proc.writeFact(pred::di_imported_entity::line, nodeId, line);

  // Record enclosing scope
  if (const llvm::DIScope* discope = diimport.getScope()) {
    refmode_t scopeId = record_di_scope::record(*discope, proc);
    proc.writeFact(pred::di_imported_entity::scope, nodeId, scopeId);
  }

  // Record entity --- should be either DIDerivedType (typedef,
  // etc), DINamespace, DISubprogram

  const auto entity = diimport.getEntity();
  using llvm::DIScope;
  using llvm::MDString;
  const llvm::Metadata& meta = *entity;

  if (const auto* mds = dyn_cast<MDString>(&meta)) {
#if LLVM_VERSION_MAJOR > 12
    std::string attribStr = mds->getString().str();
#else
    std::string attribStr = mds->getString();
#endif
    proc.writeFact(pred::di_imported_entity::entity::raw, nodeId, attribStr);
  } else if (const DIScope* scope = dyn_cast<DIScope>(entity)) {
    refmode_t attribId = record_di_scope::record(*scope, proc);
    proc.writeFact(pred::di_imported_entity::entity::node, nodeId, attribId);
  }
  // TODO consider other types of entities
}
