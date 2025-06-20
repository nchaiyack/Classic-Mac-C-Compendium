/*
>>	Dizzy 1.0
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
*/

#include	"dizzy.h"

#ifdef	MACINTOSH
#include	<stdlib.h>	/*	malloc & realloc!	*/
#else
extern char *malloc();	/*	no malloc & realloc .h-file found in our unix machines. */
extern char *realloc();
#endif

/*
>>	This function allocates "request" bytes and returns
>>	a pointer (of type Element *) to the caller. Memory
>>	is allocated in RAMCHUNK sized chunks to keep things
>>	faster. Only a few pointers are globally kept pointing
>>	to things inside the allocated memory. These pointers
>>	should be fixed to point at the correct location after
>>	resize. You should be very careful with pointers that
>>	are stored in local variables. Avoid them whenever you
>>	can. (Especially recursive functions are dangerous.)
*/
Element 	*SimAllocate(request)
long	request;
{
	long	SimBaseOffset,CurHeaderOffset,MainHeaderOffset;

	while(SimEnd+request > SimSize) 	/*	Do we have insufficient memory preallocated?	*/
	{	SimSize+=RAMCHUNK;				/*	Increase to previous size+RAMCHUNK. 			*/
		SimBaseOffset=SimBase-SimPtr;	/*	Since realloc and SetHandleSize probably move	*/
		CurHeaderOffset=((Ptr) CurHeader)-SimPtr;	/*	the memory block, we can't assume	*/
		MainHeaderOffset=((Ptr) MainHeader)-SimPtr; /*	that pointers will be valid after	*/
													/*	the operation. Store as offsets.	*/
#ifdef	MACINTOSH
		HUnlock(SimHandle);
		SetHandleSize(SimHandle,SimSize);
		HLock(SimHandle);
		SimPtr= *SimHandle;
#else
		SimPtr=realloc(SimPtr,SimSize);
#endif
		SimBase=SimPtr+SimBaseOffset;	/*	Restore offsets into pointer form.				*/
		CurHeader=(TableHeader *)(SimPtr+CurHeaderOffset);
		MainHeader=(TableHeader *)(SimPtr+MainHeaderOffset);
	}
	
	SimEnd+=request;
	
	return	(Element *)(SimPtr+SimEnd-request);
}

/*
>>	This could almost be a macro. It just converts a pointer into the
>>	current data (pointed by SimBase) into an offset.
*/
long	PtrToOffset(pt)
void	*pt;
{
	return ((char *)pt)-SimBase;
}

/*
>>	The Chip component of an input structure is stored either
>>	as a memory offset or a number. This converts indices to
>>	memory offsets. Used with garbage collection and file operations.
*/
void	IndexToOffset()
{
				long		count;
	register	long		offs;
				int 		i;
	register	Element 	*elem;
	register	Input		*ip;
				long		*table;

	count=0;

	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		if(elem->Number>count)
			count=elem->Number;
	}

#ifdef	MACINTOSH
	table=(long *)NewPtr(sizeof(long)*count);
#else
	table=(long *)malloc(sizeof(long)*count);
#endif

	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		table[elem->Number-1]=offs;
	}
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		ip=(Input *)(&elem->Out[elem->Outputs]);
		
		for(i=0;i<elem->Inputs;i++)
		{	if(ip->Chip<=0 || ip->Chip>count)
			{	ip->Chip=0;
				ip->Pin=0;
			}
			else
			{	ip->Chip=table[ip->Chip-1];
				if(ip->Pin>((Element *)(SimBase+ip->Chip))->Outputs)
				{	ip->Chip=0;
					ip->Pin=0;
				}
			}
			ip++;
		}
	}
#ifdef	MACINTOSH
	DisposPtr(table);
#else
	free(table);
