/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lib.h"
#include "db.h"

#define CHUNKSIZE 1024

typedef struct _item
{
	struct _item *next;
	struct _item *l;
	struct _item *m;
	int catid;
} ITEM;

typedef struct _chunk
{
	struct _chunk *next;
	int used;
	ITEM data[CHUNKSIZE];
} CHUNK;

static CHUNK *anchor=NULL;
static ITEM *root=NULL;
static ITEM *first=NULL;
static ITEM *last=NULL;

static int additem(int catid)
{
	ITEM **r;
	CHUNK *c;

	for(r=&root;*r;)if((*r)->catid==catid)return 1;
	else if((*r)->catid<catid)r=&(*r)->m;
	else r=&(*r)->l;
	if(!anchor)
	{
		if(!(c=malloc(sizeof(CHUNK))))
		{
			fprintf(stderr,"out of memory\n");
			return -1;
		}
		c->next=anchor;
		c->used=0;
		anchor=c;
	}
	else if(anchor->used==CHUNKSIZE)
	{
		if(!(c=malloc(sizeof(CHUNK))))
		{
			fprintf(stderr,"out of memory\n");
			return -1;
		}
		c->next=anchor;
		c->used=0;
		anchor=c;
	}
	*r=&anchor->data[(anchor->used)++];
	(*r)->next=NULL;
	(*r)->l=NULL;
	(*r)->m=NULL;
	(*r)->catid=catid;
	if(!first)first=last=*r;
	else
	{
		last->next=*r;
		last=*r;
	}
	return 0;
}

static void freelist(void)
{
	CHUNK *r;
	CHUNK *i;

	for(r=anchor;r;)
	{
		i=r;
		r=r->next;
		free(i);
	}
}

static int walkdb(DB db,int catid,int altlang)
{
	int id;
	SQL sql;

	if(sqlprep(db,&sql,"SELECT dst FROM narrow WHERE src=?","i","i",catid))
	{
		fprintf(stderr,"sql error\n");
		return -1;
	}

	while(!sqlrow(db,sql,"i",&id))switch(additem(id))
	{
	case 0:	if(!walkdb(db,id,altlang))break;
	case -1:sqlfin(db,sql);
		return -1;
	}

	sqlfin(db,sql);

	if(sqlprep(db,&sql,"SELECT dst FROM letterbar WHERE src=?","i","i",
		catid))
	{
		fprintf(stderr,"sql error\n");
		return -1;
	}

	while(!sqlrow(db,sql,"i",&id))switch(additem(id))
	{
	case 0:	if(!walkdb(db,id,altlang))break;
	case -1:sqlfin(db,sql);
		return -1;
	}

	sqlfin(db,sql);
	if(!altlang)return 0;

	if(sqlprep(db,&sql,"SELECT dst FROM langlinks WHERE src=?","i","i",
		catid))
	{
		fprintf(stderr,"sql error\n");
		return -1;
	}

	while(!sqlrow(db,sql,"i",&id))switch(additem(id))
	{
	case 0:	if(!walkdb(db,id,altlang))break;
	case -1:sqlfin(db,sql);
		return -1;
	}

	sqlfin(db,sql);
	return 0;
}

static int buildlist(char *tree,char *database,char *user,char *pass,
	int altlang)
{
	int catid;
	char *p;
	DB db;
	SQL sql;

	if(sqlopen(database,user,pass,&db,SQLFLAGS_STDMEM))
	{
		fprintf(stderr,"database access error\n");
		return -1;
	}
	if(!(p=malloc(strlen(tree)+5)))
	{
		fprintf(stderr,"out of memory\n");
		sqlclose(db,0);
		return -1;
	}
	strcpy(p,"Top/");
	strcat(p,tree);

	if(sqlprep(db,&sql,"SELECT catid FROM topics WHERE id=?","t","t",p))
	{
		fprintf(stderr,"sql error\n");
		sqlclose(db,0);
		free(p);
		return -1;
	}
	switch(sqlrow(db,sql,"i",&catid))
	{
	case -1:fprintf(stderr,"sql error\n");
		sqlfin(db,sql);
		sqlclose(db,0);
		free(p);
		return -1;

	case 1:	fprintf(stderr,"Warning: %s not found\n",tree);
		sqlfin(db,sql);
		sqlclose(db,0);
		free(p);
		return 0;

	case 0:	sqlfin(db,sql);
		free(p);
		break;
	}

	switch(additem(catid))
	{
	case 0:	if(!walkdb(db,catid,altlang))break;
	case -1:sqlclose(db,0);
		return -1;
	}

	sqlclose(db,0);
	return 0;
}

