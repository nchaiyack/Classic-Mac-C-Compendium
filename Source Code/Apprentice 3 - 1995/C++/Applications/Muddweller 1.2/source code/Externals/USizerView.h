#ifndef  __USizerView__
#define __USizerView__

/*	This unit implements views which can be divided into several resizable panes.
	It also contains commands for resizing and splitting, as well as a splitter Control.
*/

#ifndef  __UMacApp__
#include "UMacApp.h"
#endif

#ifndef  __UArray__
#include "UArray.h"
#endif


			/* Command Numbers */

const short cSizeViews			= 2300; 				/* For resizing panes in TSizerView */

			/* Miscellaneous */

const short kSplitVertically	= v;					/* use with "whichWay" parameter of */
const short kSplitHorizontally	= h;					/* ISizerCommand */

const short kSizerThickness 	= 3;					/* default thickness of splitter bar */
const short kMinSizerPane		= 60;					/* default minimum pane size */

const short kVertSizingCursor	= 1300; 				/* CURSor IDs */
const short kHorzSizingCursor	= 1301;

const Boolean kGetMinCoord		= TRUE;					/* values for min parameter of GetSizerCoord */
const Boolean kGetMaxCoord		= FALSE;

const Boolean kLocationVaries	= TRUE;					/* values for fLocDeterminer fields */
const Boolean kLocationFixed	= FALSE;				/* …of TSplitter */

const unsigned long kStdSplitter = 'splt';				/* resource signature for a TSplitter */
const unsigned long kNoId		= '    ';				/* empty view id */


	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef VRect *VRectPtr;
typedef enum {kLeftCoord, kRightCoord, kTopCoord, kBottomCoord} Coordinate;

class TVRectList : public TDynamicArray {
	public:
			/* This class represents an indexed list of VRects */

	virtual pascal void IVRectList(short initialSize);
				/* Initialize the list, making it big enough to contain initialSize VRects. */

	virtual pascal VRect At(ArrayIndex index);
				/* Return the VRect at the given list index. */

	virtual pascal void AtPut(ArrayIndex index, VRect *newItem);
				/* Set the VRect at the specified index to the given VRect. If index is greater
				  than the number of elements in the list, expand the list accordingly. */

	virtual pascal void AtCoordPut(ArrayIndex index, Coordinate whichCoord, VCoordinate newCoord);
				/* Change a single coordinate of the specified VRect to the given value. */

	virtual pascal void AtSetVRect(ArrayIndex index, VCoordinate left, VCoordinate top,
		VCoordinate right, VCoordinate bottom);
				/* Change the specified VRect’s coordinates to the given values. */

	virtual pascal void InsertBefore(ArrayIndex index, VRect *item);

	virtual pascal void Fields(pascal void (*DoToField)(StringPtr fieldName, Ptr fieldAddr, short 
	   fieldType, void *DoToField_StaticLink), void *DoToField_StaticLink);

	virtual pascal void DynamicFields(pascal void (*DoToField)(StringPtr fieldName, Ptr fieldAddr, short 
	   fieldType, void *DoToField_StaticLink), void *DoToField_StaticLink);

};

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

class TSizerView : public TView {
	public:
			/* This class maintains multiple panes (subviews of itself) with movable
			  sizer rectangles between them. */

	TList *fPanes;				/* the subviews themselves */
	TVRectList *fSetbacks;		/* the setbacks for the subpanes */
	TVRectList *fSizerRects;	/* the sizer rectangles */
	VCoordinate fMinPaneSize;	/* minimum width/height of a pane */
	short fSizerThickness;		/* thickness of splitter */
	struct TSplitter *fSplitter; /* NIL if no splitter */

				/* Initialization methods */

	virtual pascal void Initialize();
				/* Initialize fields to NIL. Called by IObject. */

	virtual pascal void IRes(TDocument *itsDocument, TView *itsSuperView, Ptr *itsParams);
				/* Initialize a view created from templates. */

