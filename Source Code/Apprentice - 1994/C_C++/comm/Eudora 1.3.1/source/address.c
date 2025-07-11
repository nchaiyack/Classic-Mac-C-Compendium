#define FILE_NUM 1
#ifndef NeXT
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Boxes
#define PState(cc,rc,ss)
#else
typedef unsigned char **Handle,*UPtr,Byte;
typedef Handle UHandle;
typedef Byte Str255[256];
#define nil ((void*)0)
#define MEM_ERR 0
#define ADDR_TOO_LONG 0
#define BAD_ADDRESS 0
#define WarnUser(a,b) puts(#a #b)
#define Boolean int
#define True 1
#define False 0
#define MemError() 0
#include "address.h"
#include <stdlib.h>
#include <stdio.h>
Handle NuHandle(long size)
{
	UPtr data=malloc(size);
	Handle hand=malloc(2*sizeof(UPtr));
	*hand = data;
	hand[1] = (UPtr)size;
	return(hand);
}
void SetHandleBig(Handle hand,int size)
{
	*hand = realloc(*hand,size);
	hand[1] = (UPtr)size;
}
void DisposHandle(Handle hand)
{
	free(*hand);
	free(hand);
}
int PtrAndHand(UPtr ptr,UHandle hand,int size)
{
	int oldSize = (int)hand[1];
	SetHandleBig(hand,oldSize+size);
	bcopy(ptr,*hand+oldSize,size);
	return(0);
}
int verbose;
#define LDRef(h) (*h)
#define UL(h)
#define IsWhite(c) (c==' ' || c=='\t')
char *stateNames[]={"sNoChange","sPlain","sParen","sAngle","sBrak","sQuot",
"sTrail","sTDone","sError","sToken","sBrakE","sQuotE","incPar","decPar"};
char *classNames[]={"Regular","Comma","lParen","rParen","lAngle","rAngle",
"lBrak","rBrak","dQuote","aDone"};
#define PState(cc,rc,ss) do {*ap = 0; if (verbose && cc!=aDone) printf("%c: %8s %-10s {%s}\n",rc,classNames[cc],stateNames[1+ss],addressBuffer+1);} while(0)
#endif
typedef enum
{
	Regular, Comma, lParen, rParen, lAngle, rAngle,
	lBrak, rBrak, dQuote, aDone
} AddressCharEnum;
typedef enum
{
	sNoChange= -1,/* remain in current state */
	sPlain, 			/* nothing special going on */
	sParen, 			/* within parentheses */
	sAngle, 			/* within angle brackets */
	sBrak,				/* within square brackets */
	sQuot,				/* with double quotes */
	sTrail, 			/* trailer after an angle bracket */
	sTDone, 			/* the token is done for this address */
/* the following are states that decay immediately into other states */
	sError, 			/* bad address; causes routine to punt */
	sToken, 			/* process completed token (sPlain) */
	sBrakE, 			/* close [] (pushed state (sPlain,sAngle,sTDone)) */
	sQuotE, 			/* close "" (pushed state (sPlain,sAngle,sTDone)) */
	incPar, 			/* increment () level (sParen) */
	decPar, 			/* decrement () level (sParen or pushed state) */
	sTrailB 			/* beginning of trailer state */
} AddressStateEnum;
char AddrStateTable[sTDone+1][aDone+1] = {
/*							. 			, 			( 			) 			< 			> 			[ 			] 			"       aDone*/
/* sPlain */		-1, 		sToken, incPar, sError, sAngle, sError, sBrak,	sError, sQuot,	sToken,
/* sParen */		-1, 		-1, 		incPar, decPar, -1, 		-1, 		-1, 		-1, 		-1, 		sError,
/* sAngle */		-1, 		-1, 		incPar, decPar, sError, sTrailB, sBrak, sError, sQuot,	sError,
/* sBrak */ 		-1, 		-1, 		-1, 		-1, 		-1, 		-1, 		sError, sBrakE, -1, 		sError,
/* sQuot */ 		-1, 		-1, 		-1, 		-1, 		-1, 		-1, 		-1, 		-1, 		sQuotE, sError,
/* sTrail */		-1, 		sToken, incPar, decPar, sError, sError, -1, 		-1, 		-1, 		sToken,
/* sTDone */		-1, 		sToken, incPar, sError, sAngle, sError, sBrak,	sError, sQuot,	sToken
};
/************************************************************************
 * SuckAddresses - parse the RFC 822 Address format
 * Returns a handle to some storage of the form:
 *			[<length byte>address<nil>]...<nil> 
 ************************************************************************/
UHandle  SuckAddresses(UHandle	text,long size,Boolean wantComments)
{
	UHandle  res;
	res = SuckPtrAddresses(LDRef(text),size,wantComments);
	UL(text);
	return(res);
}

