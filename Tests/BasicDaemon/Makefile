### DaemonFramework Basic Daemon Makefile ###

###################### Primary Build Target: ##################################
BasicDaemon : buildDaemon
	@echo Linking "$(APP_TARGET)"
	$(V_AT)$(CXX) -o $(APP_TARGET) $(OBJECTS_DAEMON) $(DF_OBJECTS_DAEMON) \
                     $(LDFLAGS) $(TARGET_ARCH)

######################## Initialize build variables: ##########################
# The daemon program's executable name:
APP_TARGET = BasicDaemon
# Set Debug or Release mode:
CONFIG ?= Debug
# enable or disable verbose output:
VERBOSE ?= 0
V_AT := $(shell if [ $(VERBOSE) != 1 ]; then echo '@'; fi)

# Select specific build architectures:
TARGET_ARCH ?= -march=native

# Save project paths: 
BASIC_DAEMON_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
TEST_DIR := $(shell dirname $(realpath $(BASIC_DAEMON_DIR)))
PROJECT_DIR := $(shell dirname $(realpath $(TEST_DIR)))
TEST_EXEC_DIR := $(TEST_DIR)/exec
OBJDIR := $(TEST_EXEC_DIR)/build/$(CONFIG)/$(APP_TARGET)

################ Configure and include framework makefile: ####################
DF_CONFIG ?= $(CONFIG)
DF_VERBOSE ?= $(VERBOSE)
DF_OBJDIR ?= $(OBJDIR)

include $(PROJECT_DIR)/MakeDaemon.mk

############################### Set build flags: ##############################
#### Config-specific flags: ####
ifeq ($(CONFIG),Debug)
    OPTIMIZATION ?= 1
    GDB_SUPPORT ?= 0
    # Debug-specific preprocessor definitions:
    CONFIG_FLAGS = -DDEBUG=1
endif

ifeq ($(CONFIG),Release)
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

#### C compilation flags: ####
CFLAGS := $(TARGET_ARCH) \
          $(CONFIG_CFLAGS) \
          $(CFLAGS)

#### C++ compilation flags: ####
CXXFLAGS := -std=gnu++14 $(CXXFLAGS)

#### C Preprocessor flags: ####


# Include directories:
RECURSIVE_INCLUDE_DIRS := $(PROJECT_DIR)/Include/Daemon \
                          $(PROJECT_DIR)/Include/Implementation
DF_DIR_FLAGS := $(DF_DIR_FLAGS) \
                $(call recursiveInclude,$(RECURSIVE_INCLUDE_DIRS))

# Disable dependency generation if multiple architectures are set
DF_DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

CPPFLAGS := -pthread \
            $(DF_DEPFLAGS) \
            $(DF_CONFIG_FLAGS) \
            $(DF_DEFINE_FLAGS) \
            $(DF_DIR_FLAGS) \
            $(CPPFLAGS)

#### Linker flags: ####
LDFLAGS := -lpthread \
           $(TARGET_ARCH) \
           $(CONFIG_LDFLAGS) \
           $(LDFLAGS)


###################### Supporting Build Targets: ##############################
.PHONY: install uninstall clean buildDaemon

install:
	$(V_AT)sudo mkdir -p $(shell dirname $(DF_DAEMON_PATH)); \
	sudo cp $(APP_TARGET) $(DF_DAEMON_PATH); \
    sudo setcap -q cap_dac_override=ep $(APP_TARGET); \
    sudo setcap -q cap_dac_override=ep $(DF_DAEMON_PATH);

uninstall:
	@echo Uninstalling "$(APP_TARGET)"
	$(V_AT)rm $(DF_DAEMON_PATH)

clean:
	@echo Cleaning "$(APP_TARGET)"
	$(V_AT)rm -rf $(OBJDIR)
	$(V_AT)rm $(APP_TARGET)

OBJECTS_DAEMON := $(OBJDIR)/Daemon_BasicDaemon.o

$(OBJDIR)/Daemon_BasicDaemon.o: $(BASIC_DAEMON_DIR)/BasicDaemon.cpp

$(OBJECTS_DAEMON) :
	-$(V_AT)mkdir -p $(OBJDIR)
	@echo "      Compiling: $(<F)"
	$(V_AT)$(CXX) $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o "$@" -c "$<"

buildDaemon : daemonFramework $(OBJECTS_DAEMON)
