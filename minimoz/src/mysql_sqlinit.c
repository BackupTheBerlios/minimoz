/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

#ifdef ENGINE
char *mysql_argv[]=
{
	"",
	NULL,
	"--skip-bdb",
	"--skip-innodb",
	"--skip-ndbcluster",
	"--skip-isam",
	"--bootstrap"
};
#endif

int sqlinit(char *database)
{
#ifdef ENGINE
	char *p;

	if(!(mysql_argv[1]=malloc(strlen(database)+12)))
	{
		printf("sqlinit: out of memory\n");
		return -1;
	}
	strcpy(mysql_argv[1],"--datadir=");
	strcpy(mysql_argv[1]+10,database);
	if((p=strchr(mysql_argv[1]+10,':')))*p=0;
	else
	{
		mysql_argv[1][10]='.';
		mysql_argv[1][11]=0;
	}
	if(!mysql_argv[1][10])
	{
		mysql_argv[1][10]='.';
		mysql_argv[1][11]=0;
	}
	my_init();
	if(mysql_server_init(7,mysql_argv,NULL))
	{
		printf("sqlinit: engine failure\n");
		free(mysql_argv[1]);
		mysql_argv[1]=NULL;
		return -1;
	}
#else
	my_init();
#endif
	return 0;
}
