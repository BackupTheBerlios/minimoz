/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

int sqlreset(char *database,char *user,char *password)
{
	int l;
	char *p;
	char *q;
	char *host;
	char *port;
	PGconn *db;
	PGresult *s;

	if(!(p=strdup(database)))
	{
		printf("sqlreset: out of memory\n");
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
	if(!(db=PQsetdbLogin(host,port,NULL,NULL,"template1",user,password)))
	{
		printf("sqlreset: out of memory\n");
		free(p);
		return -1;
	}
	if(PQstatus(db)!=CONNECTION_OK)
	{
		printf("sqlreset: %s",PQerrorMessage(db));
		PQfinish(db);
		free(p);
		return -1;
	}
	l=strlen(database);
	if(!(q=malloc(l+15)))
	{
		printf("sqlreset: out of memory\n");
		PQfinish(db);
		free(p);
		return -1;
	}
	strcpy(q,"DROP DATABASE ");
	strcpy(q+14,database);
	s=PQexec(db,q);
	if(!s)
	{
		printf("sqlreset: out of memory\n");
		PQfinish(db);
		free(q);
		free(p);
		return -1;
	}
	PQclear(s);
	free(q);
	if(!(q=malloc(l+41)))
	{
		printf("sqlreset: out of memory\n");
		PQfinish(db);
		free(p);
		return -1;
	}
	strcpy(q,"CREATE DATABASE ");
	strcpy(q+16,database);
	strcpy(q+16+l," WITH encoding='UNICODE'");
	s=PQexec(db,q);
	if(!s)
	{
		printf("sqlreset: out of memory\n");
		PQfinish(db);
		free(q);
		free(p);
		return -1;
	}
	if(PQresultStatus(s)!=PGRES_COMMAND_OK)
	{
		printf("sqlreset: CREATE DATABASE: %s",PQresultErrorMessage(s));
		PQclear(s);
		PQfinish(db);
		free(q);
		free(p);
		return -1;
	}
	PQclear(s);
	PQfinish(db);
	free(q);
	free(p);
	return 0;
}
