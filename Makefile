define HELPTEXT
## KeyDaemon Makefile ##
# Usage: make [target] [options]
# Typical installation process:
#   make
#   make install
# 
# TIMEOUT:  Seconds to wait before automatically exiting. This is intended only
#           for use in testing.
#
# Security:
#  KeyDaemon is a setuid helper program that can globally monitor keyboard 
# events, so that the application it supports can detect keyboard input even if
# another window is focused. To stop other sources from using this to spy on
# keyboard input, several limitations must be defined at compile time.
#
#
# INSTALL_PATH:    Full path to where the executable should be created. This
#                 should be a directory specific to your application, that can
#                 only be modified by root. If the KeyDaemon detects that it is
#                 not running from this path it will immediately exit.
#
# PARENT_PATH:     The path to the executable that is allowed to use the
#                 KeyDaemon. When KeyDaemon is launched, it will immediately
#                 exit unless it detects that it was launched by this
#                 executable.
#
#                  For additional security, this executable must be located in a
#                 directory that can only be modified by root. If this isn't the
#                 case, KeyDaemon will immediately exit even if launched by the
#                 correct executable, since there would be no guarantee that the
#                 executable hasn't been replaced.
#
# KEY_PIPE_PATH:   The path where the KeyDaemon will find the pipe file it 
#                 uses to communicate with its parent application.
#
# KEY_LIMIT:       The maximum number of keys that KeyDaemon may track. Set this
#                 to the lowest number required by your application.
#
# CONFIG:          Set to 'Debug' to disable optimization, and enable debugging
#                 and console output messages.  Set to 'Release' to enable
#                 optimization and disable debugging and console output
#                 messages.
#
# V:               Enable or disable verbose build output. In builds where
#                 CONFIG=Debug, this will also enable verbose KeyDaemon debug
#                 output.
endef
export HELPTEXT

######### Initialize build variables: #########
# Executable name:
TARGET_APP = KeyDaemon
# Version number:
APP_VERSION = 0.0.1
# Version hex.
APP_VERSION_HEX = 0x1

# Default Build Options:
# Build type: either Debug or Release
CONFIG ?= Release
# Command used to strip unneeded symbols from object files:
STRIP ?= strip
# Use the build system's architecture by default.
TARGET_ARCH ?= -march=native
# Enable or disable verbose output
V ?= 0

# Build directories:
BINDIR := build
LIBDIR := build
# Directory where .o files will be created:
OBJDIR := build/intermediate
# Directory where executable files will be created:
OUTDIR := build

# List additional library flags here:
LDFLAGS := -lpthread $(LDFLAGS)

# Extra compilation flags:
CPPFLAGS := -pthread $(CPPFLAGS)

ifneq ($(INSTALL_PATH),)
    CPPFLAGS := -DINSTALL_PATH="\"$(INSTALL_PATH)\"" $(CPPFLAGS)
endif

ifneq ($(PARENT_PATH),)
    CPPFLAGS := -DPARENT_PATH="\"$(PARENT_PATH)\"" $(CPPFLAGS)
endif

ifneq ($(KEY_LIMIT),)
    CPPFLAGS := -DKEY_LIMIT="$(KEY_LIMIT)" $(CPPFLAGS)
endif

ifneq ($(KEY_PIPE_PATH),)
    CPPFLAGS := -DKEY_PIPE_PATH="\"$(KEY_PIPE_PATH)\"" $(CPPFLAGS)
endif

ifneq ($(TIMEOUT),)
    CPPFLAGS := -DTIMEOUT="$(TIMEOUT)" $(CPPFLAGS)
endif

# Extra compilation flags (C++ only):
CXXFLAGS := -std=gnu++14 $(CXXFLAGS)

# Directories to search for header files:
INCLUDE_DIRS :=

# Directories to recursively search for header files:
RECURSIVE_INCLUDE_DIRS := Source

#### Setup: ####

# build with "V=1" for verbose builds
ifeq ($(V), 1)
    V_AT =
else
    V_AT = @
endif

# Disable dependency generation if multiple architectures are set
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

# Generate the list of directory include flags:
DIR_FLAGS := $(shell echo $(INCLUDE_DIRS) | xargs printf " -I'%s'") \
             $(shell find $(RECURSIVE_INCLUDE_DIRS) -type d \
                     -printf " -I'%p'")

# Keep debug and release build files in separate directories:
OBJDIR := $(OBJDIR)/$(CONFIG)
OUTDIR := $(OUTDIR)/$(CONFIG)

