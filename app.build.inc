#!/bin/sh

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
