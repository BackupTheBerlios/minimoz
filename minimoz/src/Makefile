#
# (c) 2004 Andreas Steinmetz, ast@domdv.de
#

DEFS=../Makedefs
PROGRAMS=buildmoz
EXTRAOBJS=
DBLIST=
DBREF=
DBREF+=1
PGSQLINC=
PGSQLLIB=
MYSQLINC=
MYSQLLIB=
SQLITEINC=
SQLITELIB=
ZLIBINC=
ZLIBLIB=
DMALLOCINC=
DMALLOCLIB=
NULLDBINC=
NULLDBLIB=

include $(DEFS)

ifneq ($(NULLDB),)
DBLIST+=1
CFLAGS+=-DUSE_NULL
SQLPFX=null_
NULLDBLIB=-lpthread
endif

ifneq ($(SQLITE),)
PROGRAMS+=minimoz moztract
DBLIST+=1
CFLAGS+=-DUSE_SQLITE3
SQLPFX=sqlite3_
ifeq ($(SQLITE),/usr)
SQLITEINC=
SQLITELIB=-lsqlite3
else
SQLITEINC=-I$(SQLITE)/include
ifneq ($(RPATHFLAG),)
SQLITELIB=-L$(SQLITE)/lib64 -L$(SQLITE)/lib $(RPATHFLAG)$(SQLITE)/lib64 $(RPATHFLAG)$(SQLITE)/lib -lsqlite3
else
SQLITELIB=-L$(SQLITE)/lib64 -L$(SQLITE)/lib -lsqlite3
endif
endif
endif

ifneq ($(MYSQL),)
PROGRAMS+=minimoz moztract dmozcollmy.$(SHEXT)
DBLIST+=1
CFLAGS+=-DUSE_MYSQL
SQLPFX=mysql_
MYSQLINC=-I$(MYSQL)/include/mysql
ifneq ($(RPATHFLAG),)
MYSQLLIB=-L$(MYSQL)/lib64/mysql -L$(MYSQL)/lib/mysql $(RPATHFLAG)$(MYSQL)/lib64/mysql $(RPATHFLAG)$(MYSQL)/lib/mysql -lmysqlclient_r
else
MYSQLLIB=-L$(MYSQL)/lib64/mysql -L$(MYSQL)/lib/mysql -lmysqlclient_r
endif
endif

ifneq ($(MYSQLENG),)
PROGRAMS+=minimoz moztract
DBLIST+=1
CFLAGS+=-DUSE_MYSQL -DENGINE
SQLPFX=mysql_
MYSQLINC=-I$(MYSQLENG)/include/mysql
MYSQLLIB=-L$(MYSQLENG)/lib64/mysql -L$(MYSQLENG)/lib/mysql -lmysqld $(ENGINELIBS)
endif

ifneq ($(PGSQL),)
PROGRAMS+=minimoz moztract dmozcollpg.$(SHEXT)
DBLIST+=1
CFLAGS+=-DUSE_PGSQL
SQLPFX=pgsql_
ifeq ($(PGSQL),/usr)
PGSQLINC=-I/usr/include/server
PGSQLLIB=-lpq
else
PGSQLINC=-I$(PGSQL)/include/server -I$(PGSQL)/include
ifneq ($(RPATHFLAG),)
PGSQLLIB=-L$(PGSQL)/lib64 -L$(PGSQL)/lib $(RPATHFLAG)$(PGSQL)/lib64 $(RPATHFLAG)$(PGSQL)/lib -lpq
else
PGSQLLIB=-L$(PGSQL)/lib64 -L$(PGSQL)/lib -lpq
endif
endif
endif

ifeq ($(ZLIB),/usr)
ZLIBINC=
ZLIBLIB=-lz
else
ZLIBINC=-I$(ZLIB)/include
ifneq ($(RPATHFLAG),)
ZLIBLIB=-L$(ZLIB)/lib64 -L$(ZLIB)/lib $(RPATHFLAG)$(ZLIB)/lib64 $(RPATHFLAG)$(ZLIB)/lib -lz
else
ZLIBLIB=-L$(ZLIB)/lib64 -L$(ZLIB)/lib -lz
endif
endif

ifneq ($(CURL),)
PROGRAMS+=loadmoz
ifeq ($(CURL),/usr)
CURLINC=
CURLLIB=-lcurl
else
CURLINC=-I$(CURL)/include
CURLLIB=-L$(CURL)/lib64 -L$(DMALLOC)/lib -lcurl
endif
endif

