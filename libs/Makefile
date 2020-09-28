LIBS :=
all: libs

ifdef CONFIG_IP
LIBS += build_libip
build_libip:
	$(MAKE) -C libip/
endif

ifdef CONFIG_ETHERNET
LIBS += build_libeth
build_libeth:
	$(MAKE) -C libeth/
endif

ifdef CONFIG_LIBMODBUS
LIBS += build_libmodbus
build_libmodbus:
	$(MAKE) -C libmodbus/
endif

libs: $(LIBS)
	@echo "LIBS = $(LIBS)"

clean:
	$(MAKE) -C libip/ clean
	$(MAKE) -C libeth/ clean

