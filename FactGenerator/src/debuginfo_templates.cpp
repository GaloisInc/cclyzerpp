#include <llvm/Support/raw_ostream.h>

#include "DebugInfoProcessorImpl.hpp"
#include "FactGenerator.hpp"
#include "debuginfo_predicate_groups.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::FactGenerator;
using cclyzer::refmode_t;
using llvm::cast;
using llvm::dyn_cast;
using std::string;
namespace pred = cclyzer::predicates;
namespace dwarf = llvm::dwarf;

//----------------------------------------------------------------------------
// Process Debug Info Template Parameters
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_tpl_param::write(
    const llvm::DITemplateParameter& diparam,
    const refmode_t& nodeId,
    DIProc& proc) {
  using llvm::DITemplateTypeParameter;
  using llvm::DITemplateValueParameter;

  proc.writeFact(pred::di_template_param::id, nodeId);

#if LLVM_VERSION_MAJOR > 12
  const std::string name = diparam.getName().str();
#else
  const std::string name = diparam.getName();
#endif

  const llvm::DIType* type = diparam.getType();

  // Record template parameter name
  if (!name.empty())
    proc.writeFact(pred::di_template_param::name, nodeId, name);

  // Record template parameter type
  proc.recordUnionAttribute<pred::di_template_param::type, write_di_type>(
      nodeId, type);

  if (const auto* tparam = dyn_cast<DITemplateTypeParameter>(&diparam)) {
    write_di_tpl_type_param::write(*tparam, nodeId, proc);
  } else {
    const auto* vparam = cast<DITemplateValueParameter>(&diparam);
    write_di_tpl_value_param::write(*vparam, nodeId, proc);
  }
}

void DebugInfoProcessor::Impl::write_di_tpl_type_param::write(
    const llvm::DITemplateTypeParameter& diparam,
    const refmode_t& nodeId,
    DIProc& proc) {
  proc.writeFact(pred::di_template_type_param::id, nodeId);
}

void DebugInfoProcessor::Impl::write_di_tpl_value_param::write(
    const llvm::DITemplateValueParameter& diparam,
    const refmode_t& nodeId,
    DIProc& proc) {
  const llvm::Metadata* value = diparam.getValue();
  proc.writeFact(pred::di_template_value_param::id, nodeId);

  // Record value
  if (const auto* mdtuple = dyn_cast<llvm::MDTuple>(value)) {
    size_t index = 0;

    for (auto it = mdtuple->op_begin(), end = mdtuple->op_end(); it != end;
         ++it, ++index) {
      const llvm::Metadata& item = **it;

      if (const auto* tplItem = dyn_cast<llvm::DITemplateParameter>(&item)) {
        refmode_t itemId = record_di_template_param::record(*tplItem, proc);

        proc.writeFact(
            pred::di_template_value_param::elements, nodeId, index, itemId);
      } else {
        llvm::errs() << "Unhandled template value item of node " << nodeId
                     << " at slot: " << index << '\n';
      }
    }
  } else if (const auto* mds = dyn_cast<llvm::MDString>(value)) {
#if LLVM_VERSION_MAJOR > 12
    std::string value = mds->getString().str();
#else
    std::string value = mds->getString();
#endif
    llvm::errs() << "Unhandled template value parameter " << '(' << value << ')'
                 << " for node: " << nodeId << " of type MDString\n";
  } else if (const auto* mdVal = dyn_cast<llvm::ConstantAsMetadata>(value)) {
    if (const llvm::Constant* value = mdVal->getValue()) {
      refmode_t cref = proc.recordConstant(*value);
      proc.writeFact(pred::di_template_value_param::value, nodeId, cref);
    }
  } else {
    llvm::errs() << "Unhandled template value parameter"
                 << " for node: " << nodeId << '\n';
  }
}

auto DebugInfoProcessor::Impl::recordConstant(const llvm::Constant& constant)
    -> refmode_t {
  FactGenerator& gen = FactGenerator::getInstance(getWriter());
  return gen.writeConstant(constant);
}
