#include <Memory.h>
#include <Dialogs.h>
#include <GestaltEqu.h>
#include "Dim_text.h"

static pascal void Dim_text_proc( short byteCnt, Ptr textAddr,
		Point numerPt, Point denomPt );

/*	---------------------------------------------------------------------
	Dim_text			This is a group of routines for dimming text
						items in dialogs.  As is, it assumes that you are
						not using the dialog's refCon for anything else,
						and that you are not using the QuickDraw
						bottlenecks for anything else.
	
	This code can be used freely.  I ask that you tell me about any
	improvements that you think of.
	
	James W. Walker		March 17, 1994
	JWWalker@AOL.com
	76367.2271@compuserve.com
	---------------------------------------------------------------------
*/

#define		SYSTEM_6_COMPATIBLE		1

typedef struct Dim_list_el {
	short				item_num;
	struct Dim_list_el	*next;
	Rect				bounds;
	Boolean				dim;
	Boolean				editable;
} Dim_list_el;

typedef struct {
	Dim_list_el	*dim_list;
	QDTextUPP	Old_text_proc;
#if SYSTEM_6_COMPATIBLE
	Boolean		has_gray_text;
	Boolean		has_CountDITL;
#endif
} Dim_data;


/*	---------------------------------------------------------------------
	Get_dim_data		Macro to get the list head.
						Just used to encapsulate the use of the refCon,
						so that if you need to store the list head
						somewhere else you will only need to change this
						and Init_dimmer.
	---------------------------------------------------------------------
*/
#define	Get_dim_data( dp )		((Dim_data *) ((WindowPeek)dp)->refCon)


/*	---------------------------------------------------------------------
	Init_dimmer		Set up a dialog for dimming text.  Call it once, soon
					after creating the dialog.
	---------------------------------------------------------------------
*/
void Init_dimmer( DialogPtr dp )
{
	Dim_data	*dim_data_p;
	QDProcs		*qd_procs;
	long		val;
	
	dim_data_p = (Dim_data *) NewPtrClear( sizeof(Dim_data) );
	if (dim_data_p)
	{
		// Store the pointer where we can find it later
		((WindowPeek)dp)->refCon = (long) dim_data_p;
		
		// Patch the QuickDraw bottleneck for text
		if ( (dp->portBits.rowBytes & 0x8000) == 0 )	// B&W port
		{
			qd_procs = (QDProcs *) NewPtrSysClear( sizeof(QDProcs) );
			SetStdProcs( qd_procs );
		}
		else	// color port
		{
			qd_procs = (QDProcs *) NewPtrSysClear( sizeof(CQDProcs) );
			SetStdCProcs( (CQDProcs *) qd_procs );
		}
		dim_data_p->Old_text_proc = qd_procs->textProc;
		qd_procs->textProc = NewQDTextProc( Dim_text_proc );
		dp->grafProcs = qd_procs;
		
#if SYSTEM_6_COMPATIBLE
		// Which System 7 features are available?
		dim_data_p->has_gray_text =
			(Gestalt( gestaltQuickdrawFeatures, &val ) == noErr)
			&& ( (val & (1L << gestaltHasGrayishTextOr)) != 0 );
		dim_data_p->has_CountDITL =
			(Gestalt( gestaltDITLExtAttr, &val ) == noErr)
			&& ( (val & (1L << gestaltDITLExtPresent)) != 0 );
#endif		

		New_dimmables( dp );
	}
}

/*	---------------------------------------------------------------------
	Dispose_dimmer		Called once after you are through with a dialog.
	---------------------------------------------------------------------
*/
void Dispose_dimmer( DialogPtr dp )
{
	Ptr		maybe_null;
	
	Dispose_dimmables( dp );
	maybe_null = (Ptr) Get_dim_data(dp);
	if (maybe_null)
		DisposePtr( maybe_null );
	if (dp->grafProcs)
	{
		DisposeRoutineDescriptor( qd_procs->textProc );
		DisposePtr( (Ptr) dp->grafProcs );
	}
	dp->grafProcs = NULL;
}

/*	---------------------------------------------------------------------
	Dim_text		Set the dimming state of a text item.
	---------------------------------------------------------------------
*/
void Dim_text( DialogPtr dp, short item, Boolean dim )
{
	Dim_data	*dim_head;
	Dim_list_el	*dimmable;
	Rect		iRect;
	Handle		iHandle;
	short		iType;
	short		disable_flag;
	
	dim_head = Get_dim_data(dp);
	if (dim_head != NULL)
	{
		dimmable = dim_head->dim_list;
		// Try to find the right item number in the list.
		while ( (dimmable != NULL) && (dimmable->item_num != item) )
		{
			dimmable = dimmable->next;
		}
		if (dimmable != NULL)	// found it...
		{
			dimmable->dim = dim;
			GetDItem( dp, item, &iType, &iHandle, &iRect );
			if (dimmable->editable)
			{
				/*
					To dim an editable text item, we need to turn it
					into a static text item, and also take some care
					that it is not showing the insertion point or a
					selection range.
				*/
				disable_flag = iType & itemDisable;
				if (dim)
				{
					TEDeactivate( ((DialogPeek) dp)->textH );
					if (item == ((DialogPeek) dp)->editField + 1 )
					{
						SelIText( dp, item, 0, 0 );
						((DialogPeek) dp)->editField = -1;
					}
					SetDItem( dp, item, statText | disable_flag,
						iHandle, &iRect );
					((DialogPeek) dp)->editField = -1;
					TEActivate( ((DialogPeek) dp)->textH );
					InvalRect( &iRect );
				}
				else
				{
					SetDItem( dp, item, editText | disable_flag,
						iHandle, &iRect );
					SelIText( dp, item, 0, 0 );
					EraseRect( &iRect );
					TEUpdate( &iRect, ((DialogPeek) dp)->textH );
				}
			}
			else
			{
				InvalRect( &iRect );
			}
		}
	}
}


