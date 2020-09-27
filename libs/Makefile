LIBS :=
all: libs

ifdef CONFIG_IP
LIBS += libip.a
libip.a:
	$(MAKE) -C libip/
endif

ifdef CONFIG_ETHERNET
LIBS += libeth.a
libeth.a:
	$(MAKE) -C libeth/
endif


libs: $(LIBS)
	@echo "LIBS = $(LIBS)"

clean:
	$(MAKE) -C libip/ clean
	$(MAKE) -C libeth/ clean