ifneq ($(DMALLOC),)
CFLAGS+=-DMEMDEBUG
ifeq ($(DMALLOC),/usr)
DMALLOCINC=
DMALLOCLIB=-ldmallocth
else
DMALLOCINC=-I$(DMALLOC)/include
DMALLOCLIB=-L$(DMALLOC)/lib64 -L$(DMALLOC)/lib -ldmallocth
endif
endif

ifeq ($(INCLUDED_GMTIME_R),1)
CFLAGS+=-DNEED_GMTIME_R
EXTRAOBJS+=sys_gmtime_r.o
endif

ifeq ($(INCLUDED_STRTOK_R),1)
CFLAGS+=-DNEED_STRTOK_R
EXTRAOBJS+=sys_strtok_r.o
endif

ifeq ($(INCLUDED_STRFTIME),1)
CFLAGS+=-DNEED_STRFTIME
EXTRAOBJS+=sys_strftime.o
endif

ifeq ($(INCLUDED_POLL),1)
CFLAGS+=-DNEED_POLL
EXTRAOBJS+=sys_poll.o
endif

ifeq ($(INCLUDED_DAEMON),1)
CFLAGS+=-DNEED_DAEMON
EXTRAOBJS+=sys_daemon.o
endif

ifeq ($(INCLUDED_SETRESUID),1)
CFLAGS+=-DNEED_SETRESUID
EXTRAOBJS+=sys_setresuid.o
endif

ifeq ($(INCLUDED_SETRESGID),1)
CFLAGS+=-DNEED_SETRESGID
EXTRAOBJS+=sys_setresgid.o
endif

ifeq ($(INCLUDED_TIMES),1)
CFLAGS+=-DNEED_TIMES
endif

ifeq ($(INCLUDED_STRCASECMP),1)
CFLAGS+=-DNEED_STRCASECMP
EXTRAOBJS+=sys_strcasecmp.o
endif

ifeq ($(INCLUDED_STRNCASECMP),1)
CFLAGS+=-DNEED_STRNCASECMP
EXTRAOBJS+=sys_strncasecmp.o
endif

ifeq ($(INCLUDED_MUTEX),1)
CFLAGS+=-DNEED_MUTEX
EXTRAOBJS+=sys_mutex.o
endif

ifeq ($(INCLUDED_SEM),1)
CFLAGS+=-DNEED_SEM
EXTRAOBJS+=sys_sem.o
endif

ifeq ($(INCLUDED_GETOPT),1)
CFLAGS+=-DNEED_GETOPT
EXTRAOBJS+=sys_getopt.o
endif

INCS=$(DMALLOCINC) $(NULLDBINC) $(SQLITEINC) $(MYSQLINC) $(PGSQLINC) $(ZLIBINC)
INCS+=$(CURLINC)
LIBS=$(DMALLOCLIB) $(ZLIBLIB)
DBLIB=$(NULLDBLIB) $(SQLITELIB) $(MYSQLLIB) $(PGSQLLIB)

.phony: all clean distclean

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $<

ifneq ($(DBLIST),$(DBREF))

all:
	@echo
	@echo ============================================
	@echo Exactly one database type must be specified.
	@echo Edit the file \'Makedefs\' before compiling.
	@echo ============================================
	@echo
	@false

else

all: $(PROGRAMS)

endif

clean:
	rm -f $(PROGRAMS) *.o *.a core

buildmoz: buildmoz.o libmoz.a
	$(LD) $(LDFLAGS) -o $@ $< -L. -lmoz $(LIBS) $(DBLIB) -lm

minimoz: minimoz.o libmoz.a
	$(LD) $(LDFLAGS) -o $@ $< -L. -lmoz $(LIBS) $(DBLIB)

moztract: moztract.o libmoz.a
	$(LD) $(LDFLAGS) -o $@ $< -L. -lmoz $(LIBS) $(DBLIB)

loadmoz: loadmoz.o libmoz.a
	$(LD) $(LDFLAGS) -o $@ $< -L. -lmoz $(LIBS) $(CURLLIB) -lpthread

dmozcollmy.$(SHEXT): mysql_dmozcoll.o libmoz.a
	$(LD) $(SHLIBFLAGS) -o $@ $< -L. -lmoz

dmozcollpg.$(SHEXT): pgsql_dmozcoll.o libmoz.a
	$(LD) $(SHLIBFLAGS) -o $@ $< -L. -lmoz