	virtual pascal void ISizerView(TDocument *itsDocument, TView *itsSuperView, VPoint *itsLocation,
		VPoint *itsSize, SizeDeterminer itsHSizeDet, SizeDeterminer itsVSizeDet);
				/* Initialize a view created procedurally. */

	virtual pascal void InitLists();
				/* Create and initialize the various lists of subviews and VRects */

	virtual pascal void FixupPanes(Boolean equalSpacing);
				/* a/k/a “PostRes”: Call after IRes (NewTemplateWindow).
				  Now that the subviews have been added, use them to initialize the
				  setbacks and sizer rectangles to their default values.
				  If equalSpacing is TRUE, make the panes of equal size;
				  otherwise, try to locate the panes using their fLocations. */

	virtual pascal void Free();
				/* Free the setbacks and sizer list objects */


				/* Adding, drawing, resizing panes */

	virtual pascal void AddPane(TView *newPane, VCoordinate itsLocation, VRect *itsSetbacks);
				/* Install thePane at the specified location, shrinking any existing pane
				  accordingly. */

	virtual pascal void AddEqualPane(TView *newPane, short itsPosition, VRect *itsSetbacks);
				/* Install thePane at the specified position, resizing all panes equally. */

	virtual pascal TView *DeletePane(short whichPane, short whichSizer);
				/* Delete the specified pane and sizer; expand the remaining pane and
				  return it. */

	virtual pascal void Draw(Rect *area);
				/* General draw method: call DrawSizerRect for each sizer rectangle. */

	virtual pascal void DrawSizerRect(Rect *aRect);
				/* Do the direction-dependent drawing: must be overridden. */

	virtual pascal void InstallSetbacks(short whichPane, VRect *itsSetbacks);
				/* Use this method to set different setbacks than the default ones. */
				/* This unit defines three global rects for use in setbacks:
					gVertSBarSetback - Use when installing scroller subviews which
						have only a vertical scrollbar.
					gHorzSBarSetback - Use when installing scroller subviews which
						have only a horizontal scrollbar.
					gBothSBarSetback - Use when installing scroller subviews which
						have horiz AND vert scrollbars.
				  You may also use MacApp's gZeroVRect to indicate that there is to be
				  no setback at all. */
		
	virtual pascal VPoint MergedSize(TView *oldPane, TView *changedPane);
				/* Return the combined size of oldPane and changedPane. Must be overridden. */

	virtual pascal void SetPane(short whichSizer, VRect *itsSizerRect);
				/* The specified sizer has moved, so adjust the panes on either side of it */

	virtual pascal void SetPanes(TVRectList *newSizerRects, Boolean invalidate);
				/* All the sizers have moved, so adjust all the panes in the view */

	virtual pascal void SuperViewChangedSize(VPoint *delta, Boolean invalidate);
				/* Forces commit of any pending pane resize command. Generally this
				  matters only on a window resize which normally would not commit a
				  command; such a resize will force an additional pane resize so
				  "Undo Pane Resize" (or Redo) ceases to make sense.*/


				/* Cursors and mice, etc. */

	virtual pascal struct TCommand *DoMouseCommand(Point *theMouse, EventInfo *info,
		Point * hysteresis);
				/* If the mouse is in a sizer rectangle, create and return a TSizerCommand. */

	virtual pascal Boolean DoSetCursor(Point localPoint, RgnHandle cursorRgn);
				/* Draw the appropriate sizer cursor if necessary.  If the cursor resource
				  is missing, default to the arrow. */

	virtual pascal void InvalidateFocus();
				/* Avoid traversing subviews; just set gFocusedView to NIL */

	virtual pascal void TrackConstrain(VPoint *anchorPoint, VPoint *previousPoint,
													VPoint *nextPoint);
				/* Constrain mouse tracking to my interior, allowing for the minimum pane size.
				  Must be overridden: direction dependent. */


				/* Gettors and settors */

