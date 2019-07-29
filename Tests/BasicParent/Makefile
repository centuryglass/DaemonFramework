### DaemonFramework Basic Parent Makefile ###

###################### Primary Build Target: ##################################
BasicParent : buildParent
	@echo Linking "$(APP_TARGET):"
	@if [ "$(VERBOSE)" == "1" ]; then \
        $(PROJECT_DIR)/cleanPrint.sh '$(CXX) $(LINK_ARGS)'; \
        echo ''; \
	fi
	@$(CXX) $(LINK_ARGS)

######################## Initialize build variables: ##########################
# The daemon program's executable name:
APP_TARGET=BasicParent
# Set Debug or Release mode:
CONFIG?=Debug
# enable or disable verbose output:
VERBOSE?=0
V_AT:=$(shell if [ $(VERBOSE) != 1 ]; then echo '@'; fi)

# Select specific build architectures:
TARGET_ARCH?=-march=native

# Save project paths: 
BASIC_PARENT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
TEST_DIR:=$(shell dirname $(realpath $(BASIC_PARENT_DIR)))
PROJECT_DIR:=$(shell dirname $(realpath $(TEST_DIR)))
TEST_EXEC_DIR:=$(TEST_DIR)/exec
TEST_BUILD_DIR:=$(TEST_DIR)/build
TARGET_BUILD_PATH:=$(TEST_BUILD_DIR)/$(APP_TARGET)
OBJDIR:=$(TEST_BUILD_DIR)/intermediate

################ Configure and include framework makefile: ####################
DF_CONFIG?=$(CONFIG)
DF_VERBOSE?=$(VERBOSE)
DF_OBJDIR?=$(OBJDIR)
DF_DAEMON_PATH?=$(TEST_EXEC_DIR)/secured/$(APP_TARGET)

include $(PROJECT_DIR)/Parent.mk

############################### Set build flags: ##############################
#### Config-specific flags: ####
ifeq ($(CONFIG),Debug)
    OPTIMIZATION?=0
    GDB_SUPPORT?=1
    # Debug-specific preprocessor definitions:
    CONFIG_FLAGS=-DDEBUG=1
endif

ifeq ($(CONFIG),Release)
    OPTIMIZATION?=1
    GDB_SUPPORT?=0
endif

# Set optimization level flags:
ifeq ($(OPTIMIZATION),1)
    CONFIG_CFLAGS:=-O3 -flto
    CONFIG_LDFLAGS:=-flto
else
    CONFIG_CFLAGS:=-O0
endif

# Set debugging flags:
ifeq ($(GDB_SUPPORT),1)
    CONFIG_CFLAGS:=$(CONFIG_CFLAGS) -g -ggdb
else
    CONFIG_LDFLAGS:=$(CONFIG_LDFLAGS) -fvisibility=hidden
endif

#### C compilation flags: ####
CFLAGS:=$(TARGET_ARCH) $(CONFIG_CFLAGS) $(CFLAGS)

#### C++ compilation flags: ####
CXXFLAGS:=-std=gnu++14 $(CXXFLAGS)

#### C Preprocessor flags: ####

# Disable dependency generation if multiple architectures are set
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

CPPFLAGS := -pthread \
            $(DEPFLAGS) \
            $(CONFIG_FLAGS) \
            $(DF_DEFINE_FLAGS) \
            $(DF_INCLUDE_FLAGS) \
            $(CPPFLAGS)

#### Linker flags: ####
LDFLAGS := -lpthread $(TARGET_ARCH) $(CONFIG_LDFLAGS) $(LDFLAGS)

#### Aggregated build arguments: ####
OBJECTS_PARENT := $(OBJDIR)/BasicParent.o

# Complete set of flags used to compile source files:
BUILD_FLAGS:=$(CFLAGS) $(CXXFLAGS) $(CPPFLAGS)

# Complete set of arguments used to link the program:
LINK_ARGS:= -o $(TARGET_BUILD_PATH) $(OBJECTS_PARENT) $(DF_OBJECTS_PARENT) \
               $(LDFLAGS)

###################### Supporting Build Targets: ##############################
.PHONY: install uninstall clean buildParent

install:
	$(V_AT)sudo mkdir -p $(shell dirname $(DF_REQUIRED_PARENT_PATH)); \
	sudo cp $(TARGET_BUILD_PATH) $(DF_REQUIRED_PARENT_PATH);

clean:
	@echo Cleaning "$(APP_TARGET)"
	$(V_AT)if [ -d $(OBJDIR) ]; then \
	    rm -rf $(OBJDIR); \
    fi; \
    if [ -f $(TARGET_BUILD_PATH) ]; then \
	    rm $(TARGET_BUILD_PATH); \
    fi

uninstall:
	@echo Uninstalling "$(APP_TARGET)"
	$(V_AT)if [ -f $(DF_REQUIRED_PARENT_PATH) ]; then \
        rm $(DF_REQUIRED_PARENT_PATH); \
    fi


$(OBJDIR)/BasicParent.o: $(BASIC_PARENT_DIR)/BasicParent.cpp

$(OBJECTS_PARENT) :
	@echo "Compiling: $(<F):"
	$(V_AT)mkdir -p $(OBJDIR)
	@if [ "$(VERBOSE)" == "1" ]; then \
        $(PROJECT_DIR)/cleanPrint.sh '$(CXX) $(BUILD_FLAGS)'; \
        echo '    -o "$@" \'; \
        echo '    -c "$<"'; \
        echo ''; \
	fi
	@$(CXX) $(BUILD_FLAGS) -o "$@" -c "$<"

buildParent : df-parent $(OBJECTS_PARENT)

## Enable dependency generation: ##
-include $(OBJECTS_PARENT:%.o=%.d)
