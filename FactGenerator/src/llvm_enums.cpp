#include "llvm_enums.hpp"

#include <llvm/Config/llvm-config.h>   // for LLVM_VERSION_MAJOR, LLVM_VERSI...
#include <llvm/Support/raw_ostream.h>  // for errs, raw_ostream, raw_fd_ostream

using std::string;

auto cclyzer::utils::to_string(llvm::CallingConv::ID cc) -> string {
  string conv;

  switch (cc) {
    // TODO: add the remaining calling conventions
    case llvm::CallingConv::Fast:
      conv = "fastcc";
      break;
    case llvm::CallingConv::Cold:
      conv = "coldcc";
      break;
    case llvm::CallingConv::X86_FastCall:
      conv = "x86_fastcallcc";
      break;
    case llvm::CallingConv::X86_StdCall:
      conv = "x86_stdcallcc";
      break;
    case llvm::CallingConv::X86_ThisCall:
      conv = "x86_thiscallcc";
      break;
    case llvm::CallingConv::Intel_OCL_BI:
      conv = "intel_ocl_bicc";
      break;
    case llvm::CallingConv::ARM_AAPCS:
      conv = "arm_aapcscc";
      break;
    case llvm::CallingConv::ARM_AAPCS_VFP:
      conv = "arm_aapcs_vfpcc";
      break;
    case llvm::CallingConv::ARM_APCS:
      conv = "arm_apcscc";
      break;
    case llvm::CallingConv::MSP430_INTR:
      conv = "msp430_intrcc";
      break;
    case llvm::CallingConv::PTX_Device:
      conv = "tx_device";
      break;
    case llvm::CallingConv::PTX_Kernel:
      conv = "ptx_kernel";
      break;
    default:
      conv = "cc" + to_string(cc);
      break;
  }
  return conv;
}

auto cclyzer::utils::to_string(llvm::GlobalVariable::ThreadLocalMode TLM)
    -> string {
  const char *tlm;

  switch (TLM) {
    case llvm::GlobalVariable::NotThreadLocal:
      tlm = "";
      break;
    case llvm::GlobalVariable::GeneralDynamicTLSModel:
      tlm = "thread_local";
      break;
    case llvm::GlobalVariable::LocalDynamicTLSModel:
      tlm = "thread_local(localdynamic)";
      break;
    case llvm::GlobalVariable::InitialExecTLSModel:
      tlm = "thread_local(initialexec)";
      break;
    case llvm::GlobalVariable::LocalExecTLSModel:
      tlm = "thread_local(localexec)";
      break;
    default:
      llvm::errs() << "Unrecognized thread local mode: " << TLM << '\n';
      tlm = "<invalid tlm>";
  }
  return tlm;
}

auto cclyzer::utils::to_string(llvm::GlobalValue::LinkageTypes LT) -> string {
  const char *linkTy;
  using llvm::GlobalValue;

  switch (LT) {
    // TODO do not output default linkage, ie external
    case GlobalValue::ExternalLinkage:
      linkTy = "external";
      break;

    case GlobalValue::PrivateLinkage:
      linkTy = "private";
      break;
    case GlobalValue::InternalLinkage:
      linkTy = "internal";
      break;
    case GlobalValue::LinkOnceAnyLinkage:
      linkTy = "linkonce";
      break;
    case GlobalValue::LinkOnceODRLinkage:
      linkTy = "linkonce_odr";
      break;
    case GlobalValue::WeakAnyLinkage:
      linkTy = "weak";
      break;
    case GlobalValue::WeakODRLinkage:
      linkTy = "weak_odr";
      break;
    case GlobalValue::CommonLinkage:
      linkTy = "common";
      break;
    case GlobalValue::AppendingLinkage:
      linkTy = "appending";
      break;
    case GlobalValue::ExternalWeakLinkage:
      linkTy = "extern_weak";
      break;
    case GlobalValue::AvailableExternallyLinkage:
      linkTy = "available_externally";
      break;
    default:
      llvm::errs() << "Unrecognized linkage type: " << LT << '\n';
      linkTy = "<invalid linkage>";
      break;
  }
  return linkTy;
}

auto cclyzer::utils::to_string(llvm::GlobalValue::VisibilityTypes Vis)
    -> string {
  const char *visibility;
  using llvm::GlobalValue;

  switch (Vis) {
    // TODO do not output default visibility, ie external
    case GlobalValue::DefaultVisibility:
      visibility = "default";
      break;

    case GlobalValue::HiddenVisibility:
      visibility = "hidden";
      break;
    case GlobalValue::ProtectedVisibility:
      visibility = "protected";
      break;
    default:
      llvm::errs() << "Unrecognized visibility type: " << Vis << '\n';
      visibility = "<invalid visibility>";
      break;
  }

  return visibility;
}

auto cclyzer::utils::to_string(llvm::AtomicOrdering ordering) -> string {
  const char *atomic;

#if LLVM_VERSION_MAJOR > 3 || \
    (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9)
  using llvm::AtomicOrdering;
#endif

  // Newer LLVM versions use scoped enums
  switch (ordering) {
#if LLVM_VERSION_MAJOR > 3 || \
    (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9)
    case AtomicOrdering::NotAtomic:
      atomic = "";
      break;
    case AtomicOrdering::Unordered:
      atomic = "unordered";
      break;
    case AtomicOrdering::Monotonic:
      atomic = "monotonic";
      break;
    case AtomicOrdering::Acquire:
      atomic = "acquire";
      break;
    case AtomicOrdering::Release:
      atomic = "release";
      break;
    case AtomicOrdering::AcquireRelease:
      atomic = "acq_rel";
      break;
    case AtomicOrdering::SequentiallyConsistent:
      atomic = "seq_cst";
      break;
#else
    case llvm::NotAtomic:
      atomic = "";
      break;
    case llvm::Unordered:
      atomic = "unordered";
      break;
    case llvm::Monotonic:
      atomic = "monotonic";
      break;
    case llvm::Acquire:
      atomic = "acquire";
      break;
    case llvm::Release:
      atomic = "release";
      break;
    case llvm::AcquireRelease:
      atomic = "acq_rel";
      break;
    case llvm::SequentiallyConsistent:
      atomic = "seq_cst";
      break;
#endif
    default:
      llvm::errs() << "Unrecognized atomic ordering type: "
                   << static_cast<int>(ordering) << '\n';
      atomic = "<invalid atomic ordering>";
      break;
  }
  return atomic;
}
