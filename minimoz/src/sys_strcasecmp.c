/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include "lib.h"

int strcasecmp(const char *s1, const char *s2)
{
	char c1;
	char c2;

	for(;*s1&&*s2;s1++,s2++)
	{
		c1=((*s1>='a'&&*s1<='z')?*s1-('a'-'A'):*s1);
		c2=((*s2>='a'&&*s2<='z')?*s2-('a'-'A'):*s2);
		if(c1>c2)return 1;
		else if(c1<c2)return -1;
	}
	c1=((*s1>='a'&&*s1<='z')?*s1-('a'-'A'):*s1);
	c2=((*s2>='a'&&*s2<='z')?*s2-('a'-'A'):*s2);
	if(c1>c2)return 1;
	else if(c1<c2)return -1;
	else return 0;
}
