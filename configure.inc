#!/bin/sh
# Copyright (c) 2015--2020 TK Chia
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

set -e
#
# Unset any existing variables whose names begin with `conf_'.
#
eval "`set | sed -n '/^conf_/ { s,[^[:alnum:]_].*,,g; s,^,unset ,; p; }'`"

conf__nl='
'

conf__blare() {
	echo "* $*" >&2
}

conf__blare_test() {
	echo -n "* $* ... " >&2
}

conf__blare_res() {
	echo "$*" >&2
}

conf__barf() {
	conf__blare "ERROR: $*"
	exit 1
}

conf__app_exists() {
	type "$1" >/dev/null 2>&1 || which "$1" >/dev/null 2>&1
}

conf__check_general_sanity() {
	local app
	for app in : awk basename chmod cp dirname echo grep ln make mv pwd \
	    rm strings seq test type; do
		if conf__app_exists "$app"; then :; else
			conf__barf "command line program \`$app' not found!"
		fi
	done
	if test ab = "`echo -n a; echo b`"; then :; else
		#
		# `bash' invoked as `sh' enters POSIX mode, which does not
		# recognize `-n'.  Try to test if we are in fact running
		# under `bash'.  If we are, disable POSIX mode.
		#
		if test -z "$BASH" || (unset SHELLOPTS 2>/dev/null); then
			conf__barf "\`echo' does not understand \`-n'!"; fi
		set +o posix
	fi
	if test _ = "`echo z | sed s/[[:alpha:]]/_/g`"; then :; else
		conf__barf \
		    "\`sed' does not understand POSIX character classes!"; fi
}

conf__mktemp() {
	local nm uniq
	if nm="`mktemp "$1".XXXXXXXXXX ${2+"--suffix=$2"} 2>/dev/null`"; then
		echo "$nm"
	else
		(
			set -C
			uniq=$$
			until : >"$1.$uniq$2" 2>/dev/null; do
				uniq="$uniq+"
				if test ${#uniq} -gt 1000; then
					conf__barf "cannot create" \
					    "temporary file" \
					    "with name $1.(...)$2!"
				fi
			done
			echo "$1.$uniq$2"
		)
	fi
}

conf__check_sane() {
	case "$1" in
	    *"$conf__nl"*)
		conf__barf "no newlines allowed in $2";;
	esac
}

conf__say() {
	local val
	if eval test xy = x"\${$1+y}"; then
		eval "val=\"\$$1\""
		echo "$1=$val" | \
		    sed 's,[^-_+/=.$[:alnum:][:space:]],\\&,g; s,\$,$$,g'
	fi
}

conf__say_single() {
	conf__say "$1=$2" | sed 's,[[:space:]],\\ ,g'
}

conf__blare_list() {
	local buf buf2 thang
	buf="   "
	for thang in $1; do
		buf2="$buf $thang"
		if test ${#buf2} -ge 63; then
			conf__blare "$buf"
			buf="    $thang"
		else
			buf="$buf2"
		fi
	done
	conf__blare "$buf"
}

conf__in_help_mode() {
	test help = "$conf_Mode"
}

conf__add_var() {
	local var
	for var in ${1+"$@"}; do
		case "$conf_Use_vars" in
		    *" $var "*)
			;;
		    *)	case "$var" in
			    conf_*)
				if conf__in_help_mode
				then :
				else conf_Use_vars="$conf_Use_vars$var "; fi;;
			    *)	conf_Use_vars="$conf_Use_vars$var ";;
			esac
			eval conf__check_sane \"\$$var\" \'\$$var\';;
		esac
	done
}

