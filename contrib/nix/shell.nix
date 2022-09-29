{ pkgs ? import <nixpkgs> { }
, unstable ? import <unstable> { }
}:

pkgs.mkShell {
  buildInputs =  [
    pkgs.clang_11
    pkgs.cmake
    pkgs.llvm_11
    pkgs.ninja
  ];
}
