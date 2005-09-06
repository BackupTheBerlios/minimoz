/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#define _GNU_SOURCE
#include <pthread.h>
#ifndef NEED_SEM
#include <semaphore.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef NEED_TIMES
#include <sys/times.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#ifndef NEED_POLL
#include <sys/poll.h>
#endif
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <zlib.h>
#include "lib.h"
#include "db.h"

#ifndef PTHREAD_THREADS_MAX
#define PTHREAD_THREADS_MAX 255
#endif

#define __TOSTRING(a) #a
#define TOSTRING(a) __TOSTRING(a)

#define MAXSEARCH	10
#define MAXRESULTS	1500
#define BUFSIZE		8192
#define NLSTOTAL	12
#define MAXLANG		16
#define TOPCATID	1
#define MAXHIST		256
#define IOMODE_GZIP	1
#define IOMODE_DEFLATE	2

#define QRY_PARENTID	0
#define QRY_TITLE	1
#define QRY_IDTITLE	2
#define QRY_TOPICCTR	3
#define QRY_PAGECTR	4
#define QRY_DESCINFO	5
#define QRY_DESCRIPTION	6
#define QRY_DESCNARROW	7
#define QRY_LETTERBAR	8
#define QRY_NARSYMMRG	9
#define QRY_NARSYMSEC	10
#define QRY_RELATED	11
#define QRY_LANGUAGES	12
#define QRY_EXTPAGES	13
#define QRY_NEWSGROUP	14
#define QRY_EDITOR	15
#define QRY_LASTUPDATE	16
#define QRY_GLOBAL	17
#define QRY_TOPICID	18
#define QRY_TOPICAGETOP	19
#define QRY_TOPICAGECAT	20
#define QRY_TOPICTOP	21
#define QRY_TOPICCAT	22
#define QRY_TOPICRESINS	23
#define QRY_TOPICRESSEL	24
#define QRY_PGRESAGETOP	25
#define QRY_PGRESAGECAT	26
#define QRY_PGRESTOP	27
#define QRY_PGRESCAT	28
#define QRY_PAGERESSEL	29
#define QRY_NLSSUPPORT	30
#define QRY_TOPICCATID	31
#define QRY_REDIRCATID	32
#define QRY_CLTOPICLIST	33
#define QRY_CLTOPICRES	34
#define QRY_CLRESULTS	35
#define QRY_CLPAGES	36

#define PREPTOTAL	37

#define QRY_TOPICWORDS	0
#define QRY_PAGEWORDS	1
#define QRY_PAGERESINS	2

#define TBL_TOPICLIST	0
#define TBL_TOPICRES	1
#define TBL_RESULTS	2
#define TBL_PAGES	3

#define TOTALTMPTBL	4

#define OPT_TITLEONLY	0x00000001
#define OPT_STRIPTOP	0x00000002
#define OPT_LINKSELF	0x00000004
#define OPT_SYMBOLLINK	0x00000008
#define OPT_LINKSYMBOL	0x00000010
#define OPT_PRELABEL	0x00000020
#define OPT_MEDIADATE	0x00000040
#define OPT_TYPELINK	0x00000080
#define OPT_LINKTYPE	0x00000100
#define OPT_TARGET	0x00000200
#define OPT_TOPIC	0x00000400
#define OPT_LINK	0x00000800
#define OPT_NEWS	0x00001000
#define OPT_WHERE	0x00002000
#define OPT_ADVANCED	0x00004000
#define OPT_MORE	0x00008000
#define OPT_MARK	0x00010000
#define OPT_TOTALTOPICS	0x00020000
#define OPT_TOTALPAGES	0x00040000
#define OPT_HSORT	0x00080000
#define OPT_SEARCHTYPE	0x00100000
#define OPT_KIDSSEARCH	0x00200000
#define OPT_MERGE	0x00400000
#define OPT_GLOBAL	0x00800000
#define OPT_LINKONLY	0x01000000

#define TYPE_STRING	0
#define TYPE_INTEGER	1
#define TYPE_OPTION	2

#define TAGFLAG_NAME	0x00000001
#define TAGFLAG_NLS	0x00000002
#define TAGFLAG_DATA	0x00000004
#define TAGFLAG_FIXED	0x00000008
#define TAGFLAG_PAGE	0x00000010
#define TAGFLAG_ACTION	0x00000020
#define TAGFLAG_IMG	0x00000040

#define CATALOGPAGE	0x00000001
#define INFOPAGE	0x00000002
#define ADVANCEDPAGE	0x00000004
#define QUERYPAGE	0x00000008
#define CSSPAGE		0x00000010
#define ROBOTSPAGE	0x00000020
#define ERR302PAGE	0x00000040
#define ERR400PAGE	0x00000080
#define ERR404PAGE	0x00000100
#define ERR500PAGE	0x00000200
#define ERR501PAGE	0x00000400
#define ALLPAGES	0x000007ff

#define QTYPE_CATONLY	0x00000001
#define QTYPE_SITESONLY	0x00000002
#define QTYPE_CATSITES	0x00000004
#define QTYPE_ALL	0x00000008
#define QTYPE_FULLCAT	0x00000010

#define RTYPE_HOME	0
#define RTYPE_INDEX	1
#define RTYPE_CATALOG	2
#define RTYPE_INFO	3
#define RTYPE_ADVANCED	4
#define RTYPE_QUERY	5
#define RTYPE_CSS	6
#define RTYPE_ROBOTS	7
#define RTYPE_ERR302	8
#define RTYPE_ERR400	9
#define RTYPE_ERR404	10
#define RTYPE_ERR500	11
#define RTYPE_ERR501	12
#define RTYPE_GIF	13
#define RTYPE_JPG	14
#define RTYPE_PNG	15

#define EFLAGS_CLONE	0x01
#define EFLAGS_CHAIN	0x02

#define OFLAG_OUT	0x00000001
#define OFLAG_DEP	0x00000002

typedef struct
{
	int errflag;
	int closeflag;
	int busyflag;
	int connflag;
	unsigned long ping;
	DB db;
	SQL sql[PREPTOTAL];
} DBPOOL;

typedef struct _detail
{
	struct _detail *next;
	int type;
	int topics;
	int pages;
	char *id;
	char *title;
} DETAIL;

typedef struct
{
	const char *name;
	const unsigned int bit;
} OPTION;

typedef struct
{
	const char *name;
	const int type;
	const int index;
	const unsigned int v1;
	const unsigned int v2;
} BBLOCK;

typedef struct _element
{
	struct _element *next;
	char **string;
	int *integer;
	unsigned char totalstrings;
	unsigned char flags;
	char lang[0];
} ELEMENT;

typedef struct _chain
{
	struct _chain *next;
	ELEMENT *data;
	char name[0];
} CHAIN;

typedef struct _action
{
	struct _action *next;
	void (*processor)();
	ELEMENT *data;
} ACTION;

typedef struct
{
	CHAIN **anchor;
	ACTION **page;
	char **strings;
	int *ints;
	int pages;
	int anchors;
	int usage;
} TEMPLATE;

typedef struct
{
	const char *name;
	const BBLOCK *params;
	void (*processor)();
	const int index;
	const int totalstring;
	const int totalint;
	const unsigned int flags;
	const unsigned int valid;
} TAG;

typedef struct _request
{
	int catid;
	int start;
	int ktm;
	int pagetype;
	int searchmode;
	int outputdone;
	int outputdmem;
	int iomode;
	DB db;
	SQL sql;
	TEMPLATE *tmpl;
	char *data;
	char *host;
	char *nls[NLSTOTAL];
	char *lang[MAXLANG];
	int fd;
	int ridx;
	int rlen;
	int widx;
	int wlen;
	int gzip;
	clock_t starttime;
	struct pollfd r;
	struct pollfd w;
	z_stream z;
	unsigned long crc;
	unsigned long len;
	char buf[BUFSIZE];
	char zbuf[BUFSIZE];
} REQUEST;

static void htmlclear(REQUEST *r,ELEMENT *e);
static void htmltrue(REQUEST *r,ELEMENT *e);
static void htmlfalse(REQUEST *r,ELEMENT *e);
static void htmldepclear(REQUEST *r,ELEMENT *e);
static void htmldeptrue(REQUEST *r,ELEMENT *e);
static void htmldepfalse(REQUEST *r,ELEMENT *e);
static void htmlsave(REQUEST *r,ELEMENT *e);
static void htmlrestore(REQUEST *r,ELEMENT *e);
static void htmlgifimg(REQUEST *r,ELEMENT *e);
static void htmljpgimg(REQUEST *r,ELEMENT *e);
static void htmlpngimg(REQUEST *r,ELEMENT *e);
static void htmlcsslink(REQUEST *r,ELEMENT *e);
static void htmlvar(REQUEST *r,ELEMENT *e);
static void htmlredirect(REQUEST *r,ELEMENT *e);
static void htmltitle(REQUEST *r,ELEMENT *e);
static void htmlhistory(REQUEST *r,ELEMENT *e);
static void htmltopics(REQUEST *r,ELEMENT *e);
static void htmlpages(REQUEST *r,ELEMENT *e);
static void htmldesclink(REQUEST *r,ELEMENT *e);
static void htmldesc(REQUEST *r,ELEMENT *e);
static void htmlletterbar(REQUEST *r,ELEMENT *e);
static void htmlnarsym(REQUEST *r,ELEMENT *e);
static void htmlrelated(REQUEST *r,ELEMENT *e);
static void htmlotherlang(REQUEST *r,ELEMENT *e);
static void htmlcontent(REQUEST *r,ELEMENT *e);
static void htmlnewsgroup(REQUEST *r,ELEMENT *e);
static void htmlsearchinfo(REQUEST *r,ELEMENT *e);
static void htmlexternalsearch(REQUEST *r,ELEMENT *e);
static void htmleditor(REQUEST *r,ELEMENT *e);
static void htmllastupdate(REQUEST *r,ELEMENT *e);
static void htmltermsofuse(REQUEST *r,ELEMENT *e);
static void htmlsearchform(REQUEST *r,ELEMENT *e);
static void htmladvanced(REQUEST *r,ELEMENT *e);
static void htmlengines(REQUEST *r,ELEMENT *e);
static void htmlcatlink(REQUEST *r,ELEMENT *e);
static void htmltopicquery(REQUEST *r,ELEMENT *e);
static void htmlcontentquery(REQUEST *r,ELEMENT *e);

static const struct
{
	const char *create;
	const char *truncate;
	const char *drop;
} tmptables[TOTALTMPTBL]=
{
	{
		"CREATE TEMPORARY TABLE topiclist (catid INTEGER)" SQLTBL_MEM,
		SQLSTMT_TRUNCATE("topiclist"),
		"DROP TABLE topiclist"
	},
	{
		"CREATE TEMPORARY TABLE topicresults (unused INTEGER,catid "
		"INTEGER)" SQLTBL_MEM,
		SQLSTMT_TRUNCATE("topicresults"),
		"DROP TABLE topicresults"
	},
	{
		"CREATE TEMPORARY TABLE results (weight INTEGER,pageid INTEGER)"
		SQLTBL_MEM,
		SQLSTMT_TRUNCATE("results"),
		"DROP TABLE results"
	},
	{
		"CREATE TEMPORARY TABLE pages (unused1 INTEGER,unused2 INTEGER,"
		"pageid INTEGER)" SQLTBL_MEM,
		SQLSTMT_TRUNCATE("pages"),
		"DROP TABLE pages"
	},
};

static const struct
{
	const char *sql;
	const char *fmt;
	const char *res;
} prpqry[PREPTOTAL]=
{
	{
		"SELECT parentid FROM topicref WHERE catid=? LIMIT 1",
		"i",
		"i"
	},
	{
		"SELECT title FROM topics WHERE catid=? LIMIT 1",
		"i",
		"t"
	},
	{
		"SELECT id,title FROM topics WHERE catid=? LIMIT 1",
		"i",
		"tt"
	},
	{
		"SELECT topics FROM counters WHERE catid=? LIMIT 1",
		"i",
		"i"
	},
	{
		"SELECT pages FROM counters WHERE catid=? LIMIT 1",
		"i",
		"i"
	},
	{
		"SELECT description.catid,topics.id FROM description,topics "
		"WHERE description.catid=? AND description.catid=topics.catid "
		"LIMIT 1",
		"i",
		"it"
	},
	{
		"SELECT topics.title,topics.id,description.value FROM topics,"
		"description WHERE topics.catid=? AND topics.catid="
		"description.catid LIMIT 1",
		"i",
		"ttt"
	},
	{
		"SELECT topics.title,topics.id,description.value FROM narrow,"
		"topics,description WHERE narrow.src=? AND topics.catid="
		"narrow.dst AND description.catid=narrow.dst ORDER BY "
		"topics.title",
		"i",
		"ttt"
	},
	{
		"SELECT topics.id,letterbar.title FROM letterbar,topics WHERE "
		"letterbar.src=? AND topics.catid=letterbar.dst ORDER BY "
		"letterbar.title",
		"i",
		"tt"
	},
	{
		"SELECT 2,topics.id,aliases.title AS c1,counters.topics,"
		"counters.pages" SQLSTMT_COLLATE1("aliases.title","c2")
		" FROM symlinks,aliases,topics,counters WHERE symlinks.src=? "
		"AND topics.catid=symlinks.dst AND symlinks.alias="
		"aliases.serial AND counters.catid=symlinks.dst UNION ALL "
		"SELECT 1,topics.id,topics.title AS c1,counters.topics,"
		"counters.pages" SQLSTMT_COLLATE1("topics.title","c2")
		" FROM narrow,topics,counters WHERE narrow.src=? AND "
		"topics.catid=narrow.dst AND counters.catid=narrow.dst ORDER "
		"BY " SQLSTMT_COLLATE2("c1","c2"),
		"ii",
		"ittii"
	},
	{
		"SELECT 2,topics.id,aliases.title AS c1,counters.topics,"
		"counters.pages" SQLSTMT_COLLATE1("aliases.title","c2")
		" FROM symlinks,aliases,topics,counters WHERE symlinks.src=? "
		"AND symlinks.level=? AND topics.catid=symlinks.dst AND "
		"symlinks.alias=aliases.serial AND counters.catid=symlinks.dst "
		"UNION ALL SELECT 1,topics.id,topics.title AS c1,"
		"counters.topics,counters.pages"
		SQLSTMT_COLLATE1("topics.title","c2") " FROM narrow,topics,"
		"counters WHERE narrow.src=? AND narrow.level=? AND "
		"topics.catid=narrow.dst AND counters.catid=narrow.dst ORDER "
		"BY " SQLSTMT_COLLATE2("c1","c2"),
		"iiii",
		"ittii"
	},
	{
		"SELECT topics.id,counters.topics,counters.pages FROM related,"
		"counters,topics WHERE related.src=? AND counters.catid="
		"related.dst AND topics.catid=related.dst ORDER BY "
		SQLSTMT_COLLATE("topics.id"),
		"i",
		"tii"
	},
	{
		"SELECT topics.id,languages.name,counters.topics,"
		"counters.pages FROM langlinks,topics,languages,counters "
		"WHERE langlinks.src=? AND topics.catid=langlinks.dst AND "
		"langlinks.lang=languages.id AND counters.catid=langlinks.dst "
		"ORDER BY " SQLSTMT_COLLATE("languages.name"),
		"i",
		"ttii"
	},
	{
		"SELECT externalpages.priority,externalpages.linktype,"
		"externalpages.link,externalpages.title,"
		"externalpages.description,externalpages.mediadate,topics.id,"
		"counters.topics,counters.pages FROM externalpages,topics,"
		"counters WHERE externalpages.catid=? AND topics.catid="
		"externalpages.catid AND counters.catid=externalpages.catid "
		"ORDER BY -externalpages.priority,"
		SQLSTMT_COLLATE("externalpages.title"),
		"i",
		"iitttttii"
	},
	{
		"SELECT newsgroup.name FROM chedne,newsgroup WHERE "
		"chedne.catid=? AND chedne.type=3 AND newsgroup.id=chedne.id "
		"ORDER BY " SQLSTMT_COLLATE("newsgroup.name"),
		"i",
		"t"
	},
	{
		"SELECT editor.name FROM chedne,editor WHERE chedne.catid=? "
		"AND chedne.type=2 AND editor.id=chedne.id ORDER BY "
		SQLSTMT_COLLATE("editor.name"),
		"i",
		"t"
	},
	{
		"SELECT lastupdate FROM topics WHERE catid=? LIMIT 1",
		"i",
		"t"
	},
	{
		"SELECT topics.catid,topics.title FROM topicref,topics WHERE "
		"topicref.parentid=? AND topics.catid=topicref.catid ORDER BY "
		SQLSTMT_COLLATE("topics.title"),
		"i",
		"it"
	},
	{
		"SELECT id FROM topics WHERE catid=? LIMIT 1",
		"i",
		"t"
	},
	{
		"INSERT INTO topiclist SELECT catid FROM topicsearch WHERE "
		"wordid=? AND ages=1",
		"i",
		NULL
	},
	{
		"INSERT INTO topiclist SELECT topicsearch.catid FROM "
		"topicsearch,topictrace WHERE topicsearch.wordid=? AND "
		"topicsearch.ages=1 AND topicsearch.catid=topictrace.catid "
		"AND topictrace.refid=?",
		"ii",
		NULL
	},
	{
		"INSERT INTO topiclist SELECT catid FROM topicsearch WHERE "
		"wordid=?",
		"i",
		NULL
	},
	{
		"INSERT INTO topiclist SELECT topicsearch.catid FROM "
		"topicsearch,topictrace WHERE topicsearch.wordid=? AND "
		"topicsearch.catid=topictrace.catid AND topictrace.refid=?",
		"ii",
		NULL
	},
	{
		"INSERT INTO topicresults SELECT -COUNT(catid) AS unused,"
		"catid FROM topiclist GROUP BY catid ORDER BY unused,catid",
		NULL,
		NULL
	},
	{
		"SELECT topicresults.catid,topics.id,counters.topics,"
		"counters.pages FROM topicresults,topics,counters WHERE "
		"topicresults.catid=topics.catid AND topicresults.catid="
		"counters.catid",
		NULL,
		"itii"
	},
	{
		"INSERT INTO results SELECT weight,pageid FROM pagesearch "
		"WHERE wordid=? AND ages&?!=0 LIMIT " TOSTRING(MAXRESULTS),
		"ii",
		NULL
	},
	{
		"INSERT INTO results SELECT pagesearch.weight,"
		"pagesearch.pageid FROM pagesearch,topictrace WHERE "
		"pagesearch.wordid=? AND pagesearch.ages&?!=0 AND "
		"pagesearch.catid=topictrace.catid AND topictrace.refid=? "
		"LIMIT " TOSTRING(MAXRESULTS),
		"iii",
		NULL
	},
	{
		"INSERT INTO results SELECT weight,pageid FROM pagesearch "
		"WHERE wordid=? LIMIT " TOSTRING(MAXRESULTS),
		"i",
		NULL
	},
	{
		"INSERT INTO results SELECT pagesearch.weight,"
		"pagesearch.pageid FROM pagesearch,topictrace WHERE "
		"pagesearch.wordid=? AND pagesearch.catid=topictrace.catid "
		"AND topictrace.refid=? LIMIT " TOSTRING(MAXRESULTS),
		"ii",
		NULL
	},
	{
		"SELECT pages.pageid,externalpages.linktype,"
		"externalpages.priority,externalpages.link,"
		"externalpages.title,externalpages.description,"
		"externalpages.mediadate,topics.id,counters.topics,"
		"counters.pages,counters.catid FROM pages,externalpages,"
		"topics,counters WHERE pages.pageid=externalpages.id AND "
		"externalpages.catid=topics.catid AND externalpages.catid="
		"counters.catid",
		NULL,
		"iiitttttiii"
	},
	{
		"SELECT nlssupport.id,nlssupport.seealso,nlssupport.otherlang,"
		"nlssupport.description,nlssupport.editor,"
		"nlssupport.searchonpre,nlssupport.searchonpost,"
		"nlssupport.lastupdate,nlssupport.termsofuse,"
		"nlssupport.searchfor,nlssupport.searchalldir,"
		"nlssupport.searchforin FROM nlsinfo,nlssupport WHERE "
		"nlsinfo.catid=? AND nlssupport.serial=nlsinfo.nlsid LIMIT 1",
		"i",
		"tttttttttttt"
	},
	{
		"SELECT catid FROM topics WHERE id=? LIMIT 1",
		"t",
		"i"
	},
	{
		"SELECT catid FROM redirects WHERE id=? LIMIT 1",
		"t",
		"i"
	},
	{
		"DELETE FROM topiclist",
		NULL,
		NULL
	},
	{
		"DELETE FROM topicresults",
		NULL,
		NULL
	},
	{
		"DELETE FROM results",
		NULL,
		NULL
	},
	{
		"DELETE FROM pages",
		NULL,
		NULL
	}
};

static const struct
{
	const char *pre;
	const char *post;
	const char *res;
} bldqry[3]=
{
	{
		"SELECT wordid,word FROM topicwords WHERE word IN (",
		") LIMIT " TOSTRING(MAXSEARCH),
		"it"
	},
	{
		"SELECT wordid,word FROM pagewords WHERE word IN (",
		") LIMIT " TOSTRING(MAXSEARCH),
		"it"
	},
	{
		"INSERT INTO pages SELECT -COUNT(pageid) AS unused1,"
		"-SUM(weight) AS unused2,pageid FROM results GROUP BY pageid "
		"ORDER BY unused1,unused2,pageid",
		NULL,
		NULL
	},
};

static const OPTION options[]=
{
	{"titleonly",OPT_TITLEONLY},
	{"striptop",OPT_STRIPTOP},
	{"linkself",OPT_LINKSELF},
	{"symbollink",OPT_SYMBOLLINK},
	{"linksymbol",OPT_LINKSYMBOL},
	{"prelabel",OPT_PRELABEL},
	{"mediadate",OPT_MEDIADATE},
	{"typelink",OPT_TYPELINK},
	{"linktype",OPT_LINKTYPE},
	{"target",OPT_TARGET},
	{"topic",OPT_TOPIC},
	{"link",OPT_LINK},
	{"news",OPT_NEWS},
	{"where",OPT_WHERE},
	{"advanced",OPT_ADVANCED},
	{"more",OPT_MORE},
	{"mark",OPT_MARK},
	{"totaltopics",OPT_TOTALTOPICS},
	{"totalpages",OPT_TOTALPAGES},
	{"hsort",OPT_HSORT},
	{"searchtype",OPT_SEARCHTYPE},
	{"kidssearch",OPT_KIDSSEARCH},
	{"merge",OPT_MERGE},
	{"global",OPT_GLOBAL},
	{"linkonly",OPT_LINKONLY},
	{NULL,0}
};

static const BBLOCK bbtitle[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"opts",TYPE_OPTION,0,OPT_TITLEONLY|OPT_STRIPTOP,0},
	{NULL,0,0,0}
};

static const BBLOCK bbhistory[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{"opts",TYPE_OPTION,0,OPT_LINKSELF,0},
	{NULL,0,0,0}
};

