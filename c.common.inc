#!/bin/sh

conf_mod execor.common

conf__cc_chk_compiler_with() {
	local cc cppflags cflags ldflags ldlibs execor prog src res line
	cc="$1"
	cppflags="$2"
	cflags="$3"
	ldflags="$4"
	ldlibs="$5"
	execor="$6"
	shift 6
	prog="`conf__mktemp lolwutconf`"
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
	    armv[45678]*)
		if conf__app_exists "${cross_tag}-gcc"; then
			cc="${cross_tag}-gcc"
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
			cc="arm-$rest-gcc"
			cflags="-march=$arch $cflags"
		fi;;
	    *)	cc="${cross_tag}-gcc";;
	esac
	eval "$cc_var"='"$cc"'
	eval "$cflags_var"='"$cflags"'
}

conf__cc_find_execor() {
	local plat cross_tag cc cppflags cflags ldflags ldlibs execor_var \
	    save_ifs ld_prefix
	if test $# != 8
		then conf__barf 'lolwut?!? $# != 8'; fi
	plat="$1"
	cross_tag="$2"
	cc="$3"
	cppflags="$4"
	cflags="$5"
	ldflags="$6"
	ldlibs="$7"
	execor_var="$8"
	ld_prefix="`conf__find_good_runtime_prefix "$cc" "$cflags"`"
	prog="`conf__mktemp lolwutconf`"
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
	local plat cc cppflags cflags_var ldflags ldlibs execor cflags
	plat="$1"
	cc="$2"
	cppflags="$3"
	cflags_var="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	shift 7
	eval "cflags=\"\$$cflags_var\""
	conf__blare_test "checking if Thumb mode is relevant for" \
	    "$plat-side C compiler"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" "$cflags" \
	    "$ldflags" "$ldlibs" "$execor" \
	    '#ifdef __arm__' '#error' '#endif'
	then	conf__blare_res no
		return 0
	fi
	conf__blare_res yes
	conf__blare_test "checking if Thumb mode is already enabled for" \
	    "$plat-side C compiler"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" "$cflags" \
	    "$ldflags" "$ldlibs" "$execor" \
	    '#if !defined __arm__ || !defined __thumb__' '#error' '#endif'
	then	conf__blare_res yes
		return 0
	fi
	conf__blare_res no
	conf__blare_test "checking command to enable Thumb mode for" \
	    "$plat-side C compiler"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" \
	    "$cflags -mthumb" "$ldflags" "$ldlibs" "$execor" \
	    '#if !defined __arm__ || !defined __thumb__' '#error' '#endif'
	then	cflags="$cflags -mthumb"
		eval "$cflags_var"='"$cflags"'
		conf__blare_res "$cc $cppflags $cflags"
	else	conf__blare_res '(failed; continuing)'
	fi
}

conf__cc_probe_opt_wrapper() {
	local plat cc cppflags cflags ldflags ldlibs execor var \
	    wrapper holder token got_token
	plat="$1"
	cc="$2"
	cppflags="$3"
	cflags="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	var="$8"
	shift 8
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
	    ${1+"$@"} && \
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
	local plat cc cppflags cflags ldflags ldlibs execor var
	plat="$1"
	cc="$2"
	cppflags="$3"
	cflags="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	var="$8"
	shift 8
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	conf__blare_test \
	    "checking if $plat-side C compiler supports -fno-integrated-as"
	if conf__cc_chk_compiler_with "$cc" "$cppflags" \
	    "$cflags -fno-integrated-as" "$ldflags" "$ldlibs" "$execor" \
	    ${1+"$@"}; then
		eval $var=yes
	else
		eval $var=no
	fi
	eval conf__blare_res \"\$$var\"
}
