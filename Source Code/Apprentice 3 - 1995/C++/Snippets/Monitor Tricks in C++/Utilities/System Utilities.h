// System Utilities.c
//
// by Kenneth Worley
// Public Domain
//
// Contact me at: America Online: KNEworley
//		internet: KNEworley@aol.com or kworley@fascination.com
//
// Functions that make it easier to get system information.
//

#ifndef System_Utilities_h
#define System_Utilities_h

#ifdef __cplusplus
extern "C" {
#endif

// InitToolbox initializes all the usual Mac toolbox managers.
void InitToolbox( void );

// GetSysVersion is mostly useful to determine what major
// system version is running. It returns 7 if system 7 or later
// is running, 6 if system 6 or later (but not 7) is running,
// and zero if the system version is less than 6.
short	GetSysVersion( void );

// HasColorQuickDraw returns true if the system has color QuickDraw.
Boolean HasColorQuickDraw( void );

#ifdef __cplusplus
}
#endif

#endif // System_Utilities_h