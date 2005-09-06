/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

int sqlreset(char *database,char *user,char *password)
{
	MYSQL *db;
	char *p;
	char *q;
	char *host;
	char *port;
	char *socket;
	int portn;

	if(!(p=strdup(database)))
	{
		printf("sqlreset: out of memory\n");
		return -1;
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

	if(!(db=mysql_init(NULL)))
	{
		printf("sql open of %s failed: out of memory\n",database);
		free(p);
		return -1;
	}
	if(!mysql_real_connect(db,host,user,password,NULL,portn,socket,0))
	{
		printf("sql open of %s failed: %s\n",database,mysql_error(db));
		mysql_close(db);
		free(p);
		return -1;
	}
	if(!(q=malloc(strlen(database)+17+strlen(SQLCHSET))))
	{
		printf("sql reset of %s failed: out of memory\n",database);
		mysql_close(db);
		free(p);
		return -1;
	}
	strcpy(q,"DROP DATABASE ");
	strcpy(q+14,database);
	mysql_query(db,q);
	strcpy(q,"CREATE DATABASE ");
	strcpy(q+16,database);
	strcat(q+16,SQLCHSET);
	if(mysql_query(db,q))
	{
		printf("mysql query for %s: %s\n",q,mysql_error(db));
		mysql_close(db);
		free(q);
		free(p);
		return -1;
	}
	mysql_close(db);
	free(q);
	free(p);
	return 0;
}
