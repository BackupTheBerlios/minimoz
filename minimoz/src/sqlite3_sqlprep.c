/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdarg.h>
#include <stdio.h>
#include "db.h"

int sqlprep(DB db,SQL *sql,const char *cmd,const char *tblfmt,
	const char *format,...)
{
	const char *unused;
	int column;
	va_list ap;

	if(!db)
	{
		printf("sqlprep: database was not open\n");
		*sql=NULL;
		return -1;
	}

	if(sqlite3_prepare(db,cmd,-1,sql,&unused)!=SQLITE_OK)
	{
		printf("sql prepare for %s: %s\n",cmd,sqlite3_errmsg(db));
		*sql=NULL;
		return -1;
	}
	if(!format)return 0;
	column=0;
	va_start(ap,format);
	while(*format)switch(*format++)
	{
	case 'i':
		sqlite3_bind_int(*sql,++column,va_arg(ap,int));
		break;
	case 't':
		sqlite3_bind_text(*sql,++column,va_arg(ap,char *),-1,
			SQLITE_STATIC);
		break;
	default:printf("unknown format %c\n",format[-1]);
		va_end(ap);
		sqlite3_finalize(*sql);
		*sql=NULL;
		return -1;
	}
	va_end(ap);
	return 0;
}
