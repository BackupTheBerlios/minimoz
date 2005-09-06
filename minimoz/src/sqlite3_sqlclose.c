/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlclose(DB db,int flags)
{
	int err;

	if(!db)
	{
		printf("sqlclose: database was not open\n");
		return -1;
	}

	err=-1;
	if(flags&SQLFLAGS_COMMIT)
	{
		if(sqlrun(db,"COMMIT"))goto out0;
	}
	else if(flags&SQLFLAGS_ROLLBACK)
	{
		if(sqlrun(db,"ROLLBACK"))goto out0;
	}
	err=0;
out0:	sqlite3_close(db);
	return err;
}
