/*
>>	Dizzy 0.0	elementdefs.h
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
*/

#ifdef BIGENDIAN		/*	You need to define this when necessary! 	*/
#define C4(a,b,c,d) 	((((long)d)<<24L)+(((long)c)<<16L)+(((long)b)<<8L)+a)
#else
#define C4(a,b,c,d) 	((((long)a)<<24L)+(((long)b)<<16L)+(((long)c)<<8L)+d)
#endif

/*
>>	Chip names are 32 bit constants. The interesting thing about them is
>>	that if you look at them as character constants, they spell out element
>>	names. A friend of mine claimed that this was a horrible cludge,
>>	but IMHO, as long as it works, it's ok. It's not all that hard to
>>	rewrite the program to use other constants. Just change the file
>>	reading and writing operations to do conversions.
>>
>>	I'm already over my source code size limit by a factor of 3, so I won't do it.
*/
#define NOT_	C4('N','O','T','_') 	/*	Inverter.					*/
#define AND_	C4('A','N','D','_') 	/*	AND gate.					*/
#define NAND	C4('N','A','N','D') 	/*	NAND gate.					*/
#define OR__	C4('O','R','_','_') 	/*	OR gate.					*/
#define NOR_	C4('N','O','R','_') 	/*	NOR gate.					*/
#define XOR_	C4('X','O','R','_') 	/*	XOR gate.					*/
#define NXOR	C4('N','X','O','R') 	/*	NXOR OR gate. Obscure.		*/
#define INPT	C4('I','N','P','T') 	/*	Input pin or element.		*/
#define OUTP	C4('O','U','T','P') 	/*	Output pin or bit display.	*/
#define HEXD	C4('H','E','X','D') 	/*	Hexadecimal display.		*/
#define D___	C4('D','_','_','_') 	/*	Rising edge D flip flop.	*/
#define JK__	C4('J','K','_','_') 	/*	Rising edge JK flip flop	*/
#define RS__	C4('R','S','_','_') 	/*	Reset/Set flip flop memory. */
#define CUST	C4('C','U','S','T') 	/*	Any file included as custom.*/
#define ONE_	C4('O','N','E','_') 	/*	Constant 1 signal level.	*/
#define ZERO	C4('Z','E','R','O') 	/*	Constant 0 signal level.	*/
#define CONN	C4('C','O','N','N') 	/*	Signal passing joint.		*/
#define ZAPP	C4('Z','A','P','P') 	/*	Food for garbage collector. */
#define CLOK	C4('C','L','O','K') 	/*	Clock chip. 				*/
#define ORIG	C4('O','R','I','G') 	/*	Token for graphics origin.	*/
#define ENDF	C4('E','N','D','.') 	/*	End of file token.			*/
