
AR := llvm-ar
CC := clang
CXX := clang++

# Usage: define -DWASM when building to compile with escripten.
ifdef WASM
	CC := emcc
	CXX := em++
endif

#-------------------------------------------------------------------------------
# Detect OS
#-------------------------------------------------------------------------------

OSTYPE :=
ifeq ($(OS),Windows_NT)
	OSTYPE = WIN32
else
	# Returns e.g. 'Linux' or 'Darwin'
	OSTYPE = $(shell uname -s)
endif

ifeq ($(OSTYPE),Darwin)
	# It seems homebrew/LLVM can be in multiple places based on OS version.
	export PATH := /opt/homebrew/opt/llvm/bin:$(PATH)
	export PATH := /usr/local/opt/llvm/bin:$(PATH)
endif

# Print the OS type if asked.
.PHONY: os
os:
	@echo OSTYPE is $(OSTYPE)

#-------------------------------------------------------------------------------
# Track all targets for 'clean'
#-------------------------------------------------------------------------------

ALL_TARGETS :=

#-------------------------------------------------------------------------------
# Build Folder
#-------------------------------------------------------------------------------

BUILD_DIR := build
ALL_TARGETS += $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

#-------------------------------------------------------------------------------
# Build Flags
#-------------------------------------------------------------------------------

COMMON_CFLAGS := -std=c17 -Wall -Wextra -Werror \
	-Wno-bitwise-instead-of-logical \
	-Wno-missing-field-initializers \
	-Wno-unknown-warning-option

# Different platforms have different executable suffixes and dependencies
PLATFORM_CFLAGS :=
PLATFORM_DEPS :=

ifeq ($(OSTYPE),WIN32)
	TARGET_SUFFIX:=.exe
else ifdef WASM
	TARGET_SUFFIX:=.html
	PLATFORM_DEPS += emscripten_console.html
	PLATFORM_CFLAGS += -DWASM -sASYNCIFY --shell-file emscripten_console.html -s ALLOW_MEMORY_GROWTH=1 -Wno-limited-postlink-optimizations
else
	PLATFORM_CFLAGS += -D_POSIX_C_SOURCE=200809L
endif

ifdef OPT
	# Usage: define -DOPT for optimized builds
	CFLAGS := -O3
else ifdef OPTASAN
	# Usage: define -DOPT for optimized builds with asan
	CFLAGS := -O3 -fsanitize=address
else ifdef WASM
	# ASAN seems to crash wasm after program execution is complete?
	CFLAGS := -O1 -g
else
	# Default builds with debug flags.
	CFLAGS := -O1 -g -fsanitize=address
endif

CFLAGS += $(COMMON_CFLAGS) $(PLATFORM_CFLAGS)

#-------------------------------------------------------------------------------
# Library Dependencies
#-------------------------------------------------------------------------------

AC_ALLOC_DEPS := ac_alloc.h
AC_STR_DEPS := ac_str.h ac_alloc.h
AC_TEST_DEPS := ac_test.h ac_str.h ac_alloc.h

ALL_DEPS := ac_test.h ac_str.h ac_alloc.h

#-------------------------------------------------------------------------------
# TEST ac_test
#-------------------------------------------------------------------------------

TARGET := ac_test_test
TARGET_DEPS := $(AC_TEST_DEPS) $(PLATFORM_DEPS) $(TARGET).c $(TARGET).h
ALL_TARGETS += $(BUILD_DIR)/$(TARGET)

$(TARGET): $(TARGET_DEPS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TARGET).c -o $(BUILD_DIR)/$(TARGET)$(TARGET_SUFFIX)

#-------------------------------------------------------------------------------
# TEST ALL
#-------------------------------------------------------------------------------

TARGET := test_all
TARGET_DEPS := $(ALL_DEPS) $(PLATFORM_DEPS) $(TARGET).c $(TARGET).h
ALL_TARGETS += $(BUILD_DIR)/$(TARGET)

$(TARGET): $(TARGET_DEPS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TARGET).c -o $(BUILD_DIR)/$(TARGET)$(TARGET_SUFFIX)

#-------------------------------------------------------------------------------
# Clean
#-------------------------------------------------------------------------------

.PHONY: clean
clean:
	rm -rf $(ALL_TARGETS)
