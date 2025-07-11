/*
 * Notifier.c
 *
 * Completed on 11/14/93 by Ken Worley using Symantec Think C 6.0.1.
 *
 * Contains the StrFormat and StrNotify routines.  Note that the ANSI, ANSI-small,
 * or ANSI-A4 library must be included in your project for these routines to work.
 *
 */

/*	StrFormat
 *
 * This routine creates a string out of a format string containing type specifiers
 * and an undetermined number of other arguments just like the printf group of
 * library routines.
 *
 * The second argument is a format string just like the kind used with the library
 * routine printf and the following arguments are inserted in the string based on
 * the specifiers in the format string.  For more info on this, see the explanation
 * for the printf routine in the standard libraries.  Besides the extra string
 * argument and the fact that the result is put into a Pascal string, the routines
 * are very alike.  (In fact, the library routine vsprintf is used to format the
 * string which is then transferred to a Pascal string.)  Make sure you do NOT send
 * the address of the Str255 variable.
 */
void	StrFormat( Str255 theString, const char* formatString, ... )
{
	va_list		arguments;
	char		tempString[256];
	short		x, theLength;
	
	va_start( arguments, formatString );
	vsprintf( tempString, formatString, arguments );
	va_end( arguments );

	theLength = strlen( tempString );
	theString[0] = theLength;
	
	if ( theLength )
		for ( x=1; x<=theLength; x++ )
			theString[x] = tempString[x-1];
}

/*
 * StrNotify
 *
 * This routine uses the Notification manager to send a message to the user.  The
 * memory for the NMRec and string are allocated by the calling program and pointers
 * to those items are sent to StrNotify.
 *
 * You must send a pointer to a struct of type NMRec (a notification record) and a
 * POINTER to a string of type Str255.  The memory pointed to by these two arguments
 * must be LOCKED (non-relocatable) and must continue to exist until after the
 * notification is made.  Since there is no mechanism here to determine when the
 * notification occurs, it would be best for the memory to be static or permanently
 * allocated.
 */
void	StrNotify( NMRec *nm, Str255 *str )
{	
	nm->nmMark = 0;
	nm->nmIcon = NULL;
	nm->nmSound = 0;
	nm->nmStr = (StringPtr)str;
	nm->nmResp = (NMUPP)-1L;
	nm->nmRefCon = 0L;
	nm->qType = nmType;
	
	NMInstall( nm );
}

/* EXAMPLE */
/*
 * This code:
 *
 * int		x;
 *	long	l;
 * static NMRec  myNMRec;
 * static Str255 myString;
 *
 * x = 5;
 * y = 250;
 *
 * StrFormat( myString, "The value %ld divided by 50 is %d.", y, x );
 * StrNotify( &myNMRec, &myString );
 *
 * Produces a Notification dialog box containing the following text:
 *
 *     The value 250 divided by 50 is 5.
 */
