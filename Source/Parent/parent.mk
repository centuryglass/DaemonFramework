## DaemonFramework Parent ##
# Adds source file targets used by daemon parent applications.

DF_PARENT_DIR = $(DF_ROOT_DIR)/Source/Parent
DF_PARENT_PREFIX = DaemonFramework_Parent_
DF_PARENT_OBJ = $(DF_OBJDIR)/$(DF_PARENT_PREFIX)

DF_OBJECTS_PARENT := \
  $(DF_PARENT_OBJ)DaemonControl.o

$(DF_PARENT_OBJ)DaemonControl.o: $(DF_PARENT_DIR)/DaemonControl.cpp
