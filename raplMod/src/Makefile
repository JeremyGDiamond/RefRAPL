# Define the module name
obj-m := rapl_ref_mod.o

# Kernel source directory (will be set by the Nix derivation)
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
EXTRA_CFLAGS += -I$(KERNEL_SRC)

# Default target: build the module
.PHONY: all
all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

# Install the module
.PHONY: install
install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install INSTALL_MOD_PATH=$(DESTDIR)

# Clean the build
.PHONY: clean
clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
