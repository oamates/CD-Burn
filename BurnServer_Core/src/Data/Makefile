OBJS=Database.o Service.o Stream.o rtspts.o

TARGET=libstream.a

ifeq "$(GCC_VERSION)" "ARMGCC_Hi3531"
TARGET=libHIstream.a
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
	$(CC) $(CFLAGS) $(DFLAGS) $(IFLAGS) $(DFLAGS) -c $< -o $@


install:
	cp $(TARGET) $(DIR)/lib

clean:
	rm -f $(OBJS) $(TARGET) $(LIB_INSTALL_DIR)/$(TARGET)
