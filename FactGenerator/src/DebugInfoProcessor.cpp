#include "DebugInfoProcessor.hpp"

#include <llvm/IR/Module.h>

#include "DebugInfoProcessorImpl.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;

//------------------------------------------------------------------------------
// Opaque Pointer Idiom - Delegate to Implementation instance
//------------------------------------------------------------------------------

DebugInfoProcessor::DebugInfoProcessor(
    FactWriter& writer, RefmodeEngine& engine)
    : impl(new Impl(writer, engine)) {}

DebugInfoProcessor::~DebugInfoProcessor() = default;

void DebugInfoProcessor::processModule(const llvm::Module& mod) {
  return impl->processModule(mod);
}

#if LLVM_VERSION_MAJOR < 13
void DebugInfoProcessor::processDeclare(
    const llvm::Module& mod, const llvm::DbgDeclareInst* instr) {
  return impl->processDeclare(mod, instr);
}

void DebugInfoProcessor::processValue(
    const llvm::Module& mod, const llvm::DbgValueInst* instr) {
  return impl->processValue(mod, instr);
}
#endif

void DebugInfoProcessor::reset() { return impl->reset(); }

void DebugInfoProcessor::generateDebugInfo(
    const llvm::Module& mod, const std::string& path) {
  return impl->generateDebugInfo(mod, path);
}

auto DebugInfoProcessor::record_di_file(const llvm::DIFile& difile)
    -> refmode_t {
  return Impl::record_di_file::record(difile, *impl);
}

auto DebugInfoProcessor::record_di_namespace(
    const llvm::DINamespace& dinamespace) -> refmode_t {
  return Impl::record_di_namespace::record(dinamespace, *impl);
}

auto DebugInfoProcessor::record_di_scope(const llvm::DIScope& discope)
    -> refmode_t {
  return Impl::record_di_scope::record(discope, *impl);
}

auto DebugInfoProcessor::record_di_type(const llvm::DIType& ditype)
    -> refmode_t {
  return Impl::record_di_type::record(ditype, *impl);
}

auto DebugInfoProcessor::record_di_template_param(
    const llvm::DITemplateParameter& diparam) -> refmode_t {
  return Impl::record_di_template_param::record(diparam, *impl);
}

auto DebugInfoProcessor::record_di_variable(const llvm::DIVariable& divariable)
    -> refmode_t {
  return Impl::record_di_variable::record(divariable, *impl);
}

auto DebugInfoProcessor::record_di_subprogram(
    const llvm::DISubprogram& disubprogram) -> refmode_t {
  return Impl::record_di_subprogram::record(disubprogram, *impl);
}

auto DebugInfoProcessor::record_di_enumerator(
    const llvm::DIEnumerator& dienumerator) -> refmode_t {
  return Impl::record_di_enumerator::record(dienumerator, *impl);
}

auto DebugInfoProcessor::record_di_subrange(const llvm::DISubrange& disubrange)
    -> refmode_t {
  return Impl::record_di_subrange::record(disubrange, *impl);
}

auto DebugInfoProcessor::record_di_imported_entity(
    const llvm::DIImportedEntity& diimport) -> refmode_t {
  return Impl::record_di_imported_entity::record(diimport, *impl);
}

auto DebugInfoProcessor::record_di_location(const llvm::DILocation& dilocation)
    -> refmode_t {
  return Impl::record_di_location::record(dilocation, *impl);
}
