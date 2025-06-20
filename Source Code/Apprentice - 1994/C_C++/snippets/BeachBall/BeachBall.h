/******************************************************************

BeachBall.h
Routines for asynchronous spinning beach-ball cursor

******************************************************************/


#ifndef __BEACHBALL_H_
#define __BEACHBALL_H_

#define bbAcurID 		128		//'acur' resource found in BeachBall.rsrc

OSErr StartBeachBall (short acurID, Boolean vblInstall, long vblTimeOut);
void StopBeachBall (void);
void ReverseBeachBall (void);
void SpinBeachBall (void);
void KeepSpinning (long vblTimeOut);

#endif