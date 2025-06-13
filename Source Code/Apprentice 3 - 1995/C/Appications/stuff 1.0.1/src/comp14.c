/*
 * Copyright (c) 1985, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James A. Woods, derived from original work by Spencer Thomas
 * and Joseph Orost.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "stuff.h"

typedef long int code_int;
typedef long int count_int;
typedef long int cmp_code_int;
typedef unsigned char char_type;
 
enum {maxbits = 14};
char_type inbuf[1024+64];
char_type outbuf[1024+2048];
long bytes_in;
long bytes_out;
count_int *htab;
unsigned short *codetab;

void compress(int fdin, int fdout)
{
register long hp;
int rpos;
int outbits;
int rlop;
int rsize;
int stcode;
code_int free_ent;
int boff;
int n_bits;
int ratio;
long checkpoint;
code_int extcode;
union
{
long code;
struct
{
char_type c;
unsigned short ent;
} e;
} fcode;
if (!htab) htab = (void *)NewPtr(18013*sizeof(count_int));
if (!codetab) codetab = (void *)NewPtr(18013*sizeof(unsigned short));
ratio = 0;
checkpoint = 10000;
extcode = (1L << (n_bits = 9))+1;
stcode = 1;
free_ent = 257;
memset(outbuf, 0, sizeof(outbuf));
bytes_out = 0; bytes_in = 0; 
UpdateProgress(0);
boff = outbits = 0;
fcode.code = 0;
memset(htab, -1, 18013*sizeof(count_int));
do
	{
	long inOutCount = 1024;
	OSErr iErr = FSRead(fdin,&inOutCount,inbuf);
	if (iErr != eofErr) ChkOsErr(iErr);
	rsize = inOutCount;
	if ((rsize ) > 0)
		{
		UpdateProgress(prog_div);
		crc = updcrc(crc, inbuf, rsize);
		if (bytes_in == 0)
			{
			fcode.e.ent = inbuf[0];
			rpos = 1;
			}
		else
			rpos = 0;
		rlop = 0;
		do
			{
			if (free_ent >= extcode && fcode.e.ent < 257)
				{
				if (n_bits < maxbits)
					{
					boff = outbits = (outbits-1)+((n_bits<<3)-((outbits-boff-1+(n_bits<<3))%(n_bits<<3)));
					if (++n_bits < maxbits)
						extcode = (1L << (n_bits))+1;
					else
						extcode = (1L << (n_bits));
					}
				else
					{
					extcode = (1L << (16))+1024;
					stcode = 0;
					}
				}
			if (!stcode && bytes_in >= checkpoint && fcode.e.ent < 257)
				{
				register long int rat;
				checkpoint = bytes_in + 10000;
				if (bytes_in > 0x007fffff)
					{
					rat = (bytes_out+(outbits>>3)) >> 8;
					if (rat == 0)
						rat = 0x7fffffff;
					else
						rat = bytes_in / rat;
					}
				else
					rat = (bytes_in << 8) / (bytes_out+(outbits>>3));
				if (rat >= ratio)
					ratio = (int)rat;
				else
					{
					ratio = 0;
					memset(htab, -1, 18013*sizeof(count_int));
						{ 
						register char_type *p = &(outbuf)[(outbits)>>3]; 
						register long i = ((long)(256))<<((outbits)&0x7); 
						p[0] |= (char_type)(i); 
						p[1] |= (char_type)(i>>8); 
						p[2] |= (char_type)(i>>16); 
						(outbits) += (n_bits); 
						}
					boff = outbits = (outbits-1)+((n_bits<<3)-((outbits-boff-1+(n_bits<<3))%(n_bits<<3)));
					extcode = (1L << (n_bits = 9))+1;
					free_ent = 257;
					stcode = 1;
					}
				}
			if (outbits >= (1024<<3))
				{
				long inOutCount = 1024;
				ChkOsErr(FSWrite(fdout,&inOutCount,outbuf));
				outbits -= (1024<<3);
				boff = -(((1024<<3)-boff)%(n_bits<<3));
				bytes_out += 1024;
				memcpy(outbuf, outbuf+1024, (outbits>>3)+1);
				memset(outbuf+(outbits>>3)+1, '\0', 1024);
				}
				{
				register int i;
				i = rsize-rlop;
				if ((code_int)i > extcode-free_ent) i = (int)(extcode-free_ent);
				if (i > ((sizeof(outbuf) - 32)*8 - outbits)/n_bits)
				i = ((sizeof(outbuf) - 32)*8 - outbits)/n_bits;
				if (!stcode && (long)i > checkpoint-bytes_in)
				i = (int)(checkpoint-bytes_in);
				rlop += i;
				bytes_in += i;
				}
			goto next;
			hfound: fcode.e.ent = codetab[hp];
			next: if (rpos >= rlop)
			goto endlop;
			next2: fcode.e.c = inbuf[rpos++];
				{
				register code_int i;
				hp = (((long)(fcode.e.c)) << (14-8)) ^ (long)(fcode.e.ent);
				if ((i = htab[hp]) == fcode.code)
					goto hfound;
				if (i != -1)
					{
					long disp;
					disp = (18013 - hp)-1;
					do
						{
						if ((hp -= disp) < 0) hp += 18013;
						if ((i = htab[hp]) == fcode.code)
							goto hfound;
						}
					while (i != -1);
				}
			}
			{
			register char_type *p = &(outbuf)[(outbits)>>3]; 
			register long i = ((long)(fcode.e.ent))<<((outbits)&0x7); 
			p[0] |= (char_type)(i); 
			p[1] |= (char_type)(i>>8); 
			p[2] |= (char_type)(i>>16); 
			(outbits) += (n_bits); 
			};
			{
			register long fc;
			fc = fcode.code;
			fcode.e.ent = fcode.e.c;
			if (stcode)
				{
				codetab[hp] = (unsigned short)free_ent++;
				htab[hp] = fc;
				}
			}
		goto next;
		endlop: if (fcode.e.ent >= 257 && rpos < rsize)
		goto next2;
		if (rpos > rlop)
			{
			bytes_in += rpos-rlop;
			rlop = rpos;
			}
		}
	while (rlop < rsize);
	}
}
while (rsize > 0);
if (bytes_in > 0)
	{ 
	register char_type *p = &(outbuf)[(outbits)>>3]; 
	register long i = ((long)(fcode.e.ent))<<((outbits)&0x7); 
	p[0] |= (char_type)(i); 
	p[1] |= (char_type)(i>>8); 
	p[2] |= (char_type)(i>>16); 
	(outbits) += (n_bits); 
	}
	{
	OSErr iErr;
	long inOutCount = (outbits+7)>>3;
	ChkOsErr(FSWrite(fdout,&inOutCount,outbuf));
	bytes_out += (outbits+7)>>3;
	}
UpdateProgress(0);
}

void copy(int fdin, int fdout)
{
long inOutCount;
OSErr iErr;
UpdateProgress(0);
bytes_out = 0;
do
	{
	inOutCount = 32768;
	iErr = FSRead(fdin,&inOutCount,codetab);
	if (iErr != eofErr) ChkOsErr(iErr);
	if (inOutCount > 0)
		{
		ChkOsErr(FSWrite(fdout,&inOutCount,codetab));
		bytes_out += inOutCount;
		UpdateProgress(prog_div<<5);
		}
	}
while (inOutCount > 0);
UpdateProgress(0);
}
