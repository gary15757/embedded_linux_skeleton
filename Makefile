export TFTP_DIR             := tftp_boot
export BUILD_DIR            := $(PWD)/build
export BUILDROOT_BUILD_DIR  := $(BUILD_DIR)/buildroot
export LINUX_BUILD_DIR      := $(BUILD_DIR)/linux
export LINUX_MOD_BUILD_DIR  := $(BUILD_DIR)/linux_mod
export UBOOT_BUILD_DIR      := $(BUILD_DIR)/uboot
export BIN_BUILD_DIR        := $(BUILD_DIR)/bin
export SCRIPT_BUILD_DIR     := $(PWD)/build_scripts
export SKELETON_ROOTFS_DIR  := $(SCRIPT_BUILD_DIR)/skeleton_rootfs
export ROOTFS_DIR           := $(BUILD_DIR)/rootfs

# follow https://elinux.org/RPi_U-Boot
# sudo apt-get install binutils-arm-linux-gnueabi gcc-arm-linux-gnueabi
export PATH := $(PWD)/build/buildroot/host/usr/bin:$(PATH)
export CROSS_COMPILE=arm-linux-
export ARCH=arm
#export USE_PRIVATE_LIBGCC=yes

NUM_OF_CPU := $(nproc)

all: compile_buildroot compile_uboot compile_linux_kernel make_disk

clean: clean_buildroot clean_linux_kernel clean_uboot clean_apps
	rm -rf $(BUILD_DIR)
	rm -rf $(TFTP_DIR)

$(BIN_BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILDROOT_BUILD_DIR)
	@mkdir -p $(LINUX_BUILD_DIR)
	@mkdir -p $(UBOOT_BUILD_DIR)
	@mkdir -p $(BIN_BUILD_DIR)

tftp_boot: compile_buildroot compile_linux_kernel compile_uboot
	@rm -rf $(TFTP_DIR)
	@mkdir $(TFTP_DIR)
	@cp $(LINUX_BUILD_DIR)/arch/x86/boot/bzImage $(TFTP_DIR)
	@cp $(BUILDROOT_BUILD_DIR)/images/rootfs.cpio $(TFTP_DIR)

compile_buildroot: $(BIN_BUILD_DIR)
	@echo "**********compile_buildroot**********"
	@cp configs/buildroot/config $(BUILDROOT_BUILD_DIR)/.config
	@$(MAKE) -C buildroot-2017.02.10 O=$(BUILDROOT_BUILD_DIR) > $(BUILD_DIR)/buildroot.log 2>&1
	@cp $(BUILDROOT_BUILD_DIR)/images/rootfs.cpio $(BIN_BUILD_DIR)
	@echo "**********done**********"

clean_buildroot:
	@rm -rf $(BUILDROOT_BUILD_DIR)

compile_linux_kernel: $(BIN_BUILD_DIR)
	@echo "**********compile_linux_kernel**********"
	@cp configs/linux/config $(LINUX_BUILD_DIR)/.config
	@$(MAKE) -j3 -C linux-4.14.22 O=$(LINUX_BUILD_DIR) > $(BUILD_DIR)/linux_kernel.log 2>&1
	@$(MAKE) -j3 -C linux-4.14.22 O=$(LINUX_BUILD_DIR) INSTALL_MOD_PATH=$(LINUX_MOD_BUILD_DIR) modules_install >> $(BUILD_DIR)/linux_kernel.log 2>&1
	@cp $(LINUX_BUILD_DIR)/arch/arm/boot/zImage                      $(BIN_BUILD_DIR)
	@cp $(LINUX_BUILD_DIR)/arch/arm/boot/dts/bcm2835-rpi-b-plus.dtb  $(BIN_BUILD_DIR)
	@echo "**********done**********"

clean_linux_kernel:
	@rm -rf $(LINUX_BUILD_DIR)

compile_uboot: $(BIN_BUILD_DIR)
	@echo "**********compile_uboot**********"
	@cp configs/uboot/config $(UBOOT_BUILD_DIR)/.config
	@$(MAKE) -j$(NUM_OF_CPU) -C u-boot_v2018.05-rc1 O=$(UBOOT_BUILD_DIR) > $(BUILD_DIR)/uboot.log 2>&1
	@cp $(UBOOT_BUILD_DIR)/u-boot $(UBOOT_BUILD_DIR)/u-boot.bin $(BIN_BUILD_DIR)
	@echo "**********done**********"

compile_apps: $(BIN_BUILD_DIR)
	@echo "**********compile_apps**********"
	@$(MAKE) -C applications all > $(BUILD_DIR)/apps.log 2>&1
	@echo "**********done**********"

clean_apps:
	@$(MAKE) -C applications clean

clean_uboot:
	@rm -rf $(UBOOT_BUILD_DIR)

make_disk: compile_apps
	@echo "**********make_disk**********"
	@rm -rf $(BUILD_DIR)/sdcard_boot
	@mkdir $(BUILD_DIR)/sdcard_boot

	@cp $(BIN_BUILD_DIR)/bcm2835-rpi-b-plus.dtb   $(BUILD_DIR)/sdcard_boot
	@cp pi-boot/start.elf                         $(BUILD_DIR)/sdcard_boot
	@cp pi-boot/fixup.dat                         $(BUILD_DIR)/sdcard_boot
	@cp pi-boot/bootcode.bin                      $(BUILD_DIR)/sdcard_boot

	@mkimage -C none -A arm -T script -d configs/boot.cmd  $(BUILD_DIR)/sdcard_boot/boot.scr
	@cp $(UBOOT_BUILD_DIR)/u-boot.bin    $(BUILD_DIR)/sdcard_boot/kernel.img
	@cp $(BIN_BUILD_DIR)/zImage          $(BUILD_DIR)/sdcard_boot
	@mkimage -A arm -T ramdisk -C none -n uInitrd -d $(BIN_BUILD_DIR)/rootfs.cpio $(BUILD_DIR)/sdcard_boot/uInitrd

	@fakeroot $(SCRIPT_BUILD_DIR)/fakeroot.sh

	@mkimage -f $(SCRIPT_BUILD_DIR)/fw_bcm2835_rpi_b_plus.its $(BUILD_DIR)/sdcard_boot/firmware

	@echo "**********done**********"
