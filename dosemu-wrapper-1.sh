#!/bin/sh
exec dosemu.bin -q -I 'video {none}' ${1+"$@"}