conf__help() {
	local me
	me="`basename $0`"
	conf__blare "usage:"
	conf__blare "    $me [--srcdir=(dir)] [--prefix=(dir)] \\"
	conf__blare "        [--host=(platform)] [--host-exec=[(runner)]] \\"
    if test yes = "$conf_Is_canadian_cross"; then
	conf__blare "        [--host-exe-ext=[(ext)] ` \
				`[--program-suffix=[(ext)] \\"
	conf__blare "        [--target-prefix=(dir)] [--target=(platform)] \\"
	conf__blare "        [--target-exec=[(runner)]] ` \
				`[--target-exe-ext=[(ext)]"
    else
	conf__blare "        [--host-exe-ext=[(ext)]"
    fi
	conf__blare
	conf__blare "environment variables used:"
	conf__blare_list "$conf_Use_vars"
	conf__blare
	conf__blare "build notes:"
    case "$conf_Use_vars" in
    *" CXX "* | *" CC "*)
    if test yes = "$conf_Is_canadian_cross"; then
	conf__blare "  * This project allows a \`Canadian cross' setup.  It"
	conf__blare "    uses a \`build' platform to build development tools "
	conf__blare "    for a \`host' platform, which can then create or"
	conf__blare "    deal with programs for a \`target' platform.  Host ="
	conf__blare "    build on default, and target = host on default."
	conf__blare
    fi
    set --
    case "$conf_Use_vars" in
    *" CC "*) set -- ${1+"$@"} '(platform)-gcc if CC is undefined';; esac
    case "$conf_Use_vars" in
    *" CXX "*) set -- ${1+"$@"} '(platform)-g++ if CXX is undefined';; esac
    if test $# -ne 0; then
	conf__blare "  * If you specify --host=(platform), \`$me' will"
	conf__blare "    look for"
    while test $# -gt 1; do
	conf__blare "    - $1"
	shift
    done
	conf__blare "    - $1."
	conf__blare
    fi
    set --
    case "$conf_Use_vars" in
    *" CC_FOR_TARGET "*)
    set -- ${1+"$@"} '(platform)-gcc if CC_FOR_TARGET is undefined';; esac
    case "$conf_Use_vars" in
    *" CXX_FOR_TARGET "*)
    set -- ${1+"$@"} '(platform)-g++ if CXX_FOR_TARGET is undefined';; esac
    if test $# -ne 0; then
	conf__blare "  * If you specify --target=(platform), \`$me' will"
	conf__blare "    look for"
    while test $# -gt 1; do
	conf__blare "    - $1"
	shift
    done
	conf__blare "    - $1."
	conf__blare
    fi
	conf__blare "  * Also:"
	conf__blare "    - If (platform) is i386-*, i486-*, i586-*, or"
	conf__blare "      i686-*, e.g. i386-linux-gnu, \`$me' will also try"
	conf__blare "      looking for a corresponding 64-bit compiler named"
	conf__blare "      e.g. x86_64-*-g++, and pass it -m32, -march= flags."
	conf__blare
	conf__blare "    - (platform) may be of the form (model),(arch).  Thus"
	conf__blare "      arm-*,armv6 says to run e.g. the arm-*-g++ compiler"
	conf__blare "      to produce armv6 code (arm-*-g++ -march=armv6)."
	conf__blare
	conf__blare "  * As implied above, instead of specifying --host=,"
	conf__blare "    you can directly provide compiler settings in"
    if test yes = "$conf_Is_canadian_cross"; then
	conf__blare "    environment variables.  Ditto for --target=."
    else
	conf__blare "    environment variables."
    fi
	conf__blare;;
    *)	;;
    esac
	conf__blare "  * --host-exec=(runner) specifies a command for running"
	conf__blare "    programs built for the host sytem.  If --host="
	conf__blare "    appears but not --host-exec, \`$me' will try to"
	conf__blare "    look for a corresponding Qemu user-mode emulator,"
	conf__blare "    e.g. \`qemu-arm'."
	conf__blare
	conf__blare "  * --host-exe-ext=(ext) or --program-suffix=(ext) gives"
	conf__blare "    a file name extension to use for compiled host-side"
	conf__blare "    programs, e.g. \`.exe'."
	conf__blare
    if test yes = "$conf_Is_canadian_cross"; then
	conf__blare "  * --target-exec=(runner) specifies a command for"
	conf__blare "    running programs built for the target system, while"
	conf__blare "    --target-exe-ext=(ext) gives a target-side extension."
	conf__blare
	conf__blare "  * The host installation directory (--prefix=) defaults"
	conf__blare "    to /usr/local.  \`$me' tries to derive something"
	conf__blare "    reasonable for the target installation directory"
	conf__blare "    unless --target-prefix= is given."
    else
	conf__blare "  * The host installation directory (--prefix=) defaults"
	conf__blare "    to /usr/local."
    fi
}

