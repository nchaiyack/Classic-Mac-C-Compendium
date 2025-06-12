/*
>>	Dizzy 0.0	MacStuff.h
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	You are entitled to do whatever you wish with this program, as long as
>>	you give credit all those people, who have worked on this program. If
>>	you are going to sell this program or obtain commercial benefit from it
>>	otherwise, please consult the authors first to obtain information about
>>	licensing this program for commercial use.
*/

#ifndef _MacStuff_
#define _MacStuff_

typedef char	*Ptr;
typedef char	**Handle;

typedef struct 
{
	int 	h,v;
}	Point;
	
typedef struct
{
	int 	left,top,right,bottom;
}	Rect;

GLOBAL	int 		macpen_x,macpen_y;
GLOBAL	int 		orig_x;
GLOBAL	int 		orig_y;
GLOBAL	Rect		mac_clipper;
GLOBAL	Rect		portrect;

#define NonEmpty(r) 	(((r)->left<(r)->right)?((r)->top<(r)->bottom):FALSE)
#define EmptyRect(r)	(((r)->left>=(r)->right)?TRUE:((r)->top>=(r)->bottom))
#define MoveTo(x,y) 	{macpen_x=x;macpen_y=y;}
#define Delay(a,b)
#define HILITEMODE
#define MenuFlash		2
#endif _MacStuff_
