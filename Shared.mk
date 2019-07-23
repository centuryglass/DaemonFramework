# Provides content shared by MakeDaemon and MakeDaemonParent.

######################## Initialize build variables: ##########################
# Set Debug or Release mode:
DF_CONFIG ?= Release

# enable or disable verbose output:
DF_VERBOSE ?= 0
V_AT := $(shell if [ $(DF_VERBOSE) != 1 ]; then echo '@'; fi)

# Select specific build architectures:
DF_TARGET_ARCH ?= -march=native

############################### Set build flags: ##############################
#### Config-specific flags: ####
ifeq ($(DF_CONFIG),Debug)
    DF_OPTIMIZATION ?= 0
    DF_GDB_SUPPORT ?= 1
    # Debug-specific preprocessor definitions:
    DF_CONFIG_FLAGS = -DDF_DEBUG=1
endif

ifeq ($(DF_CONFIG),Release)
    DF_OPTIMIZATION ?= 1
    DF_GDB_SUPPORT ?= 0
endif

# Set optimization level flags:
ifeq ($(DF_OPTIMIZATION), 1)
    DF_CONFIG_CFLAGS := $(DF_CONFIG_CFLAGS) -O3 -flto
    DF_CONFIG_LDFLAGS := $(DF_CONFIG_LDFLAGS) -flto
else
    DF_CONFIG_CFLAGS := $(DF_CONFIG_CFLAGS) -O0
endif

# Set debugging flags:
ifeq ($(DF_GDB_SUPPORT), 1)
    DF_CONFIG_CFLAGS := $(DF_CONFIG_CFLAGS) -g -ggdb
else
    DF_CONFIG_LDFLAGS := $(DF_CONFIG_LDFLAGS) -fvisibility=hidden
endif

#### C compilation flags: ####
DF_CFLAGS := $(DF_TARGET_ARCH) \
             $(DF_CONFIG_CFLAGS) \
             $(DF_CFLAGS)

#### C++ compilation flags: ####
DF_CXXFLAGS := -std=gnu++14 $(DF_CXXFLAGS)

#### C Preprocessor flags: ####

# Given a nonempty, nonzero makefile variable, print a corresponding C
# preprocessor definition.
addDef = $(shell if [ ! -z $($(1)) ] && [ $($(1)) != 0 ]; then \
                     echo "-D$(1)=$($(1))"; fi)

# Given a nonempty, nonzero makefile variable, print a corresponding C
# preprocessor definition, quoted to ensure it is defined as a string literal.
addStringDef = $(shell if [ ! -z $($(1)) ] && [ $($(1)) != 0 ]; then \
                     echo '-D$(1)=\"$($(1))\"'; fi)

# Given a list of root directories, recursively print flags to include those 
# directories and their subdirectories.
recursiveInclude = $(shell find $(1) -type d -printf ' "-I%p"')

DF_DEFINE_FLAGS := $(call addStringDef,DF_DAEMON_PATH) \
                   $(call addStringDef,DF_INPUT_PIPE_PATH) \
                   $(call addStringDef,DF_OUTPUT_PIPE_PATH) \
                   $(call addDef,DF_VERBOSE) \

DF_DIR_FLAGS := $(call recursiveInclude,$(DF_ROOT_DIR)/Include/Implementation)

# Disable dependency generation if multiple architectures are set
DF_DEPFLAGS := $(if $(word 2, $(DF_TARGET_ARCH)), , -MMD)

DF_CPPFLAGS := -pthread \
            $(DF_DEPFLAGS) \
            $(DF_CONFIG_FLAGS) \
            $(DF_CPPFLAGS)

#### Linker flags: ####
DF_LDFLAGS := -lpthread \
              $(DF_TARGET_ARCH) \
              $(DF_CONFIG_LDFLAGS) \
              $(DF_LDFLAGS)

######################### Load shared source lists: ############################
include $(DF_ROOT_DIR)/Source/Shared/shared.mk

############################# Build Targets: ##################################
.PHONY: print-%

## Print the value of any build variable: ##
print-%:
	@echo $($*)