static const BBLOCK bbtopics[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"alt",TYPE_STRING,2,1,0},
	{"numsplit",TYPE_STRING,3,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbpages[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"alt",TYPE_STRING,2,1,0},
	{"numsplit",TYPE_STRING,3,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbdesclink[]=
{
	{"label",TYPE_STRING,0,0,0},
	{"pre",TYPE_STRING,1,0,0},
	{"post",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbdescription[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{"h1in",TYPE_STRING,5,0,0},
	{"h1out",TYPE_STRING,6,0,0},
	{"h2in",TYPE_STRING,7,0,0},
	{"h2out",TYPE_STRING,8,0,0},
	{NULL,0,0,0}
};

static const BBLOCK bbletterbar[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{"letterin",TYPE_STRING,5,0,0},
	{"letterout",TYPE_STRING,6,0,0},
	{"rowin",TYPE_STRING,7,1,0},
	{"rowout",TYPE_STRING,8,1,0},
	{"oddin",TYPE_STRING,9,1,0},
	{"oddout",TYPE_STRING,10,1,0},
	{"evenin",TYPE_STRING,11,1,0},
	{"evenout",TYPE_STRING,12,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbnarsym[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{"elemin",TYPE_STRING,5,0,0},
	{"elemout",TYPE_STRING,6,0,0},
	{"totalin",TYPE_STRING,7,0,0},
	{"totalout",TYPE_STRING,8,0,0},
	{"totaldelim",TYPE_STRING,9,1,0},
	{"symbolic",TYPE_STRING,10,0,0},
	{"numsplit",TYPE_STRING,11,1,0},
	{"rowin",TYPE_STRING,12,1,0},
	{"rowout",TYPE_STRING,13,1,0},
	{"oddin",TYPE_STRING,14,1,0},
	{"oddout",TYPE_STRING,15,1,0},
	{"evenin",TYPE_STRING,16,1,0},
	{"evenout",TYPE_STRING,17,1,0},
	{"col1in",TYPE_STRING,18,1,0},
	{"col1out",TYPE_STRING,19,1,0},
	{"col2in",TYPE_STRING,20,1,0},
	{"col2out",TYPE_STRING,21,1,0},
	{"col3in",TYPE_STRING,22,1,0},
	{"col3out",TYPE_STRING,23,1,0},
	{"col4in",TYPE_STRING,24,1,0},
	{"col4out",TYPE_STRING,25,1,0},
	{"level2in",TYPE_STRING,26,1,0},
	{"level2out",TYPE_STRING,27,1,0},
	{"level1in",TYPE_STRING,28,1,0},
	{"level1out",TYPE_STRING,29,1,0},
	{"level0in",TYPE_STRING,30,1,0},
	{"level0out",TYPE_STRING,31,1,0},
	{"levelin",TYPE_STRING,32,1,0},
	{"levelout",TYPE_STRING,33,1,0},
	{"colin",TYPE_STRING,34,1,0},
	{"colout",TYPE_STRING,35,1,0},
	{"opts",TYPE_OPTION,0,OPT_TOTALTOPICS|OPT_TOTALPAGES|
		OPT_SYMBOLLINK|OPT_LINKSYMBOL|OPT_HSORT|OPT_MERGE,0},
	{"2columns",TYPE_INTEGER,1,2,100},
	{"3columns",TYPE_INTEGER,2,3,100},
	{"4columns",TYPE_INTEGER,3,4,100},
	{NULL,0,0,0}
};

static const BBLOCK bbrelated[]=
{
	{"label",TYPE_STRING,0,0,0},
	{"pre",TYPE_STRING,1,0,0},
	{"post",TYPE_STRING,2,0,0},
	{"delim",TYPE_STRING,3,0,0},
	{"alt",TYPE_STRING,4,1,0},
	{"style",TYPE_STRING,5,1,0},
	{"elemin",TYPE_STRING,6,0,0},
	{"elemout",TYPE_STRING,7,0,0},
	{"totalin",TYPE_STRING,8,0,0},
	{"totalout",TYPE_STRING,9,0,0},
	{"totaldelim",TYPE_STRING,10,1,0},
	{"numsplit",TYPE_STRING,11,1,0},
	{"rowin",TYPE_STRING,12,1,0},
	{"rowout",TYPE_STRING,13,1,0},
	{"oddin",TYPE_STRING,14,1,0},
	{"oddout",TYPE_STRING,15,1,0},
	{"evenin",TYPE_STRING,16,1,0},
	{"evenout",TYPE_STRING,17,1,0},
	{"labelin",TYPE_STRING,18,1,0},
	{"labelout",TYPE_STRING,19,1,0},
	{"opts",TYPE_OPTION,0,OPT_TOTALTOPICS|OPT_TOTALPAGES|OPT_PRELABEL,0},
	{NULL,0,0,0}
};

static const BBLOCK bbotherlang[]=
{
	{"label",TYPE_STRING,0,0,0},
	{"pre",TYPE_STRING,1,0,0},
	{"post",TYPE_STRING,2,0,0},
	{"delim",TYPE_STRING,3,0,0},
	{"alt",TYPE_STRING,4,1,0},
	{"style",TYPE_STRING,5,1,0},
	{"elemin",TYPE_STRING,6,0,0},
	{"elemout",TYPE_STRING,7,0,0},
	{"totalin",TYPE_STRING,8,0,0},
	{"totalout",TYPE_STRING,9,0,0},
	{"totaldelim",TYPE_STRING,10,1,0},
	{"numsplit",TYPE_STRING,11,1,0},
	{"rowin",TYPE_STRING,12,1,0},
	{"rowout",TYPE_STRING,13,1,0},
	{"oddin",TYPE_STRING,14,1,0},
	{"oddout",TYPE_STRING,15,1,0},
	{"evenin",TYPE_STRING,16,1,0},
	{"evenout",TYPE_STRING,17,1,0},
	{"col1in",TYPE_STRING,18,1,0},
	{"col1out",TYPE_STRING,19,1,0},
	{"col2in",TYPE_STRING,20,1,0},
	{"col2out",TYPE_STRING,21,1,0},
	{"col3in",TYPE_STRING,22,1,0},
	{"col3out",TYPE_STRING,23,1,0},
	{"col4in",TYPE_STRING,24,1,0},
	{"col4out",TYPE_STRING,25,1,0},
	{"colin",TYPE_STRING,26,1,0},
	{"colout",TYPE_STRING,27,1,0},
	{"labelin",TYPE_STRING,28,1,0},
	{"labelout",TYPE_STRING,29,1,0},
	{"col5in",TYPE_STRING,30,1,0},
	{"col5out",TYPE_STRING,31,1,0},
	{"col6in",TYPE_STRING,32,1,0},
	{"col6out",TYPE_STRING,33,1,0},
	{"opts",TYPE_OPTION,0,OPT_TOTALTOPICS|OPT_TOTALPAGES|
		OPT_PRELABEL|OPT_HSORT,0},
	{"2columns",TYPE_INTEGER,1,2,100},
	{"3columns",TYPE_INTEGER,2,3,100},
	{"4columns",TYPE_INTEGER,3,4,100},
	{"5columns",TYPE_INTEGER,4,5,100},
	{"6columns",TYPE_INTEGER,5,6,100},
	{NULL,0,0,0}
};

static const BBLOCK bbcontent[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{"elemin",TYPE_STRING,5,0,0},
	{"elemout",TYPE_STRING,6,0,0},
	{"prioin",TYPE_STRING,7,0,0},
	{"prioout",TYPE_STRING,8,0,0},
	{"pdf",TYPE_STRING,9,0,0},
	{"rss",TYPE_STRING,10,0,0},
	{"atom",TYPE_STRING,11,0,0},
	{"mediadatein",TYPE_STRING,12,0,0},
	{"mediadateout",TYPE_STRING,13,0,0},
	{"targetin",TYPE_STRING,14,0,0},
	{"targetout",TYPE_STRING,15,0,0},
	{"topicdelim",TYPE_STRING,16,0,0},
	{"topicin",TYPE_STRING,17,0,0},
	{"topicout",TYPE_STRING,18,0,0},
	{"totalin",TYPE_STRING,19,0,0},
	{"totalout",TYPE_STRING,20,0,0},
	{"totaldelim",TYPE_STRING,21,1,0},
	{"numsplit",TYPE_STRING,22,1,0},
	{"rowin",TYPE_STRING,23,1,0},
	{"rowout",TYPE_STRING,24,1,0},
	{"oddin",TYPE_STRING,25,1,0},
	{"oddout",TYPE_STRING,26,1,0},
	{"evenin",TYPE_STRING,27,1,0},
	{"evenout",TYPE_STRING,28,1,0},
	{"opts",TYPE_OPTION,0,OPT_MEDIADATE|OPT_TYPELINK|OPT_LINKTYPE|
		OPT_TARGET|OPT_TOPIC|OPT_TOTALTOPICS|OPT_TOTALPAGES|OPT_LINK,0},
	{NULL,0,0,0}
};

static const BBLOCK bbnewsgroup[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{"elemin",TYPE_STRING,5,0,0},
	{"elemout",TYPE_STRING,6,0,0},
	{"rowin",TYPE_STRING,7,1,0},
	{"rowout",TYPE_STRING,8,1,0},
	{"oddin",TYPE_STRING,9,1,0},
	{"oddout",TYPE_STRING,10,1,0},
	{"evenin",TYPE_STRING,11,1,0},
	{"evenout",TYPE_STRING,12,1,0},
	{"opts",TYPE_OPTION,0,OPT_NEWS,0},
	{NULL,0,0,0}
};

static const BBLOCK bbsearchinfo[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"alt",TYPE_STRING,2,1,0},
	{"searchfor",TYPE_STRING,3,0,0},
	{"searchforin",TYPE_STRING,4,0,0},
	{"searchforout",TYPE_STRING,5,0,0},
	{"wrap",TYPE_INTEGER,0,10,100},
	{NULL,0,0,0}
};

static const BBLOCK bbexternalsearch[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{"searchfor",TYPE_STRING,5,0,0},
	{"rowin",TYPE_STRING,6,1,0},
	{"rowout",TYPE_STRING,7,1,0},
	{"oddin",TYPE_STRING,8,1,0},
	{"oddout",TYPE_STRING,9,1,0},
	{"evenin",TYPE_STRING,10,1,0},
	{"evenout",TYPE_STRING,11,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbeditor[]=
{
	{"label",TYPE_STRING,0,0,0},
	{"pre",TYPE_STRING,1,0,0},
	{"post",TYPE_STRING,2,0,0},
	{"delim",TYPE_STRING,3,0,0},
	{"alt",TYPE_STRING,4,1,0},
	{"style",TYPE_STRING,5,1,0},
	{"rowin",TYPE_STRING,6,1,0},
	{"rowout",TYPE_STRING,7,1,0},
	{"oddin",TYPE_STRING,8,1,0},
	{"oddout",TYPE_STRING,9,1,0},
	{"evenin",TYPE_STRING,10,1,0},
	{"evenout",TYPE_STRING,11,1,0},
	{"opts",TYPE_OPTION,0,OPT_PRELABEL,0},
	{NULL,0,0,0}
};

static const BBLOCK bblastupdate[]=
{
	{"label",TYPE_STRING,0,0,0},
	{"pre",TYPE_STRING,1,0,0},
	{"post",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"opts",TYPE_OPTION,0,OPT_PRELABEL,0},
	{NULL,0,0,0}
};

static const BBLOCK bbtermsofuse[]=
{
	{"label",TYPE_STRING,0,0,0},
	{"pre",TYPE_STRING,1,0,0},
	{"post",TYPE_STRING,2,0,0},
	{"style",TYPE_STRING,3,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbsearchform[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"style",TYPE_STRING,2,1,0},
	{"inputin",TYPE_STRING,3,0,0},
	{"inputout",TYPE_STRING,4,0,0},
	{"submit",TYPE_STRING,5,0,0},
	{"submitin",TYPE_STRING,6,0,0},
	{"submitout",TYPE_STRING,7,0,0},
	{"selectall",TYPE_STRING,8,0,0},
	{"selectonlyin",TYPE_STRING,9,0,0},
	{"selectin",TYPE_STRING,10,0,0},
	{"selectout",TYPE_STRING,11,0,0},
	{"advanced",TYPE_STRING,12,0,0},
	{"advancedin",TYPE_STRING,13,0,0},
	{"advancedout",TYPE_STRING,14,0,0},
	{"opts",TYPE_OPTION,0,OPT_WHERE|OPT_ADVANCED|OPT_GLOBAL,0},
	{"width",TYPE_INTEGER,1,10,100},
	{NULL,0,0,0}
};

static const BBLOCK bbadvanced[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"inputin",TYPE_STRING,2,0,0},
	{"inputout",TYPE_STRING,3,0,0},
	{"submit",TYPE_STRING,4,0,0},
	{"submitin",TYPE_STRING,5,0,0},
	{"submitout",TYPE_STRING,6,0,0},
	{"category",TYPE_STRING,7,0,0},
	{"categoryall",TYPE_STRING,8,0,0},
	{"categoryin",TYPE_STRING,9,0,0},
	{"categoryout",TYPE_STRING,10,0,0},
	{"modelabel",TYPE_STRING,11,0,0},
	{"modecategories",TYPE_STRING,12,0,0},
	{"modesites",TYPE_STRING,13,0,0},
	{"modeboth",TYPE_STRING,14,0,0},
	{"modein",TYPE_STRING,15,0,0},
	{"modeout",TYPE_STRING,16,0,0},
	{"ktmlabel",TYPE_STRING,17,0,0},
	{"ktmkids",TYPE_STRING,18,0,0},
	{"ktmteens",TYPE_STRING,19,0,0},
	{"ktmmteens",TYPE_STRING,20,0,0},
	{"ktmin",TYPE_STRING,21,0,0},
	{"ktmout",TYPE_STRING,22,0,0},
	{"opts",TYPE_OPTION,0,OPT_SEARCHTYPE|OPT_KIDSSEARCH,0},
	{"width",TYPE_INTEGER,1,10,100},
	{NULL,0,0,0}
};

static const BBLOCK bbengines[]=
{
	{"label",TYPE_STRING,0,0,0},
	{"pre",TYPE_STRING,1,0,0},
	{"post",TYPE_STRING,2,0,0},
	{"delim",TYPE_STRING,3,0,0},
	{"alt",TYPE_STRING,4,1,0},
	{"style",TYPE_STRING,5,1,0},
	{"rowin",TYPE_STRING,6,1,0},
	{"rowout",TYPE_STRING,7,1,0},
	{"oddin",TYPE_STRING,8,1,0},
	{"oddout",TYPE_STRING,9,1,0},
	{"evenin",TYPE_STRING,10,1,0},
	{"evenout",TYPE_STRING,11,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbcatlink[]=
{
	{"label",TYPE_STRING,0,0,0},
	{"pre",TYPE_STRING,1,0,0},
	{"post",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbtopicquery[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{"listin",TYPE_STRING,5,0,0},
	{"listout",TYPE_STRING,6,0,0},
	{"elemin",TYPE_STRING,7,0,0},
	{"elemout",TYPE_STRING,8,0,0},
	{"totalin",TYPE_STRING,9,0,0},
	{"totalout",TYPE_STRING,10,0,0},
	{"totaldelim",TYPE_STRING,11,0,0},
	{"more",TYPE_STRING,12,0,0},
	{"morein",TYPE_STRING,13,0,0},
	{"moreout",TYPE_STRING,14,0,0},
	{"numsplit",TYPE_STRING,15,1,0},
	{"rowin",TYPE_STRING,16,1,0},
	{"rowout",TYPE_STRING,17,1,0},
	{"oddin",TYPE_STRING,18,1,0},
	{"oddout",TYPE_STRING,19,1,0},
	{"evenin",TYPE_STRING,20,1,0},
	{"evenout",TYPE_STRING,21,1,0},
	{"markin",TYPE_STRING,22,0,0},
	{"markout",TYPE_STRING,23,0,0},
	{"opts",TYPE_OPTION,0,OPT_TOTALTOPICS|OPT_TOTALPAGES|OPT_LINK|
		OPT_MORE|OPT_MARK,0},
	{"limit",TYPE_INTEGER,1,10,100},
	{NULL,0,0,0}
};

static const BBLOCK bbcontentquery[]=
{
	{"pre",TYPE_STRING,0,0,0},
	{"post",TYPE_STRING,1,0,0},
	{"delim",TYPE_STRING,2,0,0},
	{"alt",TYPE_STRING,3,1,0},
	{"style",TYPE_STRING,4,1,0},
	{"listin",TYPE_STRING,5,0,0},
	{"listout",TYPE_STRING,6,0,0},
	{"elemin",TYPE_STRING,7,0,0},
	{"elemout",TYPE_STRING,8,0,0},
	{"markin",TYPE_STRING,9,0,0},
	{"markout",TYPE_STRING,10,0,0},
	{"pdf",TYPE_STRING,11,0,0},
	{"rss",TYPE_STRING,12,0,0},
	{"atom",TYPE_STRING,13,0,0},
	{"mediadatein",TYPE_STRING,14,0,0},
	{"mediadateout",TYPE_STRING,15,0,0},
	{"targetin",TYPE_STRING,16,0,0},
	{"targetout",TYPE_STRING,17,0,0},
	{"topicdelim",TYPE_STRING,18,0,0},
	{"topicin",TYPE_STRING,19,0,0},
	{"topicout",TYPE_STRING,20,0,0},
	{"totalin",TYPE_STRING,21,0,0},
	{"totalout",TYPE_STRING,22,0,0},
	{"totaldelim",TYPE_STRING,23,0,0},
	{"prev",TYPE_STRING,24,0,0},
	{"next",TYPE_STRING,25,0,0},
	{"prevnextdelim",TYPE_STRING,26,0,0},
	{"prevnextin",TYPE_STRING,27,0,0},
	{"prevnextout",TYPE_STRING,28,0,0},
	{"numsplit",TYPE_STRING,29,1,0},
	{"prioin",TYPE_STRING,30,1,0},
	{"prioout",TYPE_STRING,31,1,0},
	{"rowin",TYPE_STRING,32,1,0},
	{"rowout",TYPE_STRING,33,1,0},
	{"oddin",TYPE_STRING,34,1,0},
	{"oddout",TYPE_STRING,35,1,0},
	{"evenin",TYPE_STRING,36,1,0},
	{"evenout",TYPE_STRING,37,1,0},
	{"opts",TYPE_OPTION,0,OPT_MEDIADATE|OPT_TYPELINK|OPT_LINKTYPE|
		OPT_MARK|OPT_TARGET|OPT_TOPIC|OPT_TOTALTOPICS|OPT_TOTALPAGES|
		OPT_LINK|OPT_MORE|OPT_LINKONLY,0},
	{"limit",TYPE_INTEGER,1,10,100},
	{NULL,0,0,0}
};

static const BBLOCK bbwebnews[]=
{
	{"label",TYPE_STRING,0,1,0},
	{"pre",TYPE_STRING,1,1,0},
	{"post",TYPE_STRING,2,0,0},
	{NULL,0,0,0}
};

static const BBLOCK bbsearchengine[]=
{
	{"label",TYPE_STRING,0,1,0},
	{"pre",TYPE_STRING,1,1,0},
	{"post",TYPE_STRING,2,0,0},
	{"home",TYPE_STRING,3,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbredirect[]=
{
	{"label",TYPE_STRING,0,1,0},
	{NULL,0,0,0}
};

static const BBLOCK bbgifimg[]=
{
	{"file",TYPE_STRING,0,1,0},
	{"style",TYPE_STRING,1,1,0},
	{"alt",TYPE_STRING,2,0,0},
	{"width",TYPE_INTEGER,0,1,100000},
	{"height",TYPE_INTEGER,1,1,100000},
	{NULL,0,0,0}
};

static const BBLOCK bbjpgimg[]=
{
	{"file",TYPE_STRING,0,1,0},
	{"style",TYPE_STRING,1,1,0},
	{"alt",TYPE_STRING,2,0,0},
	{"width",TYPE_INTEGER,0,1,100000},
	{"height",TYPE_INTEGER,1,1,100000},
	{NULL,0,0,0}
};

static const BBLOCK bbpngimg[]=
{
	{"file",TYPE_STRING,0,1,0},
	{"style",TYPE_STRING,1,1,0},
	{"alt",TYPE_STRING,2,0,0},
	{"width",TYPE_INTEGER,0,1,100000},
	{"height",TYPE_INTEGER,1,1,100000},
	{NULL,0,0,0}
};

static const TAG tags[]=
{
	{"webnews",bbwebnews,NULL,0,3,0,TAGFLAG_FIXED,0},
	{"searchengine",bbsearchengine,NULL,1,4,0,TAGFLAG_FIXED,0},
	{"gifimg",bbgifimg,htmlgifimg,2,4,3,
		TAGFLAG_NAME|TAGFLAG_NLS|TAGFLAG_IMG,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"jpgimg",bbjpgimg,htmljpgimg,3,4,3,
		TAGFLAG_NAME|TAGFLAG_NLS|TAGFLAG_IMG,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"pngimg",bbjpgimg,htmlpngimg,4,4,3,
		TAGFLAG_NAME|TAGFLAG_NLS|TAGFLAG_IMG,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"var",NULL,htmlvar,5,1,0,TAGFLAG_NAME|TAGFLAG_NLS|TAGFLAG_DATA,
		ALLPAGES},
	{"title",bbtitle,htmltitle,6,4,1,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE|INFOPAGE},
	{"history",bbhistory,htmlhistory,7,5,1,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE|INFOPAGE},
	{"topics",bbtopics,htmltopics,8,4,0,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE|INFOPAGE},
	{"pages",bbpages,htmlpages,9,4,0,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE|INFOPAGE},
	{"desclink",bbdesclink,htmldesclink,10,5,0,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE|INFOPAGE},
	{"description",bbdescription,htmldesc,11,9,0,TAGFLAG_NAME|TAGFLAG_NLS,
		INFOPAGE},
	{"letterbar",bbletterbar,htmlletterbar,12,13,0,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE},
	{"narsym",bbnarsym,htmlnarsym,13,36,4,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE},
	{"related",bbrelated,htmlrelated,14,20,1,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE},
	{"otherlang",bbotherlang,htmlotherlang,15,34,6,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE},
	{"content",bbcontent,htmlcontent,16,29,1,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE},
	{"newsgroup",bbnewsgroup,htmlnewsgroup,17,13,1,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE},
	{"searchinfo",bbsearchinfo,htmlsearchinfo,18,6,1,
		TAGFLAG_NAME|TAGFLAG_NLS,CATALOGPAGE|QUERYPAGE},
	{"externalsearch",bbexternalsearch,htmlexternalsearch,19,12,0,
		TAGFLAG_NAME|TAGFLAG_NLS,CATALOGPAGE|QUERYPAGE},
	{"editor",bbeditor,htmleditor,20,12,1,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE|INFOPAGE},
	{"lastupdate",bblastupdate,htmllastupdate,21,4,1,
		TAGFLAG_NAME|TAGFLAG_NLS,CATALOGPAGE|INFOPAGE},
	{"termsofuse",bbtermsofuse,htmltermsofuse,22,4,0,
		TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE|INFOPAGE|ADVANCEDPAGE|QUERYPAGE},
	{"searchform",bbsearchform,htmlsearchform,23,15,2,
		TAGFLAG_NAME|TAGFLAG_NLS,CATALOGPAGE|INFOPAGE|QUERYPAGE},
	{"advanced",bbadvanced,htmladvanced,24,23,2,TAGFLAG_NAME|TAGFLAG_NLS,
		ADVANCEDPAGE},
	{"engines",bbengines,htmlengines,25,12,0,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"catlink",bbcatlink,htmlcatlink,26,5,0,TAGFLAG_NAME|TAGFLAG_NLS,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"topicquery",bbtopicquery,htmltopicquery,27,24,2,
		TAGFLAG_NAME|TAGFLAG_NLS,QUERYPAGE},
	{"contentquery",bbcontentquery,htmlcontentquery,28,38,2,
		TAGFLAG_NAME|TAGFLAG_NLS,QUERYPAGE},
	{"redirect",bbredirect,htmlredirect,29,1,0,TAGFLAG_NAME|TAGFLAG_NLS,
		ERR302PAGE},
	{"true",NULL,htmltrue,30,1,0,TAGFLAG_NAME|TAGFLAG_NLS|TAGFLAG_DATA,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"false",NULL,htmlfalse,31,1,0,TAGFLAG_NAME|TAGFLAG_NLS|TAGFLAG_DATA,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"deptrue",NULL,htmldeptrue,32,1,0,
		TAGFLAG_NAME|TAGFLAG_NLS|TAGFLAG_DATA,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"depfalse",NULL,htmldepfalse,33,1,0,
		TAGFLAG_NAME|TAGFLAG_NLS|TAGFLAG_DATA,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"clear",NULL,htmlclear,34,0,0,TAGFLAG_ACTION,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"depclear",NULL,htmldepclear,35,0,0,TAGFLAG_ACTION,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"save",NULL,htmlsave,36,0,0,TAGFLAG_ACTION,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"restore",NULL,htmlrestore,37,0,0,TAGFLAG_ACTION,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"csslink",NULL,htmlcsslink,38,0,0,TAGFLAG_ACTION,
		CATALOGPAGE|INFOPAGE|QUERYPAGE|ADVANCEDPAGE},
	{"homepage",NULL,htmlvar,RTYPE_HOME,0,0,TAGFLAG_PAGE,CATALOGPAGE},
	{"indexpage",NULL,htmlvar,RTYPE_INDEX,0,0,TAGFLAG_PAGE,CATALOGPAGE},
	{"catalogpage",NULL,htmlvar,RTYPE_CATALOG,0,0,TAGFLAG_PAGE,CATALOGPAGE},
	{"infopage",NULL,htmlvar,RTYPE_INFO,0,0,TAGFLAG_PAGE,INFOPAGE},
	{"advancedpage",NULL,htmlvar,RTYPE_ADVANCED,0,0,TAGFLAG_PAGE,
		ADVANCEDPAGE},
	{"querypage",NULL,htmlvar,RTYPE_QUERY,0,0,TAGFLAG_PAGE,QUERYPAGE},
	{"csspage",NULL,htmlvar,RTYPE_CSS,0,0,TAGFLAG_PAGE,CSSPAGE},
	{"robotspage",NULL,htmlvar,RTYPE_ROBOTS,0,0,TAGFLAG_PAGE,ROBOTSPAGE},
	{"err302page",NULL,htmlvar,RTYPE_ERR302,0,0,TAGFLAG_PAGE,ERR302PAGE},
	{"err400page",NULL,htmlvar,RTYPE_ERR400,0,0,TAGFLAG_PAGE,ERR400PAGE},
	{"err404page",NULL,htmlvar,RTYPE_ERR404,0,0,TAGFLAG_PAGE,ERR404PAGE},
	{"err500page",NULL,htmlvar,RTYPE_ERR500,0,0,TAGFLAG_PAGE,ERR500PAGE},
	{"err501page",NULL,htmlvar,RTYPE_ERR501,0,0,TAGFLAG_PAGE,ERR501PAGE},
	{NULL,NULL,0,0,0,0}
};

static int gzenable;
static int timelimit;
static int requestlimit;
static int newdb;
static char *database;
static char *database1;
static char *database2;
static char *newdatabase;
static char *user;
static char *password;
static int term;
static int reload;
static int usepool;
static sem_t children;
static sem_t dbpool;
static pthread_mutex_t mtx;
static pthread_mutex_t poolmtx;
static DBPOOL *pool;

static void errmsg(char *msg,int i)
{
	int c;
	char n[32];

	if(i>0)
	{
		write(2,"line ",5);
		if(!i)write(2,"0",1);
		else
		{
			for(c=0;i;c++,i/=10)n[c]='0'+i%10;
			while(c--)write(2,n+c,1);
		}
		write(2,": ",2);

	}
	write(2,msg,strlen(msg));
}

static char *bbdata(char *data,int len,int line)
{
	int i;
	int j;
	char *ret;

	for(i=0,j=line;i<len;i++)if(data[i]=='\n')j++;
	else if(data[i]=='<'&&(data[i+1]=='@'||data[i+1]=='+'||
		data[i+1]=='-'))
	{
		errmsg("tag not allowed here\n",j);
		return NULL;
	}
	else if(!data[i])
	{
		errmsg("illegal character\n",j);
		return NULL;
	}

	j=line;
	if(checkutf8str(data,&j))
	{
		errmsg("illegal UTF8 sequence\n",j);
		return NULL;
	}

	if(!(ret=strdup(data)))errmsg("out of memory\n",0);
	return ret;
}

static int bboption(char *name,int line,unsigned int valid,int *data)
{
	int i;

	for(i=0;options[i].name;i++)if(!strcmp(options[i].name,name))break;
	if(!options[i].name)
	{
		errmsg("unknown option\n",line);
		return -1;
	}
	if(!(valid&options[i].bit))
	{
		errmsg("invalid option\n",line);
		return -1;
	}
	if(*data&options[i].bit)
	{
		errmsg("duplicate option\n",line);
		return -1;
	}
	*data|=options[i].bit;
	return 0;
}

static int bbparam(const BBLOCK *bbparam,char *name,char *data,int len,int line,
	char **strings,int *ints)
{
	int i;
	char *opt;

	for(i=0;bbparam[i].name;i++)if(!strcmp(bbparam[i].name,name))break;
	if(!bbparam[i].name)
	{
		errmsg("unknown parameter\n",line);
		return -1;
	}
	switch(bbparam[i].type)
	{
	case TYPE_STRING:
		if(strings[bbparam[i].index])
		{
			errmsg("duplicate parameter\n",line);
			return -1;
		}
		if(!(strings[bbparam[i].index]=bbdata(data,len,line)))return -1;
		if(!strings[bbparam[i].index][0]&&bbparam[i].v1)
		{
			errmsg("missing value\n",line);
			return -1;
		}
		break;

	case TYPE_OPTION:
		for(opt=NULL;*data;data++)
		{
			if(*data==' '||*data=='\t')continue;
			if(*data=='\n')
			{
				line++;
				continue;
			}
			opt=data;
			while(*data&&*data!=' '&&*data!='\t'&&*data!='\n')
				data++;
			if(*data=='\n')
			{
				*data=0;
				if(bboption(opt,line,bbparam[i].v1,
					&ints[bbparam[i].index]))return -1;
				line++;
			}
			else if(*data)
			{
				*data=0;
				if(bboption(opt,line,bbparam[i].v1,
					&ints[bbparam[i].index]))return -1;
			}
			else
			{
				data--;
				if(bboption(opt,line,bbparam[i].v1,
					&ints[bbparam[i].index]))return -1;
			}
		}
		if(!opt)
		{
			errmsg("missing option\n",line);
			return -1;
		}
		break;

	case TYPE_INTEGER:
		if(ints[bbparam[i].index])
		{
			errmsg("duplicate parameter\n",line);
			return -1;
		}
		for(;*data;data++)if(*data=='\n')line++;
		else if(*data==' '||*data=='\t')continue;
		else break;
		if(!*data)
		{
			errmsg("missing value\n",line);
			return -1;
		}
		if(!(*data>='0'&&*data<='9'))
		{
			errmsg("illegal character\n",line);
			return -1;
		}
		for(;*data>='0'&&*data<='9';data++)
		{
			ints[bbparam[i].index]*=10;
			ints[bbparam[i].index]+=*data-'0';
			if(ints[bbparam[i].index]>bbparam[i].v2)
			{
				errmsg("value out of bounds\n",line);
				return -1;
			}
		}
		if(ints[bbparam[i].index]<bbparam[i].v1)
		{
			errmsg("value out of bounds\n",line);
			return -1;
		}
		for(;*data;data++)if(*data=='\n')line++;
		else if(*data==' '||*data=='\t')continue;
		else break;
		if(*data)
		{
			errmsg("illegal character\n",line);
			return -1;
		}
		break;
	}
	return 0;
}

static int bbparse(const BBLOCK *params,char *data,int len,int line,
	char **strings,int *ints)
{
	int i;
	int state;
	int startidx;
	int endidx;
	int dataidx;
	int datalen;
	int startline;

	startidx=0;
	dataidx=0;
	startline=0;

	for(state=0,i=0;i<len;i++)if(data[i]=='<'&&data[i+1]=='+')
	{
		if(state++)
		{
			errmsg("tag nesting not allowed\n",line);
			return -1;
		}
		i+=2;
		startidx=i;
		startline=line;
		for(;data[i]!='>';i++);
		data[i]=0;
		dataidx=i+1;
	}
	else if(data[i]=='<'&&data[i+1]=='-')
	{
		datalen=i-dataidx;
		data[i]=0;
		state=0;
		i+=2;
		endidx=i;
		for(;data[i]!='>';i++);
		data[i]=0;
		if(strcmp(data+startidx,data+endidx))
		{
			errmsg("opening/closing tag mismatch\n",line);
			return -1;
		}
		if(bbparam(params,data+startidx,data+dataidx,datalen,startline,
			strings,ints))return -1;
	}
	else if(data[i]=='<'&&data[i+1]=='@')
	{
		errmsg("tag not allowed here\n",line);
		return -1;
	}
	else if(data[i]=='\n')line++;
	return 0;
}

static int addelement(char *lang,ELEMENT ***e,int totalstring,int totalint,
	char **strings, int *ints,int clone)
{
	int i;
	ELEMENT *n;

	if(!(n=malloc(sizeof(ELEMENT)+strlen(lang)+1)))
	{
		errmsg("out of memory\n",0);
		return -1;
	}
	n->next=NULL;
	n->flags=EFLAGS_CHAIN|(clone?EFLAGS_CLONE:0);
	strcpy(n->lang,lang);

	if(!totalstring)n->string=NULL;
	else if(!(n->string=malloc(totalstring*sizeof(char *))))
	{
		free(n);
		errmsg("out of memory\n",0);
		return -1;
	}
	n->totalstrings=(unsigned char)(totalstring);

	if(!totalint)n->integer=NULL;
	else if(!(n->integer=malloc(totalint*sizeof(int))))
	{
		if(n->string)free(n->string);
		free(n);
		errmsg("out of memory\n",0);
		return -1;
	}

	for(i=0;i<totalstring;i++)n->string[i]=strings[i];
	for(i=0;i<totalint;i++)n->integer[i]=ints[i];

	**e=n;
	*e=&(n->next);

	return 0;
}

static int addelements(char *nls,ELEMENT ***e,int totalstring,int totalint,
	char **strings,int *ints)
{
	int i;
	char *lang;

	for(i=0,lang=NULL;*nls;nls++)
	{
		if(*nls==' '||*nls=='\t')continue;
		lang=nls;
		for(;*nls&&*nls!=' '&&*nls!='\t';nls++);
		if(*nls)*nls=0;
		else nls--;
		if(addelement(lang,e,totalstring,totalint,strings,ints,i))
			goto fail;
		i++;
	}

	if(!lang)if(addelement("",e,totalstring,totalint,strings,ints,i))
		goto fail;

	return 0;

fail:	if(!i)for(;i<totalstring;i++)if(strings[i])free(strings[i]);
	return -1;
}

static int pageparse(TEMPLATE *t,int index,void (*defaultprocess)(REQUEST *r,
	ELEMENT *e),int valid,char *data,int len,int line)
{
	int i;
	int j;
	int lmem;
	int startidx;
	char *tag;
	char *name;
	ELEMENT *e;
	ACTION *n;
	ACTION **a;
	CHAIN *c;

	if(t->page[index])
	{
		errmsg("duplicate tag\n",line);
		return -1;
	}
	a=&t->page[index];

	for(lmem=line,startidx=0,i=0;i<len;i++)if(data[i]=='<'&&data[i+1]=='+')
	{
		errmsg("tag not allowed here\n",line);
		return -1;
	}
	else if(data[i]=='<'&&data[i+1]=='-')
	{
		errmsg("tag not allowed here\n",line);
		return -1;
	}
	else if(data[i]=='<'&&data[i+1]=='@')
	{
		data[i]=0;
		if(i!=startidx)
		{
			if(!(e=malloc(sizeof(ELEMENT)+1)))
			{
				errmsg("out of memory\n",0);
				return -1;
			}
			e->next=NULL;
			e->integer=NULL;
			e->totalstrings=1;
			e->flags=0;
			e->lang[0]=0;
			if(!(e->string=malloc(sizeof(char *))))
			{
				free(e);
				errmsg("out of memory\n",0);
				return -1;
			}
			if(!(e->string[0]=bbdata(data+startidx,i-startidx,
				lmem)))
			{
				free(e->string);
				free(e);
				return -1;
			}

			if(!(n=malloc(sizeof(ACTION))))
			{
				free(e->string[0]);
				free(e->string);
				free(e);
				errmsg("out of memory\n",0);
				return -1;
			}
			n->next=NULL;
			n->processor=defaultprocess;
			n->data=e;
			*a=n;
			a=&(n->next);
		}

		i+=2;
		for(tag=data+i,name=NULL;data[i]!='>';i++)if(data[i]==':')
		{
			data[i++]=0;
			name=data+i;
			while(data[i]!='>')i++;
			break;
		}
		data[i]=0;

		for(j=0;tags[j].name;j++)if(!strcmp(tags[j].name,tag))
		{
			if((tags[j].flags&TAGFLAG_PAGE)||!(tags[j].valid&valid))
			{
				errmsg("tag not allowed here\n",line);
				return -1;
			}
			if(!name&&(tags[j].flags&TAGFLAG_NAME))
			{
				errmsg("tag name missing\n",line);
				return -1;
			}
			if(tags[j].flags&TAGFLAG_ACTION)
			{
				if(name)
				{
					errmsg("tag name not allowed\n",line);
					return -1;
				}
				c=NULL;
			}
			else
			{
				for(c=t->anchor[tags[j].index];c;c=c->next)
					if(!strcmp(c->name,name))break;
				if(!c)
				{
					errmsg("tag name not defined\n",line);
					return -1;
				}
			}
			if(!(n=malloc(sizeof(ACTION))))
			{
				errmsg("out of memory\n",0);
				return -1;
			}
			n->next=NULL;
			n->processor=tags[j].processor;
			if(!c)n->data=NULL;
			else n->data=c->data;
			*a=n;
			a=&(n->next);
			break;
		}
		if(!tags[j].name)
		{
			errmsg("unknown tag\n",line);
			return -1;
		}

		startidx=i+1;
		lmem=line;
	}
	else if(data[i]=='\n')line++;

	data[i]=0;
	if(i!=startidx)
	{
		if(!(e=malloc(sizeof(ELEMENT)+1)))
		{
			errmsg("out of memory\n",0);
			return -1;
		}
		e->next=NULL;
		e->integer=NULL;
		e->totalstrings=1;
		e->flags=0;
		e->lang[0]=0;
		if(!(e->string=malloc(sizeof(char *))))
		{
			free(e);
			errmsg("out of memory\n",0);
			return -1;
		}
		if(!(e->string[0]=bbdata(data+startidx,i-startidx,lmem)))
		{
			free(e->string);
			return -1;
		}

		if(!(n=malloc(sizeof(ACTION))))
		{
			free(e->string[0]);
			free(e->string);
			free(e);
			errmsg("out of memory\n",0);
			return -1;
		}
		n->next=NULL;
		n->processor=defaultprocess;
		n->data=e;
		*a=n;
		a=&(n->next);
	}

	return 0;
}

static int loadimg(char **strings, int *ints,char *name,int line)
{
	int fd;
	char *img;
	struct stat stb;

	if(!strings[0])
	{
		errmsg("missing image file name\n",line);
		return -1;
	}
	if(stat(strings[0],&stb))
	{
		errmsg("no such image file\n",line);
		return -1;
	}
	if(!(img=malloc(stb.st_size)))
	{
		errmsg("out of memory\n",0);
		return -1;
	}
	if((fd=open(strings[0],O_RDONLY))==-1)
	{
		free(img);
		errmsg("can't access image file\n",line);
		return -1;
	}
	if(read(fd,img,stb.st_size)!=stb.st_size)
	{
		close(fd);
		free(img);
		errmsg("image file read error\n",line);
		return -1;
	}
	close(fd);
	ints[2]=stb.st_size;
	free(strings[0]);
	strings[0]=img;
	if(!(strings[3]=strdup(name)))
	{
		errmsg("out of memory\n",0);
		return -1;
	}
	return 0;
}

static int taghandler(TEMPLATE *t,char *base,int len,int line)
{
	int i;
	int j;
	char *intag;
	char *tagname;
	char *tagnls;
	char *outtag;
	char *data;
	CHAIN *c;
	ELEMENT **e;

	tagname=NULL;
	tagnls=NULL;
	base+=2;
	len-=2;
	for(intag=base;*base!='>';base++,len--)if(*base==':')
	{
		len--;
		*base++=0;
		tagname=base;
		for(;*base!='>';base++,len--)if(*base==':')
		{
			len--;
			*base++=0;
			tagnls=base;
			break;
		}
		break;
	}
	for(;*base!='>';base++,len--);
	len--;
	*base++=0;
	data=base;
	base+=len;
	*base=0;
	base+=2;
	outtag=base;
	for(;*base!='>';base++);
	*base=0;

	if(strcmp(intag,outtag))
	{
		errmsg("opening/closing tag mismatch\n",line);
		return -1;
	}

	if(tagname)if(!*tagname)
	{
		errmsg("empty tag name\n",line);
		return -1;
	}

	if(tagnls)if(!*tagnls)
	{
		errmsg("empty nls data in tag\n",line);
		return -1;
	}

	for(i=0;tags[i].name;i++)if(!strcmp(intag,tags[i].name))
	{
		if(tags[i].flags&TAGFLAG_ACTION)
		{
			errmsg("tag cannot be defined by user\n",line);
			return -1;
		}
		if(tagname&&!(tags[i].flags&TAGFLAG_NAME))
		{
			errmsg("tag name not allowed here\n",line);
			return -1;
		}
		if(!tagname&&(tags[i].flags&TAGFLAG_NAME))
		{
			errmsg("tag name missing\n",line);
			return -1;
		}
		if(tagnls&&!(tags[i].flags&TAGFLAG_NLS))
		{
			errmsg("tag nls data not allowed here\n",line);
			return -1;
		}

		if((tagname||tagnls)&&
			(tags[i].flags&(TAGFLAG_FIXED|TAGFLAG_PAGE)))
		{
			errmsg("tag name and tag nls data not allowed here\n",
				line);
			return -1;
		}

		if(tags[i].flags&TAGFLAG_PAGE)
			return pageparse(t,tags[i].index,tags[i].processor,
				tags[i].valid,data,len,line);

		if(tagname)
		{
			for(c=t->anchor[tags[i].index];c;c=c->next)
				if(!strcmp(c->name,tagname))break;
		}
		else c=t->anchor[tags[i].index];

		if(!c)
		{
			if(!(c=malloc(sizeof(CHAIN)+
				(tagname?strlen(tagname):0)+1)))
			{
				errmsg("out of memory\n",0);
				return -1;
			}
			c->next=t->anchor[tags[i].index];
			t->anchor[tags[i].index]=c;
			c->data=NULL;
			if(tagname)strcpy(c->name,tagname);
			else c->name[0]=0;
		}

		for(e=&c->data;*e;e=&((*e)->next));

		for(j=0;j<tags[i].totalstring;j++)t->strings[j]=NULL;
		for(j=0;j<tags[i].totalint;j++)t->ints[j]=0;

		if(tags[i].flags&TAGFLAG_DATA)
		{
			if(!(t->strings[0]=bbdata(data,len,line)))return -1;
		}
		else if(bbparse(tags[i].params,data,len,line,t->strings,
			t->ints))goto fail;

		if(tags[i].flags&TAGFLAG_IMG)
			if(loadimg(t->strings,t->ints,tagname,line))goto fail;

		if(tags[i].flags&TAGFLAG_FIXED)
		{
			for(j=0;j<tags[i].totalstring;j++)if(!t->strings[j])
			{
				errmsg("missing parameter\n",line);
				goto fail;
			}
			for(j=0;j<tags[i].totalint;j++)if(!t->ints[j])
			{
				errmsg("missing parameter\n",line);
				goto fail;
			}
		}

		if(addelements(tagnls?tagnls:"",&e,tags[i].totalstring,
			tags[i].totalint,t->strings,t->ints))return -1;

		return 0;

fail:		for(j=0;j<tags[i].totalstring;j++)if(t->strings[j])
			free(t->strings[j]);
		return -1;
	}

	errmsg("unknown tag\n",line);
	return -1;
}

static int fileparse(TEMPLATE *t,char *fn)
{
	int i;
	int fd;
	int line;
	int lmem;
	int last;
	int fill;
	int done;
	int state;
	int start;
	int end;
	int err;
	char bfr[BUFSIZE+1];

	err=-1;
	bfr[BUFSIZE]=0;

	if((fd=open(fn,O_RDONLY))==-1)
	{
		errmsg("Can't open template file\n",0);
		goto out0;
	}

	last=0;
	fill=0;
	done=0;
	lmem=1;
	line=1;
	start=0;

	do
	{
		while(!last&&fill<BUFSIZE)
		{
			i=read(fd,bfr+fill,BUFSIZE-fill);
			if(i==-1)
			{
				errmsg("file read error\n",line);
				goto out1;
			}
			else if(!i)last=1;
			fill+=i;
		}

		for(line=lmem,state=0,i=0;i<fill;i++)
		{
			if(bfr[i]=='<'&&bfr[i+1]=='+')
			{
				if(!state)
				{
					done=i;
					start=i;
					lmem=line;
				}
				state++;
				for(i+=2;i<fill;i++)
				{
					if(bfr[i]=='<'||bfr[i]=='\n')
					{
						errmsg("illegal character in "
							"tag\n",line);
						goto out1;
					}
					else if(bfr[i]=='>')break;
				}
			}
			else if(bfr[i]=='<'&&bfr[i+1]=='@')
			{
				if(!state)
				{
					errmsg("tag is illegal here\n",line);
					goto out1;
				}
				for(i+=2;i<fill;i++)
				{
					if(bfr[i]=='<'||bfr[i]=='\n')
					{
						errmsg("illegal character in "
							"tag\n",line);
						goto out1;
					}
					else if(bfr[i]=='>')break;
				}
			}
			else if(bfr[i]=='<'&&bfr[i+1]=='-')
			{
				end=i;
				for(i+=2;i<fill;i++)
				{
					if(bfr[i]=='<'||bfr[i]=='\n')
					{
						errmsg("illegal character in "
							"tag\n",line);
						goto out1;
					}
					else if(bfr[i]=='>')
					{
						if(!state--)
						{
							errmsg("illegal tag "
								"end\n",line);
							goto out1;
						}
						if(!state)
						{
							if(taghandler(t,
								bfr+start,
								end-start,lmem))
								goto out1;
							done=i+1;
							lmem=line;
						}
						break;
					}
				}
			}
			else if(bfr[i]=='\n')line++;
		}

		if(!state)
		{
			done=i;
			lmem=line;
		}

		if(!done&&fill)
		{
			if(fill==BUFSIZE)errmsg("element too large\n",line);
			else errmsg("unexpected end of file\n",line);
			goto out1;
		}
		if(done!=fill)
		{
			if(last)
			{
				errmsg("unexpected end of file\n",line);
				goto out1;
			}
			memmove(bfr,bfr+done,fill-done);
			fill-=done;
			done=0;
		}
		else
		{
			done=0;
			fill=0;
		}
	} while(!last);

	err=0;

out1:	close(fd);
out0:	return err;
}

static int defaultpages(TEMPLATE *t)
{
	ACTION *a;

	if(!t->page[RTYPE_ERR302])
	{
		if(!(a=malloc(sizeof(ACTION))))goto out0;
		a->processor=htmlvar;
		if(!(a->data=malloc(sizeof(ELEMENT)+1)))goto out1;
		a->data->next=NULL;
		a->data->integer=NULL;
		a->data->flags=EFLAGS_CLONE;
		a->data->totalstrings=1;
		a->data->lang[0]=0;
		if(!(a->data->string=malloc(sizeof(char *))))goto out2;
		a->data->string[0]="<html><head><title>302 Found"
			"</title></head><body><h1>Found</h1>"
			"<p>The document has moved ";
		t->page[RTYPE_ERR302]=a;

		if(!(a->next=malloc(sizeof(ACTION))))goto out0;
		a=a->next;
		if(!(a->data=malloc(sizeof(ELEMENT)+1)))goto out0;
		a->processor=htmlredirect;
		a->data->next=NULL;
		a->data->integer=NULL;
		a->data->flags=EFLAGS_CLONE;
		a->data->totalstrings=1;
		a->data->lang[0]=0;
		if(!(a->data->string=malloc(sizeof(char *))))goto out0;
		a->data->string[0]="here";

		if(!(a->next=malloc(sizeof(ACTION))))goto out0;
		a=a->next;
		a->next=NULL;
		if(!(a->data=malloc(sizeof(ELEMENT)+1)))goto out0;
		a->processor=htmlvar;
		a->data->next=NULL;
		a->data->integer=NULL;
		a->data->flags=EFLAGS_CLONE;
		a->data->totalstrings=1;
		a->data->lang[0]=0;
		if(!(a->data->string=malloc(sizeof(char *))))goto out0;
		a->data->string[0]=".</body></html>\n";
	}

	if(!t->page[RTYPE_ERR400])
	{
		if(!(a=malloc(sizeof(ACTION))))goto out0;
		a->processor=htmlvar;
		a->next=NULL;
		if(!(a->data=malloc(sizeof(ELEMENT)+1)))goto out1;
		a->data->next=NULL;
		a->data->integer=NULL;
		a->data->flags=EFLAGS_CLONE;
		a->data->totalstrings=1;
		a->data->lang[0]=0;
		if(!(a->data->string=malloc(sizeof(char *))))goto out2;
		a->data->string[0]="<html><head><title>400 Bad Request"
			"</title></head><body><h1>Bad Request</h1>"
			"<p>Your browser sent a request that this server "
			"couldn't understand.</body></html>\n";
		t->page[RTYPE_ERR400]=a;
	}

	if(!t->page[RTYPE_ERR404])
	{
		if(!(a=malloc(sizeof(ACTION))))goto out0;
		a->processor=htmlvar;
		a->next=NULL;
		if(!(a->data=malloc(sizeof(ELEMENT)+1)))goto out1;
		a->data->next=NULL;
		a->data->integer=NULL;
		a->data->flags=EFLAGS_CLONE;
		a->data->totalstrings=1;
		a->data->lang[0]=0;
		if(!(a->data->string=malloc(sizeof(char *))))goto out2;
		a->data->string[0]="<html><head><title>404 Not Found"
			"</title></head><body><h1>Not Found</h1>"
			"<p>The document you requested couldn't be "
			"found on this server.</body></html>\n";
		t->page[RTYPE_ERR404]=a;
	}

	if(!t->page[RTYPE_ERR500])
	{
		if(!(a=malloc(sizeof(ACTION))))goto out0;
		a->processor=htmlvar;
		a->next=NULL;
		if(!(a->data=malloc(sizeof(ELEMENT)+1)))goto out1;
		a->data->next=NULL;
		a->data->integer=NULL;
		a->data->flags=EFLAGS_CLONE;
		a->data->totalstrings=1;
		a->data->lang[0]=0;
		if(!(a->data->string=malloc(sizeof(char *))))goto out2;
		a->data->string[0]="<html><head><title>500 Internal Server "
			"Error</title></head><body><h1>Internal Server Error"
			"</h1><p>The server encountered an internal error and "
			"was unable to complete your request.</body></html>\n";
		t->page[RTYPE_ERR500]=a;
	}

	if(!t->page[RTYPE_ERR501])
	{
		if(!(a=malloc(sizeof(ACTION))))goto out0;
		a->processor=htmlvar;
		a->next=NULL;
		if(!(a->data=malloc(sizeof(ELEMENT)+1)))goto out1;
		a->data->next=NULL;
		a->data->integer=NULL;
		a->data->flags=EFLAGS_CLONE;
		a->data->totalstrings=1;
		a->data->lang[0]=0;
		if(!(a->data->string=malloc(sizeof(char *))))goto out2;
		a->data->string[0]="<html><head><title>501 Not Implemented"
			"</title></head><body><h1>Not Implemented</h1>"
			"<p>The server can't handle the specified request "
			"method.</body></html>\n";
		t->page[RTYPE_ERR501]=a;
	}

	return 0;

out2:	free(a->data);
out1:	free(a);
out0:	errmsg("out of memory\n",0);
	return -1;
}

static void freetemplate(TEMPLATE *t)
{
	int i;
	int j;
	ACTION *a;
	ACTION *aa;
	ELEMENT *e;
	ELEMENT *ee;
	CHAIN *c;
	CHAIN *cc;

	for(i=0;i<t->pages;i++)for(aa=t->page[i];aa;)
	{
		a=aa;
		aa=aa->next;
		if(a->data)if(!(a->data->flags&EFLAGS_CHAIN))
			for(ee=a->data;ee;)
		{
			e=ee;
			ee=ee->next;
			if(e->integer)free(e->integer);
			if(e->string)
			{
				if(!(e->flags&EFLAGS_CLONE))
					for(j=0;j<e->totalstrings;j++)
						if(e->string[j])
							free(e->string[j]);
				free(e->string);
			}
			free(e);
		}
		free(a);
	}

	for(i=0;i<t->anchors;i++)for(cc=t->anchor[i];cc;)
	{
		c=cc;
		cc=cc->next;
		for(ee=c->data;ee;)
		{
			e=ee;
			ee=ee->next;
			if(e->integer)free(e->integer);
			if(e->string)
			{
				if(!(e->flags&EFLAGS_CLONE))
					for(j=0;j<e->totalstrings;j++)
						if(e->string[j])
							free(e->string[j]);
				free(e->string);
			}
			free(e);
		}
		free(c);
	}

	free(t->page);
	free(t->anchor);
	free(t->strings);
	free(t->ints);
	free(t);
}

static TEMPLATE *loadtemplate(char *fn)
{
	TEMPLATE *t;
	int i;
	int maxstrings;
	int maxints;
	int pages;
	int anchors;

	if(!(t=malloc(sizeof(TEMPLATE))))goto out0;

	for(i=0,pages=0,anchors=0,maxstrings=0,maxints=0;tags[i].name;i++)
	{
		if(tags[i].totalstring>maxstrings)
			maxstrings=tags[i].totalstring;
		if(tags[i].totalint>maxints)maxints=tags[i].totalint;
		if(tags[i].flags&TAGFLAG_PAGE)
		{
			if(tags[i].index>pages)pages=tags[i].index;
		}
		else if(tags[i].index>anchors)anchors=tags[i].index;
	}
	pages++;
	anchors++;

	if(!(t->anchor=malloc(anchors*sizeof(CHAIN *))))goto out1;
	if(!(t->page=malloc(pages*sizeof(ACTION *))))goto out2;
	if(!(t->strings=malloc(maxstrings*sizeof(char *))))goto out3;
	if(!(t->ints=malloc(maxints*sizeof(int))))goto out4;

	memset(t->anchor,0,anchors*sizeof(CHAIN *));
	memset(t->page,0,pages*sizeof(ACTION *));
	t->pages=pages;
	t->anchors=anchors;
	t->usage=1;

	if(fileparse(t,fn))goto out5;
	if(defaultpages(t))goto out5;

	return t;

out5:	freetemplate(t);
	return NULL;

out4:	free(t->strings);
out3:	free(t->page);
out2:	free(t->anchor);
out1:	free(t);
out0:	errmsg("out of memory\n",0);
	return NULL;
}

static void relconn(int index)
{
	int c;

	pthread_mutex_lock(&poolmtx);
	if(pool[index].errflag||pool[index].closeflag)
	{
		c=pool[index].connflag;
		pool[index].connflag=0;
		pthread_mutex_unlock(&poolmtx);
		if(c)
		{
			for(c=0;c<PREPTOTAL;c++)
				sqlfin(pool[index].db,pool[index].sql[c]);
			for(c=0;c<TOTALTMPTBL;c++)
				sqlrun(pool[index].db,tmptables[c].drop);
			sqlclose(pool[index].db,0);
		}
		pthread_mutex_lock(&poolmtx);
		pool[index].errflag=0;
		pool[index].closeflag=0;
		pool[index].busyflag=0;
		pthread_mutex_unlock(&poolmtx);
	}
	else
	{
		pool[index].busyflag=0;
		pthread_mutex_unlock(&poolmtx);
	}
	sem_post(&dbpool);
}

static int getconn(void)
{
	int i;
	int c;
	unsigned long ping;

	sem_wait(&dbpool);
	pthread_mutex_lock(&poolmtx);
	for(i=0;i<usepool;i++)if(!pool[i].busyflag)break;
	pool[i].busyflag=1;
	c=pool[i].connflag;
	pool[i].connflag=1;
	pthread_mutex_unlock(&poolmtx);

	if(c)
	{
		ping=time(NULL);
		if(ping-pool[i].ping>9)if(sqlping(pool[i].db))
		{
			for(c=0;c<PREPTOTAL;c++)
				sqlfin(pool[i].db,pool[i].sql[c]);
			for(c=0;c<TOTALTMPTBL;c++)
				sqlrun(pool[i].db,tmptables[c].drop);
			sqlclose(pool[i].db,0);
			pthread_mutex_lock(&poolmtx);
			pool[i].errflag=0;
			pool[i].closeflag=0;
			pthread_mutex_unlock(&poolmtx);
			c=0;
		}
	}

	if(c)pool[i].ping=ping;
	else
	{
		if(sqlopen(database,user,password,&pool[i].db,SQLFLAGS_STDMEM))
		{
			pthread_mutex_lock(&poolmtx);
			pool[i].connflag=0;
			pool[i].errflag=0;
			pool[i].closeflag=0;
			pool[i].busyflag=0;
			pthread_mutex_unlock(&poolmtx);
			sem_post(&dbpool);
			return -1;
		}
		for(c=0;c<TOTALTMPTBL;c++)if(sqlrun(pool[i].db,
			tmptables[c].create))
		{
			while(c--)sqlrun(pool[i].db,tmptables[c].drop);
			sqlclose(pool[i].db,0);
			pthread_mutex_lock(&poolmtx);
			pool[i].connflag=0;
			pool[i].errflag=0;
			pool[i].closeflag=0;
			pool[i].busyflag=0;
			pthread_mutex_unlock(&poolmtx);
			sem_post(&dbpool);
			return -1;
		}
		for(c=0;c<PREPTOTAL;c++)if(sqlprep(pool[i].db,&pool[i].sql[c],
			prpqry[c].sql,prpqry[c].fmt,NULL))
		{
			while(c--)sqlfin(pool[i].db,pool[i].sql[c]);
			for(c=0;c<TOTALTMPTBL;c++)
				sqlrun(pool[i].db,tmptables[c].drop);
			sqlclose(pool[i].db,0);
			pthread_mutex_lock(&poolmtx);
			pool[i].connflag=0;
			pool[i].errflag=0;
			pool[i].closeflag=0;
			pool[i].busyflag=0;
			pthread_mutex_unlock(&poolmtx);
			sem_post(&dbpool);
			return -1;
		}
		pool[i].ping=time(NULL);
	}

	return i;
}

static void sendflush(REQUEST *r,int doclose)
{
	int complete;

	if(r->fd==-1)return;

	if(r->gzip>0)for(complete=0;!complete||r->widx||r->z.avail_out!=
		sizeof(r->zbuf);)
	{
		if(complete==1&&r->gzip==IOMODE_GZIP)
		{
			complete++;
			if(r->z.avail_out>7)
			{
				r->gzip++;
				r->z.next_out[0]=(char)(r->crc);
				r->z.next_out[1]=(char)(r->crc>>8);
				r->z.next_out[2]=(char)(r->crc>>16);
				r->z.next_out[3]=(char)(r->crc>>24);
				r->z.next_out[4]=(char)(r->len);
				r->z.next_out[5]=(char)(r->len>>8);
				r->z.next_out[6]=(char)(r->len>>16);
				r->z.next_out[7]=(char)(r->len>>24);
				r->z.next_out+=8;
				r->z.avail_out-=8;
			}
		}
		while(!r->z.avail_out||(complete&&r->z.avail_out!=
				sizeof(r->zbuf)))
		{
			if(times(NULL)-r->starttime>=timelimit)
			{
				shutdown(r->fd,SHUT_RDWR);
				close(r->fd);
				r->fd=-1;
				deflateEnd(&r->z);
				return;
			}
			poll(&r->w,1,1000);
			if(r->w.revents&(POLLERR|POLLHUP|POLLNVAL))
			{
				shutdown(r->fd,SHUT_RDWR);
				close(r->fd);
				r->fd=-1;
				deflateEnd(&r->z);
				return;
			}
			if(r->w.revents&POLLOUT)
			{
				r->wlen=write(r->fd,r->zbuf,sizeof(r->zbuf)-
					r->z.avail_out);
				if(r->wlen<=0)
				{
					shutdown(r->fd,SHUT_RDWR);
					close(r->fd);
					r->fd=-1;
					deflateEnd(&r->z);
					return;
				}
				if(r->z.avail_out+r->wlen!=sizeof(r->zbuf))
				{
					memmove(r->zbuf,r->zbuf+r->wlen,
						sizeof(r->zbuf)-
						r->z.avail_out-r->wlen);
				}
				r->z.avail_out+=r->wlen;
				r->z.next_out=r->zbuf+sizeof(r->zbuf)-
					r->z.avail_out;
			}
		}

		r->z.next_in=r->buf;
		r->z.avail_in=r->widx;
		if(r->widx&&r->gzip==IOMODE_GZIP)
			r->crc=crc32(r->crc,r->buf,r->widx);

		switch(deflate(&r->z,Z_FINISH))
		{
		case Z_STREAM_END:
			complete=1;
		case Z_OK:
			if(r->widx)if(r->widx==r->z.avail_in)continue;
			if(!r->z.avail_in)r->widx=0;
			else
			{
				memmove(r->buf,r->z.next_in,
					r->widx-r->z.avail_in);
				r->widx-=r->z.avail_in;
			}
			r->z.next_in=r->buf;
			break;
		default:
			shutdown(r->fd,SHUT_RDWR);
			close(r->fd);
			r->fd=-1;
			deflateEnd(&r->z);
			return;
		}
	}

	if(r->gzip==IOMODE_GZIP)
	{
		r->buf[0]=(char)(r->crc);
		r->buf[1]=(char)(r->crc>>8);
		r->buf[2]=(char)(r->crc>>16);
		r->buf[3]=(char)(r->crc>>24);
		r->buf[4]=(char)(r->len);
		r->buf[5]=(char)(r->len>>8);
		r->buf[6]=(char)(r->len>>16);
		r->buf[7]=(char)(r->len>>24);
		r->widx=8;
	}

	while(r->widx)
	{
		if(times(NULL)-r->starttime>=timelimit)
		{
			shutdown(r->fd,SHUT_RDWR);
			close(r->fd);
			r->fd=-1;
			if(r->gzip)deflateEnd(&r->z);
			return;
		}
		poll(&r->w,1,1000);
		if(r->w.revents&(POLLERR|POLLHUP|POLLNVAL))
		{
			shutdown(r->fd,SHUT_RDWR);
			close(r->fd);
			r->fd=-1;
			if(r->gzip)deflateEnd(&r->z);
			return;
		}
		if(r->w.revents&POLLOUT)
		{
			r->wlen=write(r->fd,r->buf,r->widx);
			if(r->wlen<=0)
			{
				shutdown(r->fd,SHUT_RDWR);
				close(r->fd);
				r->fd=-1;
				if(r->gzip)deflateEnd(&r->z);
				return;
			}
			if(r->wlen==r->widx)r->widx=0;
			else
			{
				memmove(r->buf,r->buf+r->wlen,r->widx-r->wlen);
				r->widx-=r->wlen;
			}
		}
	}

	if(doclose)
	{
		shutdown(r->fd,SHUT_RDWR);
		close(r->fd);
		r->fd=-1;
		if(r->gzip)deflateEnd(&r->z);
	}
	else if(r->gzip<0)
	{
		r->gzip=-r->gzip;
		if(r->gzip==IOMODE_GZIP)r->len=0;
	}
}

static void senddata(REQUEST *r,char *s,int l)
{
	if(r->fd==-1)return;

	while(l)
	{
		if(r->widx==sizeof(r->buf))
		{
			if(r->gzip>0)
			{
				while(!r->z.avail_out)
				{
					if(times(NULL)-r->starttime>=timelimit)
					{
						shutdown(r->fd,SHUT_RDWR);
						close(r->fd);
						r->fd=-1;
						deflateEnd(&r->z);
						return;
					}
					poll(&r->w,1,1000);
					if(r->w.revents&
						(POLLERR|POLLHUP|POLLNVAL))
					{
						shutdown(r->fd,SHUT_RDWR);
						close(r->fd);
						r->fd=-1;
						deflateEnd(&r->z);
						return;
					}
					if(r->w.revents&POLLOUT)
					{
						r->wlen=write(r->fd,r->zbuf,
							sizeof(r->zbuf)-
							r->z.avail_out);
						if(r->wlen<=0)
						{
							shutdown(r->fd,
								SHUT_RDWR);
							close(r->fd);
							r->fd=-1;
							deflateEnd(&r->z);
							return;
						}
						if(r->z.avail_out+r->wlen!=
							sizeof(r->zbuf))
						{
							memmove(r->zbuf,
								r->zbuf+r->wlen,
								sizeof(r->zbuf)-
								r->z.avail_out-
								r->wlen);
						}
						r->z.avail_out+=r->wlen;
						r->z.next_out=r->zbuf+
							sizeof(r->zbuf)-
							r->z.avail_out;
					}
				}

				r->z.next_in=r->buf;
				r->z.avail_in=r->widx;

				if(deflate(&r->z,Z_NO_FLUSH)==Z_OK)
				{
					if(r->z.avail_in==r->widx)continue;
					if(r->gzip==IOMODE_GZIP)
						r->crc=crc32(r->crc,r->buf,
							r->widx-r->z.avail_in);
					if(!r->z.avail_in)r->widx=0;
					else
					{
						memmove(r->buf,r->z.next_in,
							r->widx-r->z.avail_in);
						r->widx-=r->z.avail_in;
					}
				}
				else
				{
					shutdown(r->fd,SHUT_RDWR);
					close(r->fd);
					r->fd=-1;
					deflateEnd(&r->z);
					return;
				}
			}
			else
			{
				if(times(NULL)-r->starttime>=timelimit)
				{
					shutdown(r->fd,SHUT_RDWR);
					close(r->fd);
					r->fd=-1;
					if(r->gzip)deflateEnd(&r->z);
					return;
				}
				poll(&r->w,1,1000);
				if(r->w.revents&(POLLERR|POLLHUP|POLLNVAL))
				{
					shutdown(r->fd,SHUT_RDWR);
					close(r->fd);
					r->fd=-1;
					if(r->gzip)deflateEnd(&r->z);
					return;
				}
				if(r->w.revents&POLLOUT)
				{
					r->wlen=write(r->fd,r->buf,r->widx);
					if(r->wlen<=0)
					{
						shutdown(r->fd,SHUT_RDWR);
						close(r->fd);
						r->fd=-1;
						if(r->gzip)deflateEnd(&r->z);
						return;
					}
					if(r->wlen==r->widx)r->widx=0;
					else
					{
						memmove(r->buf,r->buf+r->wlen,
							r->widx-r->wlen);
						r->widx-=r->wlen;
					}
				}
				else continue;
			}
		}
		if(l>sizeof(r->buf)-r->widx)
		{
			memcpy(r->buf+r->widx,s,sizeof(r->buf)-r->widx);
			if(r->gzip==IOMODE_GZIP)r->len+=sizeof(r->buf)-r->widx;
			s+=sizeof(r->buf)-r->widx;
			l-=sizeof(r->buf)-r->widx;
			r->widx=sizeof(r->buf);
		}
		else
		{
			memcpy(r->buf+r->widx,s,l);
			if(r->gzip==IOMODE_GZIP)r->len+=l;
			r->widx+=l;
			s+=l;
			l=0;
		}
	}
}

static void sendstr(REQUEST *r,char *s)
{
	senddata(r,s,strlen(s));
}

static void sendchr(REQUEST *r,char c)
{
	senddata(r,&c,1);
}

static void sendint(REQUEST *r,int i,char *delim)
{
	int c;
	int m;
	char n[32];

	if(!i)
	{
		sendstr(r,"0");
		return;
	}
	if(i<0)
	{
		m=1;
		i=-i;
	}
	else m=0;
	for(c=0;i;c++,i/=10)n[c]='0'+i%10;
	if(m)n[c++]='-';
	if(delim)while(c--)
	{
		senddata(r,&n[c],1);
		if(c&&!(c%3))sendstr(r,delim);
	}
	else while(c--)senddata(r,&n[c],1);
}

static void sendelement(REQUEST *r,unsigned char *qry,int mode)
{
	for(;*qry;qry++)
	{
		if(*qry>='0'&&*qry<='9')senddata(r,qry,1);
		else if(*qry>='a'&&*qry<='z')senddata(r,qry,1);
		else if(*qry>='A'&&*qry<='Z')senddata(r,qry,1);
		else switch(*qry)
		{
		case ';':
		case '/':
		case ':':
		case '@':
		case '$':
		case ',':
		case '-':
		case '_':
		case '.':
		case '!':
		case '~':
		case '*':
		case '\'':
		case '(':
		case ')':
		case '%':
			senddata(r,qry,1);
			break;
		case ' ':
			if(mode)sendchr(r,'+');
			else sendchr(r,' ');
			break;
		default:
			sendchr(r,'%');
			if(((*qry)>>4)>9)sendchr(r,((*qry)>>4)+'a'-10);
			else sendchr(r,((*qry)>>4)+'0');
			if(((*qry)&0xf)>9)sendchr(r,((*qry)&0xf)+'a'-10);
			else sendchr(r,((*qry)&0xf)+'0');
		}
	}
}

static void sendhref(REQUEST *r,unsigned char *url)
{
	for(url+=3;*url;url++)
	{
		if(*url>='0'&&*url<='9')senddata(r,url,1);
		else if(*url>='a'&&*url<='z')senddata(r,url,1);
		else if(*url>='A'&&*url<='Z')senddata(r,url,1);
		else switch(*url)
		{
		case ';':
		case '/':
		case '?':
		case ':':
		case '@':
		case '&':
		case '=':
		case '+':
		case '$':
		case ',':
		case '-':
		case '_':
		case '.':
		case '!':
		case '~':
		case '*':
		case '\'':
		case '(':
		case ')':
		case '%':
		case '#':
			senddata(r,url,1);
			break;
		default:
			sendchr(r,'%');
			if(((*url)>>4)>9)sendchr(r,((*url)>>4)+'a'-10);
			else sendchr(r,((*url)>>4)+'0');
			if(((*url)&0xf)>9)sendchr(r,((*url)&0xf)+'a'-10);
			else sendchr(r,((*url)&0xf)+'0');
		}
	}
	if(url[-1]!='/')sendchr(r,'/');
}

static void sendtxt(REQUEST *r,unsigned char *txt,int mode)
{
	for(;*txt;txt++)switch(*txt)
	{
	case '<':
		sendstr(r,"&lt;");
		break;
	case '>':
		sendstr(r,"&gt;");
		break;
	case '&':
		sendstr(r,"&amp;");
		break;
	case '\"':
		sendstr(r,"&quot;");
		break;
	case '_':
		if(mode)
		{
			sendchr(r,' ');
			break;
		}
	default:
		senddata(r,txt,1);
		break;
	}
}

static void markit(REQUEST *r,char *str,int total,char **words,char *in,
	char *out,int mode)
{
	int l;
	int v;
	int len;
	int count;
	char *p;
	char *q;
	char *s;
	char *wrk;
	char *token1;
	char *token2;
	char *word;
	char bfr[64];

	if(!(wrk=strdup(str)))
	{
		sendtxt(r,str,0);
		return;
	}

	for(count=0,p=wrk;(p=strtok_r(p,mode?" _\t\r\n":" \t\r\n",&token1));
		p=NULL)
	{
		if(count++)sendchr(r,' ');
		len=strlen(p);
		if(len<sizeof(bfr))
		{
			strcpy(bfr,p);
			word=bfr;
		}
		else if(!(word=strdup(p)))
		{
			sendtxt(r,p,0);
			continue;
		}

		for(q=s=word;*q;)
		{
			if(!(l=utf8_to_ucs4(q,len,&v)))
			{
				*word=0;
				break;
			}
			q+=l;
			len-=l;
			v=ucs4_normalize(v);
			if(v<0x80)if(!(v>='0'&&v<='9')&&!(v>='a'&&v<='z'))
				v=0x20;
			s+=ucs4_to_utf8(&v,s);
		}
		*s=0;

		for(q=word;(q=strtok_r(q," ",&token2));q=NULL)
			for(l=0;l<total;l++)if(!strcmp(q,words[l]))goto mark;
mark:		if(q&&in)sendstr(r,in);
		sendtxt(r,p,0);
		if(q&&out)sendstr(r,out);

		if(word!=bfr)free(word);
	}
	free(wrk);
}

static int buildquery(char *search,char *query,int qlen,const char *head,
	const char *tail,int limit,int offset)
{
	int l;
	int v;
	int f;
	int q;
	int r;
	int s;
	int c;
	char n[32];

	if(head)
	{
		l=strlen(head);
		if(qlen<l+1)return -1;
		strcpy(query,head);
		query+=l;
		qlen-=l;
	}

	if(search)for(f=0,q=0,s=strlen(search);*search;search+=l,s-=l)
	{
		if(!(l=utf8_to_ucs4(search,s,&v)))return -1;
		v=ucs4_normalize(v);
		if(v<0x80)if(!(v>='0'&&v<='9')&&!(v>='a'&&v<='z'))v=0x20;
		if(v==' ')continue;
		if(f)
		{
			if(qlen<2)return -1;
			qlen-=2;
			*query++=',';
			*query++='\'';
		}
		else
		{
			if(!qlen)return -1;
			qlen--;
			*query++='\'';
			f=1;
		}
		while(1)
		{
			if(v==' '&&!q)break;
			if(qlen<6)return -1;
			if(v=='\"')q=1-q;
			if(v=='\'')
			{
				qlen--;
				*query++='\'';
			}
			r=ucs4_to_utf8(&v,query);
			query+=r;
			qlen-=r;
			search+=l;
			s-=l;
			if(!*search)
			{
				l=0;
				break;
			}
			if(!(l=utf8_to_ucs4(search,s,&v)))return -1;
			v=ucs4_normalize(v);
			if(v<0x80)if(!(v>='0'&&v<='9')&&!(v>='a'&&v<='z'))
				v=0x20;
		}
		if(!qlen)return -1;
		qlen--;
		*query++='\'';
	}
	else f=1;

	if(tail)
	{
		l=strlen(tail);
		if(qlen<l+1)return -1;
		strcpy(query,tail);
		query+=l;
		qlen-=l;
	}

	if(limit>0)
	{
		for(c=0;limit;c++,limit/=10)n[c]='0'+limit%10;
		if(qlen<c+8)return -1;
		strcpy(query," LIMIT ");
		query+=7;
		qlen-=c+7;
		while(c--)*query++=n[c];
		*query=0;
	}
	if(offset>0)
	{
		for(c=0;offset;c++,offset/=10)n[c]='0'+offset%10;
		if(qlen<c+9)return -1;
		strcpy(query," OFFSET ");
		query+=8;
		qlen-=c+8;
		while(c--)*query++=n[c];
		*query=0;
	}
	return f;
}

static void htmlclear(REQUEST *r,ELEMENT *e)
{
	r->outputdone&=~OFLAG_OUT;
}

static void htmltrue(REQUEST *r,ELEMENT *e)
{
	if(r->outputdone&OFLAG_OUT)sendstr(r,e->string[0]);
}

static void htmlfalse(REQUEST *r,ELEMENT *e)
{
	if(!(r->outputdone&OFLAG_OUT))sendstr(r,e->string[0]);
}

static void htmldepclear(REQUEST *r,ELEMENT *e)
{
	r->outputdone&=~OFLAG_DEP;
}

static void htmldeptrue(REQUEST *r,ELEMENT *e)
{
	if(r->outputdone&OFLAG_OUT)if(r->outputdone&OFLAG_DEP)
		sendstr(r,e->string[0]);
}

static void htmldepfalse(REQUEST *r,ELEMENT *e)
{
	if(r->outputdone&OFLAG_OUT)if(!(r->outputdone&OFLAG_DEP))
		sendstr(r,e->string[0]);
}

static void htmlsave(REQUEST *r,ELEMENT *e)
{
	r->outputdmem=r->outputdone;
}

static void htmlrestore(REQUEST *r,ELEMENT *e)
{
	r->outputdone=r->outputdmem;
}

static void htmlgifimg(REQUEST *r,ELEMENT *e)
{
	sendstr(r,"<img ");
	if(e->string[1])
	{
		sendstr(r," class=\"");
		sendstr(r,e->string[1]);
		sendstr(r,"\" src=\"/img/");
	}
	else sendstr(r," src=\"/img/");
	sendelement(r,e->string[3],0);
	sendstr(r,".gif\"");
	if(e->string[2])
	{
		sendstr(r," alt=\"");
		sendstr(r,e->string[2]);
		sendchr(r,'\"');
	}
	else sendstr(r," alt=\"\"");
	if(e->integer[0])
	{
		sendstr(r," width=\"");
		sendint(r,e->integer[0],NULL);
		sendchr(r,'\"');
	}
	if(e->integer[1])
	{
		sendstr(r," height=\"");
		sendint(r,e->integer[1],NULL);
		sendchr(r,'\"');
	}
	sendstr(r,"/>");
}

static void htmljpgimg(REQUEST *r,ELEMENT *e)
{
	sendstr(r,"<img ");
	if(e->string[1])
	{
		sendstr(r," class=\"");
		sendstr(r,e->string[1]);
		sendstr(r,"\" src=\"/img/");
	}
	else sendstr(r," src=\"/img/");
	sendelement(r,e->string[3],0);
	sendstr(r,".jpg\"");
	if(e->string[2])
	{
		sendstr(r," alt=\"");
		sendstr(r,e->string[2]);
		sendchr(r,'\"');
	}
	else sendstr(r," alt=\"\"");
	if(e->integer[0])
	{
		sendstr(r," width=\"");
		sendint(r,e->integer[0],NULL);
		sendchr(r,'\"');
	}
	if(e->integer[1])
	{
		sendstr(r," height=\"");
		sendint(r,e->integer[1],NULL);
		sendchr(r,'\"');
	}
	sendstr(r,"/>");
}

static void htmlpngimg(REQUEST *r,ELEMENT *e)
{
	sendstr(r,"<img ");
	if(e->string[1])
	{
		sendstr(r," class=\"");
		sendstr(r,e->string[1]);
		sendstr(r,"\" src=\"/img/");
	}
	else sendstr(r," src=\"/img/");
	sendelement(r,e->string[3],0);
	sendstr(r,".png\"");
	if(e->string[2])
	{
		sendstr(r," alt=\"");
		sendstr(r,e->string[2]);
		sendchr(r,'\"');
	}
	else sendstr(r," alt=\"\"");
	if(e->integer[0])
	{
		sendstr(r," width=\"");
		sendint(r,e->integer[0],NULL);
		sendchr(r,'\"');
	}
	if(e->integer[1])
	{
		sendstr(r," height=\"");
		sendint(r,e->integer[1],NULL);
		sendchr(r,'\"');
	}
	sendstr(r,"/>");
}

static void htmlcsslink(REQUEST *r,ELEMENT *e)
{
	if(r->tmpl->page[RTYPE_CSS])sendstr(r,"<link rel=\"stylesheet\" "
		"type=\"text/css\" href=\"/minimoz.css\"/>");
}

static void htmlvar(REQUEST *r,ELEMENT *e)
{
	sendstr(r,e->string[0]);
}

static void htmlredirect(REQUEST *r,ELEMENT *e)
{
	sendstr(r,"<a href=\"");
	sendhref(r,r->data);
	sendstr(r,"\">");
	if(e->string[0])sendstr(r,e->string[0]);
	else sendstr(r,"here");
	sendstr(r,"</a>");
}

static void htmltitle(REQUEST *r,ELEMENT *e)
{
	int m;
	int v;
	int h=-1;
	int rv;
	char *res;
	int hist[MAXHIST];

	m=1;
	hist[0]=r->catid;

	if(!(e->integer[0]&OPT_TITLEONLY))
	{
		if(usepool)
		{
			if((h=getconn())==-1)rv=-1;
			else rv=0;
		}
		else rv=sqlprep(r->db,&r->sql,prpqry[QRY_PARENTID].sql,
			prpqry[QRY_PARENTID].fmt,NULL);
		if(!rv)
		{
			for(v=hist[0],m=1;m<MAXHIST;m++)
			{
				if(usepool)
				{
					if(sqlparams(pool[h].db,
						pool[h].sql[QRY_PARENTID],
						prpqry[QRY_PARENTID].fmt,v))
					{
						pool[h].errflag=1;
						break;
					}
				}
				else if(sqlparams(r->db,r->sql,
					prpqry[QRY_PARENTID].fmt,v))break;
				if(usepool)
				{
					if((rv=sqlrow(pool[h].db,
						pool[h].sql[QRY_PARENTID],
						prpqry[QRY_PARENTID].res,&v)))
					{
						if(rv==-1)pool[h].errflag=1;
						break;
					}
				}
				else if(sqlrow(r->db,r->sql,
					prpqry[QRY_PARENTID].res,&v))break;
				if(!(hist[m]=v))break;
			}
			if(!usepool)sqlfin(r->db,r->sql);
		}
	}

	if(e->integer[0]&OPT_STRIPTOP)if(hist[m-1]==TOPCATID)if(!--m)
	{
		if(h!=-1)relconn(h);
		return;
	}

	v=0;
	if(usepool)
	{
		if(h==-1)
		{
			if((h=getconn())==-1)rv=-1;
			else rv=0;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_TITLE].sql,
		prpqry[QRY_TITLE].fmt,NULL);
	if(!rv)
	{
		for(;m--;v++)
		{
			if(usepool)
			{
				if(sqlparams(pool[h].db,
					pool[h].sql[QRY_TITLE],
					prpqry[QRY_TITLE].fmt,hist[m]))
				{
					pool[h].errflag=1;
					break;
				}
			}
			else if(sqlparams(r->db,r->sql,prpqry[QRY_TITLE].fmt,
				hist[m]))break;
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_TITLE],
					prpqry[QRY_TITLE].res,&res)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_TITLE].res,&res))
				break;
			if(v)sendstr(r,e->string[2]?e->string[2]:": ");
			else if(e->string[0])sendstr(r,e->string[0]);
			sendtxt(r,res,1);
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(h!=-1)relconn(h);

	if(v)r->outputdone=OFLAG_OUT|OFLAG_DEP;

	if(v&&e->string[1])sendstr(r,e->string[1]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmlhistory(REQUEST *r,ELEMENT *e)
{
	int m;
	int v;
	int h=-1;
	int rv;
	char *res1;
	char *res2;
	int hist[MAXHIST];

	m=1;
	hist[0]=r->catid;

	if(usepool)
	{
		if((h=getconn())==-1)
		{
			if(e->string[3])sendstr(r,e->string[3]);
			return;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_PARENTID].sql,
		prpqry[QRY_PARENTID].fmt,NULL);
	if(!rv)
	{
		for(v=hist[0],m=1;m<MAXHIST;m++)
		{
			if(usepool)
			{
				if(sqlparams(pool[h].db,
					pool[h].sql[QRY_PARENTID],
					prpqry[QRY_PARENTID].fmt,v))
				{
					pool[h].errflag=1;
					relconn(h);
					if(e->string[3])sendstr(r,e->string[3]);
					return;
				}
			}
			else if(sqlparams(r->db,r->sql,prpqry[QRY_PARENTID].fmt,
				v))break;
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,
					pool[h].sql[QRY_PARENTID],
					prpqry[QRY_PARENTID].res,&v)))
				{
					if(rv==1)break;
					pool[h].errflag=1;
					relconn(h);
					if(e->string[3])sendstr(r,e->string[3]);
					return;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_PARENTID].res,
				&v))break;
			if(!(hist[m]=v))break;
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	v=0;
	if(usepool)rv=0;
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_IDTITLE].sql,
		prpqry[QRY_IDTITLE].fmt,NULL);
	if(!rv)
	{
		for(;m--;v++)
		{
			if(v)sendstr(r,e->string[2]?e->string[2]:": ");
			else
			{
				r->outputdone=OFLAG_OUT|OFLAG_DEP;
				if(e->string[0])sendstr(r,e->string[0]);
			}
			if(usepool)
			{
				if(sqlparams(pool[h].db,
					pool[h].sql[QRY_IDTITLE],
					prpqry[QRY_IDTITLE].fmt,hist[m]))
				{
					pool[h].errflag=1;
					break;
				}
			}
			else if(sqlparams(r->db,r->sql,prpqry[QRY_IDTITLE].fmt,
				hist[m]))break;
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,
					pool[h].sql[QRY_IDTITLE],
					prpqry[QRY_IDTITLE].res,&res1,&res2)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_IDTITLE].res,
				&res1,&res2))break;
			if(m||(e->integer[0]&OPT_LINKSELF))
			{
				if(e->string[4])
				{
					sendstr(r,"<a class=\"");
					sendstr(r,e->string[4]);
					sendstr(r,"\" href=\"");
				}
				else sendstr(r,"<a href=\"");
				sendhref(r,res1);
				sendstr(r,"\">");
				sendtxt(r,res2,1);
				sendstr(r,"</a>");
			}
			else sendtxt(r,res2,1);
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)relconn(h);

	if(v&&e->string[1])sendstr(r,e->string[1]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmltopics(REQUEST *r,ELEMENT *e)
{
	int v;
	int res;
	int h=-1;
	int rv;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_TOPICCTR],
			prpqry[QRY_TOPICCTR].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_TOPICCTR].sql,
		prpqry[QRY_TOPICCTR].fmt,prpqry[QRY_TOPICCTR].fmt,r->catid);
	if(!rv)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_TOPICCTR],
				prpqry[QRY_TOPICCTR].res,&res))==-1)
				pool[h].errflag=1;
		}
		else rv=sqlrow(r->db,r->sql,prpqry[QRY_TOPICCTR].res,&res);
		if(!rv)
		{
			v=1;
			if(e->string[0])sendstr(r,e->string[0]);
			sendint(r,res,e->string[3]);
			if(e->string[1])sendstr(r,e->string[1]);
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v)r->outputdone=OFLAG_OUT|OFLAG_DEP;
	else if(!v&&e->string[2])sendstr(r,e->string[2]);
}

