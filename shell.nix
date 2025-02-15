# simple.nix
with (import <nixpkgs> {});
  mkShell {
    buildInputs = [
      msr-tools
      gcc
      linuxPackages.kernel.dev
    ];
  }
