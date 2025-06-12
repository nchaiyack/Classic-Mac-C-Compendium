/*
>>	Dizzy 1.0	SimDisplay.c
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
>>
>>	This file contains routines that affect the way elements are displayed.
*/

#include "dizzy.h"

#ifdef	MACINTOSH	/*	Grid line pattern for Macintosh.	*/
char	GridPattern[8]={0x22,0x00,0x88,0x00,0x22,0x00,0x88,0x00};
#endif
char	HexDigits[]="0123456789ABCDEF"; /*	Used for hex display element.	*/

/*
>>	ClipEditArea sets the clipping region/rectangle to contain the editing area.
>>	It also changes coordinate systems so that elements may be drawn with their
>>	coordinates intact even if the display has been scrolled.
*/
void	ClipEditArea()
{
#ifdef	MACINTOSH
	SetPort(MyWind);
#endif

	EditClipper=EditR;
	OffsetRect(&EditClipper,-EditR.left+CurHeader->XOrig,-EditR.top+CurHeader->YOrig);

	SetOrigin(-EditR.left+CurHeader->XOrig,-EditR.top+CurHeader->YOrig);
	ClipRect(&EditClipper);
}
/*
>>	This routine restores default clipping and coordinate space.
>>	Note that the implementation of these two routines does not
>>	allow for re-entrancy. This would have complicated the design
>>	of these routines unnecessarily, since re-entrancy was never
>>	needed.
*/
void	RestoreClipping()
{
	SetOrigin(0,0);
	ClipRect(&PortRect);
}
/*
>>	Given a pointer to an input array, an element pointer,
>>	the index of the wanted input and pointers to two
>>	coordinate pairs, this routine stores line endpoints
>>	in (x1,y1),(x2,y2). This useful routine is used in many
>>	other places in Dizzy.
*/
int 	GetConnectLine(ip,elem,index,x1,y1,x2,y2)
Input	*ip;
Element *elem;
int 	index;
int 	*x1,*y1,*x2,*y2;
{
	Element 	*outelem;

	if(ip->Chip)
	{	switch(elem->Type)
		{	case CONN:
				*x1=elem->Body.left+4;
				*y1=elem->Body.top+4;
				break;
			default:
				*x1=elem->InRect.left;
				*y1=elem->InRect.top+8+16*index;
				break;
		}

		outelem=(Element *)(SimBase+ip->Chip);
		switch(outelem->Type)
		{	case CONN:
				*x2=outelem->Body.left+4;
				*y2=outelem->Body.top+4;
				break;
			default:
				*x2=outelem->OutRect.left+8;
				*y2=outelem->OutRect.top+8+16*ip->Pin;
				break;
		}
		return -1;
	}
	else
	{	return 0;
	}
}
/*
>>	ConnectLine is a simple extension of the previous
>>	routine. Instead of returning the coordinates, this
>>	routine just draws the line and returns.
*/
void	ConnectLine(ip,elem,index)
Input	*ip;
Element *elem;
int 	index;
{
	int 	x1,y1,x2,y2;
	
	if(GetConnectLine(ip,elem,index,&x1,&y1,&x2,&y2))
	{	MoveTo(x1,y1);
		LineTo(x2,y2);
	}
}

/*
>>	Input and output rectangles contain pictures of input
>>	and output pins. The pins could be drawn with a simple
>>	pattern (even on the Mac), but I decided to use real
>>	lines.
*/
void	DrawLinesRect(r)
Rect	*r;
{
	int 	i;
	
	for(i=r->top+8;i<r->bottom;i+=16)
	{	MoveTo(r->left,i);
		LineTo(r->left+8,i);
	}
}

