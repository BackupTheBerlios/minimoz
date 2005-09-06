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

int ucs4_to_utf8(unsigned int *in,unsigned char *out)
{
	if(*in<0x80)
	{
		*out=(unsigned char)(*in);
		return 1;
	}
	else if(*in<0x800)
	{
		*out++=(*in>>6)|0xc0;
		*out=(*in&0x3f)|0x80;
		return 2;
	}
	else if(*in<0x10000)
	{
		*out++=(*in>>12)|0xe0;
		*out++=((*in>>6)&0x3f)|0x80;
		*out=(*in&0x3f)|0x80;
		return 3;
	}
	else if(*in<0x200000)
	{
		*out++=(*in>>18)|0xf0;
		*out++=((*in>>12)&0x3f)|0x80;
		*out++=((*in>>6)&0x3f)|0x80;
		*out=(*in&0x3f)|0x80;
		return 4;
	}
	else if(*in<0x4000000)
	{
		*out++=(*in>>24)|0xf8;
		*out++=((*in>>18)&0x3f)|0x80;
		*out++=((*in>>12)&0x3f)|0x80;
		*out++=((*in>>6)&0x3f)|0x80;
		*out=(*in&0x3f)|0x80;
		return 5;
	}
	else if(*in<0x80000000)
	{
		*out++=(*in>>30)|0xfc;
		*out++=((*in>>24)&0x3f)|0x80;
		*out++=((*in>>18)&0x3f)|0x80;
		*out++=((*in>>12)&0x3f)|0x80;
		*out++=((*in>>6)&0x3f)|0x80;
		*out=(*in&0x3f)|0x80;
		return 6;
	}
	else return 0;
}