#endif
}
/*
>>	This program assigns numbers to elements. The number is stored
>>	in the Number component of the element.
*/
void	AssignNumbers()
{
	int 	count;
	long	offs;
	Element *elem;
	
	count=0;
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		elem->Number= ++count;
	}
}
/*
>>	This does the reverse of what IndexToOffset does. If you wish to
>>	move components around in memory, first convert offsets to indices
>>	and then do the moving. After you are through, convert back to
>>	offset form, since that is what dizzy mostly expects to have.
*/
void	OffsetToIndex()
{
	int 	i;
	Input	*ip;
	long	offs;
	Element *elem,*source;
	
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		ip=(Input *)&elem->Out[elem->Outputs];
		for(i=0;i<elem->Inputs;i++)
		{	if(ip->Chip)
			{	source=(Element *)(SimBase+ip->Chip);
				ip->Chip=source->Number;
			}
			ip++;
		}
	}
	
}
/*
>>	Do garbage collection. Removes any ZAPP type elements from
>>	the database.
*/
void	DatabaseCleanup()
{
	int 	count;
	long	offs,len,i;
	Element *elem;
	char	*source,*dest;

	AssignNumbers();
	OffsetToIndex();
	
	source=SimBase+MainHeader->First;
	dest=source;

	for(offs=MainHeader->First;offs<MainHeader->Last;offs+=len)
	{	elem=(Element *)(SimPtr+offs);
		len=elem->Length;
		if(elem->Type==ZAPP)
		{	source+=len;
		}
		else
		{	i=len;
			while(i-- > 0)
			{	*dest++= *source++;
			}
		}
	}
	MainHeader->Last=dest-SimBase;
	SimEnd=MainHeader->Last;
	IndexToOffset();
}
/*
>>	This creates a new element with ins inputs, outs outputs
>>	a type of obtype and a body width of width. This routine
>>	can be used for most standard components. The connector
>>	type is an exception.
*/
Element *CreateNewElement(ins,outs,obtype,width)
int 	ins,outs;
long	obtype;
int 	width;
{
	register	long		esize;
	register	Element 	*newel;
	register	Input		*ip;
				int 		i;
				Point		spot;

	esize=sizeof(Element)+ins*sizeof(Input)+(outs-1)*sizeof(Output);
	newel=SimAllocate(esize);
	newel->Flags=0;
	newel->Type=obtype;
	newel->Length=esize;
	newel->Inputs=ins;
	newel->Outputs=outs;
	
	ip=(Input *)&(newel->Out[outs]);
	for(i=0;i<outs;i++)
	{	newel->Out[i].Data=0;
	}
	for(i=0;i<ins;i++)
	{	ip->Chip=0;
		ip->Pin=0;
		ip++;
	}
	
	GetMouseDownPoint(&spot);
	spot.h &= ~7;
	spot.v &= ~7;
	PresetElement(newel,spot.h,spot.v,width);
	
	return newel;
}
/*
>>	Since you can't create a new connector with CreateNewElement,
>>	you can at least use this routine to initialize some of the
>>	fields in the struct.
*/
void	InitConnector(elem,x,y)
Element *elem;
int 	x,y;
{
	elem->Flags=0;
	elem->Type=CONN;
	elem->Length=sizeof(Element)+sizeof(Input);
	elem->Inputs=1;
	elem->Outputs=1;
	SetRect(&elem->Body,x-4,y-4,x+5,y+5);
	elem->OutRect=NilRect;
	elem->InRect=NilRect;		
}
/*
>>	Count how many types "obtype" occurs on this level of the
>>	database. Useful for counting ZAPPs and in CUSTom chips.
*/
int CountElementType(obtype)
long	obtype;
{
	long	offs;
	Element *elem;
	int 	count=0;
	
	for(offs=CurHeader->First;offs<CurHeader->Last;offs+=elem->Length)
	{	elem=(Element *)(SimBase+offs);
		if(elem->Type==obtype)
		{	count++;
		}
	}
	return count;
}
/*
>>	This allocates some memory for starters and
>>	creates the main level header of the database.
>>	It also sets up the simulation speed of the program
>>	and resets the clock counter.
*/
void	SimSetup()
{
	SimSize=RAMCHUNK;

#ifdef	MACINTOSH
	SimHandle=NewHandle(SimSize);	
	HLock(SimHandle);

	SimPtr= *SimHandle;
#else
	SimPtr=malloc(SimSize);
#endif

	SimEnd=0;

	MainHeader=(TableHeader *)SimAllocate(sizeof(TableHeader));
	MainHeader->XOrig=1;
	MainHeader->YOrig=1;
	MainHeader->First=SimEnd;
	MainHeader->Last=SimEnd;
	SimBase=SimPtr;
	CurHeader=MainHeader;
	SimTimer=0;
	SimSpeed=0; /*	Start out at maximum speed. */
}
