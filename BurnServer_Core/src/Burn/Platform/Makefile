OBJS=Control.o Database.o DiskProc.o FileProc.o SetParam.o State.o StreamProc.o

TARGET=libburn.a

ifeq "$(GCC_VERSION)" "ARMGCC_Hi3531"
TARGET=libHIburn.a
endif

OWNLINK=cr
IFLAGS+=-I ../include

$(TARGET):$(OBJS)
	$(AR) $(OWNLINK) $(TARGET) $(OBJS)
	cp $(TARGET) $(LIB_INSTALL_DIR)


debug:$(OBJS)
	$(AR) $(OWNLINK) $(TARGET) $(OBJS)
	cp $(TARGET) $(LIB_INSTALL_DIR)

%.o:%.c
	$(CC)  $(CFLAGS) $(IFLAGS)  $(DFLAGS) -c $< -o $@


install:
	cp $(TARGET) $(DIR)/lib

clean:
	rm -f $(OBJS) $(TARGET) $(LIB_INSTALL_DIR)/$(TARGET)
