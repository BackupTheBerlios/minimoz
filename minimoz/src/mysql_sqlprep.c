/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

#if MYSQL_VERSION_ID >= 40113

int sqlprep(DB db,SQL *sql,const char *cmd,const char *tblfmt,
	const char *format,...)
{
	const char *p;
	int i;
	int count;
	va_list ap;

	if(!db)
	{
		printf("sqlprep: database was not open\n");
		*sql=NULL;
		return -1;
	}

	for(count=0,p=cmd;(p=strchr(p,'?'));p++,count++);

	if(!(*sql=malloc(sizeof(struct _sql))))
	{
		printf("sqlprep: out of memory\n");
		goto out0;
	}
	(*sql)->total=count;
	(*sql)->result=NULL;
	(*sql)->resparam=NULL;
	(*sql)->reslbuf=NULL;
	(*sql)->resibuf=NULL;
	if(count)
	{
		if(!((*sql)->param=malloc(count*sizeof(MYSQL_BIND))))
		{
			printf("sqlprep: out of memory\n");
			goto out1;
		}
		if(!((*sql)->lbuf=malloc(count*sizeof(unsigned long))))
		{
			printf("sqlprep: out of memory\n");
				goto out2;
		}
		if(!((*sql)->ibuf=malloc(count*sizeof(int))))
		{
			printf("sqlprep: out of memory\n");
			goto out3;
		}
	}
	if(!((*sql)->sql=mysql_stmt_init(db)))
	{
		printf("sqlprep: out of memory\n");
		goto out4;
	}
	if(mysql_stmt_prepare((*sql)->sql,cmd,strlen(cmd)))
	{
		printf("sqlprep: %s\n",mysql_stmt_error((*sql)->sql));
		goto out5;
	}

	if(!format)return 0;

	va_start(ap,format);
	for(count=0;*format;count++)if(count==(*sql)->total)
	{
		va_end(ap);
		goto out6;
	}
	else switch(*format++)
	{
	case 'i':
		(*sql)->param[count].buffer_type=MYSQL_TYPE_LONG;
		(*sql)->param[count].buffer=&(*sql)->ibuf[count];
		(*sql)->param[count].is_null=0;
		(*sql)->param[count].length=0;
		(*sql)->param[count].is_unsigned=0;
		(*sql)->ibuf[count]=va_arg(ap,int);
		break;

	case 't':
		p=va_arg(ap,const char *);
		i=strlen(p);
		(*sql)->param[count].buffer_type=MYSQL_TYPE_BLOB;
		(*sql)->param[count].buffer=(char *)(p);
		(*sql)->param[count].buffer_length=i;
		(*sql)->param[count].is_null=0;
		(*sql)->param[count].length=&(*sql)->lbuf[count];
		(*sql)->lbuf[count]=i;
		break;

	default:printf("unknown format %c\n",format[-1]);
		va_end(ap);
		goto out5;
	}
	va_end(ap);

	if(count!=(*sql)->total)goto out6;

	if(mysql_stmt_bind_param((*sql)->sql,(*sql)->param))
	{
		printf("sqlprep: %s\n",mysql_stmt_error((*sql)->sql));
		goto out5;
	}

	return 0;

out6:	printf("sqlprep: parameter mismatch\n");
out5:	mysql_stmt_close((*sql)->sql);
out4:	if((*sql)->total)free((*sql)->ibuf);
out3:	if((*sql)->total)free((*sql)->lbuf);
out2:	if((*sql)->total)free((*sql)->param);
out1:	free(*sql);
out0:	*sql=NULL;
	return -1;
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

int sqlprep(DB db,SQL *sql,const char *cmd,const char *tblfmt,
	const char *format,...)
{
	int i;
	int j;
	char *p;
	sqlchain **r;
	sqlchain *s;
	va_list ap;

	if(!db)
	{
		printf("sqlprep: database was not open\n");
		*sql=NULL;
		return -1;
	}

	if(!(*sql=malloc(sizeof(struct _sql))))goto out1;

	r=&(*sql)->sql;
	if(!(*r=malloc(sizeof(struct _sqlchain))))goto out1;
	(*r)->next=NULL;
	(*r)->type=SQLTYPE_RESULT;
	(*r)->len=0;
	(*r)->u.r=NULL;
	r=&((*r)->next);

	for(j=i=0;cmd[i];i++)if(cmd[i]=='?')
	{
		if(j!=i)
		{
			if(!(*r=malloc(sizeof(struct _sqlchain))))goto out1;
			(*r)->next=NULL;
			(*r)->type=SQLTYPE_FIXED;
			(*r)->len=i-j;
			if(!((*r)->u.string=malloc(i-j+1)))goto out1;
			memcpy((*r)->u.string,cmd+j,i-j);
			(*r)->u.string[i-j]=0;
			r=&((*r)->next);
		}
		if(!(*r=malloc(sizeof(struct _sql))))goto out1;
		(*r)->next=NULL;
		(*r)->type=SQLTYPE_DATA;
		(*r)->len=0;
		(*r)->u.string=NULL;
		r=&((*r)->next);
		j=i+1;
	}

	if(j!=i)
	{
		if(!(*r=malloc(sizeof(struct _sqlchain))))goto out1;
		(*r)->next=NULL;
		(*r)->type=SQLTYPE_FIXED;
		(*r)->len=i-j;
		if(!((*r)->u.string=malloc(i-j+1)))goto out1;
		memcpy((*r)->u.string,cmd+j,i-j);
		(*r)->u.string[i-j]=0;
		r=&((*r)->next);
	}

	if(!format)return 0;

	j=0;
	r=&(*sql)->sql;
	va_start(ap,format);
	while(*format)switch(*format++)
	{
	case 'i':
		while(1)
		{
			if(!*r)
			{
				printf("sql prepare for %s: format error\n",
					cmd);
				va_end(ap);
				goto out0;
			}
			if((*r)->type==SQLTYPE_DATA)break;
			r=&((*r)->next);
		}
		i=va_arg(ap,int);
		if(j<64)
		{
			(*r)->type=SQLTYPE_NUMDATA;
			(*r)->u.string=(*sql)->num[j++];
		}
		else if(!((*r)->u.string=malloc(12)))
		{
			va_end(ap);
			goto out1;
		}
		(*r)->len=itoa(i,(*r)->u.string);
		r=&((*r)->next);
		break;

	case 't':
		while(1)
		{
			if(!*r)
			{
				printf("sql prepare for %s: format error\n",
					cmd);
				va_end(ap);
				goto out0;
			}
			if((*r)->type==SQLTYPE_DATA)break;
			r=&((*r)->next);
		}
		p=va_arg(ap,char *);
		i=strlen(p);
		if(!((*r)->u.string=malloc(2*i+3)))
		{
			va_end(ap);
			goto out1;
		}
		(*r)->len=(int)mysql_real_escape_string(db,(*r)->u.string+1,
			p,i);
		(*r)->u.string[0]='\'';
		(*r)->u.string[(*r)->len+1]='\'';
		(*r)->u.string[(*r)->len+2]=0;
		(*r)->len+=2;
		r=&((*r)->next);
		break;

	default:printf("unknown format %c\n",format[-1]);
		va_end(ap);
		goto out0;
	}
	va_end(ap);

	return 0;

out1:	printf("sql prepare: out of memory\n");
out0:	if(*sql)
	{
		for(r=&(*sql)->sql;*r;)
		{
			s=*r;
			r=&((*r)->next);
			if(s->type!=SQLTYPE_NUMDATA)if(s->u.string)
				free(s->u.string);
			free(s);
		}
		free(*sql);
	}
	*sql=NULL;
	return -1;
}

#endif

