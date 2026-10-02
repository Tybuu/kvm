{
  description = "Pi4 KVM Project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
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
        pkgs = import nixpkgs {inherit system;};
        pkgsPi4 = pkgs.pkgsCross.aarch64-multiplatform;
      in {
        devShells = {
          default = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              cmake
              pkg-config
              minicom
            ];
            buildInputs = with pkgs; [
              openssl
              libusb1
              libevdev
              udev
              (python3.withPackages (ps: [ps.hid]))
            ];
          };

          pi4 = pkgsPi4.mkShell {
            nativeBuildInputs = with pkgs; [
              cmake
              pkg-config
              minicom
            ];

            buildInputs = with pkgsPi4; [
              openssl
              libusb1
              libevdev
              udev
            ];
          };
        };
      }
    );
}
