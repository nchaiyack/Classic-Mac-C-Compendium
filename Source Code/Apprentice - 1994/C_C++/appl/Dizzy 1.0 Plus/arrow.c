/*
>>	Dizzy 1.0	Arrow.c
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
>>
>>	This program contains that code that defines how the arrow tool acts
>>	when the user clicks in the edit window.
*/

#include "dizzy.h"

Element *LastElem;		/*	Last element in top level			*/
int 	ElemCount;		/*	Number of elements in top level.	*/

/*
>>	InvalLine invalides an area of the screen. That is, it forces
>>	a redraw of the general area where a line was located.
*/
void	InvalLine(x0,y0,x1,y1)
int 	x0,y0,x1,y1;
{
	Rect	inval;

	if(x0<x1)
	{	inval.left = x0-8;
		inval.right = x1+8;
	}
	else
	{	inval.left = x1-8;
		inval.right = x0+8;
	}
	if(y0<y1)
	{	inval.top = y0-8;
		inval.bottom = y1+8;
	}
	else
	{	inval.top = y1-8;
		inval.bottom = y0+8;
	}
	InvalRect(&inval);
}

/*
>>	While working on the X version of the program, I noticed that
>>	in order to be fast enough, the program couldn't update the
>>	whole display in cases when less was sufficient. I didn't
>>	want to start creating regions for lines that needed to be
>>	updated, so I decided to define a bounding box that would
>>	be scrapped. TrashRect is this bounding box and the following
>>	routines are used to manipulate it.
*/
static	Rect	TrashRect;

void	SetTrashRect(r)
Rect	*r;
{
	TrashRect= *r;
}

void	ExpandTrash(x,y)
int 	x,y;
{
	if(x<TrashRect.left)		TrashRect.left=x;
	else if(x>TrashRect.right)	TrashRect.right=x;

	if(y<TrashRect.top) 		TrashRect.top=y;
	else if(y>TrashRect.bottom) TrashRect.bottom=y; 
}

