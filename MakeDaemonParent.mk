define HELPTEXT
### DaemonFramework Daemon Parent Makefile ###
#  This makefile provides the daemonParent target, used to build all files
# required by applications that manage DaemonFramework daemons. Include this
# file in your program's makefile, and add 'daemonParent' as a dependency of
# your program's build target.
#
### Build Control:
#   DF_OBJDIR:  The directory where compiled .o files will be created.
#
#   DF_TARGET_ARCH: (default: -march=native)
#      If defined, the daemon and parent will be compiled using this
#      architecture flag.
#
#   DF_OPTIMIZATION:
#      If defined as 1, build optimization will be enabled. If defined as 0, 
#      build optimization will be disabled. This overrides the usual behavior
#      selected by DF_CONFIG.
#
#   DF_GDB_SUPPORT:
#      If defined as 1, builds will include GDB debug flags. If defined as 0,
#      the daemon and/or parent will be built without debugging enabled.
#      This overrides the usual behavior selected by DF_CONFIG.
#
#   DF_CONFIG: (default: 'Release')
#      Sets whether the daemon or parent compiles in Debug mode, with
#      optimization disabled and debugging enabled, or in Release mode, with
#      optimization enabled and debugging disabled.
#                
#      In Debug mode, the DF_DEBUG preprocessor macro will be set, and the
#      DF_DBG macro function will print to stdout.
#
#   DF_VERBOSE: (default: 0)
#      Sets whether verbose output is enabled while building and during
#      execution.  If enabled, DF_VERBOSE will be defined in the C preprocessor,
#      and the DF_DBG_V macro function will print to stdout.
#
## Daemon control options:
#   DF_INPUT_PIPE_PATH:
#      If defined, the parent application will send messages to its daemon using
#      a named pipe at this path.
#
#   DF_OUTPUT_PIPE_PATH:
#      If defined, the parent application will listen for messages from its 
#      daemon using a named pipe at this path.
#
#   DF_DAEMON_PATH:
#      The path where the daemon process executable will be found after
#      installation, used by the parent application to launch the daemon.
endef
export HELPTEXT

######################## Initialize build variables: ##########################
# Set Debug or Release mode:
DF_CONFIG ?= Release

# enable or disable verbose output:
DF_VERBOSE ?= 0
V_AT = $(shell if [ $DF_VERBOSE == 1 ]; then echo '@'; fi)

# Save makefile directory:
DF_ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

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
addDef = $(shell if [ ! -z $($(1)) ] && [ $($(1)) != 0 ]; \
        then echo -D$(1)=$($(1)); fi)

# C preprocessor definitions:
DF_DEFINE_FLAGS := $(call addDef,DF_DAEMON_PATH) \
                   $(call addDef,DF_INPUT_PIPE_PATH) \
                   $(call addDef,DF_OUTPUT_PIPE_PATH)

# Include directories:
DF_RECURSIVE_INCLUDE_DIRS := $(DF_ROOT_DIR)/Include/Parent \
                             $(DF_ROOT_DIR)/Include/Implementation
DF_DIR_FLAGS := $(shell find $(DF_RECURSIVE_INCLUDE_DIRS) -type d \
                     -printf " -I'%p'")

# Disable dependency generation if multiple architectures are set
DF_DEPFLAGS := $(if $(word 2, $(DF_TARGET_ARCH)), , -MMD)

DF_CPPFLAGS := -pthread \
            $(DF_DEPFLAGS) \
            $(DF_CONFIG_FLAGS) \
            $(DF_DEFINE_FLAGS) \
            $(DF_DIR_FLAGS) \
            $(DF_CPPFLAGS)

#### Linker flags: ####
DF_LDFLAGS := -lpthread \
              $(DF_TARGET_ARCH) \
              $(DF_CONFIG_LDFLAGS) \
              $(DF_LDFLAGS)


######################## Load module source lists: ############################
include $(DF_ROOT_DIR)/Source/Parent/parent.mk
include $(DF_ROOT_DIR)/Source/Shared/shared.mk
DF_OBJECTS_PARENT := $(DF_OBJECTS_SHARED) $(DF_OBJECTS_PARENT)


############################# Build Targets: ##################################

.PHONY: daemonParent check-defs help print-%

## Main build target: ##
# Checks definitions, then compiles daemon parent support classes.
daemonParent : check-defs $(DF_OBJECTS_PARENT)
	@echo Compiled daemon parent object files at "$(DF_OBJDIR)"

## Ensure required variables are defined: ##
check-defs:
	@if [ -z "$(DF_OBJDIR)" ]; then \
        echo >&2 "Build failed, DF_OBJDIR not defined."; exit 1; \
    elif [ -z "$(DF_DAEMON_PATH)" ]; then \
        echo >&2 "Build failed, DF_DAEMON_PATH not defined."; exit 1; \
    fi

## Compile all files needed to build Daemon applications: ##
$(DF_OBJECTS_PARENT) :
	-$(V_AT)mkdir -p $(DF_OBJDIR)
	@echo "      Compiling: $(<F)"
	$(V_AT)$(CXX) $(DF_CFLAGS) $(DF_CXXFLAGS) $(DF_CPPFLAGS) -o "$@" -c "$<"

## Print the guide to this makefile: ##
help:
	$(shell echo "$$HELPTEXT")

## Print the value of any build variable: ##
print-%:
	@echo $* = $($*)

## Enable dependency generation: ##
-include $(DF_OBJECTS_PARENT:%.o=%.d)
