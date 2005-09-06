/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlrun(DB db,const char *cmd)
{
	if(!db)
	{
		printf("sqlrun: database was not open\n");
		return -1;
	}

	if(mysql_query(db,cmd))
	{
		printf("mysql query for %s: %s\n",cmd,mysql_error(db));
		return -1;
	}
	return 0;
}
