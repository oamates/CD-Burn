# TaskModel MakeFile
#  

LIB_NAME = libTaskModel.a
LIB_PATH = /rootfs/rtsp/

COMPILER_FLAGS	+= -D_LINUX_ 

COMPILER_FLAGS	+= -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D__USE_POSIX

COMPILER_FLAGS	+= -Wall -Wno-switch -Wpointer-arith -Wredundant-decls -pipe -ffast-math -fomit-frame-pointer

SHARED			= -shared
MODULE_LIBS		=

NAME	=	TaskModel
C++		=	arm-hisiv100-linux-gcc
CC		=	arm-hisiv100-linux-gcc
LINK	=	arm-hisiv100-linux-gcc
CCFLAGS	+=	$(COMPILER_FLAGS)

# EACH DIRECTORY WITH HEADERS MUST BE APPENDED IN THIS MANNER TO THE CCFLAGS

CCFLAGS += -I.
CCFLAGS += -I..
CCFLAGS += -I../../../Source/TaskModel/base/
CCFLAGS += -I../../../Source/TaskModel/public/
CCFLAGS += -I../../../Source/TaskModel/os/
CCFLAGS += -I../../../Source/TaskModel/math/
CCFLAGS += -I../../../Source/TaskModel/log/
CCFLAGS += -I../../../Source/TaskModel/xml/
CCFLAGS += -I../../../Source/TaskModel/task/
CCFLAGS += -I../../../Source/TaskModel/io/
CCFLAGS += -I../../../Source/TaskModel/socket/
CCFLAGS += -I../../../Source/TaskModel/clib/


# OPTIMIZATION
CCFLAGS	+=  -g

C++FLAGS	= $(CCFLAGS)
LINKOPTS	= 
				
CFILES		=	
CPPFILES	=	../../../Source/TaskModel/base/Common.cpp\
				../../../Source/TaskModel/base/ZBaseStream.cpp\
				../../../Source/TaskModel/base/ZBasePin.cpp\
				../../../Source/TaskModel/clib/ChnsysContainers.cpp\
				../../../Source/TaskModel/clib/ChnsysFiles.cpp\
				../../../Source/TaskModel/clib/ChnsysLog.cpp\
				../../../Source/TaskModel/clib/ChnsysSocket.cpp\
				../../../Source/TaskModel/clib/ChnsysTask.cpp\
				../../../Source/TaskModel/clib/ChnsysThread.cpp\
				../../../Source/TaskModel/clib/ChnsysTypes.cpp\
				../../../Source/TaskModel/clib/ChnsysUtil.cpp\
				../../../Source/TaskModel/clib/CLibTask.cpp\
				../../../Source/TaskModel/clib/CLibTCPListenerEvent.cpp\
				../../../Source/TaskModel/clib/CLibThread.cpp\
				../../../Source/TaskModel/math/ZMD5.cpp\
				../../../Source/TaskModel/math/ZMD5Digest.cpp\
				../../../Source/TaskModel/public/public.cpp\
				../../../Source/TaskModel/log/ZLog.cpp\
				../../../Source/TaskModel/log/ErrorCodeTable.cpp\
				../../../Source/TaskModel/xml/ZXMLParser.cpp\
				../../../Source/TaskModel/io/ZIOBase.cpp\
				../../../Source/TaskModel/io/ZDataPin.cpp\
				../../../Source/TaskModel/os/ZOS.cpp\
				../../../Source/TaskModel/os/ZOSCond.cpp\
				../../../Source/TaskModel/os/ZOSFile.cpp\
				../../../Source/TaskModel/os/ZOSHeap.cpp\
				../../../Source/TaskModel/os/ZOSMemory.cpp\
				../../../Source/TaskModel/os/ZOSMutex.cpp\
				../../../Source/TaskModel/os/ZOSObjectTable.cpp\
				../../../Source/TaskModel/os/ZOSQueue.cpp\
				../../../Source/TaskModel/os/ZOSSystem.cpp\
				../../../Source/TaskModel/os/ZOSThread.cpp\
				../../../Source/TaskModel/socket/ZSocket.cpp\
				../../../Source/TaskModel/socket/ZTCPListenerSocket.cpp\
				../../../Source/TaskModel/socket/ZTCPSocket.cpp\
				../../../Source/TaskModel/socket/ZUDPSocket.cpp\
				../../../Source/TaskModel/task/ZDemuxer.cpp\
				../../../Source/TaskModel/task/ZEvent.cpp\
				../../../Source/TaskModel/task/ZIdleTask.cpp\
				../../../Source/TaskModel/task/ZTask.cpp\
				../../../Source/TaskModel/task/ZTimeoutTask.cpp\
				../../../Source/TaskModel/TaskModel.cpp\


all: $(LIB_NAME)

$(LIB_NAME): $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
	arm-hisiv100-linux-ar -r $(LIB_PATH)$(LIB_NAME) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 
	arm-hisiv100-linux-ranlib $(LIB_PATH)$(LIB_NAME)

release: $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
	arm-hisiv100-linux-ar -r $(LIB_PATH)$(LIB_NAME) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 
	arm-hisiv100-linux-ranlib $(LIB_PATH)$(LIB_NAME)

clean:
	rm -f $(LIB_PATH)$(LIB_NAME) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

.SUFFIXES: .cpp .c .o

.cpp.o:
	$(C++) -c -o $*.o $(DEFINES) $(C++FLAGS) $*.cpp

.c.o:
	$(CC) -c -o $*.o $(DEFINES) $(CCFLAGS) $*.c

