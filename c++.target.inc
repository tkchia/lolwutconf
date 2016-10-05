#!/bin/sh

conf_mod c++.common c++
conf_Is_canadian_cross=yes

conf__cxxt_chk_compiler_with() {
	local cxxflags ldlibs
	cxxflags="$1"
	ldlibs="$2"
	shift 2
	if conf__cxxc_chk_compiler_with "$conf_Host_exec $CXX_FOR_TARGET" \
	    "$CPPFLAGS_FOR_TARGET" "$cxxflags" "$LDFLAGS_FOR_TARGET" \
	    "$ldlibs" "$conf_Target_exec" ${1+"$@"}
	then	return 0
	else	return $?
	fi
}

conf__cxxt_compile_prog() {
	local prog res
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	prog="$1"
	shift
	src="$prog.cc"
	(
		for line in ${1+"$@"}
			do echo "$line"; done
	) >"$src"
	res=0
	$conf_Host_exec $CXX_FOR_TARGET $CPPFLAGS_FOR_TARGET \
	    $CXXFLAGS_FOR_TARGET $LDFLAGS_FOR_TARGET -o"$prog" "$src" \
	    $LDLIBS_FOR_TARGET >/dev/null 2>&1 || res=1
	rm -f "$src"
	return $res
}

conf_cxxt_chk_compiler_and_executor() {
	local
	if test -n "$conf_Have_cxxt_compiler"
		then return 0; fi
	conf__add_var CXX_FOR_TARGET CPPFLAGS_FOR_TARGET CXXFLAGS_FOR_TARGET \
	    LDFLAGS_FOR_TARGET LDLIBS_FOR_TARGET \
	    conf_Have_cxxt_compiler conf_Target_exec
	conf_cxx_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	try_thumb=no
	conf__blare_test 'deciding C++ compiler for target'
	if test -n "$conf_Crosst_tag" -a -z "$CXX_FOR_TARGET"; then
		conf__cxxc_find_cross_compiler "$conf_Crosst_tag" \
		    CXX_FOR_TARGET CXXFLAGS_FOR_TARGET
	elif test -z "$CXX_FOR_TARGET"; then
		: "${CXX_FOR_TARGET=$CXX}"
		: "${CPPFLAGS_FOR_TARGET=$CPPFLAGS}"
		: "${CXXFLAGS_FOR_TARGET=$CXXFLAGS}"
		: "${LDFLAGS_FOR_TARGET=$LDFLAGS}"
		: "${LDLIBS_FOR_TARGET=$LDLIBS}"
	else
		: "${CXXFLAGS_FOR_TARGET=-O3}"
	fi
	conf__blare_res "[$conf_Host_exec] $CXX_FOR_TARGET ` \
	    `$CPPFLAGS_FOR_TARGET $CXXFLAGS_FOR_TARGET"
	conf__blare_test 'deciding target program executor'
	if test set != "${conf_Target_exec+set}"; then
		conf__cxxc_find_execor target "$conf_Crosst_tag" \
		    "$conf_Host_exec $CXX_FOR_TARGET" \
		    "$CPPFLAGS_FOR_TARGET" "$CXXFLAGS_FOR_TARGET" \
		    "$LDFLAGS_FOR_TARGET" "$LDLIBS_FOR_TARGET" \
		    conf_Target_exec
	fi
	conf__blare_res "${conf_Target_exec:-(blank)}"
	conf__blare_test \
	    'checking if C++ compiler and executor for target work'
	if conf__cxxt_chk_compiler_with "$CXXFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET"; then
		conf__blare_res yes
		conf_Have_cxxt_compiler=yes
	else
		conf__blare_res no
		conf__barf 'C++ compiler and/or executor does not work!'
	fi
	conf__cxxc_probe_thumb target "$conf_Host_exec $CXX_FOR_TARGET" \
	    "$CPPFLAGS_FOR_TARGET" CXXFLAGS_FOR_TARGET \
	    "$LDFLAGS_FOR_TARGET" "$LDLIBS_FOR_TARGET" "$conf_Target_exec"
}

