/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#ifdef MEMDEBUG
#ifndef _MEMDEBUG_INCLUDED
#define _MEMDEBUG_INCLUDED
#include <dmalloc.h>
#if defined(DMALLOC_STRDUP_MACRO) && defined(strdup)
#undef strdup
#define strdup(str) dmalloc_strdup(__FILE__, __LINE__, (str), 0)
#endif
#endif
#endif

#ifndef LIB_H
#define LIB_H

#define CHARSET         1
#define EDITOR          2
#define NEWSGROUP       3

#define LINK            1
#define PDF             2
#define RSS             3
#define ATOM            4

#define KIDS		1
#define TEEN		2
#define MTEEN		4

extern int utf8_to_ucs4(unsigned char *in,int len,unsigned int *out);
extern int ucs4_to_utf8(unsigned int *in,unsigned char *out);
extern unsigned int ucs4_normalize(unsigned int val);
extern int checkutf8str(const unsigned char *s,int *line);
extern int checkutf8data(unsigned char *s,int len);
extern int checkutf8unicodestr(const unsigned char *s);

#ifdef NEED_STRTOK_R
extern char *local_strtok_r(char *s, const char *delim, char **ptrptr);
#ifdef strtok_r
#undef strtok_r
#endif
#define strtok_r local_strtok_r
#endif

#ifdef NEED_GMTIME_R
#include <time.h>
extern struct tm *local_gmtime_r(const time_t *timep, struct tm *result);
#ifdef gmtime_r
#undef gmtime_r
#endif
#define gmtime_r local_gmtime_r
#endif

#ifdef NEED_STRFTIME
#include <time.h>
extern size_t local_strftime(char *s, size_t max, const char *format,
	const struct tm *tm);
#ifdef strftime
#undef strftime
#endif
#define strftime local_strftime
#endif

#ifdef NEED_POLL
struct local_pollfd
{
	int fd;
	short events;
	short revents;
};
extern int local_poll(struct local_pollfd *ufds, unsigned int nfds,
	int timeout);
#ifdef POLLIN
#undef POLLIN
#endif
#ifdef POLLOUT
#undef POLLOUT
#endif
#ifdef POLLERR
#undef POLLERR
#endif
#ifdef POLLHUP
#undef POLLHUP
#endif
#ifdef POLLNVAL
#undef POLLNVAL
#endif
#define POLLIN		0x0001
#define POLLOUT		0x0004
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020
#ifdef poll
#undef poll
#endif
#define poll local_poll
#ifdef pollfd
#undef pollfd
#endif
#define pollfd local_pollfd
#endif

#ifdef NEED_DAEMON
extern int local_daemon(int nochdir, int noclose);
#ifdef daemon
#undef daemon
#endif
#define daemon local_daemon
#endif

#ifdef NEED_SETRESUID
#include <sys/types.h>
extern int local_setresuid(uid_t ruid, uid_t euid, uid_t suid);
#ifdef setresuid
#undef setresuid
#endif
#define setresuid local_setresuid
#endif

#ifdef NEED_SETRESGID
#include <sys/types.h>
extern int local_setresgid(gid_t rgid, gid_t egid, gid_t sgid);
#ifdef setresgid
#undef setresgid
#endif
#define setresgid local_setresgid
#endif

#ifdef NEED_TIMES
#include <time.h>
#ifdef times
#undef times
#endif
#define times(a) time(NULL)
#define sysconf(a) 1
#endif

#ifdef NEED_STRCASECMP
extern int local_strcasecmp(const char *s1, const char *s2);
#ifdef strcasecmp
#undef strcasecmp
#endif
#define strcasecmp local_strcasecmp
#endif

#ifdef NEED_STRNCASECMP
extern int local_strncasecmp(const char *s1, const char *s2, size_t n);
#ifdef strncasecmp
#undef strncasecmp
#endif
#define strncasecmp local_strncasecmp
#endif

#ifdef NEED_MUTEX
typedef struct
{
	int p[2];
} local_pthread_mutex_t;
extern int local_pthread_mutex_init(local_pthread_mutex_t *mutex,
	const void *mutexattr);
extern int local_pthread_mutex_lock(local_pthread_mutex_t *mutex);
extern int local_pthread_mutex_unlock(local_pthread_mutex_t *mutex);
extern int local_pthread_mutex_destroy(local_pthread_mutex_t *mutex);
#ifdef pthread_mutex_t
#undef pthread_mutex_t
#endif
#ifdef pthread_mutex_init
#undef pthread_mutex_init
#endif
#ifdef pthread_mutex_lock
#undef pthread_mutex_lock
#endif
#ifdef pthread_mutex_unlock
#undef pthread_mutex_unlock
#endif
#ifdef pthread_mutex_destroy
#undef pthread_mutex_destroy
#endif
#define pthread_mutex_t local_pthread_mutex_t
#define pthread_mutex_init local_pthread_mutex_init
#define pthread_mutex_lock local_pthread_mutex_lock
#define pthread_mutex_unlock local_pthread_mutex_unlock
#define pthread_mutex_destroy local_pthread_mutex_destroy
#endif

#ifdef NEED_SEM
typedef struct
{
	int p[2];
	int s[2];
	int val;
} local_sem_t;
extern int local_sem_init(local_sem_t *sem, int pshared, unsigned int value);
extern int local_sem_wait(local_sem_t *sem);
extern int local_sem_post(local_sem_t *sem);
extern int local_sem_getvalue(local_sem_t *sem, int *sval);
extern int local_sem_destroy(local_sem_t *sem);
#ifdef sem_t
#undef sem_t
#endif
#ifdef sem_init
#undef sem_init
#endif
#ifdef sem_wait
#undef sem_wait
#endif
#ifdef sem_post
#undef sem_post
#endif
#ifdef sem_getvalue
#undef sem_getvalue
#endif
#ifdef sem_destroy
#undef sem_destroy
#endif
#define sem_t local_sem_t
#define sem_init local_sem_init
#define sem_wait local_sem_wait
#define sem_post local_sem_post
#define sem_getvalue local_sem_getvalue
#define sem_destroy local_sem_destroy
#endif

#ifdef NEED_GETOPT
extern int local_getopt(int argc, char * const argv[], const char *optstring);
extern char *local_optarg;
extern int local_optind;
extern int local_optopt;
extern int local_opterr;
#ifdef getopt
#undef getopt
#endif
#ifdef optarg
#undef optarg
#endif
#ifdef optind
#undef optind
#endif
#ifdef optopt
#undef optopt
#endif
#ifdef opterr
#undef opterr
#endif
#define getopt local_getopt
#define optarg local_optarg
#define optind local_optind
#define optopt local_optopt
#define opterr local_opterr
#endif

#endif
