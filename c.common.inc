#!/bin/sh

conf_mod execor.common

conf__cc_chk_compiler_with() {
	local cc cppflags cflags ldflags ldlibs execor exeext prog src res \
	    line
	cc="$1"
	cppflags="$2"
	cflags="$3"
	ldflags="$4"
	ldlibs="$5"
	execor="$6"
	exeext="$7"
	shift 7
	prog="`conf__mktemp lolwutconf "$exeext"`"
	src="$prog.c"
	(
		for line in ${1+"$@"}
			do echo "$line"; done
		echo 'int main(void) { return 0; }'
	) >"$src"
	res=0
	if $cc $cppflags $cflags $ldflags -o"$prog" "$src" $ldlibs \
	    >/dev/null 2>&1; then
		$execor ./"$prog" >/dev/null 2>&1
		res=$?
	else
		res=1
	fi
	rm -f "$prog" "$src"
	return $res
}

conf__cc_find_cross_compiler() {
	local cross_tag cc_var cflags_var cc cflags save_ifs arch
	if test $# != 3
		then conf__barf 'lolwut?!? $# != 3'; fi
	cross_tag="$1"
	cc_var="$2"
	cflags_var="$3"
	eval "cflags=\"\$$cflags_var\""
	case "$cross_tag" in
	    *,*)
		save_ifs="$IFS"
		IFS=,
		set '--' $cross_tag
		IFS="$save_ifs"
		if test 2 != $#; then
			conf__barf "bad platform specification $cross_tag"; fi
		cross_tag="$1"
		arch="$2"
		cc="${cross_tag}-gcc"
		cflags="-march=$arch $cflags";;
	    i[3456]86-* | pentium[^4]* | k6-* | athlon-[^f]* | winchip* | \
	    c3-* | geode-*)
		if conf__app_exists "${cross_tag}-gcc"; then
			cc="${cross_tag}-gcc"
		else
			save_ifs="$IFS"
			IFS=-
			set '--' $cross_tag
			case "$1-$2" in
			    pentium-mmx | pentium-m | k6-[23] | \
			    athlon-tbird | athlon-4 | athlon-[xm]p | \
			    winchip-c6 | c3-2)
				arch="$1-$2"
				shift 2;;
			    *)
				arch="$1"
				shift;;
			esac
			rest="$*"
			IFS="$save_ifs"
			if conf__app_exists "i686-$rest-gcc"; then
				cc="i686-$rest-gcc"
			elif conf__app_exists "i386-$rest-gcc"; then
				cc="i386-$rest-gcc"
			else
				cc="x86_64-$rest-gcc"
			fi
			cflags="-m32 -march=$arch $cflags"
		fi;;
	    *)	cc="${cross_tag}-gcc";;
	esac
	eval "$cc_var"='"$cc"'
	eval "$cflags_var"='"$cflags"'
}

conf__cc_find_cross_tool() {
	local cross_tag tool_var tool_base tool save_ifs arch
	if test $# != 3
		then conf__barf 'lolwut?!? $# != 3'; fi
	cross_tag="$1"
	tool_var="$2"
	tool_base="$3"
	case "$cross_tag" in
	    '')	tool="$tool_base";;
	    *,*)
		save_ifs="$IFS"
		IFS=,
		set '--' $cross_tag
		IFS="$save_ifs"
		if test 2 != $#; then
			conf__barf "bad platform specification $cross_tag"; fi
		cross_tag="$1"
		tool="${cross_tag}-${tool_base}";;
	    i[3456]86-* | pentium[^4]* | k6-* | athlon-[^f]* | winchip* | \
	    c3-* | geode-*)
		if conf__app_exists "${cross_tag}-${tool_base}"; then
			tool="${cross_tag}-${tool_base}"
		else
			save_ifs="$IFS"
			IFS=-
			set '--' $cross_tag
			case "$1-$2" in
			    pentium-mmx | pentium-m | k6-[23] | \
			    athlon-tbird | athlon-4 | athlon-[xm]p | \
			    winchip-c6 | c3-2)
				arch="$1-$2"
				shift 2;;
			    *)
				arch="$1"
				shift;;
			esac
			rest="$*"
			IFS="$save_ifs"
			if conf__app_exists "i686-$rest-$tool_base"; then
				tool="i686-$rest-$tool_base"
			elif conf__app_exists "i386-$rest-$tool_base"; then
				tool="i386-$rest-$tool_base"
			else
				tool="x86_64-$rest-$tool_base"
			fi
		fi;;
	    *)	tool="${cross_tag}-${tool_base}";;
	esac
	eval "$tool_var"='"$tool"'
}

