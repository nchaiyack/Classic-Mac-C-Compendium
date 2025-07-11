/*
>>	Dizzy 0.0	protos.h
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
*/


/*
>>	Function prototypes are defined for those compilers that support them.
>>	The Think C compiler supports prototypes that look like the following:
*/
#ifdef	MACINTOSH
Element 	*SimAllocate(long request);
long		PtrToOffset(void *pt);
void		PictBit(BitMap *Bits,int PictId);
int 		GetConnectLine(Input *ip,Element *elem,int index,int *x1,int *y1,int *x2,int *y2);
Element 	*CreateNewElement(int ins,int outs,long obtype,int width);
void		PresetElement(Element *newel,int x,int y,int width);
int 		CountElementType(long obtype);
int 		PntOnLine(long px,long py,long qx,long qy,long tx,long ty);
int 		ConnectorValue(Input *Inp);
void		RunSimulation();
Element 	*InsertCustomChip(char *name,int x,int y);
int 		GetMouseTrackEvent(Point *pt);
void		SetTrashRect(Rect *r);
void		ExpandTrash(int x, int y);
#else
/*
>>	These are for compilers that do not like the above prototypes.
>>	Compilers with 32 bit ints do not need prototypes, so just defining
>>	the return types is sufficient. If you have 16 bit ints, you need to
>>	define some kind of prototypes to enforce type conversions.
*/
Element 	*SimAllocate();
long		PtrToOffset();
void		PictBit();
int 		GetConnectLine();
Element 	*CreateNewElement();
void		PresetElement();
int 		CountElementType();
int 		PntOnLine();
int 		ConnectorValue();
void		RunSimulation();
Element 	*InsertCustomChip();
int 		GetMouseTrackEvent();
#endif