static int dumpurls(char *database,char *user,char *pass)
{
	DB db;
	SQL sql;
	ITEM *r;
	char *url;

	if(sqlopen(database,user,pass,&db,SQLFLAGS_STDMEM))
	{
		fprintf(stderr,"database access error\n");
		return -1;
	}
	if(sqlprep(db,&sql,"SELECT link FROM externalpages WHERE catid=?","t",
		NULL))
	{
		fprintf(stderr,"sql error\n");
		sqlclose(db,0);
		return -1;
	}
	for(r=first;r;r=r->next)
	{
		if(sqlparams(db,sql,"i",r->catid))
		{
			fprintf(stderr,"sql error\n");
			sqlclose(db,0);
			return -1;
		}
		while(!sqlrow(db,sql,"t",&url))printf("%s\n",url);
	}
	sqlfin(db,sql);
	sqlclose(db,0);
	return 0;
}

static int allurls(char *database,char *user,char *pass)
{
	DB db;
	SQL sql;
	char *url;

	if(sqlopen(database,user,pass,&db,SQLFLAGS_STDMEM))
	{
		fprintf(stderr,"database access error\n");
		return -1;
	}
	if(sqlprep(db,&sql,"SELECT link FROM externalpages",NULL,NULL))
	{
		fprintf(stderr,"sql error\n");
		sqlclose(db,0);
		return -1;
	}
	while(!sqlrow(db,sql,"t",&url))printf("%s\n",url);
	sqlfin(db,sql);
	sqlclose(db,0);
	return 0;
}

static void usage(void)
{
	fprintf(stderr,"Usage: moztract [-a] [-d db] [-u user]"
		       " [-p password|-] <tree> [...]\n"
		       "                '-' as password means read from stdin\n"
		       "                -a  extract alternate languages, too\n"
		       "                <tree> is either a minimoz URL or the"
		       " URI of this url, e.g.\n                '/World/'\n");
	freelist();
	exit(1);
}

int main(int argc,char *argv[])
{
	int c;
	int i;
	int altlang;
	int toplevel;
	int err;
	char *db;
	char *user;
	char *pass;
	char bfr[256];

	altlang=0;
	toplevel=0;
	err=1;
	db=SQLDFLT_DB;
	user=SQLDFLT_USER;
	pass=SQLDFLT_PASS;

	while((c=getopt(argc,argv,"ad:u:p:"))!=-1)switch(c)
	{
	case 'a':
		altlang=1;
		break;

	case 'd':
		db=optarg;
		break;

	case 'u':
		user=optarg;
		break;

	case 'p':
		if(!strcmp(optarg,"-"))
		{
			if(!fgets(bfr,sizeof(bfr),stdin))
			{
				memset(bfr,0,sizeof(bfr));
				usage();
			}
			for(i=0;bfr[i];i++);
			if(!i)usage();
			if(bfr[i-1]!='\n')
			{
				memset(bfr,0,sizeof(bfr));
				usage();
			}
			bfr[i-1]=0;
			pass=bfr;
			break;
		}
		pass=optarg;
		break;

	default:if(pass)memset(pass,0,strlen(pass));
		usage();
	}

	if(optind==argc)
	{
		if(pass)memset(pass,0,strlen(pass));
		usage();
	}

	for(c=optind;c<argc;c++)
	{
		if(!strncasecmp(argv[c],"http://",7))
		{
			argv[c]+=7;
			if(!(argv[c]=strchr(argv[c],'/')))
			{
				toplevel=1;
				continue;
			}
		}
		if(!argv[c][0]||!strcmp(argv[c],"/"))
		{
			toplevel=1;
			continue;
		}
		if(argv[c][0]=='/')argv[c]++;
		if(argv[c][strlen(argv[c])-1]=='/')argv[c][strlen(argv[c])-1]=0;
		if(!argv[c][0])
		{
			toplevel=1;
			continue;
		}
		if(buildlist(argv[c],db,user,pass,altlang))goto out;
	}

	if(toplevel)
	{
		if(allurls(db,user,pass))goto out;
	}
	else if(dumpurls(db,user,pass))goto out;

	err=0;

out:	if(pass)memset(pass,0,strlen(pass));
	freelist();
	return err;
}
