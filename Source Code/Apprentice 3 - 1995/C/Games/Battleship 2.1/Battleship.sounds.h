#include	<Values.h>
#include	<stdlib.h>
#include	<Sound.h>
#include	<Quickdraw.h>

#define kGridSize					10

#define kBaseResID					128
#define kYouWindowID				kBaseResID
#define kEnemyWindowID				kBaseResID + 1
#define kTextWindowID				kBaseResID + 2
#define kMoveToFront				(WindowPtr)-1L

#define kRandomUpperLimit			32768
#define kRandomRange				kGridSize

#define kBombDropSnd				5000
#define kExplosionSnd				5001
#define kSplashSnd					5002
#define kYouSankSnd					5003
#define kYouWinSnd					5004
#define kYouLoseSnd					5005

#define kNoSound					0
#define kSound						1

#define wYou						kBaseResID
#define wComputer					kBaseResID+1
#define wText						kBaseResID+2

#define kPixelSize					20
#define kNullDotID					144
#define kBWHitDotID					143
#define kHitDotID					145
#define kMissDotID					146
#define kMinSpacedShots				35

#define kHorizDestroyerID			200
#define kVertDestroyerID			201
#define kHorizCruiserID				202
#define kVertCruiserID				203
#define kHorizSubID					204
#define kVertSubID					205
#define kHorizBattleshipID			206
#define kVertBattleshipID			207
#define kHorizCarrierID				208
#define kVertCarrierID				209

#define kCarrierMin					2
#define kCarrierMax					2
#define kBattleshipMin				1
#define kBattleshipMax				2
#define kCruiserMin					1
#define kCruiserMax					1
#define kSubMin						1
#define kSubMax						1
#define kDestroyerMin				0
#define kDestroyerMax				1

#define kCarrierSize				5
#define kBattleshipSize				4
#define kCruiserSize				3
#define kSubSize					3
#define kDestroyerSize				2

#define kNumShips					5
#define kNoShip						-1
#define kBattleship					0
#define kCarrier					1
#define kCruiser					2
#define kSub						3
#define kDestroyer					4

#define mApple						kBaseResID
#define iAbout						1

#define mFile						kBaseResID + 1
#define iNew						1
#define iQuit						2

#define  mOptions					kBaseResID + 3
#define iSound						1

#define kAddCheckMark				TRUE
#define kRemoveCheckMark			FALSE

#define iPlayAgainCheckBox			1
#define iQuitCheckBox				2

#define kExtraPopupPixels			25
	
#define kSpaceBarKey				49

#define	kVersionNumberStr			"\pv2.1"

#define kCongratulationsStr			"\pCongratulations!"
#define kYouWinStr					"\pYou Win!"
#define kSorryStr					"\pSorry."
#define kYouLoseStr					"\pYou Lose."
#define kRotateStr					"\pPress any key to rotate the ship."
#define kPlaceStr					"\pPress the mouse button to place the ship."

/***********/
/* Structs */
/***********/

typedef enum {
	right, left, up, down
} Direction;

typedef enum {
	carrier = 1, battleship, cruiser, sub, destroyer
} ShipsEnum;

typedef enum {
	noTry = 144, hit, miss
} cond;

typedef struct {
	int		shipType;
	int		shipResID;
	cond	hitOrMiss;
} Grid;


/***********/
/* Globals */
/***********/

int			youCount = 0, enemyCount = 0, shotCount = 0;
int			hitShips[kNumShips];
Boolean		gDone, gYouHasGone = FALSE, gHasSound = FALSE, useColor;
Grid		enemy [kGridSize] [kGridSize], you [kGridSize] [kGridSize];
WindowPtr	enemyWindow, youWindow;


/*************/
/* Functions */
/*************/

void	ToolBoxInit ( void );
void	WindowInit ( void );
void	MenuBarInit ( void );
void	EventLoop ( void );
void	DoEvent ( EventRecord *eventPtr );
void	HandleMouseDown ( EventRecord *eventPtr );
void	DoUpdate ( EventRecord *eventPtr );
void	HandleMenuChoice ( long menuChoice );
void	HandleAppleChoice ( short item );
void	HandleFileChoice ( short item );
void	NewGame ( void );
void	DrawDots ( Grid theGrid[][kGridSize] );
void	GridInit ( Grid aGrid[][kGridSize] );
void	PlaceEnemyShips ( void );
void	DrawShipPart ( int resID, Rect pictureRect );
int		Randomize ( void );
void	PlaceAnEnemyShip ( int shipSize, int min, int max, int c );
Boolean	FitsHoriz ( Grid theGrid[][kGridSize], int x, int y, int shipSize );
Boolean	FitsVert ( Grid theGrid[][kGridSize], int x, int y, int shipSize );
void	CheckPoint ( Point mouseLocal );
void	DrawOneDot ( int dotID, Rect *dotRect );
void	PtToDotRect ( Point aPoint, Rect *aRect );
void	PtToRect ( Point aPoint, Rect *aRect );
void	EraseShipPart ( Rect pictureRect );
void	PlaceAShip ( int theMin, int theMax, int theHorizID, int theShip );
void	PtToShipRect ( Point aPoint, Rect *aRect, Boolean horiz, int min, int max );
Boolean	Inbounds ( Point *tempPoint, Boolean horiz, int min, int max );
void	DrawHorizDots ( Point aPoint, Point tempPoint, Rect *theRect, int min, int max );
void	DrawVertDots ( Point aPoint, Point tempPoint, Rect *theRect, int min, int max );
void	SetShipToGrid ( Grid theGrid[][kGridSize], Point tempPoint, 
							Boolean horiz, int min, int max, int c );
Boolean	ItFits ( Boolean horiz, Point thePoint, int theMin, int theMax );
cond	PlaceTheShot ( Point thePoint );
void	PickEnemyShot ( void );
void	HandleOptionsChoice ( short item );
void	HandleSound ( void );
Boolean	IsColour( void );
Boolean	IsBadRandom ( Point thePoint, Boolean canCheck );
void	InitShips ( void );
int		CheckIfSink ( int theShip, Boolean play );


/**********/
/* Macros */
/**********/

pascal OSErr SetDialogDefaultItem(DialogPtr theDialog, short newItem)
		= { 0x303C, 0x0304, 0xAA68 };
pascal OSErr SetDialogCancelItem(DialogPtr theDialog, short newItem)
		= { 0x303C, 0x0305, 0xAA68 };


