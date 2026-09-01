# Copyright (c) 2026 HiSilicon (Shanghai) Technologies Co., Ltd.
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# ohos makefile to build kernel

PRODUCT_NAME=$(TARGET_PRODUCT)
OHOS_BUILD_HOME := $(realpath $(shell pwd)/../../../../../)
KERNEL_SRC_TMP_PATH := $(OUT_DIR)/kernel/${KERNEL_VERSION}
KERNEL_OBJ_TMP_PATH := $(KERNEL_SRC_TMP_PATH)
export KBUILD_OUTPUT=$(KERNEL_SRC_TMP_PATH)

KERNEL_SRC_PATH := $(OHOS_BUILD_HOME)/kernel/linux/${KERNEL_VERSION}
KERNEL_PATCH_PATH := $(OHOS_BUILD_HOME)/kernel/linux/patches/${KERNEL_VERSION}
KERNEL_CONFIG_PATH := $(OHOS_BUILD_HOME)/kernel/linux/config/${KERNEL_VERSION}
PREBUILTS_GCC_DIR := $(OHOS_BUILD_HOME)/prebuilts/gcc
CLANG_HOST_TOOLCHAIN := $(OHOS_BUILD_HOME)/prebuilts/clang/ohos/linux-x86_64/llvm/bin
KERNEL_HOSTCC := $(CLANG_HOST_TOOLCHAIN)/clang
KERNEL_PREBUILT_MAKE := make
CLANG_CC := $(CLANG_HOST_TOOLCHAIN)/clang

KERNEL_TMP_DTS_PATH := $(KERNEL_SRC_TMP_PATH)/arch/$(KERNEL_ARCH)/boot/dts/vendor

KERNEL_CROSS_COMPILE :=
KERNEL_TARGET_TOOLCHAIN := $(PREBUILTS_GCC_DIR)/linux-x86/aarch64/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin
KERNEL_TARGET_TOOLCHAIN_PREFIX := $(KERNEL_TARGET_TOOLCHAIN)/aarch64-linux-gnu-
KERNEL_CROSS_COMPILE += CC="$(CLANG_CC)"
KERNEL_CROSS_COMPILE += CROSS_COMPILE="$(KERNEL_TARGET_TOOLCHAIN_PREFIX)"

KERNEL_MAKE := \
    $(KERNEL_PREBUILT_MAKE)

ifneq ($(findstring $(BUILD_TYPE), small standard),)
DEVICE_PATCH_DIR := $(OHOS_BUILD_HOME)/kernel/linux/patches/${KERNEL_VERSION}/$(DEVICE_NAME)_patch
SMALL_PATCH_FILE := $(DEVICE_PATCH_DIR)/$(DEVICE_NAME)_$(BUILD_TYPE).patch
KERNEL_IMAGE_FILE := $(KERNEL_SRC_TMP_PATH)/arch/$(KERNEL_ARCH)/boot/uImage
DEFCONFIG_FILE := $(DEVICE_NAME)_$(BUILD_TYPE)_defconfig

UNIFIED_COLLECTION_PATCH_FILE := ${OHOS_BUILD_HOME}/kernel/linux/common_modules/ucollection/apply_ucollection.sh
CODE_SIGN_PATCH_FILE := ${OHOS_BUILD_HOME}/kernel/linux/common_modules/code_sign/apply_code_sign.sh
HIDEADDR_PATCH_FILE=${OHOS_BUILD_HOME}/kernel/linux/common_modules/memory_security/apply_hideaddr.sh
NEWIP_PATCH_FILE=${OHOS_BUILD_HOME}/kernel/linux/common_modules/newip/apply_newip.sh
TZDRIVER_PATCH_FILE=${OHOS_BUILD_HOME}/kernel/linux/common_modules/tzdriver/apply_tzdriver.sh
XPM_PATCH_FILE=${OHOS_BUILD_HOME}/kernel/linux/common_modules/xpm/apply_xpm.sh
CED_PATCH_FILE=${OHOS_BUILD_HOME}/kernel/linux/common_modules/container_escape_detection/apply_ced.sh
QOS_AUTH_PATCH_FILE=${OHOS_BUILD_HOME}/kernel/linux/common_modules/qos_auth/apply_qos_auth.sh
DEC_PATCH_FILE=${OHOS_BUILD_HOME}/kernel/linux/common_modules/dec/apply_dec.sh

