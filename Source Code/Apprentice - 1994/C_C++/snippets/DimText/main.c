#include <Dialogs.h>
#include <Menus.h>

#include "ToolBoxInit.h"
#include "Dim_text.h"

enum {	// dialog item numbers
	i_done = 1,
	i_title,
	i_checkbox1,
	i_checkbox2,
	i_stat1,
	i_edit1,
	i_edit3,
	i_stat2,
	i_edit2,
	i_refresh
};

void main( void )
{
	DialogPtr	dp;
	ControlHandle	checkbox1, checkbox2;
	Rect			iRect;
	short			iType, hit, val;
	MenuHandle		apple_menu;
	
	ToolBoxInit();
	apple_menu = NewMenu( 128, "\p\x14" );
	AppendMenu( apple_menu, "\pYo;(-" );
	AddResMenu( apple_menu, 'DRVR' );
	InsertMenu( apple_menu, 0 );
	DrawMenuBar();
	
	dp = GetNewDialog( 128, NULL, (WindowPtr)-1L );
	SetPort( dp );
	GetDItem( dp, i_checkbox1, &iType, (Handle *)&checkbox1, &iRect );
	GetDItem( dp, i_checkbox2, &iType, (Handle *)&checkbox2, &iRect );
	Init_dimmer( dp );
	
	ShowWindow( dp );
	do {
		ModalDialog( NULL, &hit );
		switch (hit)
		{
			case i_checkbox1:
				val = !GetCtlValue( checkbox1 );
				SetCtlValue( checkbox1, val );
				Dim_text( dp, i_stat1, val );
				Dim_text( dp, i_edit1, val );
				Dim_text( dp, i_edit3, val );
				break;
				
			case i_checkbox2:
				val = !GetCtlValue( checkbox2 );
				SetCtlValue( checkbox2, val );
				Dim_text( dp, i_stat2, val );
				Dim_text( dp, i_edit2, val );
				break;
			
			case i_refresh:
				EraseRect( &dp->portRect );
				InvalRect( &dp->portRect );
				break;
		}
	} while (hit != i_done);
	
	Dispose_dimmer( dp );
	DisposeDialog( dp );
}
