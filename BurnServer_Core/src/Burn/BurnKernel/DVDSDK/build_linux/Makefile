
# Name and type of the target for this Makefile
ifeq ($(debug),1)
 TARGETLIB := ../lib/debug/libDVDSDK.a
else
 TARGETLIB := ../lib/release/libDVDSDK.a
endif

CC = g++
AR = ar
RM = rm -f

INC_DIR += -I../include
INC_DIR += -I../lib
INC_DIR += -I/usr/include

SRC = ../src/LvDvdRecAPI.c \
	  ../src/cdr_cmd.c \
	  ../src/udffscore.c \
	  ../src/crc.c \
	  ../src/mkudffs.c \
	  ../src/defaults.c \
	  ../src/extent.c \
	  ../src/file.c \
	  ../src/memmalloc.c
#../src/unicode.c

SRCOBJ = $(SRC:.c=.o)

CFLAGS += $(INC_DIR)
CFLAGS += $(LIBS_DIR)
CFLAGS += -g
CFLAGS += -DLINUX
CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
#CFLAGS += -Wno-deprecated
#CFLAGS += -Wall -O2
ifeq ($(debug),1)
 CFLAGS += -DDEBUG -g
else 
 CFLAGS += -O2  
endif

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

all: $(TARGETLIB)

$(TARGETLIB):	$(SRCOBJ)
	$(AR) cr $(TARGETLIB) $?
	rm -f $(SRCOBJ)	
clean:
	rm -f $(SRCOBJ) $(TARGETLIB)


	
