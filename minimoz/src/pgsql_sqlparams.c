/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <netinet/in.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

int sqlparams(DB db,SQL sql,const char *format,...)
{
	int count;
	va_list ap;

	if(!db||!sql)
	{
		printf("sqlparams: bad database/query handle\n");
		return -1;
	}

	if(format)
	{
		va_start(ap,format);
		for(count=0;*format;format++,count++)if(count==sql->total)
		{
			printf("sqlparams: parameter mismatch\n");
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

		default:printf("sqlparams: unknown format %c\n",*format);
			va_end(ap);
			return -1;
		}
		va_end(ap);
		if(count!=sql->total)
		{
			printf("sqlparams: parameter mismatch\n");
			return -1;
		}
	}

	if(sql->res)
	{
		PQclear(sql->res);
		sql->res=NULL;
		sql->currow=0;
		sql->totrow=0;
		sql->totcol=0;
	}
	return 0;
}
