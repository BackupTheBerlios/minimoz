/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlunlock(DB db)
{
	PGresult *s;

	if(!db)
	{
		printf("sqlunlock: database was not open\n");
		return -1;
	}

	if(PQtransactionStatus(db)==PQTRANS_INTRANS)
	{
		s=PQexec(db,"COMMIT");
		if(!s)
		{
			printf("sqlunlock: out of memory\n");
			return -1;
		}
		if(PQresultStatus(s)!=PGRES_COMMAND_OK)
		{
			printf("sqlunlock COMMIT: %s",PQresultErrorMessage(s));
			PQclear(s);
			return -1;
		}
		PQclear(s);
		s=PQexec(db,"BEGIN");
		if(!s)
		{
			printf("sqlunlock: out of memory\n");
			return -1;
		}
		if(PQresultStatus(s)!=PGRES_COMMAND_OK)
		{
			printf("sqlunlock BEGIN: %s",PQresultErrorMessage(s));
			PQclear(s);
			return -1;
		}
		PQclear(s);
	}
	return 0;
}
