/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdarg.h>
#include <stdio.h>
#include "db.h"

int sqlrow(DB db,SQL sql,const char *format,...)
{
	int column;
	va_list ap;
	int *i;
	const char **c;

	if(!db||!sql)
	{
		printf("sqlrow: bad database/query handle\n");
		return -1;
	}

	switch(sqlite3_step(sql))
	{
	case SQLITE_ROW:
		if(!format)return 0;
		column=0;
		va_start(ap,format);
		while(*format)switch(*format++)
		{
		case 'i':
			i=va_arg(ap,int *);
			*i=sqlite3_column_int(sql,column++);
			break;
		case 't':
			c=va_arg(ap,const char **);
			*c=sqlite3_column_text(sql,column++);
			break;
		default:printf("unknown format %c\n",format[-1]);
			va_end(ap);
			return -1;
		}
		va_end(ap);
		return 0;
	case SQLITE_DONE:
		return 1;
	default:
		printf("sql step: %s\n",sqlite3_errmsg(db));
		return -1;
	}
}
