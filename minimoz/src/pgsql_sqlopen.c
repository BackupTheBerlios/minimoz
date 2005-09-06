/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

static void nwhandler(void *arg,const PGresult *res)
{
}

int sqlopen(char *database,char *user,char *password,DB *db,int flags)
{
	char *p;
	char *host;
	char *port;
	PGresult *s;

	if(!(p=strdup(database)))
	{
		printf("sqlopen: out of memory\n");
		return -1;
	}
	host=NULL;
	port=strchr(p,':');
	if(port)
	{
		host=p;
		*port++=0;
		database=strchr(port,':');
		if(database)*database++=0;
		else
		{
			database=port;
			port=NULL;
		}
	}
	if(!(*db=PQsetdbLogin(host,port,NULL,NULL,database,user,password)))
	{
		printf("sqlopen: out of memory\n");
		free(p);
		goto out0;
	}
	free(p);
	if(PQstatus(*db)!=CONNECTION_OK)
	{
		printf("sqlopen: %s",PQerrorMessage(*db));
		goto out1;
	}
	PQsetNoticeReceiver(*db,nwhandler,NULL);
	if(PQsetClientEncoding(*db,"UNICODE"))
	{
		printf("set client encoding failure\n");
		goto out1;
	}
	if(flags&SQLFLAGS_BEGIN)
	{
		s=PQexec(*db,"BEGIN");
		if(!s)
		{
			printf("sqlopen: out of memory\n");
			goto out1;
		}
		if(PQresultStatus(s)!=PGRES_COMMAND_OK)
		{
			printf("sqlopen BEGIN: %s",PQresultErrorMessage(s));
			goto out2;
		}
		PQclear(s);
	}
	return 0;

out2:	PQclear(s);
out1:	PQfinish(*db);
out0:	*db=NULL;
	return -1;
}
