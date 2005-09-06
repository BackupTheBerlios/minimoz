/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include <time.h>
#include "lib.h"

static const char *wd[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const char *m[12]=
{"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

size_t strftime(char *s, size_t max, const char *format,const struct tm *tm)
{
	size_t l;

	if(!max)return 0;

	for(l=0;*format;format++)switch(*format)
	{
	case '%':
		switch(format[1])
		{
		case 'a':
			if(l+3>=max)
			{
				*s=0;
				return 0;
			}
			s[l++]=wd[tm->tm_wday][0];
			s[l++]=wd[tm->tm_wday][1];
			s[l++]=wd[tm->tm_wday][2];
			format++;
			break;
		case 'b':
			if(l+3>=max)
			{
				*s=0;
				return 0;
			}
			s[l++]=m[tm->tm_mon][0];
			s[l++]=m[tm->tm_mon][1];
			s[l++]=m[tm->tm_mon][2];
			format++;
			break;
		case 'd':
			if(l+2>=max)
			{
				*s=0;
				return 0;
			}
			s[l++]=tm->tm_mday/10+'0';
			s[l++]=tm->tm_mday%10+'0';
			format++;
			break;
		case 'Y':
			if(l+4>=max)
			{
				*s=0;
				return 0;
			}
			s[l++]=(tm->tm_year/1000)%10+'0';
			s[l++]=(tm->tm_year/100)%10+'0';
			s[l++]=(tm->tm_year/10)%10+'0';
			s[l++]=tm->tm_year%10+'0';
			format++;
			break;
		case 'H':
			if(l+2>=max)
			{
				*s=0;
				return 0;
			}
			s[l++]=tm->tm_hour/10+'0';
			s[l++]=tm->tm_hour%10+'0';
			format++;
			break;
		case 'M':
			if(l+2>=max)
			{
				*s=0;
				return 0;
			}
			s[l++]=tm->tm_min/10+'0';
			s[l++]=tm->tm_min%10+'0';
			format++;
			break;
		case 'S':
			if(l+2>=max)
			{
				*s=0;
				return 0;
			}
			s[l++]=tm->tm_sec/10+'0';
			s[l++]=tm->tm_sec%10+'0';
			format++;
			break;
		case '%':
			s[l++]='%';
			if(l==max)
			{
				*s=0;
				return 0;
			}
		default:format++;
		case 0:	break;
		}
		break;
	case '\\':
		switch(format[1])
		{
		case 'n':
			s[l++]='\n';
			if(l==max)
			{
				*s=0;
				return 0;
			}
			format++;
			break;
		case 't':
			s[l++]='\t';
			if(l==max)
			{
				*s=0;
				return 0;
			}
			format++;
			break;
		case 'r':
			s[l++]='\r';
			if(l==max)
			{
				*s=0;
				return 0;
			}
		default:format++;
		case 0:	break;
		}
		break;
	default:s[l++]=*format;
		if(l==max)
		{
			*s=0;
			return 0;
		}
		break;
	}
	s[l]=0;
	return l;
}
