/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

int daemon(int nochdir, int noclose)
{
	switch(fork())
	{
	case -1:return -1;
	case 0:	break;
	default:_exit(0);
	}
	if(setsid()==-1)return -1;
	if(!nochdir)chdir("/");
	if(!noclose)
	{
		close(0);
		close(1);
		close(2);
		open("/dev/null",O_RDONLY);
		open("/dev/null",O_WRONLY);
		open("/dev/null",O_WRONLY);
	}
	return 0;
}
