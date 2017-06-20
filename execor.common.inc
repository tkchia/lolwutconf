#!/bin/sh

# Decide the file name extension to use for host-side/target-side programs.
#
conf__probe_exe_ext() {
	local plat cross_tag ext ext_var
	if test $# != 3
		then conf__barf 'lolwut?!? $% != 5'; fi
	if conf__in_help_mode
		then return 0; fi
	plat="$1"
	cross_tag="$2"
	ext_var="$3"
	if eval test -n \"\${$ext_var+set}\"
		then return 0; fi
	conf__blare_test "deciding program file name extension for $plat"
	case "$cross_tag" in
	    ia16-elf | ia16-elf,*)
		eval "$ext_var"=.exe;;
	    *)	eval "$ext_var"=;;
	esac
	eval conf__blare_res \"\${$ext_var:-\(blank\)}\"
}

# Find a command line that can execute the named program.  Then delete the
# program.
#
conf__find_execor() {
	local plat cross_tag prog ld_prefix execor_var \
	    execor execor1 execor2 save_ifs
	if test $# != 5
		then conf__barf 'lolwut?!? $# != 5'; fi
	plat="$1"
	cross_tag="$2"
	prog="$3"
	ld_prefix="$4"
	execor_var="$5"
	execor1=
	execor2=
	if eval test -n \"\$$execor_var\"; then
		rm "$prog"
		return 0
	elif ./"$prog" >/dev/null 2>&1; then
		# nothing to do -- no need for a separate program to run
		# host-side commands
		rm "$prog"
		eval "$execor_var"=
		return 0
	fi
	case "$cross_tag" in
	    i[3456]86-*)
		execor1=qemu-i386;;
	    armv[45678]* | arm-*)
		execor1=qemu-arm;;
	    aarch64-* | armeb-* | cris-* | m68k-* | microblaze-* | \
	    microblazeel-* | mips-* | mips64-* | mips64el-* | mipsel-* | \
	    mipsn32-* | mipsn32el-* | or32-* | ppc-* | ppc64-* | \
	    ppc64abi32-* | s390x-* | sh4-* | sh4eb-* | sparc-* | \
	    sparc32plus-* | sparc64-* | x86_64-*)
		# the above pattern is not really tested...
		save_ifs="$IFS"
		set '--' $cross_tag
		execor1="qemu-$1"
		IFS="$save_ifs";;
	    ia16-elf | ia16-elf,*)
		execor1="$conf_Lolwutconf_dir/dosemu-wrapper-1.sh"
		execor2="$conf_Lolwutconf_dir/dosemu-wrapper-2.sh";;
	    *)	# OK, we do not really know what to do here...
		;;
	esac
	if test -z "$execor2" -a -n "$ld_prefix"; then
		case "$execor1" in
		    qemu-*) execor2="$execor1 -L $ld_prefix";;
		esac
	fi
	for execor in "$execor1" "$execor2"; do
		if $execor ./"$prog" >/dev/null 2>&1; then
			rm "$prog"
			eval "$execor_var"='"$execor"'
			return 0
		fi
	done
	conf__barf "cannot find command line to run $plat-side program!"
}
