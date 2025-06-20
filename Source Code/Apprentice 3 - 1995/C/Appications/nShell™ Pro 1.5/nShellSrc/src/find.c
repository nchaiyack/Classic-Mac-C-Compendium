/* ==========================================

	find.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include <ctype.h>

#include "multi.h"
#include "resource.h"

#include "find.proto.h"
#include "multi.proto.h"
#include "utility.proto.h"
#include "wind.proto.h"

// find dialog items

#define	fdFIND			1		// find button
#define	fdCANCEL		2		// cancel button
#define	fdTEXT			3		// search text item
#define	fdCASE			4		// ignore case check box

Str255	find_str;
char	insensitive;

void find_init( void )
{
	find_str[0] = 0;
	insensitive = 1;
}

int find_ready( void )
{
	return( find_str[0] );  // if the length is non-zero, we have a search string
}

void find( void )
{
	
	DialogPtr	theDialog;
	GrafPtr		SavPort;
	short		itemHit = 0;
	short		itemKind;
	Rect		itemRect;
	Handle		itemHandle;
	Handle		CaseControl;
	char		local_case;

	local_case = insensitive;
	
	GetPort(&SavPort);
	
	theDialog = GetNewDialog( FindDialog, NULL, (WindowPtr) -1 );
	
	if (theDialog) {
		SetCursor(&arrow);
		SetPort( theDialog );
		outline_item(theDialog, fdFIND);			
		GetDItem( theDialog, fdTEXT, &itemKind, &itemHandle, &itemRect );
		SetIText( itemHandle, find_str );
		SelIText( theDialog, fdTEXT, 0, 32767 );
		GetDItem( theDialog, fdCASE, &itemKind, &CaseControl, &itemRect );
		SetCtlValue( (ControlHandle)CaseControl, local_case );
		while ((itemHit != fdFIND) && (itemHit != fdCANCEL)) {	
			ModalDialog( NULL, &itemHit );
			if (itemHit == fdCASE) {
				local_case = !local_case;
				SetCtlValue( (ControlHandle)CaseControl, local_case );
				}
			}
		if (itemHit != fdCANCEL) {
			GetDItem( theDialog, fdTEXT, &itemKind, &itemHandle, &itemRect );
			GetIText( itemHandle, find_str );
			insensitive = local_case;
			}
		DisposDialog( theDialog );	
		}
	else
		error_note("\pmissing dialog resource");
	
	SetPort( SavPort );

	if ((itemHit != fdCANCEL) && (find_str[0]))
		find_next(1);
}

void find_next(int direction)
{
	ShellH			shell;
	CharsHandle		hChars;
	int				i;
	int				found;
	int				start;
	int				length;
	long			window;
	
	if (!find_str[0]) {
		SysBeep(1);
		return;
		}
	
	shell = multi_front();
	
	if (!shell) {
		SysBeep(1);
		return;
		}
		
	start = (**(**shell).Text).selStart;
	length = (**(**shell).Text).teLength;

	hChars = TEGetText( (**shell).Text );
	
	found = 0;
	
	for (i=1; i<length; i++) {
		window = start + (i * direction);
		if (window > length) window -= (length + 1);
		if (window < 0) window += length;
		if (find_match(hChars, length, window)) {
			TESetSelect(window, window+find_str[0], (**shell).Text);
			wind_show_sel(shell);
			found++;
			break;
			}
		}
		
	if (!found) 
		SysBeep(1);
}

int find_match(CharsHandle hChars, int length, long window)
{
	int		i;
	int		pos;
	char	jim,bob;
	
	for (i=0; i<find_str[0]; i++ ) {
		pos = window + i;
		if (pos <= length)
			jim = (**hChars)[pos];
			bob = find_str[i+1];
			if (insensitive) {
				jim = toupper(jim);
				bob = toupper(bob);
				}
			if (jim != bob)
				return(0);
		}
	
	return(1);
}

void find_set( void )
{
	ShellH			shell;
	int				i,start,end,size;
	CharsHandle		hChars;
	
	shell = multi_front();
	
	if (shell) {
	
		start = (**(**shell).Text).selStart;
		end = (**(**shell).Text).selEnd;
		hChars = TEGetText( (**shell).Text );
		
		size = end - start;
		
		if (size > 255) {
			Notify("\pSorry, search strings are limited to 255 characters.",1);
			return;
			}
			
		if (size > 0) {
			find_str[0] = size;
			for (i=0; i < size; i++) find_str[i+1] = (**hChars)[start + i];
			}
		}
}
