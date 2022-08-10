#include <llvm/Config/llvm-config.h>

#include "DebugInfoProcessorImpl.hpp"
#include "debuginfo_predicate_groups.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;
using llvm::cast;
using llvm::dyn_cast;
using std::string;
namespace pred = cclyzer::predicates;

//----------------------------------------------------------------------------
// Process Debug Info Files
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_file::write(
    const llvm::DIFile& difile, const refmode_t& nodeId, DIProc& proc) {
#if LLVM_VERSION_MAJOR > 12
  string filename = difile.getFilename().str();
  string directory = difile.getDirectory().str();
#else
  string filename = difile.getFilename();
  string directory = difile.getDirectory();
#endif

  proc.writeFact(pred::di_file::id, nodeId);
  proc.writeFact(pred::di_file::filename, nodeId, filename);
  proc.writeFact(pred::di_file::directory, nodeId, directory);
}

//----------------------------------------------------------------------------
// Process Debug Info Namespaces
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_namespace::write(
    const llvm::DINamespace& dinamespace,
    const refmode_t& nodeId,
    DIProc& proc) {
#if LLVM_VERSION_MAJOR > 12
  const string name = dinamespace.getName().str();
#else
  const string name = dinamespace.getName();
#endif

  proc.writeFact(pred::di_namespace::id, nodeId);
  proc.writeFact(pred::di_namespace::name, nodeId, name);

#if LLVM_VERSION_MAJOR < 5  // DINamespace->getLine eliminated
  const unsigned line = dinamespace.getLine();
  proc.writeFact(pred::di_namespace::line, nodeId, line);
#endif

  // Record file information for namespace
  if (const llvm::DIFile* difile = dinamespace.getFile()) {
    refmode_t fileId = record_di_file::record(*difile, proc);
    proc.writeFact(pred::di_namespace::file, nodeId, fileId);
  }

  // Record enclosing scope
  if (const llvm::DIScope* discope = dinamespace.getScope()) {
    refmode_t scopeId = record_di_scope::record(*discope, proc);
    proc.writeFact(pred::di_namespace::scope, nodeId, scopeId);
  }
}

//----------------------------------------------------------------------------
// Process Debug Info Lexical Blocks
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_lex_block::write(
    const llvm::DILexicalBlock& dilexblock,
    const refmode_t& nodeId,
    DIProc& proc) {
  const unsigned line = dilexblock.getLine();
  const unsigned column = dilexblock.getColumn();

  proc.writeFact(pred::di_lex_block::id, nodeId);
  proc.writeFact(pred::di_lex_block::line, nodeId, line);
  proc.writeFact(pred::di_lex_block::column, nodeId, column);

  // Record file information for lexical block
  if (const llvm::DIFile* difile = dilexblock.getFile()) {
    refmode_t fileId = record_di_file::record(*difile, proc);
    proc.writeFact(pred::di_lex_block::file, nodeId, fileId);
  }

  // Record lexical block scope
  if (const llvm::DIScope* discope = dilexblock.getScope()) {
    refmode_t scopeId = record_di_scope::record(*discope, proc);
    proc.writeFact(pred::di_lex_block::scope, nodeId, scopeId);
  }
}

void DebugInfoProcessor::Impl::write_di_lex_block_file::write(
    const llvm::DILexicalBlockFile& dilexblkfile,
    const refmode_t& nodeId,
    DIProc& proc) {
  proc.writeFact(pred::di_lex_block_file::id, nodeId);

  // Record discriminator
  const unsigned discriminator = dilexblkfile.getDiscriminator();
  proc.writeFact(pred::di_lex_block_file::discriminator, nodeId, discriminator);

  // Record file information for lexical block file
  if (const llvm::DIFile* difile = dilexblkfile.getFile()) {
    refmode_t fileId = record_di_file::record(*difile, proc);
    proc.writeFact(pred::di_lex_block_file::file, nodeId, fileId);
  }

  // Record lexical block file scope
  if (const llvm::DIScope* discope = dilexblkfile.getScope()) {
    refmode_t scopeId = record_di_scope::record(*discope, proc);
    proc.writeFact(pred::di_lex_block_file::scope, nodeId, scopeId);
  }
}

