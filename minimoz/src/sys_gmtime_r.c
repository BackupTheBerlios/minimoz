/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <time.h>
#include "lib.h"

static const int m[12]={31,28,31,30,31,30,31,31,30,31,30,31};

struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
	int val;
	int rem;

	val=(int)(*timep/86400);
	rem=(int)(*timep%86400);

	result->tm_wday=(val+4)%7;

	result->tm_hour=rem/3600;
	rem%=3600;
	result->tm_min=rem/60;
	result->tm_sec=rem%60;

	result->tm_year=val/365;
	rem=val%365;
	result->tm_year+=1970;

	for(val=1972;val<result->tm_year;val+=4)if(!(val%400))rem--;
	else if(val%100)rem--;

	while(rem<0)
	{
		result->tm_year-=1;
		rem+=365;
		if(!(result->tm_year%4))
		{
			if(!(val%400))rem++;
			else if(val%100)rem++;
		}
	}
	result->tm_yday=rem;

	for(result->tm_mon=0;result->tm_mon<12;result->tm_mon+=1)
		if(result->tm_mon==1&&!(result->tm_year%4))
	{
		val=m[1];
		if(!(result->tm_year%400))val++;
		else if(result->tm_year%100)val++;
		if(rem>=val)rem-=val;
		else break;
	}
	else if(rem>=m[result->tm_mon])rem-=m[result->tm_mon];
	else break;

	result->tm_mday=rem+1;
	result->tm_year-=1900;
	result->tm_isdst=0;

	return result;
}
