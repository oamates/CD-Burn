
# Name and type of the target for this Makefile
TARGETLIB := ../lib/libDVDSDK.a

CC = arm-hisiv200-linux-gcc
AR = arm-hisiv200-linux-ar
#CC = gcc
#AR = ar
RM = rm -f

INC_DIR += -I../include
INC_DIR += -I../lib
#INC_DIR += -I/usr/include
#INC_DIR += /opt/hisi-linux/x86-arm/arm-hisiv200-linux/target/usr/include

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
CFLAGS += -DDEBUG -g3

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

all: $(TARGETLIB)

$(TARGETLIB):	$(SRCOBJ)
	$(AR) cr $(TARGETLIB) $?
	rm -f $(SRCOBJ)	
clean:
	rm -f $(SRCOBJ) $(TARGETLIB)