void	InvalTrash()
{
	InsetRect(&TrashRect,-10,-10);
	InvalRect(&TrashRect);
}
/*
>>	When an output is clicked, a new wire is drawn. This routine
>>	handles the drawing and updating of a rubberband wire and
>>	also creates connections, when necessary.
*/
void	RubberBandOutputWire(elem,sourcepin)
Element *elem;
int 	sourcepin;
{
	int 		x0,y0;
	int 		x1,y1;
	int 		pin,i,downflag;
	Input		*ip;
	Point		MousePoint,OldSpot;
	Element 	*other,*found;
	int 		legalconnection;
	Rect		invalidator;
	Rect		*inarea;
	Rect		PinLite;
	
	PenXor();
	
	/*	Find out where rubberband is anchored:	*/
	if(elem->Type==CONN)
	{	x0 = elem->Body.left+4;
		y0 = elem->Body.top+4;
	}
	else
	{	x0 = elem->OutRect.left+8;
		y0 = elem->OutRect.top+8+16*sourcepin;
	}
	
	/*	Start with a line stub. 				*/
	x1 = x0;
	y1 = y0;
	OldSpot.h = x0;
	OldSpot.v = y0;
	MoveTo(x0,y0);
	LineTo(x0,y0);
	PinLite = NilRect;	/*	No input pins are hilited.			*/
	
	found = 0;			/*	No input was found. 				*/

	do	/*	Loop until button is released.						*/
	{	downflag = GetMouseTrackEvent(&MousePoint);
		
		/*	Do some rounding and gridding.						*/
		MousePoint.h += 4;
		MousePoint.v += 4;
		MousePoint.h &= ~7;
		MousePoint.v &= ~7;
		
		/*	Did mouse move? 									*/
		if(MousePoint.h!=OldSpot.h || MousePoint.v !=OldSpot.v)
		{	/*	It is possible to find a connection.			*/
			legalconnection = -1;

			/*	Undraw old line and unhilite selected pin.		*/
			MoveTo(x0,y0);
			LineTo(x1,y1);
			InvertRect(&PinLite);

			found = 0;	/*	Nothing found, yet. 				*/
			
			/*	Is point inside edit area?						*/
			if(PtInRect(MousePoint,&EditClipper))
			{	i = ElemCount;	/*	Loop through elements to	*/
				other = LastElem;	/* find an input pin.		*/
				while(!found && i-- > 0)
				{	/*	Connector points have empty InRects.	*/
					inarea = other->Type==CONN ? &other->Body : &other->InRect;
					
					if(PtInRect(MousePoint,inarea))
					{	pin = other->Type==CONN ? 0 : (MousePoint.v-other->InRect.top)/16;
						ip = (Input *)&other->Out[other->Outputs];
						
						if(ip[pin].Chip==0)
						{	found = other;
							if(other->Type==CONN)
							{	PinLite = other->Body;
								x1 = other->Body.left+4;
								y1 = other->Body.top+4;
							}
							else
							{	x1 = other->InRect.left;
								y1 = other->InRect.top+8+16*pin;
								SetRect(&PinLite,x1-1,y1-1,x1+8,y1+2);
							}
						}
						else
						{	i = -1;
							x1 = x0;
							y1 = y0;
							PinLite = NilRect;
							legalconnection = 0;
						}
					}
					/*	Check out next element. Step backwards. */
					other = (Element *)(((char *) other)-other->PrevLength);
				}
				
				/*	No input pin found? */
				if(!found && i==-1)
				{	x1 = MousePoint.h;
					y1 = MousePoint.v;
					PinLite = NilRect;
				}
			}
			else
			{	/*	Click was outside edit area. Back to stub line. */
				x1 = x0;
				y1 = y0;
				legalconnection = 0;
			}
			/*	Draw new line and hilite input. */
			MoveTo(x0,y0);
			LineTo(x1,y1);
			InvertRect(&PinLite);
			OldSpot = MousePoint;
		}
	}	while(downflag);
	
	MoveTo(x0,y0);	/*	Unhilite line and pin.	*/
	LineTo(x1,y1);
	InvertRect(&PinLite);

	if(!found && legalconnection && (x0!=x1 || y0!=y1))
	{	/*	Create a connector	*/
		found = SimAllocate(sizeof(Element)+sizeof(Input));
		InitConnector(found,MousePoint.h,MousePoint.v);
		CurHeader->Last = SimEnd;
		ip = (Input *)&found->Out[found->Outputs];
		pin = 0;
	}

	if(found)
	{	/*	Connect output to input.	*/
		ip[pin].Chip = (char *)elem-SimBase;
		ip[pin].Pin = sourcepin;
		InvalRect(&found->Body);
	}

	InvalLine(x0,y0,x1,y1);
	PenCopy();
}

typedef struct
{		int x1,y1;
		int x2,y2;
}	wire;

