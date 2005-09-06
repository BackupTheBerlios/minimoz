/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdarg.h>
#include <stdio.h>
#include "db.h"

int sqlparams(DB db,SQL sql,const char *format,...)
{
	int column;
	va_list ap;

	if(!db||!sql)
	{
		printf("sqlparams: bad database/query handle\n");
		return -1;
	}

	if(sqlite3_reset(sql)!=SQLITE_OK)
	{
		printf("sql reset: %s\n",sqlite3_errmsg(db));
		return -1;
	}

	if(format)
	{
		column=0;
		va_start(ap,format);
		while(*format)switch(*format++)
		{
		case 'i':
			sqlite3_bind_int(sql,++column,va_arg(ap,int));
			break;

		case 't':
			sqlite3_bind_text(sql,++column,va_arg(ap,char *),-1,
				SQLITE_STATIC);
			break;

		default:printf("unknown format %c\n",format[-1]);
			va_end(ap);
			return -1;
		}
		va_end(ap);
	}
	return 0;
}
