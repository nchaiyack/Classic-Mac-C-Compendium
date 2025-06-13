/*/
     Project Arashi: Huffman.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:39
     Created: Saturday, October 6, 1990, 22:34

     Copyright � 1990-1992, Juri Munkki
/*/

/*
>>	This file contains compression routines for the sound
>>	data of Project STORM. It uses the Huffman algorithm.
*/

#include "Huffman.h"
#include "Shuddup.h"
#include "asm.h"

Handle	PlainHand;		/*	Handle to contain uncompressed sounds.	*/
long	PlainSize;		/*	Size when uncompressed.					*/

int			MaxCodeLen;	/*	Longest binary coding used.				*/
long		MsgBitSize;	/*	Number of bits in the compressed data.	*/
Handle		MsgHand;	/*	Compressed data.						*/
treenode	**QuickTable;	/*	Quick lookup table for uncompress.	*/

treenode	Nodes[VALUES*2];	/*	Huffman encoding tree.			*/
treenode	*Sorted[VALUES];	/*	Table of remaining subtrees.	*/
int			UsedNodes;			/*	Number of nodes used.			*/
int			numsorted;			/*	Number of remaining subtrees.	*/

/*	Prototypes:
*/
treenode *GetSmallest(void);
void CombineNode(treenode *node0, treenode *node1);
void AssignCode(treenode *node, int len, long code);
void DeriveSound(void);
Handle	ReadDataFiles(long ftype);

/*
>>	This routine writes the bit patterns in the compressed
>>	form. It's fairly slow on 68000 and 68010 machines, but
>>	works quickly on others.
*/
void	OutputPhase()
{
	register	Handle	Outdata;
	register	long	*OutP;
	register	char	*SrcP;
	register	long	bitpos,data;
	register	long	bitwidth;
	register	int		i;
				long	codes[VALUES];
				int		lens[VALUES];
	
	Outdata=GetResource(SKRESTYPE,SKHUFFMANN);

	SetHandleSize(Outdata,sizeof(long)*VALUES+(MsgBitSize+7)/8);	
	if(!MemErr)
	{	OutP=(long *)*Outdata;
		SrcP=*PlainHand;
		for(i=0;i<VALUES;i++)
		{	*OutP++=Nodes[i].freq;
			codes[i]=Nodes[i].code;
			lens[i]=Nodes[i].codelen;
		}
		bitpos=0;
		
		if(CPUFlag<2)	/*	Incredibly slow version for 68000 & 68010 processors.	*/
		{	while(bitpos<MsgBitSize)
			{	data=codes[*SrcP];
				i=lens[*SrcP++];
				bitwidth = 1L <<(i-1);
				
				while(i--)
				{	if(data & bitwidth)	BitSet(OutP,bitpos++);
					else				BitClr(OutP,bitpos++);
					data <<= 1;
				}
			}
		}
		else			/*	Much faster version for real processors...			;-)	*/
		{	while(bitpos<MsgBitSize)
			{	data=codes[*SrcP];
				bitwidth=lens[*SrcP++];
				asm	68020
					{	BFINS	data,(OutP){bitpos:bitwidth}
						add.l	bitwidth,bitpos
					}
			}
		}
	}
	else
		DebugStr("\PHuffmann failed");
	ChangedResource(Outdata);
	WriteResource(Outdata);
}

