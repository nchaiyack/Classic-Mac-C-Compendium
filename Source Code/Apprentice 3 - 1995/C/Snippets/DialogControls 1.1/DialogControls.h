/* These are structures that I use to keep global preferences with only a single
	pointer. */
typedef struct
{
	short		hamType;
	Boolean		lie;
	Boolean		relish;
	Boolean		onion;
	Boolean		ketchup;
	Boolean		mustard;
	Boolean		tomato;
	Boolean		lettuce;
	Boolean		pickle;
} Ham, *HamPtr;

typedef struct
{
	short		price;
	Boolean		relish;
	Boolean		onion;
	Boolean		ketchup;
	Boolean		mustard;
	short		hotType;
} HotDot, *HotDogPtr;

typedef struct
{
	Ham			ham;
	HotDot		hotDog;
} Food, *FoodPtr;


/** Required function prototypes **/
void InitToolBox( void );
void	SetUpGlobals( void );
void	DefaultHotDogSettings( FoodPtr theFood );
void	DefaultHamburgerSettings( FoodPtr theFood );
void	SetupMenus( void );
void	EventLoop( void );
void	HandleMouseDown( void );
void	HandleKeyDown( void );
void	DoThisMenu( long menuCode );
void	InterpretMenu( void );
void	HandleDeskAccessory( int	menuItem );
void 	HandleMenu( int menu, int menuItem );
void	DoAboutMenu( void );
pascal Boolean AboutFilter(DialogPtr inputDialog, EventRecord *myDialogEvent, short *theDialogItem);
void	DoTheMagic( void );
DialogPtr	MyGetNewDialogFont( int	rsrcId, Boolean moveToFront, WindowPtr behindWhich,
					short fontNo, short textSize );
void	SetDialogFontAndSize( DialogPtr theDialog, short fontNo, short fontSize );
void	FrameOptions( DialogPtr dialog );
void	SetUpFoodDitl( DialogPtr window, short standardDITL, short whichFood, FoodPtr newFood );
void	TakeOutCurrentDITL( DialogPtr window, short standardDITL );
void	SelectTheRightStuff( DialogPtr window, short standardDITL, short whichFood, FoodPtr newFood );
void	CheckHotDogItems( DialogPtr window, short whichFood, short standardDITL, FoodPtr newFood );
void	CheckHamburgerItems( DialogPtr window, short whichFood, short standardDITL, FoodPtr newFood );
void	HandleHotDogItems( DialogPtr window, short standardDITL, FoodPtr newFood, short hit );
void	HandleHamburgerItems( DialogPtr window, short standardDITL, FoodPtr newFood, short hit );
void	FindWindowToUpdate( WindowPtr window );