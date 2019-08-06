define HELPTEXT
### DaemonFramework Parent Makefile ###
#
# This makefile provides the df-parent target, used to build all files required
# by applications that manage DaemonFramework daemons.
#
## Quick Guide: ##
#
# 1. Provide valid definitions for the following variables:
#    - DF_OBJDIR : where all .o files will be created
#
# 2. Optionally, provide valid definitions for these additional variables to
#    enable features or override default values:
#    - DF_CONFIG       : set Debug or Release mode
#    - DF_VERBOSE      : enable or disable verbose output
#    - DF_OPTIMIZATION : enable or disable optimization
#    - DF_GDB_SUPPORT  : enable or disable gdb support
# 
# 3. If necessary, define CFLAGS, CXXFLAGS, and/or CPPFLAGS with any extra
#    compilation flags that should be used when compiling DaemonFramework code
#    files.
#
# 4. Include Parent.mk in your application's makefile after defining variables.
#
# 5. Add df-parent as a dependency to your main build target to ensure relevant
#    DaemonFramework code files are compiled.
#
# 6. When building code files that include DaemonFramework header files, add
#    DF_DEFINE_FLAGS and DF_INCLUDE_FLAGS to CPPFLAGS.
#
# 7. When linking your application, add DF_OBJECTS_PARENT to your list of linked
#    object files.
#
#
##### Variables Descriptions: ####
### Build Control:
#   DF_OBJDIR:  The directory where compiled .o files will be created.
#
#   DF_SHARED_OBJDIR:
#      The directory where compiled .o files shared between DaemonControl
#      subclasses will be created.
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
#   DF_CPP_VERSION:
#      Sets the version of C++ used to compile the daemon parent files. Versions
#      before C++14 are untested and not recommended.
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
#   DF_GENERATE_DEPS: (default: 1)
#      Sets whether the DaemonFramework will track dependencies when compiling
#      code files. Disable this if compiling for multiple architectures.
#
endef
export HELPTEXT

######################## Initialize build variables: ##########################
# Define DaemonFramework directories:
DF_ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DF_INCLUDE_DIR:=$(DF_ROOT_DIR)/Include
DF_PARENT_INCLUDE_DIR:=$(DF_INCLUDE_DIR)/Parent
DF_SHARED_INCLUDE_DIR:=$(DF_INCLUDE_DIR)/Shared

# Import targets and variables shared with the daemon parent makefile:
DF_SHARED_MAKEFILE:=$(DF_ROOT_DIR)/Shared.mk
ifeq ($(findstring $(DF_SHARED_MAKEFILE),$(MAKEFILE_LIST)),)
    include $(DF_SHARED_MAKEFILE)
endif

# Ensure the shared objdir has some sort of valid definition.
DF_SHARED_OBJDIR?=$(DF_SHARED_OBJDIR)
DF_SHARED_OBJDIR?=$(DF_OBJDIR)

############################### Set build flags: ##############################
# Include directories:
DF_INCLUDE_FLAGS:=$(call recursiveInclude,$(DF_PARENT_INCLUDE_DIR)) \
                   $(DF_INCLUDE_FLAGS) 

# C preprocessor definitions:
DF_DEFINE_FLAGS:=$(DF_DEFINE_FLAGS) -DDF_IS_PARENT=1 


DF_CPPFLAGS:=$(DF_CPPFLAGS) $(DF_DEFINE_FLAGS) \
             $(DF_INCLUDE_FLAGS) $(CPPFLAGS)

DF_BUILD_FLAGS:=$(DF_CFLAGS) $(DF_CXXFLAGS) $(DF_CPPFLAGS)

######################## Load module source lists: ############################
include $(DF_ROOT_DIR)/Source/Parent/parent.mk

# Only include shared objects with DF_OBJECTS_PARENT if they haven't already
# been included in DF_OBJECTS_DAEMON:
ifeq ($(findstring $(DF_OBJECTS_SHARED),$(DF_OBJECTS_DAEMON)),)
    DF_OBJECTS_PARENT:=$(DF_OBJECTS_SHARED) $(DF_OBJECTS_PARENT)
endif

############################# Build Targets: ##################################

.PHONY: df-parent df-check-parent-defs df-help

## Main build target: ##
# Checks definitions, then compiles daemon parent support classes.
df-parent: df-check-parent-defs \
 $(DF_OBJECTS_PARENT)
	@echo Compiled daemon parent object files at "$(DF_OBJDIR)"


## Ensure required variables are defined: ##
df-check-parent-defs:
	@if [ -z "$(DF_OBJDIR)" ]; then \
        echo >&2 "Build failed, DF_OBJDIR not defined."; exit 1; \
    elif [ -z "$(DF_DAEMON_PATH)" ]; then \
        echo >&2 "Build failed, DF_DAEMON_PATH not defined."; exit 1; \
    fi


## Compile all files needed to build Daemon applications: ##
$(DF_OBJECTS_PARENT) :
	@echo "Compiling $(<F):"
	$(V_AT)mkdir -p $(DF_OBJDIR)
	@if [ "$(DF_VERBOSE)" == "1" ]; then \
        $(DF_ROOT_DIR)/cleanPrint.sh '$(CXX) $(DF_BUILD_FLAGS)'; \
        echo '    -o "$@" \'; \
        echo '    -c "$<"'; \
        echo ''; \
	fi
	@$(CXX) $(DF_BUILD_FLAGS) -o "$@" -c "$<"

## Print the guide to this makefile: ##
df-help:
	$(shell echo "$$HELPTEXT")

## Enable dependency generation: ##
-include $(DF_OBJECTS_PARENT:%.o=%.d)
