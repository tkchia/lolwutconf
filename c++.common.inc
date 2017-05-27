#!/bin/sh

conf_mod execor.common

conf__cxxc_chk_compiler_with() {
	local cxx cppflags cxxflags ldflags ldlibs execor exeext prog src \
	    res line
	cxx="$1"
	cppflags="$2"
	cxxflags="$3"
	ldflags="$4"
	ldlibs="$5"
	execor="$6"
	exeext="$7"
	shift 7
	prog="`conf__mktemp lolwutconf "$exeext"`"
	src="$prog.cc"
	(
		for line in ${1+"$@"}
			do echo "$line"; done
		echo 'int main() { return 0; }'
	) >"$src"
	res=0
	if $cxx $cppflags $cxxflags $ldflags -o"$prog" "$src" $ldlibs \
	    >/dev/null 2>&1; then
		$execor ./"$prog" >/dev/null 2>&1
		res=$?
	else
		res=1
	fi
	rm -f "$prog" "$src"
	return $res
}

conf__cxxc_find_cross_compiler() {
	local cross_tag cxx_var cxxflags_var cxx cxxflags save_ifs arch
	if test $# != 3
		then conf__barf 'lolwut?!? $# != 3'; fi
	cross_tag="$1"
	cxx_var="$2"
	cxxflags_var="$3"
	eval "cxxflags=\"\$$cxxflags_var\""
	case "$cross_tag" in
	    i[3456]86-* | pentium[^4]* | k6-* | athlon-[^f]* | winchip* | \
	    c3-* | geode-*)
		if conf__app_exists "${cross_tag}-g++"; then
			cxx="${cross_tag}-g++"
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
			if conf__app_exists "i686-$rest-g++"; then
				cxx="i686-$rest-g++"
			elif conf__app_exists "i386-$rest-g++"; then
				cxx="i386-$rest-g++"
			else
				cxx="x86_64-$rest-g++"
			fi
			cxxflags="-m32 -march=$arch $cxxflags"
		fi;;
	    armv[45678]*)
		if conf__app_exists "${cross_tag}-g++"; then
			cxx="${cross_tag}-g++"
		else
			save_ifs="$IFS"
			IFS=-
			set '--' $cross_tag
			case "$2" in
			    m | a | r)
				arch="$1-$2"
				shift 2;;
			    *)
				arch="$1"
				shift;;
			esac
			rest="$*"
			IFS="$save_ifs"
			cxx="arm-$rest-g++"
			cxxflags="-march=$arch $cxxflags"
		fi;;
	    *)	cxx="${cross_tag}-g++";;
	esac
	eval "$cxx_var"='"$cxx"'
	eval "$cxxflags_var"='"$cxxflags"'
}

conf__cxxc_find_execor() {
	local plat cross_tag cxx cppflags cxxflags ldflags ldlibs exeext \
	    execor_var save_ifs ld_prefix
	if test $# != 9
		then conf__barf 'lolwut?!? $# != 9'; fi
	plat="$1"
	cross_tag="$2"
	cxx="$3"
	cppflags="$4"
	cxxflags="$5"
	ldflags="$6"
	ldlibs="$7"
	exeext="$8"
	execor_var="$9"
	ld_prefix="`conf__find_good_runtime_prefix "$cxx" "$cxxflags"`"
	prog="`conf__mktemp lolwutconf "$exeext"`"
	src="$prog.cc"
	echo 'int main() { return 0; }' >"$src"
	res=0
	if $cxx $cppflags $cxxflags $ldflags -o"$prog" "$src" $ldlibs \
	    >/dev/null 2>&1; then
		rm -f "$src"
		conf__find_execor "$plat" "$cross_tag" "$prog" "$ld_prefix" \
		    "$execor_var"
	else
		rm -f "$src"
		conf__barf 'C++ compiler does not work!'
	fi
}

conf__cxxc_probe_thumb() {
	local plat cxx cppflags cxxflags_var ldflags ldlibs execor exeext \
	    cxxflags
	plat="$1"
	cxx="$2"
	cppflags="$3"
	cxxflags_var="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	exeext="$8"
	shift 8
	eval "cxxflags=\"\$$cxxflags_var\""
	conf__blare_test "checking if Thumb mode is relevant for" \
	    "$plat-side C++ compiler"
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" \
	    '#ifdef __arm__' '#error' '#endif'
	then	conf__blare_res no
		return 0
	fi
	conf__blare_res yes
	conf__blare_test "checking if Thumb mode is already enabled for" \
	    "$plat-side C++ compiler"
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" \
	    '#if !defined __arm__ || !defined __thumb__' '#error' '#endif'
	then	conf__blare_res yes
		return 0
	fi
	conf__blare_res no
	conf__blare_test "checking command to enable Thumb mode for" \
	    "$plat-side C++ compiler"
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" \
	    "$cxxflags -mthumb" "$ldflags" "$ldlibs" "$execor" "$exeext" \
	    '#if !defined __arm__ || !defined __thumb__' '#error' '#endif'
	then	cxxflags="$cxxflags -mthumb"
		eval "$cxxflags_var"='"$cxxflags"'
		conf__blare_res "$cxx $cppflags $cxxflags $ldflags"
	else	conf__blare_res '(failed; continuing)'
	fi
}

