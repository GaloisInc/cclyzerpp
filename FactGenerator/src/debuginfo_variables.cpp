#include <llvm/IR/IntrinsicInst.h>

#include "DebugInfoProcessorImpl.hpp"
#include "debuginfo_predicate_groups.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;
using llvm::cast;
using llvm::dyn_cast;
using llvm::isa;
using std::list;
using std::string;
namespace pred = cclyzer::predicates;

//----------------------------------------------------------------------------
// Process Debug Info Variables
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_variable::write(
    const llvm::DIVariable &divar, const refmode_t &nodeId, DIProc &proc) {
  using llvm::DIGlobalVariable;
  using llvm::DILocalVariable;

  proc.writeFact(pred::di_variable::id, nodeId);

#if LLVM_VERSION_MAJOR > 12
  const std::string name = divar.getName().str();
#else
  const std::string name = divar.getName();
#endif

  // Record variable name
  if (!name.empty()) proc.writeFact(pred::di_variable::name, nodeId, name);

  // Record variable type
  proc.recordUnionAttribute<pred::di_variable::type, write_di_type>(
      nodeId, divar.getType());

  // Record variable scope
  if (const llvm::DIScope *discope = divar.getScope()) {
    refmode_t scopeId = record_di_scope::record(*discope, proc);
    proc.writeFact(pred::di_variable::scope, nodeId, scopeId);
  }

  // Record line
  if (const unsigned line = divar.getLine())  // zero indicates non-existence
    proc.writeFact(pred::di_variable::line, nodeId, line);

  // Record file information for variable
  if (const llvm::DIFile *difile = divar.getFile()) {
    refmode_t fileId = record_di_file::record(*difile, proc);
    proc.writeFact(pred::di_variable::file, nodeId, fileId);
  }

  // Write more data depending on the kind of variable
  if (const auto *gvar = dyn_cast<DIGlobalVariable>(&divar)) {
    write_di_global_variable::write(*gvar, nodeId, proc);
  } else {
    const auto *lvar = cast<DILocalVariable>(&divar);
    write_di_local_variable::write(*lvar, nodeId, proc);
  }
}

void DebugInfoProcessor::Impl::write_di_global_variable::write(
    const llvm::DIGlobalVariable &divar,
    const refmode_t &nodeId,
    DIProc &proc) {
  proc.writeFact(pred::di_global_var::id, nodeId);

  // Record linkage name
#if LLVM_VERSION_MAJOR > 12
  const std::string linkageName = divar.getLinkageName().str();
#else
  const std::string linkageName = divar.getLinkageName();
#endif

  if (!linkageName.empty())
    proc.writeFact(pred::di_global_var::linkage_name, nodeId, linkageName);

  // Record flags
  if (divar.isDefinition())
    proc.writeFact(pred::di_global_var::is_definition, nodeId);

  if (divar.isLocalToUnit())
    proc.writeFact(pred::di_global_var::is_local_to_unit, nodeId);

  // Record static member declaration debug info entry
  if (const llvm::DIDerivedType *decl =
          divar.getStaticDataMemberDeclaration()) {
    refmode_t declId = record_di_type::record(*decl, proc);

    proc.writeFact(
        pred::di_global_var::static_data_member_decl, nodeId, declId);
  }

  // Record LLVM global variable association
#if LLVM_VERSION_MAJOR < 4  // DIGlobalVariable.getVariable eliminated
  if (const llvm::Constant *gv = divar.getVariable()) {
    std::string name = "@" + gv->getName().str();
    proc.writeFact(pred::di_global_var::variable, nodeId, name);
  }
#endif
}

void DebugInfoProcessor::Impl::write_di_local_variable::write(
    const llvm::DILocalVariable &divar, const refmode_t &nodeId, DIProc &proc) {
  proc.writeFact(pred::di_local_var::id, nodeId);

  // Record that variable is function argument
  if (unsigned arg = divar.getArg()) {
    proc.writeFact(pred::di_local_var::arg_num, nodeId, arg);
  }

  // Record local variable flags

  if (divar.isArtificial())
    proc.writeFact(pred::di_local_var::flag, nodeId, "artificial");

  if (divar.isObjectPointer())
    proc.writeFact(pred::di_local_var::flag, nodeId, "objectpointer");
}

void DebugInfoProcessor::Impl::record_local_var_assoc(
    const llvm::DbgDeclareInst &inst) {
  const llvm::Value *address = inst.getAddress();

  // Record source variable name
  if (const llvm::DILocalVariable *var = inst.getVariable()) {
    // Skip undefined values
    if (isa<llvm::UndefValue>(address)) {
      undefVars.push_back(var);
    } else {
      // Obtain the refmodes of the local variable and the decl instruction
      refmode_t refmode = refmEngine.refmode<llvm::Value>(*address);

      // TODO consider also recording instruction
      // refmode_t iref = refmEngine.refmode<llvm::Instruction>(inst);

      // Record debug-info variable to LLVM variable association
      localVars.insert(LocalVarMap::value_type(var, refmode));
    }
  }
}

void DebugInfoProcessor::Impl::write_local_var_assocs() {
  for (auto &localVar : localVars) {
    const llvm::DILocalVariable *divar = localVar.first;
    refmode_t varId = localVar.second;
    refmode_t nodeId = record_di_variable::record(*divar, *this);

    writeFact(pred::di_local_var::variable, nodeId, varId);
  }

  for (const auto *divar : undefVars) {
    record_di_variable::record(*divar, *this);
  }
}
