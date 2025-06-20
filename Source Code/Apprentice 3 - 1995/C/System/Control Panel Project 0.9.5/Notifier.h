/*
 * Notifier.h
 *
 * Includes files necessary for using the StrFormat, and StrNotify routines.
 */

#ifndef NOTIFIER_H

#include <stdio.h>		/* for vsprintf routine */
#include <stdarg.h>		/* for va_ macros */

#include <Notification.h>

/* PROTOTYPES */
void		StrFormat( Str255 theString, const char* formatString, ... );
void		StrNotify( NMRec *nm, Str255 *str );

#define NOTIFIER_H

#endif