conf_cxxt_probe_cxx11() {
	local extra
	conf_cxxt_chk_compiler_and_executor
	if test -n "$conf_Have_cxxt_cxx11_compiler"
		then return 0; fi
	if conf__in_help_mode
		then return 0; fi
	conf__blare_test 'checking command to enable C++11 support for target'
	conf__add_var conf_Have_cxxt_cxx11_compiler
	for extra in '' '-std=c++11' '-std=c++0x'; do
		if conf__cxxt_chk_compiler_with \
		    "$CXXFLAGS_FOR_TARGET $extra" "$LDLIBS_FOR_TARGET" \
		    '#if __cplusplus < 201103L' '#error' '#endif'; then
			CXXFLAGS_FOR_TARGET="$CXXFLAGS_FOR_TARGET $extra"
			conf__blare_res "$CXX_FOR_TARGET ` \
			    `$CPPFLAGS_FOR_TARGET $CXXFLAGS_FOR_TARGET"
			conf_Have_cxxt_cxx11_compiler=yes
			return 0
		fi
	done
	conf__blare_res '(failed; continuing)'
	conf_Have_cxxt_cxx11_compiler=no
}

conf_cxxt_probe_opt_wrapper() {
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	conf__cxxc_probe_opt_wrapper target \
	    "$conf_Host_exec $CXX_FOR_TARGET" "$CPPFLAGS_FOR_TARGET" \
	    "$CXXFLAGS_FOR_TARGET" "$LDFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET" "$conf_Target_exec" \
	    conf_Have_cxxt_opt_wrapper
}

conf_cxxt_probe_opt_fno_integrated_as() {
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	conf__cxxc_probe_opt_fno_integrated_as target \
	    "$conf_Host_exec $CXX_FOR_TARGET" "$CPPFLAGS_FOR_TARGET" \
	    "$CXXFLAGS_FOR_TARGET" "$LDFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET" "$conf_Target_exec" \
	    conf_Have_cxxt_opt_fno_integrated_as
}

conf_cxxt_probe_typ() {
	local typ var
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	typ="$1"
	shift
	var="conf_Have_cxxt_typ_`conf__escape "$typ"`"
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	conf__blare_test "testing for target-side C++ type $typ"
	if conf__cxxt_chk_compiler_with "$CXXFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET" ${1+"$@"} "typedef $typ conf_foo_t;"
	then	eval $var=yes
	else	eval $var=no
	fi
	eval conf__blare_res \"\$$var\"
}

conf_cxxt_probe_lib() {
	local family libs lib
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	family="$1"
	libs="$2"
	shift 2
	var="conf_Have_cxxt_lib_`conf__escape "$family"`"
	if eval test -n \"\$$var\"
		then return 0; fi
	conf__add_var "$var"
	eval $var=no
	for lib in $libs; do
		conf__blare_test "testing for target-side C++ library -l$lib"
		if conf__cxxt_chk_compiler_with "$CXXFLAGS_FOR_TARGET" \
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

conf_cxxt_chk_typ() {
	local var typ
	typ="$1"
	conf_cxxt_probe_typ ${1+"$@"}
	if conf__in_help_mode
		then return 0; fi
	var="conf_Have_cxxt_typ_`conf__escape "$typ"`"
	if eval test yes = \"\$$var\"; then :; else
		conf__barf "type $1 not found target-side!"; fi
}

conf_cxxt_chk_typ_1_of() {
	local var typ foo
	test $# -ne 0
	if conf__in_help_mode
		then return 0; fi
	foo=no
	for typ in "$@"; do
		var="conf_Have_cxxt_typ_`conf__escape "$typ"`"
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

conf_cxxt_probe_endianness() {
	local prog
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Endianness_cxxt"
		then return 0; fi
	conf__add_var conf_Endianness_cxxt
	conf__blare_test "testing target-side byte order"
	prog="`conf__mktemp lolwutconf`"
	if conf__cxxt_compile_prog "$prog" \
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
		conf_Endianness_cxxt="`$conf_Target_exec ./"$prog"`"
		case "$conf_Endianness_cxxt" in
		    *-endian)	;;
		    *)		conf_Endianness_cxxt=unknown;;
		esac
	else
		conf_Endianness_cxxt=unknown
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Endianness_cxxt"
}

