/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdio.h>
#include "lib.h"
#include "db.h"

static int collate(void *unused,int l1,const void *s1,int l2,const void *s2)
{
	int len1;
	int len2;
	unsigned char *str1;
	unsigned char *str2;

	len1=l1;
	len2=l2;
	str1=(unsigned char *)(s1);
	str2=(unsigned char *)(s2);
	while(len1&&len2)
	{
		int res;
		unsigned int val1;
		unsigned int val2;

		if(!(res=utf8_to_ucs4(str1,len1,&val1)))goto utf8cmp;
		len1-=res;
		str1+=res;
		if(!(res=utf8_to_ucs4(str2,len2,&val2)))goto utf8cmp;
		len2-=res;
		str2+=res;
		val1=ucs4_normalize(val1);
		val2=ucs4_normalize(val2);
		if(val1<val2)return -1;
		if(val1>val2)return 1;
	}
	if(len1)return 1;
	if(len2)return -1;

utf8cmp:len1=l1;
	len2=l2;
	str1=(unsigned char *)(s1);
	str2=(unsigned char *)(s2);
	while(len1&&len2)
	{
		if(*str1<*str2)return -1;
		if(*str1++>*str2++)return 1;
		len1--;
		len2--;
	}
	if(len1)return 1;
	if(len2)return -1;
	return 0;
}

int sqlopen(char *database,char *user,char *password,DB *db,int flags)
{
	if(sqlite3_open(database,db)!=SQLITE_OK)
	{
		printf("sql open of %s failed\n",database);
		goto out0;
	}
	sqlite3_create_collation(*db,"dmoz",SQLITE_UTF8,NULL,collate);
	if(flags&SQLFLAGS_HIGHMEM)
	{
		if(sqlrun(*db,"PRAGMA cache_size=4000"))goto out1;
	}
	else if(flags&SQLFLAGS_LOWMEM)
	{
		if(sqlrun(*db,"PRAGMA cache_size=0"))goto out1;
	}
	if(flags&SQLFLAGS_BEGIN)
	{
		if(sqlrun(*db,"BEGIN"))goto out1;
	}
	return 0;
out1:	sqlite3_close(*db);
out0:	*db=NULL;
	return -1;
}
