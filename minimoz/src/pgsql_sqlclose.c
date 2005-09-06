/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlclose(DB db,int flags)
{
	int err;
	PGresult *s;

	if(!db)
	{
		printf("sqlclose: database was not open\n");
		return -1;
	}

	err=0;
	if(flags&(SQLFLAGS_COMMIT|SQLFLAGS_ROLLBACK))
	{
		s=PQexec(db,(flags&SQLFLAGS_COMMIT)?"COMMIT":"ROLLBACK");
		if(!s)
		{
			printf("sqlclose: out of memory\n");
			err=-1;
		}
		else if(PQresultStatus(s)!=PGRES_COMMAND_OK)
		{
			printf("sqlclose %s: %s",
				(flags&SQLFLAGS_COMMIT)?"COMMIT":"ROLLBACK",
				PQresultErrorMessage(s));
			PQclear(s);
			err=-1;
		}
		if(s)PQclear(s);
	}
	PQfinish(db);
	return err;
}