conf_cxxt_probe_var_tpls()
{
	conf_cxxt_probe_cxx11
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Have_cxxt_var_tpls"
		then return 0; fi
	conf__add_var conf_Have_cxxt_var_tpls
	conf__blare_test \
	    'checking if target-side C++ compiler does variadic templates'
	if conf__cxxt_chk_compiler_with "$CXXFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET" ${1+"$@"} \
	    'template<class... T> class foo_t { };' \
	    'template<int... V> class bar_t { };' \
	    'extern foo_t<int, char, void *> foo;' \
	    'extern bar_t<1, -1, 6> bar;'
	then	conf_Have_cxxt_var_tpls=yes
	else	conf_Have_cxxt_var_tpls=no
	fi
	conf__blare_res "$conf_Have_cxxt_var_tpls"
}

conf_cxxt_chk_var_tpls() {
	if conf__in_help_mode
		then return 0; fi
	conf_cxxt_probe_var_tpls
	if test yes = "$conf_Have_cxxt_var_tpls"; then :; else
		conf__barf 'target-side C++ compiler does not have' \
		    'variadic templates!'; fi
}

conf_cxxt_probe_decltype() {
	conf_cxxt_probe_cxx11
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Have_cxxt_decltype"
		then return 0; fi
	conf__add_var conf_Have_cxxt_decltype
	conf__blare_test \
	    "checking if target-side C++ compiler understands decltype"
	if conf__cxxt_chk_compiler_with "$CXXFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET" ${1+"$@"} \
	    'extern int foo;' \
	    'extern decltype(foo << 1) bar;'
	then	conf_Have_cxxt_decltype=yes
	else	conf_Have_cxxt_decltype=no
	fi
	conf__blare_res "$conf_Have_cxxt_decltype"
}

conf_cxxt_probe_wchar_typ() {
	conf_cxxt_probe_cxx11
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Typ_wchar_cxxt"
		then return 0; fi
	conf__add_var conf_Typ_wchar_cxxt
	conf__blare_test 'checking target-side type equivalent to wchar_t'
	prog="`conf__mktemp lolwutconf`"
	if conf__cxxt_compile_prog "$prog" \
	    "#include <cstring>" \
	    "#include <iostream>" \
	    "using namespace std;" \
	    "int main() {" \
	    "  static const char16_t c16s[] = u\"\\u263a\\u0001f634\";" \
	    "  static const char32_t c32s[] = U\"\\u263a\\U0001f634\";" \
	    "  static const wchar_t wcs[] = L\"\\u263a\\U0001f634\";" \
	    "  if (sizeof(c16s) == sizeof(wcs) &&" \
	    "   memcmp(c16s, wcs, sizeof(c16s)) == 0)" \
	    "    cout << \"char16_t\" << endl;" \
	    "  else if (sizeof(c32s) == sizeof(wcs) &&" \
	    "   memcmp(c32s, wcs, sizeof(c32s)) == 0)" \
	    "    cout << \"char32_t\" << endl;" \
	    "  else" \
	    "    return 1;" \
	    "  return 0;" \
	    "}"
	then
		conf_Typ_wchar_cxxt="`$conf_Target_exec ./"$prog"`"
		case "$conf_Typ_wchar_cxxt" in
		    char16_t | char32_t)	;;
		    *)				conf_Typ_wchar_cxxt=unknown;;
		esac
	else
		conf_Typ_wchar_cxxt=unknown
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Typ_wchar_cxxt"
}

conf_cxxt_probe_overridable_csu_init() {
	local prog
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	conf__add_var conf_Have_overridable_csu_init_cxxt
	conf__blare_test "checking if target's __libc_csu_init is overridable"
	prog="`conf__mktemp lolwutconf`"
	#
	# Although we only want to override __libc_csu_init(...), we need to
	# also define __libc_csu_fini().  Both are likely to be defined in
	# the same archive member in libc_nonshared.a, so overriding only
	# __libc_csu_init(...) will cause a problem.
	#
	if conf__cxxt_compile_prog "$prog" \
	    'extern "C" void _exit(int);' \
	    'extern "C" void __libc_csu_init(int, char **, char **)' \
	    '  { _exit(0); }' \
	    'extern "C" void __libc_csu_fini()' \
	    '  { _exit(1); }' \
	    'int main()' \
	    '  { return 2; }' && \
	    $conf_Target_exec ./"$prog" >/dev/null 2>&1
	then	conf_Have_overridable_csu_init_cxxt=yes
	else	conf_Have_overridable_csu_init_cxxt=no
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Have_overridable_csu_init_cxxt"
}

