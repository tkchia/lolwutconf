#!/bin/sh

conf_mod c

conf_efi_sdk_gnuefi() {
	local prog
	conf_c_chk_lib efi efi
	conf_c_chk_lib gnuefi gnuefi
	conf_c_probe_lib_path efi efi
	conf__add_var CPPFLAGS conf_Gnuefi_dir conf_Gnuefi_arch
	if conf__in_help_mode
		then return 0; fi
	conf_Gnuefi_dir="`dirname "$conf_Path_c_lib_efi"`"
	conf_Gnuefi_dir="`dirname "$conf_Gnuefi_dir"`"
	CPPFLAGS="-I$conf_Gnuefi_dir/include/efi ` \
		 `-I$conf_Gnuefi_dir/include/efi/protocol $CPPFLAGS"
	conf__blare_test "testing GNU EFI installation's output architecture"
	prog="`conf__mktemp lolwutconf "$conf_Host_exe_ext"`"
	if conf__c_compile_prog "$prog" \
	    "#include <stdio.h>" \
	    "int main(void) {" \
	    "#if defined __x86_64__" \
	    "  puts(\"x86_64\");" \
	    "#elif defined __i386__" \
	    "  puts(\"ia32\");" \
	    "#elif defined __ia64__" \
	    "  puts(\"ia64\");" \
	    "#elif defined __aarch64__" \
	    "  puts(\"aarch64\");" \
	    "#elif defined __arm__" \
	    "  puts(\"arm\");" \
	    "#elif defined __mips64 && \\" \
	    "	   __BYTE_ORDER__ - 0 == __ORDER_LITTLE_ENDIAN__ - 0" \
	    "  puts(\"mips64el\");" \
	    "#else" \
	    "# error" \
	    "#endif" \
	    "  return 0;" \
	    "}"
	then
		conf_Gnuefi_arch="`$conf_Host_exec ./"$prog"`"
		if test -n "$conf_Gnuefi_arch"; then
			CPPFLAGS="-I$conf_Gnuefi_dir/include/efi/` \
			    `$conf_Gnuefi_arch $CPPFLAGS"
		else
			conf_Gnuefi_arch=unknown
		fi
	else
		conf_Gnuefi_arch=unknown
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Gnuefi_arch"
	conf_c_find_cross_tool LD ld
	conf_c_find_cross_tool OBJCOPY objcopy
}
