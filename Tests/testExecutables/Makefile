### DaemonFramework Test Makefile ###
# Builds the test daemon and daemon parent:


######### Define paths: #########
TEST_EXEC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
TEST_DIR := $(shell dirname $(realpath $(TEST_EXEC_DIR)))
DF_ROOT_DIR := $(shell dirname $(realpath $(TEST_DIR)))

DAEMON_INCLUDE_DIR := $(DF_ROOT_DIR)/Include/Daemon
PARENT_INCLUDE_DIR := $(DF_ROOT_DIR)/Include/Parent

######### Initialize build variables: #########
# Shared:
DF_CONFIG ?= Debug
DF_VERBOSE ?= 1
DAEMON_TARGET = TestDaemon
PARENT_TARGET = TestParent

DF_OBJDIR := $(TEST_EXEC_DIR)/build
OUTDIR := $(DF_OBJDIR)
DF_DAEMON_PATH ?= $(TEST_EXEC_DIR)/secured/$(DAEMON_TARGET)
DF_REQUIRED_PARENT_PATH ?= $(TEST_EXEC_DIR)/secured/$(PARENT_TARGET)
DF_INPUT_PIPE_PATH ?= $(TEST_EXEC_DIR)/.inPipe
DF_OUTPUT_PIPE_PATH ?= $(TEST_EXEC_DIR)/.outPipe

# Keep debug and release build files in separate directories:
OUTDIR := $(OUTDIR)/$(DF_CONFIG)

include $(DF_ROOT_DIR)/Makefile

# Required library flags:
LDFLAGS := -lpthread $(LDFLAGS)

# Extra compilation flags:
CPPFLAGS := -pthread $(CPPFLAGS)

# Set C preprocessor definitions: 
CPPFLAGS := -DDF_DAEMON_PATH="\"$(DF_DAEMON_PATH)\"" $(CPPFLAGS)
ifneq ($(DF_REQUIRED_PARENT_PATH),)
    CPPFLAGS := -DDF_REQUIRED_PARENT_PATH="\"$(DF_REQUIRED_PARENT_PATH)\"" \
                       $(CPPFLAGS)
endif
ifeq ($(DF_VERBOSE),1)
    CPPFLAGS := -DDF_VERBOSE=1 $(CPPFLAGS)
endif
ifneq ($(DF_TIMEOUT),)
    CPPFLAGS := -DDF_TIMEOUT="$(DF_TIMEOUT)" $(CPPFLAGS)
endif

# Extra compilation flags (C++ only):
CXXFLAGS := -std=gnu++14 $(CXXFLAGS)


#### Setup: ####
# enable or disable verbose output:
ifeq ($(DF_VERBOSE), 1)
    V_AT =
else
    V_AT = @
endif

# Disable dependency generation if multiple architectures are set
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)


ifeq ($(DF_CONFIG),Debug)
    # Enable optimization and disable gdb flags and tests unless otherwise
    # specified:
    OPTIMIZATION ?= 1
    GDB_SUPPORT ?= 0
    # Debug-specific preprocessor definitions:
    CONFIG_FLAGS = -DDF_DEBUG=1
endif

ifeq ($(DF_CONFIG),Release)
    # Enable optimization and disable gdb flags and tests unless otherwise
    # specified:
    OPTIMIZATION ?= 1
    GDB_SUPPORT ?= 0
endif

# Set optimization level flags:
ifeq ($(OPTIMIZATION), 1)
    CONFIG_CFLAGS := $(CONFIG_CFLAGS) -O3 -flto
    CONFIG_LDFLAGS := $(CONFIG_LDFLAGS) -flto
else
    CONFIG_CFLAGS := $(CONFIG_CFLAGS) -O0
endif

# Set debugging flags:
ifeq ($(GDB_SUPPORT), 1)
    CONFIG_CFLAGS := $(CONFIG_CFLAGS) -g -ggdb
else
    CONFIG_LDFLAGS := $(CONFIG_LDFLAGS) -fvisibility=hidden
endif

DAEMON_FLAGS := -I$(DAEMON_INCLUDE_DIR)

PARENT_FLAGS := -I$(PARENT_INCLUDE_DIR)

CPPFLAGS := $(DEPFLAGS) \
            $(CONFIG_FLAGS) \
	        $(DIR_FLAGS) \
            $(CPPFLAGS)

CFLAGS := $(CPPFLAGS) \
     	  $(TARGET_ARCH) \
          $(CONFIG_CFLAGS) \
          $(CFLAGS)

CXXFLAGS := $(CFLAGS) \
            $(CXXFLAGS)

LDFLAGS := $(TARGET_ARCH) \
	        -L$(BINDIR) \
	        -L$(LIBDIR) \
	         $(CONFIG_LDFLAGS) \
	         $(LDFLAGS)


.PHONY: buildParent


OBJECTS_PARENT := $(TEST_EXEC_DIR)/build/Debug/Parent_TestParent.o
$(TEST_EXEC_DIR)/build/Debug/Parent_TestParent.o: $(TEST_EXEC_DIR)/TestParent.cpp

TestParent : buildParent
	@echo Linking "TestParent"
	$(V_AT)$(CXX) -o TestParent $(DF_OBJECTS_PARENT) $(DF_OBJECTS_SHARED) \
                     $(OBJECTS_PARENT) $(LDFLAGS) $(LDFLAGS_APP) $(RESOURCES) \
					 $(TARGET_ARCH)

# buildParent : daemon-parent $(OUTDIR)/$(PARENT_TARGET)
buildParent : daemonParent $(OBJECTS_PARENT)


$(OBJECTS_PARENT) :
	-$(V_AT)mkdir -p $(DF_OBJDIR)
	@echo "OBJECTS_PARENT=: $(OBJECTS_PARENT)"
	@echo "      Compiling: $(<F)"
	$(V_AT)$(CXX) $(DF_CXXFLAGS) $(PARENT_FLAGS) $(CPPFLAGS) $(CFLAGS)\
		-o "$@" -c "$<"

#-include $(OBJECTS_PARENT:%Parent_%.o=Parent_%.d)