/*
>>	GetSmallest returns the subtree with the
>>	lowest frequency count. It is used to find
>>	the two lowest counts that are to be combined
>>	into a new subtree.
*/
treenode	*GetSmallest()
{
	register	int			i;
	register	int			small;
	register	long		freq;
	register	treenode	*found;
	
	i=small=numsorted-1;
	freq=Sorted[numsorted-1]->freq;

	while(i--)
	{	if(Sorted[i]->freq<freq)
		{	small=i;
			freq=Sorted[i]->freq;
		}
	}
	found=Sorted[small];
	Sorted[small]=Sorted[--numsorted];
	return	found;
}
/*
>>	Create a new node out of the two nodes (subtrees)
>>	that are given as parameters.
*/
void	CombineNode(node0,node1)
treenode	*node0,*node1;
{
	register	treenode	*newnode;
	
	newnode=&Nodes[UsedNodes++];
	newnode->value=0xFF;
	newnode->codelen=0;
	newnode->code=0;
	newnode->freq=node0->freq+node1->freq;
	newnode->zeroptr=node0;
	newnode->oneptr=node1;
	newnode->typeflag=-1;
	
	Sorted[numsorted++]=newnode;
}
/*
>>	Recursively assigned a binary code
>>	pattern to all nodes. (This is the
>>	final step of the stage that determines
>>	the encoding.)
*/
void	AssignCode(node,len,code)
treenode	*node;
int			len;
long		code;
{
	node->code=code;
	node->codelen=len;
	
	if(node->typeflag)
	{	AssignCode(node->zeroptr,len+1,code*2);
		AssignCode(node->oneptr,len+1,code*2+1);
	}
	if(len>MaxCodeLen)
		MaxCodeLen=len;
}
/*
>>	The sound samples are run through this
>>	filter before they are packed. Packing
>>	the differences makes Huffman encoding
>>	much more efficient.
*/
void	DeriveSound()
{
	register	long	i;
	register	char	*p;
	register	char	delta,val;

	p=*PlainHand;
	for(i=PlainSize;i;i--)
	{	*p= (*p >> DROPBITS) & ANDMASK;
		p++;
	}

	p=*PlainHand;
	delta=128>>DROPBITS;
	for(i=PlainSize;i;i--)
	{	val=*p;
		*p++=(val-delta) & ANDMASK;
		delta=val;
	}
	
}
/*
>>	Build a tree out of the frequency
>>	counts. This routine is used for
>>	packing and unpacking of data.
*/
void	HuffmannTree()
{
	register	long		i;
	register	treenode	*small1,*small2;

	/*	Create pointers for used nodes.
	*/
	numsorted=0;
	for(i=0;i<VALUES;i++)
	{	if(Nodes[i].freq)
		{	Sorted[numsorted++]=&Nodes[i];
		}
	}
	
	/*	Huffmann tree generation.
	*/
	while(numsorted>1)
	{	small1=GetSmallest();
		small2=GetSmallest();
		CombineNode(small1,small2);
	}

	/*	Assign bit strings to tree nodes.
	*/
	MaxCodeLen=0;	
	AssignCode(Sorted[0],0,0);
	
	/*	Calculate useful statistics.
	*/
	MsgBitSize=0;	/*	Size of output in bits.					*/
	for(i=0;i<VALUES;i++)
	{	MsgBitSize+=Nodes[i].freq*Nodes[i].codelen;
	}
}
/*
>>	Compress the sound files in the current
>>	directory and store the compressed data
>>	in a pair of resources.
*/
void	DoCompress()
{
	register	long		i,j;
	register	char		*p;
	
	/*	First, read the data from disk:
	*/
	PlainHand=ReadDataFiles(SOUNDFILE);
	PlainSize=GetHandleSize(PlainHand);
	HLock(PlainHand);

	/*	Do a delta operation on the sound and divide by 2.
	*/
	DeriveSound();

	/*	Initialize node values.
	*/
	for(i=0;i<VALUES;i++)
	{	Nodes[i].value=i;
		Nodes[i].codelen=0;
		Nodes[i].code=0;
		Nodes[i].freq=0;
		Nodes[i].zeroptr=0;
		Nodes[i].oneptr=0;
		Nodes[i].typeflag=0;
	}

	/*	Make a frequency count.
	*/
	p=*PlainHand;
	for(i=PlainSize;i;i--)
	{	Nodes[*p++].freq++;
	}
	UsedNodes=VALUES;

	/*	Frequency plot in window.
	*/	
	for(i=0;i<VALUES;i++)
	{	j=200-(Nodes[i].freq*1000)/PlainSize;
		MoveTo(i,j);
		LineTo(i,j);
	}

	/*	Generate the tree and bit strings.
	*/
	HuffmannTree();

	if(MaxCodeLen<=32)
	{	OutputPhase();
	}
	else
	{	SysBeep(10);	/*	This should never happen.	*/
		DebugStr("\PHuffmann failed.");
	}
	HUnlock(PlainHand);
	DisposHandle(PlainHand);
}
/*
>>	DeCompress only handles the more generic part
>>	of the sound decompression. It reads the packed
>>	data and frequency counts. The frequency counts
>>	are used to rebuild the same tree that was used
>>	for compression. A fast lookup table is then
>>	built so that multiple bits can be decoded at
>>	once.
*/
void	DeCompress()
{	
	register	long	*MsgPtr;
	register	long	i,j;
			treenode	*TheNode;

	QuickTable=(treenode **)NewPtr(sizeof(treenode)*(1<<QTBITS));
	MsgHand=GetResource(SKRESTYPE,SKHUFFMANN);
	HLock(MsgHand);
	MsgPtr=(long *)*MsgHand;
	
	PlainSize=0;
	/*	Initialize node values.
	*/
	for(i=0;i<VALUES;i++)
	{	Nodes[i].value=i;
		Nodes[i].codelen=0;
		Nodes[i].code=0;
		Nodes[i].freq=*MsgPtr++;
		Nodes[i].zeroptr=0;
		Nodes[i].oneptr=0;
		Nodes[i].typeflag=0;
		
		PlainSize+=Nodes[i].freq;
	}
	UsedNodes=VALUES;

	/*	Generate the tree and bit strings.
	*/
	HuffmannTree();

	/*	Set up a table for quick access for the QTBITS first levels
	**	of the tree.
	*/
	for(i=0;i<(1<<QTBITS);i++)
	{	QuickTable[i]=0;
	}
	for(i=UsedNodes;i>=0;i--)
	{	if(Nodes[i].freq && Nodes[i].codelen<=QTBITS)
		{	QuickTable[Nodes[i].code<<(QTBITS-Nodes[i].codelen)]=&Nodes[i];
		}
	}

	for(i=0;i<(1<<QTBITS);i++)
	{	if(QuickTable[i])		TheNode=QuickTable[i];
		else					QuickTable[i]=TheNode;
	}
	
}
/*
>>	This routine first runs the previous one
>>	and then uses the tree and the lookup table
>>	to decode the data. Every sound is also
>>	processed so that it can be efficiently
>>	played with the sound kit.
*/
void	DecodeSounds()
{
				Handle		sinfo;
				Ptr			sdatap;
				long		*infop;
				int			i;

				treenode	**QTable;
	register	treenode	*TheNode;
	register	Ptr			MsgData;

	register	long		bitpos,data;
	register	long		count;
	register	char		delta;
	register	long		len;

				int			sampleframes;
				int			samplepad;

	
	if(OldSound)			/*	The sounds are padded so that they in samplepad-sized packets.	*/
	{	sampleframes=185;	/*	Each packet contains samplesframes bytes of data.				*/
		samplepad=188;
	}
	else
	{	sampleframes=512;	/*	If the sound manager is used, no padding is necessary, but the	*/
		samplepad=512;		/*	sound length has to be a multiple of 512.						*/
	}

	QTable=QuickTable;

	sinfo=GetResource(SKRESTYPE,SKSTABLE);
	NumSounds=GetHandleSize(sinfo)/sizeof(long);
	HLock(sinfo);
	infop=(long *)*sinfo;

	len=0;
	for(i=0;i<NumSounds;i++)
	{	len+=((infop[i]+(sampleframes-1))/sampleframes)*samplepad;
	}
	
	HUnlock(MsgHand);

	SKPtr=NewPtr(len+sizeof(SoundStuff)*(long)NumSounds);
	if(MemErr)
	{	NumSounds=0;
		return;
	}

	HLock(MsgHand);
	MsgData=(sizeof(long)*VALUES)+*MsgHand;

	Sounds=(void *)SKPtr;
	sdatap=SKPtr+sizeof(SoundStuff)*(long)NumSounds;
	
	for(i=0;i<NumSounds;i++)
	{	len=((infop[i]+(sampleframes-1))/sampleframes)*samplepad;
		Sounds[i].Poin=sdatap;
		Sounds[i].Len=len;
		Sounds[i].Count=len/samplepad;
		sdatap+=len;
	}

	delta=128>>DROPBITS;
	bitpos=0;

	for(i=0;i<NumSounds;i++)
	{	sdatap=Sounds[i].Poin;
		len=infop[i];
		count=sampleframes;

		if(CPUFlag<2)		/*	Check for processor type.				*/
		{	while(len--)	/*	Version for 68000 and 68010 processors.	*/
			{
				asm
					{	cmp.b	#16,bitpos
						blt.s	@nobitshift
						sub.b	#16,bitpos
						lea		2(MsgData),MsgData
					@nobitshift
						move.l	(MsgData),data
						move.l	bitpos,D0
						add.b	#QTBITS,D0
						rol.l	D0,data
						and.l	#((1<<QTBITS)-1),data
					}
					TheNode=QTable[data];
	
				if(TheNode->typeflag)
				{	bitpos+=QTBITS;
					asm	{
					@begin0		cmp.b	#16,bitpos
								blt.s	@nobitshifteither
								sub.b	#16,bitpos
								lea		2(MsgData),MsgData
					@nobitshifteither
								move.l	(MsgData),data
								addq.l	#1,bitpos
								rol.l	bitpos,data
								bcc.s	@zeropoint0
							
								move.l	OFFSET(treenode,oneptr)(TheNode),TheNode
								move.w	OFFSET(treenode,typeflag)(TheNode),D0
								bne.s	@begin0
								bra.s	@loopdone0
					@zeropoint0	move.l	OFFSET(treenode,zeroptr)(TheNode),TheNode
								move.w	OFFSET(treenode,typeflag)(TheNode),D0
								bne.s	@begin0
					@loopdone0		
					}
				}
				else
				{	bitpos+=TheNode->codelen;
				}
				delta+=TheNode->value;
				delta&=ANDMASK;
				*sdatap++=delta;
				if(!--count)
				{	count=samplepad-sampleframes;
					while(count--)
						*sdatap++=delta;
	
					count=sampleframes;
				}
			}
			
			if(count!=sampleframes)
			{	count=(count+samplepad-sampleframes) % samplepad;
				while(count--)
				{	*sdatap++=delta;
				}
			}
		}
		else	/*	Version for 68020, 68030, 68040 processors	*/
		{	while(len--)
			{
				asm	68020
					{
					bfextu	(MsgData){bitpos:QTBITS},data
					}
					TheNode=QTable[data];
	
				if(TheNode->typeflag)
				{	bitpos+=QTBITS;
					asm	68020
					{
					@begin
								bfextu	(MsgData){bitpos:1},data
								beq.s	@zeropoint
								move.l	OFFSET(treenode,oneptr)(TheNode),TheNode
								addq.l	#1,bitpos
								move.w	OFFSET(treenode,typeflag)(TheNode),D0
								bne.s	@begin
								bra.s	@loopdone
					@zeropoint	move.l	OFFSET(treenode,zeroptr)(TheNode),TheNode
								addq.l	#1,bitpos
								move.w	OFFSET(treenode,typeflag)(TheNode),D0
								bne.s	@begin
					@loopdone		
					}
				}
				else
				{	bitpos+=TheNode->codelen;
				}
				delta+=TheNode->value;
				delta&=ANDMASK;
				*sdatap++=delta;
				if(!--count)
				{	count=samplepad-sampleframes;
					while(count--)
						*sdatap++=delta;
	
					count=sampleframes;
				}
			}
			
			if(count!=sampleframes)
			{	count=(count+samplepad-sampleframes) % samplepad;
				while(count--)
				{	*sdatap++=delta;
				}
			}
		}
	}
	
	HUnlock(sinfo);
	ReleaseResource(sinfo);
	HUnlock(MsgHand);
	ReleaseResource(MsgHand);
	DisposPtr(QuickTable);	
}