conf__cxxc_probe_lbss_fix() {
	local plat cxx cppflags cxxflags ldflags_var ldlibs execor exeext \
	   ldflags goop
	plat="$1"
	cxx="$2"
	cppflags="$3"
	cxxflags="$4"
	ldflags_var="$5"
	ldlibs="$6"
	execor="$7"
	exeext="$8"
	shift 8
	eval "ldflags=\"\$$ldflags_var\""
	conf__blare_test "checking if __lbss fix is needed for" \
	    "$plat-side C++ compiler"
	#
	# For now, assume that the program will be run within dosemu, and use
	# the DOS_HELPER_REALLY_EXIT function (int $0xe6, %ax = $0xffff) to
	# get dosemu to return with an error status (in %bx).
	#
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" \
	    '#ifndef __ia16__' '#error' '#endif' \
	    '#include <cstdlib>' \
	    'extern "C"' \
	    '  { extern char __sbss[], __ebss[], __lbss[]; }' \
	    '__attribute__((constructor)) static void pre() {' \
	    '  char *p = __sbss + 2 * (__lbss - __sbss);' \
	    '  if (p != __ebss && p - 1 != __ebss) {' \
	    '    __asm volatile("int $0xe6" : : "a" (0xffffu), "b" (1));' \
	    '    std::abort();' \
	    '  }' \
	    '}'
	then
		conf__blare_res yes
		goop="-Wl,--wrap=__lbss,` \
		    `$conf_Lolwutconf_dir/ia16-elf-lbss-fix.ld"
		conf__blare_test "checking command to enable __lbss fix" \
		    "for $plat-side C++ compiler"
		if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags"\
		    "$ldflags $goop" "$ldlibs" "$execor" "$exeext"
		then	ldflags="$ldflags $goop"
			eval "$ldflags_var"='"$ldflags"'
			conf__blare_res "$cxx $cppflags $cxxflags $ldflags"
		else	conf__blare_res '(failed; continuing)'
		fi
	else
		conf__blare_res no
		return 0
	fi
}

conf__cxxc_probe_opt_wrapper() {
	local plat cxx cppflags cxxflags ldflags ldlibs execor exeext var \
	    wrapper holder token got_token
	plat="$1"
	cxx="$2"
	cppflags="$3"
	cxxflags="$4"
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
	    "checking if $plat-side C++ compiler supports -wrapper"
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
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" \
	    "$cxxflags -wrapper $wrapper" "$ldflags" "$ldlibs" "$execor" \
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

conf__cxxc_probe_opt() {
	local opt plat cxx cppflags cxxflags ldflags ldlibs execor exeext var
	opt="$1"
	plat="$2"
	cxx="$3"
	cppflags="$4"
	cxxflags="$5"
	ldflags="$6"
	ldlibs="$7"
	execor="$8"
	exeext="$9"
	shift 9
	var="$1"
	shift
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	conf__blare_test \
	    "checking if $plat-side C++ compiler supports $opt"
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags $opt" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" ${1+"$@"}; then
		eval $var=yes
	else
		eval $var=no
	fi
	eval conf__blare_res \"\$$var\"
}

conf__cxxc_probe_func() {
	local plat cxx cppflags cxxflags ldflags ldlibs execor exeext \
	    func n_args var templ proto args i
	plat="$1"
	cxx="$2"
	cppflags="$3"
	cxxflags="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	exeext="$8"
	func="$9"
	shift 9
	n_args="$1"
	var="$2"
	shift 2
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	conf__blare_test "testing for $plat-side C++ function $func"
	if test 0 = "$n_args"; then
		templ=
		proto=
		args=
	else
		templ='class T1'
		proto='T1 x1'
		args='x1'
		for i in `seq 2 "$n_args"`; do
			templ="$templ,class T$i"
			proto="$proto,T$i x$i"
			args="$args,x$i"
		done
		templ="template<$templ>"
	fi
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags" \
	    "$ldflags" "$ldlibs" "$execor" "$exeext" ${1+"$@"} \
	    "$templ void conf_foo($proto) { $func($args); }" \
	    "auto conf_bar = &$func;"
	then    eval $var=yes
	else    eval $var=no
	fi
	eval conf__blare_res \"\$$var\"
}