	virtual pascal CompareResult CompareViewLocations(TView *view1, TView *view2);
				/* Return the order of the two views by their location. This calculation
				  is direction dependent, so it must be overridden. */

	virtual pascal short FindPane(TView *aView);
				/* Return the position (index) of aView in the fPanes list */

	virtual pascal TView *FindPaneAt(VPoint *theCoords);
				/* Return the pane at the given coordinates */

	virtual pascal short FindSizerPosition(VCoordinate *itsLocation);
				/* Given the desired coordinate for a new sizer rectangle, return its position
				  (index) in the list of sizers, modifying itsLocation if necessary. */

	virtual pascal VRect GetDefaultSizerRect(short whichSizer);
				/* Return the VRect for the given sizer, assuming evenly spaced sizers. */

	virtual pascal VCoordinate GetMinPaneLength();
				/* Return the minimum width/height of a pane of this view. */

	virtual pascal VRect GetNextSizerRect(TView *aPane);
				/* Must be overridden */

	virtual pascal short GetNumberOfPanes();
				/* How many subviews do I have? */

	virtual pascal short GetNumberOfSizers();
				/* How many sets of sizer bars do I have? (Hint: number of panes - 1) */

	virtual pascal VCoordinate GetSizerCoord(short whichSizer, Boolean min);
				/* Return the left/top (min=T) or right/bottom (min=F) coordinate of the
					specified sizer rect.
				  If whichSizer is larger than the number of sizers, return the width/height
					of the view.
				  If whichSizer is 0, return 0. */

	virtual pascal VRect GetSizerRect(short whichSizer);
				/* Return the specified sizer rectangle. The count starts from the
				  top/left at 1. */

	virtual pascal short GetSizerThickness();
				/* Return the thickness of the pane splitter rectangle. */

	virtual pascal short GetSizingCursor();
				/* Return the resource id of the cursor to use when sizing.
				  Must be overridden. */

	virtual pascal VHSelect GetSplitDirection();
				/* Return the direction of the split: v or h. Defaults to h. */

	virtual pascal short IsPointInSizer(Point localPoint);
				/* Return the number of the sizer rect the localPoint is in, or 0 if none. */

	virtual pascal Boolean IsValidSplitPt(VPoint *aPoint);
				/* Return TRUE if the pane can be split at the given location. */

	virtual pascal VRect MakeSizerRect(VCoordinate itsLocation);
				/* Direction-dependent: must be overridden */

	virtual pascal void SetMinPaneLength(VCoordinate minLength);
				/* Set the minimum width/height of a pane of this view. */

	virtual pascal void SetSizerThickness(short thickness);
				/* Set the thickness of the pane splitter rectangle. */

	virtual pascal void SetSizerRect(short whichSizer, VRect *itsSizerRect);
				/* Change the specified sizerRect and invalidate both the old and new rectangles. */

	virtual pascal void Fields(pascal void (*DoToField)(StringPtr fieldName, Ptr fieldAddr, short 
	   fieldType, void *DoToField_StaticLink), void *DoToField_StaticLink);
};


class THorizontalSizer : public TSizerView {
	public:
			/* This view contains subviews which are stacked one on top of the other. */

	virtual pascal CompareResult CompareViewLocations(TView *view1, TView *view2);
				/* Tell whether view1 is above or below view2, by location. */

	virtual pascal void DrawSizerRect(Rect *aRect);
				/* Draw two parallel horizontal lines to represent the sizer rectangle. */

	virtual pascal VRect GetNextSizerRect(TView *aPane);

	virtual pascal VCoordinate GetSizerCoord(short whichSizer, Boolean min);

	virtual pascal VRect GetSizerRect(short whichSizer);

	virtual pascal short GetSizingCursor();
				/* Return the resource id of the cursor to display when it's over horizontal
				  sizer bars. */

	virtual pascal Boolean IsValidSplitPt(VPoint *aPoint);

