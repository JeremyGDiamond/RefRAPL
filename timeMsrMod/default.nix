{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation rec {
  pname = "time_msr_mod";
  version = "0.1";

  src = ./src;

  # Use kernel headers from nixpkgs
  nativeBuildInputs = pkgs.linuxPackages.kernel.moduleBuildDependencies;

  makeFlags = [
    "KERNEL_SRC=${pkgs.linuxPackages.kernel.dev}/lib/modules/${pkgs.linuxPackages.kernel.modDirVersion}/build"
  ];

  installPhase = ''
    mkdir -p $out/lib/modules/${pkgs.linuxPackages.kernel.modDirVersion}/extra
    cp time_msr_mod.ko $out/lib/modules/${pkgs.linuxPackages.kernel.modDirVersion}/extra/
  '';

  meta = {
    description = "Copy of Msr Kernel Module With Time kept for every read";
    license = pkgs.lib.licenses.gpl2;
  };
}
