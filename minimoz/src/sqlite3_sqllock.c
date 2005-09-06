/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqllock(DB db,char *table)
{
	if(!db)
	{
		printf("sqllock: database was not open\n");
		return -1;
	}
	return 0;
}
