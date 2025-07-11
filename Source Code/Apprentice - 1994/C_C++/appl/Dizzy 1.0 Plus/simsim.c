/*
>>	Dizzy 1.0	SimSim.c
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
>>
>>	The output values are stored in a shifting variable. The current value
>>	is stored as the second-least significant bit and new (possibly unknown)
>>	value is stored as 1. This way you can detect the last n (where n is the
>>	number of bits in int-1) changes in the signal.
>>
>>	This is a very simple (and inefficient) simulation system, but it does
>>	the work quite well. I recommend an event based system, if you want to
>>	improve the simulation drastically. An event based system will require
>>	changes in the "Element" data type and other structures.
*/

#include "dizzy.h"

extern	char	HexDigits[];	/*	"01234567890ABCDEF"	*/

/*
>>	Read the output indicated by an input.
*/
int 	ReadOutput(Inp)
Input	*Inp;
{
	if(Inp->Chip)
	{	return(((Element *)(SimBase+Inp->Chip))->Out[Inp->Pin].Data);
	}
	return 0;
}
/*
>>	Simulate a custom chip.
*/
void	ResolveCustom(elem)
Element *elem;
{
	int 			i;
	Input			*ip;
	TableHeader 	*customheader,*oldheader;
	Ptr 			newbaseptr,oldbaseptr;
	long			offs;
	Element 		*subelem;

	elem->Flags |= RESOLVED;
	ip=(Input *)&elem->Out[elem->Outputs];
	customheader=(TableHeader *)(ip+elem->Inputs);
	oldheader=CurHeader;
	newbaseptr=(Ptr)customheader;
	oldbaseptr=SimBase;

	for(offs=customheader->First;offs<customheader->Last;offs+=subelem->Length)
	{	subelem=(Element *)(newbaseptr+offs);
		if(subelem->Type==INPT)
		{	subelem->Out[0].Data=ConnectorValue(ip++)>>1;
		}
	}
	SimLevel++;
	SimBase=newbaseptr;
	CurHeader=customheader;

	RunSimulation();

	i=0;
	for(offs=customheader->First;offs<customheader->Last;offs+=subelem->Length)
	{	subelem=(Element *)(newbaseptr+offs);
		if(subelem->Type==OUTP)
		{	elem->Out[i++].Data=ConnectorValue((Input *)(subelem->Out+1));
		}
	}

	CurHeader=oldheader;
	SimBase=oldbaseptr;
	SimLevel--;
}

