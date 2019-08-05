## DaemonFramework Parent ##
# Adds source file targets used by daemon parent applications. If defined,
# targets will apply a $(DAEMON_PREFIX) value so that parents may be compiled
# for multiple daemons.

DF_PARENT_DIR=$(DF_ROOT_DIR)/Source/Parent

$(DAEMON_PREFIX)DF_PARENT_PREFIX=DaemonFramework_$(DAEMON_PREFIX)Parent_

$(DAEMON_PREFIX)DF_PARENT_OBJ = \
        $($(DAEMON_PREFIX)DF_OBJDIR)/$($(DAEMON_PREFIX)DF_PARENT_PREFIX)

$(DAEMON_PREFIX)DF_OBJECTS_PARENT := \
        $($(DAEMON_PREFIX)DF_PARENT_OBJ)DaemonControl.o

$($(DAEMON_PREFIX)DF_PARENT_OBJ)DaemonControl.o: \
        $(DF_PARENT_DIR)/DaemonControl.cpp
