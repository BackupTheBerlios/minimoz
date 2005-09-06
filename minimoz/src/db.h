/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#ifdef MEMDEBUG
#ifndef _MEMDEBUG_INCLUDED
#define _MEMDEBUG_INCLUDED
#include <dmalloc.h>
#if defined(DMALLOC_STRDUP_MACRO) && defined(strdup)
#undef strdup
#define strdup(str) dmalloc_strdup(__FILE__, __LINE__, (str), 0)
#endif
#endif
#endif

#ifndef _DB_H_INCLUDED
#define _DB_H_INCLUDED

#define SQLFLAGS_LOWMEM		0x00000001
#define SQLFLAGS_STDMEM		0x00000002
#define SQLFLAGS_HIGHMEM	0x00000004
#define SQLFLAGS_BEGIN		0x00000008
#define SQLFLAGS_COMMIT		0x00000010
#define SQLFLAGS_ROLLBACK	0x00000020

#ifdef USE_NULL
#define SQLTYPE_TINYINT		""			/* 0-255 */
#define SQLTYPE_SMALLINT	""			/* 0-65535 */
#define SQLTYPE_INTEGER		""			/* 0-2147483647 */
#define SQLTYPE_TINYTEXT	""			/* 0-255 characters */
#define SQLTYPE_TEXT		""			/* 0-65535 characters */
#define SQLTYPE_PRIMARY		""

#define SQLSTMT_TEXTIDX(a)	a
#define SQLSTMT_COLLATE(a)	a
#define SQLSTMT_COLLATE1(a,b)
#define SQLSTMT_COLLATE2(a,b)	a
#define SQLSTMT_TRUNCATE(a)	NULL

#define SQLTBL_STD		""
#define SQLTBL_MEM		""

#undef  SQLFLAG_MULTIROW

#define SQLDFLT_DB		NULL
#define SQLDFLT_USER		NULL
#define SQLDFLT_PASS		NULL

typedef void *DB;
typedef void *SQL;
#endif

#ifdef USE_SQLITE3

#include <sqlite3.h>

#define SQLTYPE_TINYINT		" INTEGER"		/* 0-255 */
#define SQLTYPE_SMALLINT	" INTEGER"		/* 0-65535 */
#define SQLTYPE_INTEGER		" INTEGER"		/* 0-2147483647 */
#define SQLTYPE_TINYTEXT	" TEXT"			/* 0-255 characters */
#define SQLTYPE_TEXT		" TEXT"			/* 0-65535 characters */
#define SQLTYPE_PRIMARY		" PRIMARY KEY"

#define SQLSTMT_TEXTIDX(a)	a
#define SQLSTMT_COLLATE(a)	a " COLLATE dmoz"
#define SQLSTMT_COLLATE1(a,b)
#define SQLSTMT_COLLATE2(a,b)	a " COLLATE dmoz"
#define SQLSTMT_TRUNCATE(a)	NULL

#undef  SQLIDX_EXTRA

#define SQLTBL_STD		""
#define SQLTBL_MEM		""

#undef  SQLFLAG_MULTIROW

#define SQLDFLT_DB		"dmoz.db"
#define SQLDFLT_USER		NULL
#define SQLDFLT_PASS		NULL

typedef sqlite3 *DB;
typedef sqlite3_stmt *SQL;

#endif

#ifdef USE_MYSQL

#include <mysql.h>

#if MYSQL_VERSION_ID >= 40100
#define SQLCHSET " CHARACTER SET utf8 COLLATE utf8_bin"
#else
#define SQLCHSET ""
#endif

#define SQLTYPE_TINYINT		" TINYINT UNSIGNED"	/* 0-255 */
#define SQLTYPE_SMALLINT	" SMALLINT UNSIGNED"	/* 0-65535 */
#define SQLTYPE_INTEGER		" INTEGER"		/* 0-2147483647 */
#define SQLTYPE_TINYTEXT	" TINYTEXT" SQLCHSET	/* 0-255 characters */
#define SQLTYPE_TEXT		" TEXT" SQLCHSET	/* 0-65535 characters */
#define SQLTYPE_PRIMARY		" PRIMARY KEY"

#define SQLSTMT_TEXTIDX(a)	a "(255)"
#if !defined(ENGINE)
#define SQLSTMT_COLLATE(a)	"dmozcoll(" a ")"
#define SQLSTMT_COLLATE1(a,b)
#define SQLSTMT_COLLATE2(a,b)	"dmozcoll(" a ")"
#else
#define SQLSTMT_COLLATE(a)	a
#define SQLSTMT_COLLATE1(a,b)
#define SQLSTMT_COLLATE2(a,b)	a
#endif
#define SQLSTMT_TRUNCATE(a)	NULL

