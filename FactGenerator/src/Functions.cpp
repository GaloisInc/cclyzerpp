#include <llvm/Config/llvm-config.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>

#include "FactGenerator.hpp"
#include "PredicateGroups.hpp"

using cclyzer::FactGenerator;
namespace pred = cclyzer::predicates;

//------------------------------------------------------------------------------
// Record Function (but not the function body)
//------------------------------------------------------------------------------

void FactGenerator::writeFunction(
    const llvm::Function &func, const refmode_t &funcref) {
  // Serialize function properties
  refmode_t visibility = refmode(func.getVisibility());
  refmode_t linkage = refmode(func.getLinkage());
  refmode_t typeSignature = recordType(func.getFunctionType());

  // Record function type signature
  writeFact(pred::func::ty, funcref, typeSignature);

  // Record function signature (name plus type signature) after
  // unmangling
  writeFact(pred::func::signature, funcref, demangle(func.getName().data()));

  // Record function linkage, visibility, alignment, and GC
  if (!linkage.empty()) {
    writeFact(pred::func::linkage, funcref, linkage);
  }

  if (!visibility.empty()) {
    writeFact(pred::func::visibility, funcref, visibility);
  }

  if (func.getAlignment() != 0U) {
    writeFact(pred::func::alignment, funcref, func.getAlignment());
  }

  if (func.hasGC()) {
    writeFact(pred::func::gc, funcref, func.getGC());
  }

  if (func.hasPersonalityFn()) {
    llvm::Constant *pers_fn = func.getPersonalityFn();
    refmode_t pers_fn_ref = writeConstant(*pers_fn);

    writeFact(pred::func::pers_fn, funcref, pers_fn_ref);
  }

  // Record calling convection if it not defaults to C
  if (func.getCallingConv() != llvm::CallingConv::C) {
    refmode_t cconv = refmode(func.getCallingConv());
    writeFact(pred::func::calling_conv, funcref, cconv);
  }

  // Record function name
  const std::string funcname = "@" + func.getName().str();
  writeFact(pred::func::name, funcref, funcname);

  // Address not significant

  if (func.hasGlobalUnnamedAddr()) {
    writeFact(pred::func::unnamed_addr, funcref);
  }

  // TODO Record appropriately
  if (func.hasAtLeastLocalUnnamedAddr()) {
  }

  if (func.isDeclaration()) {
    // Record as a function declaration entity
    writeFact(pred::func::id_decl, funcref);

    // Nothing more to do for function declarations
    return;
  }

  // Record function definition entity
  writeFact(pred::func::id_defn, funcref);

  // Record section
  if (func.hasSection()) {
    llvm::StringRef secStr = func.getSection();
    writeFact(pred::func::section, funcref, secStr.str());
  }

  // Record function parameters
  int index = 0;

  for (llvm::Function::const_arg_iterator arg = func.arg_begin(),
                                          arg_end = func.arg_end();
       arg != arg_end;
       arg++) {
    refmode_t varId = refmode<llvm::Value>(*arg);

    // Save parameters for CPG
    result_map_.insert({boost::flyweight<std::string>(varId), arg});

    writeFact(pred::func::param, funcref, index++, varId);
    recordVariable(varId, arg->getType());
  }
}
