/* wrongDLOG.c */

#define kWrongDialogID 	129
#define kMoveToFront	(WindowPtr)-1L
#define iEject				1

void WrongDialog( void );

/* see teck note 304 */
pascal OSErr	SetDialogDefaultItem( DialogPtr theDialog, short newItem )
					= { 0x303C, 0x0304, 0xAA68 };
pascal OSErr	SetDialogCancelItem( DialogPtr theDialog, short newItem )
					= { 0x303C, 0x0305, 0xAA68 };
pascal OSErr	SetDialogTracksCursor( DialogPtr theDialog, Boolean tracks )
					= { 0x303C, 0x0306, 0xAA68 };
					
void WrongDialog( void )
{
	DialogPtr		dialog;
	Boolean			done = false;
	short			itemHit, itemType;
	Handle			okItemHandle, itemHandle;
	Rect			itemRect;
	Str255			itemText;
	StringHandle	strHandle;
	
	dialog = GetNewDialog( kWrongDialogID, nil, kMoveToFront );
	
	ShowWindow( dialog );
	SetPort( dialog );
	
	SetDialogDefaultItem( dialog, iEject );
	SetDialogCancelItem( dialog, iEject );
	
	GetDItem( dialog, iEject, &itemType, &okItemHandle, &itemRect );
	
	while( !done )
	{	
		ModalDialog( nil, &itemHit );
		done = true;
	}
	DisposDialog( dialog );
}