#undef  SQLIDX_EXTRA

#define SQLTBL_STD		" TYPE=MYISAM"
#define SQLTBL_MEM		" TYPE=HEAP"

#define SQLFLAG_MULTIROW

#if defined(MYSQL_UNIX_ADDR) && !defined(ENGINE)
#define SQLDFLT_DB		MYSQL_UNIX_ADDR ":dmoz"
#elif defined(MYSQL_PORT) && !defined(ENGINE)
#define SQLDFLT_DB		"localhost:" MYSQL_PORT ":dmoz"
#else
#define SQLDFLT_DB		"dmoz"
#endif
#define SQLDFLT_USER		" "
#define SQLDFLT_PASS		NULL

#ifdef ENGINE
extern char *mysql_argv[];
#endif

typedef MYSQL *DB;

#if MYSQL_VERSION_ID >= 40105

typedef struct _sql
{
	MYSQL_STMT *sql;
	MYSQL_BIND *param;
	MYSQL_BIND *resparam;
	MYSQL_RES *result;
	unsigned long *lbuf;
	unsigned long *reslbuf;
	int *ibuf;
	int *resibuf;
	int total;
	int restotal;
} sqlreq;

#else

#define SQLTYPE_FIXED	0
#define SQLTYPE_RESULT	1
#define SQLTYPE_NUMDATA	2
#define SQLTYPE_DATA	3

typedef struct _sqlchain
{
	struct _sqlchain *next;
	union
	{
		char *string;
		MYSQL_RES *r;
	} u;
	int type;
	int len;
} sqlchain;

typedef struct _sql
{
	sqlchain *sql;
	char num[64][12];
} sqlreq;

#endif

typedef sqlreq *SQL;

#endif

#ifdef USE_PGSQL

#include <libpq-fe.h>

#define SQLTYPE_TINYINT		" INTEGER"		/* 0-255 */
#define SQLTYPE_SMALLINT	" INTEGER"		/* 0-65535 */
#define SQLTYPE_INTEGER		" INTEGER"		/* 0-2147483647 */
#define SQLTYPE_TINYTEXT	" TEXT"			/* 0-255 characters */
#define SQLTYPE_TEXT		" TEXT"			/* 0-65535 characters */
#define SQLTYPE_PRIMARY		" PRIMARY KEY"

#define SQLSTMT_TEXTIDX(a)	a
#define SQLSTMT_COLLATE(a)	"dmozcoll(" a ")"
#define SQLSTMT_COLLATE1(a,b)	",dmozcoll(" a ") AS " b
#define SQLSTMT_COLLATE2(a,b)	b
#define SQLSTMT_TRUNCATE(a)	"TRUNCATE TABLE " a

#define SQLIDX_EXTRA		"ANALYZE"

#define SQLTBL_STD		""
#define SQLTBL_MEM		""

#undef  SQLFLAG_MULTIROW

#define SQLDFLT_DB		"dmoz"
#define SQLDFLT_USER		"dmoz"
#define SQLDFLT_PASS		NULL

typedef PGconn *DB;

typedef struct _sql
{
	int total;
	int currow;
	int totrow;
	int totcol;
	char **pv;
	int *pl;
	int *pf;
	int *val;
	PGresult *res;
	char id[2*sizeof(unsigned long)+2];
} sqlreq;

typedef sqlreq *SQL;

#endif

extern int sqlrow(DB db,SQL sql,const char *format,...);
extern int sqlinsert(DB db,SQL sql,char *format,...);
extern int sqlprep(DB db,SQL *sql,const char *cmd,const char *tblfmt,
	const char *format,...);
extern int sqlparams(DB db,SQL sql,const char *format,...);
extern int sqlrun(DB db,const char *cmd);
extern int sqlopen(char *database,char *user,char *password,DB *db,int flags);
extern int sqlclose(DB db,int flags);
extern int sqlfin(DB db,SQL sql);
extern int sqlreset(char *database,char *user,char *password);
extern int sqllock(DB db,char *table);
extern int sqlunlock(DB db);
extern int sqlinit(char *database);
extern int sqlexit(void);
extern int sqlthreadinit(void);
extern int sqlthreadexit(void);
extern int sqlping(DB db);

#endif
