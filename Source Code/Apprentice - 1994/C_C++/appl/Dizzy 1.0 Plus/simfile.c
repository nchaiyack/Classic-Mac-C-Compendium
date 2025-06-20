/*
>>	Dizzy 1.0	SimFile.c
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
>>
>>	This file contains all file-handling routines for Dizzy.
*/

#include "dizzy.h"
#include <stdio.h>

#ifndef MACINTOSH
extern	char	CurFileName[];	/*	Used only for Unix, */
extern	char	*DestFileName;	/*	see xstuff.h		*/
#endif

/*
>>	Given a pointer to the element structure,
>>	a position and a width, this routine fills
>>	out the rectangle parts of the structure by
>>	assuming that it follows a standard format.
*/
void	PresetElement(newel,x,y,width)
register	Element *newel;
int 		x,y;
int 		width;
{
	int 	pinheight;
	int 	inoff,outoff;
	
	pinheight=newel->Inputs>newel->Outputs ?
								newel->Inputs :
								newel->Outputs;

	inoff=(pinheight-newel->Inputs)*8;
	outoff=(pinheight-newel->Outputs)*8;

	newel->Flags=0;
	newel->Body.top=y;
	newel->Body.bottom=y+pinheight*16;
	newel->Body.left=x;
	newel->Body.right=x+width;
	
	newel->InRect.left=x-8;
	newel->InRect.right=x;
	newel->InRect.top=y+inoff;
	newel->InRect.bottom=newel->InRect.top+newel->Inputs*16;
	
	newel->OutRect.left=newel->Body.right;
	newel->OutRect.right=newel->Body.right+13;
	newel->OutRect.top=y+outoff;
	newel->OutRect.bottom=newel->OutRect.top+newel->Outputs*16;
}
/*
>>	Open a file, read in all the available data into the
>>	current header and return. If an error occurs, this
>>	routine just quits. Someone could add better error handling.
*/
void	OpenFile(thefile)
FILE	*thefile;
{
				int 			version;
				long			obtype;
				int 			x,y;
	register	Element 		*newel;

				int 			ins;
				long			esize;
				Input			*InArray;

				int 			InputCount,OutputCount;
				
	InputCount=0;
	OutputCount=0;

	if(thefile)
	{	if(fscanf(thefile,"Dizzy %d\n",&version))
		{	do
			{	fread(&obtype,sizeof(char),4,thefile);
				fscanf(thefile," %d %d ",&x,&y);
				switch(obtype)
				{	case ORIG:
						fscanf(thefile,"\n");
						CurHeader->XOrig=x;
						CurHeader->YOrig=y;
						break;
					case INPT:
					case CLOK:
					case ONE_:
					case ZERO:
						newel=SimAllocate(sizeof(Element));
						newel->Flags=0;
						newel->Type=obtype;
						newel->Length=sizeof(Element);
						newel->Inputs=0;
						newel->Outputs=1;
						newel->Out[0].Data=0;

						PresetElement(newel,x,y,16);

						switch(obtype)
						{	case INPT:
								newel->PrivData= ++InputCount;
								fscanf(thefile,"%d\n",&newel->Out[0].Data);
								break;
							case CLOK:
								fscanf(thefile,"%d\n",&newel->PrivData);
								break;
						}
						break;
					case AND_:
					case NAND:
					case OR__:
					case NOR_:
					case XOR_:
					case NXOR:					
						fscanf(thefile,"%d",&ins);
						esize=sizeof(Element)+sizeof(Input)*ins;
						newel=SimAllocate(esize);
						newel->Flags=0;
						newel->Type=obtype;
						newel->Length=esize;
						newel->Inputs=ins;
						newel->Outputs=1;
						PresetElement(newel,x,y,32);
						InArray=(Input *)&(newel->Out[1]);
						while(ins--)
						{	fscanf(thefile," %ldP%d",&(InArray->Chip),&(InArray->Pin));
							InArray++;
						}
						fscanf(thefile,"\n");
						
						if(obtype==NAND || obtype==NOR_ || obtype==NXOR)
						{	newel->Flags |= INVERTED;
						}
						break;
					case RS__:
					case D___:
					case JK__:
						if(obtype==JK__)	ins=3;
						else				ins=2;
						
						esize=sizeof(Element)+sizeof(Output)+sizeof(Input)*ins;
						newel=SimAllocate(esize);
						newel->Type=obtype;
						newel->Flags=0;
						newel->Length=esize;
						newel->Inputs=ins;
						newel->Outputs=2;
						PresetElement(newel,x,y,32);
						InArray=(Input *)&(newel->Out[2]);
						while(ins--)
						{	fscanf(thefile," %ldP%d",&(InArray->Chip),&(InArray->Pin));
							InArray++;
						}
						fscanf(thefile,"\n");
						break;
					case CONN:
						esize=sizeof(Element)+sizeof(Input);
						newel=SimAllocate(esize);
						InitConnector(newel,x,y);
						InArray=(Input *)&(newel->Out[1]);
						ins=fscanf(thefile,"%ldP%d\n",&(InArray->Chip),&(InArray->Pin));
						break;
					case OUTP:
					case NOT_:
						esize=sizeof(Element)+sizeof(Input);
						newel=SimAllocate(esize);
						newel->Flags=0;
						newel->Type=obtype;
						newel->Length=esize;
						newel->Inputs=1;
						newel->Outputs=1;

						InArray=(Input *)&(newel->Out[1]);
						fscanf(thefile,"%ldP%d\n",&(InArray->Chip),&(InArray->Pin));
						PresetElement(newel,x,y,16);
						
						if(obtype==NOT_)
						{	newel->Flags |= INVERTED;
						}
						else
						{	newel->PrivData= ++OutputCount;
						}
						break;
					case HEXD:
						esize=sizeof(Element)-sizeof(Output)+4*sizeof(Input);
						newel=SimAllocate(esize);
						newel->Type=obtype;
						newel->Flags=0;
						newel->Length=esize;
						newel->Inputs=4;
						newel->Outputs=0;
						newel->PrivData=0;
						PresetElement(newel,x,y,16);
						InArray=(Input *)&(newel->Out[0]);
						for(ins=0;ins<4;ins++)
						{	fscanf(thefile," %ldP%d",&(InArray->Chip),&(InArray->Pin));
							InArray++;
						}
						fscanf(thefile,"\n");
						break;
					case CUST:
						{	unsigned	char	name[MAXFILENAME];
										int 	i;
							
							fgets((char *)name,MAXFILENAME-1,thefile);
							i= -1;
							while(name[++i]!=0)
							{	if(name[i]<32) name[i]=0;
							}
							newel=InsertCustomChip((char *)name,x,y);
							fscanf(thefile," %d",&ins);

							InArray=(Input *)&(newel->Out[newel->Outputs]);
							if(ins>newel->Inputs)
								i=newel->Inputs;
							else
								i=ins;
							while(i--)
							{	fscanf(thefile," %ldP%d",&(InArray->Chip),&(InArray->Pin));
								InArray++;
							}
							i=ins-newel->Inputs;
							while(i-->0)
							{	fscanf(thefile," %*ldP%*d");
							}

							fscanf(thefile,"\n");
						}
					case ENDF:
						break;
					default:
						CurHeader->Last=CurHeader->First;
						fclose(thefile);
						return;
				}
			}	while(obtype!=ENDF && !feof(thefile) && !ferror(thefile));

			CurHeader->Last=SimEnd+SimPtr-SimBase;
			AssignNumbers();
			IndexToOffset();
		}
		fclose(thefile);
	}
}
/*
>>	Write the main header to a file. This routines doesn't
>>	report any errors. Here's your chance to make dizzy better!
*/
void	WriteFile(filename)
char	*filename;
{
				FILE			*thefile;
				int 			version;
	register	Element 		*newel;
				int 			i;
				Input			*ip;
				long			offs;
				Element 		*elem;

	thefile=fopen(filename,"w");
	if(thefile)
	{	fprintf(thefile,"Dizzy 1\n");
		fprintf(thefile,"ORIG	%d	%d\n",MainHeader->XOrig,MainHeader->YOrig);
		DatabaseCleanup();
		AssignNumbers();
		OffsetToIndex();
		for(offs=MainHeader->First;offs<MainHeader->Last;offs+=elem->Length)
		{	elem=(Element *)(SimBase+offs);
			ip=(Input *)&elem->Out[elem->Outputs];
			
			fwrite(&elem->Type,sizeof(char),4,thefile);
			if(elem->Type == CONN)
			{	fprintf(thefile,"	%d	%d	",elem->Body.left+4,elem->Body.top+4);
			}
			else
			{	fprintf(thefile,"	%d	%d	",elem->Body.left,elem->Body.top);
			}
			switch(elem->Type)		
			{	case INPT:
					fprintf(thefile,"%d\n",elem->Out[0].Data & NEWVALUE);
					break;
				case ONE_:
				case ZERO:
					fprintf(thefile,"\n");
					break;
				case CLOK:
					fprintf(thefile,"%d\n",elem->PrivData);
					break;
				case AND_:
				case NAND:
				case OR__:
				case NOR_:
				case XOR_:
				case NXOR:
					fprintf(thefile,"%d",elem->Inputs);
					for(i=0;i<elem->Inputs;i++)
					{	fprintf(thefile,"	%dP%d",(int)ip->Chip,ip->Pin);
						ip++;
					}
					fprintf(thefile,"\n");
					break;
				case RS__:
				case D___:
				case JK__:
				case CONN:
				case OUTP:
				case NOT_:
				case HEXD:
					for(i=0;i<elem->Inputs;i++)
					{	fprintf(thefile,"	%dP%d",(int)ip->Chip,ip->Pin);
						ip++;
					}
					fprintf(thefile,"\n");
					break;
				case CUST:
					{	TableHeader *customheader;
					
						customheader=(TableHeader *)(elem->Inputs+(Input *)&elem->Out[elem->Outputs]);
						fprintf(thefile,"%s\n",customheader+1);
						
						fprintf(thefile,"			%d",elem->Inputs);
						for(i=0;i<elem->Inputs;i++)
						{	fprintf(thefile,"	%dP%d",(int)ip->Chip,ip->Pin);
							ip++;
						}
						fprintf(thefile,"\n");
					}
					break;
			}
		}
		IndexToOffset();
		fprintf(thefile,"END.\n");
		fclose(thefile);
	}
}

