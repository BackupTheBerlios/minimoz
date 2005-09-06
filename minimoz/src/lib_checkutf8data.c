/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

#include "lib.h"

int checkutf8data(unsigned char *s,int len)
{
	while(len>=6)
	{
		if(!(*s&0x80))
		{
			s++;
			len--;
		}
		else if((*s&0xe0)==0xc0)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			s+=2;
			len-=2;
		}
		else if((*s&0xf0)==0xe0)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			s+=3;
			len-=3;
		}
		else if((*s&0xf8)==0xf0)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			if((s[3]&0xc0)!=0x80)return -1;
			s+=4;
			len-=4;
		}
		else if((*s&0xfc)==0xf8)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			if((s[3]&0xc0)!=0x80)return -1;
			if((s[4]&0xc0)!=0x80)return -1;
			s+=5;
			len-=5;
		}
		else if((*s&0xfe)==0xfc)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			if((s[3]&0xc0)!=0x80)return -1;
			if((s[4]&0xc0)!=0x80)return -1;
			if((s[5]&0xc0)!=0x80)return -1;
			s+=6;
			len-=6;
		}
		else return -1;
	}
	while(len)
	{
		if(!(*s&0x80))
		{
			s++;
			len--;
		}
		else if((*s&0xe0)==0xc0)
		{
			if(len<2)return -1;
			if((s[1]&0xc0)!=0x80)return -1;
			s+=2;
			len-=2;
		}
		else if((*s&0xf0)==0xe0)
		{
			if(len<3)return -1;
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			s+=3;
			len-=3;
		}
		else if((*s&0xf8)==0xf0)
		{
			if(len<4)return -1;
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			if((s[3]&0xc0)!=0x80)return -1;
			s+=4;
			len-=4;
		}
		else if((*s&0xfc)==0xf8)
		{
			if(len<5)return -1;
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			if((s[3]&0xc0)!=0x80)return -1;
			if((s[4]&0xc0)!=0x80)return -1;
			s+=5;
			len-=5;
		}
		else return -1;
	}
	return 0;
}