conf__cc_find_execor() {
	local plat cross_tag cc cppflags cflags ldflags ldlibs exeext \
	    execor_var save_ifs ld_prefix
	if test $# != 9
		then conf__barf 'lolwut?!? $# != 9'; fi
	plat="$1"
	cross_tag="$2"
	cc="$3"
	cppflags="$4"
	cflags="$5"
	ldflags="$6"
	ldlibs="$7"
	exeext="$8"
	execor_var="$9"
	ld_prefix="`conf__find_good_runtime_prefix "$cc" "$cflags"`"
	prog="`conf__mktemp lolwutconf "$exeext"`"
	src="$prog.c"
	echo 'int main(void) { return 0; }' >"$src"
	res=0
	if $cc $cppflags $cflags $ldflags -o"$prog" "$src" $ldlibs \
	    >/dev/null 2>&1; then
		rm -f "$src"
		conf__find_execor "$plat" "$cross_tag" "$prog" "$ld_prefix" \
		    "$execor_var"
	else
		rm -f "$src"
		conf__barf 'C compiler does not work!'
	fi
}

conf__cc_probe_thumb() {
	local plat cc cppflags cflags_var ldflags ldlibs execor exeext cflags
	plat="$1"
	cc="$2"
	cppflags="$3"
	cflags_var="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	exeext="$8"
	shift 8
	eval "cflags=\"\$$cflags_var\""
	conf__blare_test "checking if Thumb mode is relevant for" \
	    "$plat-side C compiler"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" "$cflags" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" ${1+"$@"} \
	    '#ifdef __arm__' '#error' '#endif'
	then	conf__blare_res no
		return 0
	fi
	conf__blare_res yes
	conf__blare_test "checking if Thumb mode is already enabled for" \
	    "$plat-side C compiler"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" "$cflags" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" ${1+"$@"} \
	    '#if !defined __arm__ || !defined __thumb__' '#error' '#endif'
	then	conf__blare_res yes
		return 0
	fi
	conf__blare_res no
	conf__blare_test "checking command to enable Thumb mode for" \
	    "$plat-side C compiler"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" \
	    "$cflags -mthumb" "$ldflags" "$ldlibs" "$execor" "$exeext" \
	    ${1+"$@"} \
	    '#if !defined __arm__ || !defined __thumb__' '#error' '#endif'
	then	cflags="$cflags -mthumb"
		eval "$cflags_var"='"$cflags"'
		conf__blare_res "$cc $cppflags $cflags $ldflags"
	else	conf__blare_res '(failed; continuing)'
	fi
}

conf__cc_probe_lbss_fix() {
	local plat cc cppflags cflags ldflags_var ldlibs execor exeext \
	   ldflags goop
	plat="$1"
	cc="$2"
	cppflags="$3"
	cflags="$4"
	ldflags_var="$5"
	ldlibs="$6"
	execor="$7"
	exeext="$8"
	shift 8
	eval "ldflags=\"\$$ldflags_var\""
	conf__blare_test "checking if __lbss fix is needed for" \
	    "$plat-side C compiler"
	# See c++.common.inc .
	if conf__cc_chk_compiler_with "$cc" "$cppflags" "$cflags" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" ${1+"$@"} \
	    '#ifndef __ia16__' '#error' '#endif' \
	    '#include <stdlib.h>' \
	    'extern char __sbss[], __ebss[], __lbss[];' \
	    '__attribute__((constructor)) static void pre() {' \
	    '  char *p = __sbss + 2 * (__lbss - __sbss);' \
	    '  if (p != __ebss && p - 1 != __ebss) {' \
	    '    __asm volatile("int $0xe6" : : "a" (0xffffu), "b" (1));' \
	    '    abort();' \
	    '  }' \
	    '}'
	then
		conf__blare_res yes
		goop="-Wl,--wrap=__lbss,` \
		    `$conf_Lolwutconf_dir/ia16-elf-lbss-fix.ld"
		conf__blare_test "checking command to enable __lbss fix" \
		    "for $plat-side C compiler"
		if conf__cc_chk_compiler_with "$cc" "$cppflags" "$cflags" \
		    "$ldflags $goop" "$ldlibs" "$execor" "$exeext" ${1+"$@"}
		then	ldflags="$ldflags $goop"
			eval "$ldflags_var"='"$ldflags"'
			conf__blare_res "$cc $cppflags $cflags $ldflags"
		else	conf__blare_res '(failed; continuing)'
		fi
	else
		conf__blare_res no
		return 0
	fi
}

