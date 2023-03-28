{ lib
, stdenv
, fetchFromGitHub
, cmake
, llvmPackages_14
, boost
, souffle
, useUBSAN ? true
, buildInterface ? false
}:

let
  inherit (llvmPackages_14) llvm;
in
stdenv.mkDerivation rec {
  pname = "cclyzerpp";
  version = "0.7.0";

  src = fetchFromGitHub {
    owner = "GaloisInc";
    repo = "cclyzerpp";
    rev = "v${version}";
    hash = "sha256-mEL/Lv4RngD754PpKClPYk4I2U9PBcdI6FIhBv+d9Ms=";
  };

  nativeBuildInputs = [
    cmake
    llvm
    boost
    souffle
  ];

  buildInputs = [
    souffle
  ];

  CMAKE_C_FLAGS = lib.optional useUBSAN [ "-DUBSAN=1" ];
  
  buildPhase = ''
    cmake --build . -- factgen-exe ${lib.optionalString buildInterface "PAPass" }
  '';

  installPhase = ''
    mkdir -p $out/bin

    # could not build PAPass, no output
    cp factgen-exe $out/bin
    cp -r ../datalog $out

    # we should use this to install
    # cmake --install .
  '';
  
  meta = with lib; {
    description = "Cclyzer++ is a precise and scalable global pointer analysis for LLVM code";
    homepage = "https://github.com/GaloisInc/cclyzerpp/tree/main";
    license = licenses.bsd3;
  };
}