UHandle  SuckPtrAddresses(UPtr text,long size,Boolean wantComments)
{
	UHandle  newAddresses;
	UPtr spot;
	short paren=0;
	short count=0;
	char oldState, state, nextState;
	Byte c, cClass;
	Str255 addressBuffer;
	UPtr ap;
#define AddrFull				(ap-addressBuffer >= sizeof(addressBuffer)-2)
#define AddrEmpty 			(ap==addressBuffer+1)
#define AddrChar(c) 		do {if ((wantComments||!IsWhite(c)) && oldState!=sTDone) *ap++ = c;} while (0)
#define CmmntChar(c)		do {if (wantComments && oldState!=sTDone) *ap++ = c;} while (0)
#define AddrAny(c)			do {if (oldState!=sTDone) *ap++ = c;} while (0)
#define RestartAddr() 	ap=addressBuffer+1
	newAddresses = NuHandle(0L);
	if (newAddresses==nil) {WarnUser(MEM_ERR,MemError()); return(nil);}
	
	oldState = state = sPlain;
	for (spot=text;(spot<text+size) && (IsWhite(*spot));spot++);
	RestartAddr();
	do
	{
		if (spot >= text + size)
			cClass = aDone;
		else
		{
			switch (c = *spot++)
			{
				case ',': 			cClass = Comma; break;
				case '(': 			cClass = lParen; break;
				case ')': 			cClass = rParen; break;
				case '[': 			cClass = lBrak; break;
				case ']': 			cClass = rBrak; break;
				case '<': 			cClass = lAngle; break;
				case '>': 			cClass = rAngle; break;
				case '"':       cClass = dQuote; break;
				default:				cClass = Regular; break;
			}
		}
		nextState = AddrStateTable[state][cClass];
		PState(cClass,spot[-1],nextState);
		if (nextState == sNoChange) nextState = state;
		switch (nextState)
		{
			case sPlain:
				AddrChar(c);
				break;
			case sAngle:
				if (state==sAngle || wantComments)
					AddrChar(c);
				else
					RestartAddr();
				break;
			case sBrak:
				if (state!=sBrak)
					oldState = state;
				AddrAny(c);
				break;
			case sQuot:
				if (state!=sQuot) oldState = state;
				AddrAny(c);
				break;
			case sTDone:
				break;
			case sError:
				count = 0;
				goto parsePunt;
				break;
			case sToken:
				while (!AddrEmpty && IsWhite(ap[-1])) ap--; /* strip trailing space */
				*ap++ = '\0';
				if (addressBuffer[1])
				{
					*addressBuffer = ap-addressBuffer - 2;
					if (PtrAndHand(addressBuffer,newAddresses,*addressBuffer+2))
					{
						int err = MemError();
						SetHandleBig(newAddresses,0L);
						WarnUser(MEM_ERR,err);
						count = 0;
						goto parsePunt;
					}
					else
						count++;
				}
				RestartAddr();
				nextState = oldState = sPlain;
				PState(cClass,' ',nextState);
				while ((spot<text+size) && (IsWhite(*spot))) spot++;
				break;
			case sTrailB:
				nextState = sTrail;
				CmmntChar('>');
				break;
			case sBrakE:
				nextState = oldState;
				PState(cClass,' ',nextState);
				AddrChar(c);
				break;
			case sQuotE:
				nextState = oldState;
				PState(cClass,' ',nextState);
				AddrChar(c);
				break;
			case incPar:
				if (!paren++) oldState=state;
				nextState = sParen;
				PState(cClass,' ',nextState);
				CmmntChar('(');
				break;
			case decPar:
				if (!--paren)
					nextState = oldState;
				else
					nextState = sParen;
				PState(cClass,' ',nextState);
				CmmntChar(')');
				break;
			default:
				CmmntChar(c);
				break;
		}
		state = nextState;
	}
	while (cClass!=aDone && !AddrFull);
	if (PtrAndHand("",newAddresses,1))
	{
		int err = MemError();
		SetHandleBig(newAddresses,0L);
		WarnUser(MEM_ERR,err);
	}
parsePunt:	
	if ((nextState==sError)||AddrFull)
	{
		DisposHandle(newAddresses);
		if (AddrFull) WarnUser(ADDR_TOO_LONG,0);
		else if (nextState==sError) WarnUser(BAD_ADDRESS,0);
		return(nil);
	}
	else
		return(newAddresses);
}

#ifdef NeXT
main(int argc,unsigned char **argv)
{
	Str255 addr;
	if (argc>1) verbose=1;
	while(gets((char *)addr))
	{
		UHandle  h;
		UPtr p;
		
		printf("Address: {%s}",addr);
		putchar(verbose ? '\n' : ' ');
		h = SuckPtrAddresses(addr,strlen(addr),False);
		if (!h) fputs("{nil}",stdout);
		else
		{
			if (!**h) fputs("{empty}",stdout);
			else
				for (p=*h;*p;p+=*p+2)
					printf("%s{%s}",p==*h?"":", ",p+1);
			DisposHandle(h);
		}
		putchar(verbose ? '\n' : ' ');
		h = SuckPtrAddresses(addr,strlen(addr),True);
		if (!h) fputs("{nil}",stdout);
		else
		{
			if (!**h) fputs("{empty}",stdout);
			else
				for (p=*h;*p;p+=*p+2)
					printf("%s{%s}",p==*h?"":", ",p+1);
			DisposHandle(h);
		}
		putchar('\n');
	}
}
#endif
