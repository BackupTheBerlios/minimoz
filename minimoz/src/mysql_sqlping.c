#include "db.h"

int sqlping(DB db)
{
	if(mysql_ping(db))return -1;
	return 0;
}