/*
>>	Prompt for a filename and open that file, is user said it's ok.
*/
void	PromptOpenFile()
{
	FILE		*thefile;
#ifdef MACINTOSH
	long		thetype;
	Point		thepoint = {80,160};
	char		name[256];
	int 		vrefnum;
	
	thetype='TEXT';
	SFGetFile(thepoint,"\p",0L,1,&thetype,0L,&NameOfOpenFile);
	if(NameOfOpenFile.good)
	{
		CurHeader->XOrig=0;
		CurHeader->YOrig=0;
		SimEnd=CurHeader->First;
		CurHeader->Last=CurHeader->First;

		FileIsNamed= -1;
		GetVol(name,&vrefnum);
		SetVol(0L,NameOfOpenFile.vRefNum);
		BlockMove(1+NameOfOpenFile.fName,name,NameOfOpenFile.fName[0]);
		name[NameOfOpenFile.fName[0]]=0;
		SetWTitle(MyWind,NameOfOpenFile.fName); 
		thefile=fopen(name,"r");
		OpenFile(thefile);
		SetVol(0L,vrefnum);
		InvalRect(&EditR);
	}
#else
	DestFileName=CurFileName;
	if(DoFileSelection())
	{	CurHeader->XOrig=0;
		CurHeader->YOrig=0;
		SimEnd=CurHeader->First;
		CurHeader->Last=CurHeader->First;
		FileIsNamed= -1;
		thefile=fopen(CurFileName,"r");
		OpenFile(thefile);
		InvalRect(&EditR);
	}
#endif
}
void	SaveFileAs();
/*
>>	Save a file with the name that was previously given
>>	or ask for a name, if none was given.
*/
void	SaveFile()
{
#ifdef MACINTOSH
	char		name[256];
	int 		vrefnum;
#endif
	
	if(FileIsNamed)
	{
#ifdef	MACINTOSH
		GetVol(name,&vrefnum);
		SetVol(0L,NameOfOpenFile.vRefNum);
		BlockMove(1+NameOfOpenFile.fName,name,NameOfOpenFile.fName[0]);
		name[NameOfOpenFile.fName[0]]=0;
		WriteFile(name);
		SetVol(0L,vrefnum);
#else
		WriteFile(CurFileName);
#endif
	}
	else
	{	SaveFileAs();
	}
}
/*
>>	Ask for a filename and call SaveFile.
*/
void	SaveFileAs()
{
#ifdef MACINTOSH
	Point		thepoint = { 80, 160 };

	SFPutFile(thepoint,"\p",NameOfOpenFile.fName,0L,&NameOfOpenFile);
	if(NameOfOpenFile.good)
	{	FileIsNamed= -1;
		SetWTitle(MyWind,NameOfOpenFile.fName);
		SaveFile();
	}
#else
	DestFileName=CurFileName;
	if(DoFileSelection())
	{	FileIsNamed= -1;
		SaveFile();
	}
#endif
}
/*
>>	This routine is complicated, but basically it inserts
>>	a new header and circuit data inside a single element
>>	on the current level. You can use it to create subchips
>>	from files.
*/
Element *InsertCustomChip(name,x,y)
char	*name;
int 	x,y;
{
				FILE		*thefile;
				TableHeader *saved;
				long		SimBaseOffset,CurHeaderOffset,NewelOffset;
				Ptr 		savedbase;
	register	Ptr 		source,dest;
				int 		ins,outs;
	register	long		esize,size;
				int 		namelen,i,width;
				Element 	*newel;
				Input		*InArray;

#ifdef	MACINTOSH
	SetPort(MyWind);
	TextFont(geneva);
	TextSize(9);
	TextFace(0);
#endif

	/*	First extract the file name from the whole path.	*/
	namelen=0;
	while(name[namelen++]);
	for(i=namelen;i>0 && name[i]!='/';i--);
	if(name[i]=='/') i++;
	namelen-=i;

	/*	Look for the file with the given path and the current path.			*/
	thefile=fopen(name,"r");
	if(!thefile)
	{	name+=i;
		i=0;
		thefile=fopen(name,"r");
	}
	
	width=TextWidth(name,i,namelen-1);

	/*	Now save the old pointers as offsets while this routine is active.	*/
	SimBaseOffset=SimBase-SimPtr;
	CurHeaderOffset=((Ptr) CurHeader)-SimPtr;

	/*	Create a new base pointer.											*/
	SimBase=SimPtr+SimEnd;
	
	/*	Create a new header here. An element structure will later			*/
	/*	be inserted before this header.										*/
	esize=(namelen+i+sizeof(long)-1) & ~(sizeof(long)-1);	/*	force long-word alignment.	*/
	newel=SimAllocate(sizeof(TableHeader)+esize);
	CurHeader=(TableHeader *) newel;
	CurHeader->PathLen=namelen+i;
	CurHeader->First=sizeof(TableHeader)+esize;
	CurHeader->TitleLen=namelen;
	CurHeader->TitleWidth=width;

	/*	Copy the whole file name (path).	*/
	source=name;
	dest=(Ptr)(CurHeader+1);
	size=CurHeader->PathLen;
	while(size--)
	{	*dest++= *source++;
	}

	/*	Read the circuit from the file.		*/
	NewelOffset=SimBase-(char *)newel;
	OpenFile(thefile);
	newel=(Element *)(SimBase+NewelOffset);

	CurHeader->Last=SimPtr+SimEnd-SimBase;

	ins=CountElementType(INPT);		/*	Count input pins.	*/
	outs=CountElementType(OUTP);	/*	Count output pins.	*/
	
	/*	Now we know how large the element will be.			*/
	esize=sizeof(Element)+(outs-1)*sizeof(Output)+ins*sizeof(Input);

	/*	Allocate room for the element.						*/
	NewelOffset=SimBase-(char *)newel;
	SimAllocate(esize);
	newel=(Element *)(SimBase+NewelOffset);

	/*	Insert the element before the header and the data.	*/
	dest=SimPtr+SimEnd;
	source=dest-esize;
	size=CurHeader->Last;
	while(size--)
	{	*--dest= *--source;
	}
	
	esize+=CurHeader->Last;

	/*	Set element structure values.		*/
	newel->Flags=0;
	newel->Type=CUST;
	newel->Length=esize;
	newel->Inputs=ins;
	newel->Outputs=outs;
	
	PresetElement(newel,x,y,(width+31) & ~31);

	/*	Fill in inputs and outputs with default values.	*/
	InArray=(Input *)&(newel->Out[outs]);
	while(ins--)
	{	InArray->Chip=0;
		InArray->Pin=0;
		InArray++;
	}
	while(outs--)
	{	newel->Out[outs].Data=0;
	}
	
	/*	Restore previous header pointers from offsets.	*/
	SimBase=SimPtr+SimBaseOffset;
	CurHeader=(TableHeader *)(SimPtr+CurHeaderOffset);

	/*	Advance pointer of last item in current header.	*/
	CurHeader->Last=SimPtr-SimBase+SimEnd;
	return	newel;
}
static	char		CustomFileName[2048];

