/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

#if MYSQL_VERSION_ID >= 40113

int sqlrow(DB db,SQL sql,const char *format,...)
{
	int i;
	my_bool w;
	int *ii;
	char **t;
	MYSQL_FIELD *f;
	va_list ap;

	if(!db||!sql)
	{
		printf("sqlrow: bad database/query handle\n");
		return -1;
	}

	if(!sql->result)
	{
		w=1;
		if(mysql_stmt_attr_set(sql->sql,STMT_ATTR_UPDATE_MAX_LENGTH,&w))
		{
			printf("sqlsetattr: %s\n",mysql_stmt_error(sql->sql));
			goto out0;
		}
		if(mysql_stmt_execute(sql->sql))
		{
			printf("sqlexecute: %s\n",mysql_stmt_error(sql->sql));
			goto out0;
		}
		if(!(sql->restotal=mysql_stmt_field_count(sql->sql)))return 1;
		if(mysql_stmt_store_result(sql->sql))
		{
			printf("sqlstore: %s\n",mysql_stmt_error(sql->sql));
			goto out0;
		}
		if(!(sql->result=mysql_stmt_result_metadata(sql->sql)))return 1;
		if(!(f=mysql_fetch_fields(sql->result)))
		{
			printf("sqlfields: %s\n",mysql_stmt_error(sql->sql));
			goto out0;
		}
		if(!(sql->reslbuf=malloc(sql->restotal*sizeof(unsigned long))))
		{
			printf("sqlrow: out of memory\n");
			goto out0;
		}
		if(!(sql->resibuf=malloc(sql->restotal*sizeof(int))))
		{
			printf("sqlrow: out of memory\n");
			goto out0;
		}
		if(!(sql->resparam=malloc(sql->restotal*sizeof(MYSQL_BIND))))
		{
			printf("sqlrow: out of memory\n");
			goto out0;
		}
		memset(sql->resparam,0,sql->restotal*sizeof(MYSQL_BIND));
		for(i=0;i<sql->restotal;i++)switch(f[i].type)
		{
		case FIELD_TYPE_TINY:
		case FIELD_TYPE_SHORT:
		case FIELD_TYPE_LONG:
		case FIELD_TYPE_LONGLONG:
		case FIELD_TYPE_INT24:
			sql->resparam[i].buffer_type=MYSQL_TYPE_LONG;
			sql->resparam[i].buffer=&sql->resibuf[i];
			break;

		case FIELD_TYPE_STRING:
		case FIELD_TYPE_VAR_STRING:
		case FIELD_TYPE_BLOB:
			sql->resparam[i].buffer_type=MYSQL_TYPE_BLOB;
			sql->resparam[i].buffer_length=f[i].max_length+1;
			sql->resparam[i].length=&sql->reslbuf[i];
			if(!(sql->resparam[i].buffer=malloc(f[i].max_length+1)))
			{
				while(i--)if(sql->resparam[i].buffer_type==
					MYSQL_TYPE_BLOB)
					free(sql->resparam[i].buffer);
				free(sql->resparam);
				sql->resparam=NULL;
				printf("sqlrow: out of memory\n");
				goto out0;
			}
			break;

		default:
			printf("sqlrow: unknown field type %d\n",f[i].type);
			return -1;
		}
		if(mysql_stmt_bind_result(sql->sql,sql->resparam))
		{
			printf("sqlresbind: %s\n",mysql_stmt_error(sql->sql));
			goto out0;
		}
	}

	if(mysql_stmt_fetch(sql->sql))return 1;

	va_start(ap,format);
	for(i=0;*format;i++)if(i==sql->restotal)
	{
		va_end(ap);
		goto out1;
	}
	else switch(*format++)
	{
	case 'i':
		if(sql->resparam[i].buffer_type!=MYSQL_TYPE_LONG)
		{
			va_end(ap);
			goto out1;
		}
		ii=va_arg(ap,int *);
		*ii=sql->resibuf[i];
		break;
		break;
	case 't':
		if(sql->resparam[i].buffer_type!=MYSQL_TYPE_BLOB)
		{
			va_end(ap);
			goto out1;
		}
		t=va_arg(ap,char **);
		*t=sql->resparam[i].buffer;
		(*t)[sql->reslbuf[i]]=0;
		break;
	default:printf("unknown format %c\n",format[-1]);
		va_end(ap);
		goto out0;
	}
	va_end(ap);

	return 0;

out1:	printf("sqlrow format mismatch\n");
out0:	return -1;
}

#else

int sqlrow(DB db,SQL sql,const char *format,...)
{
	sqlchain *q;
	sqlchain *r;
	MYSQL_ROW row;
	char *p;
	int *i;
	int j;
	char **t;
	int column;
	va_list ap;
	char cmd[8192];

	if(!db||!sql)
	{
		printf("sqlrow: bad database/query handle\n");
		return -1;
	}

	for(q=sql->sql;q;q=q->next)if(q->type==SQLTYPE_RESULT)break;
	if(!q)
	{
		printf("sql row: internal error\n");
		goto out0;
	}
	if(!q->u.r)
	{
		for(j=1,r=sql->sql;r;r=r->next)if(r->type!=SQLTYPE_RESULT)
			j+=r->len;
		if(j<sizeof(cmd))p=cmd;
		else if(!(p=malloc(j)))goto out1;
		for(j=0,r=sql->sql;r;r=r->next)if(r->type!=SQLTYPE_RESULT)
		{
			memcpy(p+j,r->u.string,r->len);
			j+=r->len;
		}
		p[j]=0;
		if(mysql_query(db,p))
		{
			printf("mysql query for %s: %s\n",p,
				mysql_error(db));
			if(p!=cmd)free(p);
			goto out0;
		}
		if(p!=cmd)free(p);
		if(!(q->u.r=mysql_store_result(db)))
		{
			q->len=0;
			if(mysql_field_count(db))goto out1;
			else return 1;
		}
		q->len=(int)mysql_num_fields(q->u.r);
	}

	if(!(row=mysql_fetch_row(q->u.r)))return 1;

	va_start(ap,format);
	for(column=0;*format;column++)switch(*format++)
	{
	case 'i':
		if(column==q->len)
		{
			printf("format error\n");
			goto out0;
		}
		i=va_arg(ap,int *);
		*i=atoi(row[column]);
		break;
	case 't':
		if(column==q->len)
		{
			printf("format error\n");
			goto out0;
		}
		t=va_arg(ap,char **);
		*t=row[column];
		break;
	default:printf("unknown format %c\n",format[-1]);
		va_end(ap);
		goto out0;
	}
	va_end(ap);

	return 0;

out1:   printf("sqlrow: out of memory\n");
out0:   return -1;
}

#endif
