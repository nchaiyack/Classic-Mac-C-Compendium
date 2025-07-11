/* PortableFile.c */

#include "PortableFile.h"
#include "File.h"
#include "Memory.h"


void					WChar(short RefNum, char Data)
	{
		FWriteBlock(RefNum,&Data,1);
	}


void					WShort(short RefNum, short Data)
	{
		char				Buffer[2];

		Buffer[0] = Data & 0xff;
		Buffer[1] = (Data >> 8) & 0xff;
		FWriteBlock(RefNum,Buffer,2);
	}


void					WLong(short RefNum, long Data)
	{
		char				Buffer[4];

		Buffer[0] = Data & 0xff;
		Buffer[1] = (Data >> 8) & 0xff;
		Buffer[2] = (Data >> 16) & 0xff;
		Buffer[3] = (Data >> 24) & 0xff;
		FWriteBlock(RefNum,Buffer,4);
	}


char					RChar(short RefNum)
	{
		char			Buffer[1];

		FReadBlock(RefNum,Buffer,1);
		return Buffer[0];
	}


short					RShort(short RefNum)
	{
		char			Buffer[2];

		FReadBlock(RefNum,Buffer,2);
		return (uchar)Buffer[0] | ((short)(uchar)Buffer[1] << 8);
	}


long					RLong(short RefNum)
	{
		char			Buffer[4];

		FReadBlock(RefNum,Buffer,4);
		return (uchar)Buffer[0] | ((long)(uchar)Buffer[1] << 8)
			| ((long)(uchar)Buffer[2] << 16) | ((long)(uchar)Buffer[3] << 24);
	}