/*
>>	Ask for a file name of a custom chip and add that
>>	to the center of the edit area.
*/
void	AddCustomChip()
{
	int 		x,y;
	Element 	*custel;
#ifdef MACINTOSH
	SFReply 	customname;
	int 		vrefnum;
	long		thetype='TEXT';
	Point		thepoint={64,64};
	char		name[256];
	
	SFGetFile(thepoint,"\p",0L,1,&thetype,0L,&customname);
	if(customname.good)
	{	GetVol(name,&vrefnum);
		SetVol(0L,customname.vRefNum);

		BlockMove(1+customname.fName,name,customname.fName[0]);
		name[customname.fName[0]]=0;
		x=((EditR.right-EditR.left)/2+CurHeader->XOrig) & ~7;
		y=((EditR.bottom-EditR.top)/2+CurHeader->XOrig) & ~7;
		ClipEditArea();
		custel=InsertCustomChip(name,x,y);
		SetTrashRect(&custel->Body);
		InvalTrash();
		RestoreClipping();

		SetVol(0L,vrefnum);
	}
#else
	DestFileName=CustomFileName;
	if(DoFileSelection())
	{	x=((EditR.right-EditR.left)/2+CurHeader->XOrig) & ~7;
		y=((EditR.bottom-EditR.top)/2+CurHeader->XOrig) & ~7;
		ClipEditArea();
		custel=InsertCustomChip(CustomFileName,x,y);
		SetTrashRect(&custel->Body);
		InvalTrash();
		RestoreClipping();
	}
#endif
}