static void htmlpages(REQUEST *r,ELEMENT *e)
{
	int v;
	int res;
	int h=-1;
	int rv;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_PAGECTR],
			prpqry[QRY_PAGECTR].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_PAGECTR].sql,
		prpqry[QRY_PAGECTR].fmt,prpqry[QRY_PAGECTR].fmt,r->catid);
	if(!rv)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_PAGECTR],
				prpqry[QRY_PAGECTR].res,&res))==-1)
				pool[h].errflag=1;
		}
		else rv=sqlrow(r->db,r->sql,prpqry[QRY_PAGECTR].res,&res);
		if(!rv)
		{
			v=1;
			if(e->string[0])sendstr(r,e->string[0]);
			sendint(r,res,e->string[3]);
			if(e->string[1])sendstr(r,e->string[1]);
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v)r->outputdone=OFLAG_OUT|OFLAG_DEP;
	else if(!v&&e->string[2])sendstr(r,e->string[2]);
}

static void htmldesclink(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	int res1;
	char *res2;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_DESCINFO],
			prpqry[QRY_DESCINFO].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_DESCINFO].sql,
		prpqry[QRY_DESCINFO].fmt,prpqry[QRY_DESCINFO].fmt,r->catid);
	if(!rv)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_DESCINFO],
				prpqry[QRY_DESCINFO].res,&res1,&res2))==-1)
				pool[h].errflag=1;
		}
		else rv=sqlrow(r->db,r->sql,prpqry[QRY_DESCINFO].res,&res1,
			&res2);
		if(!rv)
		{
			v=1;
			if(e->string[1])sendstr(r,e->string[1]);
			if(e->string[4])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[4]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendhref(r,res2);
			sendstr(r,"desc.html\">");
			if(e->string[0])sendstr(r,e->string[0]);
			else sendtxt(r,r->nls[3],0);
			sendstr(r,"</a>");
			if(e->string[2])sendstr(r,e->string[2]);
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v)r->outputdone=OFLAG_OUT|OFLAG_DEP;
	else if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmldesc(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	char *res1;
	char *res2;
	char *res3;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_DESCRIPTION],
			prpqry[QRY_DESCRIPTION].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_DESCRIPTION].sql,
		prpqry[QRY_DESCRIPTION].fmt,prpqry[QRY_DESCRIPTION].fmt,
		r->catid);
	if(!rv)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_DESCRIPTION],
				prpqry[QRY_DESCRIPTION].res,&res1,&res2,&res3))
				==-1)pool[h].errflag=1;
		}
		else rv=sqlrow(r->db,r->sql,prpqry[QRY_DESCRIPTION].res,&res1,
			&res2,&res3);
		if(!rv)
		{
			v=1;
			r->outputdone=OFLAG_OUT|OFLAG_DEP;
			if(e->string[0])sendstr(r,e->string[0]);
			sendstr(r,e->string[5]?e->string[5]:"<h1><b>");
			if(e->string[4])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[4]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendhref(r,res2);
			sendstr(r,"\">");
			sendtxt(r,res1,1);
			sendstr(r,"</a>");
			sendstr(r,e->string[6]?e->string[6]:"</b></h1><p>\n");
			sendstr(r,res3);
			sendchr(r,'\n');
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(v)
	{
		if(usepool)
		{
			if(sqlparams(pool[h].db,pool[h].sql[QRY_DESCNARROW],
				prpqry[QRY_DESCNARROW].fmt,r->catid))
			{
				pool[h].errflag=1;
				rv=-1;
			}
		}
		else rv=sqlprep(r->db,&r->sql,prpqry[QRY_DESCNARROW].sql,
			prpqry[QRY_DESCNARROW].fmt,prpqry[QRY_DESCNARROW].fmt,
			r->catid);
		if(!rv)
		{
			while(1)
			{
				if(usepool)
				{
					if((rv=sqlrow(pool[h].db,
						pool[h].sql[QRY_DESCNARROW],
						prpqry[QRY_DESCNARROW].res,
						&res1,&res2,&res3)))
					{
						if(rv==-1)pool[h].errflag=1;
						break;
					}
				}
				else if(sqlrow(r->db,r->sql,
					prpqry[QRY_DESCNARROW].res,&res1,&res2,
					&res3))break;
				if(v++==1)sendstr(r,e->string[2]?e->string[2]:
					"<hr>\n");
				sendstr(r,e->string[7]?e->string[7]:"<h2><b>");
				if(e->string[4])
				{
					sendstr(r,"<a class=\"");
					sendstr(r,e->string[4]);
					sendstr(r,"\" href=\"");
				}
				else sendstr(r,"<a href=\"");
				sendhref(r,res2);
				sendstr(r,"\">");
				sendtxt(r,res1,1);
				sendstr(r,"</a>");
				sendstr(r,e->string[8]?e->string[8]:
					"</b></h2><p>\n");
				sendstr(r,res3);
				sendchr(r,'\n');
			}
			if(!usepool)sqlfin(r->db,r->sql);
		}
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v&&e->string[1])sendstr(r,e->string[1]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmlletterbar(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	char *res1;
	char *res2;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_LETTERBAR],
			prpqry[QRY_LETTERBAR].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_LETTERBAR].sql,
		prpqry[QRY_LETTERBAR].fmt,prpqry[QRY_LETTERBAR].fmt,r->catid);
	if(!rv)
	{
		while(1)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,
					pool[h].sql[QRY_LETTERBAR],
					prpqry[QRY_LETTERBAR].res,&res1,&res2)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_LETTERBAR].res,
				&res1,&res2))break;
			if(!v)
			{
				r->outputdone=OFLAG_OUT|OFLAG_DEP;
				if(e->string[0])sendstr(r,e->string[0]);
			}

			if(e->string[7])sendstr(r,e->string[7]);
			if(v&1)
			{
				if(e->string[11])sendstr(r,e->string[11]);
			}
			else if(e->string[9])sendstr(r,e->string[9]);

			if(v)sendstr(r,e->string[2]?e->string[2]:" | ");

			if(e->string[5])sendstr(r,e->string[5]);
			if(e->string[4])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[4]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendhref(r,res1);
			sendstr(r,"\">");
			sendtxt(r,res2,1);
			sendstr(r,"</a>");
			if(e->string[6])sendstr(r,e->string[6]);

			if(v&1)
			{
				if(e->string[12])sendstr(r,e->string[12]);
			}
			else if(e->string[10])sendstr(r,e->string[10]);
			if(e->string[8])sendstr(r,e->string[8]);

			v++;
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v&&e->string[1])sendstr(r,e->string[1]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmlnarsym(REQUEST *r,ELEMENT *e)
{
	int v;
	int vv;
	int m;
	int mm;
	int n;
	int nn;
	int c;
	int mark;
	int delta;
	int step;
	int h=-1;
	int rv;
	int res1;
	char *res2;
	char *res3;
	int res4;
	int res5;
	DETAIL *d;
	DETAIL *start;
	DETAIL *x;
	DETAIL **dd;

	v=0;
	nn=0;

	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else rv=0;
	}
	else rv=0;

	if(!rv)for(m=((e->integer[0]&OPT_MERGE)?0:2);m>=0;m--)
	{
		if(usepool)
		{
			if(e->integer[0]&OPT_MERGE)
			{
				if(sqlparams(pool[h].db,
					pool[h].sql[QRY_NARSYMMRG],
					prpqry[QRY_NARSYMMRG].fmt,
					r->catid,r->catid))
				{
					pool[h].errflag=1;
					continue;
				}
			}
			else if(sqlparams(pool[h].db,pool[h].sql[QRY_NARSYMSEC],
				prpqry[QRY_NARSYMSEC].fmt,r->catid,m,
					r->catid,m))
			{
				pool[h].errflag=1;
				continue;
			}
		}
		else if(e->integer[0]&OPT_MERGE)
		{
			if(sqlprep(r->db,&r->sql,prpqry[QRY_NARSYMMRG].sql,
				prpqry[QRY_NARSYMMRG].fmt,
				prpqry[QRY_NARSYMMRG].fmt,r->catid,r->catid))
				continue;
		}
		else if(sqlprep(r->db,&r->sql,prpqry[QRY_NARSYMSEC].sql,
			prpqry[QRY_NARSYMSEC].fmt,prpqry[QRY_NARSYMSEC].fmt,
			r->catid,m,r->catid,m))continue;

		d=NULL;
		dd=&d;
		vv=0;
		n=((e->integer[0]&OPT_MERGE)?QRY_NARSYMMRG:QRY_NARSYMSEC);
		while(1)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,pool[h].sql[n],
					prpqry[n].res,&res1,&res2,&res3,&res4,
					&res5)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[n].res,&res1,&res2,
				&res3,&res4,&res5))break;
			if(!(*dd=malloc(sizeof(DETAIL))))break;
			(*dd)->next=NULL;
			(*dd)->type=res1;;
			if(!((*dd)->id=strdup(res2)))
			{
				free(*dd);
				*dd=NULL;
				break;
			}
			if(!((*dd)->title=strdup(res3)))
			{
				free((*dd)->id);
				free(*dd);
				*dd=NULL;
				break;
			}
			(*dd)->topics=res4;
			(*dd)->pages=res5;
			dd=&((*dd)->next);

			vv++;
		}
		if(!usepool)sqlfin(r->db,r->sql);
		if(!vv)continue;

		r->outputdone=OFLAG_OUT|OFLAG_DEP;

		if(e->integer[3]&&vv>=e->integer[3])
		{
			delta=((vv*10)/4+9)/10;
			if(e->integer[0]&OPT_HSORT)step=4;
			else step=1;
		}
		else if(e->integer[2]&&vv>=e->integer[2])
		{
			delta=((vv*10)/3+9)/10;
			if(e->integer[0]&OPT_HSORT)step=3;
			else step=1;
		}
		else if(e->integer[1]&&vv>=e->integer[1])
		{
			delta=((vv*10)/2+9)/10;
			if(e->integer[0]&OPT_HSORT)step=2;
			else step=1;
		}
		else
		{
			delta=-1;
			step=1;
		}
		mark=delta;

		if(!v++)if(e->string[0])sendstr(r,e->string[0]);

		if(e->string[32])sendstr(r,e->string[32]);
		switch(m)
		{
		case 2:	if(e->string[26])sendstr(r,e->string[26]);
			break;
		case 1:	if(e->string[28])sendstr(r,e->string[28]);
			break;
		case 0:	if(e->string[30])sendstr(r,e->string[30]);
			break;
		}

		for(start=d,x=d,c=0,n=0,mm=0;mm<vv;mm++,n++)
		{
			if(!(e->integer[0]&OPT_HSORT))
			{
				if(mm==mark)
				{
					if(e->string[2])
						sendstr(r,e->string[2]);
					mark+=delta;
					n=0;
				}
			}
			else
			{
				if(!x)
				{
					if(e->string[2])
						sendstr(r,e->string[2]);
					start=start->next;
					x=start;
					n=0;
				}
			}

			if(!n)switch(++c)
			{
			case 1:	if(e->string[34])sendstr(r,e->string[34]);
				if(e->string[18])sendstr(r,e->string[18]);
				break;
			case 2:	if(e->string[19])sendstr(r,e->string[19]);
				if(e->string[35])sendstr(r,e->string[35]);
				if(e->string[34])sendstr(r,e->string[34]);
				if(e->string[20])sendstr(r,e->string[20]);
				break;
			case 3:	if(e->string[21])sendstr(r,e->string[21]);
				if(e->string[35])sendstr(r,e->string[35]);
				if(e->string[34])sendstr(r,e->string[34]);
				if(e->string[22])sendstr(r,e->string[22]);
				break;
			case 4:	if(e->string[23])sendstr(r,e->string[23]);
				if(e->string[35])sendstr(r,e->string[35]);
				if(e->string[34])sendstr(r,e->string[34]);
				if(e->string[24])sendstr(r,e->string[24]);
				break;
			}

			if(e->string[12])sendstr(r,e->string[12]);
			if(n&1)
			{
				if(e->string[16])sendstr(r,e->string[16]);
			}
			else if(e->string[14])sendstr(r,e->string[14]);

			if(e->string[5])sendstr(r,e->string[5]);
			if(x->type!=1&&(e->integer[0]&OPT_SYMBOLLINK))
				sendstr(r,e->string[10]?e->string[10]:"@");
			if(e->string[4])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[4]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendhref(r,x->id);
			sendstr(r,"\">");
			sendtxt(r,x->title,1);
			sendstr(r,"</a>");
			if(x->type!=1&&(e->integer[0]&OPT_LINKSYMBOL))
				sendstr(r,e->string[10]?e->string[10]:"@");
			if(e->string[6])sendstr(r,e->string[6]);
			if(e->integer[0]&(OPT_TOTALTOPICS|OPT_TOTALPAGES))
			{
				if(e->string[7])sendstr(r,e->string[7]);
				if(e->integer[0]&OPT_TOTALTOPICS)
					sendint(r,x->topics,e->string[11]);
				if(e->string[9])sendstr(r,e->string[9]);
				if(e->integer[0]&OPT_TOTALPAGES)
					sendint(r,x->pages,e->string[11]);
				if(e->string[8])sendstr(r,e->string[8]);
			}

			if(n&1)
			{
				if(e->string[17])sendstr(r,e->string[17]);
			}
			else if(e->string[15])sendstr(r,e->string[15]);
			if(e->string[13])sendstr(r,e->string[13]);

			switch(step)
			{
			case 4:	if(x)x=x->next;
			case 3:	if(x)x=x->next;
			case 2:	if(x)x=x->next;
			case 1:	if(x)x=x->next;
				break;
			}
		}

		switch(c)
		{
		case 1:	if(e->string[19])sendstr(r,e->string[19]);
			if(e->string[35])sendstr(r,e->string[35]);
			break;
		case 2:	if(e->string[21])sendstr(r,e->string[21]);
			if(e->string[35])sendstr(r,e->string[35]);
			break;
		case 3:	if(e->string[23])sendstr(r,e->string[23]);
			if(e->string[35])sendstr(r,e->string[35]);
			break;
		case 4:	if(e->string[25])sendstr(r,e->string[25]);
			if(e->string[35])sendstr(r,e->string[35]);
			break;
		}

		switch(m)
		{
		case 2:	if(e->string[27])sendstr(r,e->string[27]);
			break;
		case 1:	if(e->string[29])sendstr(r,e->string[29]);
			break;
		case 0:	if(e->string[31])sendstr(r,e->string[31]);
			break;
		}
		if(e->string[33])sendstr(r,e->string[33]);

		while(d)
		{
			free(d->id);
			free(d->title);
			dd=(DETAIL **)(d);
			d=d->next;
			free(dd);
		}
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v&&e->string[1])sendstr(r,e->string[1]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmlrelated(REQUEST *r,ELEMENT *e)
{
	int v;
	int m;
	int h=-1;
	int rv;
	int res1;
	int res2;
	char *p;
	char *q;
	char *s;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_RELATED],
			prpqry[QRY_RELATED].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_RELATED].sql,
		prpqry[QRY_RELATED].fmt,prpqry[QRY_RELATED].fmt,r->catid);
	if(!rv)
	{
		while(1)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,
					pool[h].sql[QRY_RELATED],
					prpqry[QRY_RELATED].res,&p,&res1,
					&res2)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_RELATED].res,&p,
				&res1,&res2))break;
			if(!(p=strdup(p)))break;
			if(!v)
			{
				r->outputdone=OFLAG_OUT|OFLAG_DEP;
				if(e->string[1]&&(e->integer[0]&OPT_PRELABEL))
					sendstr(r,e->string[1]);
				if(e->string[18])sendstr(r,e->string[18]);
				if(e->string[0])sendstr(r,e->string[0]);
				else sendstr(r,r->nls[1]);
				if(e->string[19])sendstr(r,e->string[19]);
				if(e->string[1]&&!(e->integer[0]&OPT_PRELABEL))
					sendstr(r,e->string[1]);
			}

			if(e->string[12])sendstr(r,e->string[12]);
			if(v&1)
			{
				if(e->string[16])sendstr(r,e->string[16]);
			}
			else if(e->string[14])sendstr(r,e->string[14]);

			if(e->string[6])sendstr(r,e->string[6]);
			if(e->string[5])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[5]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendhref(r,p);
			sendstr(r,"\">");
			for(q=s=p,m=0;*q;q++)if(*q=='/')
			{
				*q++=0;
				if(m>1)sendstr(r,
					e->string[3]?e->string[3]:": ");
				if(m)sendtxt(r,s,1);
				s=q;
				m++;
			}
			if(m>1)sendstr(r,e->string[3]?e->string[3]:": ");
			if(m)sendtxt(r,s,1);
			sendstr(r,"</a>");
			if(e->string[7])sendstr(r,e->string[7]);
			free(p);

			if(e->integer[0]&(OPT_TOTALTOPICS|OPT_TOTALPAGES))
			{
				if(e->string[8])sendstr(r,e->string[8]);
				if(e->integer[0]&OPT_TOTALTOPICS)
					sendint(r,res1,e->string[11]);
				if(e->string[10])sendstr(r,e->string[10]);
				if(e->integer[0]&OPT_TOTALPAGES)
					sendint(r,res2,e->string[11]);
				if(e->string[9])sendstr(r,e->string[9]);
			}

			if(v&1)
			{
				if(e->string[17])sendstr(r,e->string[17]);
			}
			else if(e->string[15])sendstr(r,e->string[15]);
			if(e->string[13])sendstr(r,e->string[13]);

			v++;
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v&&e->string[2])sendstr(r,e->string[2]);
	else if(!v&&e->string[4])sendstr(r,e->string[4]);
}

