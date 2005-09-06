/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>

#define ENC_GZIP	1
#define ENC_DEFLATE	2

typedef struct
{
	int repeats;
	int encoding;
	int urlc;
	char **urlv;
	int *length;
	int *rcode;
} PARAMS;

static struct
{
	int rcode;
	int index;
} rcodes[]=
{
	{100,0},
	{101,1},
	{200,2},
	{201,3},
	{202,4},
	{203,5},
	{204,6},
	{205,7},
	{206,8},
	{300,9},
	{301,10},
	{302,11},
	{303,12},
	{304,13},
	{305,14},
	{306,15},
	{307,16},
	{400,17},
	{401,18},
	{402,19},
	{403,20},
	{404,21},
	{405,22},
	{406,23},
	{407,24},
	{408,25},
	{409,26},
	{410,27},
	{411,28},
	{412,29},
	{413,30},
	{414,31},
	{415,32},
	{416,33},
	{417,34},
	{500,35},
	{501,36},
	{502,37},
	{503,38},
	{504,39},
	{505,40},
	{0,41}
};

static pthread_mutex_t mtx;
static int truncated;
static int wrongcode;
static int localfail;
static int remotefail;
static int results[42];
static unsigned long long totaltime;

static size_t headersink(void *ptr,size_t size,size_t nmemb,void *stream)
{
	int i;
	int l;
	int v;
	int *code;
	char *p;

	l=size*nmemb;
	code=(int *)(stream);
	if(*code==-1)
	{
		if(l<12)return -1;
		p=(char *)(ptr);
		if(strncmp(p,"HTTP/1.",7))return -1;
		p+=7;
		if(*p!='0'&&*p!='1')return -1;
		for(p++,i=0;i<l&&*p==' ';p++,i++);
		if(i==l)return -1;
		for(v=0;i<l&&*p>='0'&&*p<='9';p++,i++)v=v*10+*p-'0';
		if(i==l||v<100||v>999)return -1;
		*code=v;
	}
	return l;
}

static size_t datasink(void *ptr,size_t size,size_t nmemb,void *stream)
{
	int l;
	int *total;

	total=(int *)(stream);
	l=size*nmemb;
	*total+=l;
	return l;
}

static int runner(char *url,int encoding,int *code,int *total,
	unsigned int *elapsed)
{
	CURL *h;
	int err;
	struct timeval start;
	struct timeval end;

	*code=-1;
	*total=0;
	*elapsed=0;
	err=-1;

	if(!(h=curl_easy_init()))goto out0;

	if(curl_easy_setopt(h,CURLOPT_NOPROGRESS,1L))goto out1;
	if(curl_easy_setopt(h,CURLOPT_NOSIGNAL,1L))goto out1;
	if(curl_easy_setopt(h,CURLOPT_DNS_CACHE_TIMEOUT,-1L))goto out1;
	if(curl_easy_setopt(h,CURLOPT_NETRC,CURL_NETRC_IGNORED))goto out1;
	if(curl_easy_setopt(h,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_1_1))
		goto out1;
	if(curl_easy_setopt(h,CURLOPT_TIMEOUT,60))goto out1;
	if(curl_easy_setopt(h,CURLOPT_FORBID_REUSE,1L))goto out1;
	if(curl_easy_setopt(h,CURLOPT_WRITEDATA,total))goto out1;
	if(curl_easy_setopt(h,CURLOPT_WRITEFUNCTION,datasink))goto out1;
	if(curl_easy_setopt(h,CURLOPT_WRITEHEADER,code))goto out1;
	if(curl_easy_setopt(h,CURLOPT_HEADERFUNCTION,headersink))goto out1;
	if(curl_easy_setopt(h,CURLOPT_URL,url))goto out1;
	switch(encoding)
	{
	case ENC_GZIP:
		if(curl_easy_setopt(h,CURLOPT_ENCODING,"gzip"))goto out1;
		break;

	case ENC_DEFLATE:
		if(curl_easy_setopt(h,CURLOPT_ENCODING,"deflate"))goto out1;
		break;

	case ENC_GZIP|ENC_DEFLATE:
		if(curl_easy_setopt(h,CURLOPT_ENCODING,"gzip,deflate"))
			goto out1;
		break;
	}

	err=1;

	gettimeofday(&start,NULL);
	if(curl_easy_perform(h))goto out1;
	gettimeofday(&end,NULL);

	if(start.tv_usec>end.tv_usec)
	{
		end.tv_usec+=1000000;
		end.tv_sec--;
	}
	if(start.tv_sec<=end.tv_sec)
	{
		end.tv_usec-=start.tv_usec;
		end.tv_sec-=start.tv_sec;
		*elapsed=(end.tv_usec+50)/100+end.tv_sec*10000;
	}

	err=0;

out1:	curl_easy_cleanup(h);
out0:	return err;
}

