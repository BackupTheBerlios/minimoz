/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <unistd.h>
#include "lib.h"

int setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
	gid_t crgid;
	gid_t cegid;

	crgid=getgid();
	cegid=getegid();
	if(!crgid&&cegid)
	{
		setegid(0);
		if(rgid!=-1)setgid(rgid);
		if(egid!=-1)setegid(egid);
		else setegid(cegid);
	}
	else
	{
		if(rgid!=-1)setgid(rgid);
		if(egid!=-1)setegid(egid);
	}
	if(egid!=-1)if(getegid()!=egid)return -1;
	if(rgid!=-1)if(getgid()!=rgid)return -1;
	return 0;
}
