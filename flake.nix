# NOTE: This configuration is not evaluated during Continuous Integration (CI) processes and may not
# be as well-maintained or up-to-date as the CMake and Docker setup. Use with caution.

{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-22.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        inherit (pkgs) callPackage;
      in
      rec {
        defaultPackage = packages.cclyzerpp;
        packages = {
          cclyzerpp = callPackage ./contrib/nix/default.nix { };
        };
      });
}


