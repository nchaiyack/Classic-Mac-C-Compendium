/*
randU.c
Denis G. Pelli

i=randU();
randU() is the Standard C random number generator rand()�see K&R� modified to
return 16 bits as an unsigned short int instead of just 15 bits as a (positive)
short int. Both versions satisfy Knuth's prescriptions for a linear congruential
random number generator, namely, given that the modulus is 2^32, the multiplier
should be between 0.01m and 0.99m, the multiplier mod 8 should be 5, and the
addend should be odd. (Knuth also recommends doing spectral testing of the
multiplier, and I don't know if that's been done. One would like to think that
the Standard C committee would have chosen a multiplier that had been so tested.)

j=randUL();
randUL() returns a 32-bit random number, formed by calling randU() twice and
gluing the results together.

RandFill(address,bytes);
RandFill is a tight coding of randU() optimized for filling large buffers. It's twice
as fast as making repeated calls to randU(), but is numerically equivalent, and even 
uses the same seed. For high speed it must be compiled to use the 68020 (or better)
chip, otherwise each long multiplication requires a subroutine call. 
Fills 0.8 MB/s on a Mac IIci, i.e. 25 MHz 68030.

Kernighan, B. W. and Ritchie, D. M. (1988) The C Programming Language, Second
Ed. Englewood Cliffs, NJ: Prentice Hall, p. 46.

Knuth, D. E. (1981) The Art of Computer Programming: 2. Seminumerical
Algorithms, Second Edition.  Reading, MA: Addison Wesley. pp. 170-171.

HISTORY:
8/4/89	dgp	wrote it.
3/19/90	dgp	eliminated assembly code to make it portable. 
8/5/91	dgp	added RandFill(). 
8/24/91	dgp	Made compatible with THINK C 5.0.
10/21/91 dgp Removed obsolete inclusion of MacProto.h.
9/13/92	dgp	Added randUL().
*/
#include "VideoToolbox.h"

typedef union {
	unsigned long L;
	struct {
		short high;
		short low;
	} S;
} seedType;

static seedType seed={314159265};

unsigned short randU(void)
{
	seed.L = seed.L * 1103515245L + 12345L;
	return seed.S.high;
}

#if USHRT_MAX!=0xffff || ULONG_MAX!=0xffffffff
	#error "randUL() assumes 16 bit unsigned short and 32 bit unsigned long"
#endif

unsigned long randUL(void)
{
	return ((unsigned long)randU()<<16)+randU();
}

/*	srandU - seed pseudo-random number generator	*/

void srandU(unsigned n)
{
	seed.L = n;
}

/*
RandFill uses exactly the same algorithm as randU, using the same seed, but is
coded to fill a large buffer quickly.

The trick here is to avoid having to do a bit shift to get at the high word of
the s register. I do this by moving the entire long register to memory,
overwriting the least significant two bytes on the next iteration. As a result
this method requires a 2 byte overhang beyond the end of the desired data. That
overhang region and an odd byte, if any, is filled in by copying from a
workArea.

A simpler way to implement this overhang business would have been to copy the
memory that was to be overwritten, overfill the buffer, and then restore the
overwritten memory. The problem with that method is that it assumes the
overwritten memory exists (e.g. it might be video memory) and won't be used
while we're running, which is not an entirely safe assumption unless we turn off
interrupts. So I took the cautious approach, which makes for a messier looking
program, but it's safe. The performance cost is negligible.
*/
void RandFill(void *address,long bytes)
{
	register long i;
	register unsigned long s,mul,add;
	register unsigned short *ptr;
	unsigned short *savePtr,workArea[3];

	s=seed.L;
	mul=1103515245L;
	add=12345L;
	ptr=(unsigned short *)address;
	i=bytes;
	i-=2;					/* allow guard room for overshoot */
	/* Because of the unrolling, the loop overhead is only 3% of running time */
	for(;i>=32;i-=32) {
		/* this compiles to 4 instructions: MULU.L, ADD.L, MOVE.L, ADDQ.L */
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
	}
	for(;i>=2;i-=2) {
		s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
	}
	i+=2;					/* remove guard */
	savePtr=ptr;
	ptr=&workArea[0];
	s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
	s *= mul; s += add; *(unsigned long *)ptr=s; ptr++;
	memcpy(savePtr,workArea,i);
	seed.L=s;
}