/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <string.h>
#include "lib.h"

#ifndef NULL
#define NULL 0L
#endif

char *strtok_r(char *s, const char *delim, char **ptrptr)
{
	char *t;
	int i;

	if(s)*ptrptr=s;
	for(s=*ptrptr;*s;s++)
	{
		for(i=0;delim[i];i++)if(*s==delim[i])break;
		if(!delim[i])break;
	}
	if(!*s)return NULL;
	for(t=s+1;*t;t++)
	{
		for(i=0;delim[i];i++)if(*t==delim[i])break;
		if(delim[i])break;
	}
	if(*t)*t++=0;
	*ptrptr=t;
	return s;
}