static void htmlotherlang(REQUEST *r,ELEMENT *e)
{
	int v;
	int m;
	int n;
	int c;
	int mark;
	int delta;
	int step;
	int h=-1;
	int rv;
	char *res1;
	char *res2;
	int res3;
	int res4;
	DETAIL *d;
	DETAIL *start;
	DETAIL *x;
	DETAIL **dd;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_LANGUAGES],
			prpqry[QRY_LANGUAGES].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_LANGUAGES].sql,
		prpqry[QRY_LANGUAGES].fmt,prpqry[QRY_LANGUAGES].fmt,r->catid);
	if(!rv)
	{
		d=NULL;
		dd=&d;
		while(1)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,
					pool[h].sql[QRY_LANGUAGES],
					prpqry[QRY_LANGUAGES].res,&res1,&res2,
					&res3,&res4)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_LANGUAGES].res,
				&res1,&res2,&res3,&res4))break;
			if(!(*dd=malloc(sizeof(DETAIL))))break;
			(*dd)->next=NULL;
			if(!((*dd)->id=strdup(res1)))
			{
				free(*dd);
				*dd=NULL;
				break;
			}
			if(!((*dd)->title=strdup(res2)))
			{
				free((*dd)->id);
				free(*dd);
				*dd=NULL;
				break;
			}
			(*dd)->topics=res3;
			(*dd)->pages=res4;
			dd=&((*dd)->next);

			v++;
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(!v)
	{
		if(e->string[4])sendstr(r,e->string[4]);
		return;
	}

	r->outputdone=OFLAG_OUT|OFLAG_DEP;

	if(e->integer[5]&&v>=e->integer[5])
	{
		delta=((v*10)/6+9)/10;
		if(e->integer[0]&OPT_HSORT)step=6;
		else step=1;
	}
	else if(e->integer[4]&&v>=e->integer[4])
	{
		delta=((v*10)/5+9)/10;
		if(e->integer[0]&OPT_HSORT)step=5;
		else step=1;
	}
	else if(e->integer[3]&&v>=e->integer[3])
	{
		delta=((v*10)/4+9)/10;
		if(e->integer[0]&OPT_HSORT)step=4;
		else step=1;
	}
	else if(e->integer[2]&&v>=e->integer[2])
	{
		delta=((v*10)/3+9)/10;
		if(e->integer[0]&OPT_HSORT)step=3;
		else step=1;
	}
	else if(e->integer[1]&&v>=e->integer[1])
	{
		delta=((v*10)/2+9)/10;
		if(e->integer[0]&OPT_HSORT)step=2;
		else step=1;
	}
	else
	{
		delta=-1;
		step=1;
	}
	mark=delta;

	if(e->string[1]&&(e->integer[0]&OPT_PRELABEL))sendstr(r,e->string[1]);
	if(e->string[28])sendstr(r,e->string[28]);
	if(e->string[0])sendstr(r,e->string[0]);
	else sendstr(r,r->nls[2]);
	if(e->string[29])sendstr(r,e->string[29]);
	if(e->string[1]&&!(e->integer[0]&OPT_PRELABEL))sendstr(r,e->string[1]);

	for(start=d,x=d,n=0,c=0,m=0;m<v;m++,n++)
	{
		if(!(e->integer[0]&OPT_HSORT))
		{
			if(m==mark)
			{
				if(e->string[3])sendstr(r,e->string[3]);
				mark+=delta;
				n=0;
			}
		}
		else
		{
			if(!x)
			{
				if(e->string[3])sendstr(r,e->string[3]);
				start=start->next;
				x=start;
				n=0;
			}
		}

		if(!n)switch(++c)
		{
		case 1:	if(e->string[26])sendstr(r,e->string[26]);
			if(e->string[18])sendstr(r,e->string[18]);
			break;
		case 2:	if(e->string[19])sendstr(r,e->string[19]);
			if(e->string[27])sendstr(r,e->string[27]);
			if(e->string[26])sendstr(r,e->string[26]);
			if(e->string[20])sendstr(r,e->string[20]);
			break;
		case 3:	if(e->string[21])sendstr(r,e->string[21]);
			if(e->string[27])sendstr(r,e->string[27]);
			if(e->string[26])sendstr(r,e->string[26]);
			if(e->string[22])sendstr(r,e->string[22]);
			break;
		case 4:	if(e->string[23])sendstr(r,e->string[23]);
			if(e->string[27])sendstr(r,e->string[27]);
			if(e->string[26])sendstr(r,e->string[26]);
			if(e->string[24])sendstr(r,e->string[24]);
			break;
		case 5:	if(e->string[31])sendstr(r,e->string[31]);
			if(e->string[27])sendstr(r,e->string[27]);
			if(e->string[26])sendstr(r,e->string[26]);
			if(e->string[30])sendstr(r,e->string[30]);
			break;
		case 6:	if(e->string[33])sendstr(r,e->string[33]);
			if(e->string[27])sendstr(r,e->string[27]);
			if(e->string[26])sendstr(r,e->string[26]);
			if(e->string[32])sendstr(r,e->string[32]);
			break;
		}

		if(e->string[12])sendstr(r,e->string[12]);
		if(n&1)
		{
			if(e->string[16])sendstr(r,e->string[16]);
		}
		else if(e->string[14])sendstr(r,e->string[14]);

		if(e->string[6])sendstr(r,e->string[6]);
		if(e->string[5])
		{
			sendstr(r,"<a class=\"");
			sendstr(r,e->string[5]);
			sendstr(r,"\" href=\"");
		}
		else sendstr(r,"<a href=\"");
		sendhref(r,x->id);
		sendstr(r,"\">");
		sendstr(r,x->title);
		sendstr(r,"</a>");
		if(e->string[7])sendstr(r,e->string[7]);

		if(e->integer[0]&(OPT_TOTALTOPICS|OPT_TOTALPAGES))
		{
			if(e->string[8])sendstr(r,e->string[8]);
			if(e->integer[0]&OPT_TOTALTOPICS)
				sendint(r,x->topics,e->string[11]);
			if(e->string[10])sendstr(r,e->string[10]);
			if(e->integer[0]&OPT_TOTALPAGES)
				sendint(r,x->pages,e->string[11]);
			if(e->string[9])sendstr(r,e->string[9]);
		}

		if(n&1)
		{
			if(e->string[17])sendstr(r,e->string[17]);
		}
		else if(e->string[15])sendstr(r,e->string[15]);
		if(e->string[13])sendstr(r,e->string[13]);

		switch(step)
		{
		case 6:	if(x)x=x->next;
		case 5:	if(x)x=x->next;
		case 4:	if(x)x=x->next;
		case 3:	if(x)x=x->next;
		case 2:	if(x)x=x->next;
		case 1:	if(x)x=x->next;
			break;
		}
	}

	switch(c)
	{
	case 1:	if(e->string[19])sendstr(r,e->string[19]);
		if(e->string[27])sendstr(r,e->string[27]);
		break;
	case 2:	if(e->string[21])sendstr(r,e->string[21]);
		if(e->string[27])sendstr(r,e->string[27]);
		break;
	case 3:	if(e->string[23])sendstr(r,e->string[23]);
		if(e->string[27])sendstr(r,e->string[27]);
		break;
	case 4:	if(e->string[25])sendstr(r,e->string[25]);
		if(e->string[27])sendstr(r,e->string[27]);
		break;
	case 5:	if(e->string[31])sendstr(r,e->string[31]);
		if(e->string[27])sendstr(r,e->string[27]);
		break;
	case 6:	if(e->string[33])sendstr(r,e->string[33]);
		if(e->string[27])sendstr(r,e->string[27]);
		break;
	}

	while(d)
	{
		free(d->id);
		free(d->title);
		dd=(DETAIL **)(d);
		d=d->next;
		free(dd);
	}

	if(e->string[2])sendstr(r,e->string[2]);
}

