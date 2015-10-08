#Makefile for ChnsysRTSPLib example

COMPILER_FLAGS	+= -D_LINUX_ 
COMPILER_FLAGS	+= -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D__USE_POSIX
COMPILER_FLAGS	+= -Wall -Wno-switch -Wpointer-arith -Wredundant-decls -pipe -ffast-math -fomit-frame-pointer

C++FLAGS	+= $(COMPILER_FLAGS)
C++FLAGS	+= -I.

LIBPATH	= -L/var/tftpboot/ \


CORE_LINK_LIBS = -lpthread -lrt -ldl -lstdc++ -lm -lcrypt
		  
LIBS =  \
$(CORE_LINK_LIBS) \
-lChnsysRTSP \
-lTaskModel

LIBFILES = \
/var/tftpboot/libChnsysRTSP.so \
/var/tftpboot/libTaskModel.a

CPPFILES	=   ./TestRTSPServer.cpp \
				

TestRTSP: $(CPPFILES:.cpp=.o) $(LIBFILES)
	arm-hisiv200-linux-gcc -o /var/tftpboot/TestRTSPServer $(CPPFILES:.cpp=.o) $(C++FLAGS) $(LIBFILES) $(LIBPATH) $(LIBS)

clean:
	rm -f /var/tftpboot/TestRTSPServer $(CPPFILES:.cpp=.o)

.cpp.o:
	arm-hisiv200-linux-gcc -c -o $*.o $(C++FLAGS) $*.cpp

