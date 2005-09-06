/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <netinet/in.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "db.h"

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

#define h4toa(in,out) *(out)=((in)>9?(in)-10+'a':(in)+'0')
#define h8toa(in,out) h4toa((in)>>4,out);h4toa((in)&0xf,(out)+1)

int sqlprep(DB db,SQL *sql,const char *cmd,const char *tblfmt,
	const char *format,...)
{
	int l;
	int i;
	int c;
	unsigned long v;
	char *p;
	PGresult *s;
	va_list ap;
	char bfr[8192];

	if(!db)
	{
		printf("sqlprep: database was not open\n");
		*sql=NULL;
		return -1;
	}

	l=13+2*sizeof(unsigned long);
	for(c=0,i=0;cmd[i];i++,l++)if(cmd[i]=='?')
	{
		c++;
		if(c<10)l++;
		else if(c<100)l+=2;
		else if(c<1000)l+=3;
		else if(c<10000)l+=4;
		else
		{
			printf("sqlprep: parameter overflow\n");
			*sql=NULL;
			return -1;
		}
	}

	if(tblfmt)if(*tblfmt)
	{
		for(p=(char *)(tblfmt);*p;p++)switch(*p)
		{
		case 'i':
			l+=8;
			break;

		case 't':
			l+=5;
			break;

		default:printf("sqlprep: unknown format %c\n",*p);
			*sql=NULL;
			return -1;
		}
		l+=2;
	}

	if(!(*sql=malloc(sizeof(struct _sql))))
	{
		printf("sqlprep: out of memory\n");
		*sql=NULL;
		return -1;
	}
	if(c)
	{
		if(!((*sql)->pv=malloc(c*sizeof(char *))))
		{
			printf("sqlprep: out of memory\n");
			free(*sql);
			*sql=NULL;
			return -1;
		}
		if(!((*sql)->pl=malloc(c*sizeof(int))))
		{
			printf("sqlprep: out of memory\n");
			free((*sql)->pv);
			free(*sql);
			*sql=NULL;
			return -1;
		}
		if(!((*sql)->pf=malloc(c*sizeof(int))))
		{
			printf("sqlprep: out of memory\n");
			free((*sql)->pl);
			free((*sql)->pv);
			free(*sql);
			*sql=NULL;
			return -1;
		}
		if(!((*sql)->val=malloc(c*sizeof(int))))
		{
			printf("sqlprep: out of memory\n");
			free((*sql)->pf);
			free((*sql)->pl);
			free((*sql)->pv);
			free(*sql);
			*sql=NULL;
			return -1;
		}
	}
	else
	{
		(*sql)->pv=NULL;
		(*sql)->pl=NULL;
		(*sql)->pf=NULL;
		(*sql)->val=NULL;
	}
	(*sql)->id[0]='p';
	for(v=(unsigned long)(*sql),i=0;i<sizeof(unsigned long);i++)
	{
		h8toa(v&0xff,(*sql)->id+2*i+1);
		v>>=8;
	}
	(*sql)->id[2*i+1]=0;
	(*sql)->total=c;
	(*sql)->currow=0;
	(*sql)->totrow=0;
	(*sql)->totcol=0;
	(*sql)->res=NULL;

	if(l<sizeof(bfr))p=bfr;
	else if(!(p=malloc(l+1)))
	{
		printf("sqlprep: out of memory\n");
		if((*sql)->total)
		{
			free((*sql)->val);
			free((*sql)->pf);
			free((*sql)->pl);
			free((*sql)->pv);
		}
		free(*sql);
		*sql=NULL;
		return -1;
	}

	strcpy(p,"PREPARE ");
	strcpy(p+8,(*sql)->id);
	l=9+2*sizeof(unsigned long);
	if(tblfmt)if(*tblfmt)
	{
		c=0;
		p[l++]=' ';
		p[l++]='(';
		while(*tblfmt)switch(*tblfmt++)
		{
		case 'i':
			if(c++)p[l++]=',';
			strcpy(p+l,"integer");
			l+=7;
			break;

		case 't':
			if(c++)p[l++]=',';
			strcpy(p+l,"text");
			l+=4;
			break;
		}
		p[l++]=')';
	}
	strcpy(p+l," AS ");
	l+=4;
	for(c=0,i=0;cmd[i];i++,l++)if(cmd[i]=='?')
	{
		p[l]='$';
		l+=itoa(++c,p+l+1);
	}
	else p[l]=cmd[i];
	p[l]=0;

	if(!(s=PQexec(db,p)))
	{
		printf("sqlprep: out of memory\n");
		if(p!=bfr)free(p);
		if((*sql)->total)
		{
			free((*sql)->val);
			free((*sql)->pf);
			free((*sql)->pl);
			free((*sql)->pv);
		}
		free(*sql);
		*sql=NULL;
		return -1;
	}
	if(PQresultStatus(s)!=PGRES_COMMAND_OK)
	{
		printf("sqlprep of %s: %s",p,PQresultErrorMessage(s));
		PQclear(s);
		if(p!=bfr)free(p);
		if((*sql)->total)
		{
			free((*sql)->val);
			free((*sql)->pf);
			free((*sql)->pl);
			free((*sql)->pv);
		}
		free(*sql);
		*sql=NULL;
		return -1;
	}
	PQclear(s);
	if(p!=bfr)free(p);

	if(!format)return 0;

	va_start(ap,format);
	for(c=0;*format;format++,c++)if(c==(*sql)->total)
	{
		printf("sqlinsert: parameter mismatch\n");
		va_end(ap);
		goto fail;
	}
	else switch(*format)
	{
	case 'i':
		(*sql)->pv[c]=(char *)(&(*sql)->val[c]);
		(*sql)->pl[c]=sizeof(int);
		(*sql)->pf[c]=1;
		(*sql)->val[c]=htonl(va_arg(ap,int));
		break;

	case 't':
		(*sql)->pv[c]=va_arg(ap,char *);
		(*sql)->pl[c]=0;
		(*sql)->pf[c]=0;
		break;

	default:printf("sqlinsert: unknown format %c\n",*format);
		va_end(ap);
		goto fail;
	}
	va_end(ap);
	if(c!=(*sql)->total)
	{
		printf("sqlinsert: parameter mismatch\n");
		goto fail;
	}

	return 0;

fail:	strcpy(bfr,"DEALLOCATE ");
	strcpy(bfr+11,(*sql)->id);
	s=PQexec(db,bfr);
	if(s)PQclear(s);
	if((*sql)->total)
	{
		free((*sql)->val);
		free((*sql)->pf);
		free((*sql)->pl);
		free((*sql)->pv);
	}
	free(*sql);
	*sql=NULL;
	return -1;
}