static void htmlcontent(REQUEST *r,ELEMENT *e)
{
	int v;
	int m;
	int h=-1;
	int rv;
	int res1;
	int res2;
	char *res3;
	char *res4;
	char *res5;
	char *res7;
	int res8;
	int res9;
	char *p;
	char *q;
	char *s;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_EXTPAGES],
			prpqry[QRY_EXTPAGES].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_EXTPAGES].sql,
		prpqry[QRY_EXTPAGES].fmt,prpqry[QRY_EXTPAGES].fmt,r->catid);
	if(!rv)
	{
		while(1)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,
					pool[h].sql[QRY_EXTPAGES],
					prpqry[QRY_EXTPAGES].res,&res1,&res2,
					&res3,&res4,&res5,&p,&res7,&res8,
					&res9)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_EXTPAGES].res,
				&res1,&res2,&res3,&res4,&res5,&p,&res7,&res8,
				&res9))break;
			if(!v)
			{
				r->outputdone=OFLAG_OUT|OFLAG_DEP;
				if(e->string[0])sendstr(r,e->string[0]);
			}

			if(e->string[23])sendstr(r,e->string[23]);
			if(v&1)
			{
				if(e->string[27])sendstr(r,e->string[27]);
			}
			else if(e->string[25])sendstr(r,e->string[25]);

			if(e->string[5])sendstr(r,e->string[5]);
			if(e->integer[0]&OPT_TYPELINK)switch(res2)
			{
			case PDF:
				sendstr(r,e->string[9]?e->string[9]:"[pdf] ");
				break;
			case RSS:
				sendstr(r,
					e->string[10]?e->string[10]:"[rss] ");
				break;
			case ATOM:
				sendstr(r,
					e->string[11]?e->string[11]:"[atom] ");
				break;
			}
			if(res1)sendstr(r,e->string[7]?e->string[7]:"<b>");
			if(e->string[4])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[4]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendstr(r,res3);
			sendstr(r,"\">");
			sendtxt(r,res4,0);
			sendstr(r,"</a>");
			if(res1)sendstr(r,e->string[8]?e->string[8]:"</b>");
			if(e->integer[0]&OPT_LINKTYPE)switch(res2)
			{
			case PDF:
				sendstr(r,e->string[9]?e->string[9]:" [pdf]");
				break;
			case RSS:
				sendstr(r,
					e->string[10]?e->string[10]:" [rss]");
				break;
			case ATOM:
				sendstr(r,
					e->string[11]?e->string[11]:" [atom]");
				break;
			}
			if(e->string[6])sendstr(r,e->string[6]);

			if(*res5)sendstr(r,e->string[2]?e->string[2]:" - ");
			else if(p)if(*p)if(e->integer[0]&OPT_MEDIADATE)
				sendstr(r,e->string[2]?e->string[2]:" - ");
			sendtxt(r,res5,0);

			if(p)if(*p)if(e->integer[0]&OPT_MEDIADATE)
			{
				sendstr(r,e->string[12]?e->string[12]:"(");
				sendstr(r,p);
				sendstr(r,e->string[13]?e->string[13]:")\n");
			}

			if(e->integer[0]&OPT_TARGET)
			{
				if(e->string[14])sendstr(r,e->string[14]);
				sendtxt(r,res3,0);
				if(e->string[15])sendstr(r,e->string[15]);
			}

			if(e->integer[0]&OPT_TOPIC)
			{
				if(e->string[17])sendstr(r,e->string[17]);
				if(e->integer[0]&OPT_LINK)
				{
					if(e->string[4])
					{
						sendstr(r,"<a class=\"");
						sendstr(r,e->string[4]);
						sendstr(r,"\" href=\"");
					}
					else sendstr(r,"<a href=\"");
					sendhref(r,res7);
					sendstr(r,"\">");
				}
				if((p=strdup(res7)))
				{
					for(m=0,q=s=p;*q;q++)if(*q=='/')
					{
						*q++=0;
						if(m>1)sendstr(r,e->string[16]?
							e->string[16]:": ");
						if(m)sendtxt(r,s,1);
						m++;
						s=q;
					}
					if(m>1)sendstr(r,e->string[16]?
						e->string[16]:": ");
					if(m)sendtxt(r,s,1);
					free(p);
				}
				if(e->integer[0]&OPT_LINK)sendstr(r,"</a>");
				if(e->string[18])sendstr(r,e->string[18]);
			}

			if(e->integer[0]&(OPT_TOTALTOPICS|OPT_TOTALPAGES))
			{
				if(e->string[19])sendstr(r,e->string[19]);
				if(e->integer[0]&OPT_TOTALTOPICS)
					sendint(r,res8,e->string[22]);
				if(e->string[21])sendstr(r,e->string[21]);
				if(e->integer[0]&OPT_TOTALPAGES)
					sendint(r,res9,e->string[22]);
				if(e->string[20])sendstr(r,e->string[20]);
			}

			if(v&1)
			{
				if(e->string[28])sendstr(r,e->string[28]);
			}
			else if(e->string[26])sendstr(r,e->string[26]);
			if(e->string[24])sendstr(r,e->string[24]);

			v++;
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v&&e->string[1])sendstr(r,e->string[1]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmlnewsgroup(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	char *res;
	ELEMENT *l;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_NEWSGROUP],
			prpqry[QRY_NEWSGROUP].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_NEWSGROUP].sql,
		prpqry[QRY_NEWSGROUP].fmt,prpqry[QRY_NEWSGROUP].fmt,r->catid);
	if(!rv)
	{
		while(1)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,
					pool[h].sql[QRY_NEWSGROUP],
					prpqry[QRY_NEWSGROUP].res,&res)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_NEWSGROUP].res,
				&res))break;
			if(!v)
			{
				r->outputdone=OFLAG_OUT|OFLAG_DEP;
				if(e->string[0])sendstr(r,e->string[0]);
			}

			if(e->string[7])sendstr(r,e->string[7]);
			if(v&1)
			{
				if(e->string[11])sendstr(r,e->string[11]);
			}
			else if(e->string[9])sendstr(r,e->string[9]);

			if(e->string[5])sendstr(r,e->string[5]);
			sendtxt(r,res+5,0);
			if(e->string[6])sendstr(r,e->string[6]);

			if(e->integer[0]&OPT_NEWS)
			{
				sendstr(r,e->string[2]?e->string[2]:" - ");
				if(e->string[4])
				{
					sendstr(r,"<a class=\"");
					sendstr(r,e->string[4]);
					sendstr(r,"\" href=\"");
				}
				else sendstr(r,"<a href=\"");
				sendstr(r,res);
				sendstr(r,"\">news:</a>");
			}

			if(r->tmpl->anchor[0])
				for(l=r->tmpl->anchor[0]->data;l;l=l->next)
			{
				sendstr(r,e->string[2]?e->string[2]:" - ");
				if(e->string[4])
				{
					sendstr(r,"<a class=\"");
					sendstr(r,e->string[4]);
					sendstr(r,"\" href=\"");
				}
				else sendstr(r,"<a href=\"");
				sendstr(r,l->string[1]);
				sendstr(r,res+5);
				sendstr(r,l->string[2]);
				sendstr(r,"\">");
				sendstr(r,l->string[0]);
				sendstr(r,"</a>");
			}

			if(v&1)
			{
				if(e->string[12])sendstr(r,e->string[12]);
			}
			else if(e->string[10])sendstr(r,e->string[10]);
			if(e->string[8])sendstr(r,e->string[8]);

			v++;
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v&&e->string[1])sendstr(r,e->string[1]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmlsearchinfo(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	char *p;
	char *q;
	char *s;

	p=NULL;
	if(e->string[3])p=strdup(e->string[3]);
	else if(r->pagetype==RTYPE_QUERY)p=strdup(r->data);
	else
	{
		if(usepool)
		{
			if((h=getconn())==-1)rv=-1;
			else if(sqlparams(pool[h].db,pool[h].sql[QRY_TITLE],
				prpqry[QRY_TITLE].fmt,r->catid))
			{
				pool[h].errflag=1;
				rv=-1;
			}
			else rv=0;
		}
		else rv=sqlprep(r->db,&r->sql,prpqry[QRY_TITLE].sql,
			prpqry[QRY_TITLE].fmt,prpqry[QRY_TITLE].fmt,r->catid);
		if(!rv)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_TITLE],
					prpqry[QRY_TITLE].res,&p)))
				{
					if(rv==-1)pool[h].errflag=1;
				}
				else p=strdup(p);
			}
			else if(!sqlrow(r->db,r->sql,prpqry[QRY_TITLE].res,&p))
				p=strdup(p);
			if(!usepool)sqlfin(r->db,r->sql);
		}
		if(usepool)if(h!=-1)relconn(h);
		if(p)for(q=p;*q;q++)if(*q=='_')*q=' ';
	}

	if(!p)
	{
		if(e->string[2])sendstr(r,e->string[2]);
		return;
	}

	r->outputdone=OFLAG_OUT|OFLAG_DEP;

	if(e->string[0])sendstr(r,e->string[0]);

	if(e->string[4])sendstr(r,e->string[4]);
	else
	{
		sendstr(r,r->nls[5]);
		sendstr(r,"\"<b>");
	}
	for(v=0,s=q=p;*q;q++,v++)
		if(*q==' '&&v>=(e->integer[0]?e->integer[0]:50))
	{
		*q=0;
		sendtxt(r,s,1);
		*q=' ';
		sendstr(r,"<br />");
		s=q+1;
		v=0;
	}
	sendtxt(r,s,1);
	if(e->string[5])sendstr(r,e->string[5]);
	else
	{
		sendstr(r,"</b>\"");
		sendstr(r,r->nls[6]);
	}

	if(e->string[1])sendstr(r,e->string[1]);
	free(p);
}

