{
  description = "Pi4 KVM Project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  #
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
      in {
        devShells.default = with pkgs;
          mkShell {
            buildInputs = [
              openssl
              pkgs.libusb1
              libevdev
              (pkgs.python3.withPackages (python-pkgs: [
                python-pkgs.hid
              ]))
              pkg-config
              minicom
              cmake
            ];
          };
      }
    );
}
