#!/bin/sh
exec dosemu.bin -I 'cpuemu vm86sim video {none} keyboard {layout us}' \
    ${1+"$@"} </dev/null 2>/dev/null
