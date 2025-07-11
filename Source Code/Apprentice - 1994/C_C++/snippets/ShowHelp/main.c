#define REMOVE_ALL_EVENTS 0
#define NIL				0L

void ToolBoxInit( void );
void main( void );

/* ------------------------------------------------------------------ */
void main()
{
	pascal void (*Show_help)( short info_id,
								pascal void (*UpdateProc)(WindowPtr) );
	Handle	help_CUST;
	
	EventRecord	the_event;
	MenuHandle	apple_menu;

	ToolBoxInit();
	
	MaxApplZone();
	EventAvail( everyEvent, &the_event ); /* Move to the foreground */
	EventAvail( everyEvent, &the_event ); /* Move to the foreground */
	EventAvail( everyEvent, &the_event ); /* Move to the foreground */
	
	apple_menu = NewMenu( 1, "\p\024" );
	InsertMenu( apple_menu, 0 );
	DrawMenuBar();
	
	help_CUST = Get1NamedResource( 'CUST', "\pShow_help" );
	HLock( help_CUST );
	Show_help = (pascal void (*)( short, pascal void (*)(WindowPtr) ))
		StripAddress( *help_CUST );
	(*Show_help)( 128, NIL );
}

/* ------------------------------------------------------------------ */
void ToolBoxInit()
{
	InitGraf( &thePort );
	InitFonts();
	FlushEvents( everyEvent, REMOVE_ALL_EVENTS );
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( NIL );
	InitCursor();
}
