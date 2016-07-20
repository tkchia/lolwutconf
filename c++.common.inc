#!/bin/sh

conf__cxxc_chk_compiler_with() {
	local cxx cppflags cxxflags ldflags ldlibs execor prog src res line
	cxx="$1"
	cppflags="$2"
	cxxflags="$3"
	ldflags="$4"
	ldlibs="$5"
	execor="$6"
	shift 6
	prog="`conf__mktemp lolwutconf`"
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
	local cross_tag cxx cppflags cxxflags ldflags ldlibs execor_var \
	    execor save_ifs ld_prefix
	if test $# != 7
		then conf__barf 'lolwut?!? $# != 7'; fi
	cross_tag="$1"
	cxx="$2"
	cppflags="$3"
	cxxflags="$4"
	ldflags="$5"
	ldlibs="$6"
	execor_var="$7"
	execor=
	if test -z "$cross_tag"; then
		:	# nothing to do
	elif conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags" \
	    "$ldflags" "$ldlibs" ''; then
		:	# nothing to do too -- no need for a separate
			# program to run host-side commands
	else
		case "$cross_tag" in
		    i[3456]86-*)
			execor=qemu-i386;;
		    armv[45678]* | arm-*)
			execor=qemu-arm;;
		    aarch64-* | armeb-* | cris-* | m68k-* | \
		    microblaze-* | microblazeel-* | mips-* | \
		    mips64-* | mips64el-* | mipsel-* | mipsn32-* | \
		    mipsn32el-* | or32-* | ppc-* | ppc64-* | \
		    ppc64abi32-* | s390x-* | sh4-* | sh4eb-* | \
		    sparc-* | sparc32plus-* | sparc64-* | x86_64-*)
			# the above pattern is not really tested...
			save_ifs="$IFS"
			set '--' $cross_tag
			execor=qemu-"$1"
			IFS="$save_ifs";;
		    *)	# OK, we do not really know what to do here...
			;;
		esac
		if test -n "$execor"; then
			ld_prefix="`conf__find_good_runtime_prefix \
			    "$cxx" "$cxxflags"`"
			if test -n "$ld_prefix"
				then execor="$execor -L $ld_prefix"; fi
		fi
	fi
	eval "$execor_var"='"$execor"'
}

conf__cxxc_probe_thumb() {
	local plat cxx cppflags cxxflags_var ldflags ldlibs execor cxxflags
	plat="$1"
	cxx="$2"
	cppflags="$3"
	cxxflags_var="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	shift 7
	eval "cxxflags=\"\$$cxxflags_var\""
	conf__blare_test "checking if Thumb mode is relevant for $plat"
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags" \
	    "$ldflags" "$ldlibs" "$execor" \
	    '#ifdef __arm__' '#error' '#endif'
	then	conf__blare_res no
		return 0
	fi
	conf__blare_res yes
	conf__blare_test "checking if Thumb mode is already enabled for $plat"
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags" \
	    "$ldflags" "$ldlibs" "$execor" \
	    '#if !defined __arm__ || !defined __thumb__' '#error' '#endif'
	then	conf__blare_res yes
		return 0
	fi
	conf__blare_res no
	conf__blare_test "checking command to enable Thumb mode for $plat"
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" \
	    "$cxxflags -mthumb" "$ldflags" "$ldlibs" "$execor" \
	    '#if !defined __arm__ || !defined __thumb__' '#error' '#endif'
	then	cxxflags="$cxxflags -mthumb"
		eval "$cxxflags_var"='"$cxxflags"'
		conf__blare_res "$cxx $cppflags $cxxflags"
	else	conf__blare_res '(failed; continuing)'
	fi
}

conf__cxxc_probe_opt_wrapper() {
	local plat cxx cppflags cxxflags ldflags ldlibs execor var \
	    wrapper holder token got_token
	plat="$1"
	cxx="$2"
	cppflags="$3"
	cxxflags="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	var="$8"
	shift 8
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
	    ${1+"$@"} && \
	   read got_token <"$holder" && \
	   "$token" = "$got_token"; then
		eval $var=yes
	else
		eval $var=no
	fi
	rm -f "$wrapper" "$holder"
	eval conf__blare_res \"\$$var\"
}

conf__cxxc_probe_func() {
	local plat cxx cppflags cxxflags ldflags ldlibs execor func n_args \
	    var templ proto args i
	plat="$1"
	cxx="$2"
	cppflags="$3"
	cxxflags="$4"
	ldflags="$5"
	ldlibs="$6"
	execor="$7"
	func="$8"
	n_args="$9"
	shift 9
	var="$1"
	shift
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
	fi
	if conf__cxxc_chk_compiler_with "$cxx" "$cppflags" "$cxxflags" \
	    "$ldflags" "$ldlibs" "$execor" ${1+"$@"} \
	    "template<$templ> void conf_foo($proto) { $func($args); }"
	then    eval $var=yes
	else    eval $var=no
	fi
	eval conf__blare_res \"\$$var\"
}
