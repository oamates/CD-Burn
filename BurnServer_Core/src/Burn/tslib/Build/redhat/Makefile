#TSLib Makefile
#

LIB_NAME = libTSLib.a
LIB_PATH = ../../Bin/

COMPILER_FLAGS	+= -D_LINUX_ -DLINUX

COMPILER_FLAGS	+= -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D__USE_POSIX

COMPILER_FLAGS	+= -Wall -Wno-switch -Wpointer-arith -Wredundant-decls -pipe -ffast-math -fomit-frame-pointer

SHARED			= -shared
MODULE_LIBS		=

NAME	=	TSLib
C++		=	gcc
CC		=	gcc
LINK	=	gcc
CCFLAGS	+=	$(COMPILER_FLAGS)

# EACH DIRECTORY WITH HEADERS MUST BE APPENDED IN THIS MANNER TO THE CCFLAGS

CCFLAGS += -I.
CCFLAGS += -I..
CCFLAGS += -I../../Source/
CCFLAGS += -I../../Depends/TaskModelLib/

# OPTIMIZATION
CCFLAGS	+=  -g

C++FLAGS	= $(CCFLAGS)
LINKOPTS	= 
				
CPPFILES =  ../../Source/TSFile.cpp\
			../../Source/TSMedia.cpp\
			../../Source/ZMedia.cpp\
			../../Source/ZMediaTS.cpp\
			../../Source/ZMPEGFormat.cpp\
			../../Source/ZPESPacket.cpp\
			../../Source/ZPSITable.cpp\
			../../Source/ZTSPacket.cpp\

all: $(LIB_NAME)

$(LIB_NAME): $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
	ar -r $(LIB_PATH)$(LIB_NAME) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 
	ranlib $(LIB_PATH)$(LIB_NAME)

release: $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
	ar -r $(LIB_PATH)$(LIB_NAME) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 
	ranlib $(LIB_PATH)$(LIB_NAME)

clean:
	rm -f $(LIB_PATH)$(LIB_NAME) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

.SUFFIXES: .cpp .c .o

.cpp.o:
	$(C++) -c -o $*.o $(DEFINES) $(C++FLAGS) $*.cpp

.c.o:
	$(CC) -c -o $*.o $(DEFINES) $(CCFLAGS) $*.c


