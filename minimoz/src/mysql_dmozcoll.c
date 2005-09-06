/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#ifdef STANDARD
#include <stdio.h>
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;	/* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#endif
#include <mysql.h>
#include <m_string.h>

#include "lib.h"

#ifdef HAVE_DLOPEN

#ifdef	__cplusplus
extern "C" {
#endif
my_bool dmozcoll_init(UDF_INIT *initid,UDF_ARGS *args,char *message);
void dmozcoll_deinit(UDF_INIT *initid);
char *dmozcoll(UDF_INIT *initid,UDF_ARGS *args,char *result,
	unsigned long *length,char *is_null,char *error);
#ifdef	__cplusplus
}
#endif

my_bool dmozcoll_init(UDF_INIT *initid,UDF_ARGS *args,char *message)
{
	if (args->arg_count!=1||args->arg_type[0]!=STRING_RESULT)
	{
		strcpy(message,"Wrong arguments to dmozcoll");
		return 1;
	}

	initid->max_length=
		(initid->max_length>2047?2047:initid->max_length)*2+1;
	if(initid->max_length>255)
	{
		initid->ptr=malloc(initid->max_length);
		if(!initid->ptr)
		{
			strcpy(message,"Out of memory in dmozcoll");
			return 1;
		}
	}
	else initid->ptr=NULL;
	return 0;
}

void dmozcoll_deinit(UDF_INIT *initid)
{
	if(initid->ptr)free(initid->ptr);
}

char *dmozcoll(UDF_INIT *initid,UDF_ARGS *args,char *result,
	unsigned long *length,char *is_null,char *error)
{
	int l=(args->lengths[0]>2047?2047:args->lengths[0]);
	int n=0;
	int len;
	char *s=args->args[0];
	char *d=result;
	unsigned int v;

	if(!s)
	{
		*is_null=1;
		*length=0;
		return result;
	}

	if(initid->ptr)d=result=initid->ptr;

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
	l=(args->lengths[0]>2047?2047:args->lengths[0]);
	s=args->args[0];
	while(l--)
	{
		*d++=*s++;
		n++;
	}

	*length=(ulong)(n);
	return result;
}

#endif /* HAVE_DLOPEN */
