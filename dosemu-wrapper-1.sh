#!/bin/sh
libdir="`which dosemu.bin | xargs dirname`"/../share/dosemu
if test -f "$libdir"/commands/dosemu/exitemu.com; then
	exec dosemu.bin -q --Flibdir "$libdir" -I 'video {none}' ${1+"$@"}
else
	exec dosemu.bin -q -I 'video {none}' ${1+"$@"}
fi
