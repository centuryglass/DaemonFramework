### DaemonFramework Unit Test Makefile ###

######################## Initialize build variables: ##########################
# The test program's executable name:
APP_TARGET=DaemonTest
# Set Debug or Release mode:
CONFIG?=Debug
# enable or disable verbose output:
VERBOSE?=1
V_AT:=$(shell if [ $(VERBOSE) != 1 ]; then echo '@'; fi)

# Select specific build architectures:
TARGET_ARCH?=-march=native

# Save project paths: 
UNIT_TEST_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
TEST_DIR:=$(shell dirname $(realpath $(UNIT_TEST_DIR)))
PROJECT_DIR:=$(shell dirname $(realpath $(TEST_DIR)))
TEST_EXEC_DIR:=$(TEST_DIR)/exec
TEST_BUILD_DIR:=$(TEST_DIR)/build
TARGET_BUILD_PATH:=$(TEST_BUILD_DIR)/$(APP_TARGET)
OBJDIR:=$(TEST_BUILD_DIR)/Intermediate

###################### Primary Build Target: ##################################
$(TARGET_BUILD_PATH) : buildTests
	@echo Linking "$(APP_TARGET):"
	@if [ "$(VERBOSE)" == "1" ]; then \
        $(PROJECT_DIR)/cleanPrint.sh '$(CXX) $(LINK_ARGS)'; \
        echo ''; \
	fi
	@$(CXX) $(LINK_ARGS)

############### Configure and include framework makefiles: ####################
DF_CONFIG?=$(CONFIG)
DF_VERBOSE?=$(VERBOSE)
DF_OBJDIR?=$(OBJDIR)

include $(PROJECT_DIR)/Daemon.mk
include $(PROJECT_DIR)/Parent.mk

############################### Set build flags: ##############################
#### Config-specific flags: ####
ifeq ($(CONFIG),Debug)
    OPTIMIZATION?=1
    GDB_SUPPORT?=0
    # Debug-specific preprocessor definitions:
    CONFIG_FLAGS=-DDEBUG=1
endif

ifeq ($(CONFIG),Release)
    OPTIMIZATION?=1
    GDB_SUPPORT?=0
endif

# Set optimization level flags:
ifeq ($(OPTIMIZATION), 1)
    CONFIG_CFLAGS:=-O3 -flto
    CONFIG_LDFLAGS:=$(CONFIG_LDFLAGS) -flto
else
    CONFIG_CFLAGS:=-O0
endif

# Set debugging flags:
ifeq ($(GDB_SUPPORT), 1)
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
DEPFLAGS:=$(if $(word 2, $(TARGET_ARCH)), , -MMD)

MAKEFILE_PATH:=$(UNIT_TEST_DIR)/Makefile
DEFINE_FLAGS:=$(call addStringDef,MAKEFILE_PATH) $(DF_DEFINE_FLAGS)

CPPFLAGS:=-pthread \
          $(DEPFLAGS) \
          $(CONFIG_FLAGS) \
          $(DEFINE_FLAGS) \
          $(DF_INCLUDE_FLAGS) \
          $(CPPFLAGS)

#### Linker flags: ####
LDFLAGS:=-lpthread $(TARGET_ARCH) $(CONFIG_LDFLAGS) $(LDFLAGS)

#### Aggregated build arguments: ####
OBJECTS_TEST:=$(OBJDIR)/Test_Main.o $(OBJDIR)/Test_File_Utils.o

# Complete set of flags used to compile source files:
BUILD_FLAGS:=$(CFLAGS) $(CXXFLAGS) $(CPPFLAGS)

# Complete set of arguments used to link the program:
LINK_ARGS:= -o $(TARGET_BUILD_PATH) $(OBJECTS_TEST) $(DF_OBJECTS_DAEMON) \
               $(DF_OBJECTS_PARENT) $(LDFLAGS)

###################### Supporting Build Targets: ##############################
.PHONY: install uninstall clean buildDaemon

clean:
	@echo Cleaning "$(APP_TARGET)"
	$(V_AT)if [ -d $(OBJDIR) ]; then \
	    rm -rf $(OBJDIR); \
    fi; \
    if [ -f $(TARGET_BUILD_PATH) ]; then \
	    rm $(TARGET_BUILD_PATH); \
    fi

$(OBJDIR)/Test_Main.o: $(UNIT_TEST_DIR)/Test_Main.cpp
$(OBJDIR)/Test_File_Utils.o: $(UNIT_TEST_DIR)/Test_File_Utils.cpp

$(OBJECTS_TEST) :
	@echo "Compiling $(<F):"
	$(V_AT)mkdir -p $(OBJDIR)
	@if [ "$(VERBOSE)" == "1" ]; then \
        $(PROJECT_DIR)/cleanPrint.sh '$(CXX) $(BUILD_FLAGS)'; \
        echo '    -o "$@" \'; \
        echo '    -c "$<"'; \
        echo ''; \
	fi
	@$(CXX) $(BUILD_FLAGS) -o "$@" -c "$<"

buildTests : df-daemon df-parent $(OBJECTS_TEST)

## Enable dependency generation: ##
-include $(OBJECTS_TEST:%.o=%.d)
