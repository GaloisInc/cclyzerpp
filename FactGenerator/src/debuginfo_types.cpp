#include "DebugInfoProcessorImpl.hpp"
#include "debuginfo_predicate_groups.hpp"

using cclyzer::DebugInfoProcessor;
using cclyzer::refmode_t;
using llvm::cast;
using llvm::dyn_cast;
using std::string;
namespace pred = cclyzer::predicates;
namespace dwarf = llvm::dwarf;

//----------------------------------------------------------------------------
// Process Debug Info Types
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_type::write(
    const llvm::DIType& ditype, const refmode_t& nodeId, DIProc& proc) {
  using llvm::DIBasicType;
  using llvm::DICompositeType;
  using llvm::DIDerivedType;
  using llvm::DISubroutineType;

  if (const auto* basictype = dyn_cast<DIBasicType>(&ditype))
    write_di_basic_type::write(*basictype, nodeId, proc);
  else if (const auto* comptype = dyn_cast<DICompositeType>(&ditype))
    write_di_composite_type::write(*comptype, nodeId, proc);
  else if (const auto* derivedtype = dyn_cast<DIDerivedType>(&ditype))
    write_di_derived_type::write(*derivedtype, nodeId, proc);
  else if (const auto* subrtype = dyn_cast<DISubroutineType>(&ditype))
    write_di_subroutine_type::write(*subrtype, nodeId, proc);
}

//----------------------------------------------------------------------------
// Process Debug Info Basic Types
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_basic_type::write(
    const llvm::DIBasicType& ditype, const refmode_t& nodeId, DIProc& proc) {
  proc.write_di_type_common(ditype, nodeId);
  proc.writeFact(pred::di_basic_type::id, nodeId);
}

//----------------------------------------------------------------------------
// Process Debug Info Composite Types
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_composite_type::write(
    const llvm::DICompositeType& ditype,
    const refmode_t& nodeId,
    DIProc& proc) {
  using llvm::DIEnumerator;
  using llvm::DISubrange;
  using llvm::DIType;

  proc.write_di_type_common(ditype, nodeId);
  proc.writeFact(pred::di_composite_type::id, nodeId);

  // Record exact kind of derived type
  switch (ditype.getTag()) {
    case dwarf::Tag::DW_TAG_structure_type:
      proc.writeFact(pred::di_composite_type::structures, nodeId);
      break;
    case dwarf::Tag::DW_TAG_class_type:
      proc.writeFact(pred::di_composite_type::classes, nodeId);
      break;
    case dwarf::Tag::DW_TAG_array_type:
      proc.writeFact(pred::di_composite_type::arrays, nodeId);
      break;
    case dwarf::Tag::DW_TAG_union_type:
      proc.writeFact(pred::di_composite_type::unions, nodeId);
      break;
    case dwarf::Tag::DW_TAG_enumeration_type:
      proc.writeFact(pred::di_composite_type::enumerations, nodeId);
      break;
  }

#if LLVM_VERSION_MAJOR < 4  // const char * -> StringRef
  const char* tagStr = dwarf::TagString(ditype.getTag());
  proc.writeFact(pred::di_composite_type::kind, nodeId, tagStr);
#else
  llvm::StringRef tagStr = dwarf::TagString(ditype.getTag());
  proc.writeFact(pred::di_composite_type::kind, nodeId, tagStr.str());
#endif

  // Record ABI Identifier for this composite type
#if LLVM_VERSION_MAJOR > 12
  const string abiId = ditype.getIdentifier().str();
#else
  const string abiId = ditype.getIdentifier();
#endif

  proc.writeFact(pred::di_composite_type::abi_id, nodeId, abiId);

  // Record fields of composite type
  const llvm::DINodeArray& elements = ditype.getElements();

  for (size_t i = 0; i < elements.size(); i++) {
    if (const DIType* field = dyn_cast<DIType>(elements[i])) {
      refmode_t fieldId = record_di_type::record(*field, proc);
      proc.writeFact(pred::di_composite_type::field, nodeId, i, fieldId);
    } else if (const auto* enumfld = dyn_cast<DIEnumerator>(elements[i])) {
      refmode_t enumId = record_di_enumerator::record(*enumfld, proc);
      proc.writeFact(pred::di_composite_type::enumerator, nodeId, i, enumId);
    } else if (const auto* range = dyn_cast<DISubrange>(elements[i])) {
      refmode_t rangeId = record_di_subrange::record(*range, proc);
      proc.writeFact(pred::di_composite_type::subrange, nodeId, i, rangeId);
    }
  }

  // Record base type
  proc.recordUnionAttribute<pred::di_composite_type::basetype, write_di_type>(
      nodeId, ditype.getBaseType());

  // Record v-table
  proc.recordUnionAttribute<pred::di_composite_type::vtable, write_di_type>(
      nodeId, ditype.getVTableHolder());

  // Record template parameters
  const auto& tplParams = ditype.getTemplateParams();
  for (size_t i = 0; i < tplParams.size(); ++i) {
    // Record template parameter
    refmode_t param = record_di_template_param::record(*tplParams[i], proc);

    // Record parameter - type association
    proc.writeFact(pred::di_composite_type::template_param, nodeId, i, param);
  }

  // Record file information for type
  if (const llvm::DIFile* difile = ditype.getFile()) {
    refmode_t fileId = record_di_file::record(*difile, proc);
    proc.writeFact(pred::di_composite_type::file, nodeId, fileId);
  }
}

