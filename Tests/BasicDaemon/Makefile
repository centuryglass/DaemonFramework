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
VERBOSE ?= 1
V_AT = $(shell if [ $DF_VERBOSE == 1 ]; then echo '@'; fi)

# Select specific build architectures:
TARGET_ARCH ?= -march=native

# Save project paths: 
BASIC_DAEMON_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
TEST_DIR := $(shell dirname $(realpath $(BASIC_DAEMON_DIR)))
PROJECT_DIR := $(shell dirname $(realpath $(TEST_DIR)))
TEST_EXEC_DIR := $(TEST_DIR)/testExecutables
OBJDIR := $(TEST_EXEC_DIR)/build/$(CONFIG)/$(APP_TARGET)
DAEMON_PATH ?= $(TEST_EXEC_DIR)/secured/$(APP_TARGET)
PARENT_PATH ?= $(TEST_EXEC_DIR)/secured/BasicParent


################ Configure and include framework makefile: ####################
DF_CONFIG := $(CONFIG)
DF_VERBOSE := $(VERBOSE)
DF_OBJDIR := $(OBJDIR)
DF_DAEMON_PATH := \'\"$(DAEMON_PATH)\"\'
DF_INPUT_PIPE_PATH := \'\"$(TEST_EXEC_DIR)/.inPipe\"\'
DF_OUTPUT_PIPE_PATH := \'\"$(TEST_EXEC_DIR)/.outPipe\"\'
DF_REQUIRED_PARENT_PATH := \'\"$(PARENT_PATH)\"\'
DF_TIMEOUT ?= 1
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

# C preprocessor definitions:
DEFINE_FLAGS := $(call addDef,DF_DAEMON_PATH) \
                $(call addDef,DF_INPUT_PIPE_PATH) \
                $(call addDef,DF_OUTPUT_PIPE_PATH) \

# Include directories:
RECURSIVE_INCLUDE_DIRS := $(PROJECT_DIR)/Include/Daemon \
                          $(PROJECT_DIR)/Include/Implementation
DIR_FLAGS := $(shell find $(RECURSIVE_INCLUDE_DIRS) -type d \
                     -printf " -I'%p'")

# Disable dependency generation if multiple architectures are set
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

CPPFLAGS := -pthread \
            $(DEPFLAGS) \
            $(CONFIG_FLAGS) \
            $(DEFINE_FLAGS) \
            $(DIR_FLAGS) \
            $(CPPFLAGS)

#### Linker flags: ####
LDFLAGS := -lpthread \
           $(TARGET_ARCH) \
           $(CONFIG_LDFLAGS) \
           $(LDFLAGS)


###################### Supporting Build Targets: ##############################
.PHONY: install uninstall clean buildDaemon

install:
	$(V_AT)sudo mkdir -p $(shell dirname $(DAEMON_PATH)); \
	sudo cp $(APP_TARGET) $(DAEMON_PATH); \
    sudo setcap -q cap_dac_override=ep $(DAEMON_PATH)

uninstall:
	@echo Uninstalling "$(APP_TARGET)"
	$(V_AT)rm $(DAEMON_PATH)

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
