#include "db.h"

int sqlping(DB db)
{
	if(PQstatus(db)==CONNECTION_BAD)return -1;
	return 0;
}