ifeq ($(CONFIG),Debug)
    # Disable optimization and enable gdb flags and tests unless otherwise
    # specified:
    OPTIMIZATION ?= 0
    GDB_SUPPORT ?= 1
    BUILD_TESTS ?= 1
    # Debug-specific preprocessor definitions:
    CONFIG_FLAGS = -DDEBUG=1 -D_DEBUG=1
    ifeq ($(V), 1)
        CONFIG_FLAGS := -DDEBUG_VERBOSE=1 $(CONFIG_FLAGS)
    endif
endif

ifeq ($(CONFIG),Release)
    # Enable optimization and disable gdb flags and tests unless otherwise
    # specified:
    OPTIMIZATION ?= 1
    GDB_SUPPORT ?= 0
    BUILD_TESTS ?= 0
    # Release-specific preprocessor definitions:
    CONFIG_FLAGS = -DNDEBUG=1
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

CPPFLAGS := $(DEPFLAGS) \
            $(CONFIG_FLAGS) \
	        $(DIR_FLAGS) \
            $(CPPFLAGS)

CFLAGS := $(JUCE_CPPFLAGS) \
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

CLEANCMD = rm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)


.PHONY: build install debug release clean strip uninstall help
build : $(OUTDIR)/$(TARGET_APP)

OBJECTS_APP := $(OBJDIR)/Main.o \
               $(OBJDIR)/InputReader.o \
               $(OBJDIR)/KeyEventFiles.o \
               $(OBJDIR)/KeyReader.o \
               $(OBJDIR)/KeyCode.o \
               $(OBJDIR)/PipeWriter.o \
               $(OBJDIR)/Process_Data.o \
               $(OBJDIR)/Process_State.o \
               $(OBJDIR)/Process_Security.o \
               $(OBJECTS_APP)

$(OUTDIR)/$(TARGET_APP) : check-defs $(OBJECTS_APP) $(RESOURCES)
	@echo Linking "$(TARGET_APP)"
	-$(V_AT)mkdir -p $(BINDIR)
	-$(V_AT)mkdir -p $(LIBDIR)
	-$(V_AT)mkdir -p $(OUTDIR)
	$(V_AT)$(CXX) -o $(OUTDIR)/$(TARGET_APP) $(OBJECTS_APP) \
		             $(LDFLAGS) $(LDFLAGS_APP) $(RESOURCES) \
					 $(TARGET_ARCH)

$(OBJECTS_APP) :
	-$(V_AT)mkdir -p $(OBJDIR)
	@echo "      Compiling: $(<F)"
	$(V_AT)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(CFLAGS) \
		-o "$@" -c "$<"

check-defs:
	@if [ -z "$(INSTALL_PATH)" ]; then \
        echo >&2 "Build failed, INSTALL_PATH not defined."; exit 1; \
    elif [ -z "$(PARENT_PATH)" ]; then \
        echo >&2 "Build failed, PARENT_PATH path not defined."; exit 1; \
    elif [ -z "$(KEY_LIMIT)" ]; then \
        echo >&2 "Build failed, KEY_LIMIT count not defined."; exit 1; \
    fi

install:
	killall $(TARGET_APP);\
	sudo cp build/$(CONFIG)/$(TARGET_APP) $(INSTALL_PATH); \
    sudo chown root.root $(INSTALL_PATH); \
    sudo chmod 4755 $(INSTALL_PATH)

debug:
	$(MAKE) CONFIG=Debug
	reset
	$(MAKE) install CONFIG=Debug
	gdb $(TARGET_APP)

release:
	$(MAKE) CONFIG=Release
	reset
	$(MAKE) install CONFIG=Release

clean:
	@echo Cleaning $(TARGET_APP)
	$(V_AT)$(CLEANCMD)

strip:
	@echo Stripping $(TARGET_APP)
	-$(V_AT)$(STRIP) --strip-unneeded $(OUTDIR)/$(TARGET)

uninstall:
	@echo Uninstalling $(TARGET_APP)
	killall $(TARGET_APP);\
	sudo rm $(INSTALL_PATH) ; \

help:
	@echo "$$HELPTEXT"

-include $(OBJECTS_APP:%.o=%.d)

$(OBJDIR)/Main.o: \
	Source/Main.cpp
$(OBJDIR)/InputReader.o: \
	Source/InputReader.cpp
$(OBJDIR)/KeyEventFiles.o: \
	Source/KeyEventFiles.cpp
$(OBJDIR)/KeyReader.o: \
	Source/KeyReader.cpp
$(OBJDIR)/KeyCode.o: \
	Source/KeyCode.cpp
$(OBJDIR)/PipeWriter.o: \
	Source/PipeWriter.cpp
$(OBJDIR)/Process_Data.o: \
	Source/Process/Process_Data.cpp
$(OBJDIR)/Process_State.o: \
	Source/Process/Process_State.cpp
$(OBJDIR)/Process_Security.o: \
	Source/Process/Process_Security.cpp