$(KERNEL_IMAGE_FILE):
	$(hide) echo "build kernel..."
	$(hide) rm -rf $(KERNEL_SRC_TMP_PATH);mkdir -p $(KERNEL_SRC_TMP_PATH);cp -arfL $(KERNEL_SRC_PATH)/* $(KERNEL_SRC_TMP_PATH)/
	$(hide) echo "apply hdf patch..."
	$(hide) $(OHOS_BUILD_HOME)/drivers/hdf_core/adapter/khdf/linux/patch_hdf.sh $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(KERNEL_PATCH_PATH) $(DEVICE_NAME)

#update linux-6.6 kernel stdarg.h path to linux/stdarg.h
ifeq ($(KERNEL_VERSION), linux-6.6)
	$(hide) sed -i 's/<stdarg.h>/<linux\/stdarg.h>/' $(KERNEL_SRC_TMP_PATH)/bounds_checking_function/include/securec.h
endif

	$(hide) echo "apply kernel patch..."
	$(hide) chmod 755 $(DEVICE_PATCH_DIR)/patch_$(DEVICE_NAME).sh
	$(hide) cd $(KERNEL_SRC_TMP_PATH);$(DEVICE_PATCH_DIR)/patch_$(DEVICE_NAME).sh $(DEVICE_PATCH_DIR)

# Link SOC HDF driver directory into kernel tree for path dependency compilation
	$(hide) echo "link hi3403 hdf driver to kernel tree..."
	$(hide) mkdir -p $(KERNEL_SRC_TMP_PATH)/drivers/hdf
	$(hide) ln -sfn $(OHOS_BUILD_HOME)/device/soc/hisilicon/hi3403v100/hdf_driver \
	    $(KERNEL_SRC_TMP_PATH)/drivers/hdf/soc_driver

ifeq ($(UNIFIED_COLLECTION_PATCH_FILE), $(wildcard $(UNIFIED_COLLECTION_PATCH_FILE)))
	$(hide) $(UNIFIED_COLLECTION_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
endif
ifeq ($(CODE_SIGN_PATCH_FILE), $(wildcard $(CODE_SIGN_PATCH_FILE)))
	$(hide) $(CODE_SIGN_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
endif
ifeq ($(HIDEADDR_PATCH_FILE), $(wildcard $(HIDEADDR_PATCH_FILE)))
	$(hide) $(HIDEADDR_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
endif
ifeq ($(NEWIP_PATCH_FILE), $(wildcard $(NEWIP_PATCH_FILE)))
	$(hide) bash $(NEWIP_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
endif
ifeq ($(TZDRIVER_PATCH_FILE), $(wildcard $(TZDRIVER_PATCH_FILE)))
	$(hide) $(TZDRIVER_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
endif
ifeq ($(XPM_PATCH_FILE), $(wildcard $(XPM_PATCH_FILE)))
	$(hide) $(XPM_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
endif
ifeq ($(CED_PATCH_FILE), $(wildcard $(CED_PATCH_FILE)))
	$(hide) $(CED_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
endif
ifeq ($(QOS_AUTH_PATCH_FILE), $(wildcard $(QOS_AUTH_PATCH_FILE)))
	$(hide) bash $(QOS_AUTH_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
endif
ifeq ($(DEC_PATCH_FILE), $(wildcard $(DEC_PATCH_FILE)))
	$(hide) bash $(DEC_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
endif

#defconfig
	$(hide) cp -rf $(KERNEL_CONFIG_PATH)/. $(KERNEL_SRC_TMP_PATH)/
	$(hide) bash $(KERNEL_SRC_TMP_PATH)/scripts/kconfig/merge_config.sh -O $(KERNEL_SRC_TMP_PATH)/arch/$(KERNEL_ARCH)/configs/ \
     -m $(KERNEL_SRC_TMP_PATH)/type/small_defconfig $(KERNEL_SRC_TMP_PATH)/$(DEVICE_NAME)/arch/arm64_defconfig \
     $(KERNEL_SRC_TMP_PATH)/$(DEVICE_NAME)/arch/support_defconfig $(KERNEL_SRC_TMP_PATH)/base_defconfig

	$(hide) cp ${KERNEL_SRC_TMP_PATH}/arch/$(KERNEL_ARCH)/configs/.config ${KERNEL_SRC_TMP_PATH}/arch/$(KERNEL_ARCH)/configs/$(DEFCONFIG_FILE)
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) distclean
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) $(DEFCONFIG_FILE)
ifeq ($(KERNEL_VERSION), linux-5.10)
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) modules_prepare
endif
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) -j64 uImage
endif

	echo "$(KERNEL_OBJ_TMP_PATH)/arch/$(KERNEL_ARCH)/boot/uImage"
	$(hide) cp -rf $(KERNEL_OBJ_TMP_PATH)/arch/$(KERNEL_ARCH)/boot/uImage $(OUT_DIR)
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) scripts/module.lds
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) -j64 modules
	$(hide) mkdir -p $(OUT_DIR)/ko
	$(hide) OHOS_OUTDIR=$(OUT_DIR) KERNEL_VERSION=$(KERNEL_VERSION) bash $(OHOS_BUILD_HOME)/device/board/hisilicon/$(DEVICE_NAME)/kernel/batch_sign_ko.sh \
		-o $(OUT_DIR)/ko \
		$(KERNEL_OBJ_TMP_PATH)/drivers/gpu/drm/drm_kms_helper.ko \
		$(KERNEL_OBJ_TMP_PATH)/drivers/gpu/drm/drm_dma_helper.ko \
		$(KERNEL_OBJ_TMP_PATH)/drivers/gpu/drm/display/drm_display_helper.ko \
		$(KERNEL_OBJ_TMP_PATH)/drivers/gpu/drm/hisilicon/smart_vision/smart_drm.ko \
		$(OHOS_BUILD_HOME)/device/board/hisilicon/$(DEVICE_NAME)/kernel/sig_ko/ble_soc.ko \
		$(OHOS_BUILD_HOME)/device/board/hisilicon/$(DEVICE_NAME)/kernel/sig_ko/plat_soc.ko \
		$(OHOS_BUILD_HOME)/device/board/hisilicon/$(DEVICE_NAME)/kernel/sig_ko/sle_soc.ko \
		$(OHOS_BUILD_HOME)/device/board/hisilicon/$(DEVICE_NAME)/kernel/sig_ko/wifi_soc.ko 

.PHONY: build-kernel
build-kernel: $(KERNEL_IMAGE_FILE)
