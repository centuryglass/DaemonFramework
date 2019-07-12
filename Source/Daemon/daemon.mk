## DaemonFramework daemon ##
# Adds source file targets used by daemon programs.

DF_DAEMON_DIR = $(DF_ROOT_DIR)/Source/Daemon
DF_DAEMON_PREFIX = DaemonFramework_Daemon_
DF_DAEMON_OBJDIR := $(DF_OBJDIR)/$(DF_DAEMON_PREFIX)

DF_DAEMON_PROCESS_DIR = $(DF_OBJDIR)/Process
DF_DAEMON_PROCESS_PREFIX = $(DF_DAEMON_PREFIX)Process_
DF_DAEMON_PROCESS_OBJ = $(DF_DAEMON_OBJDIR)Process_
DF_OBJECTS_DAEMON_PROCESS := \
  $(DF_DAEMON_PROCESS_OBJ)State.o \
  $(DF_DAEMON_PROCESS_OBJ)Data.o \
  $(DF_DAEMON_PROCESS_OBJ)Security.o \

DF_OBJECTS_DAEMON := \
  $(DF_DAEMON_OBJDIR)DaemonLoop.o \
  $(DF_OBJECTS_DAEMON_PROCESS)

$(DF_DAEMON_PROCESS_OBJ)State.o: \
	$(DF_DAEMON_PROCESS_DIR)/Process_State.cpp
$(DF_DAEMON_PROCESS_OBJ)Data.o: \
	$(DF_DAEMON_PROCESS_DIR)/Process_Data.cpp
$(DF_DAEMON_PROCESS_OBJ)Security.o: \
	$(DF_DAEMON_PROCESS_DIR)/Process_Security.cpp
$(DF_DAEMON_OBJDIR)DaemonLoop.o: \
	$(DF_DAEMON_DIR)/DaemonLoop.cpp
