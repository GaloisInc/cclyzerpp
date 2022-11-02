#include <llvm/Config/llvm-config.h>
#include <llvm/IR/Constants.h>

#include "FactGenerator.hpp"
#include "PredicateGroups.hpp"

using cclyzer::FactGenerator;
namespace pred = cclyzer::predicates;

void FactGenerator::writeGlobalAlias(
    const llvm::GlobalAlias& ga, const refmode_t& aliasId) {
  //------------------------------------------------------------------
  // A global alias introduces a /second name/ for the aliasee value
  // (which can be either function, global variable, another alias
  // or bitcast of global value). It has the following form:
  //
  // @<Name> = alias [Linkage] [Visibility] <AliaseeTy> @<Aliasee>
  //------------------------------------------------------------------

  std::string name = "@" + ga.getName().str();

  // Get aliasee value as llvm constant
  const llvm::Constant* aliasee = ga.getAliasee();

  // Record alias entity
  writeFact(pred::alias::id, aliasId);
  writeFact(pred::alias::name, aliasId, name);

  // Serialize alias properties
  refmode_t visibility = refmode(ga.getVisibility());
  refmode_t linkage = refmode(ga.getLinkage());
  refmode_t alias_type = recordType(ga.getType());

  // Record visibility
  if (!visibility.empty()) {
    writeFact(pred::alias::visibility, aliasId, visibility);
  }

  // Record linkage
  if (!linkage.empty()) {
    writeFact(pred::alias::linkage, aliasId, linkage);
  }

  // Record type
  writeFact(pred::alias::type, aliasId, alias_type);

  // Record aliasee
  if (aliasee != nullptr) {
    writeFact(pred::alias::aliasee, aliasId, writeConstant(*aliasee));
  }
}

void FactGenerator::writeGlobalVar(
    const llvm::GlobalVariable& gv, const refmode_t& id) {
  std::string name = "@" + gv.getName().str();

  // Record global variable entity
  writeFact(pred::global_var::id, id);
  writeFact(pred::global_var::name, id, name);
  result_map_.insert({boost::flyweight<std::string>(id), &gv});

  // Also record original pointer type
  recordType(gv.getType());

  // Serialize global variable properties
  refmode_t visibility = refmode(gv.getVisibility());
  refmode_t linkage = refmode(gv.getLinkage());
#if LLVM_VERSION_MAJOR > 14
  refmode_t var_type = recordType(gv.getType());
#else
  refmode_t var_type = recordType(gv.getType()->getElementType());
#endif
  refmode_t thr_loc_mode = refmode(gv.getThreadLocalMode());

  // Record demangled variable name
  writeFact(
      pred::global_var::demangled_name, id, demangle(gv.getName().data()));

  // Record external linkage
  if (!gv.hasInitializer() && gv.hasExternalLinkage()) {
    writeFact(pred::global_var::linkage, id, "external");
  }

  // Record linkage
  if (!linkage.empty()) {
    writeFact(pred::global_var::linkage, id, linkage);
  }

  // Record visibility
  if (!visibility.empty()) {
    writeFact(pred::global_var::visibility, id, visibility);
  }

  // Record thread local mode
  if (!thr_loc_mode.empty()) {
    writeFact(pred::global_var::threadlocal_mode, id, thr_loc_mode);
  }

  // TODO: in lb schema - AddressSpace & hasUnnamedAddr properties
  if (gv.isExternallyInitialized()) {
    writeFact(pred::global_var::flag, id, "externally_initialized");
  }

  // Record flags and type
  const char* flag = gv.isConstant() ? "constant" : "global";

  writeFact(pred::global_var::flag, id, flag);
  writeFact(pred::global_var::type, id, var_type);

  // Record initializer
  if (gv.hasInitializer()) {
    const llvm::Constant* initializer = gv.getInitializer();
    refmode_t init_ref = writeConstant(*initializer);

    writeFact(pred::global_var::initializer, id, init_ref);
  }

  // Record section
  if (gv.hasSection()) {
    llvm::StringRef sec_str = gv.getSection();
    writeFact(pred::global_var::section, id, sec_str.str());
  }

  // Record alignment
  if (gv.getAlignment() != 0U) {
    writeFact(pred::global_var::aligned_to, id, gv.getAlignment());
  }
}
