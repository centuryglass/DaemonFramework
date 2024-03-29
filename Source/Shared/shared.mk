## DaemonFramework Shared ##
# Adds source file targets shared by daemon and daemon parent applications.   

DF_SHARED_DIR = $(DF_ROOT_DIR)/Source/Shared
DF_SHARED_PREFIX = DaemonFramework_Shared_
DF_SHARED_OBJ := $(DF_OBJDIR)/$(DF_SHARED_PREFIX)

DF_SHARED_PIPE_DIR := $(DF_SHARED_DIR)/Pipe
DF_SHARED_PIPE_PREFIX := $(DF_SHARED_PREFIX)Pipe_
DF_SHARED_PIPE_OBJ := $(DF_SHARED_OBJ)Pipe_

DF_OBJECTS_SHARED_PIPE := \
  $(DF_SHARED_PIPE_OBJ)Pipe.o \
  $(DF_SHARED_PIPE_OBJ)Reader.o \
  $(DF_SHARED_PIPE_OBJ)Writer.o

DF_SHARED_FILE_DIR := $(DF_SHARED_DIR)/File
DF_SHARED_FILE_PREFIX := $(DF_SHARED_PREFIX)File_
DF_SHARED_FILE_OBJ := $(DF_SHARED_OBJ)File_

DF_OBJECTS_SHARED_FILE := \
  $(DF_SHARED_FILE_OBJ)Utils.o

DF_OBJECTS_SHARED := \
  $(DF_SHARED_OBJ)InputReader.o \
  $(DF_SHARED_OBJ)ThreadedInit.o \
  $(DF_OBJECTS_SHARED_FILE) \
  $(DF_OBJECTS_SHARED_PIPE)

$(DF_SHARED_OBJ)InputReader.o: \
	$(DF_SHARED_DIR)/InputReader.cpp
$(DF_SHARED_OBJ)ThreadedInit.o: \
	$(DF_SHARED_DIR)/ThreadedInit.cpp

$(DF_SHARED_PIPE_OBJ)Pipe.o: \
	$(DF_SHARED_PIPE_DIR)/Pipe.cpp
$(DF_SHARED_PIPE_OBJ)Reader.o: \
	$(DF_SHARED_PIPE_DIR)/Pipe_Reader.cpp
$(DF_SHARED_PIPE_OBJ)Writer.o: \
	$(DF_SHARED_PIPE_DIR)/Pipe_Writer.cpp

$(DF_SHARED_FILE_OBJ)Utils.o: \
	$(DF_SHARED_FILE_DIR)/File_Utils.cpp
