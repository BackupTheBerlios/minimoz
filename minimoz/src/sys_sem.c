/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <unistd.h>
#include <errno.h>
#include "lib.h"

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	char u;

	if(pshared)return -1;
	sem->val=value;
	if(pipe(sem->p))return -1;
	if(pipe(sem->s))
	{
		close(sem->p[0]);
		close(sem->p[1]);
		return -1;
	}
	if(write(sem->p[1],&u,1)!=1)
	{
		close(sem->p[0]);
		close(sem->p[1]);
		close(sem->s[0]);
		close(sem->s[1]);
		return -1;
	}
	while(value--)if(write(sem->s[1],&u,1)!=1)
	{
		close(sem->p[0]);
		close(sem->p[1]);
		close(sem->s[0]);
		close(sem->s[1]);
		return -1;
	}
	return 0;
}

int sem_wait(sem_t *sem)
{
	char u;

repeat1:switch(read(sem->s[0],&u,1))
	{
	case 0:	return 0;
	case -1:if(errno!=EINTR)return 0;
		goto repeat1;
	}
repeat2:switch(read(sem->p[0],&u,1))
	{
	case 0:	return 0;
	case -1:if(errno!=EINTR)return 0;
		goto repeat2;
	}
	sem->val-=1;
	write(sem->p[1],&u,1);
	return 0;
}

int sem_post(sem_t *sem)
{
	char u;

repeat1:switch(write(sem->s[1],&u,1))
	{
	case 0:	return -1;
	case -1:if(errno!=EINTR)return -1;
		goto repeat1;
	}
repeat2:switch(read(sem->p[0],&u,1))
	{
	case 0:	return -1;
	case -1:if(errno!=EINTR)return -1;
		goto repeat2;
	}
	sem->val+=1;
	write(sem->p[1],&u,1);
	return 0;
}

int sem_getvalue(sem_t *sem, int *sval)
{
	char u;

repeat:	switch(read(sem->p[0],&u,1))
	{
	case 0:	return 0;
	case -1:if(errno!=EINTR)return 0;
	goto repeat;
	}
	*sval=sem->val;
	write(sem->p[1],&u,1);
	return 0;
}

int sem_destroy(sem_t *sem)
{
	close(sem->s[0]);
	close(sem->s[1]);
	close(sem->p[0]);
	close(sem->p[1]);
	return 0;
}
