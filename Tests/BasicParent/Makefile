### DaemonFramework Basic Parent Makefile ###

###################### Primary Build Target: ##################################
BasicParent : buildParent
	@echo Linking "$(APP_TARGET)"
	$(V_AT)$(CXX) -o $(APP_TARGET) $(OBJECTS_PARENT) $(DF_OBJECTS_PARENT) \
                     $(LDFLAGS) $(TARGET_ARCH)

######################## Initialize build variables: ##########################
# The daemon program's executable name:
APP_TARGET = BasicParent
# Set Debug or Release mode:
CONFIG ?= Debug
# enable or disable verbose output:
VERBOSE ?= 0
V_AT := $(shell if [ $(VERBOSE) != 1 ]; then echo '@'; fi)

# Select specific build architectures:
TARGET_ARCH ?= -march=native

# Save project paths: 
BASIC_PARENT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
TEST_DIR := $(shell dirname $(realpath $(BASIC_PARENT_DIR)))
PROJECT_DIR := $(shell dirname $(realpath $(TEST_DIR)))
TEST_EXEC_DIR := $(TEST_DIR)/testExecutables
OBJDIR := $(TEST_EXEC_DIR)/build/$(CONFIG)/$(APP_TARGET)
DAEMON_PATH ?= $(TEST_EXEC_DIR)/secured/BasicDaemon
PARENT_PATH ?= $(TEST_EXEC_DIR)/secured/$(APP_TARGET)


################ Configure and include framework makefile: ####################
DF_CONFIG := $(CONFIG)
DF_VERBOSE := $(VERBOSE)
DF_OBJDIR := $(OBJDIR)
DF_DAEMON_PATH := \'\"$(DAEMON_PATH)\"\'
DF_INPUT_PIPE_PATH := \'\"$(TEST_EXEC_DIR)/.inPipe\"\'
DF_OUTPUT_PIPE_PATH := \'\"$(TEST_EXEC_DIR)/.outPipe\"\'
include $(PROJECT_DIR)/MakeDaemonParent.mk


############################### Set build flags: ##############################
#### Config-specific flags: ####
ifeq ($(CONFIG),Debug)
    OPTIMIZATION ?= 0
    GDB_SUPPORT ?= 1
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
RECURSIVE_INCLUDE_DIRS := $(PROJECT_DIR)/Include/Parent \
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
.PHONY: install uninstall clean buildParent

install:
	$(V_AT)sudo mkdir -p $(shell dirname $(PARENT_PATH)); \
	sudo cp $(APP_TARGET) $(PARENT_PATH);

uninstall:
	@echo Uninstalling "$(APP_TARGET)"
	$(V_AT)rm $(PARENT_PATH)

clean:
	@echo Cleaning "$(APP_TARGET)"
	$(V_AT)rm -rf $(OBJDIR)
	$(V_AT)rm $(APP_TARGET)

OBJECTS_PARENT := $(OBJDIR)/Parent_BasicParent.o

$(OBJDIR)/Parent_BasicParent.o: $(BASIC_PARENT_DIR)/BasicParent.cpp

$(OBJECTS_PARENT) :
	-$(V_AT)mkdir -p $(OBJDIR)
	@echo "      Compiling: $(<F)"
	$(V_AT)$(CXX) $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o "$@" -c "$<"

buildParent : daemonParent $(OBJECTS_PARENT)

## Enable dependency generation: ##
-include $(OBJECTS_PARENT:%.o=%.d)
