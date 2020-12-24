include toolchain.mk

CURDIR=$(shell pwd)
CROSS_COMPILE=$(CURDIR)/toolchains/aarch32/bin/arm-linux-gnueabihf-
CROSS_COMPILE64=$(CURDIR)/toolchains/aarch64/bin/aarch64-linux-gnu-

#CROSS_COMPILE="aarch64-poky-linux-"
#CROSS_COMPILE64="aarch64-poky-linux-"

ARCH=arm64
PLATFORM=mx8mqevk
COMPILE_NS_USER= 64
LOCALVERSION=4.19.35

OUT_PATH			?=$(CURDIR)/out
# ROOTFS_PATH         ?=$(CURDIR)/rootfs
ROOTFS_PATH         ?=/media/yin/rootfs
BUILD_PATH			?= $(CURDIR)/build
UBOOT_PATH          ?= $(CURDIR)/uboot-imx
LINUX_PATH			?= $(CURDIR)/linux-imx
OPTEE_OS_PATH			?= $(CURDIR)/imx-optee-os
OPTEE_CLIENT_PATH		?= $(CURDIR)/imx-optee-client
OPTEE_CLIENT_EXPORT		?= $(OPTEE_CLIENT_PATH)/out/export
OPTEE_EXAMPLES_PATH     ?= $(CURDIR)/optee_examples
OPTEE_TEST_PATH			?= $(CURDIR)/imx-optee-test
OPTEE_TEST_OUT_PATH		?= $(CURDIR)/imx-optee-test/out
OPTEE_OS_TA_DEV_KIT_DIR	?= $(OPTEE_OS_PATH)/out/arm-plat-imx/export-ta_arm64

ATF_PATH        ?=$(CURDIR)/imx-atf
MKIMAGE_PATH    ?=$(CURDIR)/imx-mkimage
FIRMWARE_PATH   ?=$(CURDIR)/firmware-imx-8.5/firmware

CFG_TEE_CORE_LOG_LEVEL		:= 4

DEBUG :=0
#DEBUG :=1
ifeq ($(DEBUG),1)
TF_A_BUILD			?= debug
else
TF_A_BUILD			?= release
endif

define KERNEL_VERSION
$(shell cd $(LINUX_PATH) && $(MAKE) --no-print-directory kernelversion)
endef

################################################################################
# Targets
################################################################################
.PHONY: all

all: prepare imx-mkimage linux-imx optee-imx

.PHONY: clean
clean: uboot-imx-clean linux-imx-clean optee-imx-clean prepare-clean imx-mkimage-clean imx-atf-clean

.PHONY: prepare
prepare:
	mkdir -p $(OUT_PATH)

.PHONY: prepare-clean
prepare-clean:
	#rm -rf $(OUT_PATH)

################################################################################
# imx mkimage make flash.bin
################################################################################

.PHONY: imx-mkimage
imx-mkimage: optee-os imx-atf uboot-imx
	cp $(FIRMWARE_PATH)/ddr/synopsys/lpddr4_pmu_train_* $(MKIMAGE_PATH)/iMX8M/
	cp $(FIRMWARE_PATH)/hdmi/cadence/signed_hdmi_imx8m.bin $(MKIMAGE_PATH)/iMX8M/
	$(MAKE) -C $(MKIMAGE_PATH) SOC=iMX8M flash_hdmi_spl_uboot
	cp $(MKIMAGE_PATH)/iMX8M/flash.bin $(OUT_PATH)

.PHONY: imx-mkimage-clean
imx-mkimage-clean:
	$(MAKE) -C $(MKIMAGE_PATH) clean

################################################################################
# ARM Trusted Firmware
################################################################################

ARMATF_OPTEE_FLAGS ?= CROSS_COMPILE=$(CROSS_COMPILE64) \
				PLAT=imx8mq BUILD_BASE=build-optee \
				SPD=opteed

ARMATF_FLAGS ?= CROSS_COMPILE=$(CROSS_COMPILE64) \
				PLAT=imx8mq

.PHONY: imx-atf
imx-atf: optee-os
	$(MAKE) -j4 -C $(ATF_PATH) $(ARMATF_FLAGS) bl31
	$(MAKE) -j4 -C $(ATF_PATH) $(ARMATF_FLAGS) clean BUILD_BASE=build-optee
	$(MAKE) -j4 -C $(ATF_PATH) $(ARMATF_OPTEE_FLAGS) bl31 
	cp $(ATF_PATH)/build-optee/imx8mq/$(TF_A_BUILD)/bl31.bin $(MKIMAGE_PATH)/iMX8M/

