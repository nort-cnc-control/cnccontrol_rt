LIBS :=

all: libs

LIBS += build_libshell
build_libshell:
	$(MAKE) -C libshell/

ifdef CONFIG_IP
LIBS += build_libip
build_libip:
	$(MAKE) -C libip/
endif

libs: $(LIBS)
	@echo "LIBS = $(LIBS)"

clean:
	$(MAKE) -C libip/ clean
	$(MAKE) -C libshell/ clean
