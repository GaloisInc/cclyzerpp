#include <llvm/IR/Constants.h>

#include <boost/functional/hash.hpp>
#include <string>

#include "FactGenerator.hpp"
#include "PredicateGroups.hpp"

using cclyzer::FactGenerator;
using llvm::cast;
using llvm::isa;
namespace pred = cclyzer::predicates;

//------------------------------------------------------------------------------
// Top-level method that records every kind of constant
//------------------------------------------------------------------------------

auto FactGenerator::writeConstant(const llvm::Constant &c)
    -> cclyzer::refmode_t {
  using namespace llvm;
  static boost::hash<std::string> string_hash;

  refmode_t id = refmode<llvm::Constant>(c);

  // Record constant entity with its type
  writeFact(pred::constant::id, id);
  writeFact(pred::constant::type, id, recordType(c.getType()));
  result_map_.insert({boost::flyweight<std::string>(id), &c});

  // Record containing function
  const llvm::Function *containingFunction = functionContext();
  if (containingFunction != nullptr) {
    const std::string funcname = "@" + containingFunction->getName().str();
    writeFact(pred::constant::in_func, id, funcname);
  }

  // Record constant value
  std::string rv;
  raw_string_ostream rso(rv);
  c.printAsOperand(rso, /* PrintType */ false);
  const std::string &val = rso.str();
  size_t hashCode = string_hash(val);

  writeFact(pred::constant::value, id, val);
  writeFact(pred::constant::hash, id, hashCode);
  result_map_.insert({boost::flyweight<std::string>(val), &c});

  if (isa<ConstantPointerNull>(c)) {
    writeFact(pred::nullptr_constant::id, id);
  } else if (isa<ConstantInt>(c)) {
    writeFact(pred::integer_constant::id, id);
    if (c.getUniqueInteger().isIntN(16)) {
      // Compute integer string representation
      // TODO(lb): Compute both signed and unsigned representations
#if LLVM_VERSION_MAJOR > 12
      llvm::SmallString<16> s;
      c.getUniqueInteger().toStringUnsigned(s, 10);
      std::string int_value = std::string(s);
#else
      std::string int_value = c.getUniqueInteger().toString(10, true);
#endif
      // Write constant to integer fact
      writeFact(pred::constant::to_integer, id, int_value);
    }
  } else if (isa<ConstantFP>(c)) {
    writeFact(pred::fp_constant::id, id);
  } else if (isa<Function>(c)) {
    const auto &func = cast<Function>(c);
    const std::string funcname = "@" + func.getName().str();

    writeFact(pred::func_constant::id, id);
    writeFact(pred::func_constant::name, id, funcname);
  } else if (isa<GlobalVariable>(c)) {
    const auto &global_var = cast<GlobalVariable>(c);
    const std::string varname = "@" + global_var.getName().str();

    writeFact(pred::global_var_constant::id, id);
    writeFact(pred::global_var_constant::name, id, varname);
  } else if (isa<ConstantExpr>(c)) {
    writeConstantExpr(cast<ConstantExpr>(c), id);
  } else if (isa<ConstantArray>(c)) {
    writeConstantArray(cast<ConstantArray>(c), id);
  } else if (isa<ConstantStruct>(c)) {
    writeConstantStruct(cast<ConstantStruct>(c), id);
  } else if (isa<ConstantVector>(c)) {
    writeConstantVector(cast<ConstantVector>(c), id);
  } else if (isa<UndefValue>(c)) {
    writeFact(pred::undef_constant::id, id);
  }

  return id;
}

//------------------------------------------------------------------------------
// Method that records every kind of constant expression
//------------------------------------------------------------------------------

void FactGenerator::writeConstantExpr(
    const llvm::ConstantExpr &expr, const refmode_t &refmode) {
  writeFact(pred::constant_expr::id, refmode);

  if (expr.isCast()) {
    refmode_t opref;

    switch (expr.getOpcode()) {
      case llvm::Instruction::BitCast:
        opref = writeConstant(*expr.getOperand(0));

        writeFact(pred::bitcast_constant_expr::id, refmode);
        writeFact(pred::bitcast_constant_expr::from_constant, refmode, opref);
        break;
      case llvm::Instruction::IntToPtr:
        opref = writeConstant(*expr.getOperand(0));

        writeFact(pred::inttoptr_constant_expr::id, refmode);
        writeFact(
            pred::inttoptr_constant_expr::from_int_constant, refmode, opref);
        break;
      case llvm::Instruction::PtrToInt:
        opref = writeConstant(*expr.getOperand(0));

        writeFact(pred::ptrtoint_constant_expr::id, refmode);
        writeFact(
            pred::ptrtoint_constant_expr::from_ptr_constant, refmode, opref);
        break;
    }

#if LLVM_VERSION_MAJOR > 13
  } else if (expr.getOpcode() == llvm::Instruction::GetElementPtr) {
#else
  } else if (expr.isGEPWithNoNotionalOverIndexing()) {
#endif
    unsigned nOperands = expr.getNumOperands();

    for (unsigned i = 0; i < nOperands; i++) {
      const llvm::Constant *c = cast<llvm::Constant>(expr.getOperand(i));

      refmode_t index_ref = writeConstant(*c);

      if (i > 0) {
        writeFact(pred::gep_constant_expr::index, refmode, i - 1, index_ref);
      } else {
        writeFact(pred::gep_constant_expr::base, refmode, index_ref);
      }
    }

    writeFact(pred::gep_constant_expr::nindices, refmode, nOperands - 1);
    writeFact(pred::gep_constant_expr::id, refmode);
  } else {
    // TODO
  }
}

//------------------------------------------------------------------------------
// Template Method for similar constant constructs
//------------------------------------------------------------------------------

template <typename PredGroup, class ConstantType>
void FactGenerator::writeConstantWithOperands(
    const ConstantType &base, const refmode_t &refmode) {
  unsigned nOperands = base.getNumOperands();

  for (unsigned i = 0; i < nOperands; i++) {
    const llvm::Constant *c = base.getOperand(i);

    refmode_t index_ref = writeConstant(*c);
    writeFact(PredGroup::index, refmode, i, index_ref);
  }

  writeFact(PredGroup::size, refmode, nOperands);
  writeFact(PredGroup::id, refmode);
}

void FactGenerator::writeConstantArray(
    const llvm::ConstantArray &array, const refmode_t &refmode) {
  writeConstantWithOperands<pred::constant_array>(array, refmode);
}

void FactGenerator::writeConstantStruct(
    const llvm::ConstantStruct &st, const refmode_t &refmode) {
  writeConstantWithOperands<pred::constant_struct>(st, refmode);
}

void FactGenerator::writeConstantVector(
    const llvm::ConstantVector &v, const refmode_t &refmode) {
  writeConstantWithOperands<pred::constant_vector>(v, refmode);
}
