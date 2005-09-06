/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <stdlib.h>
#include <stdio.h>
#include "db.h"

#if MYSQL_VERSION_ID >= 40113

int sqlfin(DB db,SQL sql)
{
	int i;

	if(!db||!sql)
	{
		printf("sqlfin: bad database/query handle\n");
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
	}
	mysql_stmt_close(sql->sql);
	if(sql->total)
	{
		free(sql->ibuf);
		free(sql->lbuf);
		free(sql->param);
	}
	free(sql);
	return 0;
}

#else

int sqlfin(DB db,SQL sql)
{
	sqlchain *r;
	sqlchain *s;

	if(!db||!sql)
	{
		printf("sqlfin: bad database/query handle\n");
		return -1;
	}

	for(r=sql->sql;r;)
	{
		s=r;
		r=r->next;
		if(s->type==SQLTYPE_RESULT)
		{
			if(s->u.r)mysql_free_result(s->u.r);
		}
		else if(s->type!=SQLTYPE_NUMDATA)if(s->u.string)
			free(s->u.string);
		free(s);
	}
	free(sql);
	return 0;
}

#endif