static void htmlexternalsearch(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	char *p;
	char *q;
	ELEMENT *l;

	if(!r->tmpl->anchor[1])
	{
		if(e->string[3])sendstr(r,e->string[3]);
		return;
	}

	p=NULL;
	if(e->string[5])p=strdup(e->string[5]);
	else if(r->pagetype==RTYPE_QUERY)p=strdup(r->data);
	else
	{
		if(usepool)
		{
			if((h=getconn())==-1)rv=-1;
			else if(sqlparams(pool[h].db,pool[h].sql[QRY_TITLE],
				prpqry[QRY_TITLE].fmt,r->catid))
			{
				pool[h].errflag=1;
				rv=-1;
			}
			else rv=0;
		}
		else rv=sqlprep(r->db,&r->sql,prpqry[QRY_TITLE].sql,
			prpqry[QRY_TITLE].fmt,prpqry[QRY_TITLE].fmt,r->catid);
		if(!rv)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_TITLE],
					prpqry[QRY_TITLE].res,&p)))
				{
					if(rv==-1)pool[h].errflag=1;
				}
				else p=strdup(p);
			}
			else if(!sqlrow(r->db,r->sql,prpqry[QRY_TITLE].res,&p))
				p=strdup(p);
			if(!usepool)sqlfin(r->db,r->sql);
		}
		if(usepool)if(h!=-1)relconn(h);
		if(p)for(q=p;*q;q++)if(*q=='_')*q=' ';
	}
	if(!p)
	{
		if(e->string[3])sendstr(r,e->string[3]);
		return;
	}

	r->outputdone=OFLAG_OUT|OFLAG_DEP;

	if(e->string[0])sendstr(r,e->string[0]);

	for(v=0,l=r->tmpl->anchor[1]->data;l;l=l->next,v++)
	{
		if(e->string[6])sendstr(r,e->string[6]);
		if(v&1)
		{
			if(e->string[10])sendstr(r,e->string[10]);
		}
		else if(e->string[8])sendstr(r,e->string[8]);

		if(v)sendstr(r,e->string[2]?e->string[2]:" ");

		if(e->string[4])
		{
			sendstr(r,"<a class=\"");
			sendstr(r,e->string[4]);
			sendstr(r,"\" href=\"");
		}
		else sendstr(r,"<a href=\"");
		sendstr(r,l->string[1]);
		sendelement(r,p,1);
		sendstr(r,l->string[2]);
		sendstr(r,"\">");
		sendstr(r,l->string[0]);
		sendstr(r,"</a>");

		if(v&1)
		{
			if(e->string[11])sendstr(r,e->string[11]);
		}
		else if(e->string[9])sendstr(r,e->string[9]);
		if(e->string[7])sendstr(r,e->string[7]);
	}

	if(e->string[1])sendstr(r,e->string[1]);
	free(p);
}

static void htmleditor(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	char *res;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_EDITOR],
			prpqry[QRY_EDITOR].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_EDITOR].sql,
		prpqry[QRY_EDITOR].fmt,prpqry[QRY_EDITOR].fmt,r->catid);
	if(!rv)
	{
		while(1)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,
					pool[h].sql[QRY_EDITOR],
					prpqry[QRY_EDITOR].res,&res)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_EDITOR].res,
				&res))break;
			if(!v)
			{
				r->outputdone=OFLAG_OUT|OFLAG_DEP;
				if(e->string[1]&&(e->integer[0]&OPT_PRELABEL))
					sendstr(r,e->string[1]);
				if(e->string[0])sendstr(r,e->string[0]);
				else
				{
					sendstr(r,r->nls[4]);
					sendchr(r,' ');
				}
				if(e->string[1]&&!(e->integer[0]&OPT_PRELABEL))
					sendstr(r,e->string[1]);
			}

			if(e->string[6])sendstr(r,e->string[6]);
			if(v&1)
			{
				if(e->string[10])sendstr(r,e->string[10]);
			}
			else if(e->string[8])sendstr(r,e->string[8]);

			if(v)sendstr(r,e->string[3]?e->string[3]:", ");

			if(e->string[5])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[5]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendstr(r,"http://dmoz.org/profiles/");
			sendelement(r,res,0);
			sendstr(r,".html\">");
			sendtxt(r,res,0);
			sendstr(r,"</a>");

			if(v&1)
			{
				if(e->string[11])sendstr(r,e->string[11]);
			}
			else if(e->string[9])sendstr(r,e->string[9]);
			if(e->string[7])sendstr(r,e->string[7]);

			v++;
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v&&e->string[2])sendstr(r,e->string[2]);
	else if(!v&&e->string[4])sendstr(r,e->string[4]);
}

static void htmllastupdate(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	char *p;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_LASTUPDATE],
			prpqry[QRY_LASTUPDATE].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_LASTUPDATE].sql,
		prpqry[QRY_LASTUPDATE].fmt,prpqry[QRY_LASTUPDATE].fmt,r->catid);
	if(!rv)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_LASTUPDATE],
				prpqry[QRY_LASTUPDATE].res,&p))==-1)
				pool[h].errflag=1;
		}
		else rv=sqlrow(r->db,r->sql,prpqry[QRY_LASTUPDATE].res,&p);
		if(!rv)
		{
			v=1;
			if(e->string[1]&&(e->integer[0]&OPT_PRELABEL))
				sendstr(r,e->string[1]);
			if(e->string[0])sendstr(r,e->string[0]);
			else
			{
				sendstr(r,r->nls[7]);
				sendchr(r,' ');
			}
			if(e->string[1]&&!(e->integer[0]&OPT_PRELABEL))
				sendstr(r,e->string[1]);
			sendstr(r,p);
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(v&&e->string[2])sendstr(r,e->string[2]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmltermsofuse(REQUEST *r,ELEMENT *e)
{
	if(e->string[1])sendstr(r,e->string[1]);
	if(e->string[3])
	{
		sendstr(r,"<a class=\"");
		sendstr(r,e->string[3]);
		sendstr(r,"\" href=\"");
	}
	else sendstr(r,"<a href=\"");
	sendstr(r,"http://dmoz.org/termsofuse.html\">");
	if(e->string[0])sendstr(r,e->string[0]);
	else sendstr(r,r->nls[8]);
	sendstr(r,"</a>");
	if(e->string[2])sendstr(r,e->string[2]);
}

static void htmlsearchform(REQUEST *r,ELEMENT *e)
{
	int i;
	int v;
	int h=-1;
	int rv;
	char *p;

	p=NULL;
	if(r->catid!=TOPCATID&&(e->integer[0]&OPT_WHERE))
		if(!(e->integer[0]&OPT_GLOBAL))
	{
		if(usepool)
		{
			if((h=getconn())==-1)rv=-1;
			else if(sqlparams(pool[h].db,pool[h].sql[QRY_TITLE],
				prpqry[QRY_TITLE].fmt,r->catid))
			{
				pool[h].errflag=1;
				rv=-1;
			}
			else rv=0;
		}
		else rv=sqlprep(r->db,&r->sql,prpqry[QRY_TITLE].sql,
			prpqry[QRY_TITLE].fmt,prpqry[QRY_TITLE].fmt,r->catid);
		if(!rv)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_TITLE],
					prpqry[QRY_TITLE].res,&p)))
				{
					if(rv==-1)pool[h].errflag=1;
				}
				else p=strdup(p);
			}
			else if(!sqlrow(r->db,r->sql,prpqry[QRY_TITLE].res,&p))
				p=strdup(p);
			if(!usepool)sqlfin(r->db,r->sql);
		}
	}

	if(e->string[0])sendstr(r,e->string[0]);

	sendstr(r,"<form accept-charset=\"UTF-8\" action=\"/search.cgi\" "
		"method=\"get\">");

	if(e->string[3])sendstr(r,e->string[3]);
	if(!p&&!(e->integer[0]&OPT_GLOBAL))sendstr(r,"<input type=\"hidden\" "
		"name=\"all\" value=\"yes\"/>");
	if(!(e->integer[0]&OPT_GLOBAL))
	{
		sendstr(r,"<input type=\"hidden\" name=\"catid\" value=\"");
		sendint(r,r->catid,NULL);
		sendstr(r,"\"/>");
	}
	sendstr(r,"<input size=\"");
	if(e->integer[1])sendint(r,e->integer[1],NULL);
	else sendstr(r,"30");
	sendstr(r,"\" name=\"search\" type=\"text\" value=\"");
	if(r->data)sendtxt(r,r->data,0);
	sendstr(r,"\"/>");
	if(e->string[4])sendstr(r,e->string[4]);

	if(e->string[6])sendstr(r,e->string[6]);
	sendstr(r,"<input type=\"submit\" value=\"");
	if(e->string[5])sendstr(r,e->string[5]);
	else sendstr(r,r->nls[9]);
	sendstr(r,"\"/>");
	if(e->string[7])sendstr(r,e->string[7]);

	if(p)
	{
		if(e->string[10])sendstr(r,e->string[10]);
		sendstr(r,"<select name=\"all\"><option selected=\"selected\" "
			"value=\"yes\">");
		if(e->string[8])sendstr(r,e->string[8]);
		else sendstr(r,r->nls[10]);
		sendstr(r,"</option><option value=\"no\">");
		if(e->string[9])sendstr(r,e->string[9]);
		else
		{
			sendstr(r,r->nls[11]);
			sendchr(r,' ');
		}
		sendtxt(r,p,1);
		sendstr(r,"</option></select>");
		if(e->string[11])sendstr(r,e->string[11]);
		free(p);
	}

	if(e->integer[0]&OPT_GLOBAL)
	{
		if(usepool)
		{
			rv=0;
			if(h==-1)if((h=getconn())==-1)rv=-1;
			if(!rv)if(sqlparams(pool[h].db,pool[h].sql[QRY_GLOBAL],
				prpqry[QRY_GLOBAL].fmt,TOPCATID))
			{
				pool[h].errflag=1;
				rv=-1;
			}
		}
		else rv=sqlprep(r->db,&r->sql,
			prpqry[QRY_GLOBAL].sql,prpqry[QRY_GLOBAL].fmt,
			prpqry[QRY_GLOBAL].fmt,TOPCATID);
		if(!rv)
		{
			v=0;
			while(1)
			{
				if(usepool)
				{
					if((rv=sqlrow(pool[h].db,
						pool[h].sql[QRY_GLOBAL],
						prpqry[QRY_GLOBAL].res,&i,&p)))
					{
						if(rv==-1)pool[h].errflag=1;
						break;
					}
				}
				else if(sqlrow(r->db,r->sql,
					prpqry[QRY_GLOBAL].res,&i,&p))break;
				if(!v)
				{
					if(e->string[10])
						sendstr(r,e->string[10]);
					sendstr(r,"<select name=\"catid\">"
						"<option selected=\"selected\" "
						"value=\"");
					sendint(r,TOPCATID,NULL);
					sendstr(r,"\">");
					if(e->string[8])sendstr(r,e->string[8]);
					else sendstr(r,r->nls[10]);
					sendstr(r,"</option>");
				}
				sendstr(r,"<option value=\"");
				sendint(r,i,NULL);
				sendstr(r,"\">");
				if(e->string[9])sendstr(r,e->string[9]);
				else
				{
					sendstr(r,r->nls[11]);
					sendchr(r,' ');
				}
				sendtxt(r,p,1);
				sendstr(r,"</option>");

				v++;
			}
			if(!usepool)sqlfin(r->db,r->sql);
			if(v)
			{
				sendstr(r,"</select>");
				if(e->string[11])sendstr(r,e->string[11]);
			}
		}
	}

	if(usepool)if(h!=-1)relconn(h);

	if(e->integer[0]&OPT_ADVANCED)
	{
		if(e->string[13])sendstr(r,e->string[13]);
		if(e->string[2])
		{
			sendstr(r,"<a class=\"");
			sendstr(r,e->string[2]);
			sendstr(r,"\" href=\"");
		}
		else sendstr(r,"<a href=\"");
		sendstr(r,"/advanced.html\">");
		sendstr(r,e->string[12]?e->string[12]:"advanced");
		sendstr(r,"</a>");
		if(e->string[14])sendstr(r,e->string[14]);
	}

	sendstr(r,"</form>");

	if(e->string[1])sendstr(r,e->string[1]);
}

static void htmladvanced(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	int ires;
	char *tres;

	if(e->string[0])sendstr(r,e->string[0]);

	sendstr(r,"<form accept-charset=\"UTF-8\" action=\"/search.cgi\" "
		"method=\"get\">");

	if(e->string[2])sendstr(r,e->string[2]);
	sendstr(r,"<input size=\"");
	if(e->integer[1])sendint(r,e->integer[1],NULL);
	else sendstr(r,"30");
	sendstr(r,"\" name=\"search\" type=\"text\" value=\"");
	if(r->data)sendtxt(r,r->data,0);
	sendstr(r,"\"/>");
	if(e->string[3])sendstr(r,e->string[3]);

	if(e->string[5])sendstr(r,e->string[5]);
	sendstr(r,"<input type=\"submit\" value=\"");
	if(e->string[4])sendstr(r,e->string[4]);
	else sendstr(r,"Advanced Search");
	sendstr(r,"\"/>");
	if(e->string[6])sendstr(r,e->string[6]);

	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_GLOBAL],
			prpqry[QRY_GLOBAL].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_GLOBAL].sql,
		prpqry[QRY_GLOBAL].fmt,prpqry[QRY_GLOBAL].fmt,r->catid);
	if(!rv)
	{
		v=0;
		while(1)
		{
			if(usepool)
			{
				if((rv=sqlrow(pool[h].db,
					pool[h].sql[QRY_GLOBAL],
					prpqry[QRY_GLOBAL].res,&ires,&tres)))
				{
					if(rv==-1)pool[h].errflag=1;
					break;
				}
			}
			else if(sqlrow(r->db,r->sql,prpqry[QRY_GLOBAL].res,
				&ires,&tres))break;
			if(!v)
			{
				if(e->string[9])sendstr(r,e->string[9]);
				if(e->string[7])sendstr(r,e->string[7]);
				else sendstr(r,
					"Only show results in category: ");
				sendstr(r,"<select name=\"catid\"><option "
					"value=\"");
				sendint(r,r->catid,NULL);
				sendstr(r,"\" selected=\"selected\">");
				if(e->string[8])sendstr(r,e->string[8]);
				else sendstr(r,"ALL");
				sendstr(r,"</option>");
			}
			sendstr(r,"<option value=\"");
			sendint(r,ires,NULL);
			sendstr(r,"\">");
			sendtxt(r,tres,1);
			sendstr(r,"</option>");

			v++;
		}
		if(!usepool)sqlfin(r->db,r->sql);
		if(v)
		{
			sendstr(r,"</select>");
			if(e->string[10])sendstr(r,e->string[10]);
		}
	}

	if(usepool)if(h!=-1)relconn(h);

	if(e->integer[0]&OPT_SEARCHTYPE)
	{
		if(e->string[15])sendstr(r,e->string[15]);
		if(e->string[11])sendstr(r,e->string[11]);
		else sendstr(r,"Search: ");
		sendstr(r,"<input type=\"radio\" name=\"t\" value=\"c\"/>");
		if(e->string[12])sendstr(r,e->string[12]);
		else sendstr(r,"Categories Only ");
		sendstr(r,"<input type=\"radio\" name=\"t\" value=\"s\"/>");
		if(e->string[13])sendstr(r,e->string[13]);
		else sendstr(r,"Sites Only ");
		sendstr(r,"<input type=\"radio\" checked name=\"t\" "
			"value=\"b\"/>");
		if(e->string[14])sendstr(r,e->string[14]);
		else sendstr(r,"Sites and Categories");
		if(e->string[16])sendstr(r,e->string[16]);
	}

	if(e->integer[0]&OPT_KIDSSEARCH)
	{
		if(e->string[21])sendstr(r,e->string[21]);
		if(e->string[17])sendstr(r,e->string[17]);
		else sendstr(r,"Kids and Teens Sites: ");
		sendstr(r,"<input type=\"checkbox\" name=\"kids\" "
			"value=\"1\"/>");
		if(e->string[18])sendstr(r,e->string[18]);
		else sendstr(r," Kids ");
		sendstr(r,"<input type=\"checkbox\" name=\"teens\" "
			"value=\"1\"/>");
		if(e->string[19])sendstr(r,e->string[19]);
		else sendstr(r," Teens ");
		sendstr(r,"<input type=\"checkbox\" name=\"mteens\" "
			"value=\"1\"/>");
		if(e->string[20])sendstr(r,e->string[20]);
		else sendstr(r," Mature Teens");
		if(e->string[22])sendstr(r,e->string[22]);
	}

	sendstr(r,"</form>");

	if(e->string[1])sendstr(r,e->string[1]);
}

static void htmlengines(REQUEST *r,ELEMENT *e)
{
	int v;
	ELEMENT *l;

	if(!r->tmpl->anchor[1])
	{
		if(e->string[4])sendstr(r,e->string[4]);
		return;
	}

	for(v=0,l=r->tmpl->anchor[1]->data;l;l=l->next,v++)
	{
		if(!v)
		{
			if(e->string[1])sendstr(r,e->string[1]);
			if(e->string[0])sendstr(r,e->string[0]);
			else sendstr(r,"search on: ");
		}

		if(e->string[6])sendstr(r,e->string[6]);
		if(v&1)
		{
			if(e->string[10])sendstr(r,e->string[10]);
		}
		else if(e->string[8])sendstr(r,e->string[8]);

		if(v)sendstr(r,e->string[3]?e->string[3]:" ");

		if(e->string[5])
		{
			sendstr(r,"<a class=\"");
			sendstr(r,e->string[5]);
			sendstr(r,"\" href=\"");
		}
		else sendstr(r,"<a href=\"");
		sendstr(r,l->string[3]);
		sendstr(r,"\">");
		sendstr(r,l->string[0]);
		sendstr(r,"</a>");

		if(v&1)
		{
			if(e->string[11])sendstr(r,e->string[11]);
		}
		else if(e->string[9])sendstr(r,e->string[9]);
		if(e->string[7])sendstr(r,e->string[7]);
	}

	if(v&&e->string[2])sendstr(r,e->string[2]);
	else if(!v&&e->string[4])sendstr(r,e->string[4]);
}

static void htmlcatlink(REQUEST *r,ELEMENT *e)
{
	int v;
	int h=-1;
	int rv;
	char *p;

	v=0;
	if(usepool)
	{
		if((h=getconn())==-1)rv=-1;
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_TOPICID],
			prpqry[QRY_TOPICID].fmt,r->catid))
		{
			pool[h].errflag=1;
			rv=-1;
		}
		else rv=0;
	}
	else rv=sqlprep(r->db,&r->sql,prpqry[QRY_TOPICID].sql,
		prpqry[QRY_TOPICID].fmt,prpqry[QRY_TOPICID].fmt,r->catid);
	if(!rv)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_TOPICID],
				prpqry[QRY_TOPICID].res,&p))==-1)
				pool[h].errflag=1;
		}
		else rv=sqlrow(r->db,r->sql,prpqry[QRY_TOPICID].res,&p);
		if(!rv)
		{
			v=1;
			r->outputdone=OFLAG_OUT|OFLAG_DEP;
			if(e->string[1])sendstr(r,e->string[1]);
			if(e->string[4])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[4]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendhref(r,p);
			sendstr(r,"\">");
			if(e->string[0])sendstr(r,e->string[0]);
			else sendstr(r,"Category");
			sendstr(r,"</a>");
			if(e->string[2])sendstr(r,e->string[2]);
		}
		if(!usepool)sqlfin(r->db,r->sql);
	}

	if(usepool)if(h!=-1)relconn(h);

	if(!v&&e->string[3])sendstr(r,e->string[3]);
}

static void htmltopicquery(REQUEST *r,ELEMENT *e)
{
	int v;
	int m;
	int n;
	int nn;
	int total;
	int limit;
	int h=-1;
	int rv;
	int res1;
	int res3;
	int res4;
	char *p;
	char *q;
	char *s;
	int id[MAXSEARCH];
	char *word[MAXSEARCH];
	char bfr[BUFSIZE];

	if(r->start||(r->searchmode&QTYPE_SITESONLY))return;

	v=0;
	total=0;
	nn=0;
	if(buildquery(r->data,bfr,sizeof(bfr),bldqry[QRY_TOPICWORDS].pre,
		bldqry[QRY_TOPICWORDS].post,0,0)!=1)goto out;
	if(usepool)
	{
		if((h=getconn())==-1)goto out;
		else if(sqlprep(pool[h].db,&r->sql,bfr,NULL,NULL))
		{
			pool[h].errflag=1;
			goto out;
		}
	}
	else if(sqlprep(r->db,&r->sql,bfr,NULL,NULL))goto out;
	while(1)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,r->sql,
				bldqry[QRY_TOPICWORDS].res,&id[total],
				&word[total])))
			{
				if(rv==-1)pool[h].errflag=1;
				break;
			}
		}
		else if(sqlrow(r->db,r->sql,bldqry[QRY_TOPICWORDS].res,
			&id[total],&word[total]))break;
		if(!(word[total]=strdup(word[total])))break;

		total++;
	}
	sqlfin(usepool?pool[h].db:r->db,r->sql);
	if(!total)goto out;

	n=(r->ktm?((r->searchmode&QTYPE_ALL)?QRY_TOPICAGETOP:QRY_TOPICAGECAT):
		((r->searchmode&QTYPE_ALL)?QRY_TOPICTOP:QRY_TOPICCAT));

	if(usepool)
	{
		if(tmptables[TBL_TOPICLIST].truncate)
		{
			if(sqlrun(pool[h].db,tmptables[TBL_TOPICLIST].truncate))
				goto out;
		}
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_CLTOPICLIST],NULL))
		{
			pool[h].errflag=1;
			goto out;
		}
		else if(sqlrow(pool[h].db,pool[h].sql[QRY_CLTOPICLIST],NULL)
			==-1)
		{
			pool[h].errflag=1;
			goto out;
		}
	}
	else
	{
		if(sqlrun(r->db,tmptables[TBL_TOPICLIST].create))goto out;
		nn++;
		if(sqlprep(r->db,&r->sql,prpqry[n].sql,prpqry[n].fmt,NULL))
			goto out;
	}

	for(m=0;m<total;m++)
	{
		if(usepool)
		{
			if(sqlparams(pool[h].db,pool[h].sql[n],prpqry[n].fmt,
				id[m],r->catid))
			{
				pool[h].errflag=1;
				goto out;
			}
		}
		else if(sqlparams(r->db,r->sql,prpqry[n].fmt,id[m],r->catid))
		{
			sqlfin(r->db,r->sql);
			goto out;
		}
		if(usepool)
		{
			if(sqlrow(pool[h].db,pool[h].sql[n],NULL)==-1)
			{
				pool[h].errflag=1;
				goto out;
			}
		}
		else if(sqlrow(r->db,r->sql,NULL)==-1)
		{
			sqlfin(r->db,r->sql);
			goto out;
		}
	}
	if(!usepool)sqlfin(r->db,r->sql);

	if(usepool)
	{
		if(sqlparams(pool[h].db,pool[h].sql[QRY_TOPICRESSEL],NULL))
		{
			pool[h].errflag=1;
			goto out;
		}
		if(tmptables[TBL_TOPICRES].truncate)
		{
			if(sqlrun(pool[h].db,tmptables[TBL_TOPICRES].truncate))
				goto out;
		}
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_CLTOPICRES],NULL))
		{
			pool[h].errflag=1;
			goto out;
		}
		else if(sqlrow(pool[h].db,pool[h].sql[QRY_CLTOPICRES],NULL)==-1)
		{
			pool[h].errflag=1;
			goto out;
		}
		if(sqlparams(pool[h].db,pool[h].sql[QRY_TOPICRESINS],NULL))
		{
			pool[h].errflag=1;
			goto out;
		}
		if(sqlrow(pool[h].db,pool[h].sql[QRY_TOPICRESINS],NULL)==-1)
		{
			pool[h].errflag=1;
			goto out;
		}
	}
	else
	{
		if(sqlrun(r->db,tmptables[TBL_TOPICRES].create))goto out;
		nn++;
		if(sqlrun(r->db,prpqry[QRY_TOPICRESINS].sql))goto out;
		if(sqlprep(r->db,&r->sql,prpqry[QRY_TOPICRESSEL].sql,NULL,NULL))
			goto out;
	}

	limit=e->integer[1]?e->integer[1]:5;

	while(1)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_TOPICRESSEL],
				prpqry[QRY_TOPICRESSEL].res,&res1,&p,&res3,
				&res4)))
			{
				if(rv==-1)pool[h].errflag=1;
				break;
			}
		}
		else if(sqlrow(r->db,r->sql,prpqry[QRY_TOPICRESSEL].res,&res1,
			&p,&res3,&res4))break;
		if(!(p=strdup(p)))continue;
		if(!strchr(p,'/'))
		{
			free(p);
			continue;
		}

		if(!v++)
		{
			r->outputdone=OFLAG_OUT|OFLAG_DEP;
			if(e->string[0])sendstr(r,e->string[0]);
			if(e->string[5])for(q=e->string[5];*q;q++)
			{
				if(*q!='#')senddata(r,q,1);
				else sendint(r,r->start+1,NULL);
			}
		}
		else if(!(r->searchmode&QTYPE_FULLCAT)&&v>limit)
		{
			free(p);
			break;
		}

		if(e->string[16])for(q=e->string[16];*q;q++)
		{
			if(*q!='#')senddata(r,q,1);
			else sendint(r,r->start+v,NULL);
		}
		if(!(v&1))
		{
			if(e->string[20])for(q=e->string[20];*q;q++)
			{
				if(*q!='#')senddata(r,q,1);
				else sendint(r,r->start+v,NULL);
			}
		}
		else if(e->string[18])for(q=e->string[18];*q;q++)
		{
			if(*q!='#')senddata(r,q,1);
			else sendint(r,r->start+v,NULL);
		}

		if(e->string[7])sendstr(r,e->string[7]);
		if(e->string[4])
		{
			sendstr(r,"<a class=\"");
			sendstr(r,e->string[4]);
			sendstr(r,"\" href=\"");
		}
		else sendstr(r,"<a href=\"");
		sendhref(r,p);
		sendstr(r,"\">");
		for(m=0,q=s=p;*q;q++)if(*q=='/')
		{
			*q++=0;
			if(m>1)sendstr(r,e->string[2]?e->string[2]:": ");
			if(m)
			{
				if(e->integer[0]&OPT_MARK&&
				    (e->string[22]||e->string[23]))
					markit(r,s,total,word,e->string[22],
					    e->string[23],1);
				else sendtxt(r,s,1);
			}
			m++;
			s=q;
		}
		if(m>1)sendstr(r,e->string[2]?e->string[2]:": ");
		if(m)
		{
			if(e->integer[0]&OPT_MARK&&
			    (e->string[22]||e->string[23]))
				markit(r,s,total,word,e->string[22],
				    e->string[23],1);
			else sendtxt(r,s,1);
		}
		sendstr(r,"</a>");
		if(e->string[8])sendstr(r,e->string[8]);

		if(e->integer[0]&(OPT_TOTALTOPICS|OPT_TOTALPAGES))
		{
			if(e->string[9])sendstr(r,e->string[9]);
			if(e->integer[0]&OPT_LINK)
			{
				if(e->string[4])
				{
					sendstr(r,"<a class=\"");
					sendstr(r,e->string[4]);
					sendstr(r,"\" href=\"");
				}
				else sendstr(r,"<a href=\"");
				sendstr(r,"/search.cgi?search=");
				sendelement(r,r->data,1);
				sendstr(r,"&amp;all=no&amp;catid=");
				sendint(r,res1,NULL);
				sendstr(r,"\">");
			}
			if(e->integer[0]&OPT_TOTALTOPICS)
				sendint(r,res3,e->string[15]);
			if(e->string[11])sendstr(r,e->string[11]);
			if(e->integer[0]&OPT_TOTALPAGES)
				sendint(r,res4,e->string[15]);
			if(e->integer[0]&OPT_LINK)sendstr(r,"</a>");
			if(e->string[10])sendstr(r,e->string[10]);
		}
		free(p);

		if(!(v&1))
		{
			if(e->string[21])for(q=e->string[21];*q;q++)
			{
				if(*q!='#')senddata(r,q,1);
				else sendint(r,r->start+v,NULL);
			}
		}
		else if(e->string[19])for(q=e->string[19];*q;q++)
		{
			if(*q!='#')senddata(r,q,1);
			else sendint(r,r->start+v,NULL);
		}
		if(e->string[17])for(q=e->string[17];*q;q++)
		{
			if(*q!='#')senddata(r,q,1);
			else sendint(r,r->start+v,NULL);
		}
	}
	if(!usepool)sqlfin(r->db,r->sql);
	else if(h!=-1)
	{
		relconn(h);
		h=-1;
	}

	if(v&&e->string[6])sendstr(r,e->string[6]);

	if(!(r->searchmode&QTYPE_FULLCAT)&&(e->integer[0]&OPT_MORE)&&v>limit)
	{
		if(e->string[13])sendstr(r,e->string[13]);
		if(e->string[4])
		{
			sendstr(r,"<a class=\"");
			sendstr(r,e->string[4]);
			sendstr(r,"\" href=\"");
		}
		else sendstr(r,"<a href=\"");
		sendstr(r,"/search.cgi?search=");
		sendelement(r,r->data,1);
		sendstr(r,"&amp;all=yes&amp;allcat=1&amp;catid=");
		sendint(r,r->catid,NULL);
		sendstr(r,"\">");
		sendstr(r,e->string[12]?e->string[12]:"&gt;&gt;&gt;&gt;&gt;");
		sendstr(r,"</a>");
		if(e->string[14])sendstr(r,e->string[14]);
	}

