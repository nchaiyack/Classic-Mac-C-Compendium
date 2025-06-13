// main.h

void StartGame( void );
void EmptyGrid( void );
void PromptMessage( short messageID );
void ClearMessage( void );
void EventLoop( void );
void HandleMenus( long selection );
void HandleClick( Point *wherePtr );
void RedrawWindow( void );
void StartSelecting( void );
void FinishSelecting( void );
void DrawNewCard( void );
void SelectMultiples( Point *wherePtr );
void PlaceCards( Point *wherePtr );
void DrawCard( Byte card, short horiz, short vert );
void DrawBWCard( Rect *myRectP, short mySuit, short myValue );
void DrawColorCard( Rect *myRectP, short mySuit, short myValue );
void LabelGrid( short horiz, short vert );
void OutlineButton( DialogPtr theDialog, short which );
void DrawSICN( Rect *destRect, Handle SICNhandle, short which );
void Initialize( void );
void RetrieveResources( void );
void Shuffle( void );
short RandomBefore( short what );
void DrawSquare( short x, short y );

#define Empty 0xFF

enum
{
	mApple = 128,
	mFile,
	mEdit,
	iAbout = 1,
	iPlay = 1,
	iHelp,
	iQuit = 4,
	iYes = 1,
	iNo
};

enum
{
	msgSelectMultOf10 = 1,
	msgGameOver,
	msgYouWin
};

enum
{
	Jack = 11,
	Queen,
	King
};