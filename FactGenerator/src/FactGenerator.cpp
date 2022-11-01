#include "FactGenerator.hpp"

#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Operator.h>

#include <regex>
#include <string>

#include "ContextSensitivity.hpp"
#include "InstructionVisitor.hpp"
#include "PredicateGroups.hpp"
#include "Signatures.hpp"

using cclyzer::FactGenerator;
using llvm::cast;
using llvm::isa;
namespace pred = cclyzer::predicates;

auto FactGenerator::getInstance(FactWriter &writer) -> FactGenerator & {
  static FactGenerator thisInstance(writer);
  return thisInstance;
}

/* NOTE(ww): Stolen from Demangle.cpp (not present in LLVM 7).
 */
static inline auto is_itanium_encoding(const std::string &MangledName) -> bool {
  size_t Pos = MangledName.find_first_not_of('_');
  // A valid Itanium encoding requires 1-4 leading underscores, followed by
  // 'Z'.
  return Pos > 0 && Pos <= 4 && MangledName[Pos] == 'Z';
}

auto FactGenerator::processModule(
    const llvm::Module &Mod,
    const std::string &path,
    const llvm::Optional<boost::filesystem::path> &signatures,
    const ContextSensitivity &sensitivity)
    -> std::map<boost::flyweight<std::string>, const llvm::Value *> {
  InstructionVisitor IV(*this, Mod);
  ModuleContext MC(*this, Mod, path);

  // Process points-to signatures
  std::vector<std::tuple<std::string, std::regex, llvm::json::Array>>
      functions_with_signatures;
  if (signatures.hasValue()) {
    functions_with_signatures = processSignatures(signatures.getValue());
  }

  // iterating over global variables in a module
  for (llvm::Module::const_global_iterator it = Mod.global_begin(),
                                           end = Mod.global_end();
       it != end;
       ++it) {
    refmode_t id = refmode<llvm::GlobalValue>(*it);

    writeGlobalVar(*it, id);
  }

  // iterating over global alias in a module
  for (llvm::Module::const_alias_iterator it = Mod.alias_begin(),
                                          end = Mod.alias_end();
       it != end;
       ++it) {
    refmode_t id = refmode<llvm::GlobalValue>(*it);

    writeGlobalAlias(*it, id);
  }

  // Context sensitivity
  writeFact(
      predicates::user::options,
      "context_sensitivity",
      context_sensitivity_to_string(sensitivity));

  // iterating over functions in a module
  for (const auto &func : Mod) {
    Context C(*this, func);
    refmode_t funcref = refmode<llvm::Function>(func);

    // Save the results for building the CPG
    result_map_.insert({boost::flyweight<std::string>(funcref), &func});

    // Process function and record its various attributes, but do
    // not examine its body
    writeFunction(func, funcref);

    // Skip emitting facts about the body if the function has a signature
    bool matched = false;
    for (const auto &[regex_str, regex, sigs] : functions_with_signatures) {
      const auto mangledName = func.getName().str();
      auto name = mangledName;
      if (is_itanium_encoding(name)) {
        name = demangle(name);
      }
      if (std::regex_search(name, regex)) {
        emitSignatures(mangledName, sigs);
        matched = true;
      }
    }
    if (matched) {
      continue;
    }

    // Previous instruction
    const llvm::Instruction *prev_instr = nullptr;
    refmode_t prev_iref;

    // iterating over basic blocks in a function
    for (const auto &bb : func) {
      Context C(*this, bb);
      refmode_t bbRef = refmode<llvm::BasicBlock>(bb);

      // Record basic block entry as a label
      writeFact(pred::variable::id, bbRef);
      writeFact(pred::variable::type, bbRef, "label");
      writeFact(pred::variable::in_func, bbRef, func.getName().str());

      // Record variable name part
      size_t idx = bbRef.find_last_of("%!");
      std::string bbVarName = bbRef.substr(idx);
      writeFact(pred::variable::name, bbRef, bbVarName);

      // Record basic block predecessors
      for (llvm::const_pred_iterator pi = pred_begin(&bb),
                                     pi_end = pred_end(&bb);
           pi != pi_end;
           ++pi) {
        refmode_t predBB = refmode<llvm::BasicBlock>(**pi);
        writeFact(pred::block::predecessor, bbRef, predBB);
      }

      // iterating over basic block instructions
      for (const auto &instr : bb) {
        Context C(*this, instr);

        // Compute instruction refmode
        const refmode_t iref = refmode<llvm::Instruction>(instr);

        // Save instructions for the CPG
        result_map_.insert({boost::flyweight<std::string>(iref), &instr});

        // Record instruction target variable if such exists
        if (!instr.getType()->isVoidTy()) {
          refmode_t targetVar = refmode<llvm::Value>(instr);

          writeFact(pred::instr::assigns_to, iref, targetVar);
          recordVariable(targetVar, instr.getType());

          // Save variables for the CPG
          result_map_.insert(
              {boost::flyweight<std::string>(targetVar), &instr});
        }

        // Record successor instruction
        if (prev_instr != nullptr) {
          writeFact(pred::instr::successor, prev_iref, iref);
        }

        // Store the refmode of this instruction for next iteration
        prev_iref = iref;
        prev_instr = &instr;

        // Record instruction's container function
        writeFact(pred::instr::func, iref, funcref);

        // Record instruction's basic block entry (label)
        const llvm::BasicBlock *bbEntry = instr.getParent();
        refmode_t bbEntryId = refmode<llvm::BasicBlock>(*bbEntry);
        writeFact(pred::instr::bb_entry, iref, bbEntryId);

        // Visit instruction
        IV.visit(const_cast<llvm::Instruction &>(instr));

        // Get debug location if available
        if (const llvm::DebugLoc &location = instr.getDebugLoc()) {
          unsigned line = location.getLine();
          unsigned column = location.getCol();

          writeFact(pred::instr::pos, iref, line, column);
        }
      }
    }

    writeLocalVariables();
  }

  return this->result_map_;
}

auto FactGenerator::processSignatures(const boost::filesystem::path &signatures)
    -> std::vector<std::tuple<std::string, std::regex, llvm::json::Array>> {
  return preprocess_signatures(signatures);
}

void FactGenerator::emitSignatures(
    const std::string &func, const llvm::json::Array &signatures) {
  emit_signatures(*this, func, signatures);
}
