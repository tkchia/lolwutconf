#!/bin/sh
exec dosemu -dumb -quiet -K ${1+"$@"} 2>/dev/null
