#!/bin/sh
# Copyright (c) 2016--2020 TK Chia
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

conf_mod c.common

conf__c_chk_compiler_with() {
	local cflags ldlibs
	cflags="$1"
	ldlibs="$2"
	shift 2
	if conf__cc_chk_compiler_with "$CC" "$CPPFLAGS" "$cflags" \
	    "$LDFLAGS" "$ldlibs" "$conf_Host_exec" "$conf_Host_exe_ext" \
	    ${1+"$@"}
	then	return 0
	else	return $?
	fi
}

conf__c_compile_prog() {
	local prog res
	conf_c_chk_compiler_and_executor
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
	$CC $CPPFLAGS $CFLAGS $LDFLAGS -o"$prog" "$src" $LDLIBS \
	    >/dev/null 2>&1 || res=1
	rm -f "$src"
	return $res
}

conf_c_chk_compiler_and_executor() {
	local save_ifs arch rest ld_prefix
	if test -n "$conf_Have_c_compiler"
		then return 0; fi
	conf__add_var CC CPPFLAGS CFLAGS LDFLAGS LDLIBS conf_Have_c_compiler \
	    conf_Host_exec conf_Host_exe_ext
	if conf__in_help_mode
		then return 0; fi
	conf__probe_exe_ext host "$conf_Cross_tag" conf_Host_exe_ext
	conf__blare_test 'deciding C compiler for host'
	if test -n "$conf_Cross_tag" -a -z "$CC"; then
		conf__cc_find_cross_compiler "$conf_Cross_tag" CC CFLAGS
	else
		: "${CC=gcc}"
		: "${CFLAGS=-O3}"
	fi
	conf__blare_res "$CC $CPPFLAGS $CFLAGS"
	conf__blare_test 'deciding host program executor'
	if test set != "${conf_Host_exec+set}"; then
		conf__cc_find_execor host "$conf_Cross_tag" \
		    "$CC" "$CPPFLAGS" "$CFLAGS" "$LDFLAGS" "$LDLIBS" \
		    "$conf_Host_exe_ext" conf_Host_exec
	fi
	conf__blare_res "${conf_Host_exec:-(blank)}"
	conf__blare_test 'checking if C compiler and executor for host work'
	if conf__c_chk_compiler_with "$CFLAGS" "$LDLIBS"; then
		conf__blare_res yes
		conf_Have_c_compiler=yes
	else
		conf__blare_res no
		conf__barf 'C compiler and/or executor does not work!'
	fi
	conf__cc_probe_thumb host "$CC" "$CPPFLAGS" CFLAGS "$LDFLAGS" \
	    "$LDLIBS" "$conf_Host_exec" "$conf_Host_exe_ext"
	conf__cc_probe_lbss_fix host "$CC" "$CPPFLAGS" "$CFLAGS" LDFLAGS \
	    "$LDLIBS" "$conf_Host_exec" "$conf_Host_exe_ext"
}

conf_c_probe_opt_fno_integrated_as() {
	conf_c_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	conf__cc_probe_opt_fno_integrated_as host "$CC" "$CPPFLAGS" \
	    "$CFLAGS" "$LDFLAGS" "$LDLIBS" "$conf_Host_exec" \
	    "$conf_Host_exe_ext" conf_Have_c_opt_fno_integrated_as
}

conf_c_probe_typ() {
	local typ var yn
	conf_c_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	typ="$1"
	shift
	var="conf_Have_c_typ_`conf__escape "$typ"`"
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	conf__blare_test "testing for host-side C type $typ"
	if conf__c_chk_compiler_with "$CFLAGS" "$LDLIBS" ${1+"$@"} \
	    "typedef $typ conf_foo_t;"
	then	yn=yes
	else	yn=no
	fi
	eval $var=\$yn
	conf__blare_res $yn
}

