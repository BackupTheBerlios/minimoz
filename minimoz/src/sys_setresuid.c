/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <unistd.h>
#include "lib.h"

int setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
	uid_t cruid;
	uid_t ceuid;

	cruid=getuid();
	ceuid=geteuid();
	if(!cruid&&ceuid)
	{
		seteuid(0);
		if(ruid!=-1)setuid(ruid);
		if(euid!=-1)seteuid(euid);
		else seteuid(ceuid);
	}
	else
	{
		if(ruid!=-1)setuid(ruid);
		if(euid!=-1)seteuid(euid);
	}
	if(euid!=-1)if(geteuid()!=euid)return -1;
	if(ruid!=-1)if(getuid()!=ruid)return -1;
	return 0;
}
