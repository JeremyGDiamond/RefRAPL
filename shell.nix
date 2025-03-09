{ pkgs ? import <nixpkgs> {} }:

let
  kernel = pkgs.linuxPackages_latest.kernel;
  kernelHeaders = kernel.dev;
in
pkgs.mkShell {
  name = "kernel-dev-env";
  
  nativeBuildInputs = [
    pkgs.qemu
    pkgs.kmod        # For loading kernel modules
    pkgs.git
    pkgs.wget
    pkgs.cpio
  ];

  buildInputs = [
    kernelHeaders    # Kernel headers needed for compilation
    pkgs.gcc
    pkgs.binutils
    pkgs.msr-tools
    pkgs.gcc
    pkgs.linuxPackages.kernel.dev
  ];

  shellHook = ''
    echo "Setting up Qemu"

    if [ ! -f resources/nix.iso ]; then
      echo "Downloading Ubuntu minimal image..."
      wget -O resources/nix.iso https://channels.nixos.org/nixos-24.11/latest-nixos-gnome-x86_64-linux.iso
    fi

    # create qemuimage if it does not already exist
    if [ ! -f resources/kern_image ]; then
      qemu-img create -f raw resources/kern_image 12G
    fi

    # Function to start QEMU
    
    start-qemu() {
      qemu-system-x86_64 \
        -enable-kvm \
        -cdrom resources/nix.iso \
        -boot order=c \
        -drive file=resources/kern_image,format=raw \
        -m 4G \
        -net nic -net user,hostfwd=tcp::2222-:22
    }
  
    echo "done"
  '';
}
