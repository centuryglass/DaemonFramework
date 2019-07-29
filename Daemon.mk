define HELPTEXT
### DaemonFramework Daemon Makefile ###
#  This makefile provides the df-daemon target, used to build all files required
# for daemon implementations.
#
## Quick Guide: ##
# 1. Provide valid definitions for the following variables:
#    - DF_OBJDIR
#
# 2. Optionally, provide valid definitions for these additional variables to
#    enable features or override default values:
#    - DF_CONFIG
#    - DF_VERBOSE
#    - DF_OPTIMIZATION
#    - DF_GDB_SUPPORT
#    - DF_INPUT_PIPE_PATH
#    - DF_OUTPUT_PIPE_PATH
#    - DF_DAEMON_PATH
#    - DF_REQUIRED_PARENT_PATH
#    - DF_LOCK_FILE_PATH
#    - DF_VERIFY_PATH_SECURITY
#    - DF_VERIFY_PARENT_PATH_SECURITY
#    - DF_REQUIRE_RUNNING_PARENT
#    - DF_TIMEOUT
#
# 3. If necessary, define CFLAGS, CXXFLAGS, and/or CPPFLAGS with any extra
#    compilation flags that should be used when compiling DaemonFramework code
#    files.
#
# 4. Include Daemon.mk in your application's makefile after defining variables.
#
# 5. Add df-daemon as a dependency to your main build target to ensure relevant
#    DaemonFramework code files are compiled.
#
# 6. When building code files that include DaemonFramework header files, add
#    DF_DEFINE_FLAGS and DF_INCLUDE_FLAGS to CPPFLAGS.
#
# 7. When linking your application, add DF_OBJECTS_DAEMON to your list of linked
#    object files.
#
##### Variables Descriptions: ####
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
#      ensure that only one daemon instance may run at one time. Providing a
#      valid path is strongly recommended if IO pipes are enabled, as pipes will
#      behave unpredictably when shared by multiple daemon/parent instances.
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
# Define DaemonFramework directories:
DF_ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DF_INCLUDE_DIR:=$(DF_ROOT_DIR)/Include
DF_DAEMON_INCLUDE_DIR:=$(DF_INCLUDE_DIR)/Daemon
DF_SHARED_INCLUDE_DIR:=$(DF_INCLUDE_DIR)/Shared

# Import targets and variables shared with the daemon parent makefile:
DF_SHARED_MAKEFILE:=$(DF_ROOT_DIR)/Shared.mk
ifeq ($(findstring $(DF_SHARED_MAKEFILE),$(MAKEFILE_LIST)),)
    include $(DF_SHARED_MAKEFILE)
endif

# Set default daemon security options:
DF_VERIFY_PATH?=1
DF_VERIFY_PATH_SECURITY?=1
DF_VERIFY_PARENT_PATH_SECURITY?=1
DF_REQUIRE_RUNNING_PARENT?=1

############################### Set build flags: ##############################

# Include directories:
DF_INCLUDE_FLAGS:=$(call recursiveInclude,$(DF_DAEMON_INCLUDE_DIR)) \
                  $(DF_INCLUDE_FLAGS)

# C preprocessor definitions:
DF_DEFINE_FLAGS:=$(DF_DEFINE_FLAGS) \
                 $(call addStringDef,DF_LOCK_FILE_PATH) \
                 $(call addStringDef,DF_REQUIRED_PARENT_PATH) \
                 $(call addDef,DF_VERIFY_PATH) \
                 $(call addDef,DF_VERIFY_PATH_SECURITY) \
                 $(call addDef,DF_VERIFY_PARENT_PATH_SECURITY) \
                 $(call addDef,DF_REQUIRE_RUNNING_PARENT) \
                 $(call addDef,DF_TIMEOUT) \
                 -DDF_IS_DAEMON=1

DF_CPPFLAGS:=$(DF_CPPFLAGS) $(DF_DEFINE_FLAGS) $(DF_INCLUDE_FLAGS) $(CPPFLAGS) 

DF_BUILD_FLAGS:=$(DF_CFLAGS) $(DF_CXXFLAGS) $(DF_CPPFLAGS)

######################## Load module source lists: ############################
include $(DF_ROOT_DIR)/Source/Daemon/daemon.mk

# Only include shared objects with DF_OBJECTS_DAEMON if they haven't already
# been included in DF_OBJECTS_PARENT:
DF_PARENT_MAKEFILE:=$(DF_ROOT_DIR)/Parent.mk
ifeq ($(findstring $(DF_OBJECTS_SHARED),$(DF_OBJECTS_PARENT)),)
    DF_OBJECTS_DAEMON:=$(DF_OBJECTS_SHARED) $(DF_OBJECTS_DAEMON)
endif

############################# Build Targets: ##################################

.PHONY: df-daemon df-check-daemon-defs df-help

## Main build target: ##
# Checks definitions, then compiles daemon support classes.
df-daemon : df-check-daemon-defs $(DF_OBJECTS_DAEMON)
	@echo Compiled daemon object files at "$(DF_OBJDIR)"

## Ensure required variables are defined: ##
df-check-daemon-defs:
	@if [ -z "$(DF_OBJDIR)" ]; then \
        echo >&2 "Build failed, DF_OBJDIR not defined."; exit 1; \
    fi
	@if [ -z "$(DF_LOCK_FILE_PATH)" ]; then \
        if [ -n "$(DF_INPUT_PIPE_PATH)" ] || [ -n "$(DF_OUTPUT_PIPE_PATH)" ]; then \
            echo >&2 "Build failed, IO pipes used but no lock file path defined."; \
            exit 1; \
        fi \
    fi

## Compile all files needed to build Daemon applications: ##
$(DF_OBJECTS_DAEMON) :
	@echo "Compiling $(<F):"
	$(V_AT)mkdir -p $(DF_OBJDIR)
	@if [ "$(DF_VERBOSE)" == "1" ]; then \
        $(DF_ROOT_DIR)/cleanPrint.sh '$(CXX) $(DF_BUILD_FLAGS)'; \
        echo '    -o "$@" \'; \
        echo '    -c "$<"'; \
        echo ''; \
	fi
	@$(CXX) $(DF_CFLAGS) $(DF_CXXFLAGS) $(DF_CPPFLAGS) -o "$@" -c "$<"

## Enable dependency generation: ##
-include $(DF_OBJECTS_DAEMON:%.o=%.d)
