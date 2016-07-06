#!/usr/bin/awk -f

BEGIN {
	FS = RS
}

{
	gsub(/____/, "_4")
	gsub(/___/, "_3")
	gsub(/__/, "_2")
	gsub(/_/, "_1")
	gsub(/::/, "_0")
	gsub(/\./, "_d")
	gsub(/\+/, "_p")
	gsub(/-/, "_m")
	if (/^<.*>$/)
		$0 = "_i" substr($0, 2, length($0) - 2)
	gsub(/\//, "_s")
	n = length($0)
	out = ""
	for (i = 1; i <= n; ++i) {
		c = substr($0, i, 1)
		if (tolower(c) ~ /^[abcdefghijklmnopqrstuvwxyz0123456789_]$/)
			out = out c
		else {
			a = 1
			while (sprintf("%c", a) != c)
				++a
			out = out sprintf("_x%x_", a)
		}
	}
	print out
}