/*
>>	Find out the value of a zero-delay device such as a connector,
>>	an output or a custom chip. Uses recursion.
*/
int 	ConnectorValue(Inp)
Input	*Inp;
{
	Element *elem;
	
	if(Inp->Chip)
	{	elem=(Element *)(SimBase+Inp->Chip);
		
		if(elem->Flags & RESOLVED)
		{	return	elem->Out[Inp->Pin].Data;
		}
		else
		{	elem->Flags |= RESOLVED;
			if(elem->Type==CONN || elem->Type==OUTP)
			{	elem->Out[Inp->Pin].Data=ConnectorValue((Input *)&elem->Out[1]);
				return	elem->Out[Inp->Pin].Data;
			}
			else
			if(elem->Type==CUST)
			{	ResolveCustom(elem);
				return	elem->Out[Inp->Pin].Data;
			}
			else
			{	return	elem->Out[Inp->Pin].Data;
			}
		}
	}
	else
	{	return 0;
	}
}
/*
>>	Run a single "step" of the simulation. Go through all elements
>>	and recalculate new values.
*/
void	RunSimulation()
{
	register	Element 	*elem;
	register	long		offs;
	register	int 		i,a;
				Input		*ip;
				Rect		Eraser;

	/*	Mark all chips as unresolved.	*/
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		elem->Flags &= ~ RESOLVED;
		for(i=0;i<elem->Outputs;i++)
		{	elem->Out[i].Data <<= 1;
		}
	}
	/*	Resolve CONNectors, OUTPuts and CUSTom chips first.	*/
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		if(elem->Type==CONN || elem->Type==OUTP)
		{	elem->Flags |= RESOLVED;
			elem->Out[0].Data = ConnectorValue((Input *)&elem->Out[1]);
		}
		else
		if(elem->Type==CUST)
		{	if(!(elem->Flags & RESOLVED))
				ResolveCustom(elem);
		}
	}
	/*	Resolve all other elements now. These elements have a
	>>	delay of at least one step.
	*/
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		ip=(Input *)&elem->Out[elem->Outputs];

		switch(elem->Type)
		{	case AND_:
			case NAND:
				a=NEWVALUE;
				for(i=0;i<elem->Inputs;i++)
				{	if(!(ReadOutput(ip++) & CURRENTVALUE))
					{	a=0;
						i=elem->Inputs;
					}
				}
				elem->Out[0].Data |= a;
				if(elem->Type == NAND)
				{	elem->Out[0].Data ^= NEWVALUE;
				}
				break;
			case OR__:
			case NOR_:
				a=0;
				for(i=0;i<elem->Inputs;i++)
				{	if((ReadOutput(ip++) & CURRENTVALUE))
					{	a=NEWVALUE;
						i=elem->Inputs;
					}
				}
				elem->Out[0].Data |= a;
				if(elem->Type == NOR_)
				{	elem->Out[0].Data ^= NEWVALUE;
				}
				break;
			case XOR_:
			case NXOR:
				a=0;
				for(i=0;i<elem->Inputs;i++)
				{	if((ReadOutput(ip++) & CURRENTVALUE))
					{	a^=NEWVALUE;
					}
				}
				elem->Out[0].Data |= a;
				if(elem->Type == NXOR)
				{	elem->Out[0].Data ^= NEWVALUE;
				}
				break;
			case RS__:
				i=ReadOutput(ip++) & CURRENTVALUE;
				a=ReadOutput(ip++) & CURRENTVALUE;
				if(i || (elem->Out[1].Data & CURRENTVALUE)) elem->Out[1].Data |= NEWVALUE;
				if(a || (elem->Out[0].Data & CURRENTVALUE)) elem->Out[0].Data |= NEWVALUE;

				if(i)	elem->Out[0].Data &= ~NEWVALUE;
				if(a)	elem->Out[1].Data &= ~NEWVALUE;
				break;

			case D___:
				i=(ReadOutput(ip++) & CURRENTVALUE)>>1;
				a=ReadOutput(ip++);

				if((a & CURRENTVALUE) && !(a & OLDVALUE))
				{	elem->Out[0].Data |= i;
				}
				else
				{	elem->Out[0].Data |= (elem->Out[0].Data & CURRENTVALUE)>>1;
				}
				elem->Out[1].Data= ~elem->Out[0].Data;
				break;
			case JK__:
			
				a=ReadOutput(ip+1);
				
				if((a & CURRENTVALUE) && !(a & OLDVALUE))
				{
					if(ReadOutput(ip) & CURRENTVALUE)	i=1;
					else								i=0;
					if(ReadOutput(ip+2) & CURRENTVALUE) i|=2;
					switch(i)
					{	case 0:
							elem->Out[0].Data |= (elem->Out[0].Data & CURRENTVALUE)>>1;
							break;
						case 1:
							elem->Out[0].Data |= NEWVALUE;
							break;
						case 3:
							elem->Out[0].Data |= (elem->Out[1].Data & CURRENTVALUE)>>1;
							break;
					}
				}
				else
				{	elem->Out[0].Data |= (elem->Out[0].Data & CURRENTVALUE)>>1;
				}
				elem->Out[1].Data= ~elem->Out[0].Data;
				break;
			case NOT_:
				if(!(ReadOutput(ip) & CURRENTVALUE))
					elem->Out[0].Data |= NEWVALUE;
				break;
			case CLOK:
				if(SimTimer & (1<<elem->PrivData))
					elem->Out[0].Data |= NEWVALUE;
				break;
			case ZERO:
				break;
			case ONE_:
				elem->Out[0].Data |= NEWVALUE;
				break;
			case OUTP:
				if(SimLevel==0)
				if(((elem->Out[0].Data & CURRENTVALUE) ? 0 : 1) ^
				   ((elem->Flags & DISPLAYEDVALUE) ? 0 : 1))
				{	Eraser=elem->Body;
					InsetRect(&Eraser,1,1);
					EraseRect(&Eraser);
					MoveTo(elem->Body.left+6,elem->Body.bottom-4);
					elem->Flags ^= DISPLAYEDVALUE;
					DrawChar(elem->Flags & DISPLAYEDVALUE ? '1' : '0');
				}
				break;
			case HEXD:
				if(SimLevel)	break;
				a=0;
				for(i=0;i<4;i++)
				{	a = (a>>1) | ((ReadOutput(ip++) & CURRENTVALUE) ? 8 : 0);
				}
				if(a!=elem->PrivData)
				{	Eraser=elem->Body;
					InsetRect(&Eraser,1,1);
					EraseRect(&Eraser);
					MoveTo(elem->Body.left+6,elem->Body.bottom-28);
					elem->PrivData=a;
					DrawChar(HexDigits[elem->PrivData]);
				}
				break;
			case INPT:
				if(elem->Out[0].Data & CURRENTVALUE)
					elem->Out[0].Data |= NEWVALUE;
				break;
		}
	}
}
