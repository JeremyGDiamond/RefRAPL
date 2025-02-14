# simple.nix
with (import <nixpkgs> {});
  mkShell {
    buildInputs = [
      msr-tools
      gcc
    ];
  }
