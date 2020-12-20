#!/bin/sh

conf_mod app.build

conf_versioning_use_git_date() {
	local part1 part2
	if conf__in_help_mode
		then continue; fi
	part1="$*"
	conf_appb_probe_exists git
	if test yes = "$conf_Have_appb_git" && \
	   git diff --quiet HEAD 2>/dev/null; then
		part2="`TZ=UTC0 git log -n1 --oneline --date=short-local \
					--format='%ad' | sed 's/-//g'`"
	else
		conf_appb_chk_exists date
		part2="`TZ=UTC0 date +%Y%m%d`"
	fi
	conf__blare_test 'deciding package version number'
	conf__add_var conf_Pkg_ver
	conf_Pkg_ver="$part1$part2"
	conf__blare_res "$conf_Pkg_ver"
}
