# HiSpark_AiFly 内核说明

## 目录结构

```
├── kernel                     #内核相关文件
│	├── batch_sign_ko.sh       #内核模块批量签名脚本
│	├── BUILD.gn               #编译框架GN文件
│	├── kernel.mk              #用于配置内核编译交叉工具链、源码环境、defconfig等信息
│	├── kernel_module_build.sh #内核编译入口shell脚本文件
│	└── LICENSE                #许可证文件
├── linux                      #Linux相关配置
│   ├── config.gni             #用于描述产品样例所使用的单板、内核、工具链、编译器等信息
│   └── LICENSE                #许可证文件
```

## 内核编译流程

### 1. 源码拷贝

OpenHarmony工程的编译构建流程中会拷贝kernel/linux/linux-**.**的代码至`$(OUT_DIR)/kernel/${KERNEL_VERSION}`后进行打补丁动作：

```bash
$(hide) rm -rf $(KERNEL_SRC_TMP_PATH);mkdir -p $(KERNEL_SRC_TMP_PATH);cp -arfL $(KERNEL_SRC_PATH)/* $(KERNEL_SRC_TMP_PATH)/
```

### 2. 合入HDF补丁

按照`drivers/hdf_core/adapter/khdf/linux/patch_hdf.sh`中HDF补丁合入方法，应用`kernel/linux/patches/$(KERNEL_VERSION)/common_patch/hdf.patch`，合入不同内核版本对应的HDF内核补丁：

```bash
$(hide) $(OHOS_BUILD_HOME)/drivers/hdf_core/adapter/khdf/linux/patch_hdf.sh $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(KERNEL_PATCH_PATH) $(DEVICE_NAME)
```

### 3. 编译linux-6.6内核的特殊处理

若编译linux-6.6内核，则配套要修改`bounds_checking_function/include/securec.h`的stdarg.h：

```bash
ifeq ($(KERNEL_VERSION), linux-6.6)
    sed -i 's/<stdarg.h>/<linux\/stdarg.h>/' $(KERNEL_SRC_TMP_PATH)/bounds_checking_function/include/securec.h
endif
```

### 4. 应用芯片特性补丁

应用芯片特性的内核补丁和适配OpenHarmony内核的补丁，直接执行：

```bash
$(hide) chmod 755 $(DEVICE_PATCH_DIR)/patch_$(DEVICE_NAME).sh
$(hide) cd $(KERNEL_SRC_TMP_PATH);$(DEVICE_PATCH_DIR)/patch_$(DEVICE_NAME).sh $(DEVICE_PATCH_DIR)
```

### 5. 创建软链接

创建软链接到kernel/linux/common_modules目录下的软件源码：

```bash
$(hide) $(UNIFIED_COLLECTION_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
```

### 6. 合并defconfig配置文件

将如下配置文件合并生成的defconfig拷贝至`${KERNEL_SRC_TMP_PATH}/arch/$(KERNEL_ARCH)/configs/$(DEFCONFIG_FILE)`使用：

| 配置文件 | 说明 |
|---------|------|
| small_defconfig | 小型系统常用配置文件 |
| arm64_defconfig | 芯片特性的内核配置文件 |
| support_defconfig | 适配OpenHarmony内核的专用配置文件 |
| base_defconfig | OpenHarmony特性依赖的内核必选模块以及安全红线特性等必须开启的配置项 |

合并命令：

```bash
$(hide) cp -rf $(KERNEL_CONFIG_PATH)/. $(KERNEL_SRC_TMP_PATH)/
$(hide) bash $(KERNEL_SRC_TMP_PATH)/scripts/kconfig/merge_config.sh -O $(KERNEL_SRC_TMP_PATH)/arch/$(KERNEL_ARCH)/configs/ \
    -m $(KERNEL_SRC_TMP_PATH)/type/small_defconfig \
    $(KERNEL_SRC_TMP_PATH)/$(DEVICE_NAME)/arch/arm64_defconfig \
    $(KERNEL_SRC_TMP_PATH)/$(DEVICE_NAME)/arch/support_defconfig \
    $(KERNEL_SRC_TMP_PATH)/base_defconfig

$(hide) mv ${KERNEL_SRC_TMP_PATH}/arch/$(KERNEL_ARCH)/configs/.config ${KERNEL_SRC_TMP_PATH}/arch/$(KERNEL_ARCH)/configs/$(DEFCONFIG_FILE)
$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) distclean
$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) $(DEFCONFIG_FILE)
```

## 构建说明

使用工程的全量编译命令，编译生成uImage内核镜像：

```bash
./build.sh --product-name=ipcamera_hispark_aifly_linux --ccache --no-prebuilt-sdk --build-target build_kernel --gn-args linux_kernel_version="linux-6.6"
```

### 参数说明

| 参数 | 说明 |
|------|------|
| --product-name ipcamera_hispark_aifly_linux | 编译ipcamera_hispark_aifly_linux镜像 |
| --build-target build_kernel | 编译hispark_aifly的uImage内核镜像 |
| --gn-args linux_kernel_version="linux-6.6" | 编译指定内核版本，若不配置参数，则默认以config.json配置为准 |
| --no-prebuilt-sdk | 跳过SDK子系统的编译 |

## 支持的芯片

当前支持Hi3403V100芯片。

## 内核版本

- Linux 6.6.y

## 相关文件路径

- 内核源码：`kernel/linux/linux-6.6.y`
- HDF补丁脚本：`drivers/hdf_core/adapter/khdf/linux/patch_hdf.sh`
- 芯片补丁目录：`kernel/linux/patches/linux-6.6/$(DEVICE_PATCH_DIR)`
- 内核配置目录：`kernel/linux/configs`