conf_cxxt_probe_dyn_ld() {
	local prog
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Dyn_ld_cxxt"
		then return 0; fi
	conf__add_var conf_Dyn_ld_cxxt
	conf__blare_test "checking if target C++ compiler" \
	    "produces dynamically linked executables"
	#
	# If linking works even if we specify a bogus dynamic interpreter,
	# then conclude that we are creating statically linked programs.
	#
	# If linking works when we try to rope in the <dlfcn.h> routines,
	# then conclude that we are creating dynamically linked programs.
	#
	# Otherwise, we are unsure.
	#
	if conf__cxxt_chk_compiler_with \
	    "$CXXFLAGS_FOR_TARGET -Wl,--dynamic-linker=/" "$LDLIBS_FOR_TARGET"
	then
		conf__blare_res no
		conf_Dyn_ld_cxxt=no
	elif conf__cxxt_chk_compiler_with "$CXXFLAGS_FOR_TARGET" \
	    "$LDLIBS_FOR_TARGET -ldl" \
	    '#include <dlfcn.h>' \
	    'void *foo()' \
	    '  { return dlopen(0, RTLD_LAZY); }'
	then
		conf__blare_res yes
		conf_Dyn_ld_cxxt=yes
	else
		conf__blare_res unknown
		conf__barf 'cannot tell if target produces' \
		    'dynamic or static programs!'
	fi
}

conf_cxxt_probe_ctor_priority() {
	local prog
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Have_ctor_priority_cxxt"
		then return 0; fi
	conf__add_var conf_Have_ctor_priority_cxxt
	conf__blare_test \
	    "checking if target supports __attribute__((constructor(.)))"
	prog="`conf__mktemp lolwutconf`"
	if conf__cxxt_compile_prog "$prog" \
	    'static int baz = 2;' \
	    '__attribute__((constructor(456))) void foo(void)' \
	    '  { if (baz == 1) baz = 0; }' \
	    '__attribute__((constructor(123))) void bar(void)' \
	    '  { if (baz == 2) baz = 1; }' \
	    'int main()' \
	    '  { return baz; }' && \
	    $conf_Target_exec ./"$prog" >/dev/null 2>&1
	then	conf_Have_ctor_priority_cxxt=yes
	else	conf_Have_ctor_priority_cxxt=no
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Have_ctor_priority_cxxt"
}

conf_cxxt_probe_asm_goto() {
	local prog
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	if test -n "$conf_Have_asm_goto_cxxt"
		then return 0; fi
	conf__add_var conf_Have_asm_goto_cxxt
	conf__blare_test \
	    "checking if target supports __asm goto(...)"
	prog="`conf__mktemp lolwutconf`"
	if conf__cxxt_compile_prog "$prog" \
	    'int main() {' \
	    '  __asm goto("" : : : : foo);' \
	    '  return 0;' \
	    'foo:' \
	    '  return 1;' \
	    '}' && \
	    $conf_Target_exec ./"$prog" >/dev/null 2>&1
	then	conf_Have_asm_goto_cxxt=yes
	else	conf_Have_asm_goto_cxxt=no
	fi
	rm -f "$prog"
	conf__blare_res "$conf_Have_asm_goto_cxxt"
}

conf_cxxt_probe_func() {
	local func n_args var
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	func="$1"
	n_args="$2"
	shift 2
	var="conf_Have_cxxt_func_`conf__escape "$func"`"
	conf__cxxc_probe_func target "$CXX_FOR_TARGET" \
	    "$CPPFLAGS_FOR_TARGET" "$CXXFLAGS_FOR_TARGET" \
	    "$LDFLAGS_FOR_TARGET" "$LDLIBS_FOR_TARGET" "$conf_Target_exec" \
	    "$func" "$n_args" "$var" ${1+"$@"}
}

