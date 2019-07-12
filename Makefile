define HELPTEXT
### DaemonFramework Makefile ###
#  This makefile provides the daemon and daemonParent targets. When building a
# daemon or an application that creates/controls a daemon, include this file
# within your project's makefile, and add the daemon or daemonParent target to
# your main build target as a dependency, and set build parameters to configure
# the project as needed.
#
#  The DaemonFramework provides several ways to grant special privileges to your
# daemon, and a selection of optional security measures to keep malicious
# sources from exploiting your daemon to abuse those privileges. It is your
# responsibility to ensure your daemon uses the lowest privilege level and 
# the largest possible set of security restrictions that are compatible with
# your particular daemon's role.
#
### Shared Build Parameters:
#  These variables will affect both the daemon and the daemon parent
# application.
#  
## Required:
#    DF_OBJDIR:  The directory where compiled .o files will be created.
#
#    DF_DAEMON_PATH:
#      The path where the daemon process executable will be found after
#      installation. This is used by the parent application when launching the
#      daemon, and is optionally checked by the daemon as a security measure.
#
## Optional:
#    DF_CONFIG: (default: 'Release')
#      Sets whether the daemon or parent compiles in Debug mode, with
#      optimization disabled and debugging enabled, or in Release mode, with
#      optimization enabled and debugging disabled.
#                
#      In Debug mode, the DF_DEBUG preprocessor macro will be set, and the
#      DF_DBG macro function will print to stdout.
#     
#    DF_VERBOSE: (default: 0)
#      Sets whether verbose output is enabled while building and during
#      execution.  If enabled, DF_VERBOSE will be defined in the C preprocessor,
#      and the DF_DBG_V macro function will print to stdout.
#
#   DF_INPUT_PIPE_PATH:
#      If defined, the parent application will use a named pipe at this path to
#      send data to the daemon.
#
#   DF_OUTPUT_PIPE_PATH:
#      If defined, the daemon will use a named pipe at this path to send data to
#      its parent application.
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
## Daemon Build Parameters:
#
## Required:
#
#    DF_VERIFY_PATH: (default: 1)
#      If set to 1, the daemon will locate its own process, check the path of
#      the executable it is running, and automatically exit if running from any
#      executable path other than DF_DAEMON_PATH.
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
#    DF_REQUIRE_SINGULAR: (default: 1)
#      If set to 1, only one instance of the daemon will be allowed to run at
#      one time.
#
#    DF_REQUIRE_RUNNING_PARENT: (default: 1)
#      If set to 1, the daemon will periodically check if its parent process is
#      still running, and exit if its parent has exited.
#
## Optional:
#    DF_REQUIRED_PARENT_PATH:
#      If defined, the daemon will find its parent process, check the path of
#      the executable that the parent process is running, and automatically exit
#      if the parent is running from any executable path other than
#      DF_REQUIRED_PARENT_PATH.
#
#    DF_TIMEOUT:  
#      If defined, the daemon will automatically exit after running for TIMEOUT
#      seconds.
#
endef
export HELPTEXT

######### Initialize build variables: #########
# Shared:
DF_CONFIG ?= Release
DF_VERBOSE ?= 0
DF_TARGET_ARCH ?= -march=native
DF_ROOT_DIR ?= $(shell pwd)

# Daemon:
DF_VERIFY_PATH ?= 1
DF_VERIFY_PATH_SECURITY ?= 1
DF_VERIFY_PARENT_PATH_SECURITY ?= 1
DF_REQUIRE_SINGULAR ?= 1
DF_REQUIRE_RUNNING_PARENT ?= 1

# Required library flags:
DF_LDFLAGS := -lpthread $(LDFLAGS)

# Extra compilation flags:
DF_CPPFLAGS := -pthread $(CPPFLAGS)

# Set C preprocessor definitions:
DF_CPPFLAGS := -DDF_DAEMON_PATH="\"$(DF_DAEMON_PATH)\"" $(DF_CPPFLAGS)
ifeq ($(DF_VERBOSE),1)
    DF_CPPFLAGS := -DDF_VERBOSE=1 $(DF_CPPFLAGS)
endif
ifneq ($(DF_INPUT_PIPE_PATH),)
    DF_CPPFLAGS := -DDF_INPUT_PIPE_PATH="\"$(DF_INPUT_PIPE_PATH)\"" \
                   $(DF_CPPFLAGS)
endif
ifneq ($(DF_OUTPUT_PIPE_PATH),)
    DF_CPPFLAGS := -DDF_OUTPUT_PIPE_PATH="\"$(DF_OUTPUT_PIPE_PATH)\"" \
                   $(DF_CPPFLAGS)
endif
ifeq ($(DF_VERIFY_PATH),1)
    DF_DAEMON_FLAGS = -DDF_VERIFY_PATH=1
endif
ifeq ($(DF_VERIFY_PATH_SECURITY),1)
    DF_DAEMON_FLAGS := -DDF_VERIFY_PATH_SECURITY=1 $(DF_DAEMON_FLAGS)
endif
ifeq ($(DF_VERIFY_PARENT_PATH_SECURITY),1)
    DF_DAEMON_FLAGS := -DDF_VERIFY_PARENT_PATH_SECURITY=1 $(DF_DAEMON_FLAGS)
endif
ifeq ($(DF_REQUIRE_SINGULAR),1)
    DF_DAEMON_FLAGS := -DDF_REQUIRE_SINGULAR=1 $(DF_DAEMON_FLAGS)
