/*
From weber@brand.UUCP (Allan G. Weber) Sat May  3 21:21:20 1986
Path: ut-ngp!ut-sally!im4u!caip!topaz!uwvax!harvard!think!mit-eddie!genrad!decvax!ittatc!dcdwest!sdcsvax!sdcrdcf!usc-oberon!brand!weber
From: weber@brand.UUCP (Allan G. Weber)
Newsgroups: net.sources.mac
Subject: unpit.c again
Message-ID: <207@brand.UUCP>
Date: 4 May 86 02:21:20 GMT
Distribution: na
Organization: U. of So. Calif., Los Angeles
Lines: 438

A couple of people have requested that I send them copies of my program
"unpit" that unpacks Packit-II files (normal or compressed) on a Unix system
so I'm posting it again for anybody else who might find it useful.  A
typical application of this program is for breaking apart a net.sources.mac
file that contains a PackIt'ed executable program and the sources.  After
using "xbin" to get the .data, .rsrc, and .info parts of the Packit file,
run "unpit" on the .data file to break it into its component parts.  You can
then use "tr" to change the carriage returns to new line characters in the
data fork of the sources files, and print it out on your local line printer.
Saves having to download the sources and watch your Imagewriter print it
out.

				Allan Weber
				Weber%Brand@USC-ECL.ARPA
				...sdcrdcf!usc-oberon!brand!weber

*/

/*

		unpit - Macintosh PackIt file unpacker

		Version 2, for PackIt II

This program will unpack a Macintosh PackIt file into separate files.  The
data fork of a PackIt file contains both the data and resource forks of the
packed files.  The program will unpack each Mac file.

Some of the program is borrowed from the macput.c/macget.c programs.

	Author: Allan G. Weber
		Weber%Brand@USC-ECL.ARPA
		 ...sdcrdcf!usc-oberon!brand!weber
	Date:   September 30, 1985
	Revised: January 24, 1986 - added CRC checking
		 March 25, 1986 - support compressed mode of PackIt II,
				  check for illegal Unix file names

*/

/* adapted to become part of suntar: */

/*******************************************************************************\

PackIt extraction module

part of suntar, ©1991-94 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/


/*
Format of a Packit file:

Repeat the following sequence for each file in the Packit file:

	4 byte identifier ("PMag" = not compressed, "Pma4" = compressed)
	variable length compression data (if compressed file)
	92 byte header (see struct pit_header below) *
	2 bytes CRC number *
	data fork (length from header) *
	resource fork (length from header) *
	2 bytes CRC number *

Last file is followed by the 4 byte Ascii string, "Pend", and then the EOF.

* these are in compressed form if compression is on for the file

*/

#include "PB_sync.h"
#include "antiglue.h"

#include "suntar.h"
#include "windows.h"
#include <string.h>

#ifndef NULL
	#define NULL 0L
#endif

/*#define DEBUG*/
#define ASM

typedef short word;

#define H_NAMELEN 63

struct pit_header {			/* Packit file header (92 bytes)
	byte nlen;				/* number of characters in packed file name */
	byte name[H_NAMELEN];	/* name of packed file */
	byte type[4];			/* file type */
	byte creator[4];		/* file creator */
	word flags;	/* File (finder) flags. */
	word lock;	/* File locked if non-zero. */
	long dlen;	/* number of bytes in data fork */
	long rlen;	/* number of bytes in resource fork */
	long ctim;	/* file creation time */
	long mtim;	/* file modified time */
};

#define HDRBYTES  sizeof(struct pit_header) /* 92 */

struct node {
	unsigned char flag, byte;
	struct node *one, *zero;
} *nodelist=NULL, *nodeptr, *read_tree();

static Boolean decode;
static unsigned char more_bits=0;
static unsigned char current_bits;
extern ParamBlockRec pb;
#define ioDirID ioFlNum
extern Boolean devo_chiudere_out;

struct node *read_tree(void);
void read_hdr(void);
void unpit_file(void);
Boolean getcrc(void);
short getbit(void);
short getbyte(void);


void my_unpit()
{
	OSType temp;
	short crc, data_crc;
	extern OSType filecreator,filetype;

	nodelist=NewPtr(512L*sizeof(struct node));	/* 512 is big enough:
		a binary tree with n leaves, where no node has one son, has 2n-1 nodes,
		and n is at most 256 */
	fase=reading_disk;

	while (1) {
		if(readblock(&temp,4)) raise_error();

		if (temp=='PMag' || temp=='PMa4') {
			if (temp=='PMa4') {
				nodeptr = nodelist;
				read_tree();
				decode = 1;
				}
			else
				decode = 0;
			current_crc=0;
			read_hdr();
			printf("File %P (",&macbinh.nlen);
			if(decode) printf("compressed, ");
			printf("data %ld+res %ld bytes)\n",macbinh.dflen,macbinh.rflen);
			unpit_file();

			decode = 0;
			more_bits = 0;	/* flush unused bits */
		}
		else if (temp=='PEnd'){
			deall_tree();
			return;
			}
		else if(temp=='PMa5' || temp=='PMa6' ){
			error_message("Only PackIt III may unpack encrypted files !\n");
			}
		else {
			error_message("Unrecognized Packit file header\n");
		}
	}
}