	virtual pascal VRect MakeSizerRect(VCoordinate itsLocation);
				/* Return the sizer VRect with the specified top coordinate. */

	virtual pascal void Resize(VCoordinate width, VCoordinate height, Boolean invalidate);
				/* Resize the panes proportionally. */

	virtual pascal void SetPane(short whichSizer, VRect *itsSizerRect);
	
	virtual pascal void SetPanes(TVRectList *newSizerRects, Boolean invalidate);
				/* Direction-dependent resizing. See description in the parent class. */

	virtual pascal void TrackConstrain(VPoint *anchorPoint, VPoint *previousPoint,
													VPoint *nextPoint);
				/* Constrain mouse tracking to my interior, allowing for the minimum pane size */
};


	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

class TSizerCommand : public TCommand {
	public:
		/* This command is for moving existing sizer bars, resizing the panes they separate. */

	TSizerView *fSizerView;			/* view in which to track sizers */
	long fNewEdge;					/* the new sizer rectangle location */
	VRect fOldSizerRect;			/* the old sizer rectangle */
	VHSelect fSplitDir;				/* direction of the split: h or v */
	short fWhichSizer;				/* which sizer we’re tracking, */
									/* …counting from the left or top */

	virtual pascal void ISizerCommand(TSizerView *itsSizerView, short whichSizer,
		VHSelect whichWay);
				/* Add whichWay parameter so command doesn’t have to do a Member test on the
				  sizer view */

	virtual pascal TCommand *TrackMouse(TrackPhase aTrackPhase, VPoint *anchorPoint, VPoint *
		previousPoint, VPoint *nextPoint, Boolean mouseDidMove);

	virtual pascal void TrackFeedback(VPoint *anchorPoint, VPoint *nextPoint, Boolean turnItOn, 
		Boolean mouseDidMove);

	virtual pascal void TrackConstrain(VPoint *anchorPoint, VPoint *previousPoint,
													VPoint *nextPoint);

	virtual pascal void DoIt();
				/* Compute the new sizer rectangle and resize the appropriate panes */

	virtual pascal void UndoIt();
				/* Set sizer rectangle back to its previous location, and resize panes */

	virtual pascal void RedoIt();
				/* DoIt again */

	virtual pascal void SetPenForFeedback(VPoint *aPoint);
				/* Set the pen pattern and size for showing feedback when tracking at the
				  given point */

	virtual pascal void Fields(pascal void (*DoToField)(StringPtr fieldName, Ptr fieldAddr, short 
	   fieldType, void *DoToField_StaticLink), void *DoToField_StaticLink);

};	/* TSizerCommand */

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

class TSplitterCommand : public TSizerCommand {
	public:
			/* This command is for creating new sizer bars using a splitter control. The pane
			  split by the new sizer is cloned, creating a new subview of the SizerView. */

	virtual pascal void ISplitterCommand(TSizerView *itsSizerView);
				/* Set fWhichSizer to 0, so that it tracks over the entire SizerView. */

	virtual pascal void TrackConstrain(VPoint *anchorPoint, VPoint *previousPoint,
		VPoint *nextPoint);
				/* Override to do nothing, so that tracking will be constrained to the SizerView */

	virtual pascal void DoIt();
				/* Create the new pane via cloning and add it to the SizerView. */

	virtual pascal void SetPenForFeedback(VPoint *aPoint);
				/* Show a gray line when tracking outside areas where splitting is allowed. */

	virtual pascal TCommand *TrackMouse(TrackPhase aTrackPhase, VPoint *anchorPoint, VPoint *
		previousPoint, VPoint *nextPoint, Boolean mouseDidMove);
				/* If the pane is not splittable at the mouse-up location, return NIL. */

	virtual pascal void Fields(pascal void (*DoToField)(StringPtr fieldName, Ptr fieldAddr, short 
	   fieldType, void *DoToField_StaticLink), void *DoToField_StaticLink);
};

			
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef enum {kResizeOK, kResizeNotOK, kDeleteSizer} SizerTracking;

