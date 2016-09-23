#!/bin/sh

# Find a command line that can execute the named program.  Then delete the
# program.
#
conf__find_execor() {
	local plat cross_tag prog ld_prefix execor_var execor save_ifs
	if test $# != 5
		then conf__barf 'lolwut?!? $# != 5'; fi
	plat="$1"
	cross_tag="$2"
	prog="$3"
	ld_prefix="$4"
	execor_var="$5"
	execor=
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
		execor=qemu-i386;;
	    armv[45678]* | arm-*)
		execor=qemu-arm;;
	    aarch64-* | armeb-* | cris-* | m68k-* | microblaze-* | \
	    microblazeel-* | mips-* | mips64-* | mips64el-* | mipsel-* | \
	    mipsn32-* | mipsn32el-* | or32-* | ppc-* | ppc64-* | \
	    ppc64abi32-* | s390x-* | sh4-* | sh4eb-* | sparc-* | \
	    sparc32plus-* | sparc64-* | x86_64-*)
		# the above pattern is not really tested...
		save_ifs="$IFS"
		set '--' $cross_tag
		execor=qemu-"$1"
		IFS="$save_ifs";;
	    *)	# OK, we do not really know what to do here...
		;;
	esac
	if $execor ./"$prog" >/dev/null 2>&1; then
		rm "$prog"
		eval "$execor_var"='"$execor"'
		return 0
	fi
	if test -n "$execor"; then
		if test -n "$ld_prefix"
			then execor="$execor -L $ld_prefix"; fi
		if $execor ./"$prog" >/dev/null 2>&1; then
			rm "$prog"
			eval "$execor_var"='"$execor"'
			return 0
		fi
	fi
	conf__barf "cannot find command line to run $plat-side program!"
}