/*
>>	Move an element in the edit area rubberbanding input and output
>>	wires and moving around a rectangular outline of the chip.
*/
int 	MoveElementAround(elem,forcedraw)
Element *elem;
int 	forcedraw;
{
	wire		wires[MAX_WIRES+1],*wirep;
	int 		numwires;
	int 		i,downflag;
	Element 	*brother;
	Input		*ip;
	long		offs,ouroffset;
	Rect		OldFrame;
	Point		FirstPoint,OldPoint,MousePoint,OnScreen;
	int 		moved;
	Point		offset;
	
	PenXor();
	PenGray();

	numwires = 0;
	wirep = wires;
	ouroffset = (Ptr)elem-SimBase;

	SetTrashRect(&elem->Body);
	
	/*	Find lines connected to inputs of this chip. Easy.		*/
	ip = (Input *)&elem->Out[elem->Outputs];
	for(i=0;i<elem->Inputs;i++)
	{	if(ip->Chip != ouroffset)
		if(GetConnectLine(ip,elem,i,&wirep->x1,&wirep->y1,&wirep->x2,&wirep->y2))
		{	ExpandTrash(wirep->x2,wirep->y2);
			if(numwires<MAX_WIRES)
			{	numwires++;
				wirep++;
			}
		}
		ip++;
	}

	/*	Find lines connected to outputs of this chip. Harder.	*/
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=brother->Length)
	{	brother=(Element *)(SimBase+offs);
		ip=(Input *)&brother->Out[brother->Outputs];
		if(brother!=elem)
		for(i=0;i<brother->Inputs;i++)
		{	if(ip->Chip == ouroffset)
			{	GetConnectLine(ip,brother,i,&wirep->x2,&wirep->y2,&wirep->x1,&wirep->y1);
				ExpandTrash(wirep->x2,wirep->y2);
				if(numwires<MAX_WIRES)
				{	numwires++;
					wirep++;
				}
			}
			ip++;
		}
	}

	/*	forcedraw is true, previously undisplayed element.	*/
	if(forcedraw)
	{	OldFrame=elem->Body;
		FrameRect(&OldFrame);
		
		wirep=wires;
		for(i=0;i<numwires;i++)
		{	MoveTo(wirep->x1,wirep->y1);
			LineTo(wirep->x2,wirep->y2);
			wirep++;
		}
		moved= -1;
	}
	else
	{	OldFrame=NilRect;
		moved=0;
	}

	GetMouseDownPoint(&FirstPoint);
	FirstPoint.h-=4;
	FirstPoint.v-=4;

	OldPoint.h=0;
	OldPoint.v=0;

	do	/*	A simple rubberbanding loop. Simpler than above routine.	*/
	{	downflag=GetMouseTrackEvent(&MousePoint);
		OnScreen=MousePoint;
		MousePoint.h = (MousePoint.h - FirstPoint.h) & ~7;
		MousePoint.v = (MousePoint.v - FirstPoint.v) & ~7;
		if(MousePoint.h!=OldPoint.h || MousePoint.v!=OldPoint.v)
		{	if(moved<0)
			{	FrameRect(&OldFrame);
				wirep=wires;
				for(i=0;i<numwires;i++)
				{	MoveTo(wirep->x1+OldPoint.h,wirep->y1+OldPoint.v);
					LineTo(wirep->x2,wirep->y2);
					wirep++;
				}
			}
			if(PtInRect(OnScreen,&EditClipper))
			{	OldFrame=elem->Body;
				OffsetRect(&OldFrame,MousePoint.h,MousePoint.v);
				FrameRect(&OldFrame);
				offset.h=MousePoint.h-OldPoint.h;
				offset.v=MousePoint.v-OldPoint.v;
				wirep=wires;
				for(i=0;i<numwires;i++)
				{	MoveTo(wirep->x1+MousePoint.h,wirep->y1+MousePoint.v);
					LineTo(wirep->x2,wirep->y2);
					wirep++;
				}
				moved= -1;
			}
			else
			{	moved=1;	/*	Moved outside edit area, snap back. */
			}
			OldPoint=MousePoint;
		}
	}	while(downflag);
	
	if(moved<0) 			/*	Is destination of move valid?		*/
	{	FrameRect(&OldFrame);
		wirep=wires;
		for(i=0;i<numwires;i++)
		{	MoveTo(wirep->x1+OldPoint.h,wirep->y1+OldPoint.v);
			LineTo(wirep->x2,wirep->y2);
			wirep++;
		}
		if(OldPoint.h || OldPoint.v)
		{	/*	Move element and force a redraw.					*/
			OffsetRect(&elem->Body,OldPoint.h,OldPoint.v);
			OffsetRect(&elem->InRect,OldPoint.h,OldPoint.v);
			OffsetRect(&elem->OutRect,OldPoint.h,OldPoint.v);
			ExpandTrash(elem->Body.left,elem->Body.top);
			ExpandTrash(elem->Body.right,elem->Body.bottom);
			InvalTrash();
		}
	}
	PenCopy();
	PenBlack();
	
	return	moved;
}