conf_c_chk_typ() {
	local var typ
	typ="$1"
	conf_c_probe_typ ${1+"$@"}
	if conf__in_help_mode
		then return 0; fi
	var="conf_Have_c_typ_`conf__escape "$typ"`"
	if eval test yes = \"\$$var\"; then :; else
		conf__barf "type $1 not found host-side!"; fi
}

conf_c_chk_typ_1_of() {
	local var typ foo
	test $# -ne 0
	if conf__in_help_mode
		then return 0; fi
	foo=no
	for typ in "$@"; do
		var="conf_Have_c_typ_`conf__escape "$typ"`"
		if eval test yes = \"\$$var\"
			then foo=yes; fi
	done
	if test yes = "$foo"; then :; else
		foo="$1"
		shift
		for typ in ${1+"$@"}
			do foo="$foo, $typ"; done
		conf__barf "none of the types {$foo} are found host-side!"
	fi
}

conf_c_probe_asm_goto() {
	local prog
	conf_c_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Have_asm_goto_c"
		then return 0; fi
	conf__add_var conf_Have_asm_goto_c
	conf__blare_test \
	    "checking if host supports __asm goto(...)"
	prog="`conf__mktemp lolwutconf "$conf_Host_exe_ext"`"
	if conf__c_compile_prog "$prog" \
	    'int main(void) {' \
	    '  __asm goto("" : : : : foo);' \
	    '  return 0;' \
	    'foo:' \
	    '  return 1;' \
	    '}' && \
	    $conf_Host_exec ./"$prog"
	then    conf_Have_asm_goto_c=yes
	else    conf_Have_asm_goto_c=no
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Have_asm_goto_c"
}

conf_c_probe_func() {
	local func n_args var
	conf_c_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	func="$1"
	n_args="$2"
	shift 2
	var="conf_Have_c_func_`conf__escape "$func"`"
	conf__cc_probe_func host "$CC" "$CPPFLAGS" "$CFLAGS" \
	    "$LDFLAGS" "$LDLIBS" "$conf_Host_exec" "$conf_Host_exe_ext" \
	    "$func" "$n_args" "$var" ${1+"$@"}
}

conf_c_probe_lib() {
	local family libs lib
	conf_c_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	family="$1"
	libs="$2"
	shift 2
	var="conf_Have_c_lib_`conf__escape "$family"`"
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	eval $var=no
	for lib in $libs; do
		conf__blare_test "testing for host-side C library -l$lib"
		if conf__c_chk_compiler_with "$CFLAGS" "-l$lib $LDLIBS" \
		    ${1+"$@"}; then
			LDLIBS="-l$lib $LDLIBS"
			conf__blare_res yes
			eval $var=yes
			break
		else
			conf__blare_res no
		fi
	done
}

