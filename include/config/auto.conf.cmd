deps_config := \
	Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
