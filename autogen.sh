#!/bin/sh

[ -d m4 ] || mkdir m4

aclocal
autoconf
autoheader
glibtoolize --copy
automake --add-missing --copy


