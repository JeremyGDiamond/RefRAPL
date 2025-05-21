{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation rec {
  pname = "micro_bench_mod";
  version = "0.1";

  src = ./src;

  # Use kernel headers from nixpkgs
  nativeBuildInputs = pkgs.linuxPackages.kernel.moduleBuildDependencies;

  makeFlags = [
    "KERNEL_SRC=${pkgs.linuxPackages.kernel.dev}/lib/modules/${pkgs.linuxPackages.kernel.modDirVersion}/build"
  ];

  installPhase = ''
    mkdir -p $out/lib/modules/${pkgs.linuxPackages.kernel.modDirVersion}/extra
    cp rapl_ref_mod.ko $out/lib/modules/${pkgs.linuxPackages.kernel.modDirVersion}/extra/
  '';

  meta = {
    description = "Micro Benchmark Kernel Module";
    license = pkgs.lib.licenses.gpl2;
  };
}
