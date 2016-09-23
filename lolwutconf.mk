VPATH = $(conf_Srcdir)

ifeq "$(abspath $(conf_Srcdir))" "$(abspath .)"
    CPPFLAGS := -Iinclude $(CPPFLAGS)
    CPPFLAGS_FOR_TARGET := -Iinclude $(CPPFLAGS_FOR_TARGET)
else
    CPPFLAGS := -Iinclude -I$(conf_Srcdir)/include $(CPPFLAGS)
    CPPFLAGS_FOR_TARGET := -Iinclude -I$(conf_Srcdir)/include \
	$(CPPFLAGS_FOR_TARGET)
endif

conf_Prefix ?= /usr/local
ifeq "." "$(conf_Srcdir)"
    conf_Separate_build_dir ?= no
else
    conf_Separate_build_dir ?= yes
endif
includedir = $(conf_Prefix)/include

.DEFAULT_GOAL := default

.PHONY: default all check install uninstall clean distclean
