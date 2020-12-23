# Copyright (c) 2015--2016 TK Chia
#
# This file is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.

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
