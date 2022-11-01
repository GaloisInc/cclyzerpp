#include "LlvmEnums.hpp"

#include <llvm/Support/raw_ostream.h>

#include <sstream>

#include "Unknown.hpp"

using std::string;

auto cclyzer::utils::to_string(llvm::CallingConv::ID cc) -> string {
  switch (cc) {
    case llvm::CallingConv::Fast:
      return "fastcc";
    case llvm::CallingConv::Cold:
      return "coldcc";
    case llvm::CallingConv::X86_FastCall:
      return "x86_fastcallcc";
    case llvm::CallingConv::X86_StdCall:
      return "x86_stdcallcc";
    case llvm::CallingConv::X86_ThisCall:
      return "x86_thiscallcc";
    case llvm::CallingConv::Intel_OCL_BI:
      return "intel_ocl_bicc";
    case llvm::CallingConv::ARM_AAPCS:
      return "arm_aapcscc";
    case llvm::CallingConv::ARM_AAPCS_VFP:
      return "arm_aapcs_vfpcc";
    case llvm::CallingConv::ARM_APCS:
      return "arm_apcscc";
    case llvm::CallingConv::MSP430_INTR:
      return "msp430_intrcc";
    case llvm::CallingConv::PTX_Device:
      return "tx_device";  // TODO(lb): Bug! Should be ptx_device
    case llvm::CallingConv::PTX_Kernel:
      return "ptx_kernel";
  }  // -Wswitch prevents fallthrough, no need for default case
  assert(false);
  return "";  // erroneous clang-tidy: clang-diagnostic-return-type
}

auto cclyzer::utils::to_string(llvm::GlobalVariable::ThreadLocalMode TLM)
    -> string {
  switch (TLM) {
    case llvm::GlobalVariable::NotThreadLocal:
      return "";
    case llvm::GlobalVariable::GeneralDynamicTLSModel:
      return "thread_local";
    case llvm::GlobalVariable::LocalDynamicTLSModel:
      return "thread_local(localdynamic)";
    case llvm::GlobalVariable::InitialExecTLSModel:
      return "thread_local(initialexec)";
    case llvm::GlobalVariable::LocalExecTLSModel:
      return "thread_local(localexec)";
  }  // -Wswitch prevents fallthrough, no need for default case
  assert(false);
}

auto cclyzer::utils::to_string(llvm::GlobalValue::LinkageTypes LT) -> string {
  using llvm::GlobalValue;
  switch (LT) {
    case GlobalValue::ExternalLinkage:
      return "external";
    case GlobalValue::PrivateLinkage:
      return "private";
    case GlobalValue::InternalLinkage:
      return "internal";
    case GlobalValue::LinkOnceAnyLinkage:
      return "linkonce";
    case GlobalValue::LinkOnceODRLinkage:
      return "linkonce_odr";
    case GlobalValue::WeakAnyLinkage:
      return "weak";
    case GlobalValue::WeakODRLinkage:
      return "weak_odr";
    case GlobalValue::CommonLinkage:
      return "common";
    case GlobalValue::AppendingLinkage:
      return "appending";
    case GlobalValue::ExternalWeakLinkage:
      return "extern_weak";
    case GlobalValue::AvailableExternallyLinkage:
      return "available_externally";
  }  // -Wswitch prevents fallthrough, no need for default case
  assert(false);
}

auto cclyzer::utils::to_string(llvm::GlobalValue::VisibilityTypes Vis)
    -> string {
  using llvm::GlobalValue;
  switch (Vis) {
    case GlobalValue::DefaultVisibility:
      return "default";
    case GlobalValue::HiddenVisibility:
      return "hidden";
    case GlobalValue::ProtectedVisibility:
      return "protected";
  }  // -Wswitch prevents fallthrough, no need for default case
  assert(false);
}

auto cclyzer::utils::to_string(llvm::AtomicOrdering ordering) -> string {
  using llvm::AtomicOrdering;
  switch (ordering) {
    case AtomicOrdering::NotAtomic:
      return "";
    case AtomicOrdering::Unordered:
      return "unordered";
    case AtomicOrdering::Monotonic:
      return "monotonic";
    case AtomicOrdering::Acquire:
      return "acquire";
    case AtomicOrdering::Release:
      return "release";
    case AtomicOrdering::AcquireRelease:
      return "acq_rel";
    case AtomicOrdering::SequentiallyConsistent:
      return "seq_cst";
  }  // -Wswitch prevents fallthrough, no need for default case
  assert(false);
}