.PHONY: imx-atf-clean
imx-atf-clean:
	$(MAKE) -C $(ATF_PATH) $(ARMATF_FLAGS) distclean

################################################################################
# uboot-imx
################################################################################

UBOOT_DEFCONFIG_FILE=imx8mq_evk_defconfig
UBOOT_FLAGS ?= CROSS_COMPILE=$(CROSS_COMPILE64)

.PHONY: uboot-imx
uboot-imx: uboot-imx-defconfig
	@echo "building uboot....."
	$(MAKE) -j4 -C $(UBOOT_PATH) $(UBOOT_FLAGS)
	cp $(UBOOT_PATH)/u-boot-nodtb.bin $(UBOOT_PATH)/spl/u-boot-spl.bin $(UBOOT_PATH)/arch/arm/dts/fsl-imx8mq-evk.dtb $(MKIMAGE_PATH)/iMX8M/
	cp $(UBOOT_PATH)/tools/mkimage $(MKIMAGE_PATH)/iMX8M/mkimage_uboot

.PHONY: uboot-imx-defconfig
uboot-imx-defconfig:
	$(MAKE) -C $(UBOOT_PATH) $(UBOOT_FLAGS) $(UBOOT_DEFCONFIG_FILE)

.PHONY: uboot-imx-clean
uboot-imx-clean:
	$(MAKE) -C $(UBOOT_PATH) $(UBOOT_FLAGS) clean

################################################################################
# Linux
################################################################################

LINUX_COMMON_FLAGS ?= CROSS_COMPILE=$(CROSS_COMPILE64) \
					  ARCH=$(ARCH)
LINUX_MODULES_FLAGS ?= LOCALVERSION=$(LOCALVERSION) \
                      CROSS_COMPILE=$(CROSS_COMPILE64) \
					  ARCH=$(ARCH) \
					  INSTALL_MOD_PATH=$(ROOTFS_PATH)
LINUX_HEADERS_FLAGS ?= LOCALVERSION=$(LOCALVERSION) \
                      CROSS_COMPILE=$(CROSS_COMPILE64) \
					  ARCH=$(ARCH) \
					  INSTALL_HDR_PATH=$(ROOTFS_PATH)/usr
LINUX_DEFCONFIG_FILES ?= defconfig

.PHONY: linux-imx
linux-imx: linux-common
	cp -f $(LINUX_PATH)/arch/arm64/boot/Image $(OUT_PATH)

.PHONY: linux-rootfs
linux-rootfs: linux-module linux-headers

.PHONY: linux-module
linux-module:
	$(MAKE) -C $(LINUX_PATH) $(LINUX_MODULES_FLAGS) modules_install

.PHONY: linux-headers
linux-headers:
	$(MAKE) -C $(LINUX_PATH) $(LINUX_HEADERS_FLAGS) headers_install

.PHONY: linux-deb
linux-deb: linux-defconfig
	$(MAKE) -j4 -C $(LINUX_PATH) $(LINUX_COMMON_FLAGS) bindeb-pkg

.PHONY: linux-common
linux-common: linux-defconfig
	$(MAKE) -j4 -C $(LINUX_PATH) $(LINUX_COMMON_FLAGS)

.PHONY: linux-defconfig
linux-defconfig:
	$(MAKE) -C $(LINUX_PATH) $(LINUX_COMMON_FLAGS) $(LINUX_DEFCONFIG_FILES)

.PHONY: linux-imx-clean
linux-imx-clean: linux-defconfig-clean
	$(MAKE) -C $(LINUX_PATH) $(LINUX_COMMON_FLAGS) clean

.PHONY: linux-defconfig-clean
linux-defconfig-clean:
	rm -f $(LINUX_PATH)/.config

.PHONY: linux-imx-cleaner
linux-imx-cleaner: 
	$(MAKE) -C $(LINUX_PATH) $(LINUX_COMMON_FLAGS) distclean

################################################################################
# OP-TEE
################################################################################

.PHONY: optee-imx
optee-imx: optee-os optee-client optee-test optee-examples


