OBJS=debug.o
TARGET=libdebug.a
OWNLINK= cr

$(TARGET):$(OBJS)
	$(AR) $(OWNLINK) $(TARGET) $(OBJS)
	cp $(TARGET) $(LIB_INSTALL_DIR)
%.o:%.c
	$(CC) $(DEBUG) $(DFLAGS) -DDEBUG $(IFLAGS) -c $< -o $@

debug:$(OBJS)
	$(AR) $(OWNLINK) $(TARGET) $(OBJS)
	cp $(TARGET) $(LIB_INSTALL_DIR)

install:
	cp $(TARGET) $(DIR)/lib

clean:
	rm -f $(OBJS) $(TARGET) $(LIB_INSTALL_DIR)/$(TARGET)
