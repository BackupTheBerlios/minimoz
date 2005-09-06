/*
 * (c) 2004 Andreas Steinmetz, ast@domdv.de
 */

int checkutf8str(const unsigned char *s,int *line)
{
	while(*s)
	{
		if(!(*s&0x80))
		{
			if(*s=='\n'&&line)(*line)++;
			s++;
		}
		else if((*s&0xe0)==0xc0)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			s+=2;
		}
		else if((*s&0xf0)==0xe0)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			s+=3;
		}
		else if((*s&0xf8)==0xf0)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			if((s[3]&0xc0)!=0x80)return -1;
			s+=4;
		}
		else if((*s&0xfc)==0xf8)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			if((s[3]&0xc0)!=0x80)return -1;
			if((s[4]&0xc0)!=0x80)return -1;
			s+=5;
		}
		else if((*s&0xfe)==0xfc)
		{
			if((s[1]&0xc0)!=0x80)return -1;
			if((s[2]&0xc0)!=0x80)return -1;
			if((s[3]&0xc0)!=0x80)return -1;
			if((s[4]&0xc0)!=0x80)return -1;
			if((s[5]&0xc0)!=0x80)return -1;
			s+=6;
		}
		else return -1;
	}
	return 0;
}
