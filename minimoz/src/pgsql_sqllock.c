/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

int sqllock(DB db,char *table)
{
	int l;
	char *p;
	PGresult *s;

	if(!db)
	{
		printf("sqllock: database was not open\n");
		return -1;
	}

	l=strlen(table);
	if(!(p=malloc(37+l)))
	{
		printf("sqllock: out of memory\n");
		return -1;
	}
	strcpy(p,"LOCK TABLE ");
	strcpy(p+11,table);
	strcpy(p+11+l," IN ACCESS EXCLUSIVE MODE");
	s=PQexec(db,p);
	if(!s)
	{
		printf("sqllock: out of memory\n");
		free(p);
		return -1;
	}
	if(PQresultStatus(s)!=PGRES_COMMAND_OK)
	{
		printf("sqllock cmd %s: %s",p,PQresultErrorMessage(s));
		PQclear(s);
		free(p);
		return -1;
	}
	PQclear(s);
	free(p);
	return 0;
}