conf_cxxt_probe_impld_func() {
	local func n_args args err_rv f_var i_var prog
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	func="$1"
	n_args="$2"
	args="$3"
	err_rv="$4"
	shift 4
	conf_cxxt_probe_func "$func" "$n_args" ${1+"$@"}
	f_var="conf_Have_cxxt_func_`conf__escape "$func"`"
	i_var="conf_Have_cxxt_impld_func_`conf__escape "$func"`"
	conf__add_var $i_var
	if eval test no = \"$$f_var\"; then
		eval $i_var=no
		return 0
	fi
	conf__blare_test \
	    "testing if target-side C++ function $func is implemented"
	prog="`conf__mktemp lolwutconf`"
	if conf__cxxt_compile_prog "$prog" \
	    "#include <errno.h>" \
	    ${1+"$@"} \
	    "int main() {" \
	    "  errno = 0;" \
	    "  if ($func($args) == ($err_rv) && errno == ENOSYS)" \
	    "    return 1;" \
	    "  return 0;" \
	    "}" && \
	    $conf_Target_exec ./"$prog" >/dev/null 2>&1
	then	eval $i_var=yes
	else	eval $i_var=no
	fi
	rm -f "$prog"
	eval conf__blare_res \"\$$i_var\"
}

#
# This is useful for constants which may not be defined as preprocessor
# macros.  <linux/sysctl.h> e.g. has CTL_KERN as an `enum' value.
#
conf_cxxt_probe_const() {
	local const var
	conf_cxxt_chk_compiler_and_executor
	if conf__in_help_mode
		then return 0; fi
	const="$1"
	shift
	var="conf_Have_cxxt_const_`conf__escape "$const"`"
	conf__add_var "$var"
	conf__blare_test "testing for target-side C++ constant $const"
	if conf__cxxt_chk_compiler_with "$CXXFLAGS_FOR_TARGET" \
	    "$LDFLAGS_FOR_TARGET" ${1+"$@"} \
	    "int foo(void) { return (int)($const); }"
	then	eval $var=yes
	else	eval $var=no
	fi
	eval conf__blare_res \"\$$var\"
}

conf_cxxt_probe_const_val() {
	local const var prog val use_cinttypes
	const="$1"
	shift
	conf_cxxt_probe_const "$const" ${1+"$@"}
	conf_cxxt_probe_typ std::intmax_t '#include <cinttypes>'
	if conf__in_help_mode
		then return 0; fi
	var="conf_Have_cxxt_const_`conf__escape "$const"`"
	if eval test no = \"\$$var\"
		then return 0; fi
	var="conf_Val_cxxt_const_`conf__escape "$const"`"
	conf__add_var "$var"
	conf__blare_test "getting value of target-side C++ constant $const"
	prog="`conf__mktemp lolwutconf`"
	if test yes = "$conf_Have_cxxt_typ_std_0intmax_t"
	then	use_cinttypes=1
	else	use_cinttypes=0
	fi
	if conf__cxxt_compile_prog "$prog" \
	    ${1+"$@"} \
	    "#if $use_cinttypes" \
	    "# include <cinttypes>" \
	    "#else" \
	    "# include <inttypes.h>" \
	    "#endif" \
	    "#include <climits>" \
	    "#include <iostream>" \
	    "#include <iomanip>" \
	    "#include <type_traits>" \
	    "int main() {" \
	    "#if $use_cinttypes" \
	    "  using std::intmax_t;" \
	    "#endif" \
	    "  auto c = ($const);" \
	    "  if (!std::is_integral<__typeof(c)>::value &&" \
	    "      !std::is_enum<__typeof(c)>::value)" \
	    "    return 0;" \
	    "  std::cout << std::hex;" \
	    "  if (c >= CHAR_MIN && c < 0)" \
	    "    std::cout << \"-0x\" << (int)(-c);" \
	    "  else if (c >= 0 && c <= UCHAR_MAX)" \
	    "    std::cout << \"0x\" << (int)c;" \
	    "  else if (c >= 0)" \
	    "    std::cout << \"0x\" << c;" \
	    "  else if (c == INTMAX_MIN)" \
	    "    std::cout << \"(-1-0x\" << (-1 - INTMAX_MIN) << ')';" \
	    "  else" \
	    "    std::cout << \"(-0x\" << (-(intmax_t)c) << ')';" \
	    "  std::cout << std::endl;" \
	    "  return 0;" \
	    "}"
	then	val="`$conf_Target_exec ./"$prog"`"
	else	val=
	fi
	rm -f "$prog"
	if test -z "$val"; then
		conf__blare_res '(?!?)'
		conf__barf "cannot get value of $const!"
	fi
	eval $var=\"\$val\"
	conf__blare_res "$val"
}