libmoz.a: $(SQLPFX)sqlclose.o $(SQLPFX)sqlfin.o $(SQLPFX)sqlinsert.o \
	$(SQLPFX)sqlopen.o $(SQLPFX)sqlparams.o $(SQLPFX)sqlprep.o \
	$(SQLPFX)sqlreset.o $(SQLPFX)sqlrow.o $(SQLPFX)sqlrun.o \
	$(SQLPFX)sqllock.o $(SQLPFX)sqlunlock.o $(SQLPFX)sqlinit.o \
	$(SQLPFX)sqlexit.o $(SQLPFX)sqlthreadinit.o $(SQLPFX)sqlthreadexit.o \
	$(SQLPFX)sqlping.o lib_ucs4_normalize.o lib_ucs4_to_utf8.o \
	lib_utf8_to_ucs4.o lib_checkutf8str.o lib_checkutf8data.o \
	lib_checkutf8unicodestr.o $(EXTRAOBJS)
	ar rcu $@ $^

buildmoz.o: buildmoz.c db.h lib.h $(DEFS)
minimoz.o: minimoz.c db.h lib.h $(DEFS)
moztract.o: moztract.c db.h lib.h $(DEFS)
loadmoz.o: loadmoz.c db.h lib.h $(DEFS)
$(SQLPFX)sqlclose.o: $(SQLPFX)sqlclose.c db.h $(DEFS)
$(SQLPFX)sqlfin.o: $(SQLPFX)sqlfin.c db.h $(DEFS)
$(SQLPFX)sqlinsert.o: $(SQLPFX)sqlinsert.c db.h $(DEFS)
$(SQLPFX)sqlopen.o: $(SQLPFX)sqlopen.c db.h lib.h $(DEFS)
$(SQLPFX)sqlparams.o: $(SQLPFX)sqlparams.c db.h $(DEFS)
$(SQLPFX)sqlprep.o: $(SQLPFX)sqlprep.c db.h $(DEFS)
$(SQLPFX)sqlreset.o: $(SQLPFX)sqlreset.c db.h $(DEFS)
$(SQLPFX)sqlrow.o: $(SQLPFX)sqlrow.c db.h $(DEFS)
$(SQLPFX)sqlrun.o: $(SQLPFX)sqlrun.c db.h $(DEFS)
$(SQLPFX)sqllock.o: $(SQLPFX)sqllock.c db.h $(DEFS)
$(SQLPFX)sqlunlock.o: $(SQLPFX)sqlunlock.c db.h $(DEFS)
$(SQLPFX)sqlinit.o: $(SQLPFX)sqlinit.c db.h $(DEFS)
$(SQLPFX)sqlexit.o: $(SQLPFX)sqlexit.c db.h $(DEFS)
$(SQLPFX)sqlthreadinit.o: $(SQLPFX)sqlthreadinit.c db.h $(DEFS)
$(SQLPFX)sqlthreadexit.o: $(SQLPFX)sqlthreadexit.c db.h $(DEFS)
$(SQLPFX)sqlping.o: $(SQLPFX)sqlping.c db.h $(DEFS)
mysql_dmozcoll.o: mysql_dmozcoll.c lib.h $(DEFS)
pgsql_dmozcoll.o: pgsql_dmozcoll.c lib.h $(DEFS)
lib_ucs4_normalize.o: lib_ucs4_normalize.c lib.h $(DEFS)
lib_ucs4_to_utf8.o: lib_ucs4_to_utf8.c lib.h $(DEFS)
lib_utf8_to_ucs4.o: lib_utf8_to_ucs4.c lib.h $(DEFS)
lib_checkutf8str.o: lib_checkutf8str.c lib.h $(DEFS)
lib_checkutf8data.o: lib_checkutf8data.c lib.h $(DEFS)
lib_checkutf8unicodestr.o: lib_checkutf8unicodestr.c lib.h $(DEFS)
sys_gmtime_r.o: sys_gmtime_r.c lib.h $(DEFS)
sys_strtok_r.o: sys_strtok_r.c lib.h $(DEFS)
sys_strftime.o: sys_strftime.c lib.h $(DEFS)
sys_poll.o: sys_poll.c lib.h $(DEFS)
sys_daemon.o: sys_daemon.c lib.h $(DEFS)
sys_setresuid.o: sys_setresuid.c lib.h $(DEFS)
sys_setresgid.o: sys_setresgid.c lib.h $(DEFS)
sys_strcasecmp.o: sys_strcasecmp.c lib.h $(DEFS)
sys_strncasecmp.o: sys_strncasecmp.c lib.h $(DEFS)
sys_mutex.o: sys_mutex.c lib.h $(DEFS)
sys_sem.o: sys_sem.c lib.h $(DEFS)
sys_getopt.o: sys_getopt.c lib.h $(DEFS)
