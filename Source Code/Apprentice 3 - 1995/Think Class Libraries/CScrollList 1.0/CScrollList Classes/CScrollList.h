/******************************************************************************
 CScrollList.h

		Interface for Scroll List class
		
	SUPERCLASS = CPanorama
	
		© 1992 Dave Harkness

******************************************************************************/


#define _H_CScrollList			// include file only once

#include <CPanorama.h>			// include superclass interface


class CScrollPane;
class CMouseTask;
class CArray;
class CStateArray;


enum {
	kDontRedraw = 0,
	kDoRedraw
};

enum {
	kSLSelectable = 1,
	kSLDragable = 2,
	kSLCheckable = 4,
	kSLEditable = 8
};


class CScrollList : public CPanorama
{
public:

	void	IScrollList( CView *anEnclosure, CBureaucrat *aSupervisor,
						 short aWidth, short aHeight,
						 short aHEncl, short aVEncl,
						 SizingOption aHSizing, SizingOption aVSizing,
						 short fOptions);
				
	virtual void	IViewTemp( CView *anEnclosure, CBureaucrat *aSupervisor,
							   Ptr viewData);

	virtual void	SetScrollPane( CScrollPane *aScrollPane);

			// Change size
		
	virtual void	AddCell( short numCells, short afterCell);
	virtual void	DeleteCell( short numCells, short startCell);
	
			// Checking
	
	virtual Boolean		IsChecked( short theCell);
	virtual void		CheckCell( short theCell);
	virtual void		CheckAllCells( Boolean fCheckState);

			// Query
	
	virtual short	GetNumCells( void);
	virtual short	GetCellHeight( void);
	
	virtual short	GetSelectedCell( void);
		
	virtual void	GetCellRect( short theCell, LongRect *cellRect);
	
	virtual Boolean PixelsToCells( LongRect *pixelsRect, short *firstCell, short *lastCell);
	virtual Boolean CellsToPixels( short firstCell, short lastCell, LongRect *pixelsRect);
	
			// Cursor/Clicking
	
	virtual void	AdjustCursor( Point where, RgnHandle mouseRgn );
	virtual void	DoClick( Point hitPt, short modifierKeys, long when);
	virtual void	DoDblClick( short hitCell, short modifierKeys, long when);

	virtual Boolean HitSamePart( Point pointA, Point pointB);
	virtual short	FindHitCell( LongPt *hitPt);

	virtual void	SetDblClickCmd( long aCmd);
	
			// Drawing
	
	virtual void	Draw( Rect *area);

	virtual void	RefreshCell( short aCell);
	virtual void	RefreshCellRange( short firstCell, short lastCell);
	
	virtual void	SetDrawActiveBorder( Boolean fDrawActiveBorder);
	
	virtual void	Activate( void);
	virtual void	Deactivate( void);

			// Typing
	
	virtual void	DoKeyDown( char theChar, Byte keyCode, EventRecord *macEvent);
	virtual void	DoAutoKey( char theChar, Byte keyCode, EventRecord *macEvent);
	
		// Editing Commands
	
	virtual void	DoAddCell( void);
	virtual void	DoAddAnotherCell( void);
	virtual void	DoModifyCell( void);
	virtual void	DoDeleteCell( void);
	virtual void	DoInsertCell( short beforeCell);
	
	virtual Boolean	BecomeGopher( Boolean fBecoming);

			// Selecting
	
	virtual void	SelectCell( short aCell, Boolean reDraw);
	virtual void	ScrollToSelection( void);
	virtual void	FrameCell( short theCell, Boolean frameFlag);
	
			// Array Access
	
	virtual void			SetArray( CArray *anArray);
	virtual CArray			*GetArray( void);
	virtual CStateArray		*GetCheckArray( void);
	
			// Disposal
	
	virtual void	Dispose( void);

protected:

	short			numCells;					// Number of cells in the table
	short			selectedCell;				// Maintains the selected cell
	
	short			cellHeight;					// Height in pixels of each cell
	FontInfo		fontInfo;					// Info of the font used in this list
	Point			indent;						// Indent of each individual cell
	
	long			dblClickCmd;				// Command sent for cell double click
	Boolean			drawActiveBorder;			// Draw a two-pixel thick border?
	short			listOptions;				// All four options for this list
	
	Boolean			fEditing;					// Is a cell being edited?
	Boolean			fStillEditing;				// Is another cell being edited?
	
	CArray			*itsArray;					// Array being displayed in list
	CStateArray		*itsChecks;					// Array to hold checked states
	
			// Initialization
			
	void			IScrollListX( short fOptions);
	virtual void	CreateTextEnvironment( void);
					
	virtual void	AdjustBounds( void);

	virtual CMouseTask	*MakeMouseTask( short modifiers);
	virtual void		ClickOutsideBounds( Point hitPt, short modifierKeys, long when);
	
	virtual void	ProviderChanged( CCollaborator *aProvider, long reason, void* info);	
	
			// Drawing
	
	virtual void	DrawCell( short theCell, Rect *cellRect);
	virtual void	Hilite( short theCell, Boolean hiliteFlag);
	
	virtual void	MakeBorder( void);
	
			// Editing Cells

	virtual void	BeginEditing( void);
	virtual void	SetupCellData( void);
	virtual void	RetrieveCellData( void);
	virtual void	DoneEditing( void);

};


/* Change protocol for CScrollList		*/

enum
{
	scrollListSelectionChanged = bureaucratLastChange + 1,
	
	scrollListLastChange = scrollListSelectionChanged

};

typedef struct
{
	short			prevSelection;
	short			newSelection;
} tSelectionInfo;

typedef struct
{
	PanoramaTemp	panorama;
	long			dblClickCmd;
	short			drawActiveBorder;
	short			listOptions;
} tScrollListTemp, *tScrollListTempP;