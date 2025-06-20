/*
	LogLibComponent Private.h
	
	Header file for the LogLibComponent.c file.
	
*/

// ensure that this header is only included once...
//#pragma once

// make doubly-sure...
#ifndef __H_LogLibComponent_Private__
#define __H_LogLibComponent_Private__

// include the LogLibComponent.h file if it is not already included
#ifndef __H_LogLibComponent__
#include "LogLibComponent.h"
#endif

#define kLogLibComponentVersion			0x00010000
/* 1.0 (last bit denotes develop=1,alpha=2,beta=3,final=0) */

// so C++ compilers won't choke...
#ifdef __cplusplus
extern "C" {
#endif

// the main entry point (the dispatcher)
#ifdef DEBUGIT
pascal ComponentResult _LogDispatch(ComponentParameters* params,Handle storage);
#else
pascal ComponentResult main(ComponentParameters* params, Handle storage);
#endif

// required routines
pascal ComponentResult _LogOpen(Handle storage,ComponentInstance self);
pascal ComponentResult _LogClose(Handle storage,ComponentInstance self);
pascal ComponentResult _LogRegister(Handle storage,ComponentInstance self);
pascal ComponentResult _LogTarget(Handle storage,ComponentInstance kidnapper);
pascal ComponentResult _LogCanDo(short selector);
pascal ComponentResult _LogVersion(void);

// my normal component routines
pascal ComponentResult _LogStorage(Handle storage);

// log component routines
pascal ComponentResult _LogDefaults(Handle storage,OSType creator,OSType type,Boolean keepOpen);
pascal ComponentResult _LogSetup(Handle storage,StringPtr name,short vref,long dirid);
pascal ComponentResult _LogSetupFSp(Handle storage,FSSpec* fsp);
pascal ComponentResult _LogText(Handle storage,char* buffer);
pascal ComponentResult _LogTime(Handle storage,char* buffer);
pascal ComponentResult _LogPText(Handle storage,unsigned char* buffer);
pascal ComponentResult _LogPTime(Handle storage,unsigned char* buffer);
pascal ComponentResult _LogWrite(Handle storage,long* size,Ptr buffer);

#ifdef __cplusplus
}
#endif

#endif
