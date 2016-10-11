#!/bin/sh

conf_mod c c.target

conf__bfd_chk_strip_with() {
	local prog sprog res
	prog="`conf__mktemp lolwutconf`"
	sprog="`conf__mktemp lolwutconf`"
	conf__ct_compile_prog "$prog" "int main() { return 0; }"
	if $conf_Host_exec $STRIP_FOR_TARGET ${1+"$@"} -o "$sprog" "$prog" \
	    >/dev/null 2>&1 && \
	   test -s "$sprog" && \
	   $conf_Target_exec ./"$sprog"
	then
		rm -f "$prog" "$sprog"
		return 0
	else
		res=$?
		rm -f "$prog" "$sprog"
		return $res
	fi
}

conf_bfd_probe_exists() {
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Have_bfd"
		then return 0; fi
	conf__add_var conf_Have_bfd
	conf_c_probe_typ 'struct bfd' '#include <bfd.h>'
	conf_c_probe_lib bfd 'bfd-multiarch bfd' \
	    '#include <bfd.h>' \
	    'void foo(void) { bfd_openr("/dev/null", "binary"); }'
	if test yes = "$conf_Have_c_typ_struct_bbfd" && \
	   test yes = "$conf_Have_c_lib_bfd"; then
		conf_Have_bfd=yes
	else
		conf_Have_bfd=no
	fi
}

conf_bfd_chk_exists() {
	conf_bfd_probe_exists ${1+"$@"}
	if conf__in_help_mode
		then return 0; fi
	if test yes = "$conf_Have_bfd"
		then return 0; fi
	conf__barf 'libbfd not installed!'
}

conf_bfd_chk_strip() {
	local cc save_ifs prog sprog
	if test yes = "$conf_Have_bfd_strip"
		then return 0; fi
	conf_c_chk_compiler_and_executor
	conf_ct_chk_compiler_and_executor
	conf__add_var STRIP_FOR_TARGET conf_Have_bfd_strip
	if conf__in_help_mode
		then return 0; fi
	conf__blare_test 'deciding program to remove symbols from' \
	    'target-side programs'
	if test -z "$STRIP_FOR_TARGET"; then
		case "$CC_FOR_TARGET" in
		    *-gcc | *-gcc-* | *-gcc' '* | \
		    *-clang | *-clang-* | *-clang' '* | \
		    *-cc | *-cc' '*)
			STRIP_FOR_TARGET="`echo "$CC_FOR_TARGET" | \
			    sed -e 's,^[[:space:]]*,,' \
				-e 's, .*$,,' \
				-e 's,-gcc$,-strip,' \
				-e 's,-gcc-.*$,-strip,' \
				-e 's,-clang$,-strip,' \
				-e 's,-clang-.*$,-strip,' \
				-e 's,-cc$,-strip,'`";;
		    *)	STRIP_FOR_TARGET=strip;;
		esac
	fi
	conf__blare_res "[$conf_Host_exec] $STRIP_FOR_TARGET"
	conf__blare_test 'checking if strip program works'
	if conf__bfd_chk_strip_with; then
		conf__blare_res yes
		conf_Have_bfd_strip=yes
	else
		conf__blare_res no
		conf__barf 'strip program does notwork!'
	fi
}

conf_bfd_strip_probe_opt_strip_unneeded() {
	conf_bfd_chk_strip
	conf__add_var conf_Have_bfd_strip_opt_strip_unneeded
	if test -n "$conf_Have_bfd_strip_opt_strip_unneeded"
		then return 0; fi
	conf__blare_test 'checking if strip program supports --strip-unneeded'
	if conf__bfd_chk_strip_with --strip-unneeded
	then	conf_Have_bfd_strip_opt_strip_unneeded=yes
	else	conf_Have_bfd_strip_opt_strip_unneeded=no
	fi
	conf__blare_res "$conf_Have_bfd_strip_opt_strip_unneeded"
}

conf_bfd_strip_probe_opt_strip_all() {
	conf_bfd_chk_strip
	conf__add_var conf_Have_bfd_strip_opt_strip_all
	if test -n "$conf_Have_bfd_strip_opt_strip_all"
		then return 0; fi
	conf__blare_test 'checking if strip program supports --strip-all'
	if conf__bfd_chk_strip_with --strip-all
	then	conf_Have_bfd_strip_opt_strip_all=yes
	else	conf_Have_bfd_strip_opt_strip_all=no
	fi
	conf__blare_res "$conf_Have_bfd_strip_opt_strip_all"
}
