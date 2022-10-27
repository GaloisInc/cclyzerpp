// IWYU pragma: no_include <boost/unordered/detail/implementation.hpp>

#include <llvm/ADT/StringRef.h>  // for StringRef, Stri...
#include <llvm/IR/Function.h>    // for Function
#include <stddef.h>              // for size_t

#include <boost/unordered/unordered_map.hpp>  // for unordered_map<>...
#include <string>                             // for string, operator+
#include <type_traits>                        // for add_const<>::type
#include <utility>                            // for tuple_element<>...

#include "FactGenerator.hpp"     // for FactGenerator
#include "predicate_groups.hpp"  // for variable, predi...

using cclyzer::FactGenerator;
namespace pred = cclyzer::predicates;

void FactGenerator::writeLocalVariables() {
  const llvm::Function *containingFunction = functionContext();
  const std::string funcname = "@" + containingFunction->getName().str();

  // Record every variable encountered so far
  for (const auto &[varId, type] : variableTypes) {
    // Record variable entity with its type and containing function
    writeFact(pred::variable::id, varId);
    writeFact(pred::variable::type, varId, recordType(type));
    writeFact(pred::variable::in_func, varId, funcname);

    // Record variable name part
    size_t idx = varId.find_last_of("%!");
    if (idx != llvm::StringRef::npos) {
      std::string varName = varId.substr(idx);
      writeFact(pred::variable::name, varId, varName);
    }
  }

  variableTypes.clear();
}
