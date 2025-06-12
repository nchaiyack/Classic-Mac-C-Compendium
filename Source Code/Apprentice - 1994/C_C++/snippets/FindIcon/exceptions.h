/*
	These exception handling macros are based on the article "Living
	in an Exceptional World", by Sean Parent, in _develop_ issue
	11, August 1992.  He used "nrequire" where I use "forbid".
	
	Normally, each require or forbid macro needs to have a unique
	exception-handling label, because of the use of resumeLabel.
	However one could give resumeLabel a null definition and then have
	multiple exceptions sharing handler labels.
*/

#ifdef	DEBUGGING
	#define		DEBUGSTR( x )	DebugStr( "\p" x )
	#define		DEBUGMSG( x )	DebugStr( "\p" # x )
#else
	#define		DEBUGSTR( x )
	#define		DEBUGMSG( x )
#endif


#ifndef resumeLabel
	#define		resumeLabel(exception)	resume_ ## exception:
#endif

#define		require(assertion, exception)			\
				do {								\
					if (assertion) ;				\
					else							\
					{								\
						DEBUGMSG( exception );		\
						goto exception;				\
						resumeLabel(exception);		\
					}								\
				} while (false)

#define		require_action(assertion, exception, action)		\
				do {								\
					if (assertion) ;				\
					else							\
					{								\
						action;						\
						DEBUGMSG( exception );		\
						goto exception;				\
						resumeLabel(exception);		\
					}								\
				} while (false)

#define		forbid(bad_assertion, exception)		\
				do {								\
					if (bad_assertion)				\
					{								\
						DEBUGMSG( exception );		\
						goto exception;				\
						resumeLabel(exception);		\
					}								\
				} while (false)

#define		forbid_action(bad_assertion, exception, action)		\
				do {								\
					if (bad_assertion)				\
					{								\
						action;						\
						DEBUGMSG( exception );		\
						goto exception;				\
						resumeLabel(exception);		\
					}								\
				} while (false)