//----------------------------------------------------------------------------
// Process Debug Info Subprograms
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_subprogram::write(
    const llvm::DISubprogram& disubprogram,
    const refmode_t& nodeId,
    DIProc& proc) {
  using llvm::DIFile;
  using llvm::DINamespace;
  using llvm::DIType;

  proc.writeFact(pred::di_subprogram::id, nodeId);

  // Record function subprogram
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR < 8
  if (const llvm::Function* func = disubprogram.getFunction()) {
    refmode_t funcref = proc.refmEngine.refmode<llvm::Function>(*func);
    proc.writeFact(pred::di_subprogram::function, nodeId, funcref);
  }
#endif

  //-----------------------------------------------------------------
  // Record generic scope properties
  //-----------------------------------------------------------------

#if LLVM_VERSION_MAJOR > 12
  const std::string name = disubprogram.getName().str();
#else
  const std::string name = disubprogram.getName();
#endif

  if (!name.empty()) proc.writeFact(pred::di_subprogram::name, nodeId, name);

  // Record file information for namespace
  if (const llvm::DIFile* difile = disubprogram.getFile()) {
    refmode_t fileId = record_di_file::record(*difile, proc);
    proc.writeFact(pred::di_subprogram::file, nodeId, fileId);
  }

  // Record enclosing scope
  proc.recordUnionAttribute<pred::di_subprogram::scope, write_di_scope>(
      nodeId, disubprogram.getScope());

  //-----------------------------------------------------------------
  // Record subprogram-specific properties
  //-----------------------------------------------------------------

  // Record linkage name
#if LLVM_VERSION_MAJOR > 12
  const std::string linkageName = disubprogram.getLinkageName().str();
#else
  const std::string linkageName = disubprogram.getLinkageName();
#endif
  if (!linkageName.empty())
    proc.writeFact(pred::di_subprogram::linkage_name, nodeId, linkageName);

  // Record subprogram type
  if (const llvm::DIType* ditype = disubprogram.getType()) {
    refmode_t typeId = record_di_type::record(*ditype, proc);
    proc.writeFact(pred::di_subprogram::type, nodeId, typeId);
  }

  // Record lines
  const unsigned line = disubprogram.getLine();
  const unsigned scopeLine = disubprogram.getScopeLine();

  proc.writeFact(pred::di_subprogram::line, nodeId, line);
  proc.writeFact(pred::di_subprogram::scope_line, nodeId, scopeLine);

  // Record various flags
  if (disubprogram.isDefinition())
    proc.writeFact(pred::di_subprogram::is_definition, nodeId);

  if (disubprogram.isLocalToUnit())
    proc.writeFact(pred::di_subprogram::is_local_to_unit, nodeId);

  if (disubprogram.isOptimized())
    proc.writeFact(pred::di_subprogram::is_optimized, nodeId);

  // Record containing type
  proc.recordUnionAttribute<
      pred::di_subprogram::containing_type,
      write_di_type>(nodeId, disubprogram.getContainingType());

  // Record declaration
  if (const llvm::DISubprogram* decl = disubprogram.getDeclaration()) {
    refmode_t declId = record_di_subprogram::record(*decl, proc);
    proc.writeFact(pred::di_subprogram::declaration, nodeId, declId);
  }

  // Record virtuality and virtual index
  if (unsigned virtuality = disubprogram.getVirtuality()) {
#if LLVM_VERSION_MAJOR < 4  // const char * -> StringRef
    const char* virtualityStr = llvm::dwarf::VirtualityString(virtuality);
    proc.writeFact(pred::di_subprogram::virtuality, nodeId, virtualityStr);
#else
    llvm::StringRef virtualityStr = llvm::dwarf::VirtualityString(virtuality);
    proc.writeFact(
        pred::di_subprogram::virtuality, nodeId, virtualityStr.str());
#endif

    unsigned virtIdx = disubprogram.getVirtualIndex();
    proc.writeFact(pred::di_subprogram::virtual_index, nodeId, virtIdx);
  }

  // Record flags
  proc.recordFlags(pred::di_subprogram::flag, nodeId, disubprogram.getFlags());

  // Record variables
#if LLVM_VERSION_MAJOR < 4
  const auto& variables = disubprogram.getVariables();
  for (size_t i = 0; i < variables.size(); ++i) {
    // Record variable
    refmode_t varId = record_di_variable::record(*variables[i], proc);

    // Record variable - subprogram association
    proc.writeFact(pred::di_subprogram::variable, nodeId, i, varId);
  }
#endif

  // Record template parameters
  const auto& tplParams = disubprogram.getTemplateParams();
  for (size_t i = 0; i < tplParams.size(); ++i) {
    // Record template parameter
    refmode_t param = record_di_template_param::record(*tplParams[i], proc);

    // Record parameter - subprogram association
    proc.writeFact(pred::di_subprogram::template_param, nodeId, i, param);
  }
}

//----------------------------------------------------------------------------
// Process Debug Info Scopes
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_scope::write(
    const llvm::DIScope& discope, const refmode_t& nodeId, DIProc& proc) {
  using llvm::DIFile;
  using llvm::DILexicalBlock;
  using llvm::DILexicalBlockFile;
  using llvm::DINamespace;
  using llvm::DISubprogram;
  using llvm::DIType;

  if (const auto* dins = dyn_cast<DINamespace>(&discope)) {
    write_di_namespace::write(*dins, nodeId, proc);
    return;
  }

  if (const auto* difile = dyn_cast<DIFile>(&discope)) {
    write_di_file::write(*difile, nodeId, proc);
    return;
  }

  if (const auto* ditype = dyn_cast<DIType>(&discope)) {
    write_di_type::write(*ditype, nodeId, proc);
    return;
  }

  if (const auto* disp = dyn_cast<DISubprogram>(&discope)) {
    write_di_subprogram::write(*disp, nodeId, proc);
    return;
  }

  if (const auto* dilb = dyn_cast<DILexicalBlock>(&discope)) {
    write_di_lex_block::write(*dilb, nodeId, proc);
    return;
  }

  if (const auto* dilb = dyn_cast<DILexicalBlockFile>(&discope)) {
    write_di_lex_block_file::write(*dilb, nodeId, proc);
    return;
  }
}
