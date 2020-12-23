#!/usr/bin/env perl
# Copyright (c) 2017 TK Chia
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

use strict;

my $ifn = undef;
my $ofn = undef;
my @new_argv = ();
my $pass = 0;
my $o = 0;
for my $arg (@ARGV) {
	if ($pass) {
		push(@new_argv, $arg);
		$pass = 0;
	} elsif ($o) {
		$ofn = $arg;
		$o = 0;
	} elsif ($arg =~ /^-[IORSKN]$/s) {
		push(@new_argv, $arg);
		$pass = 1;
	} elsif ($arg eq '-o')
		{ $o = 1; }
	  elsif ($arg =~ /^-o(.*)$/s)
		{ $ofn = $1; }
	  elsif ($arg =~ /^-/s)
		{ push(@new_argv, $arg); }
	  elsif (!defined $ifn)
		{ $ifn = $arg; }
	  else
		{ die("$0: bad arguments\n"); }
}
die("$0: bad arguments\n") if (!defined $ifn);
push(@new_argv, $ifn);
push(@new_argv, $ofn) if (defined $ofn);
exec('ia16-elf-objcopy', '-O', 'binary', @new_argv);
die("$0: exec failed: $!\n");
