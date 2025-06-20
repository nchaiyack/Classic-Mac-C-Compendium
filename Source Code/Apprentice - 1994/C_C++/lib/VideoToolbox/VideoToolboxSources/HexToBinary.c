/*
HexToBinary.c
Two routines that translate back and forth between a binary object (some number
of bytes at a specified address) and a string of hexadecimal digits. Two hex
digits represent one binary byte.

PORTABILITY: Standard C.

HISTORY:
5/27/93 dgp wrote it, partly based on code I'd previously written for 
			PixMapToPostScript.c 
6/15/93	dgp minor editing
*/
#include "VideoToolbox.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int HexToBinary(char *string,void *ptr)
/* Fast translation of a hex string of arbitrary length. */
{
	register unsigned char *byte,c,dehex[256];
	register unsigned long i;
	size_t digits;

	for(i=0;i<sizeof(dehex);i++)dehex[i]=255;
	for(i=0;i<10;i++)dehex[i+'0']=i;
	for(i=0;i<6;i++)dehex[i+'a']=dehex[i+'A']=i+10;
	digits=strlen(string);
	byte=(unsigned char *)string;
	for(i=0;i<digits;i++)if(dehex[*byte++]==255)return 1;	/*  Illegal character */
	byte=(unsigned char *)ptr;
	for(i=digits/2;i>0;i--){
		c=dehex[*string++];
		c<<=4;
		c+=dehex[*string++];
		*byte++ =c;
	}
	return 0;	/*  Success */
}

char *BinaryToHex(size_t n,void *ptr,char *string)
/*  Fast encoding of a specified number of bytes as a hex string. */
/*  "string", if not NULL, must be able to hold at least 2*n+1 bytes. */
{
	register long i,j;
	register unsigned short *word,hex[256];
	register unsigned char *byte;
	static const unsigned char c[]="0123456789abcdef";
	long digits;
	short oddAddress;

	assert(sizeof(*byte)==1);	/*  required by our algorithm */
	assert(sizeof(*word)==2);	/*  required by our algorithm */
	for(j=0;j<16;j++)for(i=0;i<16;i++)hex[(j<<4)+i]=(c[j]<<8)+c[i];
	digits=2*n;
	if(string==NULL){
		string=(char *)malloc(digits+1);
		if(string==NULL)return NULL;
	}
	oddAddress=(unsigned long)string%2;
	if(oddAddress)string++;
	word=(unsigned short *)string;
	byte=(unsigned char *)ptr;
	for(i=n;i>0;i--)*word++ = hex[*byte++];
	if(oddAddress){
		/* move back */
		string--;
		memmove(string,string+1,digits);
	}
	string[digits]=0;
	return string;
}
