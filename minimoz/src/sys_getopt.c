/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <sys/types.h>
#include "lib.h"

char *optarg=NULL;
int optind=1;
int optopt=0;
int opterr=1;
static char *nextchar=NULL;

int getopt(int argc, char * const argv[], const char *optstring)
{
	int i;
	int j;

	if(optind>=argc)return -1;

	if(nextchar)
	{
		i=optind;
		nextchar++;
	}
	else
	{
		for(i=optind;i<argc;i++)if(argv[i][0]=='-')
		{
			if(!argv[i][1])return -1;
			if(argv[i][1]=='-')if(!argv[i][2])return -1;
			break;
		}
		else return -1;
		if(i==argc)return -1;
		optind=i;
		nextchar=argv[i]+1;
	}

	for(j=0;optstring[j];j++)if(optstring[j]!=':')
		if(optstring[j]==*nextchar)break;
	if(!optstring[j])
	{
		optopt=*nextchar;
		if(!nextchar[1])
		{
			nextchar=NULL;
			optind++;
		}
		return '?';
	}
	if(optstring[j+1]!=':')
	{
		optarg=NULL;
		if(!nextchar[1])
		{
			nextchar=NULL;
			optind++;
		}
		return optstring[j];
	}
	else if(nextchar[1])
	{
		optarg=nextchar+1;
		nextchar=NULL;
		optind++;
		return optstring[j];
	}
	else if(i+1==argc)
	{
		optopt=*nextchar;
		optarg=NULL;
		nextchar=NULL;
		optind++;
		return '?';
	}
	else
	{
		nextchar=NULL;
		optarg=argv[i+1];
		optind+=2;
		return optstring[j];
	}
}
