/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlfin(DB db,SQL sql)
{
	if(!db||!sql)
	{
		printf("sqlfin: bad database/query handle\n");
		return -1;
	}

	if(sqlite3_finalize(sql)!=SQLITE_OK)
	{
		printf("sql finalize: %s\n",sqlite3_errmsg(db));
		return -1;
	}
	return 0;
}
