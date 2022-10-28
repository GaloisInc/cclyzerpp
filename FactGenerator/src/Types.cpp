#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Type.h>

#include <string>

#include "FactGenerator.hpp"
#include "PredicateGroups.hpp"
#include "TypeAccumulator.hpp"
#include "TypeVisitor.hpp"

using cclyzer::FactGenerator;
namespace pred = cclyzer::predicates;

void FactGenerator::writeTypes(const llvm::DataLayout& layout) {
  using llvm_utils::TypeAccumulator;

  // Add basic primitive types
  writeFact(pred::primitive_type::id, "void");
  writeFact(pred::primitive_type::id, "label");
  writeFact(pred::primitive_type::id, "metadata");
  writeFact(pred::primitive_type::id, "x86mmx");

  // Find types contained in the types encountered so far, but not
  // referenced directly

  TypeAccumulator alltypes;
  alltypes.accumulate(types.begin(), types.end());

  // Create type visitor
  TypeVisitor TV(*this, layout);

  // Record each type encountered
  for (const auto* ty : alltypes) {
    TV.visitType(ty);
  }
}