out:	if(!usepool)
	{
		if(nn>1)sqlrun(r->db,tmptables[TBL_TOPICRES].drop);
		if(nn>0)sqlrun(r->db,tmptables[TBL_TOPICLIST].drop);
	}
	else if(h!=-1)relconn(h);

	if(v&&e->string[1])sendstr(r,e->string[1]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);

	while(total--)free(word[total]);
}

static void htmlcontentquery(REQUEST *r,ELEMENT *e)
{
	int v;
	int m;
	int n;
	int nn;
	int total;
	int limit;
	int h=-1;
	int rv;
	int res1;
	int res2;
	char *res3;
	char *res4;
	char *res5;
	char *res6;
	char *res7;
	int res8;
	int res9;
	int res10;
	int res11;
	char *p;
	char *q;
	char *s;
	int id[MAXSEARCH];
	char *word[MAXSEARCH];
	char bfr[BUFSIZE];

	if(r->searchmode&QTYPE_CATONLY)return;

	v=0;
	total=0;
	nn=0;
	if(buildquery(r->data,bfr,sizeof(bfr),bldqry[QRY_PAGEWORDS].pre,
		bldqry[QRY_PAGEWORDS].post,0,0)!=1)goto out;
	if(usepool)
	{
		if((h=getconn())==-1)goto out;
		else if(sqlprep(pool[h].db,&r->sql,bfr,NULL,NULL))
		{
			pool[h].errflag=1;
			goto out;
		}
	}
	else if(sqlprep(r->db,&r->sql,bfr,NULL,NULL))goto out;
	while(1)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,r->sql,
				bldqry[QRY_PAGEWORDS].res,&id[total],
				&word[total])))
			{
				if(rv==-1)pool[h].errflag=1;
				break;
			}
		}
		else if(sqlrow(r->db,r->sql,bldqry[QRY_PAGEWORDS].res,
			&id[total],&word[total]))break;
		if(!(word[total]=strdup(word[total])))break;

		total++;
	}
	sqlfin(usepool?pool[h].db:r->db,r->sql);
	if(!total)goto out;

	n=(r->ktm?((r->searchmode&QTYPE_ALL)?QRY_PGRESAGETOP:QRY_PGRESAGECAT):
		((r->searchmode&QTYPE_ALL)?QRY_PGRESTOP:QRY_PGRESCAT));

	if(usepool)
	{
		if(tmptables[TBL_RESULTS].truncate)
		{
			if(sqlrun(pool[h].db,tmptables[TBL_RESULTS].truncate))
				goto out;
		}
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_CLRESULTS],NULL))
		{
			pool[h].errflag=1;
			goto out;
		}
		else if(sqlrow(pool[h].db,pool[h].sql[QRY_CLRESULTS],NULL)==-1)
		{
			pool[h].errflag=1;
			goto out;
		}
	}
	else
	{
		if(sqlrun(r->db,tmptables[TBL_RESULTS].create))goto out;
		nn++;
		if(sqlprep(r->db,&r->sql,prpqry[n].sql,prpqry[n].fmt,NULL))
		goto out;
	}

	for(m=0;m<total;m++)
	{
		if(usepool)
		{
			if(sqlparams(pool[h].db,pool[h].sql[n],prpqry[n].fmt,
				id[m],r->ktm?r->ktm:r->catid,r->catid))
			{
				pool[h].errflag=1;
				goto out;
			}
		}
		else if(sqlparams(r->db,r->sql,prpqry[n].fmt,id[m],
			r->ktm?r->ktm:r->catid,r->catid))
		{
			sqlfin(r->db,r->sql);
			goto out;
		}
		if(usepool)
		{
			if(sqlrow(pool[h].db,pool[h].sql[n],NULL)==-1)
			{
				pool[h].errflag=1;
				goto out;
			}
		}
		else if(sqlrow(r->db,r->sql,NULL)==-1)
		{
			sqlfin(r->db,r->sql);
			goto out;
		}
	}
	if(!usepool)sqlfin(r->db,r->sql);

	limit=e->integer[1]?e->integer[1]:20;

	if(buildquery(NULL,bfr,sizeof(bfr),bldqry[QRY_PAGERESINS].pre,NULL,
		limit+1,r->start)!=1)goto out;
	if(usepool)
	{
		if(sqlparams(pool[h].db,pool[h].sql[QRY_PAGERESSEL],NULL))
		{
			pool[h].errflag=1;
			goto out;
		}
		if(tmptables[TBL_PAGES].truncate)
		{
			if(sqlrun(pool[h].db,tmptables[TBL_PAGES].truncate))
				goto out;
		}
		else if(sqlparams(pool[h].db,pool[h].sql[QRY_CLPAGES],NULL))
		{
			pool[h].errflag=1;
			goto out;
		}
		else if(sqlrow(pool[h].db,pool[h].sql[QRY_CLPAGES],NULL)==-1)
		{
			pool[h].errflag=1;
			goto out;
		}
		if(sqlrun(pool[h].db,bfr))
		{
			pool[h].errflag=1;
			goto out;
		}
	}
	else
	{
		if(sqlrun(r->db,tmptables[TBL_PAGES].create))goto out;
		nn++;
		if(sqlrun(r->db,bfr))goto out;
		if(sqlprep(r->db,&r->sql,prpqry[QRY_PAGERESSEL].sql,NULL,NULL))
			goto out;
	}

	while(1)
	{
		if(usepool)
		{
			if((rv=sqlrow(pool[h].db,pool[h].sql[QRY_PAGERESSEL],
				prpqry[QRY_PAGERESSEL].res,&res1,&res2,&res11,
				&res3,&res4,&res5,&res6,&res7,&res8,&res9,
				&res10)))
			{
				if(rv==-1)pool[h].errflag=1;
				break;
			}
		}
		else if(sqlrow(r->db,r->sql,prpqry[QRY_PAGERESSEL].res,&res1,
			&res2,&res11,&res3,&res4,&res5,&res6,&res7,&res8,&res9,
			&res10))break;
		if(!v++)
		{
			r->outputdone=OFLAG_OUT|OFLAG_DEP;
			if(e->string[0])sendstr(r,e->string[0]);
			if(e->string[5])for(p=e->string[5];*p;p++)
			{
				if(*p!='#')senddata(r,p,1);
				else sendint(r,r->start+1,NULL);
			}
		}
		else if(v>limit)break;

		if(e->string[32])for(q=e->string[32];*q;q++)
		{
			if(*q!='#')senddata(r,q,1);
			else sendint(r,r->start+v,NULL);
		}
		if(!(v&1))
		{
			if(e->string[36])for(q=e->string[36];*q;q++)
			{
				if(*q!='#')senddata(r,q,1);
				else sendint(r,r->start+v,NULL);
			}
		}
		else if(e->string[34])for(q=e->string[34];*q;q++)
		{
			if(*q!='#')senddata(r,q,1);
			else sendint(r,r->start+v,NULL);
		}

		if(e->string[7])sendstr(r,e->string[7]);
		if(e->integer[0]&OPT_TYPELINK)switch(res2)
		{
		case PDF:
			sendstr(r,e->string[11]?e->string[11]:"[pdf] ");
			break;
		case RSS:
			sendstr(r,e->string[12]?e->string[12]:"[rss] ");
			break;
		case ATOM:
			sendstr(r,e->string[13]?e->string[13]:"[atom] ");
			break;
		}
		if(res11&&e->string[30])sendstr(r,e->string[30]);
		if(e->string[4])
		{
			sendstr(r,"<a class=\"");
			sendstr(r,e->string[4]);
			sendstr(r,"\" href=\"");
		}
		else sendstr(r,"<a href=\"");
		sendstr(r,res3);
		sendstr(r,"\">");
		if(e->integer[0]&OPT_MARK&&(e->string[9]||e->string[10]))
			markit(r,res4,total,word,e->string[9],e->string[10],0);
		else sendtxt(r,res4,0);
		sendstr(r,"</a>");
		if(res11&&e->string[31])sendstr(r,e->string[31]);
		if(e->integer[0]&OPT_LINKTYPE)switch(res2)
		{
		case PDF:
			sendstr(r,e->string[11]?e->string[11]:" [pdf]");
			break;
		case RSS:
			sendstr(r,e->string[12]?e->string[12]:" [rss]");
			break;
		case ATOM:
			sendstr(r,e->string[13]?e->string[13]:" [atom]");
			break;
		}
		if(e->string[8])sendstr(r,e->string[8]);

		if(*res5)sendstr(r,e->string[2]?e->string[2]:" - ");
		else if(res6)if(*res6)if(e->integer[0]&OPT_MEDIADATE)
			sendstr(r,e->string[2]?e->string[2]:" - ");

		if(e->integer[0]&OPT_MARK&&(e->string[9]||e->string[10]))
			markit(r,res5,total,word,e->string[9],e->string[10],0);
		else sendtxt(r,res5,0);

		if(res6)if(*res6)if(e->integer[0]&OPT_MEDIADATE)
		{
			if(e->string[14])sendstr(r,e->string[14]);
			sendstr(r,res6);
			if(e->string[15])sendstr(r,e->string[15]);
		}

		if(e->integer[0]&OPT_TARGET)
		{
			if(e->string[16])sendstr(r,e->string[16]);
			sendtxt(r,res3,0);
			if(e->string[17])sendstr(r,e->string[17]);
		}

		if(e->integer[0]&OPT_TOPIC)
		{
			if(e->string[19])sendstr(r,e->string[19]);
			if(e->integer[0]&OPT_LINK)
			{
				if(e->string[4])
				{
					sendstr(r,"<a class=\"");
					sendstr(r,e->string[4]);
					sendstr(r,"\" href=\"");
				}
				else sendstr(r,"<a href=\"");
				sendhref(r,res7);
				sendstr(r,"\">");
			}
			if((p=strdup(res7)))
			{
				for(m=0,q=s=p;*q;q++)if(*q=='/')
				{
					*q++=0;
					if(m>1)sendstr(r,e->string[18]?
						e->string[18]:": ");
					if(m)sendtxt(r,s,1);
					m++;
					s=q;
				}
				if(m>1)sendstr(r,e->string[18]?
					e->string[18]:": ");
				if(m)sendtxt(r,s,1);
				free(p);
			}
			if(e->integer[0]&OPT_LINK)sendstr(r,"</a>");
			if(e->string[20])sendstr(r,e->string[20]);
		}

		if(e->integer[0]&(OPT_TOTALTOPICS|OPT_TOTALPAGES))
		{
			if(e->string[21])sendstr(r,e->string[21]);
			if(e->integer[0]&OPT_LINK)
			{
				if(e->string[4])
				{
					sendstr(r,"<a class=\"");
					sendstr(r,e->string[4]);
					sendstr(r,"\" href=\"");
				}
				else sendstr(r,"<a href=\"");
				sendstr(r,"/search.cgi?search=");
				sendelement(r,r->data,1);
				sendstr(r,"&amp;all=no&amp;catid=");
				sendint(r,res10,NULL);
				sendstr(r,"\">");
			}
			if(e->integer[0]&OPT_TOTALTOPICS)
				sendint(r,res8,e->string[29]);
			if(e->string[23])sendstr(r,e->string[23]);
			if(e->integer[0]&OPT_TOTALPAGES)
				sendint(r,res9,e->string[29]);
			if(e->integer[0]&OPT_LINK)sendstr(r,"</a>");
			if(e->string[22])sendstr(r,e->string[22]);
		}

		if(!(v&1))
		{
			if(e->string[37])for(q=e->string[37];*q;q++)
			{
				if(*q!='#')senddata(r,q,1);
				else sendint(r,r->start+v,NULL);
			}
		}
		else if(e->string[35])for(q=e->string[35];*q;q++)
		{
			if(*q!='#')senddata(r,q,1);
			else sendint(r,r->start+v,NULL);
		}
		if(e->string[33])for(q=e->string[33];*q;q++)
		{
			if(*q!='#')senddata(r,q,1);
			else sendint(r,r->start+v,NULL);
		}
	}
	if(!usepool)sqlfin(r->db,r->sql);
	else
	{
		relconn(h);
		h=-1;
	}

	if(v&&e->string[6])sendstr(r,e->string[6]);

	if(e->integer[0]&OPT_MORE)
		if(v>limit||r->start>=limit)
	{
		if(e->string[27])sendstr(r,e->string[27]);
		if(r->start>=limit)
		{
			if(e->string[4])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[4]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendstr(r,"/search.cgi?search=");
			sendelement(r,r->data,1);
			sendstr(r,"&amp;all=");
			sendstr(r,(r->searchmode&QTYPE_ALL)?"yes":"no");
			if(r->start>limit)
			{
				sendstr(r,"&amp;start=");
				sendint(r,r->start-limit,NULL);
			}
			sendstr(r,"&amp;catid=");
			sendint(r,r->catid,NULL);
			if(r->searchmode&QTYPE_SITESONLY)sendstr(r,"&amp;t=s");
			if(r->ktm&KIDS)sendstr(r,"&amp;kids=1");
			if(r->ktm&TEEN)sendstr(r,"&amp;teens=1");
			if(r->ktm&MTEEN)sendstr(r,"&amp;mteens=1");
			sendstr(r,"\">");
			sendstr(r,e->string[24]?e->string[24]:
				"&lt;&lt;&lt;&lt;&lt;");
			sendstr(r,"</a>");
		}
		else if(!(e->integer[0]&OPT_LINKONLY))
			sendstr(r,e->string[24]?e->string[24]:
				"&lt;&lt;&lt;&lt;&lt;");
		if((r->start>=limit&&v>limit)||!(e->integer[0]&OPT_LINKONLY))
			sendstr(r,e->string[26]?e->string[26]:
				" &nbsp; &nbsp; ");
		if(v>limit)
		{
			if(e->string[4])
			{
				sendstr(r,"<a class=\"");
				sendstr(r,e->string[4]);
				sendstr(r,"\" href=\"");
			}
			else sendstr(r,"<a href=\"");
			sendstr(r,"/search.cgi?search=");
			sendelement(r,r->data,1);
			sendstr(r,"&amp;all=");
			sendstr(r,(r->searchmode&QTYPE_ALL)?"yes":"no");
			sendstr(r,"&amp;start=");
			sendint(r,r->start+limit,NULL);
			sendstr(r,"&amp;catid=");
			sendint(r,r->catid,NULL);
			if(r->searchmode&QTYPE_SITESONLY)sendstr(r,"&amp;t=s");
			if(r->ktm&KIDS)sendstr(r,"&amp;kids=1");
			if(r->ktm&TEEN)sendstr(r,"&amp;teens=1");
			if(r->ktm&MTEEN)sendstr(r,"&amp;mteens=1");
			sendstr(r,"\">");
			sendstr(r,e->string[25]?e->string[25]:
				"&gt;&gt;&gt;&gt;&gt;");
			sendstr(r,"</a>");
		}
		else if(!(e->integer[0]&OPT_LINKONLY))
			sendstr(r,e->string[25]?e->string[25]:
				"&gt;&gt;&gt;&gt;&gt;");
		if(e->string[28])sendstr(r,e->string[28]);
	}

out:	if(!usepool)
	{
		if(nn>1)sqlrun(r->db,tmptables[TBL_PAGES].drop);
		if(nn>0)sqlrun(r->db,tmptables[TBL_RESULTS].drop);
	}
	else if(h!=-1)relconn(h);

	if(v&&e->string[1])sendstr(r,e->string[1]);
	else if(!v&&e->string[3])sendstr(r,e->string[3]);

	while(total--)free(word[total]);
}

static void sendheaders(REQUEST *r)
{
	time_t stamp;
	char date[64];
	struct tm tm;

	stamp=time(NULL);
	gmtime_r(&stamp,&tm);
	strftime(date,sizeof(date),"Date: %a, %d %b %Y %H:%M:%S GMT\n",&tm);

	switch(r->pagetype)
	{
	case RTYPE_ERR302:
		sendstr(r,"HTTP/1.1 302 Found\nLocation: ");
		if(r->host)
		{
			sendstr(r,"http://");
			sendstr(r,r->host);
		}
		sendhref(r,r->data);
		sendchr(r,'\n');
		break;
	case RTYPE_ERR400:
		sendstr(r,"HTTP/1.1 400 Bad Request\n");
		break;
	case RTYPE_ERR404:
		sendstr(r,"HTTP/1.1 404 Not Found\n");
		break;
	case RTYPE_ERR500:
		sendstr(r,"HTTP/1.1 500 Internal Server Error\n");
		break;
	case RTYPE_ERR501:
		sendstr(r,"HTTP/1.1 501 Not Implemented\n");
		break;
	default:
		sendstr(r,"HTTP/1.1 200 OK\n");
		break;
	}

	sendstr(r,"Server: minimoz\nConnection: close\n");
	sendstr(r,date);

	switch(r->pagetype)
	{
	case RTYPE_GIF:
		sendstr(r,"Content-Type: image/gif\n");
		r->iomode=0;
		break;
	case RTYPE_JPG:
		sendstr(r,"Content-Type: image/jpeg\n");
		r->iomode=0;
		break;
	case RTYPE_PNG:
		sendstr(r,"Content-Type: image/png\n");
		r->iomode=0;
		break;
	case RTYPE_CSS:
		sendstr(r,"Content-Type: text/css; charset=utf-8\n");
		break;
	case RTYPE_ROBOTS:
		sendstr(r,"Content-Type: text/plain; charset=utf-8\n");
		break;
	default:
		sendstr(r,"Content-Type: text/html; charset=utf-8\n");
		break;
	}

	switch(r->pagetype)
	{
	case RTYPE_HOME:
	case RTYPE_INDEX:
	case RTYPE_CATALOG:
	case RTYPE_INFO:
	case RTYPE_ADVANCED:
	case RTYPE_QUERY:
	case RTYPE_ERR302:
	case RTYPE_ERR500:
		sendstr(r,"Cache-Control: no-cache\nPragma: no-cache\n");
		break;
	default:
		stamp+=3600;
		gmtime_r(&stamp,&tm);
		strftime(date,sizeof(date),"Expires: %a, %d %b %Y %H:%M:%S "
			"GMT\n",&tm);
		sendstr(r,date);
		stamp-=3600;
		break;
	}

	if(r->iomode)
	{
		memset(&r->z,0,sizeof(r->z));
		r->z.next_in=r->buf;
		if(r->iomode&IOMODE_DEFLATE)
		{
			r->z.next_out=r->zbuf;
			r->z.avail_out=sizeof(r->zbuf);
		}
		else
		{
			r->zbuf[0]=0x1f;
			r->zbuf[1]=0x8b;
			r->zbuf[2]=0x08;
			r->zbuf[3]=0x00;
			r->zbuf[4]=(char)(stamp);
			r->zbuf[5]=(char)(stamp>>8);
			r->zbuf[6]=(char)(stamp>>16);
			r->zbuf[7]=(char)(stamp>>24);
			r->zbuf[8]=0x00;
			r->zbuf[9]=0x03;
			r->crc=crc32(0,NULL,0);
			r->len=0;
			r->z.next_out=r->zbuf+10;
			r->z.avail_out=sizeof(r->zbuf)-10;
		}
	}

	if(r->iomode&&deflateInit2(&r->z,Z_DEFAULT_COMPRESSION,Z_DEFLATED,
		(r->iomode&IOMODE_DEFLATE)?MAX_WBITS:-MAX_WBITS,8,
		Z_DEFAULT_STRATEGY)==Z_OK)
	{
		if(r->iomode&IOMODE_DEFLATE)
		{
			r->gzip=-IOMODE_DEFLATE;
			sendstr(r,"Content-Encoding: deflate\n\n");
		}
		else
		{
			r->gzip=-IOMODE_GZIP;
			sendstr(r,"Content-Encoding: gzip\n\n");
		}
		sendflush(r,0);
	}
	else sendchr(r,'\n');
}

static int loadnls(REQUEST *r)
{
	int m;
	int h=-1;
	char *res[NLSTOTAL];

	if(usepool)
	{
		if((h=getconn())==-1)return -1;
		if(sqlparams(pool[h].db,pool[h].sql[QRY_NLSSUPPORT],
			prpqry[QRY_NLSSUPPORT].fmt,r->catid))
		{
			pool[h].errflag=1;
			relconn(h);
			return -1;
		}
	}
	else if(sqlprep(r->db,&r->sql,prpqry[QRY_NLSSUPPORT].sql,
		prpqry[QRY_NLSSUPPORT].fmt,prpqry[QRY_NLSSUPPORT].fmt,r->catid))
		return -1;

	if(usepool)
	{
		if(sqlrow(pool[h].db,pool[h].sql[QRY_NLSSUPPORT],
			prpqry[QRY_NLSSUPPORT].res,&res[0],&res[1],&res[2],
			&res[3],&res[4],&res[5],&res[6],&res[7],&res[8],
			&res[9],&res[10],&res[11]))
		{
			pool[h].errflag=1;
			relconn(h);
			return -1;
		}
	}
	else if(sqlrow(r->db,r->sql,prpqry[QRY_NLSSUPPORT].res,&res[0],&res[1],
		&res[2],&res[3],&res[4],&res[5],&res[6],&res[7],&res[8],
		&res[9],&res[10],&res[11]))
	{
		sqlfin(r->db,r->sql);
		return -1;
	}
	for(m=0;m<NLSTOTAL;m++)if(!(r->nls[m]=strdup(res[m])))
	{
		if(usepool)relconn(h);
		else sqlfin(r->db,r->sql);
		return -1;
	}
	if(usepool)relconn(h);
	else sqlfin(r->db,r->sql);
	return 0;
}

static int decode(char *p,int mode)
{
	char *q;

	for(q=p;*p;p++,q++)switch(*p)
	{
	case '%':
		if(p[1]>='0'&&p[1]<='9')*q=p[1]-'0';
		else if(p[1]>='A'&&p[1]<='F')*q=p[1]-'A'+10;
		else if(p[1]>='a'&&p[1]<='f')*q=p[1]-'a'+10;
		else
		{
			*q=*p;
			break;
		}
		*q<<=4;
		if(p[2]>='0'&&p[2]<='9')*q|=p[2]-'0';
		else if(p[2]>='A'&&p[2]<='F')*q|=p[2]-'A'+10;
		else if(p[2]>='a'&&p[2]<='f')*q|=p[2]-'a'+10;
		else
		{
			*q=*p;
			break;
		}
		if(!*q||*q=='/')return -1;
		p+=2;
		break;
	case '+':
		if(mode)
		{
			*q=' ';
			break;
		}
	default:
		*q=*p;
		break;
	}
	*q=0;
	return 0;
}

static int readline(REQUEST *r,char *buf,int buflen)
{
	if(!buflen)return 0;
	if(r->fd==-1)return 0;

	while(--buflen)
	{
		if(r->ridx==r->rlen)
		{
			if(times(NULL)-r->starttime>=requestlimit)
			{
				shutdown(r->fd,SHUT_RDWR);
				close(r->fd);
				r->fd=-1;
				if(r->gzip)deflateEnd(&r->z);
				return 0;
			}
			poll(&r->r,1,1000);
			if(r->r.revents&POLLIN)
			{
				r->ridx=0;
				r->rlen=read(r->fd,r->buf,sizeof(r->buf));
				if(r->rlen<=0)
				{
					shutdown(r->fd,SHUT_RDWR);
					close(r->fd);
					r->fd=-1;
					if(r->gzip)deflateEnd(&r->z);
					return 0;
				}
			}
			else if(r->r.revents&(POLLERR|POLLHUP|POLLNVAL))
			{
				shutdown(r->fd,SHUT_RDWR);
				close(r->fd);
				r->fd=-1;
				if(r->gzip)deflateEnd(&r->z);
				return 0;
			}
			else continue;
		}

		*buf=r->buf[r->ridx++];
		if(*buf++=='\n')break;
	}

	*buf=0;
	return 1;
}