void deall_tree()
{
if(nodelist!=NULL){
	DisposPtr(nodelist);
	nodelist=NULL;
	}
}

/* This routine recursively reads the compression decoding data.
   It appears to be Huffman compression. See Knuth's Fundamental Algorithms
   for a description of the Huffman tree, and how a prefix visit describes
   a binary tree */
static struct node *read_tree()
{
	struct node *np;
	np = nodeptr++;
	if (getbit() == 1) {
		np->flag = 1;
		np->byte = getbyte();
	}
	else {
		np->flag = 0;
		np->zero = read_tree();
		np->one  = read_tree();
	}
	return(np);
}

static void read_hdr()
{
	get_pit_bytes(&macbinh,74);		/* the MacBinary and PackIt headers are almost
									identical, but MacBinary stores also fdLocation and
									fdFldr, hence I must break the read into two pieces */
	*(long*)&macbinh.finfo.fdLocation=0;
	macbinh.finfo.fdFldr = 0;

	get_pit_bytes(&macbinh.protected,HDRBYTES-74);
/*printf("protected=%d %d\n",macbinh.protected,macbinh.zero);*/
}


void unpit_file()
/* this function is obviously inspired to the untar
module of suntar, by Gail Zacharias and S&G Speranza  */
{
extern short openfile_vrefnum;
extern long openfile_dirID;
extern unsigned char mac_file_name[120];

if (getcrc()) {
	printf("File header CRC mismatch\n");
	if(!ignore_errors) raise_error();
	}
current_crc=0;

pStrcpy(tarh.name,&macbinh.nlen);
my_p2cstr(tarh.name);
full_name=strcpy(nam_buf,tarh.name);
filecreator=filetype='????';

if( ! crea_e_controlla(0) ){

	write_pit_fork(macbinh.dflen);

	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioPermssn = fsWrPerm;
	pb.ioParam.ioMisc = 0;

	if((pb.fileParam.ioDirID=openfile_dirID)!=0){
		pb.ioParam.ioVRefNum=openfile_vrefnum;
		pb.ioParam.ioNamePtr=mac_file_name;
		}
	if( PBHOpenRFSync(&pb)) pbsyserr(&pb);

	devo_chiudere_out=true;

	write_pit_fork(macbinh.rflen);
	if (getcrc()) {
		printf("File data/resource CRC mismatch\n");
		if(!ignore_errors) raise_error();
		}

	setmacbin();
	}
else	/* I should skip it, but I haven't a routine to do that, and to skip
		a compressed file requires to perform the decompression anyway, since
		the compressed size is not explicitly stored */
	raise_error();
}



static Boolean getcrc()
{
	short calc_crc,stored_crc;

	CalcCRC(0);
	CalcCRC(0);
	calc_crc=current_crc;

	stored_crc = getbyte();
	stored_crc=(stored_crc << 8) | getbyte();

	/*printf("calc, letto=%x %x %x\n",calc_crc,stored_crc,current_crc);*/
	return stored_crc!=calc_crc;
}


/* This routine returns the next bit in the input stream (MSB first) */
static short getbit()
{
	if (more_bits == 0) {
		if(readblock(&current_bits,1)) raise_error();
		more_bits = 8;
	}
	more_bits--;
	/*return((current_bits >> more_bits) & 1);*/
	{register unsigned char ret=current_bits;
	current_bits=ret<<1;
	return (ret>>7)&1;
	}
		
}

/* This routine returns the next 8 bits.  If decoding is on, it finds the
byte in the decoding tree based on the bits from the input stream.  If
decoding is not on, it either gets it directly from the input stream or
puts it together from 8 calls to getbit(), depending on whether or not we
are currently on a byte boundary
*/
static short getbyte()
{
	register struct node *np;
	register short i, b;
	if (decode) {
		np = nodelist;
		while (np->flag == 0)
			np = (getbit()) ? np->one : np->zero;
		b = np->byte;
	}
	else {
		if (more_bits == 0){	/* on byte boundary? */
			unsigned char c;
			if(readblock(&c,1)) raise_error();
			b = c;
			}
		else {		/* no, put a byte together */
			b = 0;
			for (i = 8; i > 0; i--) {
				b = (b << 1) + getbit();
			}
		}
	}
	return(b);
}

extern short more_in_bytes;


