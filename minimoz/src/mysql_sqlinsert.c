/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

#if MYSQL_VERSION_ID >= 40113

int sqlinsert(DB db,SQL sql,char *format,...)
{
	char *p;
	int i;
	int count;
	va_list ap;

	if(!db||!sql)
	{
		printf("sqlinsert: bad database/query handle\n");
		return -1;
	}

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

	if(mysql_stmt_execute(sql->sql))
	{
		printf("sqlexecute: %s\n",mysql_stmt_error(sql->sql));
		goto out0;
	}

	return 0;

out1:	printf("sqlinsert: parameter mismatch\n");
out0:	return -1;
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

int sqlinsert(DB db,SQL sql,char *format,...)
{
	int i;
	int c;
	char *p;
	sqlchain *r;
	va_list ap;
	char cmd[8192];

	if(!db||!sql)
	{
		printf("sqlinsert: bad database/query handle\n");
		return -1;
	}

	for(i=0;;i++)if(!format[i])
	{
		va_start(ap,format);
		for(p=cmd,r=sql->sql;r;r=r->next)if(r->type==SQLTYPE_FIXED)
		{
			memcpy(p,r->u.string,r->len);
			p+=r->len;
		}
		else if(r->type!=SQLTYPE_RESULT)
		{
			if(!*format++)
			{
				printf("sql insert: format error\n");
				va_end(ap);
				goto out0;
			}
			p+=itoa(va_arg(ap,int),p);
		}
		va_end(ap);
		if(*format)
		{
			printf("sql insert: format error\n");
			goto out0;
		}
		*p=0;
		if(mysql_query(db,cmd))
		{
			printf("mysql query for %s: %s\n",cmd,mysql_error(db));
			goto out0;
		}
		return 0;
	}
	else if(format[i]!='i')break;

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
				printf("sql insert: format error\n");
				va_end(ap);
				goto out0;
			}
			if(r->type>=SQLTYPE_NUMDATA)break;
			r=r->next;
		}
		if(r->type!=SQLTYPE_NUMDATA)if(r->u.string)free(r->u.string);
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
				printf("sql insert: format error\n");
				va_end(ap);
				goto out0;
			}
			if(r->type>=SQLTYPE_NUMDATA)break;
			r=r->next;
		}
		if(r->type!=SQLTYPE_NUMDATA)if(r->u.string)free(r->u.string);
		r->type=SQLTYPE_DATA;
		p=va_arg(ap,char *);
		i=strlen(p);
		if(!(r->u.string=malloc(2*i+3)))
		{
			va_end(ap);
			goto out1;
		}
		r->len=(int)mysql_real_escape_string(db,r->u.string+1,p,i);
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
		printf("sql insert: missing data\n");
		goto out0;
	}

	for(i=1,r=sql->sql;r;r=r->next)if(r->type!=SQLTYPE_RESULT)i+=r->len;
	if(i<sizeof(cmd))p=cmd;
	else if(!(p=malloc(i)))goto out1;
	for(i=0,r=sql->sql;r;r=r->next)if(r->type!=SQLTYPE_RESULT)
	{
		memcpy(p+i,r->u.string,r->len);
		i+=r->len;
	}
	p[i]=0;
	if(mysql_query(db,p))
	{
		printf("mysql query for %s: %s\n",p,mysql_error(db));
		if(p!=cmd)free(p);
		goto out0;
	}
	if(p!=cmd)free(p);

	return 0;

out1:	printf("sqlinsert: out of memory\n");
out0:	return -1;
}

#endif
