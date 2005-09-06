/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <unistd.h>
#include <errno.h>
#include "lib.h"

int pthread_mutex_init(pthread_mutex_t *mutex, const void *mutexattr)
{
	char u;

	if(mutexattr)return -1;
	if(pipe(mutex->p))return -1;
	if(write(mutex->p[1],&u,1)!=1)
	{
		close(mutex->p[0]);
		close(mutex->p[1]);
		return -1;
	}
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	char u;

	while(1)switch(read(mutex->p[0],&u,1))
	{
	case 1:	return 0;
	case 0:	return -1;
	case -1:if(errno!=EINTR)return -1;
	}
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	char u;

	while(1)switch(write(mutex->p[1],&u,1))
	{
	case 1:	return 0;
	case 0:	return -1;
	case -1:if(errno!=EINTR)return -1;
	}
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	close(mutex->p[0]);
	close(mutex->p[1]);
	return 0;
}
