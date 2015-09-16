#CC=arm_v5t_le-gcc
#AR=arm_v5t_le-ar
#LD = arm_v5t_le-ld

CC=gcc
AR=ar
LD=ld
GCC_VERSION=GCC_4_1_2
#GCC_VERSION=GCC_4_4_5

BASE_DIR=$(PWD)
INSTALL_DIR=$(BASE_DIR)/../release

LIB_INSTALL_DIR=$(INSTALL_DIR)/lib
BIN_INSTALL_DIR=$(INSTALL_DIR)/bin
LFLAGS=-L $(LIB_INSTALL_DIR) -L $(LIB_INSTALL_DIR)/$(GCC_VERSION)
IFLAGS=-I $(BASE_DIR)/../include

DFLAGS=-DLINUX -D_GNU_SOURCE -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D__USE_POSIX -g
DFLAGS+=-Wall -Wno-switch -Wpointer-arith -Wredundant-decls -pipe -ffast-math -fomit-frame-pointer 

export CC LD BFLAGS DFLAGS IFLAGS LFLAGS LIB_INSTALL_DIR BIN_INSTALL_DIR

OWNLINK=-lstream -lburn -lRingBuf -lDVDSDK -lChnsysRTSPLib -lTSLib -lTaskModel -lpthread -lrt -lcrypt -lstdc++


TARGET=BurnServer

SUBDIRS=RingBuffer Burn/Platform/ Data 

OBJS= main.o 

define make_subdir
 @for subdir in $(SUBDIRS) ; do \
 ( cd $$subdir && make $1) \
 done;
endef

ifndef DIR
  DIR=$(INSTALL_DIR)
  export DIR
endif

all:$(OBJS)
	$(call make_subdir , ) 
	$(CC) $(OBJS) $(OWNLINK) $(LFLAGS) $(LIBFILES) $(DFLAGS) -o $(TARGET)
	cp $(TARGET) $(BIN_INSTALL_DIR)
%.o:%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(IFLAGS) -DDEBUG -c $< -o $@	
 
install :
	cp $(TARGET) $(DIR)/bin
	$(call make_subdir , install)
 
debug:$(OBJS)
	$(call make_subdir , debug)
	$(CC) $(OBJS) $(OWNLINK) $(LFLAGS) -o $(TARGET)
	cp $(TARGET) $(BIN_INSTALL_DIR)

clean:
	$(call make_subdir , clean) 
	rm -f $(OBJS) $(TARGET) $(BIN_INSTALL_DIR)/$(TARGET)
	
