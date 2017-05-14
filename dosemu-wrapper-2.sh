#!/bin/sh
exec dosemu.bin -I 'cpuemu vm86sim video {none}' ${1+"$@"}
