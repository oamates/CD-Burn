ifeq ($(debug),1)
 TARGETLIB := ../lib/debug/libuuid.a
else
 TARGETLIB := ../lib/release/libuuid.a
endif

CC      = gcc
AR      = ar
RM      = rm -f

INC_DIR  +=  -I../uuid
INC_DIR  +=  -I../../../../CommLib/include
INC_DIR  +=  -I../

SOURCES =  ../clear.c \
		   ../compare.c \
		   ../copy.c \
		   ../gen_uuid.c \
		   ../isnull.c \
		   ../pack.c \
		   ../parse.c \
		   ../tst_uuid.c \
		   ../unpack.c \
		   ../unparse.c \
		   ../uuid_time.c

OBJS =  $(SOURCES:.c=.o)

CFLAGS += -DLINUX -Wall -O2 
CFLAGS += $(INC_DIR)

ifeq ($(debug),1)
 CFLAGS += -g
endif

#
# rules
#
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

all: $(TARGETLIB)

$(TARGETLIB):	$(OBJS)
	$(AR) cqs $(TARGETLIB) $?
	rm -f $(OBJS)

clean:
	rm -f $(TARGETLIB)
	rm -f $(OBJS)
