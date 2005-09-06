/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lib.h"
#include "db.h"

int sqlopen(char *database,char *user,char *password,DB *db,int flags)
{
	char *p;
	char *host;
	char *port;
	char *socket;
	int portn;

	if(!(p=strdup(database)))
	{
		printf("sqlopen: out of memory\n");
		goto out0;
	}
#ifdef ENGINE
	if((host=strchr(p,':')))database=host+1;
	host=NULL;
	port=NULL;
	socket=NULL;
#else
	if(!(port=strrchr(p,':')))
	{
		host=NULL;
		socket=NULL;
	}
	else
	{
		database=port+1;
		*port=0;
		if(!(port=strrchr(p,':')))
		{
			socket=p;
			host=NULL;
		}
		else
		{
			*port++=0;
			host=p;
			socket=NULL;
		}
	}
#endif
	portn=(port?atoi(port):0);

	if(!(*db=mysql_init(NULL)))
	{
		printf("sql open of %s failed: out of memory\n",database);
		goto out1;
	}
	if(!mysql_real_connect(*db,host,user,password,database,portn,socket,0))
	{
		printf("sql open of %s failed: %s\n",database,mysql_error(*db));
		goto out2;
	}
#if MYSQL_VERSION_ID >= 40100
	if(mysql_query(*db,"SET NAMES 'utf8'"))
	{
		printf("SET NAMES 'utf8' failed: %s\n",mysql_error(*db));
		goto out2;
	}
#endif
	free(p);
	return 0;

out2:	mysql_close(*db);
out1:	free(p);
out0:	*db=NULL;
	return -1;
}
