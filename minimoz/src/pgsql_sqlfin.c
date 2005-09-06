/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

int sqlfin(DB db,SQL sql)
{
	int res;
	PGresult *s;
	char cmd[8192];

	if(!db||!sql)
	{
		printf("sqlfin: bad database/query handle\n");
		return -1;
	}

	res=0;
	if(sql->res)PQclear(sql->res);
	strcpy(cmd,"DEALLOCATE ");
	strcpy(cmd+11,sql->id);
	s=PQexec(db,cmd);
	if(!s)
	{
		printf("sqlfin: out of memory\n");
		res=-1;
	}
	else if(PQresultStatus(s)!=PGRES_COMMAND_OK)
	{
		printf("sqlfin of %s: %s",cmd,PQresultErrorMessage(s));
		res=-1;
	}
	if(s)PQclear(s);
	if(sql->total)
	{
		free(sql->val);
		free(sql->pf);
		free(sql->pl);
		free(sql->pv);
	}
	free(sql);
	return res;
}
