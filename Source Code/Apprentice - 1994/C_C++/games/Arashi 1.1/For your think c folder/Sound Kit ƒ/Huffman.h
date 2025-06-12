/*/
     Project Arashi: Huffman.h
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 22:12
     Created: Saturday, October 6, 1990, 23:11

     Copyright � 1990-1992, Juri Munkki
/*/

/*
>>	This file contains definitions for compression routines for
>>	the sound data of Project STORM.
*/
#define	SOUNDFILE	'FSSD'

#define	QTBITS		10		/*	Should always be less than 16!	*/
#define	VALUES		128
#define	DROPBITS	1
#define	ANDMASK		0x7F

typedef	struct	treenode
{
	char				value;
	
	int					codelen;
	int					code;

	long				freq;

	struct	treenode	*zeroptr;
	struct	treenode	*oneptr;
	int					typeflag;

}	treenode;