conf__cc_probe_addr_space() {
	local addr_sp plat cc cppflags cflags ldflags ldlibs execor exeext var
	addr_sp="$1"
	plat="$2"
	cc="$3"
	cppflags="$4"
	cflags="$5"
	ldflags="$6"
	ldlibs="$7"
	execor="$8"
	exeext="$9"
	shift 9
	var="$1"
	shift
	conf__blare_test "checking if $plat-side C compiler supports " \
	    "$addr_sp address space"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" "$cflags" "$ldflags" \
	    "$ldlibs" "$execor" "$exeext" ${1+"$@"} \
	    'typedef long $as *p_int_t;' \
	    'typedef short (*p_func_t)($as unsigned * $as);' \
	    'extern $as int some_var;' \
	    'extern char $as some_other_var;' \
	    'int f() { static volatile int $as x = 1; return x; }' \
	    'int g() { static $as volatile int y = 1; return y; }' && \
	   conf__not conf__cc_chk_compiler_with "$cc" "$cppflags" "$cflags" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" ${1+"$@"} \
	    'int f() { volatile int $as x = 1; return x; }' && \
	   conf__not conf__cc_chk_compiler_with "$cc" "$cppflags" "$cflags" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" ${1+"$@"} \
	    'int g() { $as volatile int y = 1; return y; }'
	then	eval $var=yes
	else	eval $var=no
	fi
	eval conf__blare_res \"\$var\"
}

conf__cc_probe_opt_wrapper() {
	local plat cc cppflags cflags ldflags ldlibs execor exeext var \
	    wrapper holder token got_token
	plat="$1"
	cc="$2"
	cppflags="$3"
	cflags="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	exeext="$8"
	var="$9"
	shift 9
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	conf__blare_test \
	    "checking if $plat-side C compiler supports -wrapper"
	wrapper="`conf__mktemp ./lolwutconf`"
	holder="`conf__mktemp lolwutconf`"
	token="$RANDOM"
	case "$token" in
	    *[^0123456789]*) token=;; esac
	token="w00tw00tw00t$token"
	(
		echo '#!/bin/sh'
		echo "echo $token >$holder"
		echo 'exec ${1+"$@"}'
	) >"$wrapper"
	chmod u+x "$wrapper"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" \
	    "$cflags -wrapper $wrapper" "$ldflags" "$ldlibs" "$execor" \
	    "$exeext" ${1+"$@"} && \
	   read got_token <"$holder" && \
	   test "$token" = "$got_token"; then
		eval $var=yes
	else
		eval $var=no
	fi
	rm -f "$wrapper" "$holder"
	eval conf__blare_res \"\$$var\"
}

conf__cc_probe_opt_fno_integrated_as() {
	local plat cc cppflags cflags ldflags ldlibs execor exeext var
	plat="$1"
	cc="$2"
	cppflags="$3"
	cflags="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	exeext="$8"
	var="$9"
	shift 9
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	conf__blare_test \
	    "checking if $plat-side C compiler supports -fno-integrated-as"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" \
	    "$cflags -fno-integrated-as" "$ldflags" "$ldlibs" "$execor" \
	    "$exeext" ${1+"$@"}; then
		eval $var=yes
	else
		eval $var=no
	fi
	eval conf__blare_res \"\$$var\"
}
