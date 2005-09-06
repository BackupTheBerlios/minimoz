/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

/*
 * From RFC2044:
 * =============
 *
 * UCS-4 range (hex.)           UTF-8 octet sequence (binary)
 * 0000 0000-0000 007F   0xxxxxxx
 * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
 * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
 *
 * 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx
 *
 */

#include "lib.h"

int utf8_to_ucs4(unsigned char *in,int len,unsigned int *out)
{
	if(!(*in&0x80))
	{
		if(len<1)return 0;
		*out=*in;
		return 1;
	}
	else if((*in&0xe0)==0xc0)
	{
		if(len<2)return 0;
		*out=((unsigned int)(*in++&0x1f))<<6;
		if((*in&0xc0)!=0x80)return 0;
		*out|=(unsigned int)(*in&0x3f);
		if(*out<0x80)return 0;
		return 2;
	}
	else if((*in&0xf0)==0xe0)
	{
		if(len<3)return 0;
		*out=((unsigned int)(*in++&0x0f))<<12;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<6;
		if((*in&0xc0)!=0x80)return 0;
		*out|=(unsigned int)(*in&0x3f);
		if(*out<0x800)return 0;
		return 3;
	}
	else if((*in&0xf8)==0xf0)
	{
		if(len<4)return 0;
		*out=((unsigned int)(*in++&0x07))<<18;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<12;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<6;
		if((*in&0xc0)!=0x80)return 0;
		*out|=(unsigned int)(*in&0x3f);
		if(*out<0x1000)return 0;
		return 4;
	}
	else if((*in&0xfc)==0xf8)
	{
		if(len<5)return 0;
		*out=((unsigned int)(*in++&0x03))<<24;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<18;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<12;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<6;
		if((*in&0xc0)!=0x80)return 0;
		*out|=(unsigned int)(*in&0x3f);
		if(*out<0x20000)return 0;
		return 5;
	}
	else if((*in&0xfe)==0xfc)
	{
		if(len<6)return 0;
		*out=((unsigned int)(*in++&0x01))<<30;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<24;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<18;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<12;
		if((*in&0xc0)!=0x80)return 0;
		*out|=((unsigned int)(*in++&0x3f))<<6;
		if((*in&0xc0)!=0x80)return 0;
		*out|=(unsigned int)(*in&0x3f);
		if(*out<0x400000)return 0;
		return 6;
	}
	else return 0;
}
