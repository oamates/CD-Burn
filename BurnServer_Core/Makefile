.PHONY: clean debug release install

all:	debug release


release:
	cd src && $(MAKE)

debug:
	cd src && DEBUG=-DDEBUG $(MAKE) debug

install:
	cd src && $(MAKE) install
	
clean:
	cd src && $(MAKE) clean