/*
>>	CopyInsides is an extension of CopyBits and CopyPix
>>	used to copy from the misc bitmap (ButtonBits on the
>>	Macintosh) to the screen. ButtonBits contains bitmaps
>>	for AND, OR, XOR, JK, D and RS elements.
*/
void	CopyInsides(y1,y2,r)
int 	y1,y2;
Rect	*r;
{
	Rect	src,dest;
	
	dest= *r;
	dest.top=dest.top+((dest.bottom-dest.top)-(y2-y1))/2;
	dest.bottom=dest.top+y2-y1;
	dest.left+=1;
	dest.right=dest.left+27;
	src.left=0;
	src.right=27;
	src.top=y1;
	src.bottom=y2;
#ifdef	MACINTOSH
	CopyBits(&ButtonBits,&thePort->portBits,&src,&dest,srcCopy,0L);
#else
	MiscPixCopy(src.left,src.top,&dest);
#endif

}
/*
>>	UpdateSim draws the top level of the simulation. It draws all
>>	the elements even if they are outside the visible area. You
>>	might want to optimize this routine, if you have a slow system
>>	or you want to edit really large circuits.
*/
void	UpdateSim()
{

	Element 	*elem,*other;
	long		offs;
	int 		i,j;
	Input		*ip;
	Rect		Inset;

#ifdef	MACINTOSH
	TextFont(geneva);
	TextSize(9);
	PenPat(GridPattern);
	EraseRect(&EditClipper);

	/*	Draw background grid on the Macintosh.	*/
	for(i=EditClipper.left;i<EditClipper.right+63;i+=64)
	{	j=i & ~63;
		MoveTo(j,EditClipper.top);
		LineTo(j,EditClipper.bottom);
	}
	for(i=EditClipper.top;i<EditClipper.bottom+63;i+=64)
	{	j=i & ~63;
		MoveTo(EditClipper.left,j);
		LineTo(EditClipper.right,j);
	}
#else
	/*	Erase background with grid pattern on X */
	PenGrid();
	PaintRect(&EditClipper);
#endif

	PenBlack();

	/*	Initialize RefCount to 0. The RefCount is used to determine if
	**	an output should get the dot indicating a junction.
	*/
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		for(i=0;i<elem->Outputs;i++)
		{	elem->Out[i].RefCount=0;	
		}
	}
	
	/*	Find out RefCount values and draw connecting lines. */
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		ip=(Input *)(&elem->Out[elem->Outputs]);
		for(i=0;i<elem->Inputs;i++)
		{	ConnectLine(ip,elem,i);
			if(ip->Chip)
			{	other=(Element *)(ip->Chip+SimBase);
				other->Out[ip->Pin].RefCount++;
			}
			ip++;
		}
	}

	/*	Draw the elements.	*/
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		if(elem->Type!=CONN)
		{	FrameRect(&elem->Body);
			Inset=elem->Body;
			InsetRect(&Inset,1,1);
			EraseRect(&Inset);

			switch(elem->Type)
			{	case AND_:	/*	Note the yuch-yuch hard-coded constant coords	*/
				case NAND:	/*	This is the only place where they are used! 	*/
					CopyInsides(6,24,&elem->Body);
					break;
				case OR__:
				case NOR_:
					CopyInsides(24,40,&elem->Body);
					break;
				case XOR_:
				case NXOR:
					CopyInsides(40,56,&elem->Body);
					break;
				case D___:
					CopyInsides(56,84,&elem->Body);
					break;
				case RS__:
					CopyInsides(88,116,&elem->Body);
					break;
				case JK__:
					CopyInsides(116,156,&elem->Body);
					break;
				case ZERO:
					{	Rect	framer;
					
						framer=elem->Body;
						InsetRect(&framer,1,1);
						FrameRect(&framer);
						MoveTo(elem->Body.left+6,elem->Body.bottom-4);
						DrawChar('0');
					}
					break;
				case ONE_:
					{	Rect	framer;
					
						framer=elem->Body;
						InsetRect(&framer,1,1);
						FrameRect(&framer);
						MoveTo(elem->Body.left+6,elem->Body.bottom-4);
						DrawChar('1');
					}
					break;
				case INPT:
					{	char	nm[2];
					
						MoveTo(elem->Body.left+6,elem->Body.bottom-4);
						DrawChar('0'+(elem->Out[0].Data&1));
						MoveTo(elem->Body.left+3,elem->Body.bottom+9);
						nm[1]='0'+ elem->PrivData % 10;
						nm[0]='0'+ (elem->PrivData/10) % 10;
						if(nm[0]=='0') nm[0]=' ';
						DrawText(nm,0,2);
					}
					break;
				case OUTP:
					{	char	nm[2];
					
						MoveTo(elem->Body.left+6,elem->Body.bottom-4);
						DrawChar(elem->Flags & DISPLAYEDVALUE ? '1' : '0');
						MoveTo(elem->Body.left+3,elem->Body.bottom+9);
						nm[1]='0'+ elem->PrivData % 10;
						nm[0]='0'+ (elem->PrivData/10) % 10;
						if(nm[0]=='0') nm[0]=' ';
						DrawText(nm,0,2);
					}
					break;
				case HEXD:
					MoveTo(elem->Body.left+6,elem->Body.bottom-28);
					DrawChar(HexDigits[elem->PrivData]);
					break;
				case CLOK:
					MoveTo(elem->Body.left+2,elem->Body.bottom-3);
					Line(6,0);
					Line(0,-10);
					Line(5,0);
					MoveTo(elem->Body.left+2,elem->Body.bottom-4);
					DrawChar('0'+elem->PrivData);
					break;
				case CUST:
					{	TableHeader 	*customhead;
						Ptr 			pathstring;
					
						customhead=(TableHeader *)(elem->Inputs+(Input *)&elem->Out[elem->Outputs]);
						pathstring=(Ptr)(customhead+1);
						MoveTo(elem->Body.left+(elem->Body.right
												-elem->Body.left
												-customhead->TitleWidth)/2,
								elem->Body.top+(elem->Body.bottom-elem->Body.top)/2+5);
						DrawText(pathstring,
									customhead->PathLen-customhead->TitleLen,
									customhead->TitleLen-1);
					}
					break;

			}
			DrawLinesRect(&elem->InRect);
			if(elem->Flags & INVERTED)	/*	Draw one inverted output pin?	*/
			{	Rect	round;
			
				SetRect(&round, elem->OutRect.left-1,
								elem->OutRect.top+6,
								elem->OutRect.left+4,
								elem->OutRect.top+11);
#ifdef MACINTOSH	/*	The mac can draw ovals really quickly!				*/
				FrameOval(&round);
#else				/*	X is not quite as good, so use a pixmap.			*/
				round.left++;
				MiscPixCopy(23,0,&round);
#endif
				MoveTo(elem->OutRect.left+4,elem->OutRect.top+8);
				LineTo(elem->OutRect.left+8,elem->OutRect.top+8);
			}
			else
			{	DrawLinesRect(&elem->OutRect);	/*	Standard output pins.	*/
			}

			for(i=0;i<elem->Outputs;i++)		/*	Draw dots where necessary.	*/
			{	if(elem->Out[i].RefCount>1)
				{	Rect	dot;
					
					dot.left=elem->OutRect.left+6;
					dot.right=dot.left+5;
					dot.top=elem->OutRect.top+i*16+6;
					dot.bottom=dot.top+5;
					PaintRect(&dot);
				}
			}
		}
		else	/*	Draw a connector, if necessary. 	*/
		{	Rect	dot;
		
			dot=elem->Body;
			InsetRect(&dot,2,2);
			if(elem->Out[0].RefCount != 1)
				PaintRect(&dot);
			else if(ConnectorFrames)
				FrameRect(&dot);
		}
	}
}
