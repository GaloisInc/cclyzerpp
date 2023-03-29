{ lib
, fetchFromGitHub
, cmake
, llvmPackages_14
, boost
, souffle
, ninja
}:

let
  inherit (llvmPackages_14) llvm stdenv openmp;
in
stdenv.mkDerivation {
  pname = "cclyzerpp";
  version = "0.7.0";

  src = ./.;

  nativeBuildInputs = [
    cmake
    llvm
    boost
    souffle
    openmp
  ];

  buildInputs = [
    souffle
  ];

  meta = with lib; {
    description = "cclyzer++ is a precise and scalable global pointer analysis for LLVM code";
    homepage = "https://github.com/GaloisInc/cclyzerpp/tree/main";
    license = licenses.bsd3;
  };
}
