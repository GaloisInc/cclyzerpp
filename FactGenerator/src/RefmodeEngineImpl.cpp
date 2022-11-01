#include "RefmodeEngineImpl.hpp"

#include <llvm/IR/Metadata.h>
#include <llvm/Support/raw_ostream.h>

#include <boost/algorithm/string.hpp>
#include <boost/flyweight.hpp>
#include <map>

using cclyzer::refmode_t;
using cclyzer::RefmodeEngine;
using std::string;

using boost::algorithm::trim;

// Refmode for LLVM Values

auto RefmodeEngine::Impl::refmodeOf(const llvm::Value *Val) -> refmode_t {
  string rv;
  llvm::raw_string_ostream rso(rv);

  if (Val->hasName()) {
    rso << (llvm::isa<llvm::GlobalValue>(Val) ? '@' : '%') << Val->getName();
    goto print;
  }

  if (llvm::isa<llvm::Constant>(Val)) {
    Val->printAsOperand(rso, /* PrintType */ false);
    goto print;
  }

  if (Val->getType()->isVoidTy()) {
    Val->printAsOperand(rso, /* PrintType */ false);
    goto print;
  }

  if (Val->getType()->isMetadataTy()) {
    const auto *mv = llvm::cast<llvm::MetadataAsValue>(Val);
    const llvm::Metadata *meta = mv->getMetadata();

    appendMetadataId(rso, *meta);
    goto print;
  }

  // Handle unnamed variables
  for (auto it = ctx->rbegin(); it != ctx->rend(); ++it) {
    ContextManager::context &ctxt = *it;

    if (ctxt.isFunction) {
      if (ctxt.numbering.empty()) {
        computeNumbering(
            llvm::cast<llvm::Function>(ctxt.anchor), ctxt.numbering);
      }

      rso << '%' << ctxt.numbering[Val];
      goto print;
    }
  }

  // Expensive
  Val->printAsOperand(rso, false, &ctx->module());

print:
  // Trim external whitespace
  string ref = rso.str();
  trim(ref);

  return ref;
}

void RefmodeEngine::Impl::appendMetadataId(
    llvm::raw_string_ostream &rso, const llvm::Metadata &meta) {
  if (llvm::isa<llvm::MDNode>(meta)) {
    meta.printAsOperand(rso, *slotTracker);
  } else if (llvm::isa<llvm::MDString>(meta)) {
    meta.printAsOperand(rso, *slotTracker);
  } else {
    const auto &v = llvm::cast<llvm::ValueAsMetadata>(meta);
    const llvm::Value *innerValue = v.getValue();
    const llvm::Type *type = v.getType();

    if (llvm::isa<llvm::ConstantAsMetadata>(meta)) {
      meta.printAsOperand(rso, *slotTracker);
    } else {
      // For unknown reasons the printAsOperand() method is
      // super expensive for this particular metadata type,
      // at least for LLVM version 3.7.{0,1}. So instead, we
      // manually construct the refmodes ourselves.

      rso << refmode<llvm::Type>(*type) << " " << refmodeOf(innerValue);
    }
  }
}

void RefmodeEngine::Impl::computeNumbering(
    const llvm::Function *func,
    std::map<const llvm::Value *, unsigned> &numbering) {
  unsigned counter = 0;

  // Arguments get the first numbers.
  for (const auto &arg : func->args()) {
    if (!arg.hasName()) {
      numbering[&arg] = counter++;
    }
  }

  // Walk the basic blocks in order.
  for (const auto &bb : *func) {
    if (!bb.hasName()) {
      numbering[&bb] = counter++;
    }

    // Walk the instructions in order.
    for (const auto &instr : bb) {
      // void instructions don't get numbers.
      if (!instr.hasName() && !instr.getType()->isVoidTy()) {
        numbering[&instr] = counter++;
      }
    }
  }

  assert(!numbering.empty() && "asked for numbering but numbering was no-op");
}