//----------------------------------------------------------------------------
// Process Debug Info Derived Types
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_derived_type::write(
    const llvm::DIDerivedType& ditype, const refmode_t& nodeId, DIProc& proc) {
  proc.write_di_type_common(ditype, nodeId);
  proc.writeFact(pred::di_derived_type::id, nodeId);

  // Record exact kind of derived type
#if LLVM_VERSION_MAJOR < 4  // const char * -> StringRef
  const char* tagStr = dwarf::TagString(ditype.getTag());
  proc.writeFact(pred::di_derived_type::kind, nodeId, tagStr);
#else
  llvm::StringRef tagStr = dwarf::TagString(ditype.getTag());
  proc.writeFact(pred::di_derived_type::kind, nodeId, tagStr.str());
#endif

  // Record base type
  proc.recordUnionAttribute<pred::di_derived_type::basetype, write_di_type>(
      nodeId, ditype.getBaseType());

  // Record file information for type
  if (const llvm::DIFile* difile = ditype.getFile()) {
    refmode_t fileId = record_di_file::record(*difile, proc);
    proc.writeFact(pred::di_derived_type::file, nodeId, fileId);
  }
}

//----------------------------------------------------------------------------
// Process Debug Info Subroutine Types
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_subroutine_type::write(
    const llvm::DISubroutineType& ditype,
    const refmode_t& nodeId,
    DIProc& proc) {
  using llvm::DIType;
  using llvm::MDString;

  proc.write_di_type_common(ditype, nodeId);
  proc.writeFact(pred::di_subroutine_type::id, nodeId);

  auto typeArray = ditype.getTypeArray();

  for (size_t i = 0; i < typeArray.size(); ++i) {
    if (const llvm::DIType* type = typeArray[i]) {
      const llvm::Metadata& meta = *type;

      if (const auto* mds = dyn_cast<MDString>(&meta)) {
#if LLVM_VERSION_MAJOR > 12
        std::string typeStr = mds->getString().str();
#else
        std::string typeStr = mds->getString();
#endif
        proc.writeFact(
            pred::di_subroutine_type::raw_type_elem, nodeId, i, typeStr);
      } else {
        const auto& typeNode = cast<DIType>(*type);
        refmode_t typeId = record_di_type::record(typeNode, proc);
        proc.writeFact(pred::di_subroutine_type::type_elem, nodeId, i, typeId);
      }
    }
  }
}

//----------------------------------------------------------------------------
// Write common Debug Info Type Attributes
//----------------------------------------------------------------------------

void DebugInfoProcessor::Impl::write_di_type_common(
    const llvm::DIType& ditype, const refmode_t& nodeId) {
#if LLVM_VERSION_MAJOR > 12
  const string name = ditype.getName().str();
#else
  const string name = ditype.getName();
#endif

  const unsigned line = ditype.getLine();

  writeFact(pred::di_type::id, nodeId);

  if (!name.empty()) {
    writeFact(pred::di_type::name, nodeId, name);
  }

  if (line) {  // zero indicates non-existence
    writeFact(pred::di_type::line, nodeId, line);
  }

  // Record bit sizes
  switch (ditype.getTag()) {
    // Skip sizeless entries
    case dwarf::Tag::DW_TAG_typedef:
    case dwarf::Tag::DW_TAG_const_type:
    case dwarf::Tag::DW_TAG_restrict_type:
    case dwarf::Tag::DW_TAG_subroutine_type:
      break;
    default: {
      const uint64_t bitSize = ditype.getSizeInBits();
      const uint64_t bitAlign = ditype.getAlignInBits();
      const uint64_t bitOffset = ditype.getOffsetInBits();

      writeFact(pred::di_type::bitsize, nodeId, bitSize);
      writeFact(pred::di_type::bitalign, nodeId, bitAlign);
      writeFact(pred::di_type::bitoffset, nodeId, bitOffset);
    }
  }

  // Record enclosing scope
  recordUnionAttribute<pred::di_type::scope, write_di_scope>(
      nodeId, ditype.getScope());

  // Record flags
  recordFlags(pred::di_type::flag, nodeId, ditype.getFlags());
}
