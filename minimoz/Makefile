#
# (c) 2004 Andreas Steinmetz, ast@domdv.de
#
include Makedefs

all:
	@make -C src all

clean:
	@make -C src clean

distclean: clean
	@make -C data clean

db: all
	@make -C data db
