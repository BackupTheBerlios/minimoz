/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <postgres.h>
#include <fmgr.h>
#include "lib.h"
#include <stdio.h>

PG_FUNCTION_INFO_V1(dmozcoll);

Datum dmozcoll(PG_FUNCTION_ARGS)
{
	int len;
	unsigned int v;
	int32 l;
	int32 n;
	text *arg;
	char *s;
	text *res;
	char *d;

	arg=PG_GETARG_TEXT_P(0);
	l=VARSIZE(arg)-VARHDRSZ;
	n=VARHDRSZ;
	s=VARDATA(arg);
	res=(text *)palloc(2*l+1+VARHDRSZ);
	d=VARDATA(res);
	while(l)
	{
		if(!(len=utf8_to_ucs4(s,l,&v)))break;
		s+=len;
		l-=len;
		v=ucs4_normalize(v);
		len=ucs4_to_utf8(&v,d);
		d+=len;
		n+=len;
	}
	*d++=0x01;
	n++;
	l=VARSIZE(arg)-VARHDRSZ;
	s=VARDATA(arg);
	while(l--)
	{
		*d++=*s++;
		n++;
	}
	VARATT_SIZEP(res)=n;
	PG_RETURN_TEXT_P(res);
}
