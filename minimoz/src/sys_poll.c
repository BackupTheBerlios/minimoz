/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <sys/select.h>
#include "lib.h"

int poll(struct pollfd *ufds, unsigned int nfds, int timeout)
{
	int i;
	int maxfd;
	struct timeval *p;
	struct timeval t;
	fd_set r;
	fd_set w;
	fd_set e;

	FD_ZERO(&r);
	FD_ZERO(&w);
	FD_ZERO(&e);

	for(maxfd=0,i=0;i<nfds;i++)
	{
		if(maxfd<ufds[i].fd)maxfd=ufds[i].fd;
		ufds[i].revents=0;
		if(ufds[i].events&POLLIN)FD_SET(ufds[i].fd,&r);
		if(ufds[i].events&POLLOUT)FD_SET(ufds[i].fd,&w);
		if(ufds[i].events&(POLLERR|POLLHUP|POLLNVAL))
			FD_SET(ufds[i].fd,&e);
	}

	if(timeout<0)p=NULL;
	else
	{
		p=&t;
		t.tv_sec=timeout/1000;
		t.tv_usec=(timeout%1000)*1000;
	}

	switch(select(maxfd+1,&r,&w,&e,p))
	{
	case 0:	return 0;
	case -1:return -1;
	}

	for(maxfd=0,i=0;i<nfds;i++)
	{
		if(FD_ISSET(ufds[i].fd,&r))ufds[i].revents|=POLLIN;
		if(FD_ISSET(ufds[i].fd,&w))ufds[i].revents|=POLLOUT;
		if(FD_ISSET(ufds[i].fd,&e))
			ufds[i].revents|=POLLERR|POLLHUP|POLLNVAL;
		if(ufds[i].revents)maxfd++;
	}

	return maxfd;
}
