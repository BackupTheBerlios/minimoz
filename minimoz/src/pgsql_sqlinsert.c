/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <netinet/in.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

int sqlinsert(DB db,SQL sql,char *format,...)
{
	int count;
	PGresult *s;
	va_list ap;

	if(!db||!sql)
	{
		printf("sqlinsert: bad database/query handle\n");
		return -1;
	}

	va_start(ap,format);
	for(count=0;*format;format++,count++)if(count==sql->total)
	{
		printf("sqlinsert: parameter mismatch\n");
		va_end(ap);
		return -1;
	}
	else switch(*format)
	{
	case 'i':
		sql->pv[count]=(char *)(&sql->val[count]);
		sql->pl[count]=sizeof(int);
		sql->pf[count]=1;
		sql->val[count]=htonl(va_arg(ap,int));
		break;

	case 't':
		sql->pv[count]=va_arg(ap,char *);
		sql->pl[count]=0;
		sql->pf[count]=0;
		break;

	default:printf("sqlinsert: unknown format %c\n",*format);
		va_end(ap);
		return -1;
	}
	va_end(ap);
	if(count!=sql->total)
	{
		printf("sqlinsert: parameter mismatch\n");
		return -1;
	}
	if(!(s=PQexecPrepared(db,sql->id,sql->total,
		(const char * const *)(sql->pv),sql->pl,sql->pf,1)))
	{
		printf("sqlinsert: out of memory\n");
		return -1;
	}
	if(PQresultStatus(s)!=PGRES_COMMAND_OK)
	{
		printf("sqlinsert: %s",PQresultErrorMessage(s));
		PQclear(s);
		return -1;
	}
	PQclear(s);
	return 0;
}
