# Designed for processor_sdk_rtos_am335x_4_01_00_06.

# TI RTOS SDK install root
TI_INSTALL_DIR ?= $(HOME)/ti

# TI RTOS SDK tools
gcc_install_dir := $(TI_INSTALL_DIR)/gcc-arm-none-eabi-4_9-2015q3
xdc_install_dir := $(TI_INSTALL_DIR)/xdctools_3_32_01_22_core

# TI RTOS SDK packages
bios_install_dir := $(TI_INSTALL_DIR)/bios_6_46_05_55/packages
edma_install_dir := $(TI_INSTALL_DIR)/edma3_lld_2_12_05_29/packages
pdk_install_dir := $(TI_INSTALL_DIR)/pdk_am335x_1_0_8/packages
uia_install_dir := 
uniflash_install_dir := /home/clarkson/ti-other/uniflash_5.3.1

# Detect GCC version from the toolchain itself.
GCC_VER := $(shell $(gcc_install_dir)/bin/arm-none-eabi-gcc -dumpversion 2>/dev/null || echo "12.3.1")

# Find the correct libgcc path (GCC multilib may use thumb/v7-a+fp/hard instead of fpu/)
GCC_LIBGCC_DIR := $(gcc_install_dir)/lib/gcc/arm-none-eabi/$(GCC_VER)/fpu
ifeq ($(wildcard $(GCC_LIBGCC_DIR)/libgcc.a),)
  GCC_LIBGCC_DIR := $(gcc_install_dir)/lib/gcc/arm-none-eabi/$(GCC_VER)/thumb/v7-a+fp/hard
endif
ifeq ($(wildcard $(GCC_LIBGCC_DIR)/libgcc.a),)
  GCC_LIBGCC_DIR := $(gcc_install_dir)/lib/gcc/arm-none-eabi/$(GCC_VER)
endif

# Find the correct libstdc++ path
GCC_LIBSTDCPP_DIR := $(gcc_install_dir)/arm-none-eabi/lib/fpu
ifeq ($(wildcard $(GCC_LIBSTDCPP_DIR)/libstdc++.a),)
  GCC_LIBSTDCPP_DIR := $(gcc_install_dir)/arm-none-eabi/lib/thumb/v7-a+fp/hard
endif
ifeq ($(wildcard $(GCC_LIBSTDCPP_DIR)/libstdc++.a),)
  GCC_LIBSTDCPP_DIR := $(gcc_install_dir)/arm-none-eabi/lib
endif

# Use the C system libraries shipped with BIOS (not the toolchain).
sysbios_cflags = @$(sysbios_build_dir)/compiler.opt
sysbios_lflags = -nostartfiles -static \
	-Wl,-T,$(sysbios_build_dir)/linker.cmd \
	-L$(bios_install_dir)/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/fpu \
	-L$(GCC_LIBGCC_DIR) -lgcc \
	-L$(GCC_LIBSTDCPP_DIR) -lstdc++

# Set compiler tools
CC := $(gcc_install_dir)/bin/arm-none-eabi-gcc -fdiagnostics-color -fmax-errors=5
CPP := $(gcc_install_dir)/bin/arm-none-eabi-g++ -fdiagnostics-color -fmax-errors=5
OBJCOPY := $(gcc_install_dir)/bin/arm-none-eabi-objcopy
OBJDUMP := $(gcc_install_dir)/bin/arm-none-eabi-objdump
ADDR2LINE := $(gcc_install_dir)/bin/arm-none-eabi-addr2line
LD := $(gcc_install_dir)/bin/arm-none-eabi-gcc -fdiagnostics-color
AR := $(gcc_install_dir)/bin/arm-none-eabi-gcc-ar
SIZE := $(gcc_install_dir)/bin/arm-none-eabi-size
STRIP := $(gcc_install_dir)/bin/arm-none-eabi-strip
READELF := $(gcc_install_dir)/bin/arm-none-eabi-readelf
NM := $(gcc_install_dir)/bin/arm-none-eabi-nm
SWIG := $(HOME)/.local/swig-4.2.1/bin/swig
XS = $(xdc_install_dir)/xs --xdcpath="$(XDCPATH)" xdc.tools.configuro -c $(gcc_install_dir)
PYTHON := python3
ZIP := zip
# tiimage creates TI boot image headers. We use a Python replacement.
TIIMAGE := $(PYTHON) $(abspath scripts/tiimage.py)