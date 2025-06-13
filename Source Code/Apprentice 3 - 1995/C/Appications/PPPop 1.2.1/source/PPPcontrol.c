/*
 *
 * Copyright (c) 1992-1993 Merit Network, Inc. and The Regents of the
 *  University of Michigan.  Usage of this source code is restricted
 *  to non-profit, non-commercial purposes.  The source is provided
 *  "as-is", without warranty.
 */
#include "PPPcontrol.h"


void bzero(b_8 *ptr, short cnt) {
   asm {
      movea.l  ptr,a0      ;get pointer to area to zero
      move.w   cnt,d0      ;and its length in bytes
      bra.s @20
      
@10      clr.b (a0)+    ;clear a byte
@20      dbra  d0, @10
   }
}


/* Returns 1 if up, 0 if not, or -1 if not even inited */

pascal short pppup()
{
LapInfo     *lap;

   if ( Gestalt((OSType) 'PPP ', (long *) &lap) == noErr)
      if (lap != nil)
         if  (lap->ppp_fsm[IPcp].state == fsmOPENED ) 
            return(1);
         else
             return(0);
   return(-1);
}

pascal OSErr pppclose(short hard)
{
LapInfo     *lap;

   if ( Gestalt((OSType) 'PPP ', (long *) &lap) == noErr) {
         if (lap && lap->lapClose && lap->transProc) {
               (*(lap->lapClose))(lap);   /* close PPP */
               if (!hard) {
                  lap->ppp_flags |= CLOSE_PPP;
                  (*(lap->transProc))(TransitionOpen);
               }
               return (0);
         } else
            return(1);
   }
   return(-1);
}

pascal OSErr pppopen ()
{  
#define ipctlGetAddr    15       /* csCode to get our IP address */
               
CntrlParam tiopb;
LapInfo     *lap;
short    refnum;

   if ( Gestalt((OSType) 'PPP ', (long *) &lap) == noErr) {
      if (lap) {
         if (lap->transProc == nil) {
            if (noErr == OpenDriver("\p.IPP",&refnum)) { /* open TCP */
               bzero((b_8 *)&tiopb, sizeof(tiopb));
               tiopb.ioCRefNum = refnum;
               tiopb.csCode = ipctlGetAddr;
               PBControl((ParamBlockRec *) &tiopb, false);
            }
         } else
            (*(lap->transProc))(TransitionOpen);
         return (0);
      }
   }
   return (-1);
}
