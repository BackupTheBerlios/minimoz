/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

#if MYSQL_VERSION_ID >= 40113

int sqlparams(DB db,SQL sql,const char *format,...)
{
	char *p;
	int i;
	int count;
	va_list ap;

	if(!db||!sql)
	{
		printf("sqlparams: bad database/query handle\n");
		return -1;
	}

	if(sql->result)
	{
		mysql_free_result(sql->result);
		if(sql->resparam)
		{
			for(i=0;i<sql->restotal;i++)
				if(sql->resparam[i].buffer_type==
					MYSQL_TYPE_BLOB)
					free(sql->resparam[i].buffer);
			free(sql->resparam);
		}
		if(sql->reslbuf)free(sql->reslbuf);
		if(sql->resibuf)free(sql->resibuf);
		sql->result=NULL;
		sql->resparam=NULL;
		sql->reslbuf=NULL;
		sql->resibuf=NULL;
	}

	if(format)
	{
		va_start(ap,format);
		for(count=0;*format;count++)if(count==sql->total)
		{
			va_end(ap);
			goto out1;
		}
		else switch(*format++)
		{
		case 'i':
			sql->param[count].buffer_type=MYSQL_TYPE_LONG;
			sql->param[count].buffer=&sql->ibuf[count];
			sql->param[count].is_null=0;
			sql->param[count].length=0;
			sql->param[count].is_unsigned=0;
			sql->ibuf[count]=va_arg(ap,int);
			break;

		case 't':
			p=va_arg(ap,char *);
			i=strlen(p);
			sql->param[count].buffer_type=MYSQL_TYPE_BLOB;
			sql->param[count].buffer=p;
			sql->param[count].buffer_length=i;
			sql->param[count].is_null=0;
			sql->param[count].length=&sql->lbuf[count];
			sql->lbuf[count]=i;
			break;

		default:printf("unknown format %c\n",format[-1]);
			va_end(ap);
			goto out0;
		}
		va_end(ap);

		if(count!=sql->total)goto out1;
	
		if(mysql_stmt_bind_param(sql->sql,sql->param))
		{
			printf("sqlbind: %s\n",mysql_stmt_error(sql->sql));
			goto out0;
		}
	}
	return 0;

out1:   printf("sqlparam: parameter mismatch\n");
out0:   return -1;
}

#else

static int itoa(int i,char *out)
{
	int c;
	int m;
	char n[12];

	if(!i)
	{
		out[0]='0';
		out[1]=0;
		return 1;
	}
	if(i<0)
	{
		out[0]='-';
		m=1;
		i=-i;
	}
	else m=0;
	for(c=0;i;c++,i/=10)n[c]='0'+i%10;
	while(c--)out[m++]=n[c];
	out[m]=0;
	return m;
}

int sqlparams(DB db,SQL sql,const char *format,...)
{
	int i;
	int c;
	char *p;
	sqlchain *r;
	va_list ap;

	if(!db||!sql)
	{
		printf("sqlparams: bad database/query handle\n");
		return -1;
	}

	if(format)
	{
		c=0;
		r=sql->sql;
		va_start(ap,format);
		while(*format)switch(*format++)
		{
		case 'i':
			while(1)
			{
				if(!r)
				{
					printf("sql params: format error\n");
					va_end(ap);
					goto out0;
				}
				if(r->type>=SQLTYPE_NUMDATA)break;
				r=r->next;
			}
			if(r->type!=SQLTYPE_NUMDATA)if(r->u.string)
				free(r->u.string);
			i=va_arg(ap,int);
			if(c<64)
			{
				r->type=SQLTYPE_NUMDATA;
				r->u.string=sql->num[c++];
			}
			else
			{
				r->type=SQLTYPE_DATA;
				if(!(r->u.string=malloc(12)))
				{
					va_end(ap);
					goto out1;
				}
			}
			r->len=itoa(i,r->u.string);
			r=r->next;
			break;

		case 't':
			while(1)
			{
				if(!r)
				{
					printf("sql params: format error\n");
					va_end(ap);
					goto out0;
				}
				if(r->type>=SQLTYPE_NUMDATA)break;
				r=r->next;
			}
			if(r->type!=SQLTYPE_NUMDATA)if(r->u.string)
				free(r->u.string);
			p=va_arg(ap,char *);
			i=strlen(p);
			if(!(r->u.string=malloc(2*i+3)))
			{
				va_end(ap);
				goto out1;
			}
			r->len=(int)mysql_real_escape_string(db,r->u.string+1,
				p,i);
			r->u.string[0]='\'';
			r->u.string[r->len+1]='\'';
			r->u.string[r->len+2]=0;
			r->len+=2;
			r=r->next;
			break;

		default:printf("unknown format %c\n",format[-1]);
			va_end(ap);
			goto out0;
		}
		va_end(ap);

		for(;r;r=r->next)if(r->type>=SQLTYPE_NUMDATA)
		{
			printf("sql params: missing data\n");
			goto out0;
		}
	}

	for(r=sql->sql;r;r=r->next)if(r->type==SQLTYPE_RESULT)
	{
		if(r->u.r)
		{
			mysql_free_result(r->u.r);
			r->u.r=NULL;
		}
		r->len=0;
		break;
	}

	return 0;

out1:	printf("sqlparams: out of memory\n");
out0:	return -1;
}

#endif
