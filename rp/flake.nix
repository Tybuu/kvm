{
  description = "A devShell example with cross-compilation support";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = import nixpkgs {
          inherit system;
        };
        pkgsCross = pkgs.pkgsCross.aarch64-embedded;
      in {
        devShells = {
          default = pkgsCross.mkShell {
            nativeBuildInputs = with pkgs; [
              bear
              wget
            ];
            buildInputs = with pkgs; [
              openssl
              coreutils
            ];
          };
          # Native Development Shell
        };
      }
    );
}
