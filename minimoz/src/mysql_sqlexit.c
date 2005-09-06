/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdlib.h>
#include "db.h"

int sqlexit(void)
{
#ifdef ENGINE
	mysql_server_end();
	if(mysql_argv[1])free(mysql_argv[1]);
	mysql_argv[1]=NULL;
#endif
	return 0;
}
