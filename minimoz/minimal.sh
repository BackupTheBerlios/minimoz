#!/bin/sh
#
# (c) 2004 Andreas Steinmetz, ast@domdv.de
#
cd `dirname $0`/samples
ln -sf ../data/dmoz.db dmoz.db
exec ../src/minimoz -l 9999 -n -z -t minimal.tmpl
