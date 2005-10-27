/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <zlib.h>
#include "lib.h"
#include "db.h"

#if INT_MAX < 2147483647
#error int must be at least 4 bytes
#endif
#if USHRT_MAX < 65535
#error short must be at least 2 bytes
#endif
#if UCHAR_MAX < 255
#error short must be at least 1 byte
#endif

#define TOPICSTBL "topics"
#define TOPICSFMT "CREATE TABLE "TOPICSTBL " (catid" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",id" SQLTYPE_TEXT ",title" SQLTYPE_TEXT ",lastupdate" \
	SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define TOPICSROW "(?,?,?,?)"
#define TOPICSPRP "INSERT INTO " TOPICSTBL " VALUES" TOPICSROW
#define TOPICSINS "ittt"
#define AOLSEARCHTBL "aolsearch"
#define AOLSEARCHFMT "CREATE TABLE " AOLSEARCHTBL " (catid" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",value" SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define AOLSEARCHROW "(?,?)"
#define AOLSEARCHPRP "INSERT INTO " AOLSEARCHTBL " VALUES" AOLSEARCHROW
#define AOLSEARCHINS "it"
#define DISPNAMETBL "dispname"
#define DISPNAMEFMT "CREATE TABLE " DISPNAMETBL " (catid" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",value" SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define DISPNAMEROW "(?,?)"
#define DISPNAMEPRP "INSERT INTO " DISPNAMETBL " VALUES" DISPNAMEROW
#define DISPNAMEINS "it"
#define DESCRIPTIONTBL "description"
#define DESCRIPTIONFMT "CREATE TABLE " DESCRIPTIONTBL " (catid" \
	SQLTYPE_INTEGER SQLTYPE_PRIMARY ",value" SQLTYPE_TEXT ")" SQLTBL_STD
#define DESCRIPTIONROW "(?,?)"
#define DESCRIPTIONPRP "INSERT INTO " DESCRIPTIONTBL " VALUES" DESCRIPTIONROW
#define DESCRIPTIONINS "it"
#define CHEDNETBL "chedne"
#define CHEDNEFMT "CREATE TABLE " CHEDNETBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",catid" SQLTYPE_INTEGER ",type" SQLTYPE_TINYINT \
	",id" SQLTYPE_INTEGER ")" SQLTBL_STD
#define CHEDNEROW "(?,?,?,?)"
#define CHEDNEPRP "INSERT INTO " CHEDNETBL " VALUES" CHEDNEROW
#define CHEDNEINS "iiii"
#define CHARSETTBL "charset"
#define CHARSETFMT "CREATE TABLE " CHARSETTBL " (id" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",name" SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define CHARSETROW "(?,?)"
#define CHARSETPRP "INSERT INTO " CHARSETTBL " VALUES" CHARSETROW
#define CHARSETINS "it"
#define EDITORTBL "editor"
#define EDITORFMT "CREATE TABLE " EDITORTBL " (id" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",name" SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define EDITORROW "(?,?)"
#define EDITORPRP "INSERT INTO " EDITORTBL " VALUES" EDITORROW
#define EDITORINS "it"
#define NEWSGROUPTBL "newsgroup"
#define NEWSGROUPFMT "CREATE TABLE " NEWSGROUPTBL " (id" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",name" SQLTYPE_TEXT ")" SQLTBL_STD
#define NEWSGROUPROW "(?,?)"
#define NEWSGROUPPRP "INSERT INTO "NEWSGROUPTBL " VALUES" NEWSGROUPROW
#define NEWSGROUPINS "it"
#define LANGUAGESTBL "languages"
#define LANGUAGESFMT "CREATE TABLE " LANGUAGESTBL " (id" SQLTYPE_SMALLINT \
	SQLTYPE_PRIMARY ",name" SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define LANGUAGESROW "(?,?)"
#define LANGUAGESPRP "INSERT INTO " LANGUAGESTBL " VALUES" LANGUAGESROW
#define LANGUAGESINS "it"
#define LANGLINKSTBL "langlinks"
#define LANGLINKSFMT "CREATE TABLE " LANGLINKSTBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",src" SQLTYPE_INTEGER ",dst" SQLTYPE_INTEGER \
	",lang" SQLTYPE_SMALLINT ")" SQLTBL_STD
#define LANGLINKSROW "(?,?,?,?)"
#define LANGLINKSPRP "INSERT INTO " LANGLINKSTBL " VALUES" LANGLINKSROW
#define LANGLINKSINS "iiii"
#define ALIASESTBL "aliases"
#define ALIASESFMT "CREATE TABLE " ALIASESTBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",target" SQLTYPE_INTEGER ",id" SQLTYPE_TEXT \
	",title" SQLTYPE_TEXT ")" SQLTBL_STD
#define ALIASESROW "(?,?,?,?)"
#define ALIASESPRP "INSERT INTO " ALIASESTBL " VALUES" ALIASESROW
#define ALIASESINS "iitt"
#define SYMLINKSTBL "symlinks"
#define SYMLINKSFMT "CREATE TABLE " SYMLINKSTBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",src" SQLTYPE_INTEGER ",level" SQLTYPE_TINYINT \
	",alias" SQLTYPE_INTEGER ",dst" SQLTYPE_INTEGER ")" SQLTBL_STD
#define SYMLINKSROW "(?,?,?,?,?)"
#define SYMLINKSPRP "INSERT INTO " SYMLINKSTBL " VALUES" SYMLINKSROW
#define SYMLINKSINS "iiiii"
#define RELATEDTBL "related"
#define RELATEDFMT "CREATE TABLE " RELATEDTBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",src" SQLTYPE_INTEGER ",dst" SQLTYPE_INTEGER ")" \
	SQLTBL_STD
#define RELATEDROW "(?,?,?)"
#define RELATEDPRP "INSERT INTO " RELATEDTBL " VALUES" RELATEDROW
#define RELATEDINS "iii"
#define NARROWTBL "narrow"
#define NARROWFMT "CREATE TABLE " NARROWTBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",src" SQLTYPE_INTEGER ",level" SQLTYPE_TINYINT \
	",dst" SQLTYPE_INTEGER ")" SQLTBL_STD
#define NARROWROW "(?,?,?,?)"
#define NARROWPRP "INSERT INTO " NARROWTBL " VALUES" NARROWROW
#define NARROWINS "iiii"
#define LETTERBARTBL "letterbar"
#define LETTERBARFMT "CREATE TABLE " LETTERBARTBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",src" SQLTYPE_INTEGER ",dst" SQLTYPE_INTEGER \
	",title" SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define LETTERBARROW "(?,?,?,?)"
#define LETTERBARPRP "INSERT INTO " LETTERBARTBL " VALUES" LETTERBARROW
#define LETTERBARINS "iiit"
#define TERMSTBL "terms"
#define TERMSFMT "CREATE TABLE " TERMSTBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",catid" SQLTYPE_INTEGER ",term" SQLTYPE_TINYTEXT ")" \
	SQLTBL_STD
#define TERMSROW "(?,?,?)"
#define TERMSPRP "INSERT INTO " TERMSTBL " VALUES" TERMSROW
#define TERMSINS "iit"
#define TOPICWORDSTBL "topicwords"
#define TOPICWORDSFMT "CREATE TABLE " TOPICWORDSTBL " (wordid" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",word" SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define TOPICWORDSROW "(?,?)"
#define TOPICWORDSPRP "INSERT INTO " TOPICWORDSTBL " VALUES" TOPICWORDSROW
#define TOPICWORDSINS "it"
#define TOPICSEARCHTBL "topicsearch"
#define TOPICSEARCHFMT "CREATE TABLE " TOPICSEARCHTBL " (serial" \
	SQLTYPE_INTEGER SQLTYPE_PRIMARY ",wordid" SQLTYPE_INTEGER ",catid" \
	SQLTYPE_INTEGER ",ages" SQLTYPE_TINYINT ")" SQLTBL_STD
#define TOPICSEARCHROW "(?,?,?,?)"
#define TOPICSEARCHPRP "INSERT INTO " TOPICSEARCHTBL " VALUES" TOPICSEARCHROW
#define TOPICSEARCHINS "iiii"
#define TOPICREFTBL "topicref"
#define TOPICREFFMT "CREATE TABLE " TOPICREFTBL " (catid" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",parentid" SQLTYPE_INTEGER ")" SQLTBL_STD
#define TOPICREFROW "(?,?)"
#define TOPICREFPRP "INSERT INTO " TOPICREFTBL " VALUES" TOPICREFROW
#define TOPICREFINS "ii"
#define TOPICTRACETBL "topictrace"
#define TOPICTRACEFMT "CREATE TABLE " TOPICTRACETBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",refid" SQLTYPE_INTEGER ",catid" SQLTYPE_INTEGER ")" \
	SQLTBL_STD
#define TOPICTRACEROW "(?,?,?)"
#define TOPICTRACEPRP "INSERT INTO " TOPICTRACETBL " VALUES" TOPICTRACEROW
#define TOPICTRACEINS "iii"
#define NLSSUPPORTTBL "nlssupport"
#define NLSSUPPORTFMT "CREATE TABLE " NLSSUPPORTTBL " (serial" \
	SQLTYPE_SMALLINT SQLTYPE_PRIMARY ",name" SQLTYPE_TINYTEXT ",id" \
	SQLTYPE_TINYTEXT ",seealso" SQLTYPE_TINYTEXT ",otherlang" \
	SQLTYPE_TINYTEXT ",description" SQLTYPE_TINYTEXT ",editor" \
	SQLTYPE_TINYTEXT ",searchonpre" SQLTYPE_TINYTEXT ",searchonpost" \
	SQLTYPE_TINYTEXT ",searchfor" SQLTYPE_TINYTEXT ",searchalldir" \
	SQLTYPE_TINYTEXT ",searchforin" SQLTYPE_TINYTEXT ",lastupdate" \
	SQLTYPE_TINYTEXT ",termsofuse" SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define NLSSUPPORTROW "(?,?,?,?,?,?,?,?,?,?,?,?,?,?)"
#define NLSSUPPORTPRP "INSERT INTO " NLSSUPPORTTBL " VALUES" NLSSUPPORTROW
#define NLSSUPPORTINS "ittttttttttttt"
#define NLSINFOTBL "nlsinfo"
#define NLSINFOFMT "CREATE TABLE " NLSINFOTBL " (catid" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",nlsid" SQLTYPE_SMALLINT ")" SQLTBL_STD
#define NLSINFOROW "(?,?)"
#define NLSINFOPRP "INSERT INTO " NLSINFOTBL " VALUES" NLSINFOROW
#define NLSINFOINS "ii"
#define EXTERNALPAGESTBL "externalpages"
#define ALLEXTERNALPAGESFMT "CREATE TABLE " EXTERNALPAGESTBL " (id" \
	SQLTYPE_INTEGER SQLTYPE_PRIMARY ",catid" SQLTYPE_INTEGER ",linktype" \
	SQLTYPE_SMALLINT ",linklevel" SQLTYPE_SMALLINT ",priority" \
	SQLTYPE_SMALLINT ",type" SQLTYPE_INTEGER ",link" SQLTYPE_TEXT \
	",title" SQLTYPE_TEXT ",description" SQLTYPE_TEXT ",mediadate" \
	SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define STDEXTERNALPAGESFMT "CREATE TABLE " EXTERNALPAGESTBL " (id" \
	SQLTYPE_INTEGER SQLTYPE_PRIMARY ",catid" SQLTYPE_INTEGER ",linktype" \
	SQLTYPE_SMALLINT ",priority" SQLTYPE_SMALLINT ",link" SQLTYPE_TEXT \
	",title" SQLTYPE_TEXT ",description" SQLTYPE_TEXT ",mediadate" \
	SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define ALLEXTERNALPAGESROW "(?,?,?,?,?,?,?,?,?,?)"
#define ALLEXTERNALPAGESPRP "INSERT INTO " EXTERNALPAGESTBL " VALUES" \
	ALLEXTERNALPAGESROW
#define STDEXTERNALPAGESROW "(?,?,?,?,?,?,?,?)"
#define STDEXTERNALPAGESPRP "INSERT INTO " EXTERNALPAGESTBL " VALUES" \
	STDEXTERNALPAGESROW
#define ALLEXTERNALPAGESINS "iiiiiitttt"
#define STDEXTERNALPAGESINS "iiiitttt"
#define TYPESTBL "types"
#define TYPESFMT "CREATE TABLE " TYPESTBL " (id" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",value" SQLTYPE_TEXT ")" SQLTBL_STD
#define TYPESROW "(?,?)"
#define TYPESPRP "INSERT INTO " TYPESTBL " VALUES" TYPESROW
#define TYPESINS "it"
#define PAGEWORDSTBL "pagewords"
#define ALLPAGEWORDSFMT "CREATE TABLE " PAGEWORDSTBL " (wordid" \
	SQLTYPE_INTEGER SQLTYPE_PRIMARY ",word" SQLTYPE_TINYTEXT ",occurrence" \
	SQLTYPE_INTEGER ")" SQLTBL_STD
#define STDPAGEWORDSFMT "CREATE TABLE " PAGEWORDSTBL " (wordid" \
	SQLTYPE_INTEGER SQLTYPE_PRIMARY ",word" SQLTYPE_TINYTEXT ")" SQLTBL_STD
#define ALLPAGEWORDSROW "(?,?,?)"
#define ALLPAGEWORDSPRP "INSERT INTO " PAGEWORDSTBL " VALUES" ALLPAGEWORDSROW
#define STDPAGEWORDSROW "(?,?)"
#define STDPAGEWORDSPRP "INSERT INTO " PAGEWORDSTBL " VALUES" STDPAGEWORDSROW
#define ALLPAGEWORDSINS "iti"
#define STDPAGEWORDSINS "it"
#define PAGESEARCHTBL "pagesearch"
#define PAGESEARCHFMT "CREATE TABLE " PAGESEARCHTBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",wordid" SQLTYPE_INTEGER ",weight" SQLTYPE_INTEGER \
	",pageid" SQLTYPE_INTEGER ",catid" SQLTYPE_INTEGER ",ages " \
	SQLTYPE_TINYINT ")" SQLTBL_STD
#define PAGESEARCHROW "(?,?,?,?,?,?)"
#define PAGESEARCHPRP "INSERT INTO " PAGESEARCHTBL " VALUES" PAGESEARCHROW
#define PAGESEARCHINS "iiiiii"
#define COUNTERSTBL "counters"
#define COUNTERSFMT "CREATE TABLE " COUNTERSTBL " (catid" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",topics" SQLTYPE_INTEGER ",pages" SQLTYPE_INTEGER ")" \
	SQLTBL_STD
#define COUNTERSROW "(?,?,?)"
#define COUNTERSPRP "INSERT INTO " COUNTERSTBL " VALUES" COUNTERSROW
#define COUNTERSINS "iii"
#define REDIRECTSTBL "redirects"
#define REDIRECTSFMT "CREATE TABLE " REDIRECTSTBL " (serial" SQLTYPE_INTEGER \
	SQLTYPE_PRIMARY ",id" SQLTYPE_TEXT ",catid" SQLTYPE_INTEGER ")" \
	SQLTBL_STD
#define REDIRECTSROW "(?,?,?)"
#define REDIRECTSPRP "INSERT INTO " REDIRECTSTBL " VALUES" REDIRECTSROW
#define REDIRECTSINS "iti"

#define STOPWORDS 4693

#define ENTITIES  252
#define ENTISTART 128

#define MAXHTML 95
#define HTMLSTART 64

#define WORDWEIGHT(a)   (16777216.0/sqrt(a))
#define PAGEWEIGHT(a)   log10(pow((double)(a)-8.0,2.0)+10.0)
#define PRIOWEIGHT(a,b) (int)((((a)*8.0)/PAGEWEIGHT(b))+0.5)
#define HIGHWEIGHT(a,b) (int)((((a)*4.0)/PAGEWEIGHT(b))+0.5)
#define MOREWEIGHT(a,b) (int)((((a)*2.0)/PAGEWEIGHT(b))+0.5)
#define ITEMWEIGHT(a,b) (int)(((a)/PAGEWEIGHT(b))+0.5)

#define PAGEMASK 0x0fffffff
#define INDESCR  0x10000000
#define INTITLE  0x20000000
#define PRIORITY 0x40000000

#define BUFSIZEH 65536
#define BUFSIZEM 8192
#define BUFSIZES 1024
#define BUFSIZET 256
#define LUPDBUF  20
#define DATEBUF  12

#define MAXDEPTH 256

#define DATASIZE 1024

#define MINWORD  3
#define MAXWORD  42

#define LISTSIZE 0x40000
#define LISTMASK 0x3ffff

#define STOREBASE  64
#define STORESHIFT 16
#define STORECHUNK (1<<STORESHIFT)
#define STOREMASK (STORECHUNK-1)

#define DFTDYNBLOCK   0x80000
#define LARGEDYNBLOCK 0x2000

#define DFTBLOCK 0x20000

#define DFTPOOLSIZE 0x400000

typedef struct _pool
{
	struct _pool *next;
	int size;
	int avail;
	int offset;
	char data[0];
} POOL;

typedef struct _block
{
	struct _block *next;
	int *data;
	int *last;
	int total;
	int free;
	unsigned int map[0];
} BLOCK;

typedef struct _dynblock
{
	struct _dynblock *next;
	int *last;
	int size;
	int firstfree;
	int data[0];
} DYNBLOCK;

typedef struct _item
{
	struct _item *next;
	char item[0];
} DATALIST;

typedef struct _itemlist
{
	struct _itemlist *next;
	DATALIST *data;
	int catid;
} ITEMLIST;

typedef struct
{
	int total;
	int used;
	unsigned char **dta8;
	unsigned short **dta16;
	int *dta32[0];
} STORE;

typedef struct _topictrace
{
	struct _topictrace *next;
	int catid;
	int refid;
} TOPICTRACE;

typedef struct _alias
{
	struct _alias *next;
	int target;
	int titleoffset;
	char id[0];
} ALIAS;

typedef struct _langlink
{
	struct _langlink *next;
	int src;
	int dst;
	int lang;
} LANGLINK;

typedef struct _symlink
{
	struct _symlink *next;
	int src;
	int dst;
	int alias;
	int level;
} SYMLINK;

typedef struct _chedne
{
	struct _chedne *next;
	int catid;
	int id;
	int type;
} CHEDNE;

typedef struct _description
{
	struct _description *next;
	int catid;
	char desc[0];
} DESCRIPTION;

typedef struct _redirect
{
	struct _redirect *next;
	int catid;
	char id[0];
} REDIRECT;

typedef struct _related
{
	struct _related *next;
	int src;
	int dst;
} RELATED;

typedef struct _narrow
{
	struct _narrow *next;
	int src;
	int dst;
	int level;
} NARROW;

typedef struct _letterbar
{
	struct _letterbar *next;
	int src;
	int dst;
	char title[0];
} LETTERBAR;

typedef struct
{
	int pageid;
	int weight;
	int catid;
	int ages;
} PAGEDATA;

typedef struct
{
	int pageid;
	int total;
} PAGELIST;

typedef struct _wordlist
{
	struct _wordlist *next;
	union
	{
  		int *pagelist;
		int page;
	} u;
	int flags;
	unsigned int occurrence;
	unsigned char word[0];
} WORDLIST;

typedef struct _topic
{
	struct _topic *next;
	union
	{
		DATALIST *list;
		int counter;
	} u;
	int catid;
	char id[0];
} TOPIC;

typedef struct _topicitem
{
	struct _topicitem *next;
	TOPIC *topic;
	int updateoffset;
	char title[0];
} TOPICITEM;

typedef struct
{
	int src;
	int dst;
} TREE;

typedef struct
{
	int mode;
	int in_alias;
	int in_topic;
	int catid;
	int total;
	int start;
	int total2;
	int start2;
	int total3;
	int total4;
	int total5;
	int total6;
	int total7;
	int clen;
	DATALIST *list1;
	DATALIST *list2;
	DATALIST *list3;
	DATALIST **array;
	ALIAS *achain;
	ALIAS **aliases;
	LANGLINK *lchain;
	LANGLINK **langlinks;
	SYMLINK *schain;
	SYMLINK **symlinks;
	ITEMLIST *aolchain;
	ITEMLIST *dispchain;
	DESCRIPTION *descchain;
	TOPICITEM *topicchain;
	CHEDNE *cenchain;
	RELATED *rchain;
	RELATED **related;
	NARROW *nchain;
	NARROW **narrow;
	LETTERBAR *bchain;
	LETTERBAR **bar;
	DB db;
	SQL sql;
	char aolsearch[BUFSIZES];
	char dispname[BUFSIZES];
	char id[BUFSIZES];
	char target[BUFSIZES];
	char title[BUFSIZET];
	char lastupdate[LUPDBUF];
	char cdata[BUFSIZEH];
	char desc[BUFSIZEH];
} TOPICDATA;

typedef struct
{
	int total;
	int clen;
	REDIRECT *redirchain;
	DB db;
	SQL sql;
	char id[BUFSIZES];
	char target[BUFSIZES];
	char cdata[BUFSIZEH];
} REDIRECTDATA;

typedef struct
{
	int total;
	int catid;
	int notfound;
	int clen;
	ITEMLIST *termchain;
	TOPIC *r;
	DB db;
	SQL sql;
	char cdata[BUFSIZEH];
	char id[BUFSIZES];
	char target[BUFSIZES];
} TERMDATA;

typedef struct _contentlist
{
	struct _contentlist *next;
	int linktype;
	int linklevel;
	char link[0];
} CONTENTLIST;

typedef struct
{
	int count;
	int in_topic;
	int in_extpage;
	int clen;
	int ages;
#ifdef SQLFLAG_MULTIROW
	int idx;
	int catid[8];
	int priority[8];
	int type[8];
	int mem[8][2];
#else
	int catid;
	int priority;
	int type;
#endif
	int total;
	int start;
	DATALIST *list1;
	CONTENTLIST *clist;
	STORE *store;
	BLOCK *block[5];
	DB db;
	SQL sql;
#ifdef SQLFLAG_MULTIROW
	char link[8][BUFSIZEM];
	char title[8][BUFSIZEM];
	char desc[8][BUFSIZEH];
	char date[8][DATEBUF];
#else
	char link[BUFSIZEM];
	char title[BUFSIZEM];
	char desc[BUFSIZEH];
	char date[DATEBUF];
#endif
	char id[BUFSIZEM];
	char cdata[BUFSIZEH];
	WORDLIST *list[LISTSIZE];
} CONTENTDATA;

typedef struct
{
	int total;
	int start;
	int treetotal;
	int treestart;
	TREE *tree;
	POOL *main;
	POOL *aux;
	POOL *lcl;
	TOPIC **catid;
	TOPIC *id[LISTSIZE];
} CACHE;

static const char *indexes[]=
{
	"CREATE INDEX topicids on topics (" SQLSTMT_TEXTIDX("id") ")",
	"CREATE INDEX tpref on topicref (parentid)",
	"CREATE INDEX tptrc on topictrace (refid,catid)",
	"CREATE INDEX langsrc on langlinks (src,dst)",
	"CREATE INDEX symsrc on symlinks (src,level)",
	"CREATE INDEX relsrc on related (src,dst)",
	"CREATE INDEX narrowsrc on narrow (src,level)",
	"CREATE INDEX lettersrc on letterbar (src,dst)",
	"CREATE INDEX rediridx on redirects (" SQLSTMT_TEXTIDX("id") ")",
	"CREATE INDEX extpgidx on externalpages (catid)",
	"CREATE INDEX chedneidx on chedne (catid,type)",
	"CREATE INDEX tpwdwdidx on topicwords (" SQLSTMT_TEXTIDX("word") ")",
	"CREATE INDEX tpscwdidx on topicsearch (wordid)",
	"CREATE INDEX pgwdwdidx on pagewords (" SQLSTMT_TEXTIDX("word") ")",
	"CREATE INDEX pgscwdidx on pagesearch (wordid)",
#ifdef SQLIDX_EXTRA
	SQLIDX_EXTRA,
#endif
	NULL
};

static const struct
{
	const char *name;
	const char *num;
} months[]=
{
	{"january","01"},
	{"february","02"},
	{"march","03"},
	{"april","04"},
	{"may","05"},
	{"june","06"},
	{"july","07"},
	{"august","08"},
	{"september","09"},
	{"october","00"},
	{"november","11"},
	{"december","12"},
	{NULL,NULL}
};

static const char *langbase[]=
{
	"Top/World",
	"Top/Kids_and_Teens/International",
	NULL
};

static const struct
{
	const int len;
	const char *str;
} ktbase=
{
	18,
	"Top/Kids_and_Teens"
};

static const struct
{
	const char *tree;
	const char *name;
	const char *id;
} languages[]=
{
	{"","English","en"},
	{"Afrikaans","Afrikaans","af"},
	{"Arabic","Arabic","ar"},
	{"Armenian","Armenian","hy"},
	{"Azerbaijani","Azerbaijani","az"},
	{"Bable","Asturian","ast"},
	{"Bahasa_Melayu","Malay","ms"},
	{"Bangla","Bangla","bn"},
	{"Belarusian","Belarusian","be"},
	{"Bosanski","Bosnian","bs"},
	{"Brezhoneg","Breton","br"},
	{"Bulgarian","Bulgarian","bg"},
	{"Català","Catalan","ca"},
	{"Chinese_Simplified","Chinese Simplified","zh_CN"},
	{"Chinese_Traditional","Chinese Traditional","zh_TW"},
	{"Cymraeg","Welsh","cy"},
	{"Dansk","Danish","da"},
	{"Deutsch","German","de"},
	{"Eesti","Estonian","et"},
	{"Español","Spanish","es"},
	{"Esperanto","Esperanto","eo"},
	{"Euskara","Basque","eu"},
	{"Farsi","Farsi","fa"},
	{"Français","French","fr"},
	{"Frysk","Frisian","fy"},
	{"Føroyskt","Faroese","fo"},
	{"Gaeilge","Irish","ga"},
	{"Galego","Galician","gl"},
	{"Greek","Greek","el"},
	{"Gujarati","Gujarati","guj"},
	{"Gàidhlig","Scots Gaelic","gd"},
	{"Hebrew","Hebrew","he"},
	{"Hindi","Hindi","hi"},
	{"Hrvatski","Croatian","hr"},
	{"Indonesia","Indonesian","id"},
	{"Interlingua","Interlingua","ia"},
	{"Italiano","Italian","it"},
	{"Japanese","Japanese","ja"},
	{"Kannada","Kannada","kn"},
	{"Kaszëbsczi","Kashubian","csb"},
	{"Kiswahili","Swahili","sw"},
	{"Korean","Korean","ko"},
	{"Kurdî","Kurdish","ku"},
	{"Latvian","Latvian","lv"},
	{"Lietuvių","Lithuanian","lt"},
	{"Lingua_Latina","Latin","la"},
	{"Lëtzebuergesch","Luxembourgish","lb"},
	{"Magyar","Hungarian","hu"},
	{"Makedonski","Macedonian","mk"},
	{"Marathi","Marathi","mr"},
	{"Nederlands","Dutch","nl"},
	{"Norsk","Norwegian","no"},
	{"Occitan","Occitan","oc"},
	{"Polska","Polish","pl"},
	{"Português","Portuguese","pt"},
	{"Punjabi","Punjabi","pa"},
	{"Română","Romanian","ro"},
	{"Rumantsch","Romansh","rm"},
	{"Russian","Russian","ru"},
	{"Sardu","Sardinian","sc"},
	{"Shqip","Albanian","sq"},
	{"Slovensko","Slovenian","sl"},
	{"Slovensky","Slovak","sk"},
	{"Srpski","Serbian","sr"},
	{"Suomi","Finnish","fi"},
	{"Svenska","Swedish","sv"},
	{"Tagalog","Tagalog","tl"},
	{"Taiwanese","Taiwanese","th"},
	{"Tamil","Tamil","ta"},
	{"Tatarça","Tatar","tt"},
	{"Telugu","Telugu","te"},
	{"Thai","Thai","th"},
	{"Türkçe","Turkish","tr"},
	{"Ukrainian","Ukrainian","uk"},
	{"Vietnamese","Vietnamese","vi"},
	{"Íslenska","Icelandic","is"},
	{"Česky","Czech","cs"},
	{NULL,NULL,NULL}
};

static const char *seealso[]=
{
	"See also:",
	"See also:",
	"أنظر أيضا",
	"Տեսէք նաեւ",
	"Həmçinin baxın:",
	"See also:",
	"Lihat juga:",
	"See also:",
	"See also:",
	"Pogledajte i:",
	"See also:",
	"Виж също:",
	"Mira també:",
	"相关类别：",
	"相關類別：",
	"See also:",
	"Se også:",
	"Siehe auch:",
	"See also:",
	"Ver también:",
	"Vidu ankaŭ:",
	"Ikus halaber:",
	"همچنین نگاه كنید به:",
	"Voir également :",
	"See also:",
	"See also:",
	"See also:",
	"See also:",
	"Δείτε επίσης:",
	"See also:",
	"See also:",
	"ראה גם:",
	"इन्हें भी देखें:",
	"Također pogledaj:",
	"Lihat juga:",
	"See also:",
	"Vedi anche:",
	"こちらもご参照ください:",
	"See also:",
	"See also:",
	"See also:",
	"&#44288;&#47144; &#52852;&#53580;&#44256;&#47532;&#65306;",
	"See also:",
	"See also:",
	"Susijusios kategorijos:",
	"Quoque vides:",
	"See also:",
	"Lásd még:",
	"See also:",
	"See also:",
	"Zie ook:",
	"Se også:",
	"Veir tanben:",
	"Zobacz także:",
	"Veja também:",
	"See also:",
	"A se vedea şi:",
	"See also:",
	"См.также:",
	"Abbàida fintzas custu:",
	"See also:",
	"See also:",
	"Rovnako pozri:",
	"Također pogledaj:",
	"Katso myös:",
	"Se också:",
	"See also:",
	"u 關係 e 類別：",
	"See also:",
	"See also:",
	"Ikkada kooda choodandi",
	"เพิ่มเติมที่ :",
	"İlgili kategoriler:",
	"Дивись також:",
	"See also:",
	"Sjá einnig:",
	"Další informace:",
	NULL
};

static const char *otherlang[]=
{
	"This category in other languages:",
	"This category in other languages:",
	"هذا الباب بلغات أخرى",
	"Այս դասաւորումը ուրիշ լեզուներով",
	"Bu kateqoriya digər dillərdə:",
	"Esta categoría n'otres llingües:",
	"Kategori ini dalam bahasa lain:",
	"This category in other languages:",
	"This category in other languages:",
	"Ova kategorija na drugim jezicima:",
	"This category in other languages:",
	"Тази категория на други езици:",
	"Aquesta categoria en altres idiomes:",
	"这个类别的其它语文版本：",
	"這個類別的其它語文版本：",
	"This category in other languages:",
	"Denne kategori på andre sprog:",
	"Diese Kategorie in anderen Sprachen:",
	"This category in other languages:",
	"Esta categoría en otros idiomas:",
	"Tiu kategorio en aliaj lingvoj:",
	"Atal hau gainerako hizkuntzetan:",
	"این شاخه به زبان‌های دیگر:",
	"Cette catégorie en d'autres langues&nbsp;:",
	"This category in other languages:",
	"This category in other languages:",
	"This category in other languages:",
	"This category in other languages:",
	"Η κατηγορία αυτή σε άλλες γλώσσες:",
	"This category in other languages:",
	"This category in other languages:",
	"קטגוריה זו בשפות נוספות:",
	"अन्य भाषाओं में यही वर्ग",
	"Ova kategorija na drugim jezicima",
	"Kategori ini dalam bahasa lain:",
	"This category in other languages:",
	"Questa categoria in altre lingue:",
	"ほかの言語でのこのカテゴリ",
	"This category in other languages:",
	"This category in other languages:",
	"This category in other languages:",
	"&#45796;&#47480; &#50616;&#50612;&#65306;",
	"This category in other languages:",
	"This category in other languages:",
	"Ši kategorija kitomis kalbomis:",
	"Idem genus aliis linguis",
	"This category in other languages:",
	"Ez a kategória más nyelveken:",
	"This category in other languages:",
	"This category in other languages:",
	"Deze categorie met sites in andere talen:",
	"Denne kategorien på andre språk:",
	"Aguesta categoria en autes lengues:",
	"Inne wersje językowe tej kategorii:",
	"Esta categoria em outros idiomas:",
	"This category in other languages:",
	"Această categorie în alte limbi:",
	"This category in other languages:",
	"Этот раздел на других языках:",
	"Custa categoria in àteras limbas:",
	"This category in other languages:",
	"This category in other languages:",
	"Táto kategória v iných jazykoch",
	"Ova kategorija na drugim jezicima",
	"Tämä luokka muilla kielillä:",
	"Denna kategori på andra språk:",
	"This category in other languages:",
	"別種語言 e kang-khoan 類別:",
	"This category in other languages:",
	"This category in other languages:",
	"This category in other languages:",
	"หมวดหมู่นี้ในภาษาอื่น",
	"Diğer dillerde bu kategori:",
	"This category in other languages:",
	"This category in other languages:",
	"This category in other languages:",
	"Tato kategorie v jiných jazycích:",
	NULL
};

static const char *description[]=
{
	"Description",
	"Description",
	"وصف",
	"Նկարագրութիւն",
	"Təsvir",
	"Descripción",
	"Huraian",
	"Description",
	"Description",
	"Opis",
	"Description",
	"Описание",
	"Descripció",
	"说明",
	"說明",
	"Description",
	"Beskrivelse",
	"Beschreibung",
	"Description",
	"Descripción",
	"Priskribo",
	"Azalpena",
	"شرح",
	"Description",
	"Description",
	"Description",
	"Description",
	"Description",
	"Περιγραφή",
	"Description",
	"Description",
	"תיאור",
	"वर्णन",
	"Opis",
	"Keterangan",
	"Description",
	"Descrizione",
	"説明",
	"Description",
	"Description",
	"Description",
	"&#49444;&#47749;",
	"Daxuyanî",
	"Description",
	"Aprašymas",
	"Descriptio",
	"Description",
	"Leírás",
	"Description",
	"Description",
	"Omschrijving",
	"Beskrivelse",
	"Descripcion",
	"Opis",
	"Descrição",
	"Description",
	"Descriere",
	"Description",
	"Описание",
	"Relata",
	"Description",
	"Description",
	"Popis",
	"Opis",
	"Kuvaus",
	"Beskrivning",
	"Description",
	"說明",
	"Description",
	"Description",
	"vivarana",
	"คำบรรยาย",
	"Açıklama",
	"Опис розділу",
	"Description",
	"Description",
	"Informace",
	NULL
};

static const char *editor[]=
{
	"Category editors:",
	"Category editors:",
	"محرر الباب:",
	"Category editors:",
	"Kateqoriyanın redaktorları:",
	"Editor de la categoría:",
	"Editor kategori:",
	"Category editor:",
	"Category editor:",
	"Editori kategorije:",
	"Category editor:",
	"редактори на категория:",
	"Editors de la categoria:",
	"类别编辑群:",
	"類別編輯群:",
	"Category editor:",
	"Redigeret af:",
	"Kategorie-Editoren:",
	"Category editor:",
	"Editores de la categoría:",
	"Redaktantoj de la kategorio:",
	"Ataleko editorea:",
	"ویراستاران این شاخه:",
	"Éditeurs de la catégorie:",
	"Category editor:",
	"Category editors:",
	"Category editor:",
	"Category editor:",
	"Συντάκτες κατηγορίας:",
	"Category editor:",
	"Category editor:",
	"עורכ/ת הקטגוריה:",
	"वर्ग सम्पादक:",
	"Editori Kategorije:",
	"Editor kategori:",
	"Category editor:",
	"Editori della categoria:",
	"カテゴリエディタ:",
	"Category editors:",
	"Category editors:",
	"Category editors:",
	"&#52852;&#53580;&#44256;&#47532; &#50640;&#46356;&#53552;:",
	"Category editors:",
	"Category editor:",
	"Kategorijos redaktorius:",
	"Category editors:",
	"Category editor:",
	"Kategória szerkesztői:",
	"Category editor:",
	"Category editor:",
	"Redacteuren van deze categorie:",
	"Kategoriredaktører:",
	"Category editors:",
	"Redaktorzy kategorii:",
	"Editor da categoria:",
	"Category editor:",
	"Editorii categoriei:",
	"Category editors:",
	"Редакторы раздела:",
	"Editore de sa categoria:",
	"Category editor:",
	"Category editors:",
	"Editor kategórie:",
	"Editor Kategorije:",
	"Luokan toimittajat:",
	"Kategoriredaktörer:",
	"Category editors:",
	"Category editors:",
	"Category editor:",
	"Category editor:",
	"Category editor:",
	"บรรณาธิการหมวดหมู่:",
	"Kategori editörleri:",
	"Редактор розділу:",
	"Category editor:",
	"Ritstjóri flokks:",
	"Editoři kategorie:",
	NULL
};

static const struct
{
	const char *pre;
	const char *post;
} searchon[]=
{
	{""," search on:"},
	{""," search on:"},
	{""," إبحث عن"},
	{""," բառով որոնում՝"},
	{""," axtar:"},
	{"Buscar "," n':"},
	{"Cari "," di:"},
	{""," search on:"},
	{""," search on:"},
	{""," potraži na:"},
	{""," search on:"},
	{""," търси в:"},
	{"Cerca "," a:"},
	{"用其它搜寻引擎来搜寻“","”："},
	{"用其它搜尋引擎來搜尋『","』："},
	{""," search on:"},
	{"søg efter "," i:"},
	{"Suche "," mit:"},
	{""," search on:"},
	{"Buscar "," en:"},
	{""," serĉu per:"},
	{"bilatu "," hemen:"},
	{"جستجوی «","» در:"},
	{"Chercher "," avec:"},
	{""," search on:"},
	{""," search on:"},
	{""," search on:"},
	{""," search on:"},
	{""," αναζήτηση σε:"},
	{""," search on:"},
	{""," search on:"},
	{"בצע חיפוש על ",""},
	{""," इनमें खोजें:"},
	{""," pretraži na:"},
	{"Cari "," di:"},
	{""," search on:"},
	{"Cerca "," in:"},
	{"","をサーチ:"},
	{""," search on:"},
	{""," search on:"},
	{""," search on:"},
	{"&#45796;&#47480; &#44160;&#49353;&#50644;&#51652; ",":"},
	{""," search on:"},
	{""," search on:"},
	{"Ieškoti "," per:"},
	{"Quaeris "," de:"},
	{""," search on:"},
	{""," keresés:"},
	{""," search on:"},
	{""," search on:"},
	{"Zoek naar "," bij:"},
	{"Søk etter "," i:"},
	{""," cercar en:"},
	{""," szukaj w:"},
	{"Procurar "," em:"},
	{""," search on:"},
	{"Caută "," cu:"},
	{""," search on:"},
	{""," поиск в:"},
	{"Chirca "," cun:"},
	{""," search on:"},
	{""," search on:"},
	{""," vyhľadať v:"},
	{""," pretraži na:"},
	{"Haku "," muualta:"},
	{"Sök "," med:"},
	{""," search on:"},
	{"ti pat-ui chhoe ",":"},
	{""," search on:"},
	{""," search on:"},
	{""," ikkada vethakandi"},
	{""," ค้นหาที่:"},
	{""," dizgisini ara:"},
	{""," пошук в:"},
	{""," search on:"},
	{""," leita í:"},
	{""," hledat v:"},
};

static const struct
{
	const char *search;
	const char *alldir;
	const char *onlyin;
} search[]=
{
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"إبحث","إبحث كل الدليل","إبحث فقط في"},
	{"Փնտռեցէք","ամբողջ ցուցակին մէջ","միայն"},
	{"Axtar","bütün kataloqu","yalnız"},
	{"Buscar","en tol directoriu","sólo en"},
	{"Cari","seluruh direktori","hanya di"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"Pretraži","čitav direktorij","samo u"},
	{"Search","the entire directory","only in"},
	{"Търси","цялата директория","само в"},
	{"Cercar","a tot el directori","només a"},
	{"搜寻","整个目录","只限于"},
	{"搜尋","整個目錄","只限於"},
	{"Search","the entire directory","only in"},
	{"Søg","alle kategorier","kun i"},
	{"Suche","im ganzen Verzeichnis","nur in"},
	{"Search","the entire directory","only in"},
	{"Buscar","en todo el directorio","sólo en"},
	{"Serĉu","en la tuta katalogo","nur en"},
	{"Bilatu","direktorio osoan","soilik hemen:"},
	{"جستجو","در همه‌ی فهرست","فقط در"},
	{"Chercher","dans tout le répertoire","uniquement dans"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"Αναζήτηση","σε ολόκληρο τον κατάλογο","μόνο στην κατηγορία"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"חיפוש","בכל המדריך","רק ב"},
	{"खोज","पूरी निर्देशिका","केवल इसमें खोजें"},
	{"Pretražiti","cijeli direktorij","samo u"},
	{"Cari","di keseluruhan direktori","hanya di"},
	{"Search","the entire directory","only in"},
	{"Cerca","in tutta la directory","solo in"},
	{"サーチ","ディレクトリ全体","この中でのみ"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"&#44160;&#49353;","&#51204;&#52404; &#46356;&#47113;&#53664;&#47532;","&#46356;&#47113;&#53664;&#47532;"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"Ieškoti","visame kataloge","tik"},
	{"Quaere","toto repertorio","solum in"},
	{"Search","the entire directory","only in"},
	{"Keresés","a teljes katalógusban","csak itt:"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"Zoek","in de gehele index","alleen in"},
	{"Søk","hele katalogen","kun i"},
	{"Cercar","en tot eth directòri","sonque en"},
	{"Szukaj","w całym ODP","tylko w"},
	{"Procurar","em todo o diretório","somente em"},
	{"Search","the entire directory","only in"},
	{"Caută","Întreg directorul","numai în"},
	{"Search","the entire directory","only in"},
	{"Поиск","весь справочник","только в"},
	{"Chirca","in su archiviu intreu","solu in"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"Vyhľadať","celý adresár","iba v"},
	{"Pretražiti","cijeli direktorij","samo u"},
	{"Hae","koko hakemistosta","vain luokasta"},
	{"Sök","i hela katalogen","endast i"},
	{"Search","the entire directory","only in"},
	{"chhoe","kui-e 目錄","kan-na"},
	{"Search","the entire directory","only in"},
	{"Search","the entire directory","only in"},
	{"Search","mottham soochika","mathramae"},
	{"ค้นหา","directory ทั้งหมด","เฉพาะใน"},
	{"Ara","bütün dizinde","sadece <b>"},
	{"Пошук","по всій Директорії","тільки в"},
	{"Search","the entire directory","only in"},
	{"Leita","í öllu safninu","aðeins í"},
	{"Hledat","v celém adresáři ODP","pouze v kategorii"},
	{NULL,NULL,NULL}
};

static const char *lastupdate[]=
{
	"Last update:",
	"Last update:",
	"آخر تحديث:",
	"Վերջին թարմացում:",
	"Son yeniləmə:",
	"Última actualización:",
	"Kemaskini terakhir:",
	"Last update:",
	"Last update:",
	"Posljednja izmjena:",
	"Last update:",
	"Последно обновяване:",
	"Darrera actualització:",
	"最後更新时间:",
	"最後更新時間:",
	"Last update:",
	"Sidst opdateret:",
	"Letzte Änderung:",
	"Last update:",
	"Última actualización:",
	"Lasta ŝanĝo:",
	"Azken eguneratzea:",
	"آخرین بروزآوری:",
	"Dernière mise à jour&nbsp;:",
	"Last update:",
	"Last update:",
	"Last update:",
	"Last update:",
	"Τελευταία ενημέρωση:",
	"Last update:",
	"Last update:",
	"עידכון אחרון:",
	"पिछला परिवर्तन:",
	"Zadnja izmjena:",
	"Perubahan terakhir:",
	"Last update:",
	"Ultimo aggiornamento:",
	"最終更新日:",
	"Last update:",
	"Last update:",
	"Last update:",
	"&#52572;&#51333; &#44081;&#49888;:",
	"Last update:",
	"Last update:",
	"Paskutinis atnaujinimas:",
	"Novissima renovatio:",
	"Last update:",
	"Utolsó frissítés:",
	"Last update:",
	"Last update:",
	"Laatste wijziging:",
	"Siste oppdatering:",
	"Darrera actualitzacion:",
	"Ostatnia zmiana:",
	"Última atualização:",
	"Last update:",
	"Ultima actualizare:",
	"Last update:",
	"Последнее обновление:",
	"Úrtimu annoamentu:",
	"Last update:",
	"Last update:",
	"Posledná zmena:",
	"Zadnja izmjena:",
	"Päivitetty viimeksi:",
	"Senast uppdaterad:",
	"Last update:",
	"siong-au 更新 e 時間:",
	"Last update:",
	"Last update:",
	"Chivari saariga aadhuneekarana:",
	"ปรับปรุงล่าสุด:",
	"Son güncelleme:",
	"Останнє обновлення:",
	"Last update:",
	"Síðast uppfært:",
	"Poslední změna:",
	NULL
};

static const char *termsofuse[]=
{
	"Terms of Use",
	"Terms of Use",
	"Terms of Use",
	"Գործածութեան Պայմաններ",
	"İstifadə şərtləri",
	"Condiciones d'usu",
	"Syarat Penggunaan",
	"Terms of Use",
	"Terms of Use",
	"Uslovi korištenja",
	"Terms of Use",
	"Условия за Употреба",
	"Condicions d'ús",
	"使用手则",
	"使用手則",
	"Terms of Use",
	"Betingelser for brug",
	"Nutzungshinweise",
	"Terms of Use",
	"Condiciones de uso",
	"Kondiĉoj de uzo",
	"Terms of Use",
	"ضوابط کاربری",
	"Conditions d'utilisation",
	"Terms of Use",
	"Terms of Use",
	"Terms of Use",
	"Terms of Use",
	"Οροι Χρήσης",
	"Terms of Use",
	"Terms of Use",
	"כללים לשימוש",
	"इस्तेमाल की शर्तें",
	"Uvjeti Korištenja",
	"Syarat Penggunaan",
	"Terms of Use",
	"Condizioni d'Uso",
	"使用条件",
	"Terms of Use",
	"Terms of Use",
	"Terms of Use",
	"사용약관",
	"Terms of Use",
	"Terms of Use",
	"Naudojimosi sąlygos",
	"Condiciones Utendi",
	"Terms of Use",
	"Használati feltételek",
	"Terms of Use",
	"Terms of Use",
	"Bepalingen voor Gebruik",
	"Betingelser for bruk",
	"Terminis d'usatge",
	"Warunki korzystania",
	"Termos de utilização",
	"Terms of Use",
	"termeni de utilizare",
	"Terms of Use",
	"Условия использования",
	"Sas condisciones pro l’impitare",
	"Terms of Use",
	"Terms of Use",
	"Podmienky použitia",
	"Uvjeti Korištenja",
	"Käyttöehdot",
	"Villkor för användning",
	"Terms of Use",
	"使用手則",
	"Terms of Use",
	"Terms of Use",
	"Vaadutaku sharatulu",
	"เงื่อนไขการใช้งาน",
	"Kullanım Yönergesi",
	"Terms of Use",
	"Terms of Use",
	"Terms of Use",
	"Podmínky použití",
	NULL
};

static const unsigned char *stopwords[STOPWORDS]=
{
	"aan","abans","abbastanza","abbiomo","aber","able","abord",
	"about","above","aby","acaba","accidenti","according",
	"accordingly","acerca","ach","acht","achte","achten","achter",
	"achtes","across","actually","acuerdo","adelante","ademas",
	"aderton","adertonde","adesso","adeus","adjo","adrede","affinche",
	"afin","after","afterwards","again","against","agli","agora",
	"ahi","ahime","ahora","aie","aiemmin","aika","aikaa","aikaan",
	"aikaisemmin","aikaisin","aikajen","aikana","aikoina","aikoo",
	"aikovat","ain","aina","ainakaan","ainakin","ainda","ainoa",
	"ainoat","ainsi","aiomme","aion","aiotte","aist","aivan","ajan",
	"ako","ala","alas","alcuna","alcuni","alcuno","aldrig","ale",
	"alem","alemmas","algmas","algo","algumas","algun","alguna",
	"algunas","algunes","alguno","algunos","alguns","ali","alkoon",
	"alkuisin","alkuun","all","alla","allaient","allas","alle",
	"allein","allem","allen","aller","allerdings","alles",
	"allgemeinen","alli","allo","allons","allora","allow","allows",
	"allt","alltid","alltsa","almost","aloitamme","aloitan","aloitat",
	"aloitatte","aloitattivat","aloitettava","aloitettevaksi",
	"aloitettu","aloitimme","aloitin","aloitit","aloititte","aloittaa",
	"aloittamatta","aloitti","aloittivat","alone","along","alors",
	"already","alrededor","als","also","alt","alta","altd",
	"although","altijd","altmds","altre","altri","altrimenti","altro",
	"altrui","aluksi","alussa","alusta","always","ama","amb",
	"ambdos","ambos","among","amongst","ampleamos","anar","anche",
	"ancora","and","anden","andere","anderen","andern","anders",
	"andet","andra","andras","andre","ani","annan","annat",
	"annettavaksi","annetteva","annettu","anni","anno","annu","ano",
	"anos","another","ans","ansa","antaa","antamatta","antano",
	"ante","antes","antoi","any","anybody","anyhow","anyone",
	"anything","anyway","anyways","anywhere","aos","aoua","apart",
	"apenas","apoio","apontar","apos","appear","appreciate",
	"appropriate","apres","aproximadamente","apu","aquel","aquela",
	"aquelas","aquele","aqueles","aquell","aquella","aquellas",
	"aquelles","aquello","aquellos","aquells","aqui","aquilo",
	"arbeid","are","area","aren","around","arriba","arribaabajo",
	"artonde","artonn","asi","asia","asiaa","asian","asiasta",
	"asiat","aside","asioiden","asioihin","asioita","ask","asking",
	"assai","assez","assim","associated","asti","ate","atminstone",
	"atras","atraves","att","atta","attendu","attesa","attio",
	"attionde","attonde","auch","aucun","aucune","aucuns","auf",
	"aujourd","aun","aunque","auquel","aura","auront","aus","auser",
	"auserdem","ausser","ausserdem","aussi","autre","autres","aux",
	"auxquelles","auxquels","avaient","available","avais","avait",
	"avant","avanti","avec","aven","avendo","avente","aver","avere",
	"avete","aveva","avevano","avoir","avsak","avuksi","avulla",
	"avun","avuta","avute","avuti","avuto","avutta","away","awfully",
	"ayant","bada","badas","bade","bae","bah","baixo","bajo",
	"bakom","bald","bana","bara","bardziej","bardzo","bast","basta",
	"bastant","bastante","battre","bazd","beaucoup","became",
	"because","become","becomes","becoming","bedzie","been","before",
	"beforehand","begge","behind","behova","behovas","behovde",
	"behovt","bei","beide","beiden","beim","being","beispiel",
	"bekannt","believe","belki","below","bem","ben","benden","bene",
	"beni","benim","benissimo","bereits","berlusconi","bes","beside",
	"besides","beslut","beslutat","beslutit","besonders","besser",
	"best","besten","better","between","beyond","bez","bien","bigre",
	"bij","bin","bir","biri","birkac","birkez","birsey","birseyi",
	"bis","bisher","bist","biz","bizden","bizi","bizim","bland",
	"ble","blei","blev","bli","blir","blitt","blive","bliver",
	"blivit","bol","bola","boli","bolo","bom","bon","bort","borta",
	"both","boum","bowiem","bra","brava","bravo","breve","brief",
	"brrr","bruke","bude","budem","budes","buna","bunda","bundan",
	"bunu","bunun","buono","but","byl","byla","byli","bylo","byly",
	"byt","cada","came","caminho","can","cannot","cant","car",
	"casa","casi","caso","catorze","cause","causes","ceci","cedo",
	"cela","celle","celles","celui","cent","cento","cependant",
	"cerca","certa","certain","certaine","certaines","certainly",
	"certains","certamente","certe","certes","certeza","certi",
	"certo","ces","cet","cette","ceux","cez","chacun","changes",
	"chaque","che","cher","chere","cheres","chers","chez","chi",
	"chicchessia","chiche","chinque","chiunque","chut","ciascuna",
	"ciascuno","cierta","ciertas","cierto","ciertos","cima","cinco",
	"cinq","cinquantaine","cinquante","cinquantieme","cinque",
	"cinquieme","cio","cioe","circa","citta","clac","clankov",
	"clanku","clanky","clanok","claro","clearly","clic","codesta",
	"codeste","codesti","codesto","cogli","coi","coisa","cok",
	"col","colei","coll","coloro","colui","com","combien","come",
	"comes","comme","comment","como","comprare","comprido","compris",
	"con","concernant","concernente","concerning","conhecido",
	"conmigo","consecutivi","consecutivo","consegueixo","conseguim",
	"conseguimos","conseguir","conselho","consequently","consider",
	"considering","consiglio","consigo","consigue","consigueix",
	"consigueixen","consigueixes","consiguen","consigues","contain",
	"containing","contains","contigo","contra","contre","contro",
	"corrente","corresponding","cortesia","cos","cosa","cosi",
	"couic","could","couldn","course","coz","crac","csak","cual",
	"cuales","cuando","cuanta","cuantas","cuanto","cuantos","cui",
	"cunku","currently","custa","czy","czyli","daar","dabei",
	"dadurch","dafur","dag","dagar","dagarna","dagegen","dagen",
	"dagli","daha","daher","dahi","dahin","dahinter","dai","dal",
	"dall","dalla","dalle","dallo","dalsi","dalt","damals","damit",
	"dan","danach","daneben","dank","dann","dans","dao","daquela",
	"daquele","dar","daran","darauf","daraus","darf","darfor",
	"darfst","darin","daruber","darum","darunter","das","dasein",
	"daselbst","dass","dasselbe","dat","davanti","davon","davor",
	"dazu","dazwischen","debaixo","debajo","debout","debut","dedans",
	"defa","definitely","deg","degli","dehors","dei","deim","dein",
	"deine","deinem","deiner","deira","deires","del","dela",
	"delante","delen","dell","della","delle","dello","dem","demais",
	"demasiado","dementsprechend","demgegenuber","demgemas","demgemass",
	"demselben","demzufolge","den","denen","denn","denne","denselben",
	"dentro","depois","deprisa","depuis","der","deras","dere",
	"deren","deres","derjenige","derjenigen","dermasen","dermassen",
	"derriere","derselbe","derselben","des","described","desde",
	"deshalb","design","desligado","desormais","despacio","despite",
	"despues","desquelles","desquels","dess","dessa","desse",
	"desselben","dessen","dessous","dessus","desta","deste",
	"deswegen","det","detras","detta","dette","detto","deux",
	"deuxieme","deuxiemement","devant","deve","devem","devera",
	"devers","devo","devra","devrait","dez","dezanove","dezasseis",
	"dezassete","deze","dezoito","dia","diante","dias","dice",
	"dich","did","didn","die","diejenige","diejenigen","dies",
	"diese","dieselbe","dieselben","diesem","diesen","dieser",
	"dieses","dietro","different","differente","differentes",
	"differents","dig","dila","din","dina","dins","dir","dire",
	"direita","dirimpetto","disse","dit","ditt","divers","diverse",
	"diverses","dix","dixieme","diye","diz","dizem","dizer","dla",
	"dlatego","dnes","doch","dock","doen","does","doesn","dog",
	"doing","dois","doit","doivent","doksan","dokuz","don","donc",
	"donde","done","dont","door","dopo","doppio","dort","dos",
	"douze","douzieme","dove","dovra","down","downwards","doze",
	"drei","drin","dring","dritte","dritten","dritter","drittes",
	"droite","duas","due","dunque","duquel","durant","durante",
	"durch","durchaus","durfen","durft","durfte","durften","during",
	"dus","duvida","dykk","dykkar","each","eben","ebenso","ecco",
	"edella","edelle","edelleen","edelta","edemmas","edes","edessa",
	"edesta","edu","een","eens","effet","efter","eftersom","egli",
	"egy","ehka","ehrlich","eigen","eigene","eigenen","eigener",
	"eigenes","eight","eika","eilen","ein","einander","eine","einem",
	"einen","einer","eines","einige","einigen","einiger","einiges",
	"einmal","eins","eit","either","eitt","eivat","ela","elas",
	"ele","eles","elf","elfte","eli","ella","ellas","elle","ellei",
	"elleivat","ellemme","ellen","eller","elles","ellet","ellette",
	"elli","ellos","ells","els","else","elsewhere","elva","email",
	"embora","emme","empleais","emplean","emplear","empleas","empleo",
	"enaa","encima","encore","end","ende","endlich","ene","enemman",
	"eneste","enfrente","enhver","eniten","enkel","enkelt","enkla",
	"enligt","enn","ennen","enough","enquanto","ens","enseguida",
	"ensi","ensimmainen","ensimmaiseksi","ensimmaisen","ensimmaisena",
	"ensimmaiset","ensimmaisia","ensimmaisiksi","ensimmaisina",
	"ensimmaista","ensin","entao","entinen","entirely","entisen",
	"entisia","entista","entisten","entonces","entre","entweder",
	"envers","environ","eppure","era","eraat","eraiden","eramos",
	"eran","erano","eras","erem","eren","eres","eri","erittain",
	"erityisesti","ernst","erst","erste","ersten","erster","erstes",
	"ert","esa","esas","ese","esi","esiin","esilla","esimerkiksi",
	"eso","esos","especially","essa","essai","essas","esse",
	"essendo","essent","esser","essere","esses","essi","est","esta",
	"estaba","estado","estados","estais","estamos","estan","estao",
	"estar","estara","estas","estat","estava","este","estem","estes",
	"esteu","esteve","estic","estive","estivemos","estiveram",
	"estiveste","estivestes","esto","estos","estou","estoy","etaient",
	"etais","etait","etant","etat","etc","ete","eteen","etenkin",
	"etions","etre","ets","ett","etta","ette","ettei","etter",
	"ettusen","etwa","etwas","euch","euer","euh","eure","eux",
	"even","ever","every","everybody","everyone","everything",
	"everywhere","exactly","example","except","excepte","excepto",
	"exemplo","faco","facon","faig","fais","faisaient","faisant",
	"fait","faites","falta","fan","fanns","far","fara","fare","fas",
	"fatt","fatto","favor","favore","faz","fazeis","fazem","fazemos",
	"fazer","fazes","fazia","fel","fem","femte","femtio","femtionde",
	"femton","femtonde","fer","feront","feu","few","fez","fick",
	"fifth","fim","fin","final","finalmente","finche","fine",
	"finnas","finns","fino","fire","first","five","fjarde","fjorton",
	"fjortonde","flac","fler","flera","flere","flesta","fleste",
	"floc","foi","fois","foljande","folk","followed","following",
	"follows","fomos","font","for","fora","foram","force","fordi",
	"fore","forlat","forma","former","formerly","forra","forrige",
	"forse","forsta","forsuke","forth","foste","fostes","four","fra",
	"fram","framfor","fran","frattanto","from","fruher","fue",
	"fuera","fueron","fui","fuimos","funf","funfte","funften",
	"funfter","funftes","fuori","fur","furst","further","furthermore",
	"fyra","fyrtio","fyrtionde","gab","galla","galler","gallt",
	"ganz","ganze","ganzen","ganzer","ganzes","gar","garna","gatt",
	"gdy","gdzie","gedurft","geen","gegen","gegenuber","gehabt",
	"gehen","geht","gekannt","gekonnt","gemacht","gemocht","gemusst",
	"genast","general","genom","gens","gente","genug","gerade",
	"geral","gern","gesagt","geschweige","get","gets","getting",
	"geweest","gewesen","gewollt","geworden","gia","giacche","gibi",
	"gibt","gick","ging","giorni","giorno","giu","given","gives",
	"gjorde","gjort","gjure","gleich","gli","gliela","gliele",
	"glieli","glielo","gliene","god","goda","godare","godast","goes",
	"going","gone","gor","gora","got","gott","gotten","governo",
	"gran","grande","grandes","grazie","greetings","gros","grose",
	"grosen","groser","groses","gross","grosse","grossen","grosser",
	"grosses","grupo","gruppo","gueno","gut","gute","guter","gutes",
	"haar","habe","haben","habia","habla","hablan","habt","hace",
	"haceis","hacemos","hacen","hacer","haces","hacia","had","hadde",
	"hade","hadn","haft","hago","hai","halua","haluaa","haluamatta",
	"haluamme","haluan","haluat","haluatte","haluavat","halunnut",
	"halusi","halusimme","halusin","halusit","halusitte","halusivat",
	"halutessa","haluton","ham","han","haneen","hanella","hanelle",
	"hanelta","hanen","hanessa","hanesta","hanet","hanno","hans",
	"happens","har","hardly","has","hasn","hast","hasta","hat",
	"hatte","hatten","hattest","hattet","haut","havde","have",
	"haven","haver","having","hay","heb","hebben","heeft","hei",
	"heidan","heihin","heille","heilta","hein","heissa","heisst",
	"heista","heita","helas","heller","hello","hellre","help",
	"helposti","helst","helt","hem","hence","hende","hendes",
	"hennar","henne","hennes","hep","hepsi","her","here","hereafter",
	"hereby","herein","hereupon","hers","herself","het","heti",
	"hetkella","heute","hic","hieman","hier","hij","him","himself",
	"hin","hinter","his","hit","hither","hja","hoch","hoe","hog",
	"hoger","hogre","hogst","hogy","hoje","hola","hon","honom",
	"hop","hopefully","horas","hormis","hors","hos","hoss","hossen",
	"hou","houp","how","howbeit","however","hoy","hue","hui","huit",
	"huitieme","hum","hun","hundra","hundraen","hundraett",
	"huolimatta","huomenna","hur","hurrah","hva","hvad","hvem",
	"hver","hvilke","hvilken","hvis","hvor","hvordan","hvorfor",
	"hvornar","hyva","hyvaa","hyvat","hyvia","hyvien","hyviin",
	"hyviksi","hyville","hyvilta","hyvin","hyvina","hyvissa",
	"hyvista","ibland","ich","ici","icin","idag","ide","iemand",
	"ieri","iets","igar","igen","ignored","ihan","ihm","ihn",
	"ihnen","ihr","ihre","ihrem","ihren","ihrer","ihres","iki",
	"ikke","ikkje","ile","ilman","ilmeisesti","ils","immediate",
	"immer","imorgon","importe","improvviso","inasmuch","inc",
	"inclos","incluso","ind","indeed","indem","indicate","indicated",
	"indicates","indietro","ine","infatti","infolgedessen","infor",
	"informo","inga","ingen","ingenting","inget","ingi","iniciar",
	"inicio","inkje","inn","innan","inne","innen","inner","inni",
	"innych","inom","ins","insermi","insieme","insofar","instead",
	"intanto","inte","intenta","intentais","intentamos","intentan",
	"intentar","intentas","intento","intet","into","intorno","inuti",
	"invece","invere","inward","ira","irgend","ise","isn","isso",
	"ist","ista","iste","isto","its","itse","itseaan","itself",
	"itsensa","jaa","jag","jahr","jahre","jahren","jak","jako",
	"jalkeen","jalleen","jamfort","jede","jedem","jeden","jeder",
	"jedermann","jedermanns","jedes","jednak","jedoch","jeg","jego",
	"jeho","jej","jeji","jejich","jemand","jemandem","jemanden",
	"jen","jene","jenem","jenen","jener","jenes","jer","jeres",
	"jesli","jest","jeste","jeszcze","jetzt","jine","jiz","johon",
	"joiden","joihin","joiksi","joilla","joille","joilta","joissa",
	"joista","joita","joka","jokainen","jokin","joko","joku","jolla",
	"jolle","jolloin","jolta","jompikumpi","jonka","jonkin","jonne",
	"joo","jopa","jos","joskus","jossa","josta","jota","jotain",
	"joten","jotenkin","jotenkuten","jotka","jotta","jouduimme",
	"jouduin","jouduit","jouduitte","joudumme","joudun","joudutte",
	"joukkoon","joukossa","joukosta","joutua","joutui","joutuivat",
	"joutumaan","joutuu","joutuvat","jsem","jses","jsme","jsou",
	"jste","junto","jusqu","jusque","just","juste","juuri","juz",
	"kahdeksan","kahdeksannen","kahdella","kahdelle","kahdelta",
	"kahden","kahdessa","kahdesta","kahta","kahteen","kai","kaiken",
	"kaikille","kaikilta","kaikkea","kaikki","kaikkia","kaikkiaan",
	"kaikkialla","kaikkialle","kaikkialta","kaikkien","kaikkin",
	"kaksi","kam","kan","kann","kannalta","kannattaa","kannst",
	"kanske","kanssa","kanssaan","kanssamme","kanssani","kanssanne",
	"kanssasi","katrilyon","kauan","kauemmas","kaum","kautta","kde",
	"kdo","kdrk","kdyz","ked","keep","keeps","kehen","keiden",
	"keihin","keiksi","keilla","keille","keilta","kein","keina",
	"keine","keinem","keinen","keiner","keissa","keista","keita",
	"keitta","keitten","keneen","keneksi","kenella","kenelle",
	"kenelta","kenen","kenena","kenessa","kenesta","kenet","kenetta",
	"kennessasta","kept","kerran","kerta","kertaa","kesken",
	"keskimaarin","keta","ketka","kez","kiedy","kiitos","kilka",
	"kim","kimden","kime","kimi","kleine","kleinen","kleiner",
	"kleines","knappast","know","known","knows","kohti","koko",
	"kokonaan","kolmas","kolme","kolmen","kolmesti","kom","komma",
	"kommen","kommer","kommit","kommt","kon","konnen","konnt",
	"konnte","konnten","korleis","korso","koska","koskaan","kovin",
	"ktera","ktere","kteri","kterou","ktery","kto","ktora","ktore",
	"ktorego","ktorej","ktori","ktorou","ktory","ktorych","ktorym",
	"ktorzy","kuin","kuinka","kuitenkaan","kuitenkin","kuka","kukaan",
	"kukin","kumpainen","kumpainenkaan","kumpi","kumpikaan","kumpikin",
	"kun","kunde","kunna","kunnat","kunne","kunnen","kurz","kuten",
	"kuuden","kuusi","kuutta","kva","kvar","kvarhelst","kven","kvi",
	"kvifor","kylla","kymmenen","kyse","lado","lage","lahekkain",
	"lahella","lahelle","lahelta","lahemmas","lahes","lahinna",
	"lahtien","lang","lange","langre","langsam","langsammare",
	"langsammast","langsamt","langst","langt","lapi","laquelle",
	"largo","las","last","lately","later","latt","lattare","lattast",
	"latter","latterly","lav","lavoro","least","lebo","legat","lei",
	"leicht","leide","lejos","len","lequel","les","lesquelles",
	"lesquels","less","lest","lesz","let","leur","leurs","lidt",
	"lieber","ligado","ligga","ligger","liian","lik","lika","like",
	"liked","likely","liki","likstalld","likstallda","lilla","lille",
	"lisaa","lisaksi","lite","liten","litet","little","llarg",
	"llavors","local","logo","longe","longtemps","lontano","look",
	"looking","looks","loro","lorsque","los","ltd","lub","luego",
	"lugar","lui","lungo","luo","maar","macche","machen","macht",
	"machte","mag","magari","magst","mahdollisimman","mahdollista",
	"mahn","mai","mainly","maint","maintenant","maior","maioria",
	"maiorias","mais","makt","mal","male","malgrado","malgre",
	"malissimo","man","manche","manchem","manchen","mancher",
	"manches","manga","mange","mann","many","mas","maste","mat",
	"mate","maximo","may","maybe","mayor","mean","meanwhile","med",
	"medan","medesimo","mediante","medio","medzi","meer","meg",
	"meget","meglio","mehr","meidan","meilla","meille","mein",
	"meine","meinem","meinen","meiner","meines","meio","mejor",
	"melkein","melko","mellan","mellom","meme","memes","men","menee",
	"meneet","menemme","menen","menet","menette","menevat","meni",
	"menimme","menin","menit","menivat","mennessa","mennyt","meno",
	"menor","menos","menossa","mens","mensch","menschen","mentre",
	"menudo","mer","mera","merci","mere","merely","mes","meses",
	"mesi","mesmo","mest","met","meu","meus","mezi","mezzo","mia",
	"mias","mich","mie","miedzy","miei","mien","mienne","miennes",
	"miens","mientras","mieri","mig","might","mihin","mij","mijn",
	"mika","mikaan","mikali","mikin","miksi","mil","mila","miliardi",
	"milioni","mille","milloin","milyar","milyon","min","mina",
	"mince","mindre","mine","minha","minhas","ministro","minne",
	"minst","mint","minun","minut","mio","mios","mir","mis","mismo",
	"missa","mista","mit","mita","mitaan","miten","mitt","mittel",
	"mittemot","mnie","mochte","mochten","mod","mode","modo","moet",
	"moga","mogen","moglich","mogt","moi","moins","moj","mojlig",
	"mojligen","mojligt","mojligtvis","molemmat","molt","molta",
	"molti","moltissimo","molto","molts","momento","mon","mondo",
	"mones","monesti","monet","moni","moniaalla","moniaalle",
	"moniaalta","monta","more","moreover","morgen","most","mostly",
	"mot","moyennant","moze","mozna","muassa","much","mucho",
	"muchos","muiden","muita","muito","muitos","muj","muka","mukaan",
	"mukaansa","mukana","mus","muss","mussen","musst","musste",
	"mussten","must","mutta","muu","muualla","muualle","muualta",
	"muuanne","muulloin","muun","muut","muuta","muutama","muutaman",
	"muuten","muy","muze","mycket","mye","mykje","myohemmin","myos",
	"myoskaan","myoskin","myota","myself","naar","nach","nachdem",
	"nad","nada","nadie","nagon","nagonting","nagot","nagra","nahm",
	"naiden","nain","naissa","naissahin","naissalle","naissalta",
	"naissasta","naita","nam","nama","name","namely","nao","napiste",
	"naquela","naquele","nar","nas","nasdl","nasi","nasta","naste",
	"nasten","nasu","naszego","naszych","naturlich","navn","nawet",
	"nazionale","neanmoins","near","nearly","neben","nebo",
	"necessary","ned","neden","nederst","nedersta","nedre","need",
	"needs","negli","nei","nein","neither","nej","nejsou","nel",
	"nelja","neljaa","neljan","nell","nella","nelle","nello","nem",
	"nemmeno","nenhuma","neni","neppure","ner","nerde","nerede",
	"nereye","nessa","nesse","nessuna","nessuno","nesta","neste",
	"neue","neuen","neuf","neun","neunte","neunten","neunter",
	"neuntes","neuvieme","never","nevertheless","new","next","nez",
	"nic","nich","nicht","nichts","nicin","nie","niemand",
	"niemandem","niemanden","niente","niet","niets","niiden","niin",
	"niista","niita","nim","nine","ninguna","nio","nionde","nittio",
	"nittionde","nitton","nittonde","nivel","niye","niz","nobody",
	"noch","nodvandig","nodvandiga","nodvandigt","nodvandigtvis","noe",
	"noen","nog","nogen","noget","nogle","noi","noin","noite",
	"noka","noko","nokon","nokor","nokre","noll","nombreuses",
	"nombreux","nome","nommes","non","nondimeno","nondimento","none",
	"noone","nopeammin","nopeasti","nopeiten","nor","normally","nos",
	"nosaltres","nosotras","nosotros","nossa","nossas","nosso",
	"nossos","nostra","nostre","nostri","nostro","not","nothing",
	"notre","notres","nous","nouveaux","nova","nove","novel","novo",
	"novos","novy","now","nowhere","nro","nuestra","nuestras",
	"nuestro","nuestros","nueva","nuevo","nul","nulla","num","numa",
	"numero","nummer","nun","nunca","nuo","nuovi","nuovo","nur",
	"nyt","oben","obra","obrigada","obrigado","obviously","och",
	"ocksa","oda","oder","off","offen","oft","ofta","oftast",
	"often","oggi","ogni","ognuna","ognuno","ogsa","ohe","ohi",
	"ohne","oikein","oitava","oitavo","oito","okay","old","ole",
	"olemme","olen","olet","olette","oleva","olevan","olevat","oli",
	"olika","olikt","olimme","olin","olisi","olisimme","olisin",
	"olisit","olisitte","olisivat","olit","olitte","olivat","olla",
	"olle","olleet","olli","ollut","oltre","oma","omaa","omaan",
	"omaksi","omalle","omalta","oman","omassa","omat","omdat","omia",
	"omien","omiin","omiksi","omille","omilta","omissa","omista",
	"ona","once","ondan","onde","onder","one","ones","onkin","onko",
	"onlar","onlardan","onlardn","onlari","only","ons","ont","ontem",
	"onto","onu","onze","onzieme","ook","opp","oppure","ora","oraz",
	"ordnung","ore","osi","oss","ossia","ossze","other","others",
	"otherwise","otra","otro","otros","otte","otto","otuz","ouf",
	"ought","ouias","our","ours","ourselves","oust","ouste","out",
	"outra","outras","outre","outro","outros","outside","ovat",
	"over","overall","overmorgon","overst","ovre","own","paalle",
	"paese","paf","paikoittain","pais","paitsi","pak","pakosti",
	"paljon","pan","par","para","parce","parecchi","parecchie",
	"parecchio","parece","paremmin","parempi","parhaillaan",
	"parhaiten","parmi","parole","part","partant","parte","partendo",
	"particular","particularly","particulier","particuliere",
	"particulierement","partir","pas","pasado","passe","peccato",
	"pegar","peggio","pela","pelas","pelo","pelos","pendant","peor",
	"per","perati","perche","percio","perfino","perhaps","pero",
	"perque","persone","personne","personnes","perto","perusteella",
	"pessoas","peu","peut","peuvent","peux","pff","pfft","pfut",
	"pian","piece","piedi","pieneen","pieneksi","pienella","pienelle",
	"pienelta","pienempi","pienesta","pieni","pienin","pieno","pif",
	"piglia","piu","placed","please","plein","plouf","plupart",
	"plus","plusieurs","plutot","pochissimo","poco","pod","pode",
	"podeis","podem","podemos","poden","poder","podera","podeu",
	"podia","podla","podle","podria","podriais","podriamos","podrian",
	"podrias","poe","poem","poi","poiche","pokial","pokud","ponto",
	"pontos","por","porque","posicao","possible","possivel",
	"possivelmente","posso","potom","potser","pouah","pouca","pouco",
	"pour","pourquoi","pouze","povo","poza","prave","pre","preco",
	"pred","premier","premiere","premierement","pres","press",
	"presumably","preto","pretoze","pri","prima","primeira",
	"primeiro","primer","primero","primo","pro","probably","proc",
	"proche","promeiro","promesso","pronto","proprio","proto",
	"protoze","provides","proximo","prvi","prvni","prvy","przed",
	"przede","przez","przy","psitt","pta","puc","puderam","puede",
	"pueden","puedo","puisque","punkt","puo","puolesta","puolestaan",
	"pure","purtroppo","pyta","qeu","qua","qual","qualche",
	"qualcuna","qualcuno","quale","quali","qualquer","qualsiani",
	"qualunque","quan","quand","quando","quant","quanta","quante",
	"quanti","quanto","quantunque","quarante","quarta","quarto",
	"quasi","quatorze","quatre","quatrieme","quatriemement","quatro",
	"quattro","que","quel","quelconque","quella","quelle","quelles",
	"quelli","quello","quelqu","quelque","quelques","quels","quem",
	"quer","quero","quest","questa","questao","queste","questi",
	"questo","qui","quiconque","quien","quienes","quieto","quindi",
	"quinta","quinto","quinze","quite","quiza","quizas","quoi",
	"quoique","rakt","raras","rather","ratt","really","reasonably",
	"recht","rechte","rechten","rechter","rechtes","redan","reeds",
	"regarding","regardless","regards","relacao","relatively",
	"repente","respectively","rett","revoici","revoila","richtig",
	"riecco","rien","right","riktig","rispetto","rowniez","rund",
	"runsaasti","saakka","sabe","sabeis","sabem","sabemos","saben",
	"saber","sabes","sabeu","sache","sacrebleu","sadam","sadan",
	"sade","saga","sager","sagt","sagte","sah","said","saltro",
	"salvo","sama","samaa","samaan","samalla","samallalta",
	"samallassa","samallasta","saman","samat","same","samma","samme",
	"samoin","samre","samst","sanki","sann","sans","sant","sao",
	"sap","sapristi","saps","sara","sarebbe","sata","sataa",
	"satojen","satt","sauf","saw","say","saying","says","schlecht",
	"schluss","schon","scopo","scorso","sechs","sechste","sechsten",
	"sechster","sechstes","second","secondly","secondo","sedan","see",
	"seeing","seem","seemed","seeming","seems","seen","seg",
	"seguente","segun","segunda","segundo","sehr","sei","seid",
	"seien","sein","seine","seinem","seinen","seiner","seines",
	"seis","seit","seitdem","seitseman","seize","seka","sekiz",
	"seks","seksen","selbst","self","selon","selv","selves","sem",
	"sembra","sembrava","sempre","sen","senare","senast","senden",
	"seni","senin","sense","sensible","sent","senza","sept",
	"septieme","ser","sera","seria","serious","seriously","seront",
	"ses","sete","setima","setimo","sette","seu","seulement",
	"seuraavat","seus","seven","several","sex","sexta","sextio",
	"sextionde","sexto","sexton","sextonde","sey","seyden","seyi",
	"seyler","shall","she","should","shouldn","sia","siamo","sich",
	"sidan","siden","sido","sie","sieben","siebente","siebenten",
	"siebenter","siebentes","siella","sielta","siempre","sien",
	"siendo","sienne","siennes","siens","siete","sig","siihen",
	"siina","siis","siita","sijaan","siksi","silla","silloin",
	"silti","sim","sin","sina","since","sind","sine","sinne",
	"sinon","sinua","sinulle","sinulta","sinun","sinussa","sinusta",
	"sinut","sisakkain","sisalla","sist","sista","siste","sistema",
	"sit","sita","siten","sitt","sitten","six","sixieme","siz",
	"sizden","sizi","sizin","sjatte","sjol","sju","sjunde","sjuttio",
	"sjuttionde","sjutton","sjuttonde","ska","skal","skall","skulle",
	"slik","slutligen","slutt","sma","smatt","sme","snart","sob",
	"sobie","sobre","soc","soi","sois","soit","soixante","solament",
	"solamente","solang","solche","solchem","solchen","solcher",
	"solches","solito","soll","sollen","sollst","sollt","sollte",
	"sollten","solo","sols","som","some","somebody","somehow",
	"somente","someone","something","sometime","sometimes","somewhat",
	"somewhere","somme","somos","somt","son","sondern","sono",
	"sonst","sont","soon","soppra","sopra","soprattutto","sorry",
	"sota","sotto","sou","sous","soweit","sowie","soy","soyez",
	"soyos","spat","spater","specified","specify","specifying",
	"spravy","sta","staranno","start","stata","state","stati",
	"stato","statt","ste","stesso","still","stille","stop","stor",
	"stora","store","storre","storst","stort","strana","stresso",
	"sua","suas","sub","subito","successivo","such","sue","sugli",
	"sui","suis","suivant","sujet","sul","sull","sulla","sulle",
	"sullo","suna","sunda","sundan","sunu","suo","suoi","suoraan",
	"sup","supuesto","sur","sure","surtout","sus","suuntaan",
	"suuren","suuret","suuri","suuria","suurin","suurten","suya",
	"suyas","suyo","sve","svoj","svoje","svojich","svojimi","svych",
	"svym","svymi","swoje","syv","szet","taa","taalla","taalta",
	"taas","tac","tack","taemmas","tag","tage","tagen","tahan",
	"tahansa","tai","tak","takaa","takaisin","takana","take","taken",
	"takia","takie","takze","tal","tale","talla","talloin","talvez",
	"talvolta","tam","tama","taman","tambe","tambem","tambien",
	"tampoco","tana","tanaan","tandis","tanne","tant","tanto","tao",
	"tapauksessa","tarde","tassa","tasta","tat","tata","taten",
	"tato","tavalla","tavoitteena","taysin","taytyvat","taytyy",
	"teda","tedy","tegen","tego","teil","tej","tejto","tel","tell",
	"telle","tellement","telles","tels","tem","tema","temos","tempo",
	"temprano","ten","tenant","tendes","tends","tene","teneis",
	"tenemos","tener","tengo","tenho","tenim","tenir","teniu","tens",
	"tentar","tentaram","tente","tentei","tento","ter","terceira",
	"terceiro","terzo","tes","teto","teu","teus","teve","tez",
	"than","thank","thanks","thanx","that","thats","the","their",
	"theirs","them","themselves","then","thence","there","thereafter",
	"thereby","therefore","therein","theres","thereupon","these",
	"they","thi","think","third","this","thorough","thoroughly",
	"those","though","three","through","throughout","thru","thus",
	"tic","tid","tidig","tidigare","tidigast","tidigt","tiempo",
	"tien","tiene","tienen","tienne","tiennes","tiens","tieto",
	"tietysti","tiez","til","tilbake","till","tills","tillsammans",
	"tilstand","tim","timto","tinc","tio","tionde","tipo","tipy",
	"tive","tivemos","tiveram","tiveste","tivestes","tjugo","tjugoen",
	"tjugoett","tjugonde","tjugotre","tjugotva","tjungo","toc","toch",
	"toda","todas","todavia","todella","todo","todos","toen",
	"together","tohle","toho","tohoto","toi","toinen","toisaalla",
	"toisaalle","toisaalta","toiseen","toiseksi","toisella","toiselle",
	"toiselta","toisemme","toisen","toisensa","toisessa","toisesta",
	"toista","toistaiseksi","toki","tolfte","tolv","tom","tomto",
	"tomuto","ton","too","took","torino","tosin","tot","toto",
	"touchant","toujours","tous","tout","toute","toutes","toward",
	"towards","tra","trabaja","trabajais","trabajamos","trabajan",
	"trabajar","trabajas","trabajo","trabalhar","trabalho","tranne",
	"trannefino","tras","tre","tredje","treize","trente","tres",
	"trettio","trettionde","tretton","trettonde","treze","tried",
	"tries","trilyon","triplo","tritt","trois","troisieme",
	"troisiemement","trop","troppo","trotzdem","truly","try","trying",
	"tsoin","tsouin","tua","tuas","tudo","tue","tuhannen","tuhat",
	"tule","tulee","tulemme","tulen","tulet","tulette","tulevat",
	"tulimme","tulin","tulisi","tulisimme","tulisin","tulisit",
	"tulisitte","tulisivat","tulit","tulitte","tulivat","tulla",
	"tulleet","tullut","tum","tun","tuntuu","tuo","tuoi","tuolla",
	"tuolloin","tuolta","tuonne","tus","tuskin","tuto","tutta",
	"tuttavia","tutte","tutti","tutto","tuya","tuyas","tuyo","tuyos",
	"tva","tvahundra","twice","two","tych","tyko","tylko","tym",
	"tymito","tymto","tyto","uber","uberhaupt","ubrigens","uden",
	"uguali","uhr","uit","ultim","ultimo","uma","umas","una","unas",
	"und","under","une","unes","unfortunately","unless","unlikely",
	"uno","unos","uns","unser","unsere","unserer","unter","until",
	"unto","uomo","uori","upon","upp","ursakt","usa","usais",
	"usamos","usan","usar","usas","use","usea","useasti","used",
	"useful","useimmiten","usein","useita","uses","using","uso",
	"usted","ustedes","usually","utan","utanfor","ute","uten","uucp",
	"uudeksi","uudelleen","uuden","uudet","uusi","uusia","uusien",
	"uusinta","uuteen","uutta","vaan","vad","vagy","vahan",
	"vahemman","vahintaan","vahiten","vai","vaig","vaiheessa",
	"vaikea","vaikean","vaikeat","vaikeilla","vaikeille","vaikeilta",
	"vaikeissa","vaikeista","vaikka","vain","vais","vale","valeur",
	"valilla","valor","value","vam","vamos","van","vanster",
	"vanstra","vao","var","vara","vare","varet","varfor","varia",
	"varie","varifran","vario","varios","various","varit","varken",
	"varmasti","varre","varsagod","varsin","varsinkin","vart","varte",
	"varten","vas","vase","vasta","vastaan","vastakkain","vaya",
	"veces","ved","veel","veja","vem","vems","vens","ver","verdad",
	"verdade","verdadeiro","verdadera","verdadero","verdi","vere",
	"vergangenen","verkligen","verran","vers","verso","verte","very",
	"veya","vez","vezes","via","viac","viagem","vice","vicino",
	"vid","vidare","viel","viela","viele","vielem","vielen",
	"vielleicht","vier","vierekkain","vieri","vierte","vierten",
	"vierter","viertes","vif","vifs","viiden","viime","viimeinen",
	"viimeisen","viimeksi","viisi","viktig","viktigare","viktigast",
	"viktigt","vil","vilka","vilken","vilket","vill","ville","vindo",
	"vingt","vinte","vise","vissza","visto","vita","vite","vivat",
	"vive","vives","viz","vlan","voce","voces","voi","voici",
	"voidaan","voie","voient","voila","voimme","voin","voisi","voit",
	"voitte","voivat","volt","volta","volte","vom","von","vont",
	"voor","vor","vore","vors","vort","vos","vosaltres","vosotras",
	"vosotros","vossa","vossas","vosso","vossos","vostra","vostre",
	"vostri","vostro","votre","votres","vous","voy","vsak","vuestra",
	"vuestras","vuestro","vuestros","vuoden","vuoksi","vuosi",
	"vuosien","vuosina","vuotta","wahr","wahrend","wahrenddem",
	"wahrenddessen","wann","want","wants","war","ware","waren",
	"wart","warum","was","wasn","wat","way","wegen","weil","weit",
	"weiter","weitere","weiteren","weiteres","wel","welche","welchem",
	"welchen","welcher","welches","welcome","well","wem","wen",
	"wenig","wenige","weniger","weniges","wenigstens","wenn","went",
	"wer","werd","werde","werden","werdet","were","weren","weshalb",
	"wessen","wezen","what","whatever","when","whence","whenever",
	"where","whereafter","whereas","whereby","wherein","whereupon",
	"wherever","whether","which","while","whither","who","whoever",
	"whole","whom","whose","why","wie","wiec","wieder","wiele",
	"wielu","wieso","wij","wil","will","willing","willst","wir",
	"wird","wirklich","wirst","wish","with","within","without",
	"wlasnie","woher","wohin","wohl","wollen","wollt","wollte",
	"wollten","won","wonder","worden","wordt","would","wouldn",
	"wszystkich","wszystkim","wszystko","wurde","wurden","yani",
	"yedi","yes","yet","yetmis","yha","yhdeksan","yhden","yhdessa",
	"yhta","yhtaalla","yhtaalle","yhtaalta","yhtaan","yhteen",
	"yhteensa","yhteydessa","yhteyteen","yirmi","yksi","yksin",
	"yksittain","yleensa","ylemmas","yli","ylos","ympari","you",
	"your","yours","yourself","yourselves","yuz","zal","zawsze",
	"zda","zde","zehn","zehnte","zehnten","zehnter","zehntes","zei",
	"zeit","zelf","zero","zich","zij","zijn","zonder","zou","zpet",
	"zpravy","zuerst","zugleich","zum","zunachst","zur","zuruck",
	"zusammen","zut","zwanzig","zwar","zwei","zweite","zweiten",
	"zweiter","zweites","zwischen","zwolf","але","алло",
	"без","близко","более","больше","будем",
	"будет","будете","будешь","будто","буду",
	"будут","будь","бывает","бывь","был",
	"была","были","было","быть","важная",
	"важное","важные","важный","вам","вами",
	"вас","ваш","ваша","ваше","ваши","вверх",
	"вдали","вдруг","ведь","везде","весь",
	"вниз","внизу","вокруг","вон","вона",
	"вони","воно","восемнадцатый",
	"восемнадцать","восемь","восьмой","вот",
	"впрочем","времени","время","все",
	"всегда","всего","всем","всеми","всему",
	"всех","всею","всю","всюду","вся","всё",
	"второй","він","в╡д","где","говорил",
	"говорит","год","года","году","давно",
	"даже","далеко","дальше","даром","два",
	"двадцатый","двадцать","две",
	"двенадцатый","двенадцать","двух",
	"девятнадцатый","девятнадцать",
	"девятый","девять","действительно",
	"день","десятый","десять","для",
	"довольно","долго","должно","другая",
	"другие","других","друго","другое",
	"другой","его","ему","если","есть","еще",
	"ещё","занят","занята","занято","заняты",
	"затем","зато","зачем","здесь","значит",
	"или","именно","иметь","ими","имя",
	"иногда","каждая","каждое","каждые",
	"каждый","кажется","как","какая","какой",
	"кем","когда","кого","коли","ком","кому",
	"конечно","которая","которого",
	"которой","которые","который","которых",
	"кроме","кругом","кто","куда","лет",
	"либо","лишь","лучше","люди","мало",
	"между","менее","меньше","меня",
	"миллионов","мимо","мне","много",
	"многочисленная","многочисленное",
	"многочисленные","многочисленный",
	"мной","мною","мог","могут","может",
	"можно","мои","мой","мор","мочь","моя",
	"моё","наверху","над","надо","назад",
	"наиболее","наконец","нам","нами","нас",
	"наш","наша","наше","наши","него",
	"недавно","недалеко","нее","ней",
	"нельзя","нем","немного","нему",
	"непрерывно","нередко","несколько",
	"нет","нею","неё","нибудь","ниже","низко",
	"никогда","никуда","ними","них","ничего",
	"нужно","оба","обычно","один",
	"одиннадцатый","одиннадцать","однажды",
	"однако","одного","одной","около","она",
	"они","оно","опять","особенно",
	"отовсюду","отсюда","очень","первый",
	"перед","под","пожалуйста","позже",
	"пока","пор","пора","после","посреди",
	"потом","потому","почему","почти",
	"прекрасно","при","про","просто",
	"против","процент","процентов",
	"пятнадцатый","пятнадцать","пятый",
	"пять","раз","разве","рано","раньше",
	"рядом","сам","сама","сами","самим",
	"самими","самих","само","самого","самой",
	"самом","самому","саму","своего","своей",
	"свои","своих","свою","своё","себе",
	"себя","сегодня","седьмой","сейчас",
	"семнадцатый","семнадцать","семь","сих",
	"сказал","сказала","сказать","сколько",
	"слишком","сначала","снова","собой",
	"собою","совсем","спасибо","стал","суть",
	"так","такая","также","такие","такое",
	"такой","там","твой","твоя","твоё","тебе",
	"тебя","тем","теми","теперь","тех",
	"тобой","тобою","тогда","того","тоже",
	"той","только","том","тому","тот","тою",
	"третий","три","тринадцатый",
	"тринадцать","туда","тут","тысяч",
	"тысяча","уже","уметь","хорошо","хотеть",
	"хоть","хотя","хоча","хочешь","цей",
	"часто","чаще","чего","чей","человек",
	"чем","чему","через","четвертый",
	"четыре","четырнадцатый",
	"четырнадцать","чого","что","чтоб",
	"чтобы","чуть","чье","чья",
	"шестнадцатый","шестнадцать","шестой",
	"шесть","эта","эти","этим","этими","этих",
	"это","этого","этой","этом","этому",
	"этот","эту","яко╞","інших","ابو","اتفاق",
	"اثر","اجتماع","اجل","احد","اخرى","اذا",
	"اربعة","اسرائيل","اطار","اطلاق","اعادة",
	"اعلن","اعلنت","اكتوبر","اكثر","اكد",
	"الا","الاتحاد","الاتفاق","الاثنين",
	"الاحد","الاخيرة","الاربعاء","الاردن",
	"الاسبوع","الاسرائيلي","الاسرائيلية",
	"الاسلامية","الاسلحة","الامم","الامن",
	"الاميركي","الاميركية","الان","الانباء",
	"الانتخابات","الاوروبي","الاوسط",
	"الاول","الاولى","البلاد","البوسنة",
	"التحرير","التعاون","التى","التي",
	"الثاني","الثانية","الثلاثاء","الثلثاء",
	"الجزائر","الجمعة","الجيش","الحدود",
	"الحرب","الحكم","الحكومة","الخارجية",
	"الخليج","الخميس","الدفاع","الدور",
	"الدول","الدولة","الدولي","الدولية",
	"الذاتي","الذى","الذي","الذين","الرئيس",
	"الروسي","الروسية","السابق","الساعة",
	"السبت","السعودية","السلام","السلطات",
	"السلطة","الشرطة","الشرق","الشيخ",
	"الضفة","العاصمة","العالم","العام",
	"العراق","العراقي","العراقية","العربية",
	"العسكرية","العلاقات","العمل","الغربية",
	"الف","الفرنسي","الفرنسية","الفلسطيني",
	"الفلسطينية","الفلسطينيين","القاهرة",
	"القدس","القدم","القوات","الكويت",
	"اللجنة","الله","الماضي","المانيا",
	"المباراة","المتحدة","المتحدث","المجلس",
	"المجموعة","المحتلة","المدينة",
	"المرحلة","المصدر","المصري","المصرية",
	"المفاوضات","المقبل","المقرر","الملك",
	"المنطقة","النار","النهائي","الوزراء",
	"الوضع","الوطنية","الوقت","الولايات",
	"الى","اليمن","اليوم","اما","امام","امس",
	"انه","انها","اول","ايار","ايام","ايران",
	"ايضا","ايلول","باريس","باسم","بان","برس",
	"بسبب","بشكل","بطولة","بعد","بعض","بغداد",
	"بها","بيان","بيروت","بيريز","بين",
	"تشرين","تموز","ثلاثة","جديدة","جميع",
	"جنوب","جهة","حاليا","حتى","حركة","حزب",
	"حزيران","حسين","حماس","حوالى","حول",
	"حيث","حين","خلال","دمشق","دورة","دول",
	"دولار","دولة","دون","ذكرت","ذلك","رئيس",
	"رابين","روسيا","زيارة","ساراييفو",
	"سبتمبر","سنة","سنوات","سوريا","شخصا",
	"شرق","شمال","صباح","صحيفة","صرب","صفر",
	"ضمن","عام","عاما","عبد","عدة","عدد","عدم",
	"عرفات","عشر","عشرة","على","علي","عليه",
	"عليها","عمان","عملية","عند","عندما",
	"غدا","غزة","غير","فاز","فان","فرانس",
	"فرنسا","فيه","فيها","قال","قبل","قتل",
	"قرار","قطاع","قوات","قوة","كأس","كان",
	"كانت","كانون","كرة","كريستوفر","كلم",
	"كلينتون","كما","لبنان","لدى","لقاء",
	"لكرة","لكن","للامم","لندن","لها",
	"لوكالة","مايو","مباراة","مبارك","مجلس",
	"مجموعة","محمد","مدينة","مساء","مصادر",
	"مصدر","مصر","مقابل","مقتل","مليار",
	"مليون","منذ","منطقة","منظمة","منها",
	"موسكو","نحو","نفسه","نقطة","نهاية",
	"نيويورك","هذا","هذه","هناك","واحد",
	"واشنطن","واضاف","واضافت","واكد","وان",
	"واوضح","وزارة","وزير","وفي","وقال",
	"وقالت","وقد","وقف","وكالة","وكان",
	"وكانت","ولا","ولم","ومن","وهو","وهي",
	"ياسر","يذكر","يكون","يمكن","يوليو",
	"يوم","يونيو"
};

static const struct
{
	const int len;
	const char *entity;
	const char *utf8;
} deent[ENTITIES]=
{
	{7,"&AElig;","\xc3\x86"},
	{8,"&Aacute;","\xc3\x81"},
	{7,"&Acirc;","\xc3\x82"},
	{8,"&Agrave;","\xc3\x80"},
	{7,"&Alpha;","\xce\x91"},
	{7,"&Aring;","\xc3\x85"},
	{8,"&Atilde;","\xc3\x83"},
	{6,"&Auml;","\xc3\x84"},
	{6,"&Beta;","\xce\x92"},
	{8,"&Ccedil;","\xc3\x87"},
	{5,"&Chi;","\xce\xa7"},
	{8,"&Dagger;","\xe2\x80\xa1"},
	{7,"&Delta;","\xce\x94"},
	{5,"&ETH;","\xc3\x90"},
	{8,"&Eacute;","\xc3\x89"},
	{7,"&Ecirc;","\xc3\x8a"},
	{8,"&Egrave;","\xc3\x88"},
	{9,"&Epsilon;","\xce\x95"},
	{5,"&Eta;","\xce\x97"},
	{6,"&Euml;","\xc3\x8b"},
	{7,"&Gamma;","\xce\x93"},
	{8,"&Iacute;","\xc3\x8d"},
	{7,"&Icirc;","\xc3\x8e"},
	{8,"&Igrave;","\xc3\x8c"},
	{6,"&Iota;","\xce\x99"},
	{6,"&Iuml;","\xc3\x8f"},
	{7,"&Kappa;","\xce\x9a"},
	{8,"&Lambda;","\xce\x9b"},
	{4,"&Mu;","\xce\x9c"},
	{8,"&Ntilde;","\xc3\x91"},
	{4,"&Nu;","\xce\x9d"},
	{7,"&OElig;","\xc5\x92"},
	{8,"&Oacute;","\xc3\x93"},
	{7,"&Ocirc;","\xc3\x94"},
	{8,"&Ograve;","\xc3\x92"},
	{7,"&Omega;","\xce\xa9"},
	{9,"&Omicron;","\xce\x9f"},
	{8,"&Oslash;","\xc3\x98"},
	{8,"&Otilde;","\xc3\x95"},
	{6,"&Ouml;","\xc3\x96"},
	{5,"&Phi;","\xce\xa6"},
	{4,"&Pi;","\xce\xa0"},
	{7,"&Prime;","\xe2\x80\xb3"},
	{5,"&Psi;","\xce\xa8"},
	{5,"&Rho;","\xce\xa1"},
	{8,"&Scaron;","\xc5\xa0"},
	{7,"&Sigma;","\xce\xa3"},
	{7,"&THORN;","\xc3\x9e"},
	{5,"&Tau;","\xce\xa4"},
	{7,"&Theta;","\xce\x98"},
	{8,"&Uacute;","\xc3\x9a"},
	{7,"&Ucirc;","\xc3\x9b"},
	{8,"&Ugrave;","\xc3\x99"},
	{9,"&Upsilon;","\xce\xa5"},
	{6,"&Uuml;","\xc3\x9c"},
	{4,"&Xi;","\xce\x9e"},
	{8,"&Yacute;","\xc3\x9d"},
	{6,"&Yuml;","\xc5\xb8"},
	{6,"&Zeta;","\xce\x96"},
	{8,"&aacute;","\xc3\xa1"},
	{7,"&acirc;","\xc3\xa2"},
	{7,"&acute;","\xc2\xb4"},
	{7,"&aelig;","\xc3\xa6"},
	{8,"&agrave;","\xc3\xa0"},
	{9,"&alefsym;","\xe2\x84\xb5"},
	{7,"&alpha;","\xce\xb1"},
	{5,"&amp;","\x26"},
	{5,"&and;","\xe2\x88\xa7"},
	{5,"&ang;","\xe2\x88\xa0"},
	{7,"&aring;","\xc3\xa5"},
	{7,"&asymp;","\xe2\x89\x88"},
	{8,"&atilde;","\xc3\xa3"},
	{6,"&auml;","\xc3\xa4"},
	{7,"&bdquo;","\xe2\x80\x9e"},
	{6,"&beta;","\xce\xb2"},
	{8,"&brvbar;","\xc2\xa6"},
	{6,"&bull;","\xe2\x80\xa2"},
	{5,"&cap;","\xe2\x88\xa9"},
	{8,"&ccedil;","\xc3\xa7"},
	{7,"&cedil;","\xc2\xb8"},
	{6,"&cent;","\xc2\xa2"},
	{5,"&chi;","\xcf\x87"},
	{6,"&circ;","\xcb\x86"},
	{7,"&clubs;","\xe2\x99\xa3"},
	{6,"&cong;","\xe2\x89\x85"},
	{6,"&copy;","\xc2\xa9"},
	{7,"&crarr;","\xe2\x86\xb5"},
	{5,"&cup;","\xe2\x88\xaa"},
	{8,"&curren;","\xc2\xa4"},
	{6,"&dArr;","\xe2\x87\x93"},
	{8,"&dagger;","\xe2\x80\xa0"},
	{6,"&darr;","\xe2\x86\x93"},
	{5,"&deg;","\xc2\xb0"},
	{7,"&delta;","\xce\xb4"},
	{7,"&diams;","\xe2\x99\xa6"},
	{8,"&divide;","\xc3\xb7"},
	{8,"&eacute;","\xc3\xa9"},
	{7,"&ecirc;","\xc3\xaa"},
	{8,"&egrave;","\xc3\xa8"},
	{7,"&empty;","\xe2\x88\x85"},
	{6,"&emsp;","\xe2\x80\x83"},
	{6,"&ensp;","\xe2\x80\x82"},
	{9,"&epsilon;","\xce\xb5"},
	{7,"&equiv;","\xe2\x89\xa1"},
	{5,"&eta;","\xce\xb7"},
	{5,"&eth;","\xc3\xb0"},
	{6,"&euml;","\xc3\xab"},
	{6,"&euro;","\xe2\x82\xac"},
	{7,"&exist;","\xe2\x88\x83"},
	{6,"&fnof;","\xc6\x92"},
	{8,"&forall;","\xe2\x88\x80"},
	{8,"&frac12;","\xc2\xbd"},
	{8,"&frac14;","\xc2\xbc"},
	{8,"&frac34;","\xc2\xbe"},
	{7,"&frasl;","\xe2\x81\x84"},
	{7,"&gamma;","\xce\xb3"},
	{4,"&ge;","\xe2\x89\xa5"},
	{4,"&gt;","\x3e"},
	{6,"&hArr;","\xe2\x87\x94"},
	{6,"&harr;","\xe2\x86\x94"},
	{8,"&hearts;","\xe2\x99\xa5"},
	{8,"&hellip;","\xe2\x80\xa6"},
	{8,"&iacute;","\xc3\xad"},
	{7,"&icirc;","\xc3\xae"},
	{7,"&iexcl;","\xc2\xa1"},
	{8,"&igrave;","\xc3\xac"},
	{7,"&image;","\xe2\x84\x91"},
	{7,"&infin;","\xe2\x88\x9e"},
	{5,"&int;","\xe2\x88\xab"},
	{6,"&iota;","\xce\xb9"},
	{8,"&iquest;","\xc2\xbf"},
	{6,"&isin;","\xe2\x88\x88"},
	{6,"&iuml;","\xc3\xaf"},
	{7,"&kappa;","\xce\xba"},
	{6,"&lArr;","\xe2\x87\x90"},
	{8,"&lambda;","\xce\xbb"},
	{6,"&lang;","\xe2\x8c\xa9"},
	{7,"&laquo;","\xc2\xab"},
	{6,"&larr;","\xe2\x86\x90"},
	{7,"&lceil;","\xe2\x8c\x88"},
	{7,"&ldquo;","\xe2\x80\x9c"},
	{4,"&le;","\xe2\x89\xa4"},
	{8,"&lfloor;","\xe2\x8c\x8a"},
	{8,"&lowast;","\xe2\x88\x97"},
	{5,"&loz;","\xe2\x97\x8a"},
	{5,"&lrm;","\xe2\x80\x8e"},
	{8,"&lsaquo;","\xe2\x80\xb9"},
	{7,"&lsquo;","\xe2\x80\x98"},
	{4,"&lt;","\x3c"},
	{6,"&macr;","\xc2\xaf"},
	{7,"&mdash;","\xe2\x80\x94"},
	{7,"&micro;","\xc2\xb5"},
	{8,"&middot;","\xc2\xb7"},
	{7,"&minus;","\xe2\x88\x92"},
	{4,"&mu;","\xce\xbc"},
	{7,"&nabla;","\xe2\x88\x87"},
	{6,"&nbsp;","\xc2\xa0"},
	{7,"&ndash;","\xe2\x80\x93"},
	{4,"&ne;","\xe2\x89\xa0"},
	{4,"&ni;","\xe2\x88\x8b"},
	{5,"&not;","\xc2\xac"},
	{7,"&notin;","\xe2\x88\x89"},
	{6,"&nsub;","\xe2\x8a\x84"},
	{8,"&ntilde;","\xc3\xb1"},
	{4,"&nu;","\xce\xbd"},
	{8,"&oacute;","\xc3\xb3"},
	{7,"&ocirc;","\xc3\xb4"},
	{7,"&oelig;","\xc5\x93"},
	{8,"&ograve;","\xc3\xb2"},
	{7,"&oline;","\xe2\x80\xbe"},
	{7,"&omega;","\xcf\x89"},
	{9,"&omicron;","\xce\xbf"},
	{7,"&oplus;","\xe2\x8a\x95"},
	{4,"&or;","\xe2\x88\xa8"},
	{6,"&ordf;","\xc2\xaa"},
	{6,"&ordm;","\xc2\xba"},
	{8,"&oslash;","\xc3\xb8"},
	{8,"&otilde;","\xc3\xb5"},
	{8,"&otimes;","\xe2\x8a\x97"},
	{6,"&ouml;","\xc3\xb6"},
	{6,"&para;","\xc2\xb6"},
	{6,"&part;","\xe2\x88\x82"},
	{8,"&permil;","\xe2\x80\xb0"},
	{6,"&perp;","\xe2\x8a\xa5"},
	{5,"&phi;","\xcf\x86"},
	{4,"&pi;","\xcf\x80"},
	{5,"&piv;","\xcf\x96"},
	{8,"&plusmn;","\xc2\xb1"},
	{7,"&pound;","\xc2\xa3"},
	{7,"&prime;","\xe2\x80\xb2"},
	{6,"&prod;","\xe2\x88\x8f"},
	{6,"&prop;","\xe2\x88\x9d"},
	{5,"&psi;","\xcf\x88"},
	{6,"&quot;","\x22"},
	{6,"&rArr;","\xe2\x87\x92"},
	{7,"&radic;","\xe2\x88\x9a"},
	{6,"&rang;","\xe2\x8c\xaa"},
	{7,"&raquo;","\xc2\xbb"},
	{6,"&rarr;","\xe2\x86\x92"},
	{7,"&rceil;","\xe2\x8c\x89"},
	{7,"&rdquo;","\xe2\x80\x9d"},
	{6,"&real;","\xe2\x84\x9c"},
	{5,"&reg;","\xc2\xae"},
	{8,"&rfloor;","\xe2\x8c\x8b"},
	{5,"&rho;","\xcf\x81"},
	{5,"&rlm;","\xe2\x80\x8f"},
	{8,"&rsaquo;","\xe2\x80\xba"},
	{7,"&rsquo;","\xe2\x80\x99"},
	{7,"&sbquo;","\xe2\x80\x9a"},
	{8,"&scaron;","\xc5\xa1"},
	{6,"&sdot;","\xe2\x8b\x85"},
	{6,"&sect;","\xc2\xa7"},
	{5,"&shy;","\xc2\xad"},
	{7,"&sigma;","\xcf\x83"},
	{8,"&sigmaf;","\xcf\x82"},
	{5,"&sim;","\xe2\x88\xbc"},
	{8,"&spades;","\xe2\x99\xa0"},
	{5,"&sub;","\xe2\x8a\x82"},
	{6,"&sube;","\xe2\x8a\x86"},
	{5,"&sum;","\xe2\x88\x91"},
	{6,"&sup1;","\xc2\xb9"},
	{6,"&sup2;","\xc2\xb2"},
	{6,"&sup3;","\xc2\xb3"},
	{5,"&sup;","\xe2\x8a\x83"},
	{6,"&supe;","\xe2\x8a\x87"},
	{7,"&szlig;","\xc3\x9f"},
	{5,"&tau;","\xcf\x84"},
	{8,"&there4;","\xe2\x88\xb4"},
	{7,"&theta;","\xce\xb8"},
	{10,"&thetasym;","\xcf\x91"},
	{8,"&thinsp;","\xe2\x80\x89"},
	{7,"&thorn;","\xc3\xbe"},
	{7,"&tilde;","\xcb\x9c"},
	{7,"&times;","\xc3\x97"},
	{7,"&trade;","\xe2\x84\xa2"},
	{6,"&uArr;","\xe2\x87\x91"},
	{8,"&uacute;","\xc3\xba"},
	{6,"&uarr;","\xe2\x86\x91"},
	{7,"&ucirc;","\xc3\xbb"},
	{8,"&ugrave;","\xc3\xb9"},
	{5,"&uml;","\xc2\xa8"},
	{7,"&upsih;","\xcf\x92"},
	{9,"&upsilon;","\xcf\x85"},
	{6,"&uuml;","\xc3\xbc"},
	{8,"&weierp;","\xe2\x84\x98"},
	{4,"&xi;","\xce\xbe"},
	{8,"&yacute;","\xc3\xbd"},
	{5,"&yen;","\xc2\xa5"},
	{6,"&yuml;","\xc3\xbf"},
	{6,"&zeta;","\xce\xb6"},
	{5,"&zwj;","\xe2\x80\x8d"},
	{6,"&zwnj;","\xe2\x80\x8c"}
};

static const struct
{
	const int len;
	const int hint;
	const char *tag;
} html[MAXHTML]=
{
	{8,1,"!doctype"},
	{1,3,"a"},
	{4,0,"abbr"},
	{7,0,"acronym"},
	{7,0,"address"},
	{6,0,"applet"},
	{4,0,"area"},
	{1,0,"b"},
	{4,0,"base"},
	{8,0,"basefont"},
	{3,0,"bdo"},
	{3,0,"big"},
	{10,0,"blockquote"},
	{4,1,"body"},
	{2,0,"br"},
	{6,0,"button"},
	{7,0,"caption"},
	{6,0,"center"},
	{4,0,"cite"},
	{4,0,"code"},
	{3,0,"col"},
	{8,0,"colgroup"},
	{2,0,"dd"},
	{3,0,"del"},
	{3,0,"dfn"},
	{3,0,"dir"},
	{3,0,"div"},
	{2,0,"dl"},
	{2,0,"dt"},
	{2,0,"em"},
	{5,0,"embed"},
	{8,0,"fieldset"},
	{4,0,"font"},
	{4,0,"form"},
	{5,0,"frame"},
	{8,0,"frameset"},
	{2,0,"h1"},
	{2,0,"h2"},
	{2,0,"h3"},
	{2,0,"h4"},
	{2,0,"h5"},
	{2,0,"h6"},
	{4,1,"head"},
	{2,0,"hr"},
	{4,1,"html"},
	{1,0,"i"},
	{6,0,"iframe"},
	{3,5,"img"},
	{5,0,"input"},
	{3,0,"ins"},
	{7,0,"isindex"},
	{3,0,"kbd"},
	{5,0,"label"},
	{6,0,"legend"},
	{2,0,"li"},
	{4,0,"link"},
	{3,0,"map"},
	{4,0,"menu"},
	{4,1,"meta"},
	{4,0,"nobr"},
	{8,0,"noframes"},
	{8,0,"noscript"},
	{6,0,"object"},
	{2,0,"ol"},
	{8,0,"optgroup"},
	{6,0,"option"},
	{1,0,"p"},
	{5,0,"param"},
	{3,0,"pre"},
	{1,0,"q"},
	{1,0,"s"},
	{4,0,"samp"},
	{6,0,"script"},
	{6,0,"select"},
	{5,0,"small"},
	{4,0,"span"},
	{6,0,"strike"},
	{6,0,"strong"},
	{5,0,"style"},
	{3,0,"sub"},
	{3,0,"sup"},
	{5,0,"table"},
	{5,0,"tbody"},
	{2,0,"td"},
	{8,0,"textarea"},
	{5,0,"tfoot"},
	{2,0,"th"},
	{5,0,"thead"},
	{5,2,"title"},
	{2,0,"tr"},
	{2,0,"tt"},
	{1,0,"u"},
	{2,0,"ul"},
	{3,0,"var"},
	{3,4,"xmp"}
};

static const struct
{
	const int len;
	const unsigned char *bad;
	const unsigned char *good;
} htmlfix[]=
{
	{13,"align=center>","<p align=center>"},
	{3,"bi>",NULL},
	{5,"bold>","b>"},
	{2,"c>",NULL},
	{2,"e>","em>"},
	{11,"font=green>","font color=green>"},
	{9,"font=red>","font color=red>"},
	{3,"i.>","i>"},
	{4,"lul>","ul>"},
	{4,"o:p>",NULL},
	{8,"strong.>","strong>"},
	{7,"storng>","strong>"},
	{6,"stron>","strong>"},
	{0,NULL,NULL}
};

static const struct
{
	const int len;
	const unsigned char *bad;
	const unsigned char *good;
} directfix[]=
{
	{8,"<br>div>","<br><div>"},
	{8,"</b</p>>","</b></p>"},
	{6,"</li>>","</li>"},
	{9,"</p> <  \n","</p>\n"},
	{7,"</p<  \n","</p>\n"},
	{8,"<br><br<","<br><br>"},
	{7,"<br>br>","<br><br>"},
	{6,"</</a>","</a>"},
	{7,"<dl<dt>","<dl><dt>"},
	{7,"</b</i>","</b></i>"},
	{5,"</dd.","</dd>"},
	{17,"<font color=blue)","<font color=blue>"},
	{4,"</p?","</p>"},
	{0,NULL,NULL}
};

static const struct
{
	const int len;
	const unsigned char *bad;
	const unsigned char *good;
} startupfix[]=
{
	{8,"<pSeiten","<p>Seiten"},
	{4,"<<b>","<b>"},
	{2,"b>","<b>"},
	{0,NULL,NULL}
};

static const struct
{
	const unsigned char *bad;
	const unsigned char *good;
} finalfix[]=
{
	{"</p  \n","</p>\n"},
	{"</p\n","</p>\n"},
	{"</strong.  \n","</strong>\n"},
	{"</strong. \n","</strong>\n"},
	{"</strong.\n","</strong>\n"},
	{"</strong  \n","</strong>\n"},
	{"</strong \n","</strong>\n"},
	{"</strong\n","</strong>\n"},
	{NULL,NULL}
};

static int all=0;
static int verbose=0;
static CACHE cache;

static void tagwarn(unsigned char *tag,int taglen,unsigned char *label)
{
	int lf;
	int i;

	if(!verbose)return;
	printf("Warning: %s ",label);
	for(lf=i=0;i<taglen;i++)
	{
		if(tag[i]=='\n')lf=1;
		else lf=0;
		printf("%c",tag[i]);
	}
	if(!lf)printf("\n");
}

static int dstcpy(unsigned char **dst,int *max,unsigned char *src,int len)
{
	while(len--)
	{
		if(!(*max)--)return -1;
		*(*dst)++=*src++;
	}
	return 0;
}

static int dotext(unsigned char **dst,int *max,unsigned char *src,int len,
	int state)
{
	if(state&2)
	{
		tagwarn(src,len,"skipping text of illegal tag");
		return 0;
	}

again:	while(len)
	{
		int i;

		for(i=0;i<len;i++)switch(src[i])
		{
		case '&':
			if(i)if(dstcpy(dst,max,src,i))return -1;
			if(dstcpy(dst,max,"&amp;",5))return -1;
			len-=i+1;
			src+=i+1;
			goto again;
		case '<':
			if(i)if(dstcpy(dst,max,src,i))return -1;
			if(dstcpy(dst,max,"&lt;",4))return -1;
			len-=i+1;
			src+=i+1;
			goto again;
		case '>':
			if(i)if(dstcpy(dst,max,src,i))return -1;
			if(dstcpy(dst,max,"&gt;",4))return -1;
			len-=i+1;
			src+=i+1;
			goto again;
		case '\n':
			if(i+1!=len)if(src[i+1]=='\n')
			{
				if(i)if(dstcpy(dst,max,src,i))return -1;
				if(dstcpy(dst,max,"\n<p>",4))return -1;
				for(i+=2;i<len;i++)if(src[i]!='\n')break;
				len-=i;
				src+=i;
				goto again;
			}
			break;
		}
		if(i)if(dstcpy(dst,max,src,i))return -1;
		len-=i;
		src+=i;
	}
	return 0;
}

static int dohref(unsigned char **dst,int *max,unsigned char *src,int len,
	int quiet)
{
	int fixed;
	int kept;
	int addclose;
	int href;
	int name;
	unsigned char *p;
	unsigned char target[BUFSIZEH];

	fixed=0;
	kept=0;
	addclose=0;
	href=0;
	name=0;
	for(p=src+1;*p==' '||*p=='\n';p++);
	for(p++;*p==' '||*p=='\n';p++);
	while(*p!='>')
	{
		if(!strncasecmp(p,"href",4)&&!href&&!name)
		{
			p+=4;
			href=1;
		}
		if(!strncasecmp(p,"h ref",5)&&!href&&!name)
		{
			fixed=1;
			p+=5;
			href=1;
		}
		if(!strncasecmp(p,"hef",3)&&!href&&!name)
		{
			fixed=1;
			p+=3;
			href=1;
		}
		if(!strncasecmp(p,"http://",7)&&!href&&!name)
		{
			fixed=1;
			p--;
			href=1;
		}
		else if(!strncasecmp(p,"name",4)&&!href&&!name)
		{
			p+=4;
			name=1;
		}
		if(href==1||name==1)
		{
			unsigned char *q;

			if(href==1)href=2;
			else name=2;
			for(;*p==' '||*p=='\t'||*p=='\n';p++);
			if(*p!='=')
			{
				if(*p=='\"')
				{
					p--;
					fixed=1;
				}
				else if(*p==':')fixed=1;
				else if(!strncasecmp(p,"http://",7))fixed=1;
				else
				{
					printf("can't process link\n");
					return -1;
				}
			}
			for(p++;*p==' '||*p=='\t'||*p=='\n';p++);
			if(*p=='\"'&&p[1]=='\"'&&(p[2]!=' '&&p[2]!='\t'&&
				p[2]!='\n'&&p[2]!='>'))
			{
				fixed=1;
				p+=2;
			}
			if(*p=='\"')
			{
				if(!strncasecmp(p+1,"<a href=\"",9))
				{
					fixed=1;
					p+=9;
				}
				p++;
				for(q=p;*q!='\"'&&*q!='>';q++);
				if(*q=='>')
				{
					fixed=1;
					if(q==p)target[0]=0;
					else
					{
						if((q-p)>=sizeof(target))
						{
							printf("can't process "
								"link\n");
							return -1;
						}
						memcpy(target,p,(q-p));
						target[(q-p)]=0;
					}
					p=q;
				}
				else
				{
					if(q==p)target[0]=0;
					else
					{
						if((q-p)>=sizeof(target))
						{
							printf("can't process "
								"link\n");
							return -1;
						}
						memcpy(target,p,(q-p));
						target[(q-p)]=0;
					}
					for(p=q+1;*p==' '||*p=='\t'||*p=='\n';
						p++);
				}
			}
			else
			{
				fixed=1;
				for(q=p;*q!=' '&&*q!='\t'&&*q!='\n'&&*q!='>';
					q++);
				if(*q=='>')if(q[-1]=='\"')
				{
					fixed=1;
					q--;
				}

				if(q==p)target[0]=0;
				else
				{
					if((q-p)>=sizeof(target))
					{
						printf("can't process link\n");
						return -1;
					}
					memcpy(target,p,(q-p));
					target[(q-p)]=0;
				}
				for(p=q;*p==' '||*p=='\t'||*p=='\n';p++);
			}
		}
		else
		{
			fixed=1;
			for(;(*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z');p++);
			for(p++;*p==' '||*p=='\t'||*p=='\n';p++);
			if(*p=='='||*p=='\"')
			{
				if(*p=='=')for(p++;*p==' '||*p=='\t'||*p=='\n'
					;p++);
				if(*p=='\"')
				{
					for(p++;*p!='\"'&&*p!='>';p++);
					if(*p=='\"')
						for(p++;*p==' '||*p=='\t'||
							*p=='\n';p++);
				}
				else
				{
					for(;*p!=' '&&*p!='\t'&&*p!='\n'
						&&*p!='>';p++);
					for(;*p==' '||*p=='\t'||*p=='\n';p++);
				}
			}
		}
	}

	if(!href&&!name)
	{
		tagwarn(src,len,"keeping broken link");
		if(dstcpy(dst,max,src,len))return -1;
		return 0;
	}

	for(p=target;*p;p++)if(*p>='a'&&*p<='z')break;
	else if(*p>='A'&&*p<='Z')break;
	else if(*p>='0'&&*p<='9')break;
	else if(*p=='/'||*p=='#'||*p=='.'||*p=='\"'||*p=='\'')break;
	else fixed=1;
	if(p!=target)memmove(target,p,strlen(p)+1);
	if(*target=='\"'||*target=='\'')
	{
		fixed=1;
		for(p=target;*p;p++);
		if(p[-1]=='\"'||p[-1]=='\'')p[-1]=0;
		if(*target)memmove(target,target+1,strlen(target));
	}

	if(*target)
	{
		for(p=target;*p;p++);
		for(;p!=target;p--)if(p[-1]!=' '&&p[-1]!='\t'&&p[-1]!='\n')
			break;
		if(*p)
		{
			fixed=1;
			*p=0;
		}
	}

	if(href)
	{
		for(p=target;*p&&*p!='#'&&*p!='?';)
			if(*p=='/'&&(p[1]==' '||p[1]=='\t'||p[1]=='\n'))
		{
			fixed=1;
			memmove(p+1,p+2,strlen(p+1));
		}
		else p++;
		for(p=target;*p&&*p!='#'&&*p!='?';)
			if((*p==' '||*p=='\t'||*p=='\n')&&p[1]=='/')
		{
			fixed=1;
			memmove(p,p+1,strlen(p));
		}
		else p++;
		for(p=target;*p&&*p!='#'&&*p!='?';p++)
			if(*p==' '||*p=='\t'||*p=='\n')
		{
			fixed=1;
			*p='_';
		}
		if(!strncmp(target,"World/",6)||
			!strncmp(target,"Recreation/",11)||
			!strncmp(target,"Computers/",10)||
			!strncmp(target,"Shopping/",9)||
			!strncmp(target,"Regional/",9)||
			!strncmp(target,"Business/",9)||
			!strncmp(target,"Society/",8)||
			!strncmp(target,"Health/",7)||
			!strncmp(target,"News/",5)||
			!strncmp(target,"Arts/",5))
		{
			int llen;

			fixed=1;
			llen=strlen(target)+1;
			if(llen>=sizeof(target))
			{
				printf("can't process link\n");
				return -1;
			}
			memmove(target+1,target,len);
			*target='/';
		}
		else if(!strncasecmp(target,"http//",6))
		{
			int llen;

			fixed=1;
			llen=strlen(target)+1;
			if(llen>=sizeof(target))
			{
				printf("can't process link\n");
				return -1;
			}
			memmove(target+5,target+4,len-4);
			target[4]=':';
		}
		else if(!strncmp(target,"htt/World/",10))
		{
			fixed=1;
			memmove(target,target+3,strlen(target+2));
		}
		else if(!strncasecmp(target,"dmoz.org/",9)||
			!strncasecmp(target,"www.",4))
		{
			int llen;

			fixed=1;
			llen=strlen(target)+1;
			if(llen+7>=sizeof(target))
			{
				printf("can't process link\n");
				return -1;
			}
			memmove(target+7,target,len);
			memcpy(target,"http://",7);
		}
		else if(!strchr(target,'/'))if((p=strrchr(target,'.')))
			if(!strcasecmp(p,".com")||!strcasecmp(p,".net")||
				!strcasecmp(p,".org"))
		{
			int llen;

			fixed=1;
			llen=strlen(target)+1;
			if(llen+7>=sizeof(target))
			{
				printf("can't process link\n");
				return -1;
			}
			memmove(target+7,target,len);
			memcpy(target,"http://",7);
		}

		if(!*target)
		{
			fixed=1;
			kept=1;
		}
		else if(strchr(target,'?'))
		{
			if(strncasecmp(target,"http://",7)&&
				strncasecmp(target,"https://",8))
			{
				fixed=1;
				kept=1;
			}
		}
		else if(!strncasecmp(target,"http://",7))
		{
			p=NULL;
			if(!strncasecmp(target+7,"dmoz.org:8080",13))
				p=target+20;
			else if(!strncasecmp(target+7,"dmoz.org",8))p=target+15;
			else if(!strncasecmp(target+7,"www.dmoz.com",12))
				p=target+19;
			else if(!strncasecmp(target+7,"www.dmoz.org",12))
				p=target+19;
			else if(!strncasecmp(target+7,"www.dmoz",8))p=target+15;
			else if(!strncasecmp(target+7,"dmoz",4))p=target+11;
			if(p)
			{
				if(!*p)
				{
					fixed=1;
					strcpy(target,"/");
				}
				else if(*p=='/')
				{
					unsigned char *q;

					q=strrchr(p,'/');
					q++;
					if(!*q||!strcmp(q,"desc.html")||
						!strncmp(q,"desc.html#",10))
					{
						fixed=1;
						memmove(target,p,strlen(p)+1);
					}
					else if(!(q=strchr(q,'.')))
					{
						fixed=1;
						memmove(target,p,strlen(p)+1);
					}
					else if(strncmp(q,".html",5))
					{
						fixed=1;
						memmove(target,p,strlen(p)+1);
					}
				}
				else if(*p>='A'&&*p<='Z')
				{
					unsigned char *q;

					q=strrchr(p,'/');
					q++;
					if(!*q||!strcmp(q,"desc.html")||
						!strncmp(q,"desc.html#",10))
					{
						fixed=1;
						memmove(target+1,p,strlen(p)+1);
						*target='/';
					}
					else if(!(q=strchr(q,'.')))
					{
						fixed=1;
						memmove(target+1,p,strlen(p)+1);
						*target='/';
					}
					else if(strncmp(q,".html",5))
					{
						fixed=1;
						memmove(target+1,p,strlen(p)+1);
						*target='/';
					}
					else
					{
						int llen;

						fixed=1;
						llen=strlen(target)+1;
						if(llen>=sizeof(target))
						{
							printf("can't process "
								"link\n");
							return -1;
						}
						memmove(p+1,p,strlen(p)+1);
						*p='/';
					}
				}
			}
		}
		else if(strncasecmp(target,"https://",8)&&
			strncasecmp(target,"ftp://",6)&&
			strncasecmp(target,"news:",5))
		{
			unsigned char *q;

			p=strrchr(target,'/');
			if(p)p++;
			else p=target;
			if((q=strchr(p,'.')))if(!strcmp(q,".html")||
				!strncmp(q,".html#",6))if(strncmp(p,"desc.",5))
			{
				if(*target=='/')
				{
						int llen;

						fixed=1;
						llen=strlen(target)+1;
						if(llen+15>=sizeof(target))
						{
							printf("can't process "
								"link\n");
							return -1;
						}
						memmove(target+15,target,llen);
						memcpy(target,"http://dmoz.org",
							15);
				}
				else
				{
						int llen;

						fixed=1;
						llen=strlen(target)+1;
						if(llen+16>=sizeof(target))
						{
							printf("can't process "
								"link\n");
							return -1;
						}
						memmove(target+16,target,llen);
						memcpy(target,
							"http://dmoz.org/",16);
				}
			}
		}

		for(p=target;*p;p++)if(*p=='/')if(!strcasecmp(p+1,"</a"))
		{
			fixed=1;
			addclose=1;
			p[1]=0;
		}

		if(*target!='#')if(strncasecmp(target,"http:",5)&&
			strncasecmp(target,"https:",6)&&
			strncasecmp(target,"ftp:",4)&&
			strncasecmp(target,"news:",5))if(!strchr(target,'?'))
		{
			p=strrchr(target,'/');
			if(!p)p=target;
			else p++;
			if(*p)if(strncmp(p,"desc.html",9))
			{
				int llen;

				fixed=1;
				llen=strlen(target)+1;
				if(llen>=sizeof(target))
				{
					printf("can't process link\n");
					return -1;
				}
				target[llen-1]='/';
				target[llen]=0;
			}

			for(p=target;*p;)if(*p=='%')
			{
				unsigned int val;

				if(p[1]>='0'&&p[1]<='9')val=p[1]-'0';
				else if(p[1]>='A'&&p[1]<='F')val=p[1]-'A'+10;
				else if(p[1]>='a'&&p[1]<='f')val=p[1]-'a'+10;
				else
				{
					if(!fixed)kept=1;
					fixed=1;
					break;
				}
				val<<=4;
				if(p[2]>='0'&&p[2]<='9')val|=p[2]-'0';
				else if(p[2]>='A'&&p[2]<='F')val|=p[2]-'A'+10;
				else if(p[2]>='a'&&p[2]<='f')val|=p[2]-'a'+10;
				else
				{
					if(!fixed)kept=1;
					fixed=1;
					break;
				}
				if(val<0x80)
				{
					p+=3;
					continue;
				}
				if(p[3]!='%')
				{
					p+=3;
					continue;
				}
				if((val&0xe0)==0xc0)val=1;
				else if((val&0xf0)==0xe0)val=2;
				else if((val&0xf8)==0xf0)val=3;
				else if((val&0xfc)==0xf8)val=4;
				else if((val&0xfe)==0xfc)val=5;
				else
				{
					if(!fixed)kept=1;
					fixed=1;
					break;
				}
				for(p+=3;val;val--)
				{
					if(*p!='%')
					{
						if(!*p)p-=3;
						break;
					}
					if(p[1]!='8'&&p[1]!='9'&&p[1]!='a'&&
						p[1]!='b'&&p[1]!='A'&&p[1]!='B')
						break;
					if(!(p[2]>='0'&&p[2]<='9')&&
						!(p[2]>='a'&&p[2]<='f')&&
						!(p[2]>='A'&&p[2]<='F'))break;
					p+=3;
				}
				if(val)
				{
					if(!fixed)kept=1;
					fixed=1;
					break;
				}
			}
			else p++;

			if(!*p)for(p=target;*p;p++)if(*p=='%')
			{
				unsigned int val=0;

				if(p[1]>='0'&&p[1]<='9')val=p[1]-'0';
				else if(p[1]>='A'&&p[1]<='F')val=p[1]-'A'+10;
				else if(p[1]>='a'&&p[1]<='f')val=p[1]-'a'+10;
				val<<=4;
				if(p[2]>='0'&&p[2]<='9')val|=p[2]-'0';
				else if(p[2]>='A'&&p[2]<='F')val|=p[2]-'A'+10;
				else if(p[2]>='a'&&p[2]<='f')val|=p[2]-'a'+10;
				if(val<0x80)
				{
					p+=2;
					continue;
				}
				if(p[3]!='%')
				{
					int llen;

					fixed=1;
					llen=strlen(target)+1;
					if(llen+2>=sizeof(target))
					{
						printf("can't process link\n");
						return -1;
					}
					memmove(p+3,p,strlen(p)+1);
					p[1]='a';
					p[2]='0'+(unsigned char)(val>>6);
					val=(val&0x3f)|0x80;
					p[4]=(unsigned char)(val>>4);
					if(p[4]<10)p[4]+='0';
					else p[4]+='a'-10;
					p[5]=(unsigned char)(val&0xf);
					if(p[5]<10)p[5]+='0';
					else p[5]+='a'-10;
					p+=5;
					continue;
				}
				if((val&0xe0)==0xc0)val=1;
				else if((val&0xf0)==0xe0)val=2;
				else if((val&0xf8)==0xf0)val=3;
				else if((val&0xfc)==0xf8)val=4;
				else if((val&0xfe)==0xfc)val=5;
				for(p+=2;val;val--)p+=3;
			}

			for(p=target;*p;p++)if(!(*p>='0'&&*p<='9')&&
				!(*p>='a'&&*p<='z')&&!(*p>='A'&&*p<='Z'))
				switch(*p)
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
				break;
			default:
				fixed=1;
				if(strlen(target)+2>=sizeof(target))
				{
					printf("can't process link\n");
					return -1;
				}
				memmove(p+2,p,strlen(p)+1);
				p[1]=p[0]>>4;
				if(p[1]<10)p[1]+='0';
				else p[1]+='a'-10;
				p[2]=p[0]&0xf;
				if(p[2]<10)p[2]+='0';
				else p[2]+='a'-10;
				p[0]='%';
				p+=2;
				break;
			}
		}
	}

	if(dstcpy(dst,max,name?"<a name=\"":"<a href=\"",9))
	{
		printf("can't process link\n");
		return -1;
	}
	if(dstcpy(dst,max,target,strlen(target)))
	{
		printf("can't process link\n");
		return -1;
	}
	if(dstcpy(dst,max,"\">",2))
	{
		printf("can't process link\n");
		return -1;
	}
	if(addclose)if(dstcpy(dst,max,"</a>",4))
	{
		printf("can't process link\n");
		return -1;
	}

	if(fixed&&!quiet)tagwarn(src,len,kept?"keeping broken link":
		"trying to fix broken link");
	return 0;
}

static int brokenhref(unsigned char **dst,int *max,unsigned char *src,int len)
{
	int fill;
	unsigned char *p;
	unsigned char bfr[BUFSIZEH];

	for(p=src+1;*p==' '||*p=='\n';p++);
	if(!strncasecmp(p,"http://",7))
	{
		int llen;
		unsigned char *q;

		strcpy(bfr,"<a href=\"");
		fill=9;
		for(q=src+len-2;*q=='\"'||*q==' '||*q=='\n';q--);
		llen=(q-p)+1;
		if(llen+fill>=sizeof(bfr))return -1;
		memcpy(bfr+fill,p,llen);
		fill+=llen;
		if(fill+2>=sizeof(bfr))return -1;
		bfr[fill++]='\"';
		bfr[fill++]='>';
	}
	else if(!strncasecmp(p,"ahref",5))
	{
		p+=5;
		strcpy(bfr,"<a href");
		fill=7;
		len-=(p-src);
		if(len+fill>=sizeof(bfr))return -1;
		memcpy(bfr+fill,p,len);
		fill+=len;
	}
	else if(!strncasecmp(p,"href",4))
	{
		p+=4;
		strcpy(bfr,"<a href");
		fill=7;
		len-=(p-src);
		if(len+fill>=sizeof(bfr))return -1;
		memcpy(bfr+fill,p,len);
		fill+=len;
	}
	else
	{
		printf("unknown broken link type, aborting\n");
		return -1;
	}
	tagwarn(src,len,"trying to fix broken link");
	return dohref(dst,max,bfr,fill,1);
}

static int doimg(unsigned char **dst,int *max,unsigned char *src,int len)
{
	unsigned char *p;

	for(p=src;*p&&*p!='>';p++)if(*p==' '||*p=='\t'||*p=='\n')
		if(!strncasecmp(p+1,"src",3))
	{
		for(p+=4;*p==' '||*p=='\t'||*p=='\n'||*p=='='||*p=='\"'||
			*p=='\'';p++);
		if(*p!='/')break;
		if(dstcpy(dst,max,src,(p-src)))
		{
			printf("can't process link\n");
			return -1;
		}
		if(dstcpy(dst,max,"http://dmoz.org",15))
		{
			printf("can't process link\n");
			return -1;
		}
		if(dstcpy(dst,max,p,len-(p-src)))
		{
			printf("can't process link\n");
			return -1;
		}
		tagwarn(src,len,"trying to fix broken link");
		return 0;
	}
	if(dstcpy(dst,max,src,len))return -1;
	return 0;
}

static int dotag(unsigned char **dst,int *max,unsigned char *src,int len,
	int *state)
{
	int i;
	int current;
	int delta;
	int closing;
	unsigned char *p;

	closing=0;
	for(p=src+1;*p==' '||*p=='\n';p++);
	if(*p=='/')
	{
		closing=1;
		for(p++;*p==' '||*p=='\n';p++);
	}

	current=HTMLSTART;
	delta=HTMLSTART;
	while(1)
	{
		if(!delta)
		{
			if(current<MAXHTML)if(!strncasecmp(p,html[current].tag,
				html[current].len))if(p[html[current].len]==' '
				||p[html[current].len]=='/'||
				p[html[current].len]=='>'||
				p[html[current].len]=='\n')break;
			current=-1;
			break;
		}
		else if(current>=MAXHTML)
		{
			current-=delta;
			delta>>=1;
			continue;
		}
		if(!(i=strncasecmp(p,html[current].tag,html[current].len)))
		{
			if(p[html[current].len]==' '||p[html[current].len]=='/'
				||p[html[current].len]=='>'||
				p[html[current].len]=='\n')break;
			i=1;
		}
		if(i<0)
		{
			if(!current)
			{
				current=-1;
				break;
			}
			current-=delta;
			delta>>=1;
		}
		else
		{
			current+=delta;
			delta>>=1;
		}
	}

	if(current==-1)
	{
		for(i=0;htmlfix[i].bad;i++)if(!strncasecmp(p,htmlfix[i].bad,
			htmlfix[i].len))
		{
		    if(htmlfix[i].good)
		    {
			tagwarn(src,len,
				(unsigned char *)"fixing broken sequence");
			if(dstcpy(dst,max,(unsigned char *)htmlfix[i].good,
				strlen(htmlfix[i].good)))return -1;
			return 0;
		    }
		    else
		    {
			tagwarn(src,len,
				(unsigned char *)"skipping illegal sequence");
			return 0;
		    }
		}
		if(!strncasecmp(p,"href=",5)||!strncasecmp(p,"ahref=",6)||
			!strncasecmp(p,"http://",7))
			if(!closing)
		{
			if(brokenhref(dst,max,src,len))return -1;
			return 0;
		}
	}

	if(current!=-1)
	{
		if(html[current].hint==4)
		{
			if(closing)*state&=~1;
			else *state|=1;
		}
		else if(html[current].hint==1&&!(*state&1))
		{
			tagwarn(src,len,"skipping illegal tag");
			return 0;
		}
		else if(html[current].hint==2&&!(*state&1))
		{
			if(closing)*state&=~2;
			else *state|=2;
			tagwarn(src,len,"skipping illegal tag");
			return 0;
		}
		else if(html[current].hint==3&&!closing)
		{
			if(dohref(dst,max,src,len,0))return -1;
			return 0;
		}
		else if(html[current].hint==5&&!closing)
		{
			if(doimg(dst,max,src,len))return -1;
			return 0;
		}
		if(dstcpy(dst,max,src,len))return -1;
		return 0;
	}

	return dotext(dst,max,src,len,*state);
}

static int htmlify(unsigned char *dst,int max,unsigned char *src)
{
	int i;
	int state;
	unsigned char *p;
	unsigned char *s;

	state=0;
	s=p=src;

	for(i=0;startupfix[i].bad;i++)if(!strncasecmp(p,startupfix[i].bad,
		startupfix[i].len))
	{
		tagwarn((unsigned char *)startupfix[i].bad,startupfix[i].len,
			(unsigned char *)"fixing broken sequence");
		if(dstcpy(&dst,&max,(unsigned char *)startupfix[i].good,
			strlen(startupfix[i].good)))return -1;
		s+=startupfix[i].len;
		p+=startupfix[i].len;
		break;
	}

	for(;*p;p++)if(*p=='<')
	{
		int quote;
		unsigned char *q;

		if(s!=p)if(dotext(&dst,&max,s,(p-s),state))return -1;
		s=p;

		for(i=0;directfix[i].bad;i++)if(!strncasecmp(p,directfix[i].bad,
			directfix[i].len))
		{
			tagwarn((unsigned char *)directfix[i].bad,
				directfix[i].len,
				(unsigned char *)"fixing broken sequence");
			if(dstcpy(&dst,&max,(unsigned char *)directfix[i].good,
				strlen(directfix[i].good)))return -1;
			s+=directfix[i].len;
			p+=directfix[i].len-1;
			break;
		}
		if(directfix[i].bad)continue;

		if(!strncmp(p+1,"!--",3))
		{
			for(q=p+4;*q;q++)if(*q=='-')if(q[1]=='-')
				if(q[2]=='>')
			{
				q+=2;
				break;
			}
			s=q+1;
			if(*q)
			{
				if(dstcpy(&dst,&max,p,(q-p)+1))
					return -1;
				p=q;
				continue;
			}
			else
			{
				if(dstcpy(&dst,&max,p,(q-p)))
					return -1;
				tagwarn("-->",3,"adding missing tag");
				if(dstcpy(&dst,&max,"-->",3))
					return -1;
				return 0;
			}
		}

		for(quote=0,q=p+1;*q;q++)if(*q=='\"')quote=1-quote;
		else if(*q=='<'&&!quote)
		{
			if(dotext(&dst,&max,s,(q-s),state))return -1;
			s=p=q;
		}
		else if(*q=='>')
		{
			if(p!=src)if(p[-1]=='<'&&q[1]=='>')
			{
				p=q;
				break;
			}
			if(dotag(&dst,&max,p,(q-p)+1,&state))return -1;
			s=q+1;
			p=q;
			break;
		}
		if(!*q)
		{
			for(i=0;finalfix[i].bad;i++)
				if(!strcasecmp(p,finalfix[i].bad))
			{
				tagwarn((unsigned char *)finalfix[i].bad,
					strlen(finalfix[i].bad),
					(unsigned char *)
					"fixing broken sequence");
				if(dstcpy(&dst,&max,
					(unsigned char *)finalfix[i].good,
					strlen(finalfix[i].good)))return -1;
				if(!max)return -1;
				*dst=0;
				return 0;
			}
		}
	}
	if(s!=p)if(dotext(&dst,&max,s,(p-s),state))return -1;
	if(!max)return -1;
	*dst=0;
	return 0;
}

static int urlify(char *url,int max)
{
	int len;
	int warned;
	char *mem;

	len=strlen(url)+1;
	warned=0;
	mem=url;

	for(;*url;url++)if(!(*url>='0'&&*url<='9')&&!(*url>='a'&&*url<='z')&&
		!(*url>='A'&&*url<='Z'))switch(*url)
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
		break;
	default:
		if(!warned)
		{
			warned=1;
			tagwarn(mem,strlen(mem),"trying to fix broken url");
		}
		if(len+2>=max)
		{
			printf("can't process url\n");
			return -1;
		}
		memmove(url+2,url,strlen(url)+1);
		url[1]=url[0]>>4;
		if(url[1]<10)url[1]+='0';
		else url[1]+='a'-10;
		url[2]=url[0]&0xf;
		if(url[2]<10)url[2]+='0';
		else url[2]+='a'-10;
		url[0]='%';
		url+=2;
		len+=2;
		break;
	}
	return 0;
}

static void *palloc(POOL **pool,int size)
{
	POOL *p;
	union
	{
		int s;
		void *n;
	} u;

	if(!size)size=sizeof(void *);
	else size=(size+sizeof(void *)-1)&(~(sizeof(void *)-1));
	for(p=*pool;p;p=p->next)if(p->avail>=size)
	{
		u.n=(void *)(p->data+p->offset);
		p->offset+=size;
		p->avail-=size;
		return u.n;
	}
	u.s=(size<=DFTPOOLSIZE?DFTPOOLSIZE:size);
	if(!(p=malloc(sizeof(POOL)+u.s)))
	{
		printf("out of memory\n");
		return NULL;
	}
	p->next=*pool;
	*pool=p;
	p->size=u.s;
	p->avail=u.s-size;
	p->offset=size;
	return (void *)(p->data);
}

static void pclear(POOL **pool)
{
	POOL *p;

	for(p=*pool;p;p=p->next)
	{
		p->avail=p->size;
		p->offset=0;
	}
}

static void pfree(POOL **pool)
{
	while(*pool)
	{
		POOL *p;

		p=*pool;
		*pool=p->next;
		free(p);
	}
}

static int *getblock(BLOCK **block,int size,int total)
{
	BLOCK *b;

	for(b=*block;b;b=b->next)if(b->free!=b->total)
	{
		unsigned int i;
		int j;

		for(i=0x80000000,j=(b->free*size)<<5;b->map[b->free]&i;
			i>>=1,j+=size);
		for(b->map[b->free]|=i;b->free<b->total;b->free++)
			if(b->map[b->free]!=0xffffffff)break;
		return b->data+j;
	}
	if(!(b=palloc(&cache.aux,sizeof(BLOCK)+(total>>5)*sizeof(int)
		+total*size*sizeof(int))))return NULL;
	b->next=*block;
	*block=b;
	b->total=total>>5;
	b->free=0;
	b->data=b->map+b->total;
	b->last=b->data+(total-1)*size;
	memset(b->map,0,b->total*sizeof(int));
	b->map[0]=0x80000000;
	return b->data;
}

static void relblock(BLOCK **block,int *element,int size)
{
	BLOCK *b;

	for(b=*block;b;b=b->next)
	{
		unsigned int i;
		int j;

		if(element<b->data||element>b->last)continue;
		switch(size)
		{
		case 4:	j=(element-b->data)>>2;
			break;
		case 8:	j=(element-b->data)>>3;
			break;
		case 16:j=(element-b->data)>>4;
			break;
		case 32:j=(element-b->data)>>5;
			break;
		default:j=(element-b->data)/size;
			break;
		}
		i=0x80000000>>(j&0x1f);
		j>>=5;
		b->map[j]&=~i;
		if(j<b->free)b->free=j;
		break;
	}
}

static int *getdynblock(DYNBLOCK **block,int *old,int oldsize,int newsize)
{
	int *o;
	DYNBLOCK *b;
	union
	{
		DYNBLOCK *n;
		int o;
	} u;
	union
	{
		DYNBLOCK **b;
		int i;
	} v;
	union
	{
		int *new;
		int j;
	} w;

	if(!old)
	{
		for(b=*block;b;b=b->next)for(o=&b->firstfree;*o<b->size;
			o=&b->data[*o])if(b->data[*o+1]>=newsize)
		{
			u.o=*o;
			if(b->data[u.o+1]==newsize)*o=b->data[u.o];
			else
			{
				*o+=newsize;
				b->data[u.o+newsize]=b->data[u.o];
				b->data[u.o+newsize+1]=b->data[u.o+1]-newsize;
			}
			return b->data+u.o;
		}
		for(u.o=DFTDYNBLOCK;u.o<newsize;u.o<<=1)if(u.o<0)
		{
			printf("request %d too large\n",newsize);
			return NULL;
		}
		if(!(b=palloc(&cache.aux,sizeof(DYNBLOCK)+u.o*sizeof(int))))
			return NULL;
		b->size=u.o;
		if(newsize>LARGEDYNBLOCK)
		{
			for(v.b=block;*v.b;v.b=&((*v.b)->next));
			*v.b=b;
			b->next=NULL;
		}
		else
		{
			b->next=*block;
			*block=b;
		}
		b->last=b->data+b->size-1;
		b->firstfree=newsize;
		if(newsize<b->size)
		{
			b->data[newsize]=b->size;
			b->data[newsize+1]=b->size-newsize;
		}
		return b->data;
	}
	for(b=*block;b;b=b->next)
	{
		if(old<b->data||old>b->last)continue;
		w.j=(old-b->data);
		v.i=w.j+oldsize;
		for(o=&b->firstfree;*o<b->size;o=&b->data[*o])if(*o>v.i)break;
		else if(*o==v.i)
		{
			u.o=newsize-oldsize;
			if(b->data[*o+1]<u.o)break;
			if(b->data[*o+1]==u.o)
			{
				*o=b->data[*o];
				return old;
			}
			b->data[*o+u.o]=b->data[*o];
			b->data[*o+u.o+1]=b->data[*o+1]-u.o;
			*o+=u.o;
			return old;
		}
		else if(*o+b->data[*o+1]==w.j&&b->data[*o]!=v.i)
		{
			u.o=newsize-oldsize;
			if(b->data[*o+1]<u.o)break;
			if(b->data[*o+1]==u.o)
			{
				*o=b->data[*o];
				memmove(old-u.o,old,oldsize*sizeof(int));
				return old-u.o;
			}
			v.i=b->data[*o];
			w.j=b->data[*o+1];
			memmove(b->data+*o,old,oldsize*sizeof(int));
			b->data[*o+newsize]=v.i;
			b->data[*o+newsize+1]=w.j-u.o;
			w.new=b->data+*o;
			*o+=newsize;
			return w.new;
		}
		break;
	}
	for(u.n=*block;u.n;u.n=u.n->next)
		for(o=&u.n->firstfree;*o<u.n->size;o=&u.n->data[*o])
			if(u.n->data[*o+1]>=newsize)
	{
		w.j=*o;
		if(u.n->data[*o+1]==newsize)*o=u.n->data[*o];
		else
		{
			u.n->data[*o+newsize]=u.n->data[*o];
			u.n->data[*o+newsize+1]=u.n->data[*o+1]-newsize;
			*o+=newsize;
		}
		goto nxt;
	}
nxt:	if(!u.n)
	{
		w.j=0;
		for(v.i=DFTDYNBLOCK;v.i<newsize;v.i<<=1)if(v.i<0)
		{
			printf("request %d too large\n",newsize);
			return NULL;
		}
		if(!(u.n=palloc(&cache.aux,sizeof(DYNBLOCK)+v.i*sizeof(int))))
			return NULL;
		u.n->size=v.i;
		if(newsize>LARGEDYNBLOCK)
		{
			for(v.b=block;*v.b;v.b=&((*v.b)->next));
			*v.b=u.n;
			u.n->next=NULL;
		}
		else
		{
			u.n->next=*block;
			*block=u.n;
		}
		u.n->last=u.n->data+u.n->size-1;
		u.n->firstfree=newsize;
		if(newsize<u.n->size)
		{
			u.n->data[newsize]=u.n->size;
			u.n->data[newsize+1]=u.n->size-newsize;
		}
	}
	memcpy(u.n->data+w.j,old,oldsize*sizeof(int));
	w.new=u.n->data+w.j;
	v.i=(old-b->data);
	old[1]=oldsize;
	for(u.o=b->firstfree,o=&b->firstfree;;o=&b->data[*o])if(*o>v.i)
	{
		if(o==&b->firstfree)
		{
			u.o=*o;
			*o=v.i;
		}
		else
		{
			u.o=(o-b->data);
			if(u.o+b->data[u.o+1]==v.i)
			{
				v.i=u.o;
				u.o=*o;
				b->data[v.i+1]+=old[1];
				old=b->data+v.i;
			}
			else
			{
				u.o=*o;
				*o=v.i;
			}
		}
		if(u.o==b->size)old[0]=b->size;
		else if(v.i+old[1]==u.o)
		{
			old[0]=b->data[u.o];
			old[1]+=b->data[u.o+1];
		}
		else old[0]=u.o;
		break;
	}
	return w.new;
}

static int additem(BLOCK **block,WORDLIST *data,int item)
{
	int i;
	int s;
	int *tmp;

	switch(data->occurrence)
	{
	case 0:	data->u.page=item;
		data->occurrence=1;
		break;
	case 1:	i=data->u.page;
		if(!(data->u.pagelist=getblock(&block[0],4,DFTBLOCK/4)))
			return -1;
		data->u.pagelist[0]=i;
		data->u.pagelist[1]=item;
		data->occurrence=2;
		break;
	case 2:
	case 3:	data->u.pagelist[data->occurrence++]=item;
		break;
	case 4:	if(!(tmp=getblock(&block[1],8,DFTBLOCK/8)))return -1;
		for(i=0;i<4;i++)tmp[i]=data->u.pagelist[i];
		relblock(&block[0],data->u.pagelist,4);
		data->u.pagelist=tmp;
		data->u.pagelist[4]=item;
		data->occurrence=5;
		break;
	case 5:
	case 6:
	case 7:	data->u.pagelist[data->occurrence++]=item;
		break;
	case 8:	if(!(tmp=getblock(&block[2],16,DFTBLOCK/16)))return -1;
		for(i=0;i<8;i++)tmp[i]=data->u.pagelist[i];
		relblock(&block[1],data->u.pagelist,8);
		data->u.pagelist=tmp;
		data->u.pagelist[8]=item;
		data->occurrence=9;
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:data->u.pagelist[data->occurrence++]=item;
		break;
	case 16:if(!(tmp=getblock(&block[3],32,DFTBLOCK/32)))return -1;
		for(i=0;i<16;i++)tmp[i]=data->u.pagelist[i];
		relblock(&block[2],data->u.pagelist,16);
		data->u.pagelist=tmp;
		data->u.pagelist[16]=item;
		data->occurrence=17;
		break;
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:data->u.pagelist[data->occurrence++]=item;
		break;
	case 32:if(!(tmp=getdynblock((DYNBLOCK **)(&block[4]),NULL,0,64)))
			return -1;
		for(i=0;i<32;i++)tmp[i]=data->u.pagelist[i];
		relblock(&block[3],data->u.pagelist,32);
		data->u.pagelist=tmp;
		data->u.pagelist[32]=item;
		data->occurrence=33;
		break;
	default:if((i=data->occurrence)>=LARGEDYNBLOCK)
		{
			if(i>=8*LARGEDYNBLOCK)s=0x3fff;
			else s=0x3ff;
		}
		else s=0x3f;
		if(i&s)
		{
			data->u.pagelist[i]=item;
			data->occurrence=i+1;
			break;
		}
		if(!(tmp=getdynblock((DYNBLOCK **)(&block[4]),data->u.pagelist,
			i,i+s+1)))return -1;
		data->u.pagelist=tmp;
		data->u.pagelist[i]=item;
		data->occurrence=i+1;
		break;
	}
	return 0;
}

static TOPIC *addtocache(int catid,char *id)
{
	int l;
	unsigned int w;
	union
	{
		unsigned char *p;
		TOPIC *r;
	} u;

	for(w=0,l=1,u.p=id;*u.p;u.p++,l++)
	{
		w*=*u.p;
		w+=*u.p;
		w^=(w>>16);
	}
	if(!(u.r=palloc(&cache.main,l+sizeof(TOPIC))))return NULL;
	u.r->next=cache.id[w&LISTMASK];
	cache.id[w&LISTMASK]=u.r;
	u.r->catid=catid;
	u.r->u.list=NULL;
	strcpy(u.r->id,id);
	cache.total++;
	return u.r;
}

static DATALIST *addcacheitem(TOPIC *r,char *item)
{
	DATALIST *l;

	if(!(l=palloc(&cache.aux,strlen(item)+1+sizeof(DATALIST))))return NULL;
	l->next=r->u.list;
	r->u.list=l;
	strcpy(l->item,item);
	return l;
}

static TOPIC *findbyid(char *id)
{
	unsigned int w;
	union
	{
		unsigned char *p;
		TOPIC *r;
	} u;

	for(w=0,u.p=id;*u.p;u.p++)
	{
		w*=*u.p;
		w+=*u.p;
		w^=(w>>16);
	}
	for(u.r=cache.id[w&LISTMASK];u.r;u.r=u.r->next)
		if(!strcmp(u.r->id,id))return u.r;
	return NULL;
}

static TOPIC *findbycatid(int catid,int *index)
{
	int current;
	int delta;

	current=cache.start;
	delta=cache.start;
	while(1)
	{
		if(!delta)
		{
			if(current<cache.total)
				if(cache.catid[current]->catid==catid)
			{
				if(index)*index=current;
				return cache.catid[current];
			}
			return NULL;
		}
		else if(current>=cache.total)
		{
			current-=delta;
			delta>>=1;
		}
		else if(cache.catid[current]->catid==catid)
		{
			if(index)*index=current;
			return cache.catid[current];
		}
		else if(cache.catid[current]->catid>catid)
		{
			if(!current)return NULL;
			current-=delta;
			delta>>=1;
		}
		else
		{
			current+=delta;
			delta>>=1;
		}
	}
}

static int findintree(int catid)
{
	int current;
	int delta;

	current=cache.treestart;
	delta=cache.treestart;
	while(1)
	{
		if(!delta)
		{
			if(current<cache.treetotal)
				if(cache.tree[current].src==catid)
			{
				for(;current;current--)
				    if(cache.tree[current-1].src!=catid)break;
				return current;
			}
			return -1;
		}
		else if(current>=cache.treetotal)
		{
			current-=delta;
			delta>>=1;
		}
		else if(cache.tree[current].src==catid)
		{
			for(;current;current--)
				if(cache.tree[current-1].src!=catid)break;
			return current;
		}
		else if(cache.tree[current].src>catid)
		{
			if(!current)return -1;
			current-=delta;
			delta>>=1;
		}
		else
		{
			current+=delta;
			delta>>=1;
		}
	}
}

static int mkstore(CONTENTDATA *u)
{
	if(!(u->store=palloc(&cache.aux,sizeof(STORE)+
		3*STOREBASE*sizeof(void *))))return -1;
	u->store->total=STOREBASE;
	u->store->used=0;
	u->store->dta16=(unsigned short **)(&u->store->dta32[STOREBASE]);
	u->store->dta8=(unsigned char **)(&u->store->dta16[STOREBASE]);
	return 0;
}

static int setstore(CONTENTDATA *u,int index,int v32,int v16,int v8)
{
	int high;

	if((high=index>>STORESHIFT)==u->store->used)
	{
		if(high==u->store->total)
		{
			int i;
			STORE *tmp;

			if(!(tmp=palloc(&cache.aux,sizeof(STORE)+3*
				(u->store->total<<1)*sizeof(void *))))return -1;
			tmp->total=u->store->total<<1;
			tmp->used=u->store->used;
			tmp->dta16=(unsigned short **)(&tmp->dta32[tmp->total]);
			tmp->dta8=(unsigned char **)(&tmp->dta16[tmp->total]);
			for(i=0;i<u->store->total;i++)
			{
				tmp->dta8[i]=u->store->dta8[i];
				tmp->dta16[i]=u->store->dta16[i];
				tmp->dta32[i]=u->store->dta32[i];
			}
			u->store=tmp;
		}
		u->store->used++;
		if(!(u->store->dta8[high]=palloc(&cache.aux,STORECHUNK*
			sizeof(unsigned char))))return -1;
		if(!(u->store->dta16[high]=palloc(&cache.aux,STORECHUNK*
			sizeof(unsigned short))))return -1;
		if(!(u->store->dta32[high]=palloc(&cache.aux,STORECHUNK*
			sizeof(int))))return -1;
		memset(u->store->dta8[high],0,STORECHUNK*sizeof(unsigned char));
		memset(u->store->dta16[high],0,
			STORECHUNK*sizeof(unsigned short));
		memset(u->store->dta32[high],0,STORECHUNK*sizeof(int));
	}
	index&=STOREMASK;
	u->store->dta8[high][index]=
		(v8>UCHAR_MAX?UCHAR_MAX:(unsigned char)(v8));
	u->store->dta16[high][index]=
		(v16>USHRT_MAX?USHRT_MAX:(unsigned short)(v16));
	u->store->dta32[high][index]=v32;
	return 0;
}

static void getstore(CONTENTDATA *u,int index,int *v32,int *v16,int *v8)
{
	int high=index>>STORESHIFT;
	int low=index&STOREMASK;

	*v8=(int)(u->store->dta8[high][low]);
	*v16=(int)(u->store->dta16[high][low]);
	*v32=u->store->dta32[high][low];
}

static int wsort(const void *p1,const void *p2)
{
	WORDLIST *w1=*((WORDLIST **)(p1));
	WORDLIST *w2=*((WORDLIST **)(p2));
	unsigned char *str1=w1->word;
	unsigned char *str2=w2->word;
	int len1=strlen(str1);
	int len2=strlen(str2);

	while(len1&&len2)
	{
		int res;
		unsigned int val1;
		unsigned int val2;

		if(!(res=utf8_to_ucs4(str1,len1,&val1)))goto utf8cmp;
		len1-=res;
		str1+=res;
		if(!(res=utf8_to_ucs4(str2,len2,&val2)))goto utf8cmp;
		len2-=res;
		str2+=res;
		val1=ucs4_normalize(val1);
		val2=ucs4_normalize(val2);
		if(val1<val2)return -1;
		if(val1>val2)return 1;
	}
	if(len1)return 1;
	if(len2)return -1;

utf8cmp:str1=w1->word;
	str2=w2->word;
	len1=strlen(str1);
	len2=strlen(str2);
	while(len1&&len2)
	{
		if(*str1<*str2)return -1;
		if(*str1++>*str2++)return 1;
		len1--;
		len2--;
	}
	if(len1)return 1;
	if(len2)return -1;

	if(w1->occurrence<w2->occurrence)return -1;
	else if(w1->occurrence>w2->occurrence)return 1;
	return 0;
}

static int psort(const void *p1,const void *p2)
{
	PAGEDATA *l1=((PAGEDATA *)(p1));
	PAGEDATA *l2=((PAGEDATA *)(p2));

	if(l1->weight>l2->weight)return -1;
	else if(l1->weight<l2->weight)return 1;
	else if(l1->pageid<l2->pageid)return -1;
	else if(l1->pageid>l2->pageid)return 1;
	else return 0;
}

static int treesort(const void *p1,const void *p2)
{
	TREE *t1=((TREE *)(p1));
	TREE *t2=((TREE *)(p2));

	if(t1->src<t2->src)return -1;
	else if(t1->src>t2->src)return 1;
	else if(t1->dst<t2->dst)return -1;
	else if(t1->dst>t2->dst)return 1;
	else return 0;
}

static int llsort(const void *p1,const void *p2)
{
	LANGLINK *l1=*((LANGLINK **)(p1));
	LANGLINK *l2=*((LANGLINK **)(p2));

	if(l1->src<l2->src)return -1;
	else if(l1->src>l2->src)return 1;
	else if(l1->dst<l2->dst)return -1;
	else if(l1->dst>l2->dst)return 1;
	else return 0;
}

static int ssort(const void *p1,const void *p2)
{
	SYMLINK *s1=*((SYMLINK **)(p1));
	SYMLINK *s2=*((SYMLINK **)(p2));

	if(s1->src<s2->src)return -1;
	else if(s1->src>s2->src)return 1;
	else if(s1->level<s2->level)return -1;
	else if(s1->level>s2->level)return 1;
	else if(s1->alias<s2->alias)return -1;
	else if(s1->alias>s2->alias)return 1;
	else return 0;
}

static int rsort(const void *p1,const void *p2)
{
	RELATED *r1=*((RELATED **)(p1));
	RELATED *r2=*((RELATED **)(p2));

	if(r1->src<r2->src)return -1;
	else if(r1->src>r2->src)return 1;
	else if(r1->dst<r2->dst)return -1;
	else if(r1->dst>r2->dst)return 1;
	else return 0;
}

static int nsort(const void *p1,const void *p2)
{
	NARROW *n1=*((NARROW **)(p1));
	NARROW *n2=*((NARROW **)(p2));

	if(n1->src<n2->src)return -1;
	else if(n1->src>n2->src)return 1;
	else if(n1->level<n2->level)return -1;
	else if(n1->level>n2->level)return 1;
	else if(n1->dst<n2->dst)return -1;
	else if(n1->dst>n2->dst)return 1;
	else return 0;
}

static int bsort(const void *p1,const void *p2)
{
	LETTERBAR *b1=*((LETTERBAR **)(p1));
	LETTERBAR *b2=*((LETTERBAR **)(p2));
	int cmp;

	if(b1->src<b2->src)return -1;
	else if(b1->src>b2->src)return 1;
	else if((cmp=strcmp(b1->title,b2->title))<0)return -1;
	else if(cmp>0)return 1;
	else if(b1->dst<b2->dst)return -1;
	else if(b1->dst>b2->dst)return 1;
	else return 0;
}

static int censort(const void *p1,const void *p2)
{
	CHEDNE *cen1=*((CHEDNE **)(p1));
	CHEDNE *cen2=*((CHEDNE **)(p2));

	if(cen1->catid<cen2->catid)return -1;
	else if(cen1->catid>cen2->catid)return 1;
	else if(cen1->type<cen2->type)return -1;
	else if(cen1->type>cen2->type)return 1;
	else if(cen1->id<cen2->id)return -1;
	else if(cen1->id>cen2->id)return 1;
	else return 0;
}

static int tsort(const void *p1,const void *p2)
{
	TOPICITEM *t1=*((TOPICITEM **)(p1));
	TOPICITEM *t2=*((TOPICITEM **)(p2));

	if(t1->topic->catid<t2->topic->catid)return -1;
	else if(t1->topic->catid>t2->topic->catid)return 1;
	else return 0;
}

static int rdsort(const void *p1,const void *p2)
{
	REDIRECT *r1=*((REDIRECT **)(p1));
	REDIRECT *r2=*((REDIRECT **)(p2));
	int cmp;

	if((cmp=strcmp(r1->id,r2->id))<0)return -1;
	else if(cmp>0)return 1;
	else if(r1->catid<r2->catid)return -1;
	else if(r1->catid>r2->catid)return 1;
	else return 0;
}

static int dsort(const void *p1,const void *p2)
{
	DESCRIPTION *d1=*((DESCRIPTION **)(p1));
	DESCRIPTION *d2=*((DESCRIPTION **)(p2));

	if(d1->catid<d2->catid)return -1;
	else if(d1->catid>d2->catid)return 1;
	else return 0;
}

static int ilsort(const void *p1,const void *p2)
{
	ITEMLIST *i1=*((ITEMLIST **)(p1));
	ITEMLIST *i2=*((ITEMLIST **)(p2));

	if(i1->catid<i2->catid)return -1;
	else if(i1->catid>i2->catid)return 1;
	else return strcmp(i1->data->item,i2->data->item);
}

static int trsort(const void *p1,const void *p2)
{
	TOPICTRACE *t1=*((TOPICTRACE **)(p1));
	TOPICTRACE *t2=*((TOPICTRACE **)(p2));

	if(t1->refid<t2->refid)return -1;
	else if(t1->refid>t2->refid)return 1;
	else if(t1->catid<t2->catid)return -1;
	else if(t1->catid>t2->catid)return 1;
	else return 0;
}

static int isort(const void *p1,const void *p2)
{
	int i1=*((int *)(p1));
	int i2=*((int *)(p2));

	if(i1<i2)return -1;
	else if(i1>i2)return 1;
	else return 0;
}

static int lsort(const void *p1,const void *p2)
{
	DATALIST *l1=*((DATALIST **)(p1));
	DATALIST *l2=*((DATALIST **)(p2));

	return strcmp(l1->item,l2->item);
}

static int asort(const void *p1,const void *p2)
{
	ALIAS *a1=*((ALIAS **)(p1));
	ALIAS *a2=*((ALIAS **)(p2));

	return strcmp(a1->id,a2->id);
}

static int parse(char *data,int len,int *line,int final,int checkutf,
	void *userdata,
	int (*datahandler)(char *,int,void *),
	int (*starthandler)(char *,char *,char *,void *),
	int (*endhandler)(char *,void *))
{
	int i;
	int j;
	int intag=0;
	int linestart=0;
	int endtag;
	int quoted;
	char *name;
	char *attr;
	char *value;

	for(i=0;i<len;i++)switch(data[i])
	{
	case '\r':
		if(intag)break;
		if(i+1==len)break;
		if(data[i+1]=='\n')break;
		if(verbose)printf("Warning: spurious CR on line %d\n",*line);
		data[i]='\n';
		(*line)--;
	case '\n':
		(*line)++;
		if(intag)break;
		if(checkutf)if(checkutf8data(data+linestart,i-linestart+1))
		{
			printf("utf8 error 1 on line %d\n",*line);
			return -1;
		}
		if(datahandler(data+linestart,i-linestart+1,userdata))
		{
			printf("processing aborted on line %d\n",*line);
			return -1;
		}
		linestart=i+1;
		break;
	case '<':
		if(intag)
		{
			printf("tagging error 1 on line %d\n",*line);
			return -1;
		}
		if(linestart!=i)if(datahandler(data+linestart,i-linestart,
			userdata))
		{
			printf("processing aborted on line %d\n",*line);
			return -1;
		}
		intag=1;
		linestart=i;
		break;
	case '>':
		if(checkutf)if(checkutf8data(data+linestart,i-linestart+1))
		{
			printf("utf8 error 2 on line %d\n",*line);
			return -1;
		}
		if(!intag)
		{
			printf("tagging error 2 on line %d\n",*line);
			return -1;
		}
		j=linestart+1;
		if(data[j]=='?'||data[j]=='!')
		{
			intag=0;
			linestart=i+1;
			break;
		}
		if(data[j]=='/')
		{
			endtag=1;
			j++;
		}
		else endtag=0;
		while(data[j]==' '||data[j]=='\t')j++;
		if(j==i)
		{
			printf("tagging error 3 on line %d\n",*line);
			return -1;
		}
		name=data+j;
		while(j<i&&data[j]!=' '&&data[j]!='\t')j++;
		if(j==i)
		{
			data[j]=0;
			if(!*name)
			{
				printf("tagging error 4 on line %d\n",*line);
				return -1;
			}
			if(!endtag)
			{
				if(starthandler(name,"","",userdata))
				{
					printf("processing aborted on "
						"line %d\n",*line);
					return -1;
				}
			}
			else if(endhandler(name,userdata))
			{
				printf("processing aborted on line %d\n",*line);
				return -1;
			}
			intag=0;
			linestart=i+1;
			break;
		}
		data[j++]=0;
		if(!*name)
		{
			printf("tagging error 5 on line %d\n",*line);
			return -1;
		}
		while(data[j]==' '||data[j]=='\t')j++;
		if(data[j]=='/'&&j+1==i)
		{
			if(!endtag)
			{
				if(starthandler(name,"","",userdata))
				{
					printf("processing aborted on "
						"line %d\n",*line);
					return -1;
				}
			}
			else if(endhandler(name,userdata))
			{
				printf("processing aborted on line %d\n",*line);
				return -1;
			}
			intag=0;
			linestart=i+1;
			break;
		}
		if(endtag)
		{
			printf("tagging error 6 on line %d\n",*line);
			return -1;
		}
		attr=data+j;
		while(j<i&&data[j]!=' '&&data[j]!='\t'&&data[j]!='=')j++;
		if(data[j]=='=')data[j++]=0;
		else
		{
			data[j++]=0;
			while(data[j]==' '||data[j]=='\t')j++;
			if(data[j++]!='=')
			{
				printf("tagging error 7 on line %d\n",*line);
				return -1;
			}
		}
		while(data[j]==' '||data[j]=='\t')j++;
		if(!*attr)
		{
			printf("tagging error 8 on line %d\n",*line);
			return -1;
		}
		if(data[j]=='\"')
		{
			quoted=1;
			j++;
		}
		else quoted=0;
		value=data+j;
		if(quoted)
		{
			while(j<i&&data[j]!='\"')j++;
			if(data[j]!='\"')
			{
				printf("tagging error 9 on line %d\n",*line);
				return -1;
			}
			data[j++]=0;
			if(starthandler(name,attr,value,userdata))
			{
				printf("processing aborted on line %d\n",*line);
				return -1;
			}
			intag=0;
			linestart=i+1;
			break;
		}
		while(j<i&&data[j]!=' '&&data[j]!='\t')j++;
		data[j]=0;
		if(starthandler(name,attr,value,userdata))
		{
			printf("processing aborted on line %d\n",*line);
			return -1;
		}
		intag=0;
		linestart=i+1;
		break;
	}

	if(!final)return linestart;
	if(intag)
	{
		printf("tagging error 10 on line %d\n",*line);
		return -1;
	}
	if(i!=linestart)
	{
		if(data[i-1]=='\r')
		{
			if(verbose)
			    printf("Warning: spurious CR on line %d\n",*line);
			data[i-1]='\n';
		}
		if(datahandler(data+linestart,i-linestart,userdata))
		{
			printf("processing aborted on line %d\n",*line);
			return -1;
		}
	}
	return i;
}

static int srccpy(char *dst,char *src,int max)
{
	do
	{
		if(!max--)
		{
			printf("input too long\n");
			return -1;
		}
		if(*src=='&')
		{
			char *replaced=NULL;

			if(!strncmp(src,"&amp;",5))
			{
				src+=4;
				replaced=src;
				*src='&';
			}
			if(src[1]=='#')
			{
			    unsigned int v=0;
			    int j;
			    char utf[7];

			    if(src[2]=='x')for(j=3;src[j]&&src[j]!=';';j++)
			    {
				v<<=4;
				if(src[j]>='0'&&src[j]<='9')v|=src[j]-'0';
				else if(src[j]>='a'&&src[j]<='f')
					v|=src[j]-'a'+10;
				else if(src[j]>='A'&&src[j]<='F')
					v|=src[j]-'A'+10;
				else
				{
					printf("hex conv. error\n");
					if(replaced)*replaced=';';
					return -1;
				}
			    }
			    else for(j=2;src[j]&&src[j]!=';';j++)
				if(src[j]>='0'&&src[j]<='9')v=v*10+src[j]-'0';
			    else
			    {
				printf("decimal conversion error\n");
				if(replaced)*replaced=';';
				return -1;
			    }
			    if(src[j]!=';'||src[j-1]=='#'||src[j-1]=='x'||!v)
			    {
				printf("conversion error\n");
				if(replaced)*replaced=';';
				return -1;
			    }
			    utf[ucs4_to_utf8(&v,utf)]=0;
			    src+=j;
			    max++;
			    for(j=0;utf[j];j++)
			    {
				if(!max--)
				{
					printf("input too long\n");
					if(replaced)*replaced=';';
					return -1;
				}
				*dst++=utf[j];
			    }
			}
			else
			{
			    int current=ENTISTART;
			    int delta=ENTISTART;

			    while(1)
			    {
				int l;

				if(!delta)
				{
					if(current<ENTITIES)
					  if(!strncmp(deent[current].entity,src,
					    deent[current].len))break;
					current=-1;
					break;
				}
				else if(current>=ENTITIES)
				{
					current-=delta;
					delta>>=1;
				}
				else if(!(l=strncmp(deent[current].entity,src,
					deent[current].len)))break;
				else if(l>0)
				{
					if(!current)
					{
						current=-1;
						break;
					}
					current-=delta;
					delta>>=1;
				}
				else
				{
					current+=delta;
					delta>>=1;
				}
			    }
			    if(current!=-1)
			    {
				const char *utf;
	
				src+=deent[current].len-1;
				utf=deent[current].utf8;
				max++;
				while(*utf)
				{
					if(!max--)
					{
						printf("input too long\n");
						if(replaced)*replaced=';';
						return -1;
					}
					*dst++=*utf++;
				}
			    }
			    else *dst++=*src;
			}
			if(replaced)*replaced=';';
		}
		else *dst++=*src;
	} while(*src++);
	return 0;
}

static int getmediadate(CONTENTDATA *u)
{
	char *p;
	int i;
	int j;
	char year[5];
	char month[3];
	char day[3];
	char bfr[BUFSIZET];

	year[0]=0;
	month[0]=0;
	day[0]=0;
	if(srccpy(bfr,u->cdata,BUFSIZET))return -1;
	for(p=bfr;*p;)
	{
		while(*p)if(*p>='0'&&*p<='9')break;
		else if(*p>='a'&&*p<='z')break;
		else if(*p>='A'&&*p<='Z')break;
		else p++;
		if(!*p)break;
		if((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z'))
		{
			for(i=1;(p[i]>='a'&&p[i]<='z')||(p[i]>='A'&&p[i]<='Z');
				i++);
			if(!month[0])
			{
				for(j=0;months[j].name;j++)
					if(!strncasecmp(months[j].name,p,i))
				{
					strcpy(month,months[j].num);
					break;
				}
				if(!months[j].name)month[0]=1;
			}
			p+=i;
		}
		else
		{
			for(i=1;p[i]>='0'&&p[i]<='9';i++);
			if(i==8)
			{
				if(p[0]!='0'||p[1]!='0'||p[2]!='0'||p[3]!='0')
				{
					if(!year[0])
					{
						for(j=0;j<4;j++)year[j]=p[j];
						year[j]=0;
					}
					else year[0]=1;
				}
				else if(!year[0])year[0]=1;
				if(p[4]!='0'||p[5]!='0')
				{
					if(!month[0])
					{
						month[0]=p[4];
						month[1]=p[5];
						month[2]=0;
					}
					else year[0]=1;
				}
				else if(!month[0])month[0]=1;
				if(p[6]!='0'||p[7]!='0')
				{
					if(!day[0])
					{
						day[0]=p[6];
						day[1]=p[7];
						day[2]=0;
					}
					else year[0]=1;
				}
				else if(!day[0])day[0]=1;
			}
			else if(i==6)
			{
				if(p[0]!='0'||p[1]!='0'||p[2]!='0'||p[3]!='0')
				{
					if(!year[0])
					{
						for(j=0;j<4;j++)year[j]=p[j];
						year[j]=0;
					}
					else year[0]=1;
				}
				else if(!year[0])year[0]=1;
				if(p[4]!='0'||p[5]!='0')
				{
					if(!month[0])
					{
						month[0]=p[4];
						month[1]=p[5];
						month[2]=0;
					}
					else year[0]=1;
				}
				else if(!month[0])month[0]=1;
			}
			else if(i==4)
			{
				if(p[0]!='0'||p[1]!='0'||p[2]!='0'||p[3]!='0')
				{
					if(!year[0])
					{
						if(day[0]>='0'&&month[0]>='0')
						{
							char tmp;

							tmp=day[0];
							day[0]=month[0];
							month[0]=tmp;
							tmp=day[1];
							day[1]=month[1];
							month[1]=tmp;
						}
						for(j=0;j<4;j++)year[j]=p[j];
						year[j]=0;
					}
				}
				else if(!year[0])year[0]=1;
			}
			else if(i==2)
			{
				if(p[0]!='0'||p[1]!='0')
				{
					if(!month[0])
					{
						month[0]=p[0];
						month[1]=p[1];
						month[2]=0;
					}
					else if(!day[0])
					{
						day[0]=p[0];
						day[1]=p[1];
						day[2]=0;
					}
				}
				else
				{
					if(!month[0])month[0]=1;
					else if(!day[0])day[0]=1;
				}
			}
			else if(i==1)
			{
				if(p[0]!='0')
				{
					if(!month[0])
					{
						month[0]='0';
						month[1]=p[1];
						month[2]=0;
					}
					else if(!day[0])
					{
						day[0]='0';
						day[1]=p[1];
						day[2]=0;
					}
				}
				else
				{
					if(!month[0])month[0]=1;
					else if(!day[0])day[0]=1;
				}
			}
			else year[0]=1;
			p+=i;
		}
	}

	if(month[0]>='0')i=atoi(month);
	else i=0;
	if(day[0]>='0')j=atoi(day);
	else j=0;
	if((i>12&&i<=31&&j>0&&j<=12)||(!j&&i>12&&i<=31))
	{
		int k;
		char tmp;

		tmp=day[0];
		day[0]=month[0];
		month[0]=tmp;
		tmp=day[1];
		day[1]=month[1];
		month[1]=tmp;
		k=i;
		i=j;
		j=k;
	}
	if(i>12||j>31)year[0]=1;

	if(year[0]==1||(year[0]<'0'&&(month[0]||day[0]))||
		(month[0]==1&&day[0]>1))
	{
		if(verbose)
			printf("Warning: can't format mediadate %s\n",u->cdata);
#ifdef SQLFLAG_MULTIROW
		u->date[u->idx][0]=0;
#else
		u->date[0]=0;
#endif
		return 0;
	}

	if(year[0]<'0')
	{
#ifdef SQLFLAG_MULTIROW
		u->date[u->idx][0]=0;
#else
		u->date[0]=0;
#endif
		return 0;
	}

#ifdef SQLFLAG_MULTIROW
	strcpy(u->date[u->idx],year);
#else
	strcpy(u->date,year);
#endif
	if(month[0]>1)
	{
#ifdef SQLFLAG_MULTIROW
		strcat(u->date[u->idx],"/");
		strcat(u->date[u->idx],month);
#else
		strcat(u->date,"/");
		strcat(u->date,month);
#endif
	}
	if(day[0]>1)
	{
#ifdef SQLFLAG_MULTIROW
		strcat(u->date[u->idx],"/");
		strcat(u->date[u->idx],day);
#else
		strcat(u->date,"/");
		strcat(u->date,day);
#endif
	}
	return 0;
}

static void tagcheck(char *str,char *label)
{
	unsigned char *p;

	for(p=str;*p;p++)if(*p=='<')
	{
	    unsigned char *q;

	    for(q=p+1;*q;q++)if(*q=='<')p=q;
	    else if(*q=='>'&&q!=p+1&&strncmp(p,"<->",3)&&strncmp(p,"<=>",3)&&
		strncmp(p,"<*>",3)&&strncmp(p,"< >",3))
	    {
		unsigned char *r;

		for(r=p+1;r!=q;r++)if(*r>=0x80)break;

		if(r==q)
		{
		    printf("Warning: possibly illegal html in %s: %s\n",
			label,str);
		    return;
		}
	    }
	}
}

static int getdescription(char *dst,int dstlen,char *src)
{
	unsigned char tmp[BUFSIZEH];

	if(srccpy(tmp,src,sizeof(tmp)))
	{
		printf("description %s too long\n",src);
		return -1;
	}
	if(htmlify(dst,dstlen,tmp))
	{
		printf("description %s processing failed\n",src);
		return -1;
	}
	return 0;
}

static void fixcontent(CONTENTDATA *u)
{
	int i;

	for(i=0;i<2;i++)
	{
		char *q;

#ifdef SQLFLAG_MULTIROW
		if(i)q=u->desc[u->idx];
		else q=u->title[u->idx];
#else
		if(i)q=u->desc;
		else q=u->title;
#endif

		while(*q)if(*q=='<')
		{
			int j=0;
			char *r;

			for(r=q+1;*r==' ';r++);
			if(!strncasecmp(r,"/ExternalPage>",14))j=14;
			else if(!strncasecmp(r,"/strong>",8))j=8;
			else if(!strncasecmp(r,"strong>",7))j=7;
			else if(!strncasecmp(r,"/title>",7))j=7;
			else if(!strncasecmp(r,"/cite>",6))j=6;
			else if(!strncasecmp(r,"cite>",5))j=5;
			else if(!strncasecmp(r,"br>",3))j=3;
			else if(!strncasecmp(r,"/a>",3))j=3;
			else if(!strncasecmp(r,"/i>",3))j=3;
			else if(!strncasecmp(r,"i>",2))j=2;
			if(j)memmove(q,r+j,strlen(r+j)+1);
			else q++;
		}
		else q++;
	}
}

static int addlink(CONTENTDATA *u,char *target,int type,int level)
{
	int l;
	CONTENTLIST *n;

	l=strlen(target)+1;
	if(!(n=palloc(&cache.lcl,sizeof(CONTENTLIST)+l)))return -1;
	n->linktype=type;
	n->linklevel=level;
	if(srccpy(n->link,target,l))return -1;
	n->next=u->clist;
	u->clist=n;
	return 0;
}

static int buildlist(DATALIST **u,char *p,int mode,int pool)
{
	int i;
	DATALIST *r;
	char bfr[BUFSIZEM];

	if(srccpy(bfr,p,sizeof(bfr)))
	{
		printf("%s too long for list\n",p);
		return 0;
	}
	for(i=1,r=*u;r;r=r->next,i++)
		if(!(mode?strcmp(r->item,bfr):strcasecmp(r->item,bfr)))return i;
	if(!(r=palloc(pool?&cache.aux:&cache.lcl,
		sizeof(DATALIST)+strlen(bfr)+1)))return 0;
	strcpy(r->item,bfr);
	r->next=NULL;
	for(;*u;u=&((*u)->next));
	*u=r;
	return i;
}

static int process_string(const unsigned char *p,int count,int start,int total,
	int flags,WORDLIST **data,WORDLIST **list)
{
	int j;
	int l;
	int m;
	int current;
	int delta;
	unsigned int v;
	unsigned int w;
	unsigned char *q;
	WORDLIST *r;
	unsigned char word[MAXWORD*6+1];
	unsigned int wrk[BUFSIZEH];

	if(!p)return count;
	m=0;
	l=strlen(p);
	while(l)
	{
		if(!(j=utf8_to_ucs4((unsigned char *)(p),l,&v)))
		{
			printf("input conversion error\n");
			return -1;
		}
		p+=j;
		l-=j;
		v=ucs4_normalize(v);
		if(v<0x80)if(!(v>='0'&&v<='9')&&!(v>='a'&&v<='z'))v=0x20;
		wrk[m++]=v;
		if(m==BUFSIZEH)
		{
			printf("input overflow\n");
			return -1;
		}
	}
	wrk[m]=0;
	for(j=0;wrk[j];j++)
	{
		if(wrk[j]==0x20)continue;
		for(l=0,m=j;wrk[j]&&wrk[j]!=0x20;j++,l++);
		v=wrk[j];
		wrk[j]=0;
		if(wrk[m]<'0'||wrk[m]>'9')if(l>=MINWORD&&l<=MAXWORD)
		{
			for(q=word;m<j;m++)
			{
				if(!(l=ucs4_to_utf8(wrk+m,q)))
				{
					printf("conversion error\n");
					return -1;
				}
				q+=l;
			}
			*q=0;
			current=start;
			delta=start;
			while(1)
			{
				if(!delta)
				{
					if(current<total)
					  if(!strcmp(stopwords[current],word))
					    break;
					current=-1;
					break;
				}
				else if(current>=total)
				{
					current-=delta;
					delta>>=1;
				}
				else if(!(l=strcmp(stopwords[current],word)))
					break;
				else if(l>0)
				{
					if(!current)
					{
						current=-1;
						break;
					}
					current-=delta;
					delta>>=1;
				}
				else
				{
					current+=delta;
					delta>>=1;
				}
			}
			if(current==-1)
			{
				for(w=0,p=word;*p;p++)
				{
					w*=*p;
					w+=*p;
					w^=(w>>16);
				}
				for(r=list[w&LISTMASK];r;r=r->next)
					if(!strcmp(r->word,word))break;
				if(!r)
				{
					if(!(r=palloc(&cache.aux,
						sizeof(WORDLIST)
						+strlen(word)+1)))return -1;
					r->next=list[w&LISTMASK];
					list[w&LISTMASK]=r;
					r->occurrence=0;
					strcpy(r->word,word);
				}
				for(m=0;m<count;m++)if(data[m]==r)break;
				if(m==count)
				{
					if(count==DATASIZE)
					{
						printf("overwords\n");
						return -1;
					}
					r->flags=flags;
					data[count++]=r;
				}
				else r->flags|=flags;
			}
		}
		if(!(wrk[j]=v))break;
	}
	return count;
}

static int update_pagesearch(CONTENTDATA *u)
{
	int i;
	int m;
	WORDLIST *data[DATASIZE];

#ifdef SQLFLAG_MULTIROW
	if((i=process_string(u->title[u->idx],0,u->start,u->total,INTITLE,data,
		u->list))==-1)return -1;
	if((i=process_string(u->desc[u->idx],i,u->start,u->total,INDESCR,data,
		u->list))==-1)return -1;
	if(setstore(u,u->count+u->idx-1,u->catid[u->idx],i,u->ages))return -1;
	for(m=0;m<i;m++)if(additem(u->block,data[m],
		(u->count+u->idx-1)|(u->priority[u->idx]?PRIORITY:0)|
		data[m]->flags))return -1;
#else
	if((i=process_string(u->title,0,u->start,u->total,INTITLE,data,u->list))
		==-1)return -1;
	if((i=process_string(u->desc,i,u->start,u->total,INDESCR,data,u->list))
		==-1)return -1;
	if(setstore(u,u->count-1,u->catid,i,u->ages))return -1;
	for(m=0;m<i;m++)if(additem(u->block,data[m],
		(u->count-1)|(u->priority?PRIORITY:0)|data[m]->flags))return -1;
#endif
	return 0;
}

static int donarrow(TOPICDATA *u,char *attr,char *value,int l)
{
	NARROW *n;
	TOPIC *r;
	char bfr[BUFSIZEM];

	if(!strcasecmp(attr,"r:resource"))
	{
		if(!u->id[0]||!value[0])
		{
			printf("missing narrow data\n");
			return -1;
		}
		if(srccpy(bfr,value,sizeof(bfr)))
		{
			printf("narrow %s too long\n",value);
			return -1;
		}
		if(!u->catid)
		{
			if(!(r=findbyid(u->id)))
			{
				printf("id %s not found\n",value);
				return -1;
			}
			else u->catid=r->catid;
		}
		if(!(r=findbyid(bfr)))
		{
			if(verbose)printf("Warning: narrowing target "
				"%s not in database\n",u->target);
		}
		else
		{
			if(!(n=palloc(&cache.aux,sizeof(NARROW))))return -1;
			n->next=u->nchain;
			u->nchain=n;
			n->src=u->catid;
			n->dst=r->catid;
			n->level=l;
			u->total6++;
		}
	}
	return 0;
}

static int dosym(TOPICDATA *u,char *attr,char *value,int l)
{
	int j;
	int k;
	int cmp;
	TOPIC *r;
	SYMLINK *s;
	char bfr[BUFSIZEM];

	if(!strcasecmp(attr,"r:resource"))
	{
		if(!u->id[0]||!value[0])
		{
			printf("missing symbolic data\n");
			return -1;
		}
		if(srccpy(bfr,value,sizeof(bfr)))
		{
			printf("narrow %s too long\n",value);
			return -1;
		}
		if(!u->catid)
		{
			if(!(r=findbyid(u->id)))
			{
				printf("id %s not found\n",u->id);
				return -1;
			}
			else u->catid=r->catid;
		}
		j=u->start2;
		k=u->start2;
		while(1)
		{
			if(!k)
			{
				if(j<u->total2)if(!strcmp(u->aliases[j]->id,
					bfr))k=u->aliases[j]->target;
				break;
			}
			else if(j>=u->total2)
			{
				j-=k;
				k>>=1;
			}
			else if(!(cmp=strcmp(u->aliases[j]->id,bfr)))
			{
				k=u->aliases[j]->target;
				break;
			}
			else if(cmp>0)
			{
				if(!j)
				{
					k=0;
					break;
				}
				j-=k;
				k>>=1;
			}
			else
			{
				j+=k;
				k>>=1;
			}
		}
		if(k)
		{
			if(!(s=palloc(&cache.aux,sizeof(SYMLINK))))return -1;
			s->next=u->schain;
			u->schain=s;
			s->src=u->catid;
			s->dst=k;
			s->alias=j+1;
			s->level=l;
			u->total4++;
		}
		else if(verbose)printf("Warning: symbolic target %s not in "
			"database\n",value);
	}
	return 0;
}

static int topicstart(char *name,char *attr,char *value,void *_u)
{
	int j;
	int k;
	int delta;
	int cmp;
	char *p;
	TOPICDATA *u;
	TOPIC *r;
	LANGLINK *l;
	CHEDNE *cen;
	RELATED *rel;
	LETTERBAR *b;
	char bfr[BUFSIZEM];

	u=(TOPICDATA *)(_u);
	if(!strcasecmp(name,"Topic"))
	{
		u->catid=0;
		u->id[0]=0;
		u->title[0]=0;
		u->lastupdate[0]=0;
		u->target[0]=0;
		u->aolsearch[0]=0;
		u->dispname[0]=0;
		u->desc[0]=0;
		u->clen=0;
		u->in_topic=1;
		if(!u->mode||u->mode==1||u->mode==3)if(!strcasecmp(attr,"r:id"))
			if(srccpy(u->id,value,sizeof(u->id)))
		{
			printf("id %s too long\n",value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"editor"))
	{
		if(u->in_topic&&u->mode==1)
		{
			if(!strcasecmp(attr,"r:resource"))
			{
				if(checkutf8unicodestr(value))
				{
					if(verbose)printf("Warning: invalid "
						"unicode in editor for catid "
						"%d\n",u->catid);
				}
				else if(!(j=buildlist(&u->list2,value,1,0)))
					return -1;
				else
				{
					if(!u->catid)
					{
						if(!(r=findbyid(u->id)))
						{
							printf("id %s not "
								"found\n",
								u->id);
							return -1;
						}
						else u->catid=r->catid;
					}
					if(!(cen=palloc(&cache.lcl,
						sizeof(CHEDNE))))return -1;
					cen->next=u->cenchain;
					u->cenchain=cen;
					cen->catid=u->catid;
					cen->id=j;
					cen->type=EDITOR;
					u->total4++;
				}
			}
		}
	}
	else if(!strcasecmp(name,"newsGroup"))
	{
		if(u->in_topic&&u->mode==1)
		{
			if(!strcasecmp(attr,"r:resource"))
			{
				if(checkutf8unicodestr(value))
				{
					if(verbose)printf("Warning: invalid "
						"unicode in newsgroup for "
						"catid %d\n",u->catid);
				}
				else if(!(j=buildlist(&u->list3,value,1,0)))
					return -1;
				else
				{
					if(!u->catid)
					{
						if(!(r=findbyid(u->id)))
						{
							printf("id %s not "
								"found\n",
								u->id);
							return -1;
						}
						else u->catid=r->catid;
					}
					if(!(cen=palloc(&cache.lcl,
						sizeof(CHEDNE))))return -1;
					cen->next=u->cenchain;
					u->cenchain=cen;
					cen->catid=u->catid;
					cen->id=j;
					cen->type=NEWSGROUP;
					u->total4++;
				}
			}
		}
	}
	else if(!strcasecmp(name,"altlang"))
	{
		if(u->in_topic&&u->mode==2)
		{
			if(!strcasecmp(attr,"r:resource"))
			{
				if(!(p=strchr(value,':')))
				{
					printf("illegal altlang %s\n",value);
					return -1;
				}
				*p=0;
				if(checkutf8unicodestr(value))
				{
					if(verbose)printf("Warning: invalid "
						"unicode in altlang for "
						"catid %d\n",u->catid);
				}
				else if(!buildlist(&u->list1,value,1,1))
					return -1;
				*p=':';
			}
		}
		else if(u->in_topic&&u->mode==3)
		{
			if(!strcasecmp(attr,"r:resource"))
			{
				if(!u->id[0]||!value)
				{
					printf("missing altlang data\n");
					return -1;
				}
				if(srccpy(bfr,value,sizeof(bfr)))
				{
					printf("altlang %s too long\n",value);
					return -1;
				}
				if(!(p=strchr(bfr,':')))
				{
					printf("illegal altlang %s\n",value);
					return -1;
				}
				if(!p[1])
				{
					printf("missing altlang data\n");
					return -1;
				}
				if(!u->catid)
				{
					if(!(r=findbyid(u->id)))
					{
						printf("id %s not found\n",
							u->id);
						return -1;
					}
					else u->catid=r->catid;
				}
				if(!(r=findbyid(p+1)))
				{
					if(verbose)printf("Warning: language "
						"target %s not in database\n",
						p+1);
					k=0;
				}
				else k=r->catid;
				if(checkutf8unicodestr(value))
				{
					if(verbose)printf("Warning: invalid "
						"unicode in altlang for "
						"catid %d\n",u->catid);
					k=0;
				}
				j=u->start;
				delta=u->start;
				*p=0;
				while(1)
				{
					if(!delta)
					{
						if(j<u->total)if(!strcmp(
						    u->array[j]->item,
						    bfr))break;
						printf("language %s not "
						  "found\n",value);
						return -1;
					}
					else if(j>=u->total)
					{
						j-=delta;
						delta>>=1;
					}
					else if(!(cmp=strcmp(u->array[j]->item,
						bfr)))break;
					else if(cmp>0)
					{
						if(!j)
						{
							printf("language %s not"
							  " found\n",value);
							return -1;
						}
						j-=delta;
						delta>>=1;
					}
					else
					{
						j+=delta;
						delta>>=1;
					}
				}
				if(k)
				{
					if(!(l=palloc(&cache.aux,
						sizeof(LANGLINK))))return -1;
					l->next=u->lchain;
					u->lchain=l;
					l->src=u->catid;
					l->dst=k;
					l->lang=j+1;
					u->total3++;
				}
			}
		}
	}
	else if(!strcasecmp(name,"symbolic"))
	{
		if(u->in_topic&&u->mode==3)if(dosym(u,attr,value,0))return -1;
	}
	else if(!strcasecmp(name,"symbolic1"))
	{
		if(u->in_topic&&u->mode==3)if(dosym(u,attr,value,1))return -1;
	}
	else if(!strcasecmp(name,"symbolic2"))
	{
		if(u->in_topic&&u->mode==3)if(dosym(u,attr,value,2))return -1;
	}
	else if(!strcasecmp(name,"related"))
	{
		if(u->in_topic&&u->mode==3)
		{
			if(!strcasecmp(attr,"r:resource"))
			{
				if(!u->id[0]||!value[0])
				{
					printf("missing related data\n");
					return -1;
				}
				if(srccpy(bfr,value,sizeof(bfr)))
				{
					printf("related %s too long\n",value);
					return -1;
				}
				if(!u->catid)
				{
					if(!(r=findbyid(u->id)))
					{
						printf("id %s not found\n",
							u->id);
						return -1;
					}
					else u->catid=r->catid;
				}
				if(!(r=findbyid(bfr)))
				{
					if(verbose)printf("Warning: related "
						"target %s not in database\n",
						value);
				}
				else
				{
					if(!(rel=palloc(&cache.aux,
						sizeof(RELATED))))return -1;
					rel->next=u->rchain;
					u->rchain=rel;
					rel->src=u->catid;
					rel->dst=r->catid;
					u->total5++;
				}
			}
		}
	}
	else if(!strcasecmp(name,"narrow"))
	{
		if(u->in_topic&&u->mode==3)
			if(donarrow(u,attr,value,0))return -1;
	}
	else if(!strcasecmp(name,"narrow1"))
	{
		if(u->in_topic&&u->mode==3)
			if(donarrow(u,attr,value,1))return -1;
	}
	else if(!strcasecmp(name,"narrow2"))
	{
		if(u->in_topic&&u->mode==3)
			if(donarrow(u,attr,value,2))return -1;
	}
	else if(!strcasecmp(name,"letterbar"))
	{
		if(u->in_topic&&u->mode==3)
		{
			if(!strcasecmp(attr,"r:resource"))
			{
				if(!u->id[0]||!value[0])
				{
					printf("missing letterbar data\n");
					return -1;
				}
				if(srccpy(bfr,value,sizeof(bfr)))
				{
					printf("letterbar %s too long\n",value);
					return -1;
				}
				if(!(p=strrchr(bfr,'/')))
				{
					printf("illegal letterbar data\n");
					return -1;
				}
				if(!p[1])
				{
					printf("missing letterbar data\n");
					return -1;
				}
				if(!u->catid)
				{
					if(!(r=findbyid(u->id)))
					{
						printf("id %s not found\n",
							u->id);
						return -1;
					}
					else u->catid=r->catid;
				}
				if(!(r=findbyid(value)))
				{
					if(verbose)
					    printf("Warning: letterbar target "
						"%s not in database\n",value);
				}
				else if(checkutf8unicodestr(p+1))
				{
					if(verbose)printf("Warning: invalid "
						"unicode in letterbar title "
						"for catid %d\n",u->catid);
				}
				else
				{
					if(!(b=palloc(&cache.aux,
						sizeof(LETTERBAR)+strlen(p))))
						return -1;
					b->next=u->bchain;
					u->bchain=b;
					b->src=u->catid;
					b->dst=r->catid;
					strcpy(b->title,p+1);
					u->total7++;
				}
			}
		}
	}
	else if(!strcasecmp(name,"Alias"))
	{
		u->catid=0;
		u->id[0]=0;
		u->title[0]=0;
		u->lastupdate[0]=0;
		u->target[0]=0;
		u->aolsearch[0]=0;
		u->dispname[0]=0;
		u->desc[0]=0;
		u->clen=0;
		u->in_alias=1;
		if(u->mode==2)if(!strcasecmp(attr,"r:id"))
			if(srccpy(u->id,value,sizeof(u->id)))
		{
			printf("alias id %s too long\n",value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"Target"));
	{
		if(u->in_alias&&u->mode==2)if(!strcasecmp(attr,"r:resource"))
			if(srccpy(u->target,value,sizeof(u->target)))
		{
			printf("target %s too long\n",value);
			return -1;
		}
	}
	u->clen=0;
	return 0;
}

static int topicend(char *name,void *_u)
{
	int i;
	TOPICDATA *u;
	TOPIC *r;
	ALIAS *a;
	DATALIST *l;
	ITEMLIST *il;
	DESCRIPTION *d;
	TOPICITEM *ti;
	CHEDNE *cen;

	u=(TOPICDATA *)(_u);
	if(!strcasecmp(name,"catid"))
	{
		if(u->in_topic)
		{
			u->cdata[u->clen]=0;
			i=atoi(u->cdata);
			if(i>0)u->catid=i;
		}
	}
	else if(!strcasecmp(name,"d:Title"))
	{
		u->cdata[u->clen]=0;
		if(srccpy(u->title,u->cdata,sizeof(u->title)))
		{
			printf("title %s too long\n",u->cdata);
			return -1;
		}
	}
	else if(!strcasecmp(name,"lastUpdate"))
	{
		if(u->in_topic&&!u->mode)
		{
			u->cdata[u->clen]=0;
			if(srccpy(u->lastupdate,u->cdata,
				sizeof(u->lastupdate)))
			{
				printf("lastupdate %s too long\n",u->cdata);
				return -1;
			}
			for(i=0;u->lastupdate[i];i++)if(u->lastupdate[i]=='-')
				u->lastupdate[i]='/';
		}
	}
	else if(!strcasecmp(name,"dispname"))
	{
		if(u->in_topic&&u->mode==1)
		{
			u->cdata[u->clen]=0;
			if(srccpy(u->dispname,u->cdata,sizeof(u->dispname)))
			{
				printf("dispname %s too long\n",u->cdata);
				return -1;
			}
		}
	}
	else if(!strcasecmp(name,"aolsearch"))
	{
		if(u->in_topic&&u->mode==1)
		{
			u->cdata[u->clen]=0;
			if(srccpy(u->aolsearch,u->cdata,sizeof(u->aolsearch)))
			{
				printf("aolsearch %s too long\n",u->cdata);
				return -1;
			}
		}
	}
	else if(!strcasecmp(name,"d:Description"))
	{
		if(u->in_topic&&u->mode==1)
		{
			u->cdata[u->clen]=0;
			if(getdescription(u->desc,sizeof(u->desc),u->cdata))
				return -1;
		}
	}
	else if(!strcasecmp(name,"d:charset"))
	{
		if(u->in_topic&&u->mode==1)
		{
			u->cdata[u->clen]=0;
			if(checkutf8unicodestr(u->cdata))
			{
				if(verbose)printf("Warning: invalid unicode in "
					"charset for catid %d\n",u->catid);
			}
			else if(!(i=buildlist(&u->list1,u->cdata,0,0)))
				return -1;
			else
			{
				if(!u->catid)
				{
					if(!(r=findbyid(u->id)))
					{
						printf("id %s not found\n",
							u->id);
						return -1;
					}
					else u->catid=r->catid;
				}
				if(all)
				{
					if(!(cen=palloc(&cache.lcl,
						sizeof(CHEDNE))))return -1;
					cen->next=u->cenchain;
					u->cenchain=cen;
					cen->catid=u->catid;
					cen->id=i;
					cen->type=CHARSET;
					u->total4++;
				}
			}
		}
	}
	else if(!strcasecmp(name,"Topic"))
	{
	    u->in_topic=0;
	    if(!u->mode)
	    {
		if(!u->catid||!u->id[0]||!u->title[0])
		{
			printf("missing topic data\n");
			return -1;
		}
		else if(checkutf8unicodestr(u->id))
		{
			if(verbose)printf("Warning: invalid unicode in "
				"id of catid %d\n",u->catid);
		}
		else if(checkutf8unicodestr(u->title))
		{
			if(verbose)printf("Warning: invalid unicode in "
				"title of catid %d\n",u->catid);
		}
		else if(checkutf8unicodestr(u->lastupdate))
		{
			if(verbose)printf("Warning: invalid unicode in "
				"lastupdate of catid %d\n",u->catid);
		}
		else
		{
			if(!(r=addtocache(u->catid,u->id)))return -1;
			if(!addcacheitem(r,u->title))return -1;
			i=strlen(u->title)+1;
			if(!(ti=palloc(&cache.lcl,sizeof(TOPICITEM)+i+
				strlen(u->lastupdate)+1)))return -1;
			ti->next=u->topicchain;
			u->topicchain=ti;
			ti->topic=r;
			ti->updateoffset=i;
			strcpy(ti->title,u->title);
			strcpy(ti->title+i,u->lastupdate);
			if(verbose)
			{
				tagcheck(u->id,"topic id");
				tagcheck(u->title,"topic title");
			}
		}
	    }
	    else if(u->mode==1&&(u->aolsearch[0]||u->dispname[0]||u->desc[0]))
	    {
		if(!u->catid)
		{
			printf("missing topic data\n");
			return -1;
		}
		if(!(r=findbycatid(u->catid,NULL)))
		{
			if(verbose)printf("Warning: target %s not in database",
				u->target);
		}
		else
		{
		    if(u->aolsearch[0])
		    {
			if(checkutf8unicodestr(u->aolsearch))
			{
			    if(verbose)printf("Warning: invalid unicode in "
				"aolsearch of catid %d\n",u->catid);
			}
			else
			{
			    if(!(l=addcacheitem(r,u->aolsearch)))return -1;
			    if(!(il=palloc(&cache.lcl,sizeof(ITEMLIST))))
				return -1;
			    il->next=u->aolchain;
			    u->aolchain=il;
			    il->data=l;
			    il->catid=u->catid;
			    u->total++;
			    if(verbose)tagcheck(u->aolsearch,"aolsearch");
			}
		    }
		    if(u->dispname[0])
		    {
			if(checkutf8unicodestr(u->dispname))
			{
			    if(verbose)printf("Warning: invalid unicode in "
				"dispname of catid %d\n",u->catid);
			}
			else
			{
			    if(!(l=addcacheitem(r,u->dispname)))return -1;
			    if(!(il=palloc(&cache.lcl,sizeof(ITEMLIST))))
				return -1;
			    il->next=u->dispchain;
			    u->dispchain=il;
			    il->data=l;
			    il->catid=u->catid;
			    u->total2++;
			    if(verbose)tagcheck(u->dispname,"dispname");
			}
		    }
		    if(u->desc[0])
		    {
			if(checkutf8unicodestr(u->desc))
			{
			    if(verbose)printf("Warning: invalid unicode in "
				"description of catid %d\n",u->catid);
			}
			else
			{
			    if(!(d=palloc(&cache.lcl,sizeof(DESCRIPTION)+
				strlen(u->desc)+1)))return -1;
			    d->next=u->descchain;
			    u->descchain=d;
			    d->catid=u->catid;
			    strcpy(d->desc,u->desc);
			    u->total3++;
			}
		    }
		}
	    }
	}
	else if(!strcasecmp(name,"Alias"))
	{
	    u->in_alias=0;
	    if(u->mode==2)
	    {
		if(!u->id[0]||!u->title[0]||!u->target[0])
		{
			printf("missing alias data\n");
			return -1;
		}
		if(!(r=findbyid(u->target)))
		{
			if(verbose)
			    printf("Warning: alias target %s not in database\n",
				u->target);
			u->catid=0;
		}
		else u->catid=r->catid;
		if(u->catid)
		{
			if(checkutf8unicodestr(u->id))
			{
				if(verbose)printf("Warning: invalid unicode in "
					"alias id for catid %d\n",u->catid);
				u->catid=0;
			}
			else if(checkutf8unicodestr(u->title))
			{
				if(verbose)printf("Warning: invalid unicode in "
					"alias title for catid %d\n",u->catid);
				u->catid=0;
			}
		}
		if(u->catid)
		{
			i=strlen(u->id)+1;
			if(!(a=palloc(&cache.aux,sizeof(ALIAS)+i
				+strlen(u->title)+1)))return -1;
			a->next=u->achain;
			u->achain=a;
			a->target=u->catid;
			a->titleoffset=i;
			strcpy(a->id,u->id);
			strcpy(a->id+i,u->title);
			u->total2++;
			if(verbose)
			{
				tagcheck(u->id,"alias id");
				tagcheck(u->title,"alias title");
			}
		}
	    }
	}
	else if(verbose)
	{
		if(!strcasecmp(name,"Target"));
		else if(!strcasecmp(name,"altlang"));
		else if(!strcasecmp(name,"related"));
		else if(!strcasecmp(name,"narrow"));
		else if(!strcasecmp(name,"narrow1"));
		else if(!strcasecmp(name,"narrow2"));
		else if(!strcasecmp(name,"letterbar"));
		else if(!strcasecmp(name,"symbolic"));
		else if(!strcasecmp(name,"symbolic1"));
		else if(!strcasecmp(name,"symbolic2"));
		else if(!strcasecmp(name,"editor"));
		else if(!strcasecmp(name,"newsGroup"));
		else if(!strcasecmp(name,"RDF"));
		else printf("Warning: unknown tag %s\n",name);
	}
	u->clen=0;
	return 0;
}

static int topicdata(char *s,int len,void *_u)
{
	TOPICDATA *u;

	u=(TOPICDATA *)(_u);
	if(len>1)if(s[len-2]=='\r')if(s[len-1]=='\n')
	{
		s[len-2]='\n';
		len--;
	}
	if(u->clen+len>=sizeof(u->cdata))
	{
		printf("data oversize: have %d, need %d\n",
			(int)sizeof(u->cdata),u->clen+len);
		return -1;
	}
	memcpy(u->cdata+u->clen,s,len);
	u->clen+=len;
	return 0;
}

static int get_structure1(char *topicfile,char *database,char *user,
	char *password)
{
	int len;
	int err;
	int i;
	int line;
	TOPICITEM **ti;
	ITEMLIST **il;
	DESCRIPTION **d;
	CHEDNE **c;
	DATALIST *r;
#ifdef SQLFLAG_MULTIROW
	char *rr[8];
	int j;
	int k;
#endif
	gzFile fp;
	TOPICDATA u;
	unsigned char bfr[BUFSIZEH];

	err=-1;
	u.catid=0;
	u.id[0]=0;
	u.title[0]=0;
	u.lastupdate[0]=0;
	u.target[0]=0;
	u.desc[0]=0;
	u.clen=0;
	u.in_topic=0;
	u.in_alias=0;
	u.mode=0;
	u.topicchain=NULL;
	if(sqlopen(database,user,password,&u.db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out0;
	if(!(fp=gzopen(topicfile,"r")))goto out1;
	line=1;
	i=0;
	while((len=gzread(fp,bfr+i,sizeof(bfr)-i))>0)
	{
		len+=i;
		if((i=parse(bfr,len,&line,0,1,&u,topicdata,topicstart,
			topicend))==-1)goto out2;
		memmove(bfr,bfr+i,len-i);
		i=len-i;
	}
	if(i)if(parse(bfr,i,&line,1,1,&u,topicdata,topicstart,topicend)==-1)
		goto out2;
	if(!(cache.catid=palloc(&cache.main,cache.total*sizeof(TOPIC *))))
		goto out2;
	if(!(ti=palloc(&cache.lcl,cache.total*sizeof(TOPICITEM *))))
		goto out2;
	for(i=0;u.topicchain;i++,u.topicchain=u.topicchain->next)
		ti[i]=u.topicchain;
	qsort(ti,cache.total,sizeof(TOPICITEM *),tsort);
	for(i=0;i<cache.total;i++)cache.catid[i]=ti[i]->topic;
	for(cache.start=1;cache.start>0&&cache.start<cache.total;
		cache.start<<=1);
	if(cache.start<=0)
	{
		printf("overflow error\n");
		goto out2;
	}
	else cache.start>>=1;
	if(sqlrun(u.db,TOPICSFMT))goto out2;
	if(sqllock(u.db,TOPICSTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,TOPICSPRP "," TOPICSROW "," TOPICSROW ","
		TOPICSROW "," TOPICSROW "," TOPICSROW "," TOPICSROW ","
		TOPICSROW,TOPICSINS TOPICSINS TOPICSINS TOPICSINS TOPICSINS
		TOPICSINS TOPICSINS TOPICSINS,NULL))goto out3;
	for(;i+8<=cache.total;i+=8)if(sqlinsert(u.db,u.sql,TOPICSINS TOPICSINS
		TOPICSINS TOPICSINS TOPICSINS TOPICSINS TOPICSINS TOPICSINS,
		ti[i]->topic->catid,ti[i]->topic->id,ti[i]->title,
		ti[i]->title+ti[i]->updateoffset,
		ti[i+1]->topic->catid,ti[i+1]->topic->id,ti[i+1]->title,
		ti[i+1]->title+ti[i+1]->updateoffset,
		ti[i+2]->topic->catid,ti[i+2]->topic->id,ti[i+2]->title,
		ti[i+2]->title+ti[i+2]->updateoffset,
		ti[i+3]->topic->catid,ti[i+3]->topic->id,ti[i+3]->title,
		ti[i+3]->title+ti[i+3]->updateoffset,
		ti[i+4]->topic->catid,ti[i+4]->topic->id,ti[i+4]->title,
		ti[i+4]->title+ti[i+4]->updateoffset,
		ti[i+5]->topic->catid,ti[i+5]->topic->id,ti[i+5]->title,
		ti[i+5]->title+ti[i+5]->updateoffset,
		ti[i+6]->topic->catid,ti[i+6]->topic->id,ti[i+6]->title,
		ti[i+6]->title+ti[i+6]->updateoffset,
		ti[i+7]->topic->catid,ti[i+7]->topic->id,ti[i+7]->title,
		ti[i+7]->title+ti[i+7]->updateoffset))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<cache.total)
#endif
	{
		if(sqlprep(u.db,&u.sql,TOPICSPRP,TOPICSINS,NULL))goto out3;
		for(;i<cache.total;i++)if(sqlinsert(u.db,u.sql,TOPICSINS,
			ti[i]->topic->catid,ti[i]->topic->id,ti[i]->title,
			ti[i]->title+ti[i]->updateoffset))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	gzclose(fp);
	u.catid=0;
	u.id[0]=0;
	u.title[0]=0;
	u.lastupdate[0]=0;
	u.target[0]=0;
	u.aolsearch[0]=0;
	u.dispname[0]=0;
	u.desc[0]=0;
	u.clen=0;
	u.in_topic=0;
	u.in_alias=0;
	u.mode=1;
	u.total=0;
	u.total2=0;
	u.total3=0;
	u.total4=0;
	u.aolchain=NULL;
	u.dispchain=NULL;
	u.descchain=NULL;
	u.cenchain=NULL;
	u.list1=NULL;
	u.list2=NULL;
	u.list3=NULL;
	if(!(fp=gzopen(topicfile,"r")))goto out1;
	line=1;
	i=0;
	while((len=gzread(fp,bfr+i,sizeof(bfr)-i))>0)
	{
		len+=i;
		if((i=parse(bfr,len,&line,0,1,&u,topicdata,topicstart,
			topicend))==-1)goto out2;
		memmove(bfr,bfr+i,len-i);
		i=len-i;
	}
	if(i)if(parse(bfr,i,&line,1,1,&u,topicdata,topicstart,topicend)==-1)
		goto out2;
	if(all)
	{
		if(!(il=palloc(&cache.lcl,u.total*sizeof(ITEMLIST *))))
			goto out2;
		for(i=0;i<u.total;i++,u.aolchain=u.aolchain->next)
			il[i]=u.aolchain;
		qsort(il,u.total,sizeof(ITEMLIST *),ilsort);
		if(sqlrun(u.db,AOLSEARCHFMT))goto out2;
		if(sqllock(u.db,AOLSEARCHTBL))goto out2;
		i=0;
#ifdef SQLFLAG_MULTIROW
		if(sqlprep(u.db,&u.sql,AOLSEARCHPRP "," AOLSEARCHROW ","
			AOLSEARCHROW "," AOLSEARCHROW "," AOLSEARCHROW ","
			AOLSEARCHROW "," AOLSEARCHROW "," AOLSEARCHROW,
			AOLSEARCHINS AOLSEARCHINS AOLSEARCHINS AOLSEARCHINS
			AOLSEARCHINS AOLSEARCHINS AOLSEARCHINS AOLSEARCHINS,
			NULL))goto out3;
		for(;i+8<=u.total;i+=8)if(sqlinsert(u.db,u.sql,AOLSEARCHINS
			AOLSEARCHINS AOLSEARCHINS AOLSEARCHINS AOLSEARCHINS
			AOLSEARCHINS AOLSEARCHINS AOLSEARCHINS,
			il[i]->catid,il[i]->data->item,
			il[i+1]->catid,il[i+1]->data->item,
			il[i+2]->catid,il[i+2]->data->item,
			il[i+3]->catid,il[i+3]->data->item,
			il[i+4]->catid,il[i+4]->data->item,
			il[i+5]->catid,il[i+5]->data->item,
			il[i+6]->catid,il[i+6]->data->item,
			il[i+7]->catid,il[i+7]->data->item))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
		if(i<u.total)
#endif
		{
			if(sqlprep(u.db,&u.sql,AOLSEARCHPRP,AOLSEARCHINS,NULL))
				goto out3;
			for(;i<u.total;i++)if(sqlinsert(u.db,u.sql,AOLSEARCHINS,
				il[i]->catid,il[i]->data->item))goto out4;
			if(sqlfin(u.db,u.sql))goto out3;
		}
		if(sqlunlock(u.db))goto out2;
		if(!(il=palloc(&cache.lcl,u.total2*sizeof(ITEMLIST *))))
			goto out2;
		for(i=0;i<u.total2;i++,u.dispchain=u.dispchain->next)
			il[i]=u.dispchain;
		qsort(il,u.total2,sizeof(ITEMLIST *),ilsort);
		if(sqlrun(u.db,DISPNAMEFMT))goto out2;
		if(sqllock(u.db,DISPNAMETBL))goto out2;
		i=0;
#ifdef SQLFLAG_MULTIROW
		if(sqlprep(u.db,&u.sql,DISPNAMEPRP "," DISPNAMEROW ","
			DISPNAMEROW "," DISPNAMEROW "," DISPNAMEROW ","
			DISPNAMEROW "," DISPNAMEROW "," DISPNAMEROW,
			DISPNAMEINS DISPNAMEINS DISPNAMEINS DISPNAMEINS
			DISPNAMEINS DISPNAMEINS DISPNAMEINS DISPNAMEINS,NULL))
			goto out3;
		for(;i+8<=u.total2;i+=8)if(sqlinsert(u.db,u.sql,DISPNAMEINS
			DISPNAMEINS DISPNAMEINS DISPNAMEINS DISPNAMEINS
			DISPNAMEINS DISPNAMEINS DISPNAMEINS,
			il[i]->catid,il[i]->data->item,
			il[i+1]->catid,il[i+1]->data->item,
			il[i+2]->catid,il[i+2]->data->item,
			il[i+3]->catid,il[i+3]->data->item,
			il[i+4]->catid,il[i+4]->data->item,
			il[i+5]->catid,il[i+5]->data->item,
			il[i+6]->catid,il[i+6]->data->item,
			il[i+7]->catid,il[i+7]->data->item))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
		if(i<u.total2)
#endif
		{
			if(sqlprep(u.db,&u.sql,DISPNAMEPRP,DISPNAMEINS,NULL))
				goto out3;
			for(;i<u.total2;i++)if(sqlinsert(u.db,u.sql,DISPNAMEINS,
				il[i]->catid,il[i]->data->item))goto out4;
			if(sqlfin(u.db,u.sql))goto out3;
		}
		if(sqlunlock(u.db))goto out2;
	}
	if(!(d=palloc(&cache.lcl,u.total3*sizeof(DESCRIPTION *))))goto out2;
	for(i=0;i<u.total3;i++,u.descchain=u.descchain->next)d[i]=u.descchain;
	qsort(d,u.total3,sizeof(DESCRIPTION *),dsort);
	if(sqlrun(u.db,DESCRIPTIONFMT))goto out2;
	if(sqllock(u.db,DESCRIPTIONTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,DESCRIPTIONPRP "," DESCRIPTIONROW ","
		DESCRIPTIONROW "," DESCRIPTIONROW "," DESCRIPTIONROW ","
		DESCRIPTIONROW "," DESCRIPTIONROW "," DESCRIPTIONROW,
		DESCRIPTIONINS DESCRIPTIONINS DESCRIPTIONINS DESCRIPTIONINS
		DESCRIPTIONINS DESCRIPTIONINS DESCRIPTIONINS DESCRIPTIONINS,
		NULL))goto out3;
	for(;i+8<=u.total3;i+=8)if(sqlinsert(u.db,u.sql,DESCRIPTIONINS
		DESCRIPTIONINS DESCRIPTIONINS DESCRIPTIONINS DESCRIPTIONINS
		DESCRIPTIONINS DESCRIPTIONINS DESCRIPTIONINS,
		d[i]->catid,d[i]->desc,d[i+1]->catid,d[i+1]->desc,
		d[i+2]->catid,d[i+2]->desc,d[i+3]->catid,d[i+3]->desc,
		d[i+4]->catid,d[i+4]->desc,d[i+5]->catid,d[i+5]->desc,
		d[i+6]->catid,d[i+6]->desc,d[i+7]->catid,d[i+7]->desc))
		goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total3)
#endif
	{
		if(sqlprep(u.db,&u.sql,DESCRIPTIONPRP,DESCRIPTIONINS,NULL))
			goto out3;
		for(;i<u.total3;i++)if(sqlinsert(u.db,u.sql,DESCRIPTIONINS,
			d[i]->catid,d[i]->desc))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	if(!(c=palloc(&cache.lcl,u.total4*sizeof(CHEDNE *))))goto out2;
	for(i=0;i<u.total4;i++,u.cenchain=u.cenchain->next)c[i]=u.cenchain;
	qsort(c,u.total4,sizeof(CHEDNE *),censort);
	if(sqlrun(u.db,CHEDNEFMT))goto out2;
	if(sqllock(u.db,CHEDNETBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,CHEDNEPRP "," CHEDNEROW "," CHEDNEROW ","
		CHEDNEROW "," CHEDNEROW "," CHEDNEROW "," CHEDNEROW ","
		CHEDNEROW,CHEDNEINS CHEDNEINS CHEDNEINS CHEDNEINS CHEDNEINS
		CHEDNEINS CHEDNEINS CHEDNEINS,NULL))goto out3;
	for(;i+8<=u.total4;i+=8)if(sqlinsert(u.db,u.sql,CHEDNEINS CHEDNEINS
		CHEDNEINS CHEDNEINS CHEDNEINS CHEDNEINS CHEDNEINS CHEDNEINS,
		i+1,c[i]->catid,c[i]->type,c[i]->id,
		i+2,c[i+1]->catid,c[i+1]->type,c[i+1]->id,
		i+3,c[i+2]->catid,c[i+2]->type,c[i+2]->id,
		i+4,c[i+3]->catid,c[i+3]->type,c[i+3]->id,
		i+5,c[i+4]->catid,c[i+4]->type,c[i+4]->id,
		i+6,c[i+5]->catid,c[i+5]->type,c[i+5]->id,
		i+7,c[i+6]->catid,c[i+6]->type,c[i+6]->id,
		i+8,c[i+7]->catid,c[i+7]->type,c[i+7]->id))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total4)
#endif
	{
		if(sqlprep(u.db,&u.sql,CHEDNEPRP,CHEDNEINS,NULL))goto out3;
		for(;i<u.total4;i++)if(sqlinsert(u.db,u.sql,CHEDNEINS,i+1,
			c[i]->catid,c[i]->type,c[i]->id))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	if(all)
	{
		if(sqlrun(u.db,CHARSETFMT))goto out2;
		if(sqllock(u.db,CHARSETTBL))goto out2;
#ifdef SQLFLAG_MULTIROW
		if(sqlprep(u.db,&u.sql,CHARSETPRP "," CHARSETROW "," CHARSETROW
			"," CHARSETROW "," CHARSETROW "," CHARSETROW ","
			CHARSETROW "," CHARSETROW,CHARSETINS CHARSETINS
			CHARSETINS CHARSETINS CHARSETINS CHARSETINS CHARSETINS
			CHARSETINS,NULL))goto out3;
		for(j=0,i=1,r=u.list1;r;r=r->next)
		{
			rr[j++]=r->item;
			if(j==8)
			{
				if(sqlinsert(u.db,u.sql,CHARSETINS CHARSETINS
					CHARSETINS CHARSETINS CHARSETINS
					CHARSETINS CHARSETINS CHARSETINS,
					i,rr[0],i+1,rr[1],i+2,rr[2],i+3,rr[3],
					i+4,rr[4],i+5,rr[5],i+6,rr[6],
					i+7,rr[7]))goto out4;
				i+=8;
				j=0;
			}
		}
		if(sqlfin(u.db,u.sql))goto out3;
		if(j)
		{
			if(sqlprep(u.db,&u.sql,CHARSETPRP,CHARSETINS,NULL))
				goto out3;
			for(k=0;k<j;k++)if(sqlinsert(u.db,u.sql,CHARSETINS,
				i++,rr[k]))goto out4;
			if(sqlfin(u.db,u.sql))goto out3;
		}
#else
		if(sqlprep(u.db,&u.sql,CHARSETPRP,CHARSETINS,NULL))goto out3;
		for(i=1,r=u.list1;r;r=r->next,i++)if(sqlinsert(u.db,u.sql,
			CHARSETINS,i,r->item))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
#endif
		if(sqlunlock(u.db))goto out2;
	}
	if(sqlrun(u.db,EDITORFMT))goto out2;
	if(sqllock(u.db,EDITORTBL))goto out2;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,EDITORPRP "," EDITORROW "," EDITORROW ","
		EDITORROW "," EDITORROW "," EDITORROW "," EDITORROW ","
		EDITORROW,EDITORINS EDITORINS EDITORINS EDITORINS EDITORINS
		EDITORINS EDITORINS EDITORINS,NULL))goto out3;
	for(j=0,i=1,r=u.list2;r;r=r->next)
	{
		rr[j++]=r->item;
		if(j==8)
		{
			if(sqlinsert(u.db,u.sql,EDITORINS EDITORINS EDITORINS
				EDITORINS EDITORINS EDITORINS EDITORINS
				EDITORINS,
				i,rr[0],i+1,rr[1],i+2,rr[2],i+3,rr[3],i+4,rr[4],
				i+5,rr[5],i+6,rr[6],i+7,rr[7]))goto out4;
			i+=8;
			j=0;
		}
	}
	if(sqlfin(u.db,u.sql))goto out3;
	if(j)
	{
		if(sqlprep(u.db,&u.sql,EDITORPRP,EDITORINS,NULL))goto out3;
		for(k=0;k<j;k++)if(sqlinsert(u.db,u.sql,EDITORINS,i++,rr[k]))
			goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
#else
	if(sqlprep(u.db,&u.sql,EDITORPRP,EDITORINS,NULL))goto out3;
	for(i=1,r=u.list2;r;r=r->next,i++)if(sqlinsert(u.db,u.sql,EDITORINS,
		i,r->item))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
#endif
	if(sqlunlock(u.db))goto out2;
	if(sqlrun(u.db,NEWSGROUPFMT))goto out2;
	if(sqllock(u.db,NEWSGROUPTBL))goto out2;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,NEWSGROUPPRP "," NEWSGROUPROW "," NEWSGROUPROW
		"," NEWSGROUPROW "," NEWSGROUPROW "," NEWSGROUPROW ","
		NEWSGROUPROW "," NEWSGROUPROW,NEWSGROUPINS NEWSGROUPINS
		NEWSGROUPINS NEWSGROUPINS NEWSGROUPINS NEWSGROUPINS NEWSGROUPINS
		NEWSGROUPINS,NULL))goto out3;
	for(j=0,i=1,r=u.list3;r;r=r->next)
	{
		rr[j++]=r->item;
		if(j==8)
		{
			if(sqlinsert(u.db,u.sql,NEWSGROUPINS NEWSGROUPINS
				NEWSGROUPINS NEWSGROUPINS NEWSGROUPINS
				NEWSGROUPINS NEWSGROUPINS NEWSGROUPINS,
				i,rr[0],i+1,rr[1],i+2,rr[2],i+3,rr[3],i+4,rr[4],
				i+5,rr[5],i+6,rr[6],i+7,rr[7]))goto out4;
			i+=8;
			j=0;
		}
	}
	if(sqlfin(u.db,u.sql))goto out3;
	if(j)
	{
		if(sqlprep(u.db,&u.sql,NEWSGROUPPRP,NEWSGROUPINS,NULL))
			goto out3;
		for(k=0;k<j;k++)if(sqlinsert(u.db,u.sql,NEWSGROUPINS,i++,rr[k]))
			goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
#else
	if(sqlprep(u.db,&u.sql,NEWSGROUPPRP,NEWSGROUPINS,NULL))goto out3;
	for(i=1,r=u.list3;r;r=r->next,i++)if(sqlinsert(u.db,u.sql,NEWSGROUPINS,
		i,r->item))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
#endif
	if(sqlunlock(u.db))goto out2;
	err=0;
out4:	if(err)sqlfin(u.db,u.sql);
out3:	if(err)sqlunlock(u.db);
out2:	gzclose(fp);
out1:	if(sqlclose(u.db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out0:	pfree(&cache.lcl);
	return err;
}

static int get_structure2(char *topicfile,char *database,char *user,
	char *password)
{
	int len;
	int err;
	int i;
	int line;
	int tot;
	gzFile fp;
	DATALIST *r;
	TREE *t;
	TOPICDATA u;
	unsigned char bfr[BUFSIZEH];

	err=-1;
	u.catid=0;
	u.id[0]=0;
	u.title[0]=0;
	u.lastupdate[0]=0;
	u.target[0]=0;
	u.desc[0]=0;
	u.clen=0;
	u.in_topic=0;
	u.in_alias=0;
	u.mode=2;
	u.list1=NULL;
	u.array=NULL;
	u.achain=NULL;
	u.lchain=NULL;
	u.schain=NULL;
	u.rchain=NULL;
	u.nchain=NULL;
	u.bchain=NULL;
	u.total=0;
	u.total2=0;
	u.total3=0;
	u.total4=0;
	u.total5=0;
	u.total6=0;
	u.total7=0;
	if(sqlopen(database,user,password,&u.db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out0;
	if(!(fp=gzopen(topicfile,"r")))goto out1;
	line=1;
	i=0;
	while((len=gzread(fp,bfr+i,sizeof(bfr)-i))>0)
	{
		len+=i;
		if((i=parse(bfr,len,&line,0,1,&u,topicdata,topicstart,
			topicend))==-1)goto out2;
		memmove(bfr,bfr+i,len-i);
		i=len-i;
	}
	if(i)if(parse(bfr,i,&line,1,1,&u,topicdata,topicstart,topicend)==-1)
		goto out2;
	for(u.start2=1;u.start2>0&&u.start2<u.total2;u.start2<<=1);
	if(u.start2<=0)
	{
		printf("overflow error\n");
		goto out2;
	}
	else u.start2>>=1;
	if(!(u.aliases=palloc(&cache.aux,u.total2*sizeof(ALIAS *))))goto out2;
	for(i=0;u.achain;u.achain=u.achain->next,i++)u.aliases[i]=u.achain;
	qsort(u.aliases,u.total2,sizeof(ALIAS *),asort);
	for(i=0,r=u.list1;r;r=r->next,i++)u.total++;
	for(u.start=1;u.start>0&&u.start<u.total;u.start<<=1);
	if(u.start<=0)
	{
		printf("overflow error\n");
		goto out2;
	}
	else u.start>>=1;
	if(!(u.array=palloc(&cache.aux,u.total*sizeof(DATALIST *))))goto out2;
	for(i=0;u.list1;u.list1=u.list1->next,i++)u.array[i]=u.list1;
	qsort(u.array,u.total,sizeof(DATALIST *),lsort);
	gzclose(fp);
	u.catid=0;
	u.id[0]=0;
	u.title[0]=0;
	u.lastupdate[0]=0;
	u.target[0]=0;
	u.desc[0]=0;
	u.clen=0;
	u.in_topic=0;
	u.in_alias=0;
	u.mode=3;
	if(!(fp=gzopen(topicfile,"r")))goto out1;
	line=1;
	i=0;
	while((len=gzread(fp,bfr+i,sizeof(bfr)-i))>0)
	{
		len+=i;
		if((i=parse(bfr,len,&line,0,1,&u,topicdata,topicstart,
			topicend))==-1)goto out2;
		memmove(bfr,bfr+i,len-i);
		i=len-i;
	}
	if(i)if(parse(bfr,i,&line,1,1,&u,topicdata,topicstart,topicend)==-1)
		goto out2;
	if(sqlrun(u.db,LANGUAGESFMT))goto out2;
	if(sqllock(u.db,LANGUAGESTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,LANGUAGESPRP "," LANGUAGESROW "," LANGUAGESROW
		"," LANGUAGESROW "," LANGUAGESROW "," LANGUAGESROW ","
		LANGUAGESROW "," LANGUAGESROW,LANGUAGESINS LANGUAGESINS
		LANGUAGESINS LANGUAGESINS LANGUAGESINS LANGUAGESINS LANGUAGESINS
		LANGUAGESINS,NULL))goto out3;
	for(;i+8<=u.total;i+=8)if(sqlinsert(u.db,u.sql,LANGUAGESINS
		LANGUAGESINS LANGUAGESINS LANGUAGESINS LANGUAGESINS
		LANGUAGESINS LANGUAGESINS LANGUAGESINS,
		i+1,u.array[i]->item,i+2,u.array[i+1]->item,
		i+3,u.array[i+2]->item,i+4,u.array[i+3]->item,
		i+5,u.array[i+4]->item,i+6,u.array[i+5]->item,
		i+7,u.array[i+6]->item,i+8,u.array[i+7]->item))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total)
#endif
	{
		if(sqlprep(u.db,&u.sql,LANGUAGESPRP,LANGUAGESINS,NULL))
			goto out3;
		for(;i<u.total;i++)if(sqlinsert(u.db,u.sql,LANGUAGESINS,i+1,
			u.array[i]->item))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	if(!(u.langlinks=palloc(&cache.aux,u.total3*sizeof(LANGLINK *))))
		goto out2;
	for(i=0;u.lchain;u.lchain=u.lchain->next,i++)u.langlinks[i]=u.lchain;
	qsort(u.langlinks,u.total3,sizeof(LANGLINK *),llsort);
	if(sqlrun(u.db,LANGLINKSFMT))goto out2;
	if(sqllock(u.db,LANGLINKSTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,LANGLINKSPRP "," LANGLINKSROW "," LANGLINKSROW
		"," LANGLINKSROW "," LANGLINKSROW "," LANGLINKSROW ","
		LANGLINKSROW "," LANGLINKSROW,LANGLINKSINS LANGLINKSINS
		LANGLINKSINS LANGLINKSINS LANGLINKSINS LANGLINKSINS LANGLINKSINS
		LANGLINKSINS,NULL))goto out3;
	for(;i+8<=u.total3;i+=8)if(sqlinsert(u.db,u.sql,LANGLINKSINS
		LANGLINKSINS LANGLINKSINS LANGLINKSINS LANGLINKSINS
		LANGLINKSINS LANGLINKSINS LANGLINKSINS,
		i+1,u.langlinks[i]->src,u.langlinks[i]->dst,
		u.langlinks[i]->lang,
		i+2,u.langlinks[i+1]->src,u.langlinks[i+1]->dst,
		u.langlinks[i+1]->lang,
		i+3,u.langlinks[i+2]->src,u.langlinks[i+2]->dst,
		u.langlinks[i+2]->lang,
		i+4,u.langlinks[i+3]->src,u.langlinks[i+3]->dst,
		u.langlinks[i+3]->lang,
		i+5,u.langlinks[i+4]->src,u.langlinks[i+4]->dst,
		u.langlinks[i+4]->lang,
		i+6,u.langlinks[i+5]->src,u.langlinks[i+5]->dst,
		u.langlinks[i+5]->lang,
		i+7,u.langlinks[i+6]->src,u.langlinks[i+6]->dst,
		u.langlinks[i+6]->lang,
		i+8,u.langlinks[i+7]->src,u.langlinks[i+7]->dst,
		u.langlinks[i+7]->lang))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total3)
#endif
	{
		if(sqlprep(u.db,&u.sql,LANGLINKSPRP,LANGLINKSINS,NULL))
			goto out3;
		for(;i<u.total3;i++)if(sqlinsert(u.db,u.sql,LANGLINKSINS,i+1,
			u.langlinks[i]->src,u.langlinks[i]->dst,
			u.langlinks[i]->lang))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	if(sqlrun(u.db,ALIASESFMT))goto out2;
	if(sqllock(u.db,ALIASESTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,ALIASESPRP "," ALIASESROW "," ALIASESROW ","
		ALIASESROW "," ALIASESROW "," ALIASESROW "," ALIASESROW ","
		ALIASESROW,ALIASESINS ALIASESINS ALIASESINS ALIASESINS
		ALIASESINS ALIASESINS ALIASESINS ALIASESINS,NULL))goto out3;
	for(;i+8<=u.total2;i+=8)if(sqlinsert(u.db,u.sql,ALIASESINS ALIASESINS
		ALIASESINS ALIASESINS ALIASESINS ALIASESINS ALIASESINS
		ALIASESINS,
		i+1,u.aliases[i]->target,u.aliases[i]->id,
		u.aliases[i]->id+u.aliases[i]->titleoffset,
		i+2,u.aliases[i+1]->target,u.aliases[i+1]->id,
		u.aliases[i+1]->id+u.aliases[i+1]->titleoffset,
		i+3,u.aliases[i+2]->target,u.aliases[i+2]->id,
		u.aliases[i+2]->id+u.aliases[i+2]->titleoffset,
		i+4,u.aliases[i+3]->target,u.aliases[i+3]->id,
		u.aliases[i+3]->id+u.aliases[i+3]->titleoffset,
		i+5,u.aliases[i+4]->target,u.aliases[i+4]->id,
		u.aliases[i+4]->id+u.aliases[i+4]->titleoffset,
		i+6,u.aliases[i+5]->target,u.aliases[i+5]->id,
		u.aliases[i+5]->id+u.aliases[i+5]->titleoffset,
		i+7,u.aliases[i+6]->target,u.aliases[i+6]->id,
		u.aliases[i+6]->id+u.aliases[i+6]->titleoffset,
		i+8,u.aliases[i+7]->target,u.aliases[i+7]->id,
		u.aliases[i+7]->id+u.aliases[i+7]->titleoffset))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total2)
#endif
	{
		if(sqlprep(u.db,&u.sql,ALIASESPRP,ALIASESINS,NULL))goto out3;
		for(;i<u.total2;i++)if(sqlinsert(u.db,u.sql,ALIASESINS,i+1,
			u.aliases[i]->target,u.aliases[i]->id,u.aliases[i]->id+
			u.aliases[i]->titleoffset))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	if(!(u.symlinks=palloc(&cache.aux,u.total4*sizeof(SYMLINK *))))
		goto out2;
	for(i=0;u.schain;u.schain=u.schain->next,i++)u.symlinks[i]=u.schain;
	qsort(u.symlinks,u.total4,sizeof(SYMLINK *),ssort);
	if(sqlrun(u.db,SYMLINKSFMT))goto out2;
	if(sqllock(u.db,SYMLINKSTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,SYMLINKSPRP "," SYMLINKSROW "," SYMLINKSROW ","
		SYMLINKSROW "," SYMLINKSROW "," SYMLINKSROW "," SYMLINKSROW ","
		SYMLINKSROW,SYMLINKSINS SYMLINKSINS SYMLINKSINS SYMLINKSINS
		SYMLINKSINS SYMLINKSINS SYMLINKSINS SYMLINKSINS,NULL))goto out3;
	for(;i+8<=u.total4;i+=8)if(sqlinsert(u.db,u.sql,SYMLINKSINS SYMLINKSINS
		SYMLINKSINS SYMLINKSINS SYMLINKSINS SYMLINKSINS SYMLINKSINS
		SYMLINKSINS,
		i+1,u.symlinks[i]->src,u.symlinks[i]->level,
		u.symlinks[i]->alias,u.symlinks[i]->dst,
		i+2,u.symlinks[i+1]->src,u.symlinks[i+1]->level,
		u.symlinks[i+1]->alias,u.symlinks[i+1]->dst,
		i+3,u.symlinks[i+2]->src,u.symlinks[i+2]->level,
		u.symlinks[i+2]->alias,u.symlinks[i+2]->dst,
		i+4,u.symlinks[i+3]->src,u.symlinks[i+3]->level,
		u.symlinks[i+3]->alias,u.symlinks[i+3]->dst,
		i+5,u.symlinks[i+4]->src,u.symlinks[i+4]->level,
		u.symlinks[i+4]->alias,u.symlinks[i+4]->dst,
		i+6,u.symlinks[i+5]->src,u.symlinks[i+5]->level,
		u.symlinks[i+5]->alias,u.symlinks[i+5]->dst,
		i+7,u.symlinks[i+6]->src,u.symlinks[i+6]->level,
		u.symlinks[i+6]->alias,u.symlinks[i+6]->dst,
		i+8,u.symlinks[i+7]->src,u.symlinks[i+7]->level,
		u.symlinks[i+7]->alias,u.symlinks[i+7]->dst))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total4)
#endif
	{
		if(sqlprep(u.db,&u.sql,SYMLINKSPRP,SYMLINKSINS,NULL))goto out3;
		for(;i<u.total4;i++)if(sqlinsert(u.db,u.sql,SYMLINKSINS,i+1,
			u.symlinks[i]->src,u.symlinks[i]->level,
			u.symlinks[i]->alias,u.symlinks[i]->dst))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	if(!(u.related=palloc(&cache.aux,u.total5*sizeof(RELATED *))))
		goto out2;
	for(i=0;u.rchain;u.rchain=u.rchain->next,i++)u.related[i]=u.rchain;
	qsort(u.related,u.total5,sizeof(RELATED *),rsort);
	if(sqlrun(u.db,RELATEDFMT))goto out2;
	if(sqllock(u.db,RELATEDTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,RELATEDPRP "," RELATEDROW "," RELATEDROW ","
		RELATEDROW "," RELATEDROW "," RELATEDROW "," RELATEDROW ","
		RELATEDROW,RELATEDINS RELATEDINS RELATEDINS RELATEDINS
		RELATEDINS RELATEDINS RELATEDINS RELATEDINS,NULL))goto out3;
	for(;i+8<=u.total5;i+=8)if(sqlinsert(u.db,u.sql,RELATEDINS RELATEDINS
		RELATEDINS RELATEDINS RELATEDINS RELATEDINS RELATEDINS
		RELATEDINS,
		i+1,u.related[i]->src,u.related[i]->dst,
		i+2,u.related[i+1]->src,u.related[i+1]->dst,
		i+3,u.related[i+2]->src,u.related[i+2]->dst,
		i+4,u.related[i+3]->src,u.related[i+3]->dst,
		i+5,u.related[i+4]->src,u.related[i+4]->dst,
		i+6,u.related[i+5]->src,u.related[i+5]->dst,
		i+7,u.related[i+6]->src,u.related[i+6]->dst,
		i+8,u.related[i+7]->src,u.related[i+7]->dst))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total5)
#endif
	{
		if(sqlprep(u.db,&u.sql,RELATEDPRP,RELATEDINS,NULL))goto out3;
		for(;i<u.total5;i++)if(sqlinsert(u.db,u.sql,RELATEDINS,i+1,
			u.related[i]->src,u.related[i]->dst))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	if(!(u.narrow=palloc(&cache.aux,u.total6*sizeof(NARROW *))))goto out2;
	for(i=0;u.nchain;u.nchain=u.nchain->next,i++)u.narrow[i]=u.nchain;
	qsort(u.narrow,u.total6,sizeof(NARROW *),nsort);
	if(sqlrun(u.db,NARROWFMT))goto out2;
	if(sqllock(u.db,NARROWTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,NARROWPRP "," NARROWROW "," NARROWROW ","
		NARROWROW "," NARROWROW "," NARROWROW "," NARROWROW ","
		NARROWROW,NARROWINS NARROWINS NARROWINS NARROWINS NARROWINS
		NARROWINS NARROWINS NARROWINS,NULL))goto out3;
	for(;i+8<=u.total6;i+=8)if(sqlinsert(u.db,u.sql,NARROWINS NARROWINS
		NARROWINS NARROWINS NARROWINS NARROWINS NARROWINS NARROWINS,
		i+1,u.narrow[i]->src,u.narrow[i]->level,u.narrow[i]->dst,
		i+2,u.narrow[i+1]->src,u.narrow[i+1]->level,u.narrow[i+1]->dst,
		i+3,u.narrow[i+2]->src,u.narrow[i+2]->level,u.narrow[i+2]->dst,
		i+4,u.narrow[i+3]->src,u.narrow[i+3]->level,u.narrow[i+3]->dst,
		i+5,u.narrow[i+4]->src,u.narrow[i+4]->level,u.narrow[i+4]->dst,
		i+6,u.narrow[i+5]->src,u.narrow[i+5]->level,u.narrow[i+5]->dst,
		i+7,u.narrow[i+6]->src,u.narrow[i+6]->level,u.narrow[i+6]->dst,
		i+8,u.narrow[i+7]->src,u.narrow[i+7]->level,u.narrow[i+7]->dst))
		goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total6)
#endif
	{
		if(sqlprep(u.db,&u.sql,NARROWPRP,NARROWINS,NULL))goto out3;
		for(;i<u.total6;i++)if(sqlinsert(u.db,u.sql,NARROWINS,i+1,
			u.narrow[i]->src,u.narrow[i]->level,u.narrow[i]->dst))
			goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	if(!(u.bar=palloc(&cache.aux,u.total7*sizeof(LETTERBAR *))))goto out2;
	for(i=0;u.bchain;u.bchain=u.bchain->next,i++)u.bar[i]=u.bchain;
	qsort(u.bar,u.total7,sizeof(LETTERBAR *),bsort);
	if(sqlrun(u.db,LETTERBARFMT))goto out2;
	if(sqllock(u.db,LETTERBARTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,LETTERBARPRP "," LETTERBARROW "," LETTERBARROW
		"," LETTERBARROW "," LETTERBARROW "," LETTERBARROW ","
		LETTERBARROW "," LETTERBARROW,LETTERBARINS LETTERBARINS
		LETTERBARINS LETTERBARINS LETTERBARINS LETTERBARINS LETTERBARINS
		LETTERBARINS,NULL))goto out3;
	for(;i+8<=u.total7;i+=8)if(sqlinsert(u.db,u.sql,LETTERBARINS
		LETTERBARINS LETTERBARINS LETTERBARINS LETTERBARINS
		LETTERBARINS LETTERBARINS LETTERBARINS,
		i+1,u.bar[i]->src,u.bar[i]->dst,u.bar[i]->title,
		i+2,u.bar[i+1]->src,u.bar[i+1]->dst,u.bar[i+1]->title,
		i+3,u.bar[i+2]->src,u.bar[i+2]->dst,u.bar[i+2]->title,
		i+4,u.bar[i+3]->src,u.bar[i+3]->dst,u.bar[i+3]->title,
		i+5,u.bar[i+4]->src,u.bar[i+4]->dst,u.bar[i+4]->title,
		i+6,u.bar[i+5]->src,u.bar[i+5]->dst,u.bar[i+5]->title,
		i+7,u.bar[i+6]->src,u.bar[i+6]->dst,u.bar[i+6]->title,
		i+8,u.bar[i+7]->src,u.bar[i+7]->dst,u.bar[i+7]->title))
		goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total7)
#endif
	{
		if(sqlprep(u.db,&u.sql,LETTERBARPRP,LETTERBARINS,NULL))
			goto out3;
		for(;i<u.total7;i++)if(sqlinsert(u.db,u.sql,LETTERBARINS,i+1,
			u.bar[i]->src,u.bar[i]->dst,u.bar[i]->title))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	cache.treetotal=tot=u.total6+u.total7;
	if(!(t=palloc(&cache.aux,tot*sizeof(TREE))))goto out2;
	for(i=0;i<u.total6;i++)
	{
		t[i].src=u.narrow[i]->src;
		t[i].dst=u.narrow[i]->dst;
	}
	for(i=0;i<u.total7;i++)
	{
		t[i+u.total6].src=u.bar[i]->src;
		t[i+u.total6].dst=u.bar[i]->dst;
	}
	qsort(t,tot,sizeof(TREE),treesort);
	for(i=1;i<tot;i++)if(t[i-1].src==t[i].src&&t[i-1].dst==t[i].dst)
	{
		t[i-1].src=0;
		cache.treetotal--;
	}
	if(!(cache.tree=palloc(&cache.main,cache.treetotal*sizeof(TREE))))
		goto out2;
	for(cache.treetotal=i=0;i<tot;i++)if(t[i].src)
	{
		cache.tree[cache.treetotal].src=t[i].src;
		cache.tree[cache.treetotal].dst=t[i].dst;
		cache.treetotal++;
	}
	for(cache.treestart=1;cache.treestart>0&&
		cache.treestart<cache.treetotal;cache.treestart<<=1);
	if(cache.treestart<=0)
	{
		printf("overflow error\n");
		goto out2;
	}
	else cache.treestart>>=1;
	err=0;

out4:	if(err)sqlfin(u.db,u.sql);
out3:	if(err)sqlunlock(u.db);
out2:	gzclose(fp);
out1:	if(sqlclose(u.db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out0:	pfree(&cache.aux);
	return err;
}

static int termstart(char *name,char *attr,char *value,void *_u)
{
	TERMDATA *u;

	u=(TERMDATA *)(_u);
	if(!strcasecmp(name,"Topic"))
	{
		u->catid=0;
		u->notfound=0;
		u->id[0]=0;
		u->target[0]=0;
		u->clen=0;
		u->r=NULL;
		if(!strcasecmp(attr,"r:id"))
			if(srccpy(u->id,value,sizeof(u->id)))
		{
			printf("id %s too long\n",value);
			return -1;
		}
	}
	u->clen=0;
	return 0;
}

static int termend(char *name,void *_u)
{
	TERMDATA *u;
	DATALIST *l;
	ITEMLIST *il;

	u=(TERMDATA *)(_u);
	if(!strcasecmp(name,"d:term"))
	{
		u->cdata[u->clen]=0;
		if(srccpy(u->target,u->cdata,sizeof(u->target)))
		{
			printf("target %s too long\n",u->cdata);
			return -1;
		}
		if(!u->id[0]||!u->target[0])
		{
			printf("missing data\n");
			return -1;
		}
		if(!u->catid&&!u->notfound)
		{
			if(!(u->r=findbyid(u->id)))
			{
				if(verbose)
				    printf("Warning: %s not found\n",u->id);
				u->notfound=1;
			}
			else u->catid=u->r->catid;
		}
		if(checkutf8str(u->target,NULL))
		{
			if(verbose)
			    printf("Warning: term %s is not valid UTF-8\n",
				u->target);
		}
		else if(checkutf8unicodestr(u->target))
		{
			if(verbose)
			    printf("Warning: term %s is not in unicode\n",
				u->target);
		}
		else if(u->catid)
		{
			if(!(l=addcacheitem(u->r,u->target)))return -1;
			if(!(il=palloc(&cache.lcl,sizeof(ITEMLIST))))return -1;
			il->next=u->termchain;
			u->termchain=il;
			il->data=l;
			il->catid=u->catid;
			u->total++;
			if(verbose)tagcheck(u->target,"term");
		}
	}
	else if(verbose)
	{
		if(!strcasecmp(name,"Topic"));
		else if(!strcasecmp(name,"d:charset"));
		else if(!strcasecmp(name,"RDF"));
		else printf("Warning: unknown tag %s\n",name);
	}
	u->clen=0;
	return 0;
}

static int termdata(char *s,int len,void *_u)
{
	TERMDATA *u;

	u=(TERMDATA *)(_u);
	if(len>1)if(s[len-2]=='\r')if(s[len-1]=='\n')
	{
		s[len-2]='\n';
		len--;
	}
	if(u->clen+len>=sizeof(u->cdata))
	{
		printf("data oversize: have %d, need %d\n",
			(int)sizeof(u->cdata),u->clen+len);
		return -1;
	}
	memcpy(u->cdata+u->clen,s,len);
	u->clen+=len;
	return 0;
}

static int get_terms(char *termfile,char *database,char *user,char *password)
{
	int i;
	int line;
	int len;
	int err;
	ITEMLIST **il;
	gzFile fp;
	TERMDATA u;
	unsigned char bfr[BUFSIZEH];

	err=-1;
	u.catid=0;
	u.notfound=0;
	u.id[0]=0;
	u.target[0]=0;
	u.clen=0;
	u.r=NULL;
	u.total=0;
	u.termchain=NULL;
	if(!(fp=gzopen(termfile,"r")))goto out0;
	if(sqlopen(database,user,password,&u.db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out1;
	line=1;
	i=0;
	while((len=gzread(fp,bfr+i,sizeof(bfr)-i))>0)
	{
		len+=i;
		if((i=parse(bfr,len,&line,0,0,&u,termdata,termstart,
			termend))==-1)goto out2;
		memmove(bfr,bfr+i,len-i);
		i=len-i;
	}
	if(i)if(parse(bfr,i,&line,1,0,&u,termdata,termstart,termend)==-1)
		goto out2;
	if(all)
	{
		if(!(il=palloc(&cache.lcl,u.total*sizeof(ITEMLIST *))))
			goto out2;
		for(i=0;i<u.total;i++,u.termchain=u.termchain->next)
			il[i]=u.termchain;
		qsort(il,u.total,sizeof(ITEMLIST *),ilsort);
		if(sqlrun(u.db,TERMSFMT))goto out2;
		if(sqllock(u.db,TERMSTBL))goto out2;
		i=0;
#ifdef SQLFLAG_MULTIROW
		if(sqlprep(u.db,&u.sql,TERMSPRP "," TERMSROW "," TERMSROW ","
		TERMSROW "," TERMSROW "," TERMSROW "," TERMSROW "," TERMSROW,
		TERMSINS TERMSINS TERMSINS TERMSINS TERMSINS TERMSINS TERMSINS
		TERMSINS,NULL))goto out3;
		for(;i+8<=u.total;i+=8)if(sqlinsert(u.db,u.sql,TERMSINS
			TERMSINS TERMSINS TERMSINS TERMSINS TERMSINS TERMSINS
			TERMSINS,
			i+1,il[i]->catid,il[i]->data->item,
			i+2,il[i+1]->catid,il[i+1]->data->item,
			i+3,il[i+2]->catid,il[i+2]->data->item,
			i+4,il[i+3]->catid,il[i+3]->data->item,
			i+5,il[i+4]->catid,il[i+4]->data->item,
			i+6,il[i+5]->catid,il[i+5]->data->item,
			i+7,il[i+6]->catid,il[i+6]->data->item,
			i+8,il[i+7]->catid,il[i+7]->data->item))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
		if(i<u.total)
#endif
		{
			if(sqlprep(u.db,&u.sql,TERMSPRP,TERMSINS,NULL))
				goto out3;
			for(;i<u.total;i++)if(sqlinsert(u.db,u.sql,TERMSINS,i+1,
				il[i]->catid,il[i]->data->item))goto out4;
			if(sqlfin(u.db,u.sql))goto out3;
		}
		if(sqlunlock(u.db))goto out2;
	}
	err=0;

out4:	if(err)sqlfin(u.db,u.sql);
out3:	if(err)sqlunlock(u.db);
out2:	if(sqlclose(u.db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out1:	gzclose(fp);
out0:	pfree(&cache.lcl);
	return err;
}

static int build_topicsearch(char *database,char *user,char *password)
{
	int i;
	int j;
	int l;
	int m;
	int total;
	int start;
	int serial;
#ifdef SQLFLAG_MULTIROW
	int catid[8];
	int ages[8];
	int mem[8];
#else
	int catid;
	int ages;
#endif
	int err;
	DB db;
	SQL qry;
	DATALIST *item;
	TOPIC *t;
	WORDLIST *r;
	WORDLIST **rr;
	BLOCK *block[5];
	WORDLIST *list[LISTSIZE];
	WORDLIST *data[DATASIZE];

	err=-1;
	serial=1;
	total=STOPWORDS;
	memset(list,0,sizeof(list));
	memset(block,0,sizeof(block));
	for(start=1;start>0&&start<total;start<<=1);
	if(start<=0)
	{
		printf("overflow error\n");
		goto out0;
	}
	else start>>=1;
	if(sqlopen(database,user,password,&db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out0;

	for(j=0;j<cache.total;j++)
	{
		for(i=0,item=cache.catid[j]->u.list;item;item=item->next)
			if((i=process_string(item->item,i,start,total,0,data,
				list))==-1)goto out1;
		for(m=0;m<i;m++)if(additem(block,data[m],cache.catid[j]->catid))
			goto out1;
	}
	for(total=i=0;i<LISTSIZE;i++)for(r=list[i];r;r=r->next)total++;
	if(!(rr=palloc(&cache.aux,total*sizeof(WORDLIST *))))goto out1;
	for(l=i=0;i<LISTSIZE;i++)for(r=list[i];r;r=r->next)rr[l++]=r;
	qsort(rr,total,sizeof(WORDLIST *),wsort);
	for(i=0;i<total;i++)if(rr[i]->occurrence>1)
		qsort(rr[i]->u.pagelist,rr[i]->occurrence,sizeof(int),isort);
	if(sqlrun(db,TOPICWORDSFMT))goto out1;
	if(sqllock(db,TOPICWORDSTBL))goto out1;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(db,&qry,TOPICWORDSPRP "," TOPICWORDSROW "," TOPICWORDSROW
		"," TOPICWORDSROW "," TOPICWORDSROW "," TOPICWORDSROW ","
		TOPICWORDSROW "," TOPICWORDSROW,TOPICWORDSINS TOPICWORDSINS
		TOPICWORDSINS TOPICWORDSINS TOPICWORDSINS TOPICWORDSINS
		TOPICWORDSINS TOPICWORDSINS,NULL))goto out2;
	for(;i+8<=total;i+=8)if(sqlinsert(db,qry,TOPICWORDSINS TOPICWORDSINS
		TOPICWORDSINS TOPICWORDSINS TOPICWORDSINS TOPICWORDSINS
		TOPICWORDSINS TOPICWORDSINS,
		i+1,rr[i]->word,i+2,rr[i+1]->word,i+3,rr[i+2]->word,
		i+4,rr[i+3]->word,i+5,rr[i+4]->word,i+6,rr[i+5]->word,
		i+7,rr[i+6]->word,i+8,rr[i+7]->word))goto out3;
	if(sqlfin(db,qry))goto out2;
	if(i<total)
#endif
	{
		if(sqlprep(db,&qry,TOPICWORDSPRP,TOPICWORDSINS,NULL))goto out2;
		for(;i<total;i++)if(sqlinsert(db,qry,TOPICWORDSINS,i+1,
			rr[i]->word))goto out3;
		if(sqlfin(db,qry))goto out2;
	}
	if(sqlunlock(db))goto out1;
	if(sqlrun(db,TOPICSEARCHFMT))goto out1;
	if(sqllock(db,TOPICSEARCHTBL))goto out1;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(db,&qry,TOPICSEARCHPRP "," TOPICSEARCHROW ","
		TOPICSEARCHROW "," TOPICSEARCHROW "," TOPICSEARCHROW ","
		TOPICSEARCHROW "," TOPICSEARCHROW "," TOPICSEARCHROW,
		TOPICSEARCHINS TOPICSEARCHINS TOPICSEARCHINS TOPICSEARCHINS
		TOPICSEARCHINS TOPICSEARCHINS TOPICSEARCHINS TOPICSEARCHINS,
		NULL))goto out2;
	for(j=0,i=0;i<total;i++)for(m=0;m<rr[i]->occurrence;m++)
	{
		if(rr[i]->occurrence==1)catid[j]=rr[i]->u.page;
		else catid[j]=rr[i]->u.pagelist[m];
		if(!(t=findbycatid(catid[j],NULL)))ages[j]=0;
		else if(strncmp(t->id,ktbase.str,ktbase.len))ages[j]=0;
		else if(t->id[ktbase.len]&&t->id[ktbase.len]!='/')ages[j]=0;
		else ages[j]=1;
		mem[j]=i+1;
		if(++j<8)continue;
		if(sqlinsert(db,qry,TOPICSEARCHINS TOPICSEARCHINS
			TOPICSEARCHINS TOPICSEARCHINS TOPICSEARCHINS
			TOPICSEARCHINS TOPICSEARCHINS TOPICSEARCHINS,
			serial,mem[0],catid[0],ages[0],
			serial+1,mem[1],catid[1],ages[1],
			serial+2,mem[2],catid[2],ages[2],
			serial+3,mem[3],catid[3],ages[3],
			serial+4,mem[4],catid[4],ages[4],
			serial+5,mem[5],catid[5],ages[5],
			serial+6,mem[6],catid[6],ages[6],
			serial+7,mem[7],catid[7],ages[7]))goto out3;
		serial+=8;
		j=0;
	}
	if(sqlfin(db,qry))goto out2;
	if(j)
	{
		if(sqlprep(db,&qry,TOPICSEARCHPRP,TOPICSEARCHINS,NULL))
			goto out2;
		for(i=0;i<j;i++)if(sqlinsert(db,qry,TOPICSEARCHINS,serial++,
			mem[i],catid[i],ages[i]))goto out3;
		if(sqlfin(db,qry))goto out2;
	}
#else
	if(sqlprep(db,&qry,TOPICSEARCHPRP,TOPICSEARCHINS,NULL))goto out2;
	for(i=0;i<total;i++)for(m=0;m<rr[i]->occurrence;m++)
	{
		if(rr[i]->occurrence==1)catid=rr[i]->u.page;
		else catid=rr[i]->u.pagelist[m];
		if(!(t=findbycatid(catid,NULL)))ages=0;
		else if(strncmp(t->id,ktbase.str,ktbase.len))ages=0;
		else if(t->id[ktbase.len]&&t->id[ktbase.len]!='/')ages=0;
		else ages=1;
		if(sqlinsert(db,qry,TOPICSEARCHINS,serial++,i+1,catid,ages))
			goto out3;
	}
	if(sqlfin(db,qry))goto out2;
#endif
	if(sqlunlock(db))goto out1;
	err=0;
out3:	if(err)sqlfin(db,qry);
out2:	if(err)sqlunlock(db);
out1:	if(sqlclose(db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out0:	pfree(&cache.aux);
	return err;
}

static int build_topicref(char *database,char *user,char *password)
{
	int i;
	int total;
	int start;
	int current;
	int delta;
	int target;
	int total2;
	int err;
	DB db;
	SQL sql;
	unsigned char *p;
	int *data;
	TOPIC *r;
	TOPICTRACE *t;
	TOPICTRACE *trace;
	TOPICTRACE **array;
	unsigned char bfr[BUFSIZEM];

	err=-1;
	data=NULL;
	if(sqlopen(database,user,password,&db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out0;
	if(!(data=palloc(&cache.lcl,2*cache.total*sizeof(int))))goto out1;
	current=0;
	for(i=0;i<cache.total;i++)
	{
		data[current++]=cache.catid[i]->catid;
		if(strlen(cache.catid[i]->id)>=sizeof(bfr))goto out1;
		strcpy(bfr,cache.catid[i]->id);
		if(!(p=strrchr(bfr,'/')))
		{
			data[current++]=0;
			continue;
		}
		*p=0;
		if(!(r=findbyid(bfr)))
		{
			if(verbose)printf("Warning: no parent for %s\n",
				cache.catid[i]->id);
			data[current++]=0;
		}
		else data[current++]=r->catid;
	}
	total=current;
	if(sqlrun(db,TOPICREFFMT))goto out1;
	if(sqllock(db,TOPICREFTBL))goto out1;
	current=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(db,&sql,TOPICREFPRP "," TOPICREFROW "," TOPICREFROW ","
		TOPICREFROW "," TOPICREFROW "," TOPICREFROW "," TOPICREFROW
		"," TOPICREFROW,TOPICREFINS TOPICREFINS TOPICREFINS TOPICREFINS
		TOPICREFINS TOPICREFINS TOPICREFINS TOPICREFINS,NULL))goto out2;
	for(;current+16<=total;current+=16)if(sqlinsert(db,sql,TOPICREFINS
		TOPICREFINS TOPICREFINS TOPICREFINS TOPICREFINS TOPICREFINS
		TOPICREFINS TOPICREFINS,
		data[current],data[current+1],
		data[current+2],data[current+3],
		data[current+4],data[current+5],
		data[current+6],data[current+7],
		data[current+8],data[current+9],
		data[current+10],data[current+11],
		data[current+12],data[current+13],
		data[current+14],data[current+15]))goto out3;
	if(sqlfin(db,sql))goto out2;
	if(current<total)
#endif
	{
		if(sqlprep(db,&sql,TOPICREFPRP,TOPICREFINS,NULL))goto out2;
		for(;current<total;current+=2)if(sqlinsert(db,sql,TOPICREFINS,
			data[current],data[current+1]))goto out3;
		if(sqlfin(db,sql))goto out2;
	}
	if(sqlunlock(db))goto out1;

	total>>=1;
	qsort(data,total,2*sizeof(int),isort);

	for(start=1;start>0&&start<total;start<<=1);
	if(start<=0)
	{
		printf("overflow error\n");
		goto out1;
	}
	else start>>=1;

	for(trace=NULL,total2=total,i=0;i<total;i++)
	{
		if(!(t=palloc(&cache.lcl,sizeof(TOPICTRACE))))goto out1;
		t->next=trace;
		trace=t;
		t->catid=data[2*i];
		t->refid=data[2*i];
		target=data[2*i+1];
		if(!target)continue;
		while(1)
		{
			if(!(t=palloc(&cache.lcl,sizeof(TOPICTRACE))))goto out1;
			t->next=trace;
			trace=t;
			t->catid=data[2*i];
			t->refid=target;
			total2++;
			current=start;
			delta=start;
			while(1)
			{
				if(!delta)
				{
					if(current<total)
					  if(data[2*current]==target)
					    break;
					current=-1;
					break;
				}
				else if(current>=total)
				{
					current-=delta;
					delta>>=1;
				}
				else if(data[2*current]==target)break;
				else if(data[2*current]>target)
				{
					if(!current)
					{
						current=-1;
						break;
					}
					current-=delta;
					delta>>=1;
				}
				else
				{
					current+=delta;
					delta>>=1;
				}
			}
			if(current==-1)break;
			target=data[2*current+1];
			if(!target)break;
		}
	}
	if(!(array=palloc(&cache.lcl,total2*sizeof(TOPICTRACE *))))goto out1;
	for(i=0,t=trace;t;t=t->next,i++)array[i]=t;
	qsort(array,total2,sizeof(TOPICTRACE *),trsort);
	if(sqlrun(db,TOPICTRACEFMT))goto out1;
	if(sqllock(db,TOPICTRACETBL))goto out1;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(db,&sql,TOPICTRACEPRP "," TOPICTRACEROW "," TOPICTRACEROW
		"," TOPICTRACEROW "," TOPICTRACEROW "," TOPICTRACEROW ","
		TOPICTRACEROW "," TOPICTRACEROW,TOPICTRACEINS TOPICTRACEINS
		TOPICTRACEINS TOPICTRACEINS TOPICTRACEINS TOPICTRACEINS
		TOPICTRACEINS TOPICTRACEINS,NULL))goto out2;
	for(;i+8<=total2;i+=8)if(sqlinsert(db,sql,TOPICTRACEINS TOPICTRACEINS
		TOPICTRACEINS TOPICTRACEINS TOPICTRACEINS TOPICTRACEINS
		TOPICTRACEINS TOPICTRACEINS,
		i+1,array[i]->refid,array[i]->catid,
		i+2,array[i+1]->refid,array[i+1]->catid,
		i+3,array[i+2]->refid,array[i+2]->catid,
		i+4,array[i+3]->refid,array[i+3]->catid,
		i+5,array[i+4]->refid,array[i+4]->catid,
		i+6,array[i+5]->refid,array[i+5]->catid,
		i+7,array[i+6]->refid,array[i+6]->catid,
		i+8,array[i+7]->refid,array[i+7]->catid))goto out3;
	if(sqlfin(db,sql))goto out2;
	if(i<total2)
#endif
	{
		if(sqlprep(db,&sql,TOPICTRACEPRP,TOPICTRACEINS,NULL))goto out2;
		for(;i<total2;i++)if(sqlinsert(db,sql,TOPICTRACEINS,i+1,
			array[i]->refid,array[i]->catid))goto out3;
		if(sqlfin(db,sql))goto out2;
	}
	if(sqlunlock(db))goto out1;
	err=0;

out3:	if(err)sqlfin(db,sql);
out2:	if(err)sqlunlock(db);
out1:	if(sqlclose(db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out0:	pfree(&cache.lcl);
	return err;
}

static int verify_langlist(void)
{
	int i;
	int j;
	int k;
	int l;
	int err;
	int catid;
	int total;
	int start;
	int current;
	int delta;
	TOPIC *r;
	char *p;

	err=-1;
	for(total=0;languages[total].tree;total++);
	for(start=1;start>0&&start<total;start<<=1);
	if(start<=0)
	{
		printf("overflow error\n");
		goto out0;
	}
	else start>>=1;
	for(i=0;langbase[i];i++)
	{
		l=strlen(langbase[i]);
		if(!(r=findbyid((char *)langbase[i])))
		{
			printf("%s not found\n",langbase[i]);
			goto out0;
		}
		else catid=r->catid;
		if((k=findintree(catid))==-1)
		{
			printf("%s not found in tree\n",langbase[i]);
			goto out0;
		}
		for(;cache.tree[k].src==catid;k++)
		{
			if(!(r=findbycatid(cache.tree[k].dst,NULL)))continue;
			if(strncmp(r->id,langbase[i],l))continue;
			p=r->id+l;
			if(*p++!='/')continue;
			current=start;
			delta=start;
			while(1)
			{
				if(!delta)
				{
					if(current<total)
					  if(!strcmp(languages[current].tree,p))
					    break;
					printf("*** language '%s' not found\n",
						p);
					break;
				}
				else if(current>=total)
				{
					current-=delta;
					delta>>=1;
				}
				else if(!(j=strcmp(languages[current].tree,p)))
					break;
				else if(j>0)
				{
					if(!current)
					{
						printf("*** language '%s' not "
							"found\n",p);
						break;
					}
					current-=delta;
					delta>>=1;
				}
				else
				{
					current+=delta;
					delta>>=1;
				}
			}
		}
	}
	err=0;

out0:	return err;
}

static int build_nlssupport(char *database,char *user,char *password)
{
	int i;
	int err;
	DB db;
	SQL sql;
	struct
	{
		char name[BUFSIZET];
		char id[BUFSIZET];
		char seealso[BUFSIZET];
		char otherlang[BUFSIZET];
		char description[BUFSIZET];
		char editor[BUFSIZET];
		char pre[BUFSIZET];
		char post[BUFSIZET];
		char search[BUFSIZET];
		char alldir[BUFSIZET];
		char onlyin[BUFSIZET];
		char lastupdate[BUFSIZET];
		char termsofuse[BUFSIZET];
	} u;

	err=-1;
	if(sqlopen(database,user,password,&db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out0;
	if(sqlrun(db,NLSSUPPORTFMT))goto out1;
	if(sqllock(db,NLSSUPPORTTBL))goto out1;
	if(sqlprep(db,&sql,NLSSUPPORTPRP,NLSSUPPORTINS,NULL))goto out2;
	for(i=0;languages[i].name;i++)
	{
		if(srccpy(u.name,(char *)languages[i].name,BUFSIZET))goto out3;
		if(checkutf8str(u.name,NULL))goto out3;
		if(checkutf8unicodestr(u.name))goto out3;
		if(srccpy(u.id,(char *)languages[i].id,BUFSIZET))goto out3;
		if(checkutf8str(u.id,NULL))goto out3;
		if(checkutf8unicodestr(u.id))goto out3;
		if(srccpy(u.seealso,(char *)seealso[i],BUFSIZET))goto out3;
		if(checkutf8str(u.seealso,NULL))goto out3;
		if(checkutf8unicodestr(u.seealso))goto out3;
		if(srccpy(u.otherlang,(char *)otherlang[i],BUFSIZET))goto out3;
		if(checkutf8str(u.otherlang,NULL))goto out3;
		if(checkutf8unicodestr(u.otherlang))goto out3;
		if(srccpy(u.description,(char *)description[i],BUFSIZET))
			goto out3;
		if(checkutf8str(u.description,NULL))goto out3;
		if(checkutf8unicodestr(u.description))goto out3;
		if(srccpy(u.editor,(char *)editor[i],BUFSIZET))goto out3;
		if(checkutf8str(u.editor,NULL))goto out3;
		if(checkutf8unicodestr(u.editor))goto out3;
		if(srccpy(u.pre,(char *)searchon[i].pre,BUFSIZET))goto out3;
		if(checkutf8str(u.pre,NULL))goto out3;
		if(checkutf8unicodestr(u.pre))goto out3;
		if(srccpy(u.post,(char *)searchon[i].post,BUFSIZET))goto out3;
		if(checkutf8str(u.post,NULL))goto out3;
		if(checkutf8unicodestr(u.post))goto out3;
		if(srccpy(u.search,(char *)search[i].search,BUFSIZET))goto out3;
		if(checkutf8str(u.search,NULL))goto out3;
		if(checkutf8unicodestr(u.search))goto out3;
		if(srccpy(u.alldir,(char *)search[i].alldir,BUFSIZET))goto out3;
		if(checkutf8str(u.alldir,NULL))goto out3;
		if(checkutf8unicodestr(u.alldir))goto out3;
		if(srccpy(u.onlyin,(char *)search[i].onlyin,BUFSIZET))goto out3;
		if(checkutf8str(u.onlyin,NULL))goto out3;
		if(checkutf8unicodestr(u.onlyin))goto out3;
		if(srccpy(u.lastupdate,(char *)lastupdate[i],BUFSIZET))
			goto out3;
		if(checkutf8str(u.lastupdate,NULL))goto out3;
		if(checkutf8unicodestr(u.lastupdate))goto out3;
		if(srccpy(u.termsofuse,(char *)termsofuse[i],BUFSIZET))
			goto out3;
		if(checkutf8str(u.termsofuse,NULL))goto out3;
		if(checkutf8unicodestr(u.termsofuse))goto out3;
		if(sqlinsert(db,sql,NLSSUPPORTINS,i+1,u.name,u.id,u.seealso,
			u.otherlang,u.description,u.editor,u.pre,u.post,
			u.search,u.alldir,u.onlyin,u.lastupdate,u.termsofuse))
			goto out3;
	}
	err=0;
out3:	if(sqlfin(db,sql))err=-1;
out2:	if(sqlunlock(db))err=-1;
out1:	if(sqlclose(db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out0:	return err;
}

static int build_nlsinfo(char *database,char *user,char *password)
{
	int i;
	int j;
	int k;
	int l;
	int n;
	int start;
	int current;
	int delta;
	int err;
	DB db;
	SQL qry;
	int *nlsinfo;
	const char *p;
	char *q;
	char m;

	err=-1;
	if(sqlopen(database,user,password,&db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out0;
	if(!(nlsinfo=palloc(&cache.aux,cache.total*sizeof(int))))goto out1;
	memset(nlsinfo,0,cache.total*sizeof(int));
	for(k=0;languages[k].name;k++);
	for(start=1;start>0&&start<k;start<<=1);
	if(start<=0)
	{
		printf("overflow error\n");
		goto out1;
	}
	else start>>=1;
	for(i=0;i<cache.total;i++)
	{
		for(j=0;langbase[j];j++)
		{
			p=cache.catid[i]->id;
			l=strlen(langbase[j]);
			if(strncmp(p,langbase[j],l))continue;
			if(p[l]!='/')continue;
			p+=l+1;
			current=start;
			delta=start;
			for(q=(char *)(p);*q&&*q!='/';*q++);
			m=*q;
			*q=0;
			while(1)
			{
				if(!delta)
				{
					if(current<k)
					  if(!strcmp(languages[current].tree,p))
					    break;
					p=NULL;
					break;
				}
				else if(current>=k)
				{
					current-=delta;
					delta>>=1;
				}
				else if(!(n=strcmp(languages[current].tree,p)))
					break;
				else if(n>0)
				{
					if(!current)
					{
						p=NULL;
						break;
					}
					current-=delta;
					delta>>=1;
				}
				else
				{
					current+=delta;
					delta>>=1;
				}
			}
			*q=m;
			if(p)
			{
				nlsinfo[i]=current;
				break;
			}
		}
	}
	if(sqlrun(db,NLSINFOFMT))goto out1;
	if(sqllock(db,NLSINFOTBL))goto out1;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(db,&qry,NLSINFOPRP "," NLSINFOROW "," NLSINFOROW ","
		NLSINFOROW "," NLSINFOROW "," NLSINFOROW "," NLSINFOROW ","
		NLSINFOROW,NLSINFOINS NLSINFOINS NLSINFOINS NLSINFOINS
		NLSINFOINS NLSINFOINS NLSINFOINS NLSINFOINS,NULL))goto out2;
	for(;i+8<=cache.total;i+=8)if(sqlinsert(db,qry,NLSINFOINS NLSINFOINS
		NLSINFOINS NLSINFOINS NLSINFOINS NLSINFOINS NLSINFOINS
		NLSINFOINS,
		cache.catid[i]->catid,nlsinfo[i]+1,
		cache.catid[i+1]->catid,nlsinfo[i+1]+1,
		cache.catid[i+2]->catid,nlsinfo[i+2]+1,
		cache.catid[i+3]->catid,nlsinfo[i+3]+1,
		cache.catid[i+4]->catid,nlsinfo[i+4]+1,
		cache.catid[i+5]->catid,nlsinfo[i+5]+1,
		cache.catid[i+6]->catid,nlsinfo[i+6]+1,
		cache.catid[i+7]->catid,nlsinfo[i+7]+1))goto out3;
	if(sqlfin(db,qry))goto out2;
	if(i<cache.total)
#endif
	{
		if(sqlprep(db,&qry,NLSINFOPRP,NLSINFOINS,NULL))goto out2;
		for(;i<cache.total;i++)if(sqlinsert(db,qry,NLSINFOINS,
			cache.catid[i]->catid,nlsinfo[i]+1))goto out3;
		if(sqlfin(db,qry))goto out2;
	}
	if(sqlunlock(db))goto out1;
	err=0;
out3:	if(err)sqlfin(db,qry);
out2:	if(err)sqlunlock(db);
out1:	if(sqlclose(db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out0:	pfree(&cache.aux);
	return err;
}

static int contentstart(char *name,char *attr,char *value,void *_u)
{
	CONTENTDATA *u;

	u=(CONTENTDATA *)(_u);
	if(!strcasecmp(name,"Topic"))
	{
		if(!u->in_extpage)
		{
			u->clist=NULL;
			u->id[0]=0;
			u->ages=0;
			u->in_topic=1;
#ifdef SQLFLAG_MULTIROW
			u->catid[u->idx]=0;
			u->priority[u->idx]=0;
			u->type[u->idx]=0;
			u->link[u->idx][0]=0;
			u->title[u->idx][0]=0;
			u->desc[u->idx][0]=0;
			u->date[u->idx][0]=0;
#else
			u->catid=0;
			u->priority=0;
			u->type=0;
			u->link[0]=0;
			u->title[0]=0;
			u->desc[0]=0;
			u->date[0]=0;
#endif
			pclear(&cache.lcl);
			if(!strcasecmp(attr,"r:id"))
				if(srccpy(u->id,value,sizeof(u->id)))
			{
				printf("topic id %s too long\n",value);
				return -1;
			}
		}
	}
	else if(!strcasecmp(name,"link"))
	{
		if(!u->in_extpage)if(u->in_topic)
			if(!strcasecmp(attr,"r:resource"))
				if(addlink(u,value,LINK,0))
		{
			printf("adding %s=%s failed\n",name,value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"link1"))
	{
		if(!u->in_extpage)if(u->in_topic)
			if(!strcasecmp(attr,"r:resource"))
				if(addlink(u,value,LINK,1))
		{
			printf("adding %s=%s failed\n",name,value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"pdf"))
	{
		if(!u->in_extpage)if(u->in_topic)
			if(!strcasecmp(attr,"r:resource"))
				if(addlink(u,value,PDF,0))
		{
			printf("adding %s=%s failed\n",name,value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"pdf1"))
	{
		if(!u->in_extpage)if(u->in_topic)
			if(!strcasecmp(attr,"r:resource"))
				if(addlink(u,value,PDF,1))
		{
			printf("adding %s=%s failed\n",name,value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"rss"))
	{
		if(!u->in_extpage)if(u->in_topic)
			if(!strcasecmp(attr,"r:resource"))
				if(addlink(u,value,RSS,0))
		{
			printf("adding %s=%s failed\n",name,value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"rss1"))
	{
		if(!u->in_extpage)if(u->in_topic)
			if(!strcasecmp(attr,"r:resource"))
				if(addlink(u,value,RSS,1))
		{
			printf("adding %s=%s failed\n",name,value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"atom"))
	{
		if(!u->in_extpage)if(u->in_topic)
			if(!strcasecmp(attr,"r:resource"))
				if(addlink(u,value,ATOM,0))
		{
			printf("adding %s=%s failed\n",name,value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"ExternalPage"))
	{
#ifdef SQLFLAG_MULTIROW
		u->link[u->idx][0]=0;
		u->title[u->idx][0]=0;
		u->desc[u->idx][0]=0;
		u->date[u->idx][0]=0;
		u->priority[u->idx]=0;
		u->type[u->idx]=0;
		u->ages=0;
		u->in_extpage=1;
		if(!strcasecmp(attr,"about"))
			if(srccpy(u->link[u->idx],value,
				sizeof(u->link[u->idx])))
		{
			printf("ExternalPage %s too long\n",value);
			return -1;
		}
#else
		u->link[0]=0;
		u->title[0]=0;
		u->desc[0]=0;
		u->date[0]=0;
		u->priority=0;
		u->type=0;
		u->ages=0;
		u->in_extpage=1;
		if(!strcasecmp(attr,"about"))
			if(srccpy(u->link,value,sizeof(u->link)))
		{
			printf("ExternalPage %s too long\n",value);
			return -1;
		}
#endif
	}
	u->clen=0;
	return 0;
}

static int contentend(char *name,void *_u)
{
	CONTENTDATA *u;
	CONTENTLIST *l;
	TOPIC *r;
	char *p;

	u=(CONTENTDATA *)(_u);
	if(!strcasecmp(name,"Topic"))
	{
		if(!u->in_extpage)u->in_topic=0;
	}
#ifdef SQLFLAG_MULTIROW
	else if(!strcasecmp(name,"ExternalPage"))
	{
		l=NULL;
		r=NULL;
		u->in_extpage=0;
		if(u->link[u->idx][0])
		{
			if(!u->catid[u->idx]&&verbose)
				printf("Warning: no catid for %s\n",
					u->link[u->idx]);
			else
			{
				if(!(r=findbycatid(u->catid[u->idx],NULL)))
				{
					if(verbose)printf("Warning: catid %d "
						"not found\n",u->catid[u->idx]);
					u->catid[u->idx]=0;
				}
				else if(strcmp(r->id,u->id))
				{
					if(verbose)printf("Warning: id "
						"mismatch for catid %d\n",
						u->catid[u->idx]);
					u->catid[u->idx]=0;
				}
			}
			if(u->catid[u->idx])
			{
				for(l=u->clist;l;l=l->next)
					if(!strcmp(l->link,u->link[u->idx]))
						break;
				if(!l)
				{
					if(verbose)
						printf("Warning: link data not "
						"found for %s\n",
						u->link[u->idx]);
					u->catid[u->idx]=0;
				}
			}
		}
		if(u->link[u->idx][0]&&u->catid[u->idx])
		{
		    if(checkutf8unicodestr(u->link[u->idx]))
		    {
			if(verbose)printf("Warning: link %s is not valid "
				"unicode\n",u->link[u->idx]);
		    }
		    else if(checkutf8unicodestr(u->title[u->idx]))
		    {
			if(verbose)printf("Warning: title for link %s is not "
				"valid unicode\n",u->link[u->idx]);
		    }
		    else if(checkutf8unicodestr(u->desc[u->idx]))
		    {
			if(verbose)printf("Warning: description for link %s is "
				"not valid unicode\n",u->link[u->idx]);
		    }
		    else if(checkutf8unicodestr(u->date[u->idx]))
		    {
			if(verbose)printf("Warning: date for link %s is not "
				"valid unicode\n",u->link[u->idx]);
		    }
		    else
		    {
			if(verbose)
			{
				tagcheck(u->title[u->idx],"content title");
				tagcheck(u->desc[u->idx],"content description");
			}
			fixcontent(u);
			r->u.counter++;
			if(update_pagesearch(u))return -1;
			if(urlify(u->link[u->idx],sizeof(u->link[u->idx])))
				return -1;
			u->mem[u->idx][0]=l->linktype;
			u->mem[u->idx][1]=l->linklevel;
			if(++(u->idx)==8)
			{
				if(all)
				{
				    if(sqlinsert(u->db,u->sql,
					ALLEXTERNALPAGESINS ALLEXTERNALPAGESINS
					ALLEXTERNALPAGESINS ALLEXTERNALPAGESINS
					ALLEXTERNALPAGESINS ALLEXTERNALPAGESINS
					ALLEXTERNALPAGESINS ALLEXTERNALPAGESINS,
					u->count,u->catid[0],u->mem[0][0],
					u->mem[0][1],u->priority[0],u->type[0],
					u->link[0],u->title[0],u->desc[0],
					u->date[0],
					u->count+1,u->catid[1],u->mem[1][0],
					u->mem[1][1],u->priority[1],u->type[1],
					u->link[1],u->title[1],u->desc[1],
					u->date[1],
					u->count+2,u->catid[2],u->mem[2][0],
					u->mem[2][1],u->priority[2],u->type[2],
					u->link[2],u->title[2],u->desc[2],
					u->date[2],
					u->count+3,u->catid[3],u->mem[3][0],
					u->mem[3][1],u->priority[3],u->type[3],
					u->link[3],u->title[3],u->desc[3],
					u->date[3],
					u->count+4,u->catid[4],u->mem[4][0],
					u->mem[4][1],u->priority[4],u->type[4],
					u->link[4],u->title[4],u->desc[4],
					u->date[4],
					u->count+5,u->catid[5],u->mem[5][0],
					u->mem[5][1],u->priority[5],u->type[5],
					u->link[5],u->title[5],u->desc[5],
					u->date[5],
					u->count+6,u->catid[6],u->mem[6][0],
					u->mem[6][1],u->priority[6],u->type[6],
					u->link[6],u->title[6],u->desc[6],
					u->date[6],
					u->count+7,u->catid[7],u->mem[7][0],
					u->mem[7][1],u->priority[7],u->type[7],
					u->link[7],u->title[7],u->desc[7],
					u->date[7]))return -1;
				}
				else if(sqlinsert(u->db,u->sql,
					STDEXTERNALPAGESINS STDEXTERNALPAGESINS
					STDEXTERNALPAGESINS STDEXTERNALPAGESINS
					STDEXTERNALPAGESINS STDEXTERNALPAGESINS
					STDEXTERNALPAGESINS STDEXTERNALPAGESINS,
					u->count,u->catid[0],u->mem[0][0],
					u->priority[0],u->link[0],u->title[0],
					u->desc[0],u->date[0],
					u->count+1,u->catid[1],u->mem[1][0],
					u->priority[1],u->link[1],u->title[1],
					u->desc[1],u->date[1],
					u->count+2,u->catid[2],u->mem[2][0],
					u->priority[2],u->link[2],u->title[2],
					u->desc[2],u->date[2],
					u->count+3,u->catid[3],u->mem[3][0],
					u->priority[3],u->link[3],u->title[3],
					u->desc[3],u->date[3],
					u->count+4,u->catid[4],u->mem[4][0],
					u->priority[4],u->link[4],u->title[4],
					u->desc[4],u->date[4],
					u->count+5,u->catid[5],u->mem[5][0],
					u->priority[5],u->link[5],u->title[5],
					u->desc[5],u->date[5],
					u->count+6,u->catid[6],u->mem[6][0],
					u->priority[6],u->link[6],u->title[6],
					u->desc[6],u->date[6],
					u->count+7,u->catid[7],u->mem[7][0],
					u->priority[7],u->link[7],u->title[7],
					u->desc[7],u->date[7]))return -1;
				u->count+=8;
				u->idx=0;
				u->catid[0]=u->catid[7];
			}
			else u->catid[u->idx]=u->catid[u->idx-1];
		    }
		}
	}
#else
	else if(!strcasecmp(name,"ExternalPage"))
	{
		l=NULL;
		r=NULL;
		u->in_extpage=0;
		if(u->link[0])
		{
			if(!u->catid&&verbose)
				printf("Warning: no catid for %s\n",u->link);
			else
			{
				if(!(r=findbycatid(u->catid,NULL)))
				{
					if(verbose)printf("Warning: catid %d "
						"not found\n",u->catid);
					u->catid=0;
				}
				else if(strcmp(r->id,u->id))
				{
					if(verbose)printf("Warning: id "
						"mismatch for catid %d\n",
						u->catid);
					u->catid=0;
				}
			}
			if(u->catid)
			{
				for(l=u->clist;l;l=l->next)
					if(!strcmp(l->link,u->link))break;
				if(!l)
				{
					if(verbose)
						printf("Warning: link data not "
						"found for %s\n",u->link);
					u->catid=0;
				}
			}
		}
		if(u->link[0]&&u->catid)
		{
		    if(checkutf8unicodestr(u->link))
		    {
			if(verbose)printf("Warning: link %s is not valid "
				"unicode\n",u->link);
		    }
		    else if(checkutf8unicodestr(u->title))
		    {
			if(verbose)printf("Warning: title for link %s is not "
				"valid unicode\n",u->link);
		    }
		    else if(checkutf8unicodestr(u->desc))
		    {
			if(verbose)printf("Warning: description for link %s is "
				"not valid unicode\n",u->link);
		    }
		    else if(checkutf8unicodestr(u->date))
		    {
			if(verbose)printf("Warning: date for link %s is not "
				"valid unicode\n",u->link);
		    }
		    else
		    {
			if(verbose)
			{
				tagcheck(u->title,"content title");
				tagcheck(u->desc,"content description");
			}
			fixcontent(u);
			r->u.counter++;
			if(update_pagesearch(u))return -1;
			if(urlify(u->link,sizeof(u->link)))return -1;
			if(all)
			{
			    if(sqlinsert(u->db,u->sql,ALLEXTERNALPAGESINS,
				u->count++,u->catid,l->linktype,l->linklevel,
				u->priority,u->type,u->link,u->title,u->desc,
				u->date))return -1;
			}
			else if(sqlinsert(u->db,u->sql,STDEXTERNALPAGESINS,
				u->count++,u->catid,l->linktype,u->priority,
				u->link,u->title,u->desc,u->date))return -1;
		    }
		}
	}
#endif
	else if(!strcasecmp(name,"d:Description"))
	{
		if(u->in_extpage)
		{
			u->cdata[u->clen]=0;
#ifdef SQLFLAG_MULTIROW
			if(srccpy(u->desc[u->idx],u->cdata,
				sizeof(u->desc[u->idx])))
#else
			if(srccpy(u->desc,u->cdata,sizeof(u->desc)))
#endif
			{
				printf("Description %s too long\n",u->cdata);
				return -1;
			}
		}
	}
	else if(!strcasecmp(name,"mediadate"))
	{
		if(u->in_extpage)
		{
			u->cdata[u->clen]=0;
			if(getmediadate(u))
			{
				printf("mediadate %s too long\n",u->cdata);
				return -1;
			}
		}
	}
	else if(!strcasecmp(name,"d:Title"))
	{
		if(u->in_extpage)
		{
			u->cdata[u->clen]=0;
#ifdef SQLFLAG_MULTIROW
			if(srccpy(u->title[u->idx],u->cdata,
				sizeof(u->title[u->idx])))
#else
			if(srccpy(u->title,u->cdata,sizeof(u->title)))
#endif
			{
				printf("Title %s too long\n",u->cdata);
				return -1;
			}
		}
	}
	else if(!strcasecmp(name,"priority"))
	{
		if(u->in_extpage)
		{
			u->cdata[u->clen]=0;
#ifdef SQLFLAG_MULTIROW
			u->priority[u->idx]=atoi(u->cdata);
#else
			u->priority=atoi(u->cdata);
#endif
		}
	}
	else if(!strcasecmp(name,"ages"))
	{
		if(u->in_extpage)
		{
			u->cdata[u->clen]=0;
			for(p=u->cdata;(p=strtok(p,"/"));p=NULL)
			{
				if(!strcmp(p,"kids"))u->ages|=KIDS;
				else if(!strcmp(p,"teen"))u->ages|=TEEN;
				else if(!strcmp(p,"mteen"))u->ages|=MTEEN;
				else if(verbose)printf("Warning: unknown age "
					"specification %s\n",p);
			}
		}
	}
	else if(!strcasecmp(name,"type"))
	{
		if(u->in_extpage)
		{
			u->cdata[u->clen]=0;
#ifdef SQLFLAG_MULTIROW
			if(!(u->type[u->idx]=buildlist(&u->list1,u->cdata,1,1)))
#else
			if(!(u->type=buildlist(&u->list1,u->cdata,1,1)))
#endif
			{
				printf("type %s handling failed\n",u->cdata);
				return -1;
			}
		}
	}
	else if(!strcasecmp(name,"catid"))
	{
		if(u->in_topic)
		{
			u->cdata[u->clen]=0;
#ifdef SQLFLAG_MULTIROW
			u->catid[u->idx]=atoi(u->cdata);
#else
			u->catid=atoi(u->cdata);
#endif
		}
	}
	else if(verbose)
	{
		if(!strcasecmp(name,"link"));
		else if(!strcasecmp(name,"link1"));
		else if(!strcasecmp(name,"pdf"));
		else if(!strcasecmp(name,"pdf1"));
		else if(!strcasecmp(name,"rss"));
		else if(!strcasecmp(name,"rss1"));
		else if(!strcasecmp(name,"atom"));
		else if(!strcasecmp(name,"RDF"));
		else printf("Warning: unknown %s\n",name);
	}
	u->clen=0;
	return 0;
}

static int contentdata(char *s,int len,void *_u)
{
	CONTENTDATA *u;

	u=(CONTENTDATA *)(_u);
	if(len>1)if(s[len-2]=='\r')if(s[len-1]=='\n')
	{
		s[len-2]='\n';
		len--;
	}
	if(u->clen+len>=sizeof(u->cdata))
	{
		printf("data oversize: have %d, need %d\n",
			(int)sizeof(u->cdata),u->clen+len);
		return -1;
	}
	memcpy(u->cdata+u->clen,s,len);
	u->clen+=len;
	return 0;
}

static int get_content(char *contentfile,char *database,char *user,
	char *password)
{
	int len;
	int err;
	gzFile fp;
	DATALIST *r;
	CONTENTDATA u;
	int i;
	int l;
	int line;
	int total;
	int flags;
	int max;
#ifdef SQLFLAG_MULTIROW
	int j;
	int k;
	int mem[8][5];
	char *rl[8];
#endif
	WORDLIST *w;
	WORDLIST **rr;
	double dval;
	PAGEDATA *pd;
	unsigned char bfr[BUFSIZEH];

	err=-1;
	for(i=0;i<cache.total;i++)cache.catid[i]->u.counter=0;
	u.id[0]=0;
	u.ages=0;
#ifdef SQLFLAG_MULTIROW
	u.idx=0;
	u.catid[0]=0;
	u.priority[0]=0;
	u.type[0]=0;
	u.link[0][0]=0;
	u.title[0][0]=0;
	u.desc[0][0]=0;
	u.date[0][0]=0;
#else
	u.catid=0;
	u.priority=0;
	u.type=0;
	u.link[0]=0;
	u.title[0]=0;
	u.desc[0]=0;
	u.date[0]=0;
#endif
	u.clen=0;
	u.in_topic=0;
	u.in_extpage=0;
	u.count=1;
	u.list1=NULL;
	u.clist=NULL;
	u.total=STOPWORDS;
	memset(u.list,0,sizeof(u.list));
	memset(u.block,0,sizeof(u.block));
	if(mkstore(&u))goto out0;
	for(u.start=1;u.start>0&&u.start<u.total;u.start<<=1);
	if(u.start<=0)
	{
		printf("overflow error\n");
		goto out0;
	}
	else u.start>>=1;
	if(!(fp=gzopen(contentfile,"r")))
	{
		printf("%s: no such file\n",contentfile);
		goto out0;
	}
	if(sqlopen(database,user,password,&u.db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out1;
	if(sqlrun(u.db,all?ALLEXTERNALPAGESFMT:STDEXTERNALPAGESFMT))goto out2;
	if(sqllock(u.db,EXTERNALPAGESTBL))goto out2;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,all?ALLEXTERNALPAGESPRP "," ALLEXTERNALPAGESROW
		"," ALLEXTERNALPAGESROW "," ALLEXTERNALPAGESROW ","
		ALLEXTERNALPAGESROW "," ALLEXTERNALPAGESROW ","
		ALLEXTERNALPAGESROW "," ALLEXTERNALPAGESROW:STDEXTERNALPAGESPRP
		"," STDEXTERNALPAGESROW "," STDEXTERNALPAGESROW ","
		STDEXTERNALPAGESROW "," STDEXTERNALPAGESROW ","
		STDEXTERNALPAGESROW "," STDEXTERNALPAGESROW ","
		STDEXTERNALPAGESROW,all?ALLEXTERNALPAGESINS ALLEXTERNALPAGESINS
		ALLEXTERNALPAGESINS ALLEXTERNALPAGESINS ALLEXTERNALPAGESINS
		ALLEXTERNALPAGESINS ALLEXTERNALPAGESINS ALLEXTERNALPAGESINS:
		STDEXTERNALPAGESINS STDEXTERNALPAGESINS STDEXTERNALPAGESINS
		STDEXTERNALPAGESINS STDEXTERNALPAGESINS STDEXTERNALPAGESINS
		STDEXTERNALPAGESINS STDEXTERNALPAGESINS,NULL))goto out3;
#else
	if(sqlprep(u.db,&u.sql,all?ALLEXTERNALPAGESPRP:STDEXTERNALPAGESPRP,
		all?ALLEXTERNALPAGESINS:STDEXTERNALPAGESINS,NULL))goto out3;
#endif
	line=1;
	l=0;
	while((len=gzread(fp,bfr+l,sizeof(bfr)-l))>0)
	{
		len+=l;
		if((l=parse(bfr,len,&line,0,1,&u,contentdata,contentstart,
			contentend))==-1)goto out4;
		memmove(bfr,bfr+l,len-l);
		l=len-l;
	}
	if(l)if(parse(bfr,l,&line,1,1,&u,contentdata,contentstart,
		contentend)==-1)goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
#ifdef SQLFLAG_MULTIROW
	if(u.idx)
	{
		if(sqlprep(u.db,&u.sql,all?ALLEXTERNALPAGESPRP:
			STDEXTERNALPAGESPRP,all?ALLEXTERNALPAGESINS:
			STDEXTERNALPAGESINS,NULL))goto out3;
		if(all)for(i=0;i<u.idx;i++)
		{
		    if(sqlinsert(u.db,u.sql,ALLEXTERNALPAGESINS,
			u.count++,u.catid[i],u.mem[i][0],u.mem[i][1],
			u.priority[i],u.type[i],u.link[i],u.title[i],
			u.desc[i],u.date[i]))goto out4;
		}
		else for(i=0;i<u.idx;i++)if(sqlinsert(u.db,u.sql,
			STDEXTERNALPAGESINS,u.count++,u.catid[i],u.mem[i][0],
			u.priority[i],u.link[i],u.title[i],u.desc[i],u.date[i]))
			goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
#endif
	pfree(&cache.lcl);
	if(sqlunlock(u.db))goto out2;
	if(all)
	{
		if(sqlrun(u.db,TYPESFMT))goto out2;
		if(sqllock(u.db,TYPESTBL))goto out2;
#ifdef SQLFLAG_MULTIROW
		if(sqlprep(u.db,&u.sql,TYPESPRP "," TYPESROW "," TYPESROW ","
			TYPESROW "," TYPESROW "," TYPESROW "," TYPESROW ","
			TYPESROW,TYPESINS TYPESINS TYPESINS TYPESINS TYPESINS
			TYPESINS TYPESINS TYPESINS,NULL))goto out3;
		for(j=0,u.count=1,r=u.list1;r;r=r->next)
		{
			rl[j++]=r->item;
			if(j==8)
			{
				if(sqlinsert(u.db,u.sql,TYPESINS TYPESINS
					TYPESINS TYPESINS TYPESINS TYPESINS
					TYPESINS TYPESINS,
					u.count,rl[0],u.count+1,rl[1],
					u.count+2,rl[2],u.count+3,rl[3],
					u.count+4,rl[4],u.count+5,rl[5],
					u.count+6,rl[6],u.count+7,rl[7]))
					goto out4;
				u.count+=8;
				j=0;
			}
		}
		if(sqlfin(u.db,u.sql))goto out3;
		if(j)
		{
			if(sqlprep(u.db,&u.sql,TYPESPRP,TYPESINS,NULL))
				goto out3;
			for(k=0;k<j;k++)if(sqlinsert(u.db,u.sql,TYPESINS,
				u.count++,rl[k]))goto out4;
			if(sqlfin(u.db,u.sql))goto out3;
		}
#else
		if(sqlprep(u.db,&u.sql,TYPESPRP,TYPESINS,NULL))goto out3;
		for(u.count=1,r=u.list1;r;r=r->next,u.count++)if(sqlinsert(u.db,
			u.sql,TYPESINS,u.count,r->item))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
#endif
		if(sqlunlock(u.db))goto out2;
	}
	for(total=i=0;i<LISTSIZE;i++)for(w=u.list[i];w;w=w->next)total++;
	if(!(rr=palloc(&cache.aux,total*sizeof(WORDLIST *))))goto out2;
	for(max=l=i=0;i<LISTSIZE;i++)for(w=u.list[i];w;w=w->next)
	{
		rr[l++]=w;
		if(w->occurrence>max)max=w->occurrence;
	}
	qsort(rr,total,sizeof(WORDLIST *),wsort);
	if(sqlrun(u.db,all?ALLPAGEWORDSFMT:STDPAGEWORDSFMT))goto out2;
	if(sqllock(u.db,PAGEWORDSTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,all?ALLPAGEWORDSPRP "," ALLPAGEWORDSROW ","
		ALLPAGEWORDSROW "," ALLPAGEWORDSROW "," ALLPAGEWORDSROW ","
		ALLPAGEWORDSROW "," ALLPAGEWORDSROW "," ALLPAGEWORDSROW:
		STDPAGEWORDSPRP "," STDPAGEWORDSROW "," STDPAGEWORDSROW ","
		STDPAGEWORDSROW "," STDPAGEWORDSROW "," STDPAGEWORDSROW ","
		STDPAGEWORDSROW "," STDPAGEWORDSROW,all?ALLPAGEWORDSINS
		ALLPAGEWORDSINS ALLPAGEWORDSINS ALLPAGEWORDSINS ALLPAGEWORDSINS
		ALLPAGEWORDSINS ALLPAGEWORDSINS ALLPAGEWORDSINS:STDPAGEWORDSINS
		STDPAGEWORDSINS STDPAGEWORDSINS STDPAGEWORDSINS STDPAGEWORDSINS
		STDPAGEWORDSINS STDPAGEWORDSINS STDPAGEWORDSINS,NULL))goto out3;
	if(all)
	{
		for(;i+8<=total;i+=8)if(sqlinsert(u.db,u.sql,ALLPAGEWORDSINS
			ALLPAGEWORDSINS ALLPAGEWORDSINS ALLPAGEWORDSINS
			ALLPAGEWORDSINS ALLPAGEWORDSINS ALLPAGEWORDSINS
			ALLPAGEWORDSINS,
			i+1,rr[i]->word,rr[i]->occurrence,
			i+2,rr[i+1]->word,rr[i+1]->occurrence,
			i+3,rr[i+2]->word,rr[i+2]->occurrence,
			i+4,rr[i+3]->word,rr[i+3]->occurrence,
			i+5,rr[i+4]->word,rr[i+4]->occurrence,
			i+6,rr[i+5]->word,rr[i+5]->occurrence,
			i+7,rr[i+6]->word,rr[i+6]->occurrence,
			i+8,rr[i+7]->word,rr[i+7]->occurrence))goto out4;
	}
	else for(;i+8<=total;i+=8)if(sqlinsert(u.db,u.sql,STDPAGEWORDSINS
		STDPAGEWORDSINS STDPAGEWORDSINS STDPAGEWORDSINS STDPAGEWORDSINS
		STDPAGEWORDSINS STDPAGEWORDSINS STDPAGEWORDSINS,
		i+1,rr[i]->word,i+2,rr[i+1]->word,i+3,rr[i+2]->word,
		i+4,rr[i+3]->word,i+5,rr[i+4]->word,i+6,rr[i+5]->word,
		i+7,rr[i+6]->word,i+8,rr[i+7]->word))goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<total)
#endif
	{
		if(sqlprep(u.db,&u.sql,all?ALLPAGEWORDSPRP:STDPAGEWORDSPRP,
			all?ALLPAGEWORDSINS:STDPAGEWORDSINS,NULL))goto out3;
		if(all)
		{
			for(;i<total;i++)if(sqlinsert(u.db,u.sql,
				ALLPAGEWORDSINS,i+1,rr[i]->word,
				rr[i]->occurrence))goto out4;
		}
		else for(;i<total;i++)if(sqlinsert(u.db,u.sql,STDPAGEWORDSINS,
			i+1,rr[i]->word))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	if(!(pd=palloc(&cache.aux,max*sizeof(PAGEDATA))))goto out2;
	if(sqlrun(u.db,PAGESEARCHFMT))goto out2;
	if(sqllock(u.db,PAGESEARCHTBL))goto out2;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,PAGESEARCHPRP "," PAGESEARCHROW ","
		PAGESEARCHROW "," PAGESEARCHROW "," PAGESEARCHROW ","
		PAGESEARCHROW "," PAGESEARCHROW "," PAGESEARCHROW,PAGESEARCHINS
		PAGESEARCHINS PAGESEARCHINS PAGESEARCHINS PAGESEARCHINS
		PAGESEARCHINS PAGESEARCHINS PAGESEARCHINS,NULL))
		goto out3;
	for(j=0,u.count=1,i=0;i<total;i++)
	{
		dval=WORDWEIGHT(rr[i]->occurrence);
		if(rr[i]->occurrence==1)
		{
			int catid;
			int totals;
			int ages;

			flags=rr[i]->u.page;
			getstore(&u,flags&PAGEMASK,&catid,&totals,&ages);
			pd[0].pageid=(flags&PAGEMASK)+1;
			pd[0].catid=catid;
			pd[0].ages=ages;
			if(flags&PRIORITY)pd[0].weight=PRIOWEIGHT(dval,totals);
			else if((flags&INTITLE)&&(flags&INDESCR))
				pd[0].weight=HIGHWEIGHT(dval,totals);
			else if(flags&INTITLE)
				pd[0].weight=MOREWEIGHT(dval,totals);
			else pd[0].weight=ITEMWEIGHT(dval,totals);
		}
		else for(l=0;l<rr[i]->occurrence;l++)
		{
			int catid;
			int totals;
			int ages;

			flags=rr[i]->u.pagelist[l];
			getstore(&u,flags&PAGEMASK,&catid,&totals,&ages);
			pd[l].pageid=(flags&PAGEMASK)+1;
			pd[l].catid=catid;
			pd[l].ages=ages;
			if(flags&PRIORITY)pd[l].weight=PRIOWEIGHT(dval,totals);
			else if((flags&INTITLE)&&(flags&INDESCR))
				pd[l].weight=HIGHWEIGHT(dval,totals);
			else if(flags&INTITLE)
				pd[l].weight=MOREWEIGHT(dval,totals);
			else pd[l].weight=ITEMWEIGHT(dval,totals);
		}
		qsort(pd,rr[i]->occurrence,sizeof(PAGEDATA),psort);
		for(l=0;l<rr[i]->occurrence;l++)
		{
			mem[j][0]=i+1;
			mem[j][1]=pd[l].weight;
			mem[j][2]=pd[l].pageid;
			mem[j][3]=pd[l].catid;
			mem[j][4]=pd[l].ages;
			if(++j<8)continue;
			if(sqlinsert(u.db,u.sql,PAGESEARCHINS PAGESEARCHINS
				PAGESEARCHINS PAGESEARCHINS PAGESEARCHINS
				PAGESEARCHINS PAGESEARCHINS PAGESEARCHINS,
			u.count,mem[0][0],mem[0][1],mem[0][2],
			mem[0][3],mem[0][4],
			u.count+1,mem[1][0],mem[1][1],mem[1][2],
			mem[1][3],mem[1][4],
			u.count+2,mem[2][0],mem[2][1],mem[2][2],
			mem[2][3],mem[2][4],
			u.count+3,mem[3][0],mem[3][1],mem[3][2],
			mem[3][3],mem[3][4],
			u.count+4,mem[4][0],mem[4][1],mem[4][2],
			mem[4][3],mem[4][4],
			u.count+5,mem[5][0],mem[5][1],mem[5][2],
			mem[5][3],mem[5][4],
			u.count+6,mem[6][0],mem[6][1],mem[6][2],
			mem[6][3],mem[6][4],
			u.count+7,mem[7][0],mem[7][1],mem[7][2],
			mem[7][3],mem[7][4]))goto out4;
			u.count+=8;
			j=0;
		}
	}
	if(sqlfin(u.db,u.sql))goto out3;
	if(j)
	{
		if(sqlprep(u.db,&u.sql,PAGESEARCHPRP,PAGESEARCHINS,NULL))
			goto out3;
		for(i=0;i<j;i++)if(sqlinsert(u.db,u.sql,PAGESEARCHINS,u.count++,
			mem[i][0],mem[i][1],mem[i][2],mem[i][3],mem[0][i]))
			goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
#else
	if(sqlprep(u.db,&u.sql,PAGESEARCHPRP,PAGESEARCHINS,NULL))goto out3;
	for(u.count=1,i=0;i<total;i++)
	{
		dval=WORDWEIGHT(rr[i]->occurrence);
		if(rr[i]->occurrence==1)
		{
			int catid;
			int totals;
			int ages;

			flags=rr[i]->u.page;
			getstore(&u,flags&PAGEMASK,&catid,&totals,&ages);
			pd[0].pageid=(flags&PAGEMASK)+1;
			pd[0].catid=catid;
			pd[0].ages=ages;
			if(flags&PRIORITY)pd[0].weight=PRIOWEIGHT(dval,totals);
			else if((flags&INTITLE)&&(flags&INDESCR))
				pd[0].weight=HIGHWEIGHT(dval,totals);
			else if(flags&INTITLE)
				pd[0].weight=MOREWEIGHT(dval,totals);
			else pd[0].weight=ITEMWEIGHT(dval,totals);
		}
		else for(l=0;l<rr[i]->occurrence;l++)
		{
			int catid;
			int totals;
			int ages;

			flags=rr[i]->u.pagelist[l];
			getstore(&u,flags&PAGEMASK,&catid,&totals,&ages);
			pd[l].pageid=(flags&PAGEMASK)+1;
			pd[l].catid=catid;
			pd[l].ages=ages;
			if(flags&PRIORITY)pd[l].weight=PRIOWEIGHT(dval,totals);
			else if((flags&INTITLE)&&(flags&INDESCR))
				pd[l].weight=HIGHWEIGHT(dval,totals);
			else if(flags&INTITLE)
				pd[l].weight=MOREWEIGHT(dval,totals);
			else pd[l].weight=ITEMWEIGHT(dval,totals);
		}
		qsort(pd,rr[i]->occurrence,sizeof(PAGEDATA),psort);
		for(l=0;l<rr[i]->occurrence;l++)if(sqlinsert(u.db,u.sql,
			PAGESEARCHINS,u.count++,i+1,pd[l].weight,pd[l].pageid,
			pd[l].catid,pd[l].ages))goto out4;
	}
	if(sqlfin(u.db,u.sql))goto out3;
#endif
	if(sqlunlock(u.db))goto out2;
	err=0;
out4:	if(err)sqlfin(u.db,u.sql);
out3:	if(err)sqlunlock(u.db);
out2:	if(sqlclose(u.db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out1:	gzclose(fp);
out0:	pfree(&cache.aux);
	pfree(&cache.lcl);
	return err;
}

static int counttopics(int catid,int *counter,int depth)
{
	int current;
	int total;
	int result;
	int k;

	if(++depth==MAXDEPTH)
	{
		printf("loop detected at %d\n",catid);
		return -1;
	}
	if(!findbycatid(catid,&current))
	{
		printf("catid %d not found\n",catid);
		return -1;
	}
	if(counter[current])
	{
		if(verbose)
			printf("Warning: possible loop for catid %d\n",catid);
		return counter[current];
	}
	total=1;
	if((k=findintree(catid))!=-1)for(;cache.tree[k].src==catid;k++)
	{
		if((result=counttopics(cache.tree[k].dst,counter,depth))==-1)
			return -1;
		total+=result;
	}
	if(counter[current])
	{
		if(verbose)
			printf("Warning: possible loop for catid %d\n",catid);
		return counter[current];
	}
	counter[current]=total;
	return total;
}

static int countpages(int catid,int *counter,int depth)
{
	int current;
	int total;
	int result;
	int k;
	TOPIC *r;

	if(++depth==MAXDEPTH)
	{
		printf("loop detected at %d\n",catid);
		return -1;
	}
	if(!findbycatid(catid,&current))
	{
		printf("catid %d not found\n",catid);
		return -1;
	}
	if(counter[current])
	{
		if(verbose)
			printf("Warning: possible loop for catid %d\n",catid);
		return counter[current]-1;
	}
	total=0;
	if((k=findintree(catid))!=-1)for(;cache.tree[k].src==catid;k++)
	{
		if((result=countpages(cache.tree[k].dst,counter,depth))==-1)
			return -1;
		total+=result;
	}
	if((r=findbycatid(catid,NULL)))total+=r->u.counter;
	if(counter[current])
	{
		if(verbose)
			printf("Warning: possible loop for catid %d\n",catid);
		return counter[current]-1;
	}
	counter[current]=total+1;
	return total;
}

static int build_counts(char *database,char *user,char *password)
{
	int i;
	int err;
	DB db;
	SQL qry;
	int *topics;
	int *pages;

	err=-1;
	pages=NULL;
	if(sqlopen(database,user,password,&db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out0;
	if(!(topics=palloc(&cache.aux,cache.total*sizeof(int))))goto out1;
	memset(topics,0,cache.total*sizeof(int));
	for(i=0;i<cache.total;i++)if(!topics[i])
		if(counttopics(cache.catid[i]->catid,topics,0)==-1)goto out1;
	if(!(pages=palloc(&cache.aux,cache.total*sizeof(int))))goto out1;
	memset(pages,0,cache.total*sizeof(int));
	for(i=0;i<cache.total;i++)if(!pages[i])
		if(countpages(cache.catid[i]->catid,pages,0)==-1)goto out1;
	if(sqlrun(db,COUNTERSFMT))goto out1;
	if(sqllock(db,COUNTERSTBL))goto out1;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(db,&qry,COUNTERSPRP "," COUNTERSROW "," COUNTERSROW ","
		COUNTERSROW "," COUNTERSROW "," COUNTERSROW "," COUNTERSROW
		"," COUNTERSROW,COUNTERSINS COUNTERSINS COUNTERSINS COUNTERSINS
		COUNTERSINS COUNTERSINS COUNTERSINS COUNTERSINS,NULL))goto out2;
	for(;i+8<=cache.total;i+=8)if(sqlinsert(db,qry,COUNTERSINS COUNTERSINS
		COUNTERSINS COUNTERSINS COUNTERSINS COUNTERSINS COUNTERSINS
		COUNTERSINS,
		cache.catid[i]->catid,topics[i]-1,pages[i]-1,
		cache.catid[i+1]->catid,topics[i+1]-1,pages[i+1]-1,
		cache.catid[i+2]->catid,topics[i+2]-1,pages[i+2]-1,
		cache.catid[i+3]->catid,topics[i+3]-1,pages[i+3]-1,
		cache.catid[i+4]->catid,topics[i+4]-1,pages[i+4]-1,
		cache.catid[i+5]->catid,topics[i+5]-1,pages[i+5]-1,
		cache.catid[i+6]->catid,topics[i+6]-1,pages[i+6]-1,
		cache.catid[i+7]->catid,topics[i+7]-1,pages[i+7]-1))goto out3;
	if(sqlfin(db,qry))goto out2;
	if(i<cache.total)
#endif
	{
		if(sqlprep(db,&qry,COUNTERSPRP,COUNTERSINS,NULL))goto out2;
		for(;i<cache.total;i++)if(sqlinsert(db,qry,COUNTERSINS,
			cache.catid[i]->catid,topics[i]-1,pages[i]-1))goto out3;
		if(sqlfin(db,qry))goto out2;
	}
	if(sqlunlock(db))goto out1;
	err=0;

out3:	if(err)sqlfin(db,qry);
out2:	if(err)sqlunlock(db);
out1:	if(sqlclose(db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out0:	pfree(&cache.aux);
	return err;
}

static int redirectstart(char *name,char *attr,char *value,void *_u)
{
	REDIRECTDATA *u;

	u=(REDIRECTDATA *)(_u);
	if(!strcasecmp(name,"Topic"))
	{
		u->id[0]=0;
		u->target[0]=0;
		u->clen=0;
		if(!strcasecmp(attr,"r:id"))
			if(srccpy(u->id,value,sizeof(u->id)))
		{
			printf("id %s too long\n",value);
			return -1;
		}
	}
	else if(!strcasecmp(name,"redirect"))
	{
		if(!strcasecmp(attr,"r:resource"))
			if(srccpy(u->target,value,sizeof(u->target)))
		{
			printf("target %s too long\n",value);
			return -1;
		}
	}
	u->clen=0;
	return 0;
}

static int redirectend(char *name,void *_u)
{
	int catid;
	REDIRECTDATA *u;
	TOPIC *r;
	REDIRECT *rd;

	u=(REDIRECTDATA *)(_u);
	if(!strcasecmp(name,"Topic"))
	{
		if(!u->id[0]||!u->target[0])
		{
			printf("missing data\n");
			return -1;
		}
		if(!(r=findbyid(u->target)))
		{
			if(verbose)
			    printf("Warning: target %s not found\n",u->target);
			catid=0;
		}
		else catid=r->catid;
		if(catid)if(checkutf8str(u->id,NULL))
		{
			if(verbose)printf("Warning: source %s is not valid "
				"UTF-8\n",u->id);
			catid=0;
		}
		if(catid)if(checkutf8unicodestr(u->id))
		{
			if(verbose)printf("Warning: source %s is not unicode\n",
				u->id);
			catid=0;
		}
		if(catid)
		{
			if(!(rd=palloc(&cache.aux,sizeof(REDIRECT)+
				strlen(u->id)+1)))return -1;
			rd->next=u->redirchain;
			u->redirchain=rd;
			rd->catid=catid;
			strcpy(rd->id,u->id);
			u->total++;
		}
	}
	else if(verbose)
	{
		if(!strcasecmp(name,"redirect"));
		else if(!strcasecmp(name,"RDF"));
		else printf("Warning: unknown tag %s\n",name);
	}
	u->clen=0;
	return 0;
}

static int redirectdata(char *s,int len,void *_u)
{
	REDIRECTDATA *u;

	u=(REDIRECTDATA *)(_u);
	if(len>1)if(s[len-2]=='\r')if(s[len-1]=='\n')
	{
		s[len-2]='\n';
		len--;
	}
	if(u->clen+len>=sizeof(u->cdata))
	{
		printf("data oversize: have %d, need %d\n",
			(int)sizeof(u->cdata),u->clen+len);
		return -1;
	}
	memcpy(u->cdata+u->clen,s,len);
	u->clen+=len;
	return 0;
}

static int get_redirects(char *redirectfile,char *database,char *user,
	char *password)
{
	int i;
	int line;
	int len;
	int err;
	REDIRECT **r;
	gzFile fp;
	REDIRECTDATA u;
	unsigned char bfr[BUFSIZEH];

	err=-1;
	u.id[0]=0;
	u.target[0]=0;
	u.clen=0;
	u.total=0;
	u.redirchain=NULL;
	if(!(fp=gzopen(redirectfile,"r")))goto out0;
	if(sqlopen(database,user,password,&u.db,SQLFLAGS_LOWMEM|SQLFLAGS_BEGIN))
		goto out1;
	line=1;
	i=0;
	while((len=gzread(fp,bfr+i,sizeof(bfr)-i))>0)
	{
		len+=i;
		if((i=parse(bfr,len,&line,0,0,&u,redirectdata,redirectstart,
			redirectend))==-1)goto out2;
		memmove(bfr,bfr+i,len-i);
		i=len-i;
	}
	if(i)if(parse(bfr,i,&line,1,0,&u,redirectdata,redirectstart,
		redirectend)==-1)goto out2;
	if(!(r=palloc(&cache.aux,u.total*sizeof(REDIRECT *))))goto out2;
	for(i=0;u.redirchain;u.redirchain=u.redirchain->next,i++)
		r[i]=u.redirchain;
	qsort(r,u.total,sizeof(REDIRECT *),rdsort);
	if(sqlrun(u.db,REDIRECTSFMT))goto out2;
	if(sqllock(u.db,REDIRECTSTBL))goto out2;
	i=0;
#ifdef SQLFLAG_MULTIROW
	if(sqlprep(u.db,&u.sql,REDIRECTSPRP "," REDIRECTSROW "," REDIRECTSROW
		"," REDIRECTSROW "," REDIRECTSROW "," REDIRECTSROW ","
		REDIRECTSROW "," REDIRECTSROW,REDIRECTSINS REDIRECTSINS
		REDIRECTSINS REDIRECTSINS REDIRECTSINS REDIRECTSINS REDIRECTSINS
		REDIRECTSINS,NULL))goto out3;
	for(;i+8<=u.total;i+=8)if(sqlinsert(u.db,u.sql,REDIRECTSINS
		REDIRECTSINS REDIRECTSINS REDIRECTSINS REDIRECTSINS
		REDIRECTSINS REDIRECTSINS REDIRECTSINS,
		i+1,r[i]->id,r[i]->catid,i+2,r[i+1]->id,r[i+1]->catid,
		i+3,r[i+2]->id,r[i+2]->catid,i+4,r[i+3]->id,r[i+3]->catid,
		i+5,r[i+4]->id,r[i+4]->catid,i+6,r[i+5]->id,r[i+5]->catid,
		i+7,r[i+6]->id,r[i+6]->catid,i+8,r[i+7]->id,r[i+7]->catid))
		goto out4;
	if(sqlfin(u.db,u.sql))goto out3;
	if(i<u.total)
#endif
	{
		if(sqlprep(u.db,&u.sql,REDIRECTSPRP,REDIRECTSINS,NULL))
			goto out3;
		for(;i<u.total;i++)if(sqlinsert(u.db,u.sql,REDIRECTSINS,i+1,
			r[i]->id,r[i]->catid))goto out4;
		if(sqlfin(u.db,u.sql))goto out3;
	}
	if(sqlunlock(u.db))goto out2;
	err=0;
out4:	if(err)sqlfin(u.db,u.sql);
out3:	if(err)sqlunlock(u.db);
out2:	if(sqlclose(u.db,err?SQLFLAGS_ROLLBACK:SQLFLAGS_COMMIT))err=-1;
out1:	gzclose(fp);
out0:	pfree(&cache.aux);
	return err;
}

static int builder(char *database,char *user,char *password,char *sname,
	char *cname,char *rname,char *tname)
{
	int err;

	err=-1;
	memset(&cache,0,sizeof(cache));
	printf("preparing database...\n");
	if(sqlreset(database,user,password))goto out;
	printf("processing structure (1/2)...\n");
	if(get_structure1(sname,database,user,password))goto out;
	printf("processing terms...\n");
	if(get_terms(tname,database,user,password))goto out;
	printf("building topic search tables...\n");
	if(build_topicsearch(database,user,password))goto out;
	printf("building topic reference tables...\n");
	if(build_topicref(database,user,password))goto out;
	printf("processing structure (2/2)...\n");
	if(get_structure2(sname,database,user,password))goto out;
	printf("verifying internal language list...\n");
	if(verify_langlist())goto out;
	printf("building nls support table...\n");
	if(build_nlssupport(database,user,password))goto out;
	printf("building nls info table...\n");
	if(build_nlsinfo(database,user,password))goto out;
	printf("processing content...\n");
	if(get_content(cname,database,user,password))goto out;
	printf("building counters table...\n");
	if(build_counts(database,user,password))goto out;
	printf("processing redirects...\n");
	if(get_redirects(rname,database,user,password))goto out;
	err=0;
out:	pfree(&cache.lcl);
	pfree(&cache.aux);
	pfree(&cache.main);
	return err;
}

static int indexer(char *database,char *user,char *password)
{
	int i;
	int err;
	DB db;

	err=-1;
	printf("building indexes...\n");
	if(sqlopen(database,user,password,&db,SQLFLAGS_HIGHMEM))goto out0;
	for(i=0;indexes[i];i++)if(sqlrun(db,(char *)(indexes[i])))goto out1;
	err=0;
out1:	if(sqlclose(db,0))err=-1;
out0:	return err;
}

static void usage(void)
{
	printf("Usage: buildmoz [-va] [-d db] [-s structure] [-c content]\n");
	printf("                [-r redirect] [-t terms] [-u user]\n");
	printf("                [-p password|-]\n");
	printf("                '-' as password means read from stdin\n");
	printf("                -v  verbose mode\n");
	printf("                -a  generate all possible data\n");
	exit(1);
}

int main(int argc,char *argv[])
{
	int i;
	int c;
	char *db=SQLDFLT_DB;
	char *user=SQLDFLT_USER;
	char *pass=SQLDFLT_PASS;
	char *structure="structure.rdf.u8.gz";
	char *content="content.rdf.u8.gz";
	char *redirect="redirect.rdf.u8.gz";
	char *terms="terms.rdf.u8.gz";
	char bfr[256];

	while((c=getopt(argc,argv,"vas:c:r:t:d:u:p:"))!=-1)switch(c)
	{
	case 'v':
		verbose=1;
		break;
	case 'a':
		all=1;
		break;
	case 's':
		structure=optarg;
		break;
	case 'c':
		content=optarg;
		break;
	case 'r':
		redirect=optarg;
		break;
	case 't':
		terms=optarg;
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
	default:
		usage();
	}

	if(optind<argc)usage();

	if(sqlinit(db))
	{
		printf("sql initialization failure\n");
		if(pass)memset(pass,0,strlen(pass));
		return 1;
	}
	if(builder(db,user,pass,structure,content,redirect,terms))
	{
		if(pass)memset(pass,0,strlen(pass));
		sqlexit();
		return 1;
	}
	if(indexer(db,user,pass))
	{
		if(pass)memset(pass,0,strlen(pass));
		sqlexit();
		return 1;
	}
	if(pass)memset(pass,0,strlen(pass));
	sqlexit();
	printf("done!\n");
	return 0;
}
