{ pkgs ? import <nixpkgs> {} }:

let
  kernel = pkgs.linuxPackages.kernel;
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
    kernel.dev    # Kernel headers needed for compilation
    pkgs.gcc
    pkgs.binutils
    pkgs.msr-tools
    pkgs.gcc
    pkgs.linuxPackages.kernel.dev
    
    pkgs.bear          # to generate compile_commands.json
    pkgs.clang-tools   # for clangd
    pkgs.clang         # for compiling
    pkgs.glibc         # system headers

    pkgs.busybox            # for debugging binary file read
    
  ];

  shellHook = ''
    echo "Setting up Qemu"

    mkdir -p resources

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

    export CC=clang
    export CXX=clang++
  
    echo "done"
  '';
}