.PHONY: optee-imx-clean
optee-imx-clean:  optee-test-clean optee-examples-clean optee-client-clean optee-os-clean 

OPTEE_OS_FLAGS ?= PLATFORM=imx \
			PLATFORM_FLAVOR=$(PLATFORM) \
			CROSS_COMPILE=$(CROSS_COMPILE64) \
			CROSS_COMPILE64=${CROSS_COMPILE64} \
			CFG_TEE_CORE_LOG_LEVEL=$(CFG_TEE_CORE_LOG_LEVEL) \
			NOWERROR=1

OPTEE_OS_CLEAN_FLAGS ?= PLATFORM=imx \
					ARCH=arm \
					PLATFORM_FLAVOR=$(PLATFORM) \
					CFG_ARM64_core=y

.PHONY: optee-os
optee-os: optee-os-common
	${CROSS_COMPILE64}objcopy -O binary $(OPTEE_OS_PATH)/out/arm-plat-imx/core/tee.elf $(OPTEE_OS_PATH)/out/arm-plat-imx/core/tee.bin
	cp $(OPTEE_OS_PATH)/out/arm-plat-imx/core/tee.bin $(MKIMAGE_PATH)/iMX8M/

optee-os-common:
	$(MAKE) -C $(OPTEE_OS_PATH) $(OPTEE_OS_FLAGS) all

.PHONY: optee-os-clean
optee-os-clean: 
	$(MAKE) -C $(OPTEE_OS_PATH) $(OPTEE_OS_CLEAN_FLAGS) clean


################################################################################
# optee_client
################################################################################

OPTEE_CLIENT_FLAGS ?= CROSS_COMPILE=$(CROSS_COMPILE64)

PHONY: optee-client
optee-client: 	
	$(MAKE) -C $(OPTEE_CLIENT_PATH) $(OPTEE_CLIENT_FLAGS)

.PHONY: optee-client-clean
optee-client-clean: 
	$(MAKE) -C $(OPTEE_CLIENT_PATH) $(OPTEE_CLIENT_FLAGS) clean

################################################################################
# xtest / optee_test
################################################################################

OPTEE_TEST_FLAGS ?= CROSS_COMPILE_HOST=$(CROSS_COMPILE64)\
	CROSS_COMPILE_TA=$(CROSS_COMPILE64) \
	TA_DEV_KIT_DIR=$(OPTEE_OS_TA_DEV_KIT_DIR) \
	OPTEE_CLIENT_EXPORT=$(OPTEE_CLIENT_EXPORT) \
	COMPILE_NS_USER=$(COMPILE_NS_USER) \
	O=$(OPTEE_TEST_OUT_PATH)

.PHONY: optee-test
optee-test: optee-os optee-client
	$(MAKE) -C $(OPTEE_TEST_PATH) $(OPTEE_TEST_FLAGS)

OPTEE_TEST_CLEAN_FLAGS ?= O=$(OPTEE_TEST_OUT_PATH) \
	TA_DEV_KIT_DIR=$(OPTEE_OS_TA_DEV_KIT_DIR) \

.PHONY: optee-test-clean
optee-test-clean:
	$(MAKE) -C $(OPTEE_TEST_PATH) $(OPTEE_TEST_CLEAN_FLAGS) clean

################################################################################
# sample applications / optee_examples
################################################################################
OPTEE_EXAMPLES_FLAGS ?= HOST_CROSS_COMPILE=$(CROSS_COMPILE64)\
	TA_CROSS_COMPILE=$(CROSS_COMPILE64) \
	TA_DEV_KIT_DIR=$(OPTEE_OS_TA_DEV_KIT_DIR) \
	TEEC_EXPORT=$(OPTEE_CLIENT_EXPORT)

.PHONY: optee-examples
optee-examples: optee-os optee-client
	$(MAKE) -C $(OPTEE_EXAMPLES_PATH) $(OPTEE_EXAMPLES_FLAGS)

OPTEE_EXAMPLES_CLEAN_FLAGS ?= TA_DEV_KIT_DIR=$(OPTEE_OS_TA_DEV_KIT_DIR)

.PHONY: optee-examples-clean
optee-examples-clean:
	$(MAKE) -C $(OPTEE_EXAMPLES_PATH) \
			$(OPTEE_EXAMPLES_CLEAN_FLAGS) clean
