#!/usr/bin/env perl

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
