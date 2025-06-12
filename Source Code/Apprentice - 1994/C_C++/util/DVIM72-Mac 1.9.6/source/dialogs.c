/* external prototypes */
void	Do_update(		WindowPtr  where );
Boolean Cancel_key( EventRecord *theEvent );

/* internal prototypes */
pascal Boolean Dialog_filter( DialogPtr the_dialog,
							EventRecord *the_event,
							int			*item_number );

ControlHandle	Get_Ditem_handle( DialogPtr theDialog, int item_no );
void Flash_button( DialogPtr the_dialog, short item_number );

enum {
	return_char = 0x0D,
	enter_char = 0x03
};

void Flash_button( DialogPtr the_dialog, short item_number )
{
	ControlHandle	item_h;
	long	time;
	short	itype;
	Rect	box;
	
	GetDItem( the_dialog, item_number, &itype, (Handle *)&item_h, &box );
	HiliteControl( item_h, inButton );
	Delay( 9L, &time );
	HiliteControl( item_h, 0 );
}

/* ------------------------- Dialog_filter ------------------------ */
pascal Boolean Dialog_filter( DialogPtr the_dialog,
							EventRecord *the_event,
							int			*item_number )
{
	int		charcode;
	char	the_char;
	
	if ( (the_event->what == updateEvt) &&
		((WindowPtr)the_event->message != the_dialog) )
	{
		Do_update( (WindowPtr)the_event->message );
		return( FALSE );
	}
	
	/* We'll only mess with keyDown events. */
	if (the_event->what != keyDown)
		return( FALSE );
	
	charcode = the_event->message & charCodeMask;
	the_char = (char)charcode;

	if ( (the_char == return_char) || (the_char == enter_char) )
	{
		*item_number = 1;	/* OK */
		Flash_button( the_dialog, *item_number );
		return( TRUE );
	}
	
	if ( Cancel_key( the_event ) )
	{
		*item_number = 2;	/* Cancel */
		Flash_button( the_dialog, *item_number );
		return( TRUE );
	}
	
	/* The following assumes that editing operations apply to disabled
	   editText items.  Thus we don't want ModalDialog to return, so the
	   filter changes the event to a null event and returns FALSE.
	*/
	if (the_event->modifiers & cmdKey)
	{
		switch (charcode)
		{
			case 'x':
				*item_number = ((DialogPeek)the_dialog)->editField + 1; /* current text box */
				DlgCut( the_dialog );
				the_event->what = 0;	/* Change it to a null event */
				return( FALSE );
				break;
			case 'c':
				*item_number = ((DialogPeek)the_dialog)->editField + 1; /* current text box */
				DlgCopy( the_dialog );
				the_event->what = 0;	/* Change it to a null event */
				return( FALSE );
				break;
			case 'v':
				*item_number = ((DialogPeek)the_dialog)->editField + 1; /* current text box */
				DlgPaste( the_dialog );
				the_event->what = 0;	/* Change it to a null event */
				return( FALSE );
				break;
		}
	}
	else
		return( FALSE );	/* ordinary keystroke */
}
