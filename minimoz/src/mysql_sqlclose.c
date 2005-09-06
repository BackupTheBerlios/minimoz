/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlclose(DB db,int flags)
{
	if(!db)
	{
		printf("sqlclose: database was not open\n");
		return -1;
	}
	mysql_close(db);
	return 0;
}
