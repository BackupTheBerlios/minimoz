/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

int sqllock(DB db,char *table)
{
	int l;
	char *p;

	if(!db)
	{
		printf("sqllock: database was not open\n");
		return -1;
	}

	l=strlen(table);
	if(!(p=malloc(l+19)))
	{
		printf("sqllock: out of memory\n");
		return -1;
	}
	strcpy(p,"LOCK TABLES ");
	strcpy(p+12,table);
	strcpy(p+12+l," WRITE");
	if(mysql_query(db,p))
	{
		printf("mysql query for %s: %s\n",p,mysql_error(db));
		free(p);
		return -1;
	}
	free(p);
	return 0;
}
