/*
>>	Dizzy 1.0	Zapper.c
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
*/

#include "dizzy.h"

extern	Element *LastElem;	/*	From Arrow.c		*/
extern	int 	ElemCount;	/*	Also from Arrow.c	*/

/*
>>	The zapper tool allows the user to delete line segments and
>>	elements. If the mouse button is held down, the user can
>>	browse through the possible objects by moving the mouse.
>>	An object is only deleted after the mouse button is released.
*/
void	DoZapperTool()
{
	Element 	*elem,*found,*brother;
	int 		foundpin;
	long		offs,elemsize;
	int 		i,j,downflag;
	Input		*ip;
	Point		MousePoint,oldspot;
	Rect		selector;
	int 		typefound;
	int 		x1,y1,x2,y2;

	ClipEditArea();
	PenSize(5,5);
	PenXor();
#ifdef	MACINTOSH
	SetCursor(*GetCursor(129));
#endif

	/*	Prepare the data so that it can be searched backwards.	*/
	elemsize=0;
	ElemCount=0;
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		elem->PrevLength=elemsize;
		elemsize=elem->Length;
		ElemCount++;
	}
	LastElem=elem;

	typefound=0;
	found=0;
	
	GetMouseDownPoint(&MousePoint);
	oldspot.h= MousePoint.h+9999;	/*	Some improbable point	*/
	oldspot.v= MousePoint.v+9999;	/*	to simulate motion. 	*/
	
	do
	{	downflag=GetMouseTrackEvent(&MousePoint);
		
		/*	Did mouse move? */
		if(MousePoint.h!=oldspot.h || MousePoint.v!=oldspot.v)
		{	if(typefound)	/*	Was there a previous hilite to unhilite?	*/
			{	HILITEMODE;
				if(typefound>0)
				{	MoveTo(selector.left,selector.top);
					LineTo(selector.right,selector.bottom);
				}
				else
					InvertRect(&selector);
			}
			typefound=0;	/*	Current hilite==nil 						*/
			found=0;

			if(PtInRect(MousePoint,&EditClipper))	/*	Mouse in edit area? */
			{	i=ElemCount;
				elem=LastElem;
				while(i-- && !found)	/*	Search for a hit in body area.	*/
				{	if(PtInRect(MousePoint,&elem->Body))
					{	found=elem;
						typefound= -1;	/*	An element body was found.		*/
						selector=elem->Body;
						HILITEMODE; 	/*	Hilite the found element.		*/
						InvertRect(&selector);
					}
					elem=(Element *)(((char *) elem)-elem->PrevLength);
				}
		
				if(!typefound)			/*	No element body was found?	*/
				{	i=ElemCount;		/*	Look for a line instead.	*/
					elem=LastElem;
					while(i-- && !typefound)
					{	ip=(Input *)&elem->Out[elem->Outputs];
						for(j=0;j<elem->Inputs;j++)
						{	if(GetConnectLine(ip,elem,j,&x1,&y1,&x2,&y2))
							if(PntOnLine(x1,y1,
										 x2,y2,
										 MousePoint.h,MousePoint.v)==2)
							{	typefound=1;	/*	A line was found.	*/
								found=elem;
								foundpin=j;

								selector.left=x1-LCENTERING;
								selector.top=y1-LCENTERING;
								selector.right=x2-LCENTERING;
								selector.bottom=y2-LCENTERING;
								HILITEMODE; 	/*	Hilite the found line.	*/
								MoveTo(selector.left,selector.top);
								LineTo(selector.right,selector.bottom);

								j=elem->Inputs; /*	Fall out of ip-loop.	*/
								i=0;
							}
							ip++;
						}
						elem=(Element *)(((char *) elem)-elem->PrevLength);
					}
				}
			}
		}
		oldspot=MousePoint;
	}	while(downflag);

	if(typefound)		/*	Did we find something?		*/
	{	if(typefound>0) /*	Was it a line segment?		*/
		{	HILITEMODE; /*	Disconnect line segment.	*/
			MoveTo(selector.left,selector.top);
			LineTo(selector.right,selector.bottom);
			ip=(Input *)&found->Out[found->Outputs];
			ip[foundpin].Chip=0;
			ip[foundpin].Pin=0;
			InvalLine(selector.left,selector.top,selector.right,selector.bottom);
		}
		else			/*	It was an element.			*/
		{	long	ouroffset;
			int 	inptcount,outpcount;
			Input	ip2;
		
			if(found->Type==CONN)	/*	Unsplit lines	*/
			{	ip2= *(Input *)&found->Out[1];
			}
			else
			{	ip2.Chip=0; 	/*	Disconnect lines.	*/
				ip2.Pin=0;
			}
			
			/*	First, let's invalidate the display.					*/
			if(found->Type==OUTP || found->Type==INPT)
			{	SetTrashRect(&EditClipper);
			}
			else
			{	SetTrashRect(&found->Body);
			}

			/*	Find lines connected to inputs of this chip. Easy.		*/
			ouroffset=((char *)found)-SimBase;	/*	Our "signature".	*/
			ip = (Input *)&found->Out[found->Outputs];
			for(j=0;j<found->Inputs;j++)
			{	if(ip->Chip != ouroffset)
				if(GetConnectLine(ip,found,j,&x1,&y1,&x2,&y2))
				{	ExpandTrash(x2,y2);
				}
				ip++;
			}
		
			/*	Find lines connected to outputs of this chip. Harder.	*/
			for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
			{	elem=(Element *)(SimBase+offs);
				ip=(Input *)&elem->Out[elem->Outputs];
				for(j=0;j<elem->Inputs;j++)
				{	if(ip->Chip == ouroffset)
					{	GetConnectLine(ip,elem,j,&x2,&y2,&x1,&y1);
						ExpandTrash(x2,y2);
					}
					ip++;
				}
			}
			InvalTrash();	/*	Invalidate the whole area.	*/

			inptcount=0;	/*	Number of inputs so far.				*/
			outpcount=0;	/*	Number of outputs so far.				*/
			
			/*	Disconnect everything from the dead chip.				*/
			for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
			{	elem=(Element *)(SimBase+offs);
			
				if(elem->Type==INPT)
					elem->PrivData= ++inptcount;
				else
				if(elem->Type==OUTP)
					elem->PrivData= ++outpcount;

				ip=(Input *)&elem->Out[elem->Outputs];
				for(j=0;j<elem->Inputs;j++)
				{	if(ip->Chip==ouroffset)
					{	*ip=ip2;
					}
					ip++;
				}
			}
			HILITEMODE;
			InvertRect(&selector);

			found->Type=ZAPP;	/*	Food for garbage collector. 		*/
			found->Flags=0;
			found->Body=NilRect;
			found->InRect=NilRect;
			found->OutRect=NilRect;
			found->Inputs=0;
			found->Outputs=0;
			
			if(CountElementType(ZAPP)>MAX_ZAPS)
			{	DatabaseCleanup();	/*	Time to garbage collect.		*/
			}
		}
	}

#ifdef	MACINTOSH
	InitCursor();
#endif
	PenSize(1,1);
	PenCopy();
	RestoreClipping();
}