class TDeSizerCommand : public TSizerCommand {
	public:
			/* This command is for moving or deleting existing sizer bars.
			  The SizerView must contain a splitter control. */

	VRect fResizeRect;			/* valid area for resizing */
	Boolean fResizeOK;			/* characterize area of fSizerView… */
								/* …we're tracking in */

	virtual pascal void IDeSizerCommand(TSizerView *itsSizerView, short whichSizer,
														  VHSelect whichWay);

	virtual pascal void DoIt();
				/* If final mouse position was at either end of the SizerView,
				  delete the sizer bars; otherwise, just resize. */

	virtual pascal void SetPenForFeedback(VPoint *aPoint);
				/* Set the pen to black, gray, or white, depending on where aPoint is. */

	virtual pascal void TrackConstrain(VPoint *anchorPoint, VPoint *previousPoint,
		VPoint *nextPoint);
				/* Track over the entire SizerView, but keep track of whether nextPoint
				  is a valid spot for the sizer bars to move to. */

	virtual pascal TCommand *TrackMouse(TrackPhase aTrackPhase, VPoint *anchorPoint, VPoint *
		previousPoint, VPoint *nextPoint, Boolean mouseDidMove);
				/* Return NIL if trackRelease is in original sizer rect, or if trackRelease
				  is outside the valid resizing area (but not in the sizer deletion area). */

	virtual pascal void Fields(pascal void (*DoToField)(StringPtr fieldName, Ptr fieldAddr, short 
	   fieldType, void *DoToField_StaticLink), void *DoToField_StaticLink);
};


	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

class TSplitter : public TControl {
	public:
	TSizerView *fSizerView;		/* the view that handles multiple panes */
	Boolean fLocDeterminer[2];	/* T: adjust location coordinate */
								/* …in SuperViewChangedSize */

	virtual pascal void IRes(TDocument *itsDocument, TView *itsSuperView, Ptr *itsParams);

	virtual pascal void ISplitter(TDocument *itsDocument, TView *itsSuperView,
											  VPoint *itsLocation, VPoint *itsSize);

	virtual pascal void IFinish(TView *itsSuperView);
				/* Finish up initialization by setting the fSizerView and fLocDeterminer fields.
				  The latter are determined by the SizerView’s split direction. */

	virtual pascal void Draw(Rect *area);
				/* Default is just a black rectangle, like a splitter well */

	virtual pascal struct TCommand *DoMouseCommand(Point *theMouse, EventInfo *info,
			Point * hysteresis);
				/* Override to return a TSplitterCommand. */

	virtual pascal long GetThickness();
				/* Return the thickness of the splitter control. */

	virtual pascal void SuperViewChangedSize(VPoint *delta, Boolean invalidate);
				/* Adjust the coordinates for which fLocDeterminer = T */

	virtual pascal void Fields(pascal void (*DoToField)(StringPtr fieldName, Ptr fieldAddr, short 
	   fieldType, void *DoToField_StaticLink), void *DoToField_StaticLink);
};


	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

extern pascal VRect gVertSBarSetback;		/* Use when installing scroller subviews which
											have only a vertical scrollbar */
extern pascal VRect gHorzSBarSetback;		/* Use when installing scroller subviews which
											have only a horizontal scrollbar */
extern pascal VRect gBothSBarSetback;		/* Use when installing scroller subviews which
											have horiz AND vert scrollbars */

extern pascal TIntegerArray *gNonPanes;		/* list of ids of classes that may not be panes */


extern pascal void InitUSizerView();
		/* Initializes utility VRects and registers TSizerView */

extern pascal TView *CloneAView(TView *aView);
		/* Clone aView and all its subviews */

extern pascal void ExcludeAsPane(TObject *obj);
		/* Make the class of obj ineligible for pane-dom (i.e., add the class id of obj
		  to the gNonPanes list). */

#endif