static void get_request(REQUEST *r)
{
	int l;
	int mode;
	int searchall;
	int allcat;
	int type;
	char *p;
	char *q;
	char *token;
	char *req;
	char *query;
	char *search;
	char get[BUFSIZE];
	char opt[BUFSIZE];

	mode=1;
	searchall=0;
	allcat=0;
	req=NULL;
	query=NULL;
	search=NULL;

	if(!readline(r,get,sizeof(get)))goto purge;
	l=strlen(get);
	if(!l)goto purge;
	if(get[l-1]!='\n')goto purge;
	while(l)if(get[l-1]=='\n'||get[l-1]=='\r')l--;
	else break;
	if(!l)goto purge;
	get[l]=0;
	p=strtok_r(get," \t",&token);
	if(!p)goto purge;
	if(strcasecmp(p,"GET"))
	{
		r->pagetype=RTYPE_ERR501;
		goto purge;
	}
	p=strtok_r(NULL," \t",&token);

	if(!p)goto purge;
	if(!strncasecmp(p,"http://",7))for(p+=7;*p&&*p!='?'&&*p!='/';p++);
	if(*p!='/')goto purge;
	for(req=p;*p;p++)if(*p=='?')
	{
		*p=0;
		query=p+1;
		break;
	}
	if(decode(req,0))goto purge;

	p=strtok_r(NULL," \t",&token);
	if(p)
	{
		if(!strcasecmp(p,"HTTP/0.9")||!strcasecmp(p,"HTTP/1.0"))
			mode=-1;
	}
	else mode=0;

	for(q=p=req;*p;p++,q++)if(*p!='/')*q=*p;
	else
	{
		*q='/';
		while(p[1]=='/')p++;
		if(p[1]=='.')
		{
			if(!p[2]||p[2]=='/')goto purge;
			if(p[2]=='.')if(!p[3]||p[3]=='/')goto purge;
		}
	}
	*q=0;

	while(mode)
	{
		if(!readline(r,opt,sizeof(opt)))goto purge;
		l=strlen(opt);
		if(!l)goto purge;
		if(opt[l-1]!='\n')goto purge;
		while(l)if(opt[l-1]=='\n'||opt[l-1]=='\r')l--;
		else break;
		if(!l)break;
		opt[l]=0;
		if(!strncasecmp(opt,"Accept-Encoding:",16))
		{
			p=opt+16;
			while((p=strtok_r(p,", \t",&token)))
			{
				for(q=p;*q&&*q!=';';q++);
				if(*q)
				{
				    *q=0;
				    if(!strcasecmp(q+1,"q=0"))
				    {
					if(!strcasecmp(p,"*"))goto purge;
					if(!strcasecmp(p,"identity"))goto purge;
				    }
				}
				if(!strcasecmp(p,"gzip"))
				{
					if(gzenable)
						r->iomode|=IOMODE_GZIP;
				}
				else if(!strcasecmp(p,"deflate"))
				{
					if(gzenable)r->iomode|=IOMODE_DEFLATE;
				}
				else if(!strcasecmp(p,"*"))
				{
					if(gzenable)r->iomode|=
						IOMODE_GZIP|IOMODE_DEFLATE;
				}
				p=NULL;
			}
			if(r->iomode&IOMODE_GZIP)r->iomode=IOMODE_GZIP;
			else if(r->iomode==IOMODE_DEFLATE)
				r->iomode|=IOMODE_GZIP;
		}
		else if(!strncasecmp(opt,"Accept-Language:",16))
		{
			p=opt+16;
			for(l=0;l<MAXLANG;l++)if(!r->lang[l])break;
			if(l!=MAXLANG)while((p=strtok_r(p,", \t",&token)))
			{
				for(q=p;*q&&*q!=';';q++)if(*q=='-')*q='_';
				if(*q)
				{
					*q=0;
					if(!strcasecmp(q+1,"q=0"))
					{
						p=NULL;
						continue;
					}
				}
				if(!(r->lang[l++]=strdup(p)))
				{
					r->pagetype=RTYPE_ERR500;
					goto purge;
				}
				if(l==MAXLANG)break;
				p=NULL;
			}
		}
		else if(!strncasecmp(opt,"Host:",5))
		{
			p=opt+5;
			if(!r->host)if((p=strtok_r(p," \t",&token)))
				if(!(r->host=strdup(p)))
			{
				r->pagetype=RTYPE_ERR500;
				goto purge;
			}
		}
	}
	if(mode>0&&!r->host)return;

	if(!strcmp(req,"/"))type=RTYPE_HOME;
	else if(!strcmp(req,"/robots.txt"))type=RTYPE_ROBOTS;
	else if(!strcmp(req,"/search.cgi"))type=RTYPE_QUERY;
	else if(!strcmp(req,"/advanced.html"))type=RTYPE_ADVANCED;
	else if(!strcmp(req,"/minimoz.css"))type=RTYPE_CSS;
	else if(!strcmp(req,"/index.html"))
	{
		req[1]=0;
		type=RTYPE_INDEX;
	}
	else if(!strncmp(req,"/img/",5))
	{
		req+=5;
		if(strchr(req,'/'))
		{
			r->pagetype=RTYPE_ERR404;
			return;
		}
		if(!(p=strrchr(req,'.')))
		{
			r->pagetype=RTYPE_ERR404;
			return;
		}
		*p++=0;
		if(!strcmp(p,"gif"))type=RTYPE_GIF;
		else if(!strcmp(p,"jpg"))type=RTYPE_JPG;
		else if(!strcmp(p,"png"))type=RTYPE_PNG;
		else
		{
			r->pagetype=RTYPE_ERR404;
			return;
		}
	}
	else
	{
		type=RTYPE_CATALOG;
		p=strrchr(req,'/');
		{
			if(!strcmp(p+1,"desc.html"))
			{
				*p=0;
				type=RTYPE_INFO;
			}
		}
	}

	if((type==RTYPE_QUERY&&!query)||(type!=RTYPE_QUERY&&query))return;

	if(type==RTYPE_CATALOG||type==RTYPE_HOME||type==RTYPE_INDEX)
	{
		p=strrchr(req,'/');
		if(p[1]==0)*p=0;
	}

	if(type==RTYPE_CATALOG||type==RTYPE_HOME||type==RTYPE_INDEX||
		type==RTYPE_INFO)
	{
		if(!*req)strcpy(req,"Top");
		else
		{
			req-=3;
			memcpy(req,"Top",3);
		}
	}

	if(type==RTYPE_ADVANCED)r->catid=TOPCATID;

	if(query)
	{
	    r->searchmode=QTYPE_CATSITES;
	    for(p=query;(p=strtok_r(p,"&",&token));p=NULL)
	    {
		if(!strncasecmp(p,"search=",7))search=p+7;
		else if(!strncasecmp(p,"all=",4))
		{
			if(!strcasecmp(p+4,"yes"))searchall=1;
		}
		else if(!strncasecmp(p,"start=",6))r->start=atoi(p+6);
		else if(!strncasecmp(p,"allcat=",7))allcat=atoi(p+7);
		else if(!strncasecmp(p,"catid=",6))r->catid=atoi(p+6);
		else if(!strncasecmp(p,"t=",2))
		{
			switch(p[2])
			{
			case 'c':
			case 'C':
				allcat=1;
				break;
			case 's':
			case 'S':
				r->searchmode=QTYPE_SITESONLY;
				break;
			case 'b':
			case 'B':
				r->searchmode=QTYPE_CATSITES;
				break;
			default:return;
			}
		}
		else if(!strncasecmp(p,"kids=",5))
		{
			if(p[5]=='1'&&!p[6])r->ktm|=KIDS;
		}
		else if(!strncasecmp(p,"teens=",6))
		{
			if(p[6]=='1'&&!p[7])r->ktm|=TEEN;
		}
		else if(!strncasecmp(p,"mteens=",7))
		{
			if(p[7]=='1'&&!p[8])r->ktm|=MTEEN;
		}
		else return;
	    }
	    if(allcat)r->searchmode=QTYPE_CATONLY|QTYPE_FULLCAT;
	    if(searchall||r->catid==TOPCATID)r->searchmode|=QTYPE_ALL;
	    if(!search||r->start<0||r->catid<=0)return;
	    if(decode(search,1))return;
	}

	r->pagetype=RTYPE_ERR500;
	if(type==RTYPE_CATALOG||type==RTYPE_HOME||type==RTYPE_INDEX||
		type==RTYPE_INFO||type==RTYPE_GIF||type==RTYPE_JPG||
		type==RTYPE_PNG)
	{
		if(!(r->data=strdup(req)))return;
	}
	else if(type==RTYPE_QUERY)
	{
		if(!(r->data=strdup(search)))return;
	}

	if(type==RTYPE_HOME&&!r->tmpl->page[RTYPE_HOME])type=RTYPE_CATALOG;
	if(type==RTYPE_INDEX&&!r->tmpl->page[RTYPE_INDEX])type=RTYPE_CATALOG;

	if(type!=RTYPE_GIF&&type!=RTYPE_JPG&&type!=RTYPE_PNG)
		if(!r->tmpl->page[type])type=RTYPE_ERR404;

	r->pagetype=type;
	return;

purge:	while(readline(r,opt,sizeof(opt)))
	{
		l=strlen(opt);
		if(!l)break;
		if(opt[l-1]!='\n')continue;
		while(l)if(opt[l-1]=='\n'||opt[l-1]=='\r')l--;
		else break;
		if(!l)break;
	}
}

static int catid_from_request(REQUEST *r)
{
	int h=-1;
	char *p;

	if(usepool)
	{
		if((h=getconn())==-1)goto out;
		if(sqlparams(pool[h].db,pool[h].sql[QRY_TOPICCATID],
			prpqry[QRY_TOPICCATID].fmt,r->data))
		{
			pool[h].errflag=1;
			relconn(h);
			goto out;
		}
	}
	else if(sqlprep(r->db,&r->sql,prpqry[QRY_TOPICCATID].sql,
		prpqry[QRY_TOPICCATID].fmt,prpqry[QRY_TOPICCATID].fmt,r->data))
		goto out;

	if(usepool)switch(sqlrow(pool[h].db,pool[h].sql[QRY_TOPICCATID],
		prpqry[QRY_TOPICCATID].res,&r->catid))
	{
	case 0:	relconn(h);
		free(r->data);
		r->data=NULL;
		return 0;

	case -1:pool[h].errflag=1;
		relconn(h);
		goto out;
	}
	else if(!sqlrow(r->db,r->sql,prpqry[QRY_TOPICCATID].res,&r->catid))
	{
		sqlfin(r->db,r->sql);
		free(r->data);
		r->data=NULL;
		return 0;
	}

	if(usepool)
	{
		if(sqlparams(pool[h].db,pool[h].sql[QRY_REDIRCATID],
			prpqry[QRY_REDIRCATID].fmt,r->data))
		{
			pool[h].errflag=1;
			relconn(h);
			goto out;
		}
	}
	else if(sqlprep(r->db,&r->sql,prpqry[QRY_REDIRCATID].sql,
		prpqry[QRY_REDIRCATID].fmt,prpqry[QRY_REDIRCATID].fmt,r->data))
		goto out;

	if(usepool)switch(sqlrow(pool[h].db,pool[h].sql[QRY_REDIRCATID],
		prpqry[QRY_REDIRCATID].res,&r->catid))
	{
	case 0:	if(sqlparams(pool[h].db,pool[h].sql[QRY_TOPICID],
			prpqry[QRY_TOPICID].fmt,r->catid))
		{
			pool[h].errflag=1;
			relconn(h);
			goto out;
		}
		switch(sqlrow(pool[h].db,pool[h].sql[QRY_TOPICID],
			prpqry[QRY_TOPICID].res,&p))
		{
		case -1:pool[h].errflag=1;
			relconn(h);
			goto out;

		case 1:	p="Top";
			break;
		}
		break;

	case -1:pool[h].errflag=1;
		relconn(h);
		goto out;

	case 1:	p="Top";
		break;
	}
	else if(!sqlrow(r->db,r->sql,prpqry[QRY_REDIRCATID].res,&r->catid))
	{
		sqlfin(r->db,r->sql);
		if(sqlprep(r->db,&r->sql,prpqry[QRY_TOPICID].sql,
			prpqry[QRY_TOPICID].fmt,prpqry[QRY_TOPICID].fmt,
			r->catid))goto out;
		if(sqlrow(r->db,r->sql,prpqry[QRY_TOPICID].res,&p))p="Top";
	}
	else p="Top";

	if(!(p=strdup(p)))
	{
		if(usepool)relconn(h);
		else sqlfin(r->db,r->sql);
		goto out;
	}
	if(usepool)relconn(h);
	else sqlfin(r->db,r->sql);

	free(r->data);
	r->data=p;
	r->pagetype=RTYPE_ERR302;
	return -1;

out:	r->pagetype=RTYPE_ERR500;
	return -1;
}

static void free_request(REQUEST *r)
{
	int m;

	if(r->data)free(r->data);
	if(r->host)free(r->host);
	for(m=0;m<NLSTOTAL;m++)if(r->nls[m])free(r->nls[m]);
	else break;
	for(m=0;m<MAXLANG;m++)if(r->lang[m])free(r->lang[m]);
	else break;
	pthread_mutex_lock(&mtx);
	m=--(r->tmpl->usage);
	pthread_mutex_unlock(&mtx);
	if(!m)freetemplate(r->tmpl);
	free(r);
}

static void *do_request(void *request)
{
	int i;
	int m;
	ACTION *a;
	CHAIN *c;
	ELEMENT *e;
	ELEMENT *x;
	REQUEST *r;

	r=(REQUEST *)(request);

	get_request(r);

	switch(r->pagetype)
	{
	case RTYPE_GIF:
		for(c=r->tmpl->anchor[2];c;c=c->next)
			if(!strcmp(c->name,r->data))break;
		if(!c)r->pagetype=RTYPE_ERR404;
		break;
	case RTYPE_JPG:
		for(c=r->tmpl->anchor[3];c;c=c->next)
			if(!strcmp(c->name,r->data))break;
		if(!c)r->pagetype=RTYPE_ERR404;
		break;
	case RTYPE_PNG:
		for(c=r->tmpl->anchor[4];c;c=c->next)
			if(!strcmp(c->name,r->data))break;
		if(!c)r->pagetype=RTYPE_ERR404;
		break;
	case RTYPE_HOME:
	case RTYPE_INDEX:
	case RTYPE_CATALOG:
	case RTYPE_INFO:
	case RTYPE_ADVANCED:
	case RTYPE_QUERY:
		if(sqlthreadinit())r->pagetype=RTYPE_ERR500;
		else if(!usepool)if(sqlopen(database,user,password,&r->db,
			SQLFLAGS_STDMEM))
		{
			sqlthreadexit();
			r->pagetype=RTYPE_ERR500;
		}
	default:c=NULL;
		break;
	}

	switch(r->pagetype)
	{
	case RTYPE_HOME:
	case RTYPE_INDEX:
	case RTYPE_CATALOG:
	case RTYPE_INFO:
		if(catid_from_request(r))
		{
			if(!usepool)sqlclose(r->db,0);
			sqlthreadexit();
		}
		break;
	}

	switch(r->pagetype)
	{
	case RTYPE_HOME:
	case RTYPE_INDEX:
	case RTYPE_CATALOG:
	case RTYPE_INFO:
	case RTYPE_ADVANCED:
	case RTYPE_QUERY:
		if(loadnls(r))
		{
			if(!usepool)sqlclose(r->db,0);
			sqlthreadexit();
			r->pagetype=RTYPE_ERR500;
		}
		break;
	}

	sendheaders(r);

	switch(r->pagetype)
	{
	case RTYPE_GIF:
	case RTYPE_JPG:
	case RTYPE_PNG:
		x=NULL;
		m=MAXLANG;
		for(e=c->data;e->next;e=e->next)if(!e->lang[0]&&!x)
		{
			x=e;
			if(!r->lang[0])break;
		}
		else if(e->lang[0])for(i=0;i<MAXLANG&&r->lang[i];i++)
			if(!strcasecmp(e->lang,r->lang[i]))
		{
			if(i<m)
			{
				x=e;
				m=i;
			}
			break;
		}
		if(x)e=x;
		senddata(r,e->string[0],e->integer[2]);
		break;
	default:for(a=r->tmpl->page[r->pagetype];a;a=a->next)
		{
			if(a->data)
			{
				x=NULL;
				m=MAXLANG;
				for(e=a->data;e->next;e=e->next)
					if(!e->lang[0]&&!x)
				{
					x=e;
					if(!r->lang[0])break;
				}
				else if(e->lang[0])
				    for(i=0;i<MAXLANG&&r->lang[i];i++)
					if(!strcasecmp(e->lang,r->lang[i]))
				{
					if(i<m)
					{
						x=e;
						m=i;
					}
					break;
				}
				if(x)e=x;
			}
			else e=NULL;
			a->processor(r,e);
		}
		break;
	}

	switch(r->pagetype)
	{
	case RTYPE_HOME:
	case RTYPE_INDEX:
	case RTYPE_CATALOG:
	case RTYPE_INFO:
	case RTYPE_ADVANCED:
	case RTYPE_QUERY:
		if(!usepool)sqlclose(r->db,0);
		sqlthreadexit();
		break;
	}

	sendflush(r,1);
	free_request(r);
	sem_post(&children);
	pthread_exit(NULL);
}

static void usr1handler(int unused)
{
	if(database2)
	{
		newdatabase=database1;
		newdb++;
		sem_post(&children);
	}
}

static void usr2handler(int unused)
{
	if(database2)
	{
		newdatabase=database2;
		newdb++;
		sem_post(&children);
	}
}

static void inthandler(int unused)
{
	term++;
}

static void termhandler(int unused)
{
	if(!term)term=1;
}

static void reloadhandler(int unused)
{
	reload++;
	sem_post(&children);
}

static void usage(void)
{
	errmsg("Usage: minimoz [-zn] [-d db] [-t template] [-R timeout]\n"
	       "               [-T timeout] [-L clientlimit] [-P pidfile]\n"
	       "               [-i listenip] [-l listenport] [-U runas]\n"
	       "               [-u database-user] [-p database-password]\n"
	       "               [-a alternative-db] [-C pool-limit]\n"
	       "               '-' as password means read from stdin\n"
	       "               -z  enable gzip/deflate content encoding\n"
	       "               -n  don't daemonize\n",0);
	exit(1);
}

int main(int argc,char *argv[])
{
	int l;
	int v;
	int c;
	int i;
	int dosleep;
	int conncount;
	int collect;
	unsigned long stamp;
	unsigned long curr;
	socklen_t sl;
	struct sockaddr_in a;
	pthread_t thr;
	pthread_attr_t attr;
	struct passwd *pw;
	in_addr_t addr=INADDR_ANY;
	int port=2345;
	int limit=50;
	int timeout=60;
	int request=10;
	int gzip=0;
	int nodaemon=0;
	int maxconn=-1;
	char *pidfile=NULL;
	char *runas=NULL;
	char *template="minimoz.tmpl";
	REQUEST *r;
	TEMPLATE *t;
	TEMPLATE *tmpl;
	struct pollfd pfd;
	sigset_t ss;
	char n[128];

	database1=SQLDFLT_DB;
	database2=NULL;
	user=SQLDFLT_USER;
	password=SQLDFLT_PASS;
	usepool=0;
	pool=NULL;

	while((v=getopt(argc,argv,"znD:R:L:P:U:t:i:l:d:u:p:a:C:"))!=-1)switch(v)
	{
	case 'z':
		gzip=1;
		break;
	case 'n':
		nodaemon=1;
		break;
	case 'D':
		timeout=atoi(optarg);
		break;
	case 'R':
		request=atoi(optarg);
		break;
	case 'L':
		limit=atoi(optarg);
		break;
	case 'P':
		pidfile=optarg;
		break;
	case 'U':
		runas=optarg;
		break;
	case 't':
		template=optarg;
		break;
	case 'i':
		addr=inet_addr(optarg);
		break;
	case 'l':
		port=atoi(optarg);
		break;
	case 'd':
		database1=optarg;
		break;
	case 'u':
		user=optarg;
		break;
	case 'p':
		password=optarg;
		break;
	case 'a':
		database2=optarg;
		break;
	case 'C':
		maxconn=atoi(optarg);
		break;
	default:usage();
	}

	if(optind<argc)usage();

	database=database1;
	gzenable=gzip;
	if(timeout<10||timeout>900)timeout=60;
	if(request<5||request>60)request=10;
	if(request>timeout)request=timeout;
	if(port<=0||port>65535)port=2345;
	if(limit>PTHREAD_THREADS_MAX)limit=PTHREAD_THREADS_MAX;
	if(limit<=0)limit=1;
	if(maxconn!=-1)
	{
		if(maxconn<1||maxconn>100)maxconn=10;
		usepool=maxconn;
	}

	if(password)if(!strcmp(password,"-"))
	{
		for(i=0;i<sizeof(n);i++)if(read(0,n+i,1)!=1)
		{
			errmsg("password read error\n",0);
			return 1;
		}
		else if(n[i]=='\n')
		{
			n[i]=0;
			if(!(password=strdup(n)))
			{
				memset(n,0,sizeof(n));
				errmsg("out of memory\n",0);
				return 1;
			}
			break;
		}
		memset(n,0,sizeof(n));
		if(i==sizeof(n))
		{
			errmsg("password too long\n",0);
			return 1;
		}
	}

	if((l=socket(PF_INET,SOCK_STREAM,0))==-1)
	{
		if(password)memset(password,0,strlen(password));
		errmsg("socket failure\n",0);
		return 1;
	}

	v=1;
	if(setsockopt(l,SOL_SOCKET,SO_REUSEADDR,&v,sizeof(v))==-1)
	{
		close(l);
		if(password)memset(password,0,strlen(password));
		errmsg("setsockopt failure\n",0);
		return 1;
	}

	memset(&a,0,sizeof(a));
	a.sin_family=AF_INET;
	a.sin_addr.s_addr=addr;
	a.sin_port=htons(port);

	if(bind(l,(struct sockaddr *)(&a),sizeof(a))==-1)
	{
		close(l);
		if(password)memset(password,0,strlen(password));
		errmsg("bind failure\n",0);
		return 1;
	}

	if(listen(l,limit+limit/10)==-1)
	{
		close(l);
		if(password)memset(password,0,strlen(password));
		errmsg("listen failure\n",0);
		return 1;
	}

	if(fcntl(l,F_SETFL,O_NONBLOCK))
	{
		close(l);
		if(password)memset(password,0,strlen(password));
		errmsg("fcntl failure\n",0);
		return 1;
	}

	if(runas)
	{
		if(!(pw=getpwnam(runas)))
		{
			close(l);
			if(password)memset(password,0,strlen(password));
			errmsg("no such user\n",0);
			return 1;
		}
		if(setresgid(pw->pw_gid,pw->pw_gid,pw->pw_gid))
		{
			close(l);
			if(password)memset(password,0,strlen(password));
			errmsg("group setup failure\n",0);
			return 1;
		}
		if(setresuid(pw->pw_uid,pw->pw_uid,pw->pw_uid))
		{
			close(l);
			if(password)memset(password,0,strlen(password));
			errmsg("user setup failure\n",0);
			return 1;
		}
	}

	if(!(tmpl=loadtemplate(template)))
	{
		close(l);
		if(password)memset(password,0,strlen(password));
		return 1;
	}

	if(sqlinit(database))
	{
		errmsg("sql initialization failure\n",0);
		close(l);
		if(password)memset(password,0,strlen(password));
		return 1;
	}

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_mutex_init(&mtx,NULL);
	sem_init(&children,0,limit);
	if(usepool)
	{
		pthread_mutex_init(&poolmtx,NULL);
		sem_init(&dbpool,0,maxconn);
		if(!(pool=malloc(maxconn*sizeof(DBPOOL))))
		{
			errmsg("out of memory\n",0);
			close(l);
			if(password)memset(password,0,strlen(password));
			return 1;
		}
		for(i=0;i<maxconn;i++)
		{
			pool[i].errflag=0;
			pool[i].closeflag=0;
			pool[i].busyflag=0;
			pool[i].connflag=0;
		}
	}
	timelimit=timeout*sysconf(_SC_CLK_TCK);
	requestlimit=request*sysconf(_SC_CLK_TCK);

	sigemptyset(&ss);
	sigaddset(&ss,SIGHUP);
	sigaddset(&ss,SIGINT);
	sigaddset(&ss,SIGTERM);
	sigaddset(&ss,SIGUSR1);
	sigaddset(&ss,SIGUSR2);

	term=0;
	signal(SIGINT,inthandler);
	signal(SIGTERM,termhandler);

	reload=0;
	signal(SIGHUP,reloadhandler);

	newdb=0;
	newdatabase=NULL;
	signal(SIGUSR1,usr1handler);
	signal(SIGUSR2,usr2handler);

	pfd.fd=l;
	pfd.events=POLLIN;

	if(pidfile)if((v=open(pidfile,O_WRONLY|O_TRUNC|O_CREAT,0644))==-1)
	{
		close(l);
		if(password)memset(password,0,strlen(password));
		errmsg("can't open/create pid file\n",0);
		return 1;
	}

	if(!nodaemon)if(daemon(1,0))
	{
		close(l);
		if(pidfile)
		{
			close(v);
			unlink(pidfile);
		}
		if(password)memset(password,0,strlen(password));
		errmsg("can't become daemon\n",0);
		return 1;
	}

	if(pidfile)
	{
		if(!(i=getpid()))write(v,"0\n",2);
		else
		{
			for(c=0;i;c++,i/=10)n[c]='0'+i%10;
			while(c--)write(v,&n[c],1);
			write(v,"\n",1);
		}
		close(v);
	}

	conncount=0;
	collect=0;
	stamp=time(NULL);

	while(!term)
	{
		if(reload)
		{
			pthread_sigmask(SIG_BLOCK,&ss,NULL);
			for(;reload;reload--)sem_wait(&children);
			pthread_sigmask(SIG_UNBLOCK,&ss,NULL);
			if(!(t=loadtemplate(template)))continue;
			pthread_mutex_lock(&mtx);
			c=--(tmpl->usage);
			pthread_mutex_unlock(&mtx);
			if(!c)freetemplate(tmpl);
			tmpl=t;
			continue;
		}
		if(newdb)
		{
			pthread_sigmask(SIG_BLOCK,&ss,NULL);
			for(;newdb;newdb--)sem_wait(&children);
			database=newdatabase;
			pthread_sigmask(SIG_UNBLOCK,&ss,NULL);
			if(!usepool)continue;
			pthread_mutex_lock(&poolmtx);
			for(c=0;c<maxconn;c++)if(pool[c].connflag)
			{
				if(pool[c].busyflag)pool[c].closeflag=1;
				else
				{
					for(v=0;v<PREPTOTAL;v++)
						sqlfin(pool[c].db,
							pool[c].sql[v]);
					for(v=0;v<TOTALTMPTBL;v++)
						sqlrun(pool[c].db,
							tmptables[v].drop);
					sqlclose(pool[c].db,0);
					pool[c].connflag=0;
					pool[c].errflag=0;
					pool[c].closeflag=0;
				}
			}
			pthread_mutex_unlock(&poolmtx);
			conncount=0;
			collect=0;
			stamp=time(NULL);
			continue;
		}
		if(collect)
		{
			conncount=0;
			collect=0;
			stamp=time(NULL);
			if(!usepool)continue;
			pthread_mutex_lock(&poolmtx);
			for(v=usepool-1;v>=0;v--)if(pool[v].connflag&&
				!pool[v].busyflag&&stamp-pool[v].ping>599)
			{
				for(c=0;c<PREPTOTAL;c++)sqlfin(pool[v].db,
					pool[v].sql[c]);
				for(c=0;c<TOTALTMPTBL;c++)sqlrun(pool[v].db,
					tmptables[c].drop);
				sqlclose(pool[v].db,0);
				pool[v].connflag=0;
				pool[v].errflag=0;
				pool[v].closeflag=0;
				break;
			}
			else if(pool[v].busyflag)break;
			pthread_mutex_unlock(&poolmtx);
			continue;
		}
		sem_wait(&children);
		dosleep=0;
		while(1)
		{
			if(term||reload||newdb||collect)
			{
				sem_post(&children);
				break;
			}
			if(dosleep)if(poll(&pfd,1,1000)!=1)
			{
				curr=time(NULL);
				if(curr-stamp>59)collect=1;
				continue;
			}
			dosleep=0;
			sl=sizeof(a);
			v=accept(l,(struct sockaddr *)(&a),&sl);
			if(v==-1)
			{
				dosleep=1;
				continue;
			}
			if(++conncount>249)collect=1;
			if(sl!=sizeof(a))
			{
				close(v);
				continue;
			}
			if(fcntl(v,F_SETFL,O_NONBLOCK))
			{
				close(v);
				continue;
			}
#if defined(SOL_TCP) && defined(TCP_CORK)
			c=1;
			if(setsockopt(v,SOL_TCP,TCP_CORK,&c,sizeof(c)))
			{
				close(v);
				continue;
			}
#endif
			if(!(r=malloc(sizeof(REQUEST))))
			{
				close(v);
				continue;
			}

			pthread_mutex_lock(&mtx);
			(tmpl->usage)++;
			pthread_mutex_unlock(&mtx);

			r->catid=0;
			r->start=0;
			r->ktm=0;
			r->pagetype=RTYPE_ERR400;
			r->searchmode=0;
			r->outputdone=0;
			r->outputdmem=0;
			r->iomode=0;
			r->starttime=times(NULL);
			r->tmpl=tmpl;
			r->data=NULL;
			r->host=NULL;
			for(c=0;c<NLSTOTAL;c++)r->nls[c]=NULL;
			for(c=0;c<MAXLANG;c++)r->lang[c]=NULL;
			r->w.fd=r->r.fd=r->fd=v;
			r->ridx=r->rlen=r->widx=r->wlen=r->gzip=0;
			r->r.events=POLLIN;
			r->w.events=POLLOUT;

			pthread_sigmask(SIG_BLOCK,&ss,NULL);
			if(pthread_create(&thr,&attr,do_request,r))
			{
				pthread_mutex_lock(&mtx);
				(tmpl->usage)--;
				pthread_mutex_unlock(&mtx);
				free(r);
				close(v);
				pthread_sigmask(SIG_UNBLOCK,&ss,NULL);
				continue;
			}
			pthread_sigmask(SIG_UNBLOCK,&ss,NULL);
			break;
		}
	}

	close(l);
	while(term<2)
	{
		sem_getvalue(&children,&c);
		if(c==limit)break;
		usleep(100000);
	}
	if(pidfile)unlink(pidfile);
	if(password)memset(password,0,strlen(password));
	pthread_attr_destroy(&attr);
	sem_destroy(&children);
	pthread_mutex_destroy(&mtx);
	if(usepool)
	{
		pthread_mutex_lock(&poolmtx);
#ifdef Linux
		pthread_kill_other_threads_np();
#endif
		for(i=0;i<maxconn;i++)if(pool[i].connflag)
		{
			for(c=0;c<PREPTOTAL;c++)
				sqlfin(pool[i].db,pool[i].sql[c]);
			for(c=0;c<TOTALTMPTBL;c++)
				sqlrun(pool[i].db,tmptables[c].drop);
			sqlclose(pool[i].db,0);
		}
		free(pool);
		pool=NULL;
		pthread_mutex_unlock(&poolmtx);
		pthread_mutex_destroy(&poolmtx);
		sem_destroy(&dbpool);
	}
	sqlexit();
	freetemplate(tmpl);

	return 0;
}