# Use some GNU compiler or other to find a good prefix (as in `/usr',
# `/usr/local', etc.) to use for host-/target-specific libraries.
#
conf__find_good_runtime_prefix() {
	local gnu gnuflags good_prefix prefix
	gnu="$1"
	gnuflags="$2"
	good_prefix=
	for arg in -print-file-name=libc.so \
		   -print-file-name=libc.a \
		   -print-libgcc-file-name \
		   -print-file-name=crt1.o
	do
		#
		# We cannot really rely on `realpath' or `readlink' being
		# available, so do this nasty thang...
		#
		prefix="`$gnu $gnuflags $arg`"
		prefix="`dirname "$prefix"`"
		prefix="`cd "$prefix" && pwd`"
		case "$prefix" in
		    */lib)
			prefix="`dirname "$prefix"`"
			if test -z "$good_prefix" -o \
			    ${#prefix} -lt ${#good_prefix}
				then good_prefix="$prefix"; fi;;
		esac
	done
	echo "$good_prefix"
}

conf__tweak_host_prefix() {
	conf__blare_test 'tweaking host installation directory'
	case "$conf_Prefix" in
	    /*)	;;
	    *)	conf_Prefix="`pwd`/$conf_Prefix";;
	esac
	conf__blare_res "$conf_Prefix"
	case "$conf_Prefix" in
	    *[,\;:\'\"]*)
		conf__blare "WARNING: directory path contains characters"
		conf__blare "WARNING: from the set [,;:\'\"]; this may cause"
		conf__blare "WARNING: problems later on";;
	esac
}

conf__tweak_target_prefix() {
	if test -n "$conf_Target_prefix"; then
		conf__blare_test 'tweaking target installation directory'
		case "$conf_Target_prefix" in
		    /*)	;;
		    *)	conf_Target_prefix="`pwd`/$conf_Target_prefix";;
		esac
	else
		conf__blare_test \
		    'deriving a good target installation directory'
		prefix="`conf__find_good_runtime_prefix \
		    "$conf_Host_exec $CXX_FOR_TARGET" "$CXXFLAGS_FOR_TARGET"`"
		case "$prefix" in
		    /usr/local | /usr | /local)
			conf_Target_prefix="$conf_Prefix";;
		    /usr/local/*)
			conf_Target_prefix="$conf_Prefix"/"`echo "$prefix" | \
			    sed s,^/usr/local/,,`";;
		    /usr/*)
			conf_Target_prefix="$conf_Prefix"/"`echo "$prefix" | \
			    sed s,^/usr/,,`";;
		    /local/*)
			conf_Target_prefix="$conf_Prefix"/"`echo "$prefix" | \
			    sed s,^/local/,,`";;
		    '')	if test -n "$conf_Crosst_tag"; then
				conf_Target_prefix="$conf_Prefix/` \
				    `$conf_Crosst_tag"
			else	conf_Target_prefix="$conf_Prefix"
			fi;;
		    *)	conf_Target_prefix="$prefix";;
		esac
	fi
	conf__blare_res "$conf_Target_prefix"
}

conf__escape() {
	local word
	for word in ${1+"$@"}; do
		echo "$word"
	done | awk -f "$conf_Lolwutconf_dir/escape.awk"
}

conf__not() {
	if ${1+"$@"}
	then	return 1
	else	return 0
	fi
}

conf_begin() {
	local opt var
	conf__check_general_sanity
	conf_Srcdir="`dirname "$0"`"
	if test -e "$conf_Srcdir/lolwutconf/configure.inc"; then
		conf_Lolwutconf_dir="`cd $conf_Srcdir/lolwutconf && pwd`"
	elif test -e "lolwutconf/configure.inc"; then
		conf_Lolwutconf_dir="`pwd`/lolwutconf"
	else
		conf__barf 'cannot find myself (configure.inc)!'
	fi
	conf_Prefix=/usr/local
	conf_Target_prefix=
	conf_Cross_tag=
	conf_Crosst_tag=
	unset conf_Host_exec
	unset conf_Host_exe_ext
	unset conf_Target_exec
	unset conf_Target_exe_ext
	conf_Use_vars=' SHELL '
	conf_Use_mods=' configure '
	conf_Mode=conf
	conf_Is_canadian_cross=no
	for opt in ${1+"$@"}; do
		conf__check_sane "$opt" arguments
		case "$opt" in
		    --srcdir=?*)
			conf_Srcdir="`echo "$opt" | sed s,^[^=]*=,,`";;
		    --prefix=?*)
			conf_Prefix="`echo "$opt" | sed s,^[^=]*=,,`";;
		    --host=?*)
			conf_Cross_tag="`echo "$opt" | sed s,^[^=]*=,,`";;
		    --host-exec=*)
			conf_Host_exec="`echo "$opt" | sed s,^[^=]*=,,`";;
		    --host-exe-ext=* | --program-suffix=*)
			conf_Host_exe_ext="`echo "$opt" | sed s,^[^=]*=,,`";;
		    --target-prefix=?*)
			conf_Target_prefix="`echo "$opt" | sed s,^[^=]*=,,`";;
		    --target=?*)
			conf_Crosst_tag="`echo "$opt" | sed s,^[^=]*=,,`";;
		    --target-exec=*)
			conf_Target_exec="`echo "$opt" | sed s,^[^=]*=,,`";;
		    --target-exe-ext=*)
			conf_Target_exe_ext="`echo "$opt" | sed s,^[^=]*=,,`";;
		    *)	conf_Mode=help;;
		esac
	done
	if conf__in_help_mode; then :; else
		conf_Use_vars="$conf_Use_vars` \
		    `conf_Srcdir conf_Lolwutconf_dir conf_Prefix ` \
		    `conf_Target_prefix conf_Cross_tag conf_Crosst_tag "; fi
}

conf_mod() {
	local mod
	for mod in ${1+"$@"}; do
		case "$conf_Use_mods" in
		    *" $mod "*)
			;;
		    *)
			if conf__in_help_mode; then :; else
				conf__blare \
				    "reading $conf_Lolwutconf_dir/$mod.inc"
			fi
			conf_Use_mods="$conf_Use_mods$mod "
			. "$conf_Lolwutconf_dir/$mod.inc";;
		esac
	done
}

conf_end() {
	local var
	if conf__in_help_mode; then
		conf__help
		exit 0
	fi
	conf__tweak_host_prefix
	if test yes = "$conf_Is_canadian_cross"
		then conf__tweak_target_prefix; fi
	conf__blare 'writing config.cache'
	: "${srcdir=`dirname "$0"`}"
	if test \! . = "$conf_Srcdir" -a \! . -ef "$conf_Srcdir"; then
		conf_Separate_build_dir=yes
	else
		conf_Separate_build_dir=no
	fi
	conf__add_var conf_Separate_build_dir
	tmp="`conf__mktemp config.cache`"
	(
		echo "###### AUTOMATICALLY GENERATED `date` ######"
		for var in $conf_Use_vars
			do conf__say $var; done
	) >"$tmp"
	mv "$tmp" config.cache
	if test yes = "$conf_Separate_build_dir"; then
		conf__blare "linking GNUmakefile to $conf_Srcdir/GNUmakefile"
		rm -f GNUmakefile
		if ln -s "$conf_Srcdir"/GNUmakefile GNUmakefile; then :; else
			conf__blare "...failed, copying instead"
			cp "$conf_Srcdir"/GNUmakefile GNUmakefile
		fi
	fi
}
