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
    uint64_t allocSize = layout.getTypeAllocSize(const_cast<Type *>(type));
    uint64_t storeSize = layout.getTypeStoreSize(const_cast<Type *>(type));

    // Store size of type in bytes
    refmode_t typeId = gen.refmode<llvm::Type>(*type);

    gen.writeFact(pred::type::alloc_size, typeId, allocSize);
    gen.writeFact(pred::type::store_size, typeId, storeSize);
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
  refmode_t typeId = gen.refmode<llvm::Type>(*ptrType);

  // Record pointer type entity
  gen.writeFact(pred::ptr_type::id, typeId);

  // Record pointer address space
  if (unsigned addressSpace = ptrType->getPointerAddressSpace()) {
    gen.writeFact(pred::ptr_type::addr_space, typeId, addressSpace);
  }

  // Record pointer element type
#if LLVM_VERSION_MAJOR > 14
  auto cond = !ptrType->isOpaque();
#else
  auto cond = true;
#endif
  if (cond) {
    const llvm::Type *elemType = ptrType->getPointerElementType();
    refmode_t elemTypeId = gen.refmode<llvm::Type>(*elemType);
    gen.writeFact(pred::ptr_type::component_type, typeId, elemTypeId);
  }
}

void TypeVisitor::visitArrayType(const ArrayType *arrayType) {
  const llvm::Type *elemType = arrayType->getArrayElementType();
  size_t nElements = arrayType->getArrayNumElements();

  refmode_t typeId = gen.refmode<llvm::Type>(*arrayType);
  refmode_t elemTypeId = gen.refmode<llvm::Type>(*elemType);

  gen.writeFact(pred::array_type::id, typeId);
  gen.writeFact(pred::array_type::component_type, typeId, elemTypeId);
  gen.writeFact(pred::array_type::size, typeId, nElements);
}

void TypeVisitor::visitStructType(const StructType *structType) {
  using llvm::StructLayout;

  refmode_t tref = gen.refmode<llvm::Type>(*structType);
  unsigned nFields = structType->getStructNumElements();

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
    const StructLayout *structLayout =
        layout.getStructLayout(const_cast<StructType *>(structType));

    // Record struct field types
    for (unsigned i = 0; i < nFields; i++) {
      refmode_t fieldType =
          gen.refmode<llvm::Type>(*(structType->getStructElementType(i)));

      uint64_t fieldOffset = structLayout->getElementOffset(i);
      uint64_t fieldBitOffset = structLayout->getElementOffsetInBits(i);

      gen.writeFact(pred::struct_type::field_type, tref, i, fieldType);
      gen.writeFact(pred::struct_type::field_offset, tref, i, fieldOffset);
      gen.writeFact(
          pred::struct_type::field_bit_offset, tref, i, fieldBitOffset);
    }

    // Record number of fields
    gen.writeFact(pred::struct_type::nfields, tref, nFields);
  }
}

void TypeVisitor::visitFunctionType(const FunctionType *functionType) {
  unsigned nParameters = functionType->getFunctionNumParams();
  const llvm::Type *returnType = functionType->getReturnType();

  refmode_t funcId = gen.refmode<llvm::Type>(*functionType);
  refmode_t returnTypeId = gen.refmode<llvm::Type>(*returnType);

  // Record function type entity
  gen.writeFact(pred::func_type::id, funcId);

  // TODO: which predicate/entity do we need to update for varagrs?
  if (functionType->isVarArg()) {
    gen.writeFact(pred::func_type::varargs, funcId);
  }

  // Record return type
  gen.writeFact(pred::func_type::return_type, funcId, returnTypeId);

  // Record function formal parameters
  for (unsigned i = 0; i < nParameters; i++) {
    const llvm::Type *paramType = functionType->getFunctionParamType(i);
    refmode_t paramTypeId = gen.refmode<llvm::Type>(*paramType);

    gen.writeFact(pred::func_type::param_type, funcId, i, paramTypeId);
  }

  // Record number of formal parameters
  gen.writeFact(pred::func_type::nparams, funcId, nParameters);
}

void TypeVisitor::visitVectorType(const VectorType *vectorType) {
  refmode_t tref = gen.refmode<llvm::Type>(*vectorType);
#if LLVM_VERSION_MAJOR > 10
  Type *componentType = vectorType->getElementType();
  if (llvm::isa<llvm::FixedVectorType>(vectorType)) {
    size_t nElements =
        llvm::cast<llvm::FixedVectorType>(vectorType)->getNumElements();
    gen.writeFact(pred::vector_type::size, tref, nElements);
  }
#else
  Type *componentType = vectorType->getVectorElementType();
  size_t nElements = vectorType->getVectorNumElements();
  gen.writeFact(pred::vector_type::size, tref, nElements);
#endif

  // Record vector type entity
  gen.writeFact(pred::vector_type::id, tref);

  // Record vector component type
  refmode_t compref = gen.refmode<llvm::Type>(*componentType);
  gen.writeFact(pred::vector_type::component_type, tref, compref);
}
