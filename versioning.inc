#!/bin/sh
# Copyright (c) 2020 TK Chia
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

conf_mod app.build

conf_versioning_use_git_date() {
	local name part1 part2
	if conf__in_help_mode
		then continue; fi
	name="$1"
	shift
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
	conf__blare_test 'deciding package name'
	conf__add_var conf_Pkg_name conf_Pkg_ver
	conf_Pkg_name="$name"
	conf__blare_res "$conf_Pkg_name"
	conf__blare_test 'deciding package version number'
	conf_Pkg_ver="$part1$part2"
	conf__blare_res "$conf_Pkg_ver"
}
