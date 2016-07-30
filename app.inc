#!/bin/sh

conf_app_chk_exists() {
	local app c_app
	for app in ${1+"$@"}; do
		c_app="`conf__escape "$app"`"
		conf__add_var conf_Have_app_"$c_app"
		if eval test yes = \"\$conf_Have_app_"$c_app"\"
			then continue; fi
		conf__blare_test \
		    "checking if command line program \`$app' exists"
		if conf__app_exists "$app"; then
			eval conf_Have_app_"$c_app"=yes
			conf__blare_res yes
		else
			conf__blare_res no
			conf__barf "program \`$app' not found!"
		fi
	done
}
