#!/usr/bin/awk -f
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

BEGIN {
	FS = RS
}

{
	gsub(/ /, " b")
	gsub(/____/, " 4")
	gsub(/___/, " 3")
	gsub(/__/, " 2")
	gsub(/_/, " 1")
	gsub(/ /, "_")
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