/*	---------------------------------------------------------------------
	Dispose_dimmables		Dispose of the individual dimming items.
							This is called by Dispose_dimmer, so you will
							not ordinarily have to call it directly
							unless you are changing item lists
							dynamically using ShortenDITL and AppendDITL.
	---------------------------------------------------------------------
*/
void Dispose_dimmables( DialogPtr dp )
{
	Dim_data	*dim_head;
	Dim_list_el	*next;
	
	dim_head = Get_dim_data(dp);
	while (dim_head->dim_list != NULL)
	{
		EraseRect( &dim_head->dim_list->bounds );
		next = dim_head->dim_list->next;
		DisposePtr( (Ptr) dim_head->dim_list );
		dim_head->dim_list = next;
	}
}

/*	---------------------------------------------------------------------
	New_dimmables			Makes all text items in the dialog dimmable,
							both static and editable text.  Called by
							Init_dimmer, so you would not normally need
							to call it directly.
							But if you change the item list using
							ShortenDITL and AppendDITL, you would call
							Dispose_dimmables before the change and call
							New_dimmables after the change.
	---------------------------------------------------------------------
*/
void New_dimmables( DialogPtr dp )
{
	Dim_data	*dim_head;
	Dim_list_el	*new_dim;
	Rect		iRect;
	Handle		iHandle;
	short		iType, item, max_item;
	
	dim_head = Get_dim_data(dp);
	if (dim_head != NULL)
	{
#if SYSTEM_6_COMPATIBLE
		if (dim_head->has_CountDITL)
		{
			max_item = CountDITL( dp );
		}
		else
		{
			max_item = (**(short **) (((DialogPeek) dp)->items)) + 1;
		}
#else
		max_item = CountDITL( dp );
#endif
		
		for (item = max_item; item > 0; --item)
		{
			GetDItem( dp, item, &iType, &iHandle, &iRect );
			if ( (iType & (statText | editText)) != 0 )
			{
				new_dim = (Dim_list_el *) 
					NewPtrClear( sizeof(Dim_list_el) );
				if (new_dim)
				{
					new_dim->next = dim_head->dim_list;
					dim_head->dim_list = new_dim;
					new_dim->item_num = item;
					new_dim->editable = (iType & editText) != 0;
					new_dim->bounds = iRect;
					if (new_dim->editable)
					{
						InsetRect( &new_dim->bounds, -3, -3 );
					}
				}
			}
		}
	}
}


/*	---------------------------------------------------------------------
	Dim_text_proc			The QuickDraw bottleneck routine that does
							the actual dimming, and also draws the frame
							around dimmed editable text.
	---------------------------------------------------------------------
*/
static pascal void Dim_text_proc( short byteCnt, Ptr textAddr,
		Point numerPt, Point denomPt )
{
	short			item_num;
	DialogPtr		dp;
	Dim_list_el		*dimmable;
	Dim_data		*dim_head;
	Rect			gray_rect;
	RgnHandle		save_clip;
	PenState		save_pen;
	
	GetPort( &dp );
	dim_head = Get_dim_data( dp );
	item_num = FindDItem( dp, dp->pnLoc ) + 1;
	dimmable = dim_head->dim_list;
	while ( (dimmable != NULL) && (dimmable->item_num != item_num) )
	{
		dimmable = dimmable->next;
	}
	if ( (dimmable != NULL) && (dimmable->dim) )
	{
#if SYSTEM_6_COMPATIBLE
		if (dim_head->has_gray_text)
#endif
		{
			TextMode( grayishTextOr );
		}
		if (dimmable->editable)
		{
			save_clip = NewRgn();
			GetClip( save_clip );
			ClipRect( &dimmable->bounds );
			FrameRect( &dimmable->bounds );
			SetClip( save_clip );
			DisposeRgn( save_clip );
		}
	}

	CallQDTextProc( dim_head->Old_text_proc, byteCnt, textAddr,
		numerPt, denomPt );

#if SYSTEM_6_COMPATIBLE	
	if ( !dim_head->has_gray_text && (dimmable != NULL) && dimmable->dim )
	{
		gray_rect = dimmable->bounds;
		InsetRect( &gray_rect, 1, 1 );
		GetPenState( &save_pen );
		PenMode( patBic );
		/*
			The reason I used a string literal rather than the QuickDraw
			global gray is so that it can be used in a code resource
			without problems.
		*/
		PenPat( (ConstPatternParam) "\xAA\x55\xAA\x55\xAA\x55\xAA\x55" );
		PaintRect( &gray_rect );
		SetPenState( &save_pen );
	}
#endif
}
