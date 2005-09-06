/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <unistd.h>
#include "db.h"

int sqlreset(char *database,char *user,char *password)
{
	int err=-1;
	DB db;

	unlink(database);
	if(sqlopen(database,user,password,&db,SQLFLAGS_LOWMEM))goto out0;
	if(sqlrun(db,"PRAGMA default_synchronous=OFF"))goto out1;
	if(sqlrun(db,"PRAGMA default_temp_store=3"))goto out1;
	err=0;
out1:	if(sqlclose(db,0))err=-1;
out0:	return err;
}