/*
>>	This routine finds out what to do when the user click with the
>>	arrow tool.
*/
void	DoArrowTool()
{
	Element 	*elem,*found;
	long		offs,elemsize;
	int 		i,j,foundpart;
	Input		*ip;
	Point		spot;
	Rect		Eraser;

	ClipEditArea();
	
	GetMouseDownPoint(&spot);
	
	/*	First, make it possible to scan element list backwards. */
	elemsize=0;
	ElemCount=0;
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		elem->PrevLength=elemsize;
		elemsize=elem->Length;
		ElemCount++;
	}
	
	LastElem=elem;
	found=0;
	
	i=ElemCount;
	while(i-- && !found)				/*	Search backwards for a possible */
	{	if(PtInRect(spot,&elem->Body))	/*	hit in an element.				*/
		{	found=elem;
			if(found->Type==CONN)		/*	Connector hit?					*/
			{	Rect	temp;			/*	Do we draw a new line or move?	*/
			
				temp=elem->Body;
				InsetRect(&temp,2,2);
				foundpart=PtInRect(spot,&temp) ? 2 : 1;
			}
			else
				foundpart=1;			/*	Move element.					*/
		}
		if(PtInRect(spot,&elem->OutRect))
		{	found=elem; 				/*	Output was hit. Draw a new line.*/
			foundpart=2;
		}
		elem=(Element *)(((char *) elem)-elem->PrevLength);
	}

	if(found)	/*	Click was found */
	{	switch(foundpart)
		{	case 1: /*	Click was found within element body.	*/
				switch(found->Type)
				{	case INPT:	/*	Move or flip value. */
						if(MoveElementAround(found,0)==0)
						{	found->Out[0].Data ^= 1;
							Eraser=found->Body;
							InsetRect(&Eraser,1,1);
							EraseRect(&Eraser);
							MoveTo(found->Body.left+6,found->Body.bottom-4);
							DrawChar('0'+(found->Out[0].Data & 1));
						}
						break;
					case CLOK:	/*	Move or change clock rate.	*/
						if(MoveElementAround(found,0)==0)
						{	if(spot.h < found->Body.left+8)
							{	found->PrivData=(found->PrivData+1) % 10;
								Eraser=found->Body;
								InsetRect(&Eraser,1,1);
								EraseRect(&Eraser);
								MoveTo(found->Body.left+2,found->Body.bottom-3);
								Line(6,0);
								Line(0,-10);
								Line(5,0);
								MoveTo(found->Body.left+2,found->Body.bottom-4);
								DrawChar('0'+found->PrivData);
							}
						}
						break;
					default:	/*	Just move the element.	*/
						MoveElementAround(found,0);
						break;
				}
				break;
			case 2: /*	Click was found within output rectangle.	*/
				i=(found->Type==CONN) ? 0 : (spot.v-found->OutRect.top)/16;
				RubberBandOutputWire(found,i);
				break;
		}
	}
	else	/*	Split wire into two with a new connector point. */
	{	int 	x1,y1,x2,y2;
		Input	*ip2;
	
		i=ElemCount;
		elem=LastElem;
		found=0;
		while(i-- && !found)	/*	First find if a line was really hit.	*/
		{	ip=(Input *)&elem->Out[elem->Outputs];
			for(j=0;j<elem->Inputs;j++)
			{	if(GetConnectLine(ip,elem,j,&x1,&y1,&x2,&y2))
				if(PntOnLine(x1,y1,
							 x2,y2,
							 spot.h,spot.v)==2)
				{	found=SimAllocate(sizeof(Element)+sizeof(Input));
					InitConnector(found,(spot.h+4) & ~7,(spot.v+4) & ~7);
					CurHeader->Last=SimEnd;
					
					ip2=(Input *)&found->Out[1];
					*ip2= *ip;
					ip->Chip=(char *)found-SimBase;
					ip->Pin=0;

					switch(MoveElementAround(found,-1))
					{	case -1:	
						case  0:
							InvalLine(x1,y1,x2,y2);
							break;
						case  1:	/*	User canceled by moving outside edit area	*/
							*ip= *ip2;
							CurHeader->Last=(char *)found-SimBase;
							break;
					}
					
					j=elem->Inputs; /*	Fall out of search loop.	*/
					i=0;
				}
				ip++;
			}
			elem=(Element *)(((char *) elem)-elem->PrevLength);
		}
	}
	RestoreClipping();
}
