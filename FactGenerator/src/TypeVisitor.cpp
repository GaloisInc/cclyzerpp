#include "TypeVisitor.hpp"

#include "FactGenerator.hpp"
#include "PredicateGroups.hpp"
#include "Unknown.hpp"

using cclyzer::TypeVisitor;
namespace pred = cclyzer::predicates;

// Add basic LLVM types to current namespace
using llvm::ArrayType;
using llvm::cast;
using llvm::FunctionType;
using llvm::PointerType;
using llvm::StructType;
using llvm::Type;
using llvm::VectorType;

//-------------------------------------------------------------------
// Methods for recording different kinds of LLVM types.
//-------------------------------------------------------------------

void TypeVisitor::visitType(const llvm::Type *type) {
  // Record type sizes while skipping unsized types (e.g.,
  // labels, functions)

  if (type->isSized()) {
    uint64_t alloc_size = layout.getTypeAllocSize(const_cast<Type *>(type));
    uint64_t store_size = layout.getTypeStoreSize(const_cast<Type *>(type));

    // Store size of type in bytes
    refmode_t type_id = gen.refmode<llvm::Type>(*type);

    gen.writeFact(pred::type::alloc_size, type_id, alloc_size);
    gen.writeFact(pred::type::store_size, type_id, store_size);
  }

  refmode_t tref = gen.refmode<llvm::Type>(*type);

  // Record each different kind of type
  switch (type->getTypeID()) {  // Fallthrough is intended
    case Type::VoidTyID:
    case Type::LabelTyID:
    case Type::MetadataTyID:
      gen.writeFact(pred::primitive_type::id, tref);
      return;
    case Type::HalfTyID:  // Fallthrough to all 6 floating point types
    case Type::FloatTyID:
    case Type::DoubleTyID:
    case Type::X86_FP80TyID:
    case Type::FP128TyID:
    case Type::PPC_FP128TyID:
      assert(type->isFloatingPointTy());
      gen.writeFact(pred::fp_type::id, tref);
      return;
    case llvm::Type::IntegerTyID:
      gen.writeFact(pred::integer_type::id, tref);
      return;
    case llvm::Type::FunctionTyID:
      visitFunctionType(cast<FunctionType>(type));
      return;
    case llvm::Type::StructTyID:
      visitStructType(cast<StructType>(type));
      return;
    case llvm::Type::ArrayTyID:
      visitArrayType(cast<ArrayType>(type));
      return;
    case llvm::Type::PointerTyID:
      visitPointerType(cast<PointerType>(type));
      return;
#if LLVM_VERSION_MAJOR > 10
    case llvm::Type::ScalableVectorTyID:
      visitVectorType(cast<VectorType>(type));
      return;
    case llvm::Type::FixedVectorTyID:
      visitVectorType(cast<VectorType>(type));
      return;
#else
    case llvm::Type::VectorTyID:
      visitVectorType(cast<VectorType>(type));
      return;
#endif
    case llvm::Type::X86_MMXTyID:  // TODO: handle this type
      unknown("type", type);
      return;
    case llvm::Type::TokenTyID:  // TODO: handle this type
      unknown("type", type);
      return;
#if LLVM_VERSION_MAJOR > 10
    case llvm::Type::BFloatTyID:  // TODO: handle this type
      unknown("type", type);
      return;
#endif
#if LLVM_VERSION_MAJOR > 11
    case llvm::Type::X86_AMXTyID:  // TODO: handle this type
      unknown("type", type);
      return;
#endif
#if LLVM_VERSION_MAJOR == 15
    case llvm::Type::DXILPointerTyID:  // TODO: handle this type
      unknown("type", type);
      return;
#endif
#if LLVM_VERSION_MAJOR > 15
    case llvm::Type::TypedPointerTyID:  // TODO: handle this type
      unknown("type", type);
      return;
#endif
  }  // -Wswitch prevents fallthrough, no need for default case
  assert(false);
}

void TypeVisitor::visitPointerType(const PointerType *ptrType) {
  refmode_t type_id = gen.refmode<llvm::Type>(*ptrType);

  // Record pointer type entity
  gen.writeFact(pred::ptr_type::id, type_id);

  // Record pointer address space
  if (unsigned address_space = ptrType->getPointerAddressSpace()) {
    gen.writeFact(pred::ptr_type::addr_space, type_id, address_space);
  }

  // Record pointer element type
#if LLVM_VERSION_MAJOR > 14
  auto cond = !ptrType->isOpaque();
#else
  auto cond = true;
#endif
  if (cond) {
    const llvm::Type *elem_type = ptrType->getPointerElementType();
    refmode_t elem_type_id = gen.refmode<llvm::Type>(*elem_type);
    gen.writeFact(pred::ptr_type::component_type, type_id, elem_type_id);
  }
}

