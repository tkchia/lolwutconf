#!/bin/sh

conf_mod c.common c
conf_Is_canadian_cross=yes

conf__ct_chk_compiler_with() {
	local cflags ldlibs
	cflags="$1"
	ldlibs="$2"
	shift 2
	if conf__cc_chk_compiler_with "$conf_Host_exec $CC_FOR_TARGET" \
	    "$CPPFLAGS_FOR_TARGET" "$cflags" "$LDFLAGS_FOR_TARGET" \
	    "$ldlibs" "$conf_Target_exec" ${1+"$@"}
	then	return 0
	else	return $?
	fi
}

conf__ct_compile_prog() {
	local prog res
	conf_ct_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	prog="$1"
	shift
	src="$prog.c"
	(
		for line in ${1+"$@"}
			do echo "$line"; done
	) >"$src"
	res=0
	$conf_Host_exec $CC_FOR_TARGET $CPPFLAGS_FOR_TARGET \
	    $CFLAGS_FOR_TARGET $LDFLAGS_FOR_TARGET -o"$prog" "$src" \
	    $LDLIBS_FOR_TARGET >/dev/null 2>&1 || res=1
	rm -f "$src"
	return $res
}

conf_ct_chk_compiler_and_executor() {
	if test -n "$conf_Have_ct_compiler"
		then return 0; fi
	conf__add_var CC_FOR_TARGET CPPFLAGS_FOR_TARGET CFLAGS_FOR_TARGET \
	    LDFLAGS_FOR_TARGET LDLIBS_FOR_TARGET \
	    conf_Have_ct_compiler conf_Target_exec
	conf_c_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	try_thumb=no
	conf__blare_test 'deciding C compiler for target'
	if test -n "$conf_Crosst_tag" -a -z "$CC_FOR_TARGET"; then
		conf__cc_find_cross_compiler "$conf_Crosst_tag" \
		    CC_FOR_TARGET CFLAGS_FOR_TARGET
	elif test -z "$CC_FOR_TARGET"; then
		: "${CC_FOR_TARGET=$CC}"
		: "${CPPFLAGS_FOR_TARGET=$CPPFLAGS}"
		: "${CFLAGS_FOR_TARGET=$CFLAGS}"
		: "${LDFLAGS_FOR_TARGET=$LDFLAGS}"
		: "${LDLIBS_FOR_TARGET=$LDLIBS}"
	else
		: "${CFLAGS_FOR_TARGET=-O3}"
	fi
	conf__blare_res "[$conf_Host_exec] $CC_FOR_TARGET ` \
	    `$CPPFLAGS_FOR_TARGET $CFLAGS_FOR_TARGET"
	conf__blare_test 'deciding target program executor'
	if test set != "${conf_Target_exec+set}"; then
		conf__cc_find_execor target "$conf_Crosst_tag" \
		    "$conf_Host_exec $CC_FOR_TARGET" \
		    "$CPPFLAGS_FOR_TARGET" "$CFLAGS_FOR_TARGET" \
		    "$LDFLAGS_FOR_TARGET" "$LDLIBS_FOR_TARGET" \
		    conf_Target_exec
	fi
	conf__blare_res "${conf_Target_exec:-(blank)}"
	conf__blare_test \
	    'checking if C compiler and executor for target work'
	if conf__ct_chk_compiler_with "$CFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET"; then
		conf__blare_res yes
		conf_Have_ct_compiler=yes
	else
		conf__blare_res no
		conf__barf 'C compiler and/or executor does not work!'
	fi
	conf__cc_probe_thumb target "$conf_Host_exec $CC_FOR_TARGET" \
	    "$CPPFLAGS_FOR_TARGET" CFLAGS_FOR_TARGET \
	    "$LDFLAGS_FOR_TARGET" "$LDLIBS_FOR_TARGET" "$conf_Target_exec"
}

conf_ct_probe_opt_wrapper() {
	conf_ct_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	conf__cc_probe_opt_wrapper target \
	    "$conf_Host_exec $CC_FOR_TARGET" "$CPPFLAGS_FOR_TARGET" \
	    "$CFLAGS_FOR_TARGET" "$LDFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET" "$conf_Target_exec" \
	    conf_Have_ct_opt_wrapper
}

conf_ct_probe_opt_fno_integrated_as() {
	conf_ct_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	conf__cc_probe_opt_fno_integrated_as target \
	    "$conf_Host_exec $CC_FOR_TARGET" "$CPPFLAGS_FOR_TARGET" \
	    "$CFLAGS_FOR_TARGET" "$LDFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET" "$conf_Target_exec" \
	    conf_Have_ct_opt_fno_integrated_as
}

conf_ct_probe_typ() {
	local typ var
	conf_ct_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	typ="$1"
	shift
	var="conf_Have_ct_typ_`conf__escape "$typ"`"
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	conf__blare_test "testing for target-side C type $typ"
	if conf__ct_chk_compiler_with "$CFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET" ${1+"$@"} "typedef $typ conf_foo_t;"
	then	eval $var=yes
	else	eval $var=no
	fi
	eval conf__blare_res \"\$$var\"
}

