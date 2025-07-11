/**********************************************************************\

File:		SM2 VBL.c

Purpose:	This module handles setting low-level globals that make
			the mouse move twice as fast as normal (via 'mcky' resources).
			

Speedy Mouse ][ -=- all the mouse, twice the speed, none of the hassle
Copyright �1994, Mark Pilgrim

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "Retrace.h"

extern	unsigned int	keymap[8] : 0x174;
extern	unsigned char	RawThreshhold : 0x208;
extern	unsigned long	SysHeapStart : 0x2a6;

unsigned long	me;						/* tag so we only init globals once */
unsigned long	gMyMckyData[2];			/* our data for current threshhold */
unsigned long*	gOriginalMckyData;		/* pointer to original mcky data array */
unsigned char	gCurrentThreshhold;		/* current threshhold (0-6) */
unsigned long	gMckyDataAddress;		/* address of mcky data in system heap */
Boolean			gIsSpeedy;				/* currently using our speedy mcky data? */

void header(void);
void main(void);
void FindCorrectMckyDataAddress(Boolean findOriginalData);
void OriginalToSpeedy(void);
void SpeedyToOriginal(void);

void header(void)
{
	asm {
		dc.l	0
		move.l a0, d0
		lea header, a0
		jmp main
	}
}

#include "SetUpA4.h"

void main(void)
{
	VBLTask*		myVBL;
	unsigned char	realThreshhold;
	
	RememberA0();
	SetUpA4();
	
	asm
	{
		move.l d0, myVBL
	}
	gOriginalMckyData=(long*)((long)myVBL-56);	/* initted at INIT time */
	realThreshhold=((RawThreshhold>>3)&0x07);
	
	if (me != '�MSG')						/* need to initialize our globals */
	{
		me = '�MSG';						/* only once */
		gCurrentThreshhold=-1;				/* we'll get the real one below */
		gIsSpeedy=FALSE;					/* we'll fix this below if needed */
		gMckyDataAddress=SysHeapStart;		/* we'll search for the real one below */
	}
	
	if (gCurrentThreshhold!=realThreshhold)	/* changed, we need to update */
	{
		gCurrentThreshhold=realThreshhold;	/* save threshhold value */
		FindCorrectMckyDataAddress(TRUE);	/* find address of mcky data in system heap */
		if (gIsSpeedy)						/* if speedy, change mcky data in sys heap */
			OriginalToSpeedy();
	}
	
	if ((!gIsSpeedy) && (!(keymap[3]&2)))	/* change to speedy mode */
	{
		gIsSpeedy=TRUE;						/* so we know we're in speedy mode */
		FindCorrectMckyDataAddress(TRUE);	/* find address of mcky data in system heap */
		OriginalToSpeedy();					/* change mcky data in system heap */
	}
	else if ((gIsSpeedy) && (keymap[3]&2))	/* change to normal mode */
	{
		gIsSpeedy=FALSE;					/* so we know we're in normal mode */
		FindCorrectMckyDataAddress(FALSE);	/* find address of mcky data in system heap */
		SpeedyToOriginal();					/* change mcky data in system heap */
	}
	
	myVBL->vblCount = 30;					/* check it again in half a second */
	RestoreA4();
}

void FindCorrectMckyDataAddress(Boolean findOriginalData)
{
	unsigned long	target[2];
	
	if (!findOriginalData)				/* Q: what mcky data are we searching for? */
	{
		target[0]=gMyMckyData[0];								/* A: ours */
		target[1]=gMyMckyData[1];
	}
	else
	{
		target[0]=gOriginalMckyData[gCurrentThreshhold*2];		/* A: theirs */
		target[1]=gOriginalMckyData[gCurrentThreshhold*2+1];
	}
	
	/* search in memory for correct mcky data */
	if ((*((unsigned long*)gMckyDataAddress)!=target[0]) ||
		(*((unsigned long*)(gMckyDataAddress+4))!=target[1]))
	{
		gMckyDataAddress=SysHeapStart;	/* must be in system heap somewhere */
		while ((*((unsigned long*)gMckyDataAddress)!=target[0]) ||
			(*((unsigned long*)(gMckyDataAddress+4))!=target[1]))
			gMckyDataAddress+=4;
	}
}

void OriginalToSpeedy(void)
{
	unsigned char	temp;
	int				i;
	
	if (gCurrentThreshhold>=0x05)	/* threshhold=5 or 6, halve data manually */
	{
		gMyMckyData[0]=gOriginalMckyData[gCurrentThreshhold*2];
		gMyMckyData[1]=gOriginalMckyData[gCurrentThreshhold*2+1];
		
		for (i=0; i<8; i++)
		{
			temp=*((unsigned char*)gMyMckyData+i);
			temp=(temp==0x01) ? 0x01 : ((temp==0xFF) ? 0xFF : (temp+1)>>1);
			*((unsigned char*)gMyMckyData+i)=temp;
		}
	}
	else	/* threshhold less than 5, use data of threshhold+2 */
	{
		gMyMckyData[0]=gOriginalMckyData[gCurrentThreshhold*2+4];
		gMyMckyData[1]=gOriginalMckyData[gCurrentThreshhold*2+5];
	}
	
	*((unsigned long*)gMckyDataAddress)=gMyMckyData[0];		/* set in memory */
	*((unsigned long*)(gMckyDataAddress+4))=gMyMckyData[1];
}

void SpeedyToOriginal(void)
{
	/* look up real data in ready-made table (made at INIT time) */
	*((unsigned long*)gMckyDataAddress)=gOriginalMckyData[gCurrentThreshhold*2];
	*((unsigned long*)(gMckyDataAddress+4))=gOriginalMckyData[gCurrentThreshhold*2+1];
}
