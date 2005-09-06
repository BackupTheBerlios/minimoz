/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlunlock(DB db)
{
	if(!db)
	{
		printf("sqlunlock: database was not open\n");
		return -1;
	}
	return 0;
}
