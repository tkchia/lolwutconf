#!/bin/sh
# Copyright (c) 2016--2017 TK Chia
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

conf_appb_probe_exists() {
	local app c_app
	for app in ${1+"$@"}; do
		c_app="`conf__escape "$app"`"
		if conf__in_help_mode || \
		   eval test -n \"\$conf_Have_appb_"$c_app"\"
			then continue; fi
		conf__add_var conf_Have_appb_"$c_app"
		conf__blare_test "checking if build-side" \
		    "command line program \`$app' exists"
		if conf__app_exists "$app"; then
			eval conf_Have_appb_"$c_app"=yes
			conf__blare_res yes
		else
			eval conf_Have_appb_"$c_app"=no
			conf__blare_res no
		fi
	done
}

conf_appb_chk_exists() {
	local app c_app
	for app in ${1+"$@"}; do
		c_app="`conf__escape "$app"`"
		if conf__in_help_mode || \
		   eval test yes = \"\$conf_Have_appb_"$c_app"\"
			then continue; fi
		conf__add_var conf_Have_appb_"$c_app"
		conf__blare_test "checking if build-side" \
		    "command line program \`$app' exists"
		if conf__app_exists "$app"; then
			eval conf_Have_appb_"$c_app"=yes
			conf__blare_res yes
		else
			conf__blare_res no
			conf__barf "program \`$app' not found!"
		fi
	done
}
