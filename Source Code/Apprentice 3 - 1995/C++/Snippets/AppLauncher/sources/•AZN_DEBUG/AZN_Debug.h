#pragma once
/*
*	AZN_Debug.h
*
*	Decl's for utility functions for debugging apps
*	� Andrew Nemeth, Warrimoo Australia, 1995
*
*	File Created:		6 Mar 95
*	File Ammended:		6, 7, 11, 12 Mar 95.
*/

//														uncomment for DEBUG version
//#define	DEBUG


#ifdef	DEBUG

	void			INIT_DEBUG( void );
	void			SALT_MEMORY( void *, long );
	void			myAlert( char *, unsigned );

	#define		DEBUG_TRAP()		Debugger()
	#define		DEBUG_TRAP( )		Debugger()
	
	#define		ASSERT(f)			if (f)		\
									NULL;	\
								else			\
									myAlert( __FILE__, __LINE__ )

#else
//														all debug stuff become no-op's
	#define		INIT_DEBUG()		NULL
	#define		SALT_MEMORY(a,b)	NULL
	#define		DEBUG_TRAP()		NULL
	#define		DEBUG_TRAP( )		NULL
	#define		ASSERT(f)			NULL

#endif