void get_pit_bytes(buffer,len)
/* by Speranza, 18 Jan 1992
further optimized (with asm instructions too) 5 Sept 1993
 An example of a simple but effective optimization: as in the dehqx module,
move the loop from the caller to the callee, so that register variables 
have a long life, and the stack frame creation/destruction is performed
once rather than a few thousands times, and expand the body of some called
functions to save the overhead of their calls.
 That is, this one is an exception to the universally accepted rule that
"function call overhead is usually negligible".
 The next step is to code the bit extraction in assembly (with a fixed shift
of 1 bit every time) or at least keeping the next bit always in bit 7 rather
than in bit more_bits and expanding the CRC computation
*/
register char *buffer;
register long len;
{
extern unsigned short CRCtable[256];
extern unsigned short current_crc;

/* non-optimized
while(len--){
	CalcCRC(*buffer++=getbyte());
	}
return;
*/


	if(!decode && ! more_bits){
		register unsigned short curr_crc;
		register unsigned short *crc_tab;
		long llen=len; char *bbuf=buffer;
		while(len){
			if(len<512)
				{if(readblock(buffer,len)) raise_error();
				break;
				}
			else{
				if(readblock(buffer,512)) raise_error();
				len-=512; buffer+=512;
				}
			}
		buffer=bbuf;
		#ifdef ASM
			curr_crc=current_crc;
			crc_tab=CRCtable;
			while(llen>0){
				if(llen<65536){
					len=llen-1;
					llen=0;
					}
				else{
					len=65535;
					llen-= 65536;
					}
					/* without the "goto whileloop" the loop is
					executed len+1, and the goto is now useless since is purpose
					was to behave correctly for zero length, the while(llen) now
					provides that */

				/*goto whileloop;*/
			nextloop:
				/*current_crc = ((lowbyte(current_crc)<<8) + (unsigned char)(*buffer++)) ^ CRCtable[highbyte(current_crc)];*/
				asm{
					rol.w	#8,curr_crc	/* scambia i byte */
					moveq.l	#0,d1
					move.b	curr_crc,d1	/* in d1 il byte alto del current_crc */
					move.b	(buffer)+,curr_crc	/* in d0 byte basso<< 8 + current_byte */
					add.w	d1,d1
					move.w	0(crc_tab,d1.w),d1
					eor.w	d1,curr_crc
				whileloop:
					dbra	len,@nextloop
					}
				}
			current_crc=curr_crc;
		#else
		len=llen; 
		while(len--)
			CalcCRC(*buffer++);
		#endif
		}
	else if(!decode){	/* happens only in readtree, which does not call this function,
						but it's safer to handle all cases anyway */
		while(len--)
			CalcCRC(*buffer++ = getbyte());
		}
	else{		/* decode: optimize avoiding to call getbit so many times ! */
		register struct node *np;
		register unsigned char m_bits=more_bits;
		register unsigned char c_b=current_bits;
		register short m_bytes=more_in_bytes;
		register unsigned short curr_crc=current_crc;
		register unsigned short *crc_tab=CRCtable;

		while(len--){
			np = nodelist;		/* copy of the body of getbyte */
		cwhileloop:
			while (!np->flag){
				if (!m_bits) {		/* copy of the body of getbit, with one further
										optimization */
					if(!m_bytes){
						more_in_bytes=0;
						if(readblock(&current_bits,1)) raise_error();
						c_b=current_bits;
						m_bytes=more_in_bytes;
						}
					else
						c_b= disk_buffer[511-(--m_bytes)];
					m_bits = 8;
					}
				m_bits--;
				#ifdef ASM
				asm{
					lsl.b	#1,c_b
					bcs.s	@one
					}
				np=np->zero;
				asm{
					bra.s @cwhileloop	/* inutile saltare alla '{' che  un salto su */
					}
				one:
				np=np->one;
				#else
				/*np = ((current_bits >> m_bits) & 1) ? np->one : np->zero;*/
				np = (c_b &0x80) ? np->one : np->zero;
				c_b<<=1;
				#endif
				}
			#ifndef ASM
			CalcCRC(*buffer++ = np->byte);
			/*printf("byte %d=%d\n",n,buffer[-1]);*/
			#else
			asm{
				rol.w	#8,curr_crc	/* scambia i byte */
				moveq.l	#0,d1
				move.b	curr_crc,d1	/* in d1 il byte alto del current_crc */
				move.b	OFFSET(struct node,byte)(np),curr_crc
				move.b	curr_crc,(buffer)+	/* in d0 byte basso<< 8 + current_byte */
				add.w	d1,d1
				move.w	0(crc_tab,d1.w),d1
				eor.w	d1,curr_crc
				}
			#endif
			}
		more_bits=m_bits;
		current_bits=c_b;
		more_in_bytes=m_bytes;
		current_crc=curr_crc;
		}
}