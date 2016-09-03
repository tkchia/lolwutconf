#!/bin/sh

conf_mod c++

conf_bfd_probe_exists() {
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Have_bfd"
		then return 0; fi
	conf__add_var conf_Have_bfd
	conf_cxx_probe_typ 'struct bfd' '#include <bfd.h>'
	conf_cxx_probe_lib bfd 'bfd-multiarch bfd' \
	    '#include <bfd.h>' \
	    'void foo() { bfd_openr("/dev/null", "binary"); }'
	if test yes = "$conf_Have_cxx_typ_struct_bbfd" && \
	   test yes = "$conf_Have_cxx_lib_bfd"; then
		conf_Have_bfd=yes
	else
		conf_Have_bfd=no
	fi
}

conf_bfd_chk_exists() {
	conf_bfd_probe_exists
	if conf__in_help_mode
		then return 0; fi
	if test yes = "$conf_Have_bfd"
		then return 0; fi
	conf__barf 'libbfd not installed!'
}
