# Name and type of the target for this Makefile

APP_TARGET = dvdtest	

CC = gcc
AR = ar

RM = rm -f

INC_DIR += -I../include
INC_DIR += -I../src

LIBS_DIR = -L../lib

LIBS += -lDVDSDK  

SRC = main1.c
				
SRCOBJ = $(SRC:.c=.o)

CFLAGS += $(INC_DIR)
CFLAGS += $(LIBS_DIR)
CFLAGS += -DLINUX -DDEBUG -g
CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE 
#CFLAGS += -static

all:$(APP_TARGET)
  	$(APP_TARGET): $(SRCOBJ)
	$(CC) $(SRCOBJ) $(CFLAGS) $(LIBS) -g -o $(APP_TARGET) 
	rm -f $(SRCOBJ)
	
clean:
	rm -f $(SRCOBJ) $(APP_TARGET)

