/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

int sqlrow(DB db,SQL sql,const char *format,...)
{
	int count;
	int v;
	int j;
	int *i;
	unsigned char *p;
	char **t;
	va_list ap;

	if(!db||!sql)
	{
		printf("sqlrow: bad database/query handle\n");
		return -1;
	}

	if(!sql->res)
	{
		if(!(sql->res=PQexecPrepared(db,sql->id,sql->total,
			(const char * const *)(sql->pv),sql->pl,sql->pf,1)))
		{
			printf("sqlrow: out of memory\n");
			return -1;
		}
		switch(PQresultStatus(sql->res))
		{
		case PGRES_COMMAND_OK:
			return 1;

		case PGRES_TUPLES_OK:
			break;

		default:printf("sqlrow: %s",PQresultErrorMessage(sql->res));
			return -1;
		}
		sql->currow=0;
		sql->totrow=PQntuples(sql->res);
		sql->totcol=PQnfields(sql->res);
		if(!sql->totrow)return 1;
		if(!sql->totcol)
		{
			printf("sqlrow: no columns\n");
			return -1;
		}
	}
	if(sql->currow==sql->totrow)return 1;
	va_start(ap,format);
	for(count=0;*format;format++,count++)if(count==sql->totcol)
	{
		printf("sqlrow: parameter mismatch\n");
		va_end(ap);
		return -1;
	}
	else switch(*format)
	{
	case 'i':
		p=PQgetvalue(sql->res,sql->currow,count);
		j=PQgetlength(sql->res,sql->currow,count);
		for(v=0;j;p++,j--)
		{
			v<<=8;
			v|=*p;
		}
		i=va_arg(ap,int *);
		*i=v;
		break;

	case 't':
		t=va_arg(ap,char **);
		*t=PQgetvalue(sql->res,sql->currow,count);
		break;

	default:printf("sqlrow: unknown format %c\n",*format);
		va_end(ap);
		return -1;
	}
	va_end(ap);
	sql->currow+=1;
	return 0;
}
