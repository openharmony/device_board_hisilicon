#
# Copyright (c) 2022 Huawei Device Co., Ltd.
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
#

KHDF_DISPLAY_KHDF_ROOT_DIR = drivers/hdf/khdf
KHDF_FRAMEWORK_ROOT_DIR = drivers/hdf/framework

obj-$(CONFIG_DRIVERS_HDF_DUAL_LCD_ICN9700_ST7701SN) += \
        mipi_icn9700_st7701sn.o

ccflags-$(CONFIG_DRIVERS_HDF_DUAL_LCD_ICN9700_ST7701SN) += \
        -I$(srctree)/$(KHDF_DISPLAY_KHDF_ROOT_DIR)/osal/include \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/include \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/include/core \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/include/utils \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/include/osal \
        -I$(srctree)/drivers/hdf/inner_api/osal/shared \
        -I$(srctree)/drivers/hdf/inner_api/utils \
        -I$(srctree)/drivers/hdf/inner_api/core \
        -I$(srctree)/drivers/hdf/inner_api/host/shared \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/include/platform \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/include/config \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/include/display \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/ability/sbuf/include \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/core/common/include/host \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/core/host/include \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/model/display/driver \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/model/display/driver/backlight \
        -I$(srctree)/$(KHDF_FRAMEWORK_ROOT_DIR)/model/display/driver/adapter_soc \
        -I$(srctree)/bounds_checking_function/include