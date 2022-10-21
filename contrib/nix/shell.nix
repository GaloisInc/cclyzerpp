{ pkgs ? import <nixpkgs> { }
, unstable ? import <unstable> { }
}:

pkgs.mkShell {
  buildInputs =  [
    pkgs.clang_10
    pkgs.cmake
    pkgs.llvm_10
    pkgs.ninja
  ];
}