void TypeVisitor::visitArrayType(const ArrayType *arrayType) {
  const llvm::Type *elem_type = arrayType->getArrayElementType();
  size_t n_elements = arrayType->getArrayNumElements();

  refmode_t type_id = gen.refmode<llvm::Type>(*arrayType);
  refmode_t elem_type_id = gen.refmode<llvm::Type>(*elem_type);

  gen.writeFact(pred::array_type::id, type_id);
  gen.writeFact(pred::array_type::component_type, type_id, elem_type_id);
  gen.writeFact(pred::array_type::size, type_id, n_elements);
}

void TypeVisitor::visitStructType(const StructType *structType) {
  using llvm::StructLayout;

  refmode_t tref = gen.refmode<llvm::Type>(*structType);
  unsigned n_fields = structType->getStructNumElements();

  // Record struct type entity
  gen.writeFact(pred::struct_type::id, tref);

  // Write names of named struct types
  if (structType->hasName()) {
    gen.writeFact(
        pred::struct_type::has_name, tref, structType->getName().str());
  }

  if (structType->isOpaque()) {
    // Opaque structs carry no info about their internal structure
    gen.writeFact(pred::struct_type::opaque, tref);
  } else {
    // Get struct layout
    const StructLayout *struct_layout =
        layout.getStructLayout(const_cast<StructType *>(structType));

    // Record struct field types
    for (unsigned i = 0; i < n_fields; i++) {
      refmode_t field_type =
          gen.refmode<llvm::Type>(*(structType->getStructElementType(i)));

      uint64_t field_offset = struct_layout->getElementOffset(i);
      uint64_t field_bit_offset = struct_layout->getElementOffsetInBits(i);

      gen.writeFact(pred::struct_type::field_type, tref, i, field_type);
      gen.writeFact(pred::struct_type::field_offset, tref, i, field_offset);
      gen.writeFact(
          pred::struct_type::field_bit_offset, tref, i, field_bit_offset);
    }

    // Record number of fields
    gen.writeFact(pred::struct_type::nfields, tref, n_fields);
  }
}

void TypeVisitor::visitFunctionType(const FunctionType *functionType) {
  unsigned n_parameters = functionType->getFunctionNumParams();
  const llvm::Type *return_type = functionType->getReturnType();

  refmode_t func_id = gen.refmode<llvm::Type>(*functionType);
  refmode_t return_type_id = gen.refmode<llvm::Type>(*return_type);

  // Record function type entity
  gen.writeFact(pred::func_type::id, func_id);

  // TODO: which predicate/entity do we need to update for varagrs?
  if (functionType->isVarArg()) {
    gen.writeFact(pred::func_type::varargs, func_id);
  }

  // Record return type
  gen.writeFact(pred::func_type::return_type, func_id, return_type_id);

  // Record function formal parameters
  for (unsigned i = 0; i < n_parameters; i++) {
    const llvm::Type *param_type = functionType->getFunctionParamType(i);
    refmode_t param_type_id = gen.refmode<llvm::Type>(*param_type);

    gen.writeFact(pred::func_type::param_type, func_id, i, param_type_id);
  }

  // Record number of formal parameters
  gen.writeFact(pred::func_type::nparams, func_id, n_parameters);
}

void TypeVisitor::visitVectorType(const VectorType *vectorType) {
  refmode_t tref = gen.refmode<llvm::Type>(*vectorType);
#if LLVM_VERSION_MAJOR > 10
  Type *component_type = vectorType->getElementType();
  if (llvm::isa<llvm::FixedVectorType>(vectorType)) {
    size_t n_elements =
        llvm::cast<llvm::FixedVectorType>(vectorType)->getNumElements();
    gen.writeFact(pred::vector_type::size, tref, n_elements);
  }
#else
  Type *component_type = vectorType->getVectorElementType();
  size_t n_elements = vectorType->getVectorNumElements();
  gen.writeFact(pred::vector_type::size, tref, n_elements);
#endif

  // Record vector type entity
  gen.writeFact(pred::vector_type::id, tref);

  // Record vector component type
  refmode_t compref = gen.refmode<llvm::Type>(*component_type);
  gen.writeFact(pred::vector_type::component_type, tref, compref);
}
