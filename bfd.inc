#!/bin/sh

conf_mod c++

conf_bfd_chk_exists() {
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Have_bfd"
		then return 0; fi
	conf__add_var conf_Have_bfd
	conf_cxx_chk_typ 'struct bfd' '#include <bfd.h>'
	conf_cxx_chk_lib bfd 'bfd-multiarch bfd' \
	    '#include <bfd.h>' \
	    'void foo() { bfd_openr("/dev/null", "binary"); }'
	conf_Have_bfd=yes
}
