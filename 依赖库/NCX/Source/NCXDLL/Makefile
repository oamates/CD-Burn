# NCX MakeFile
#  

LIB_NAME = libNCX.a
LIB_PATH = ./

COMPILER_FLAGS	+= -D_LINUX_ 

COMPILER_FLAGS	+= -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D__USE_POSIX

COMPILER_FLAGS	+= -Wall -Wno-switch -Wpointer-arith -Wredundant-decls -pipe -ffast-math -fomit-frame-pointer

SHARED			= -shared
MODULE_LIBS		=

NAME	=	NCX
C++		=	gcc
CC		=	gcc
LINK	=	gcc
CCFLAGS	+=	$(COMPILER_FLAGS)

# EACH DIRECTORY WITH HEADERS MUST BE APPENDED IN THIS MANNER TO THE CCFLAGS

CCFLAGS += -I.
CCFLAGS += -I../TaskModelLib/


# OPTIMIZATION
CCFLAGS	+=  -g

C++FLAGS	= $(CCFLAGS)
LINKOPTS	= 
				
CFILES		=	
CPPFILES	=	./ClientProcessor.cpp \
                ./NCX.cpp \
                ./NCXClient.cpp \
                ./NCXServer.cpp \
                ./TagUtillity.cpp \




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

