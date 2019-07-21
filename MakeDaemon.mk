define HELPTEXT
### DaemonFramework Daemon Makefile ###
#  This makefile provides the daemon target, used to build all files required
# for daemon implementations. Include this file in your daemon's makefile, and
# add 'daemon' as a dependency of your daemon's build target.
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
## Communication options:
#   DF_INPUT_PIPE_PATH:
#      If defined, the daemon will listen for messages from its parent 
#      application using a named pipe at this path.
#
#   DF_OUTPUT_PIPE_PATH:
#      If defined, the daemon will use a named pipe at this path to send data to
#      its parent application.
#
## Security Options:
#   DF_DAEMON_PATH:
#      The path where the daemon process executable will be found after
#      installation. If provided, the daemon will exit if it finds that its
#      process is running any other executable path.
#
#    DF_REQUIRED_PARENT_PATH:
#      If defined, the daemon will find its parent process, check the path of
#      the executable that the parent process is running, and automatically exit
#      if the parent is running from any executable path other than
#      DF_REQUIRED_PARENT_PATH.
#
#    DF_LOCK_FILE_PATH:
#      If defined, the daemon will create and use a lock file at this path to
#      ensure that only one daemon instance may run at one time.
#
#    DF_VERIFY_PATH_SECURITY: (default: 1)
#      If set to 1, the daemon will check if it is running from a secured
#      directory that only the root user/group may modify, and exit if the
#      directory is insecure.
#
#    DF_VERIFY_PARENT_PATH_SECURITY: (default: 1)
#      If set to 1, the daemon will check if its parent process is running from
#      a secured directory that only the root user/group may modify, and exit if
#      the directory is insecure.
#
#    DF_REQUIRE_RUNNING_PARENT: (default: 1)
#      If set to 1, the daemon will periodically check if its parent process is
#      still running, and exit if its parent has exited.
#
#    DF_TIMEOUT:  
#      If defined, the daemon will automatically exit after running for TIMEOUT
#      seconds.
endef
export HELPTEXT


######################## Initialize build variables: ##########################
# Save makefile directory:
DF_ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# Import targets and variables shared with the daemon parent makefile:
include $(DF_ROOT_DIR)/Shared.mk

# Daemon security options:
DF_VERIFY_PATH ?= 1
DF_VERIFY_PATH_SECURITY ?= 1
DF_VERIFY_PARENT_PATH_SECURITY ?= 1
DF_REQUIRE_RUNNING_PARENT ?= 1


############################### Set build flags: ##############################

# C preprocessor definitions:
DF_DEFINE_FLAGS := $(DF_DEFINE_FLAGS) \
                   $(call addDef,DF_LOCK_FILE_PATH) \
                   $(call addDef,DF_VERIFY_PATH) \
                   $(call addDef,DF_VERIFY_PATH_SECURITY) \
                   $(call addDef,DF_VERIFY_PARENT_PATH_SECURITY) \
                   $(call addDef,DF_REQUIRE_RUNNING_PARENT) \
                   $(call addDef,DF_REQUIRED_PARENT_PATH) \
                   $(call addDef,DF_TIMEOUT) \
                   -DDF_IS_DAEMON=1

# Include directories:
DF_DIR_FLAGS := $(DF_DIR_FLAGS) \
                $(call recursiveInclude,$(DF_ROOT_DIR)/Include/Daemon)

DF_CPPFLAGS := $(DF_CPPFLAGS) \
            $(DF_DEFINE_FLAGS) \
            $(DF_DIR_FLAGS) 

######################## Load module source lists: ############################
include $(DF_ROOT_DIR)/Source/Daemon/daemon.mk
DF_OBJECTS_DAEMON := $(DF_OBJECTS_SHARED) $(DF_OBJECTS_DAEMON)


############################# Build Targets: ##################################

.PHONY: daemonFramework check-defs help

## Main build target: ##
# Checks definitions, then compiles daemon support classes.
daemonFramework : check-defs $(DF_OBJECTS_DAEMON)
	@echo Compiled daemon object files at "$(DF_OBJDIR)"

## Ensure required variables are defined: ##
check-defs:
	@if [ -z "$(DF_OBJDIR)" ]; then \
        echo >&2 "Build failed, DF_OBJDIR not defined."; exit 1; \
    fi

## Compile all files needed to build Daemon applications: ##
$(DF_OBJECTS_DAEMON) :
	-$(V_AT)mkdir -p $(DF_OBJDIR)
	@echo "      Compiling: $(<F)"
	$(V_AT)$(CXX) $(DF_CFLAGS) $(DF_CXXFLAGS) $(DF_CPPFLAGS) -o "$@" -c "$<"

## Enable dependency generation: ##
-include $(DF_OBJECTS_DAEMON:%.o=%.d)
