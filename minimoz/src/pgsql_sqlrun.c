/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlrun(DB db,const char *cmd)
{
	PGresult *s;

	if(!db)
	{
		printf("sqlrun: database was not open\n");
		return -1;
	}

	s=PQexec(db,cmd);
	if(!s)
	{
		printf("sqlrun: out of memory\n");
		return -1;
	}
	if(PQresultStatus(s)!=PGRES_COMMAND_OK)
	{
		printf("sqlrun of %s: %s",cmd,PQresultErrorMessage(s));
		PQclear(s);
		return -1;
	}
	PQclear(s);
	return 0;
}
