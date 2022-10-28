#include <llvm/IR/InlineAsm.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/functional/hash.hpp>
#include <string>

#include "FactGenerator.hpp"
#include "PredicateGroups.hpp"

using cclyzer::FactGenerator;
using llvm::cast;
using llvm::isa;
namespace pred = cclyzer::predicates;

static auto canonicalize(const std::string &in) -> std::string {
  std::string base(in);
  boost::replace_all(base, "\t", "\\t");
  boost::replace_all(base, "\n", "\\n");
  return base;
}

auto FactGenerator::writeAsm(const llvm::InlineAsm &asmVal)
    -> cclyzer::refmode_t {
  using namespace llvm;
  static boost::hash<std::string> string_hash;

  refmode_t id = refmode<llvm::InlineAsm>(asmVal);
  const llvm::Type *type = asmVal.getType();

  std::string constraints = canonicalize(asmVal.getConstraintString());
  std::string assem = canonicalize(asmVal.getAsmString());
  std::string val = "<asm>(" + assem + ")";
  size_t hashCode = string_hash(val);

  // Record inline ASM as constant entity with its type
  writeFact(pred::constant::id, id);
  writeFact(pred::constant::type, id, recordType(type));
  writeFact(pred::constant::value, id, val);
  writeFact(pred::constant::hash, id, hashCode);
  types.insert(type);

  // Record its attributes separately
  writeFact(pred::inline_asm::id, id);
  writeFact(pred::inline_asm::constraints, id, constraints);
  writeFact(pred::inline_asm::text, id, assem);

  return id;
}
