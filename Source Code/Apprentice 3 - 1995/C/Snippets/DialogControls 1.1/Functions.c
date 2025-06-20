#include "Functions.h"

/* Gets the ControlHandle for the item you want in the dialog box thebox.  */
/* Handy for setting checkboxes and radio buttons */
ControlHandle SnatchHandle(DialogPtr thebox, short theGetItem)
{
    short itemtype;
    Rect itemrect;
    Handle thandle;
    
    GetDialogItem(thebox, theGetItem, &itemtype, &thandle, &itemrect);
    return((ControlHandle)thandle);
} /* end SnatchHandle */

/** Sets the parameter text, but usually I only do one of them for dialog
	boxes and stuff **/
void	ParamOne( StringPtr string )
{
	ParamText( string, "\p", "\p", "\p" );
}

/** Selects and unselects buttons based on the argument "which" **/
void	SelectButton(DialogPtr dialog, int butID, Boolean which )
{
	ControlHandle	handle;
	
	handle = SnatchHandle( dialog, butID );

	if( handle )
		SetControlValue( handle, (which?1:0) );
}

/** Checks/Unchecks a button based on it's current state **/
void	CheckButton( DialogPtr dialog, int butID )
{
	ControlHandle	handle;
	
	handle = SnatchHandle( dialog, butID );

	if( handle )
		SetControlValue( handle, !(GetControlValue( handle ) ));
}

/** Enables/Disables a button **/
void	EnableButton( DialogPtr dialog, int butID, Boolean which )
{
	ControlHandle	handle;
	short			how;
	
	if( which )
		how = 0;
	else
		how = 255;
		
	handle = SnatchHandle( dialog, butID );
	
	if( handle )
		HiliteControl( handle, how );
}

/** Tells you whether a button is currently selected or not **/
Boolean	ButtonIsSelected( DialogPtr dialog, int butID )
{
	ControlHandle		handle;
	
	handle = SnatchHandle( dialog, butID );
	
	if( handle )
		return GetControlValue( handle );
	else
		return false;
}

/** Gets the rect of an item in a dialog **/
void	GetDialogItemRect( DialogPtr dialog, int ID, Rect *theRect )
{
   short itemtype;
   Handle thandle;
    
   GetDialogItem(dialog, ID, &itemtype, &thandle, theRect);
}

/** Draws a line around the rect of an item in a dialog **/
void	FrameDialogItemRect( DialogPtr dialog, int ID )
{
	Rect	theRect;
	short	itemtype;
	Handle	theHandle;
	
	GetDialogItem( dialog, ID, &itemtype, &theHandle, &theRect );
	FrameRect( &theRect );
}

/** Enables/Disables menus **/
void		MenuEnable( int  menuID, int  item, Boolean which )
{
	MenuHandle		menu;
	
	menu = GetMenuHandle( menuID );
	
	if( !menu )
	{	
		SysBeep( 10 );
		return;
	}
	
	if( which )
		EnableItem ( menu, item );
	else
		DisableItem( menu, item );
}

/** Checks/Unchecks Menus **/
void 	MenuCheck( int theMenu, short whichItem, Boolean which )
{
	MenuHandle		menu;
	
	menu = GetMenuHandle( theMenu );
	if( !menu )
	{
		SysBeep( 10 );
		ExitToShell();
	}
	
	if( which )
		SetItemMark( menu, whichItem, checkMark);
	else
		SetItemMark( menu, whichItem, noMark );
}

/** Deletes menu items **/
void	MyDeleteMenuItem( int menuID, int menuItem )
{
	MenuHandle		menu;
	
	menu = GetMenuHandle( menuID );
	if( menu )
	{
		DeleteMenuItem( menu, menuItem );
	}
}

/** Gets a resource number from the name of that resource and the type **/
short	ResourceNumberFromName( StringPtr name, ResType type )
{
	Handle		rHandle;
	short		ID;
	
	SetResLoad( false );
	rHandle = GetNamedResource( type, name );
	if( !rHandle ) return 0;
	GetResInfo( rHandle, &ID, &type, name );
	SetResLoad( true );
	return ID;
}

/** Gets the resource name from the ID number **/
void	GetResourceNameFromID( short ID, ResType type, StringPtr name )
{
	Handle		handle;
	
	SetResLoad( false );
	
	handle = GetResource( type, ID );
	if( handle )
	{
		GetResInfo( handle, &ID, &type, name );
	}
	else
	{
		SetString( &name, "\p" );
	}
}

/** Returns the width of a rect **/
int		RectWidth( Rect *rect )
{
	return (rect->right - rect->left );
}

/** Returns the height of a rect **/
int		RectHeight( Rect *rect )
{
	return (rect->bottom - rect->top );
}

/** My own (very stupid) wait function **/
pascal void	Wait( int ticks )
{
	long	oldTicks;
	
	oldTicks = TickCount();
	
	while( oldTicks > TickCount() - (long)ticks )
		SystemTask();
}

/** If you are handling key strokes, this is a good function
	to flash a button to make the user think they pressed it **/
void	FlashButton( DialogPtr dialog, int number )
{
	ControlHandle	handle;

	handle = SnatchHandle( dialog, number );
	if( !handle ) return;

	HiliteControl( handle, kInButtonControlPart );
	Wait( 4 );
	HiliteControl( handle, 0 );
}