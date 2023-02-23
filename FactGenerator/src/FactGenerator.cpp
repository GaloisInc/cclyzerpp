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
  static FactGenerator this_instance(writer);
  return this_instance;
}

auto FactGenerator::processModule(
    const llvm::Module &Mod,
    const std::string &path,
    const llvm::Optional<boost::filesystem::path> &signatures,
    const ContextSensitivity &sensitivity)
    -> std::map<boost::flyweight<std::string>, const llvm::Value *> {
  InstructionVisitor iv(*this, Mod);
  ModuleContext mc(*this, Mod, path);

  // Process points-to signatures
  std::vector<std::tuple<std::string, std::regex, llvm::json::Array>>
      functions_with_signatures;
  if (signatures.hasValue()) {
    functions_with_signatures = processSignatures(signatures.getValue());
  }

  // iterating over global variables in a module
  for (const auto &global_var : Mod.globals()) {
    refmode_t id = refmode<llvm::GlobalValue>(global_var);
    writeGlobalVar(global_var, id);
  }

  // iterating over global alias in a module
  for (const auto &alias : Mod.aliases()) {
    refmode_t id = refmode<llvm::GlobalValue>(alias);
    writeGlobalAlias(alias, id);
  }

  // Context sensitivity
  writeFact(
      predicates::user::options,
      "context_sensitivity",
      context_sensitivity_to_string(sensitivity));

  // iterating over functions in a module
  for (const auto &func : Mod) {
    Context c(*this, func);
    refmode_t funcref = refmode<llvm::Function>(func);

    // Save the results for building the CPG
    result_map_.insert({boost::flyweight<std::string>(funcref), &func});

    // Process function and record its various attributes, but do
    // not examine its body
    writeFunction(func, funcref);

    // Skip emitting facts about the body if the function has a signature
    bool matched = false;
    for (const auto &[regex_str, regex, sigs] : functions_with_signatures) {
      const auto mangled_name = func.getName().str();
      auto name = mangled_name;
      if (is_itanium_encoding(name)) {
        name = demangle(name);
      }
      if (std::regex_search(name, regex)) {
        emitSignatures(mangled_name, sigs);
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
      Context c(*this, bb);
      refmode_t bb_ref = refmode<llvm::BasicBlock>(bb);

      // Record basic block entry as a label
      writeFact(pred::variable::id, bb_ref);
      writeFact(pred::variable::type, bb_ref, "label");
      writeFact(pred::variable::in_func, bb_ref, "@" + func.getName().str());

      // Record variable name part
      size_t idx = bb_ref.find_last_of("%!");
      std::string bb_var_name = bb_ref.substr(idx);
      writeFact(pred::variable::name, bb_ref, bb_var_name);

      // Record basic block predecessors
      for (llvm::const_pred_iterator pi = pred_begin(&bb),
                                     pi_end = pred_end(&bb);
           pi != pi_end;
           ++pi) {
        refmode_t pred_bb = refmode<llvm::BasicBlock>(**pi);
        writeFact(pred::block::predecessor, bb_ref, pred_bb);
      }

      // iterating over basic block instructions
      for (const auto &instr : bb) {
        Context c(*this, instr);

        // Compute instruction refmode
        const refmode_t iref = refmode<llvm::Instruction>(instr);

        // Save instructions for the CPG
        result_map_.insert({boost::flyweight<std::string>(iref), &instr});

        // Record instruction target variable if such exists
        if (!instr.getType()->isVoidTy()) {
          refmode_t target_var = refmode<llvm::Value>(instr);

          writeFact(pred::instr::assigns_to, iref, target_var);
          recordVariable(target_var, instr.getType());

          // Save variables for the CPG
          result_map_.insert(
              {boost::flyweight<std::string>(target_var), &instr});
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
        const llvm::BasicBlock *bb_entry = instr.getParent();
        refmode_t bb_entry_id = refmode<llvm::BasicBlock>(*bb_entry);
        writeFact(pred::instr::bb_entry, iref, bb_entry_id);

        // Visit instruction
        iv.visit(const_cast<llvm::Instruction &>(instr));

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