endif
ifeq ($(DF_REQUIRE_RUNNING_PARENT),1)
    DF_DAEMON_FLAGS := -DDF_REQUIRE_RUNNING_PARENT=1 $(DF_DAEMON_FLAGS)
endif
ifneq ($(DF_REQUIRED_PARENT_PATH),)
    DF_DAEMON_FLAGS := -DDF_REQUIRED_PARENT_PATH="\"$(DF_REQUIRED_PARENT_PATH)\"" \
                       $(DF_DAEMON_FLAGS)
endif
ifneq ($(DF_TIMEOUT),)
    DF_DAEMON_FLAGS := -DDF_TIMEOUT="$(DF_TIMEOUT)" $(DF_DAEMON_FLAGS)
endif

# Extra compilation flags (C++ only):
DF_CXXFLAGS := -std=gnu++14 $(CXXFLAGS)

# Directories to recursively search for header files:
DF_RECURSIVE_INCLUDE_DIRS := $(DF_ROOT_DIR)/Include

#### Setup: ####
# enable or disable verbose output:
ifeq ($(DF_VERBOSE), 1)
    V_AT =
else
    V_AT = @
endif

# Disable dependency generation if multiple architectures are set
DF_DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

# Generate the list of directory include flags:
DF_DIR_FLAGS := $(shell find $(DF_RECURSIVE_INCLUDE_DIRS) -type d \
                     -printf " -I'%p'")

# Keep debug and release build files in separate directories:
DF_OBJDIR := $(DF_OBJDIR)/$(DF_CONFIG)
DF_OUTDIR := $(DF_OUTDIR)/$(DF_CONFIG)

ifeq ($(DF_CONFIG),Debug)
    # Enable optimization and disable gdb flags and tests unless otherwise
    # specified:
    DF_OPTIMIZATION ?= 1
    DF_GDB_SUPPORT ?= 0
    # Debug-specific preprocessor definitions:
    DF_CONFIG_FLAGS = -DDF_DEBUG=1
endif

ifeq ($(DF_CONFIG),Release)
    # Enable optimization and disable gdb flags and tests unless otherwise
    # specified:
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

DF_CPPFLAGS := $(DF_DEPFLAGS) \
            $(DF_CONFIG_FLAGS) \
	        $(DF_DIR_FLAGS) \
            $(DF_CPPFLAGS)

DF_CFLAGS := $(DF_CPPFLAGS) \
     	     $(DF_TARGET_ARCH) \
             $(DF_CONFIG_CFLAGS) \
             $(CFLAGS)

DF_CXXFLAGS := $(DF_CFLAGS) \
            $(CXXFLAGS)

DF_LDFLAGS := $(TARGET_ARCH) \
	        -L$(BINDIR) \
	        -L$(LIBDIR) \
	         $(CONFIG_LDFLAGS) \
	         $(LDFLAGS)


.PHONY: help print-% check-defs daemon daemonParent

help:
	$(shell echo "$$HELPTEXT")

print-%:
	@echo $* = $($*)

check-defs:
	@if [ -z "$(DF_OBJDIR)" ]; then \
        echo >&2 "Build failed, DF_OBJDIR not defined."; exit 1; \
    elif [ -z "$(DF_DAEMON_PATH)" ]; then \
        echo >&2 "Build failed, DF_DAEMON_PATH path not defined."; exit 1; \
    fi

# Include makefiles defining the DF_OBJECTS_DAEMON, DF_OBJECTS_PARENT, and
# DF_OBJECTS_SHARED target lists:
include $(DF_ROOT_DIR)/Source/Daemon/daemon.mk
include $(DF_ROOT_DIR)/Source/Parent/parent.mk
include $(DF_ROOT_DIR)/Source/Shared/shared.mk

#build : $(OUTDIR)/$(TARGET_APP)

daemon : check-defs $(DF_OBJECTS_DAEMON) $(DF_OBJECTS_SHARED)
	@echo Compiled daemon object files at "$(DF_OBJDIR)"

daemonParent : check-defs $(DF_OBJECTS_PARENT) $(DF_OBJECTS_SHARED)
	@echo Compiled daemon parent object files at "$(DF_OBJDIR)"

$(DF_OBJECTS_DAEMON) :
	-$(V_AT)mkdir -p $(DF_OBJDIR)
	@echo "daemon objects='$(DF_OBJECTS_DAEMON)'"
	@echo "      Compiling: $(<F)"
	$(V_AT)$(CXX) $(DF_CXXFLAGS) $(DF_CPPFLAGS) $(DF_DAEMON_FLAGS) $(DF_CFLAGS)\
		-o "$@" -c "$<"

$(DF_OBJECTS_PARENT) :
	-$(V_AT)mkdir -p $(DF_OBJDIR)
	@echo "parent objects='$(DF_OBJECTS_PARENT)'"
	@echo "      Compiling: $(<F)"
	$(V_AT)$(CXX) $(DF_CXXFLAGS) $(DF_CPPFLAGS) $(DF_CFLAGS)\
		-o "$@" -c "$<"

$(DF_OBJECTS_SHARED) :
	-$(V_AT)mkdir -p $(DF_OBJDIR)
	@echo "shared objects='$(DF_OBJECTS_SHARED)'"
	@echo "      Compiling: $(<F)"
	$(V_AT)$(CXX) $(DF_CXXFLAGS) $(DF_CPPFLAGS) $(DF_CFLAGS)\
		-o "$@" -c "$<"


#-include $(DF_OBJECTS_DAEMON:%DaemonFramework_Daemon_%.o=DaemonFramework_Daemon_%.d)