conf_c_probe_lib_path() {
	local family libs lib file path base dir foo
	conf_c_probe_lib ${1+"$@"}
	if conf__in_help_mode
		then return 0; fi
	family="$1"
	libs="$2"
	var="conf_Have_c_lib_`conf__escape "$family"`"
	if eval test yes = \"\$$var\"; then
		var="conf_Path_c_lib_`conf__escape "$family"`"
		if eval test -n \"\$$var\"
			then return 0; fi
		conf__add_var "$var"
		path=
		foo=
		for lib in $libs; do
			if test -n "$path"
				then continue; fi
			if test -n "$foo"
			then	foo="$foo, $lib"
			else	foo="$lib"
			fi
			conf__blare_test "finding path of" \
					 "host-side C library -l$lib"
			for file in lib"$lib".so lib"$lib".a; do
				if test -n "$path"
					then continue; fi
				path="`$CC $CPPFLAGS $CFLAGS $LDFLAGS $LDLIBS \
				    -print-file-name="$file"`"
				case "$path" in
				    */*)
					base="`basename "$path"`"
					dir="`dirname "$path"`"
					dir="`cd "$dir" && pwd`"
					case "$dir" in
					    */)  path="$dir$base";;
					    */*) path="$dir/$base";;
					    *)	 path==;;
					esac;;
				    *)	path=;;
				esac
			done
			conf__blare_res "${path:-(not found)}"
		done
		if test -z "$path"; then
			conf__barf "cannot figure out path of" \
				   "any of the libraries {$foo}!"
		fi
		eval $var='"$path"'
	fi
}

conf_c_chk_lib() {
	local family libs foo lib
	family="$1"
	libs="$2"
	conf_c_probe_lib "$@"
	if conf__in_help_mode
		then return 0; fi
	var="conf_Have_c_lib_`conf__escape "$family"`"
	if eval test yes = \"\$$var\"; then :; else
		set -- $libs
		foo="$1"
		shift
		for lib in ${1+"$@"}
			do foo="$foo, $lib"; done
		conf__barf "none of the libraries {$foo} are found host-side!"
	fi
}

conf_c_probe_endianness() {
	local prog
	conf_c_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Endianness_c"
		then return 0; fi
	conf__add_var conf_Endianness_c
	conf__blare_test "testing host-side byte order"
	prog="`conf__mktemp lolwutconf "$conf_Host_exe_ext"`"
	if conf__c_compile_prog "$prog" \
	    "#include <stdio.h>" \
	    "#include <string.h>" \
	    "typedef unsigned char UC;" \
	    "#ifdef __ia16__" \
	    "typedef unsigned long UI;" \
	    "#else" \
	    "typedef unsigned int UI;" \
	    "#endif" \
	    "#define M  ((UI)1 + (UI)(UC)~(UC)0)" \
	    "static union { UC c[4]; UI i; } u;" \
	    "int main(void) {" \
	    "  memset(&u, 0, sizeof u);" \
	    "  u.i = (UC)'g' * M * M * M + (UC)'r' * M * M + \
		     (UC)'o' * M + (UC)'k';" \
	    "  if (memcmp(&u, \"grok\", 4) == 0)" \
	    "    puts(\"big-endian\");" \
	    "  else if (memcmp(&u, \"korg\", 4) == 0)" \
	    "    puts(\"little-endian\");" \
	    "  else if (memcmp(&u, \"rgko\", 4) == 0)" \
	    "    puts(\"pdp-endian\");" \
	    "  else" \
	    "    return 1;" \
	    "  return 0;" \
	    "}"
	then
		conf_Endianness_c="`$conf_Host_exec ./"$prog"`"
		case "$conf_Endianness_c" in
		    *-endian)	;;
		    *)		conf_Endianness_c=unknown;;
		esac
	else
		conf_Endianness_c=unknown
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Endianness_c"
}

conf_c_probe_macro() {
	local macro var
	conf_c_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	macro="$1"
	shift
	var="conf_Have_c_macro_`conf__escape "$macro"`"
	conf__add_var "$var"
	conf__blare_test "testing for host-side C macro $macro"
	if conf__c_chk_compiler_with "$CFLAGS" "$LDLIBS" ${1+"$@"} \
	    "#ifndef $macro" "#error" "#endif"
	then    eval $var=yes
	else    eval $var=no
	fi
	eval conf__blare_res \"\$$var\"
}

conf_c_chk_macro() {
	local var macro
	macro="$1"
	conf_c_probe_macro ${1+"$@"}
	if conf__in_help_mode
		then return 0; fi
	var="conf_Have_c_macro_`conf__escape "$macro"`"
	if eval test yes = \"\$$var\"; then :; else
		conf__barf "macro $1 not found host-side!"; fi
}

conf_c_find_cross_tool() {
	local tool_var tool_base
	tool_var="$1"
	tool_base="$2"
	conf__add_var "$tool_var"
	if conf__in_help_mode
		then return 0; fi
	if eval test -z \"\$$tool_var\"; then
		conf__blare_test "deciding build $tool_base tool"
		conf__cc_find_cross_tool "$conf_Cross_tag" "$tool_var" \
		    "$tool_base"
		eval conf__blare_res \"\$$tool_var\"
		eval conf_appb_chk_exists \"\$$tool_var\"
	fi
}
