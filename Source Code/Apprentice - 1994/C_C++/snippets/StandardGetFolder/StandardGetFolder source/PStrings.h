/* PStrings.h -- A collection of useful pascal string functions. Chris Larson 1993    */
/*               PStringCopy is based on (read: shamelessly stolen from) a remarkably */
/*               similar routine placed into the public domain by Christopher Tate.   */

#ifndef __PSTRINGS__
#define __PSTRINGS__

#ifdef __cplusplus
extern "C" {
#endif

#pragma parameter PStringCopy(__A0,__A1)
void PStringCopy (StringPtr destination, StringPtr source) =
{
    0x7000,         //          moveq   #0, d0      ; clear d0
    0x1011,         //          move.b  (a1), d0    ; init to string length
    0x10D9,         // @x	    move.b  (a1)+,(a0)+	; move a byte
    0x51C8, 0xFFFC  //          dbra    d0, @x	  	; decrement and loop
};

#pragma parameter PStringCat(__A0,__A1)
void PStringCat (StringPtr string, StringPtr tail) =
{
    0x7000,         //      moveq   #0, d0          ; Clear d0
    0x1010,         //      move.b  (a0), d0        ; string size into d0
    0x7200,         //      moveq   #0, d1          ; clear d1
    0x1219,         //      move.b  (a1)+, d1       ; tail length into d1
    0x670C,			//		beq @done				; exit if no tail
    0xD318,         //      add.b   d1,(a0)+        ; total length to string
    0xD0C0,         //      adda.w  d0, a0          ; offset a0 to string end
    0x5341,			//		subq	#1,d1			; correct # of loops
    0x10D9,         //  @x  move.b  (a1)+, (a0)+    ; move a byte
    0x51C9, 0xFFFC  //      dbra    d1, @x          ; decrement and loop
    				//  @done
};

#ifdef __cplusplus
}
#endif

#endif
