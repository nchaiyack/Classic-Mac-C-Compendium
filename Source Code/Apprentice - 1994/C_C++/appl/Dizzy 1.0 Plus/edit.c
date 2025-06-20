/*
>>	Dizzy 1.0 Edit.c
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
*/

#include "dizzy.h"

/*
>>	This routine is called when the user clicks in the edit area.
>>	The main purpose is to decide which tool to use and how to
>>	manage the use of that tool.
*/
void	DoEditClick()
{
	int 			WhichButton;
	ToolSelection	*sel;
	Element 		*elem;
	Point			spot;
	long			obtype;
	int 			createflag;
	
#ifdef	MACINTOSH
	SetPort(MyWind);
#endif

	if(SplashVisible)
	{	SplashVisible=0;
		InvalTrash();
	}

	WhichButton=GetDownButton();
	/*	Hilite the tool we are going to use.	*/
	sel= &ToolButtons[WhichButton];
	if(sel->Function)
	{	HILITEMODE;
		InvertRect(&sel->Prime);
		if(sel->Function<0)
		{	HILITEMODE;
			InvertRect(&sel->Secondary);
		}
	}

	ClipEditArea();
	createflag=0;
	switch(sel->Function)	/*	Handle tool actions:	*/
	{	case 1:
			elem=CreateNewElement(1,1,NOT_,16);
			elem->Flags |= INVERTED;
			goto movement;
		case -2:
		case -3:
		case -4:
		case 2:
		case 3:
		case 4:
			switch(sel->Function)
			{	case -2:	obtype=NAND;	break;
				case -3:	obtype=NOR_;	break;
				case -4:	obtype=NXOR;	break;
				case  2:	obtype=AND_;	break;
				case  3:	obtype=OR__;	break;
				case  4:	obtype=XOR_;	break;
			}
			elem=CreateNewElement(InputSelector,1,obtype,32);
			if((*(char *)(&obtype))=='N')		
				elem->Flags |= INVERTED;
			goto movement;
		case 8:
		case 10:
			if(sel->Function==8)
				obtype=RS__;
			else
				obtype=D___;

			elem=CreateNewElement(2,2,obtype,32);
			goto movement;
		case 9:
			elem=CreateNewElement(3,2,JK__,32);
			goto movement;
			break;
		case 11:
			elem=CreateNewElement(1,1,INPT,16);
			elem->Out[0].Data=0;
			elem->InRect=NilRect;
			elem->PrivData=CountElementType(INPT)+1;
			goto movement;
		case 12:
			elem=CreateNewElement(1,1,OUTP,16);
			elem->PrivData=CountElementType(OUTP)+1;
			elem->Out[0].Data=0;
			goto movement;
		case 13:
			elem=CreateNewElement(4,0,HEXD,16);
			goto movement;
		case 14:
			elem=CreateNewElement(0,1,ONE_,16);
			elem->Out[0].Data= -1;
			goto movement;
		case 15:
			elem=CreateNewElement(0,1,ZERO,16);
			elem->Out[0].Data=0;
			goto movement;
		case 16:
			elem=CreateNewElement(0,1,CLOK,16);
			elem->PrivData=0;
			goto movement;
movement:
			if(MoveElementAround(elem,-1)<0)
			{	createflag= -1;
			}
			else
			{	createflag=0;
				SimEnd=((char *)elem)-SimBase;
			}
			break;
		case 17:	/*	Arrow tool for moving & manipulating elements.	*/
			DoArrowTool();
			break;
		case 18:	/*	Scoller tool									*/
			DoHandScroller();
			break;
		case 19:	/*	Zapper tool for deleting elements and lines.	*/
			DoZapperTool();
			break;
	}
	
	if(createflag)
	{	SetTrashRect(&elem->Body);
		InvalTrash();
		CurHeader->Last=SimEnd;
	}

	RestoreClipping();
	
	/*	Unhilite selected tool: */
	if(sel->Function)
	{	HILITEMODE;
		InvertRect(&sel->Prime);
		if(sel->Function<0)
		{	HILITEMODE;
			InvertRect(&sel->Secondary);
		}
	}
	
	/*	Reset button function the locked tool.	*/
	if(ToolLocks[WhichButton].Function != sel->Function)
	{	InvertButtonTag(WhichButton);
		*sel=ToolLocks[WhichButton];
		InvertButtonTag(WhichButton);
	}
}
