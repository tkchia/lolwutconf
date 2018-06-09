#!/bin/sh
libdir="`which dosemu.bin | xargs dirname`"/../share/dosemu
if test -f "$libdir"/commands/dosemu/exitemu.com; then
	dosemu.bin --Flibdir "$libdir" -I 'video {none}' -K ${1+"$@"} | \
	    sed '1,/Execute : / ! p'
else
	dosemu.bin -I 'video {none}' -K ${1+"$@"} | sed '1,/Execute : / ! p'
fi
