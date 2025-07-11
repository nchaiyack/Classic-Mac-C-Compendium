/*
	SpinLib.h

	A beachball cursor spinning library.
*/

#pragma once

#ifndef _H_SpinLib_
#define _H_SpinLib_

#ifdef __cplusplus
extern "C"{
#endif

OSErr SpinInit(void);
OSErr SpinStart(short direction);
OSErr SpinStop(void);
OSErr SpinCleanup(void);
Boolean SpinSpinning(void);

#ifdef __cplusplus
}
#endif

#endif
