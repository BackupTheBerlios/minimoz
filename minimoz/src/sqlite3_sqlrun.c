/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlrun(DB db,const char *cmd)
{
	SQL sql;
	const char *unused;

	if(!db)
	{
		printf("sqlrun: database was not open\n");
		return -1;
	}

	if(sqlite3_prepare(db,cmd,-1,&sql,&unused)!=SQLITE_OK)
	{
		printf("sql prepare for %s: %s\n",cmd,sqlite3_errmsg(db));
		return -1;
	}
	if(sqlite3_step(sql)!=SQLITE_DONE)
	{
		printf("sql step for %s: %s\n",cmd,sqlite3_errmsg(db));
		sqlite3_finalize(sql);
		return -1;
	}
	if(sqlite3_finalize(sql))
	{
		printf("sql finalize for %s: %s\n",cmd,sqlite3_errmsg(db));
		return -1;
	}
	return 0;
}
