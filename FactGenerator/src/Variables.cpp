#include <llvm/IR/Type.h>

#include <string>

#include "FactGenerator.hpp"
#include "PredicateGroups.hpp"

using cclyzer::FactGenerator;
namespace pred = cclyzer::predicates;

void FactGenerator::writeLocalVariables() {
  const llvm::Function *containing_function = functionContext();
  const std::string funcname = "@" + containing_function->getName().str();

  // Record every variable encountered so far
  for (const auto &[varId, type] : variableTypes) {
    // Record variable entity with its type and containing function
    writeFact(pred::variable::id, varId);
    writeFact(pred::variable::type, varId, recordType(type));
    writeFact(pred::variable::in_func, varId, funcname);

    // Record variable name part
    size_t idx = varId.find_last_of("%!");
    if (idx != llvm::StringRef::npos) {
      std::string var_name = varId.substr(idx);
      writeFact(pred::variable::name, varId, var_name);
    }
  }

  variableTypes.clear();
}
