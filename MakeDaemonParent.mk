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
# Save makefile directory:
DF_ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# Import targets and variables shared with the daemon parent makefile:
include $(DF_ROOT_DIR)/Shared.mk

############################### Set build flags: ##############################

# Include directories:
DF_DIR_FLAGS := $(DF_DIR_FLAGS) \
                $(call recursiveInclude,$(DF_ROOT_DIR)/Include/Parent)

DF_CPPFLAGS := $(DF_CPPFLAGS) \
            $(DF_DEFINE_FLAGS) \
            $(DF_DIR_FLAGS) 

######################## Load module source lists: ############################
include $(DF_ROOT_DIR)/Source/Parent/parent.mk
DF_OBJECTS_PARENT := $(DF_OBJECTS_SHARED) $(DF_OBJECTS_PARENT)


############################# Build Targets: ##################################

.PHONY: daemonParent check-defs help

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

## Enable dependency generation: ##
-include $(DF_OBJECTS_PARENT:%.o=%.d)
