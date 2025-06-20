//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//|   680x0-specific optimizations
//|
//| This code was created by Greg Ferrar, based on original code
//| created by Jeff Palmer.  There are no restrictions on the use
//| or distribution of this library, except that you may not
//| restrict its use or distribution.  Any project which is
//| created from this code, or any significant modification of
//| this code, may be distributed in any way you choose; you may
//| even sell it or its source code.  However, any limitations
//| placed on the distribution or use of such a project or
//| modification may not in any way limit the distribution or use
//| of this original code.
//|________________________________________________________________



long FixedMult68020(long x, long y)
{

  asm
    {
    move.l	x, d1        ; Put x in register d1
    move.l  y, d0       ; Put y in register d2
    muls.l  d1, d2:d0   ; Compute x*y; put hi in d2, lo in d0
    
    move.w  d2, d0      ; Move hi 16 bits (from lo word of d2) into lo word of d0
    swap    d0          ; Swap halves of d0; this puts lo 16 bits in lo word, and
                        ; hi 16 bits in hi word.
    }

}	/**** FixedMult68020() ****/




long FixedDiv68020(long x, long y)
{

  asm
    {
    move.l	x, d0       ; Put x in register d0 (put hi word of x in hi word of d0)
    swap    d0          ; Move hi word of x to lo word of d0, and vice versa
    move.w  d0, d2      ; Move the lo word of x to the lo word of d2
    ext.l   d2          ; Sign extend d2
    move.l  y, d1       ; Put y in register d1
    divs.l  d1, d2:d0   ; Compute x/y; get quotient in d2
    }

}	/**** FixedDiv68020() ****/

