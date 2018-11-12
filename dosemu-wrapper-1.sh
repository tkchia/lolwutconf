#!/bin/sh
libdir="`which dosemu.bin | xargs dirname`"/../share/dosemu
if test -f "$libdir"/commands/dosemu/exitemu.com; then
	dosemu.bin --Flibdir "$libdir" \
	    -I 'video {none} keyboard {layout us}' -p -K ${1+"$@"} \
	    </dev/null 2>/dev/null
else
	dosemu.bin -I 'video {none} keyboard {layout us}' -p -K ${1+"$@"} \
	    </dev/null 2>/dev/null
fi
