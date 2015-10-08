# Makefile for NCXExampleClient

COMPILER_FLAGS	+= -D_LINUX_ -D_LINUX

COMPILER_FLAGS	+= -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D__USE_POSIX

COMPILER_FLAGS	+= -Wall -Wno-switch -Wpointer-arith -Wredundant-decls -march=pentium-m -mtune=pentium-m -pipe -ffast-math -fomit-frame-pointer

C++		=	g++
CC		=	gcc
CCFLAGS	+= $(COMPILER_FLAGS)  -g 
#CCFLAGS	+= $(COMPILER_FLAGS)
C++LINK = g++

# EACH DIRECTORY WITH HEADERS MUST BE APPENDED IN THIS MANNER TO THE CCFLAGS

CCFLAGS += -I./
CCFLAGS += -I../TaskModelLib/
CCFLAGS += -I../NCXDLL/

# DEPEND LIB PATH
CCFLAGS += -I../TaskModelLib/

C++FLAGS	= $(CCFLAGS)
				
CFILES		=	

CPPFILES	=	./main.cpp \


# LIBDPENDS
LIBFILES = 	\
../NCXDLL/libNCX.a \
../TaskModelLib/libTaskModel.a

CORE_LINK_LIBS = -lpthread -lrt -ldl -lstdc++ -lm -lcrypt

LIBS = \
$(CORE_LINK_LIBS) \
-lNCX \
-lTaskModel

LINKPATH = \
-L/usr/lib \
-L../NCXDLL/ \
-L../TaskModelLib/

	
NCXExampleClient: $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(LIBFILES)
	$(C++LINK) -o NCXExampleClient $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(C++FLAGS) $(LINKPATH) $(LIBS)

clean:
	rm -f NCXExampleClient $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

.cpp.o:
	$(C++) -c -o $*.o $(C++FLAGS) $*.cpp

.c.o:
	$(CC) -c -o $*.o $(CCFLAGS) $*.c
