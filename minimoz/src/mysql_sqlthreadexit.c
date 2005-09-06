/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include "db.h"

int sqlthreadexit(void)
{
	mysql_thread_end();
	return 0;
}