conf_ct_probe_lib() {
	local family libs lib
	conf_ct_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	family="$1"
	libs="$2"
	shift 2
	var="conf_Have_ct_lib_`conf__escape "$family"`"
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	eval $var=no
	for lib in $libs; do
		conf__blare_test "testing for target-side C library -l$lib"
		if conf__ct_chk_compiler_with "$CFLAGS_FOR_TARGET" \
		    "-l$lib $LDLIBS_FOR_TARGET" ${1+"$@"}; then
			LDLIBS_FOR_TARGET="-l$lib $LDLIBS_FOR_TARGET"
			conf__blare_res yes
			eval $var=yes
			break
		else
			conf__blare_res no
		fi
	done
}

conf_ct_chk_typ() {
	local var typ
	typ="$1"
	conf_ct_probe_typ ${1+"$@"}
	if conf__in_help_mode
		then return 0; fi
	var="conf_Have_ct_typ_`conf__escape "$typ"`"
	if eval test yes = \"\$$var\"; then :; else
		conf__barf "type $1 not found target-side!"; fi
}

conf_ct_chk_typ_1_of() {
	local var typ foo
	test $# -ne 0
	if conf__in_help_mode
		then return 0; fi
	foo=no
	for typ in "$@"; do
		var="conf_Have_ct_typ_`conf__escape "$typ"`"
		if eval test yes = \"\$$var\"
			then foo=yes; fi
	done
	if test yes = "$foo"; then :; else
		foo="$1"
		shift
		for typ in ${1+"$@"}
			do foo="$foo, $typ"; done
		conf__barf "none of the types {$foo} are found target-side!"
	fi
}

conf_ct_probe_endianness() {
	local prog
	conf_ct_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Endianness_ct"
		then return 0; fi
	conf__add_var conf_Endianness_ct
	conf__blare_test "testing target-side byte order"
	prog="`conf__mktemp lolwutconf`"
	if conf__ct_compile_prog "$prog" \
	    "#include <cstring>" \
	    "#include <iostream>" \
	    "using namespace std;" \
	    "typedef unsigned char UC;" \
	    "typedef unsigned int UI;" \
	    "#define M  ((UI)1 + (UI)(UC)~(UC)0)" \
	    "static union { UC c[4]; UI i; } u;" \
	    "int main() {" \
	    "  memset(&u, 0, sizeof u);" \
	    "  u.i = (UC)'g' * M * M * M + (UC)'r' * M * M + \
		     (UC)'o' * M + (UC)'k';" \
	    "  if (memcmp(&u, \"grok\", 4) == 0)" \
	    "    cout << \"big-endian\" << endl;" \
	    "  else if (memcmp(&u, \"korg\", 4) == 0)" \
	    "    cout << \"little-endian\" << endl;" \
	    "  else if (memcmp(&u, \"rgko\", 4) == 0)" \
	    "    cout << \"pdp-endian\" << endl;" \
	    "  else" \
	    "    return 1;" \
	    "  return 0;" \
	    "}"
	then
		conf_Endianness_ct="`$conf_Target_exec ./"$prog"`"
		case "$conf_Endianness_ct" in
		    *-endian)	;;
		    *)		conf_Endianness_ct=unknown;;
		esac
	else
		conf_Endianness_ct=unknown
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Endianness_ct"
}

conf_ct_probe_dyn_ld() {
	local prog
	conf_ct_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Dyn_ld_ct"
		then return 0; fi
	conf__add_var conf_Dyn_ld_ct
	conf__blare_test "checking if target C compiler" \
	    "produces dynamically linked executables"
	# See c++.target.inc.
	if conf__ct_chk_compiler_with \
	    "$CFLAGS_FOR_TARGET -Wl,--dynamic-linker=/" "$LDLIBS_FOR_TARGET"
	then
		conf__blare_res no
		conf_Dyn_ld_ct=no
	elif conf__ct_chk_compiler_with "$CFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET -ldl" \
	    '#include <dlfcn.h>' \
	    'void *foo()' \
	    '  { return dlopen(0, RTLD_LAZY); }'
	then
		conf__blare_res yes
		conf_Dyn_ld_ct=yes
	else
		conf__blare_res unknown
		conf__barf 'cannot tell if C compiler produces' \
		    'dynamic or static programs!'
	fi
}


conf_ct_probe_asm_goto() {
	local prog
	conf_ct_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Have_asm_goto_ct"
		then return 0; fi
	conf__add_var conf_Have_asm_goto_ct
	conf__blare_test \
	    "checking if target supports __asm goto(...)"
	prog="`conf__mktemp lolwutconf`"
	if conf__ct_compile_prog "$prog" \
	    'int main() {' \
	    '  __asm goto("" : : : : foo);' \
	    '  return 0;' \
	    'foo:' \
	    '  return 1;' \
	    '}' && \
	    $conf_Target_exec ./"$prog" >/dev/null 2>&1
	then	conf_Have_asm_goto_ct=yes
	else	conf_Have_asm_goto_ct=no
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Have_asm_goto_ct"
}