static void *thread(void *prm)
{
	int i;
	int j;
	int k;
	int l;
	int repeat;
	int code;
	int total;
	unsigned int elapsed;
	PARAMS *params;

	params=(PARAMS *)(prm);
	repeat=params->repeats;

	while(repeat--)for(i=0;i<params->urlc;i++)switch(runner(params->urlv[i],
		params->encoding,&code,&total,&elapsed))
	{
	case 0:	for(j=0;rcodes[j].rcode;j++)if(rcodes[j].rcode==code)break;
		j=rcodes[j].index;
		if(total!=params->length[i])k=1;
		else k=0;
		if(code!=params->rcode[i])l=1;
		else l=0;
		pthread_mutex_lock(&mtx);
		results[j]++;
		totaltime+=elapsed;
		truncated+=k;
		wrongcode+=l;
		pthread_mutex_unlock(&mtx);
		break;

	case 1:	pthread_mutex_lock(&mtx);
		remotefail++;
		pthread_mutex_unlock(&mtx);
		break;

	case -1:pthread_mutex_lock(&mtx);
		localfail++;
		pthread_mutex_unlock(&mtx);
		break;
	}

	pthread_exit(NULL);
}

static void printit(int clients,PARAMS *p)
{
	int i;
	int f;
	unsigned long long avgtotal;

	avgtotal=totaltime/clients;
	printf("Parallel clients: %d\n",clients);
	printf("Total requests: %d\n",clients*p->repeats*p->urlc);
	printf("Average total request time per client: %llu.%04llus\n",
		avgtotal/10000,avgtotal%10000);
	avgtotal/=p->repeats*p->urlc;
	printf("Average time per request: %llu.%04llus\n",avgtotal/10000,
		avgtotal%10000);
	if(localfail)printf("Requests failed locally: %d\n",localfail);
	if(remotefail)printf("Requests failed remotely: %d\n",remotefail);
	if(wrongcode)printf("Requests with wrong status code: %d\n",wrongcode);
	if(truncated)printf("Requests with wrong body length: %d\n",truncated);
	for(i=0,f=0;rcodes[i].rcode;i++)if(results[rcodes[i].index])
	{
		if(!f++)printf("Status code totals:\n");
		printf("HTTP %03d: %d\n",rcodes[i].rcode,
			results[rcodes[i].index]);
	}
	if(results[rcodes[i].index])
		printf("Unknown result codes: %d\n",results[rcodes[i].index]);
}

static int preset(PARAMS *p)
{
	int i;
	int code;
	int total;
	int elapsed;

	for(i=0;i<p->urlc;i++)switch(runner(p->urlv[i],p->encoding,&code,&total,
		&elapsed))
	{
	case -1:printf("internal error\n");
		return -1;

	case 1:	printf("failed to process %s\n",p->urlv[i]);
		return -1;

	case 0:	p->length[i]=total;
		p->rcode[i]=code;
		break;
	}

	return 0;
}

static void usage(void)
{
	printf("Usage: loadmoz [-a|-g|-d] [-c clients] [-r repeats] <url>"
	       " [...]\n"
	       "               -a   any encoding (gzip,deflate)\n"
	       "               -g   gzip encoding\n"
	       "               -d   deflate encoding\n"
               "               -c   parallel clients (1-100, default 1)\n"
	       "               -r   request repeats (1-10000, default 1)\n");
	exit(1);
}

int main(int argc,char *argv[])
{
	int i;
	int clients;
	PARAMS p;
	pthread_t *worker;

	clients=1;
	p.repeats=1;
	p.encoding=0;

	while((i=getopt(argc,argv,"c:r:gda"))!=-1)switch(i)
	{
	case 'c':
		clients=atoi(optarg);
		break;
	case 'r':
		p.repeats=atoi(optarg);
		break;
	case 'g':
		if(p.encoding)usage();
		p.encoding=ENC_GZIP;
		break;
	case 'd':
		if(p.encoding)usage();
		p.encoding=ENC_DEFLATE;
		break;
	case 'a':
		if(p.encoding)usage();
		p.encoding=ENC_GZIP|ENC_DEFLATE;
		break;
	default:usage();
	}

	if(optind==argc)usage();
	if(clients<1||clients>100)usage();
	if(p.repeats<1||p.repeats>10000)usage();

	p.urlc=argc-optind;
	p.urlv=argv+optind;
	if(!(p.length=malloc(p.urlc*sizeof(int))))
	{
		printf("out of memory\n");
		return 1;
	}
	if(!(p.rcode=malloc(p.urlc*sizeof(int))))
	{
		printf("out of memory\n");
		free(p.length);
		return 1;
	}
	if(!(worker=malloc(clients*sizeof(pthread_t))))
	{
		printf("out of memory\n");
		free(p.rcode);
		free(p.length);
		return 1;
	}

	if(curl_global_init(CURL_GLOBAL_ALL))
	{
		printf("libcurl initalization failure\n");
		free(worker);
		free(p.rcode);
		free(p.length);
		return 1;
	}

	if(preset(&p))
	{
		curl_global_cleanup();
		free(worker);
		free(p.rcode);
		free(p.length);
		return 1;
	}

	localfail=0;
	remotefail=0;
	truncated=0;
	wrongcode=0;
	totaltime=0;
	memset(results,0,sizeof(results));
	pthread_mutex_init(&mtx,NULL);

	for(i=0;i<clients;i++)if(pthread_create(&worker[i],NULL,thread,&p))
	{
		while(i--)
		{
			pthread_cancel(worker[i]);
			pthread_join(worker[i],NULL);
		}
		printf("client startup failure\n");
		curl_global_cleanup();
		free(worker);
		free(p.rcode);
		free(p.length);
		return 1;
	}

	for(i=0;i<clients;i++)pthread_join(worker[i],NULL);

	printit(clients,&p);

	pthread_mutex_destroy(&mtx);
	curl_global_cleanup();
	free(worker);
	free(p.rcode);
	free(p.length);
	return 0;
}
