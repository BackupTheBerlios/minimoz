/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "db.h"

int sqlthreadinit(void)
{
	if(!mysql_thread_safe())
	{
		printf("mysql not thread safe\n");
		return -1;
	}
	if(mysql_thread_init())
	{
		printf("sqlthreadinit: out of memory\n");
		return -1;
	}
	return 0;
}
