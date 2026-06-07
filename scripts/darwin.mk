# Build Tools for OSX
#
# Use the system clang (macOS default) instead of Homebrew gcc.
# Note: the stock firmware uses gcc-10; clang may produce different codegen.
GCC_VERSION = 10

# Detect available compiler
GCC_EXISTS := $(shell which gcc-$(GCC_VERSION) 2>/dev/null || echo 0)
CLANG_EXISTS := $(shell which clang++ 2>/dev/null || which c++ 2>/dev/null || echo 0)

ifeq ($(CLANG_EXISTS),0)
  # Fallback: use Homebrew gcc
  CC := gcc-$(GCC_VERSION) -fdiagnostics-color -fmax-errors=5
  CPP := g++-$(GCC_VERSION) -fdiagnostics-color -fmax-errors=5
  AR := gcc-ar-$(GCC_VERSION)
else
  # Use system clang
  CC := clang -fdiagnostics-color -fmax-errors=5
  CPP := clang++ -fdiagnostics-color -fmax-errors=5
  AR := ar
endif

OBJCOPY := objcopy
OBJDUMP := objdump
ADDR2LINE := addr2line
LD := ld
SIZE := size
STRIP := strip
READELF := readelf
NM := nm
SWIG := swig
PYTHON := python3
ZIP := zip