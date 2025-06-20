#include "window layer.h"

static	WindowPtr MyNewWindowDispatch(short layer, Boolean useColor,
				ExtendedWindowPtr storage, const Rect *boundsRect, ConstStr255Param title,
				short procID, Boolean goAwayFlag, long refCon);
static	void SetFrontWindowInLayer(WindowPtr newWindow, WindowPtr oldFrontWindow,
				short layer);
static	WindowPtr GetFrontWindowInLayer(short layer);
static	void SetWindowPrevious(WindowPtr window, WindowPtr previousWindow);
static	WindowPtr GetWindowPrevious(WindowPtr window);
static	void SetWindowNext(WindowPtr window, WindowPtr nextWindow);
static	WindowPtr GetWindowNext(WindowPtr window);
static	void SetWindowLayer(WindowPtr window, short layer);
static	short GetWindowLayer(WindowPtr window);
static	pascal void MyHiliteWindow(WindowPtr theWindow, Boolean fHilite);
static	void ActivateWindow(WindowPtr window, Boolean activate);

enum	/* window layers */
{
	kFloatLayer=0, kDocumentLayer
};

#define NUM_LAYERS		2
#define kMagicNumber	0x16435934

static	WindowPtr			gFrontWindowInLayer[NUM_LAYERS];
static	UniversalProcPtr	gOldHiliteRoutine;

Boolean			gIgnoreNextActivateEvent;

void InitTheWindowLayer(void)
{
	short			i;
	
	for (i=0; i<NUM_LAYERS; i++)
		gFrontWindowInLayer[i]=0L;
	
	InstallHilitePatch();
	gIgnoreNextActivateEvent=FALSE;
}

void ShutDownTheWindowLayer(void)
{
// well, no
}

WindowPtr MyNewWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag, long refCon)
{
	return MyNewWindowDispatch(kDocumentLayer, FALSE, (ExtendedWindowPtr)wStorage, boundsRect,
		title, procID, goAwayFlag, refCon);
}

WindowPtr MyNewCWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag, long refCon)
{
	return MyNewWindowDispatch(kDocumentLayer, TRUE, (ExtendedWindowPtr)wStorage, boundsRect,
		title, procID, goAwayFlag, refCon);
}

WindowPtr MyNewFloatWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag, long refCon)
{
	return MyNewWindowDispatch(kFloatLayer, FALSE, (ExtendedWindowPtr)wStorage, boundsRect,
		title, procID, goAwayFlag, refCon);
}

WindowPtr MyNewFloatCWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag, long refCon)
{
	return MyNewWindowDispatch(kFloatLayer, TRUE, (ExtendedWindowPtr)wStorage, boundsRect,
		title, procID, goAwayFlag, refCon);
}

WindowPtr MyNewWindowDispatch(short layer, Boolean useColor,
	ExtendedWindowPtr storage, const Rect *boundsRect, ConstStr255Param title, short procID,
	Boolean goAwayFlag, long refCon)
{
	WindowPtr			window;
	WindowPtr			oldFrontWindow;
	WindowPtr			behind;
	WindowPtr			iter, nextIter;
	
	behind=(WindowPtr)-1L;
	if (layer==kDocumentLayer)
	{
		iter=GetFrontWindowInLayer(kFloatLayer);
		if (iter!=0L)
		{
			while ((nextIter=GetWindowNext(iter))!=0L)
				iter=nextIter;
			behind=iter;
		}
	}
	
	window=useColor ?
			NewCWindow(storage, boundsRect, title, TRUE, procID, behind, goAwayFlag, refCon) :
			NewWindow(storage, boundsRect, title, TRUE, procID, behind, goAwayFlag, refCon);
	
	oldFrontWindow=GetFrontWindowInLayer(layer);
	RemoveHilitePatch();
	if ((layer==kDocumentLayer) && (oldFrontWindow!=0L))
		HiliteWindow(oldFrontWindow, FALSE);
	HiliteWindow(window, TRUE);
	InstallHilitePatch();
	
	if ((layer==kFloatLayer) && (GetFrontDocumentWindow()!=0L))
		gIgnoreNextActivateEvent=TRUE;
	
	storage->magic=kMagicNumber;		/* so we know it's one of ours */
	SetWindowLayer(window, layer);
	SetFrontWindowInLayer(window, oldFrontWindow, layer);
	
	return window;
}

void MyDisposeWindow(WindowPtr window)
{
	WindowPtr		newFrontWindow;
	short			layer;
	
	if (WindowHasLayer(window))
	{
		layer=GetWindowLayer(window);
		newFrontWindow=GetWindowNext(window);
		if (newFrontWindow!=0L)
			SetWindowPrevious(newFrontWindow, 0L);
		gFrontWindowInLayer[layer]=newFrontWindow;
		CloseWindow(window);
		DisposePtr((Ptr)window);
		window=0L;
		if (newFrontWindow!=0L)
		{
			RemoveHilitePatch();
			HiliteWindow(newFrontWindow, TRUE);
			InstallHilitePatch();
		}
	}
	else
	{
		DisposeWindow(window);
	}
}

Boolean MySelectWindow(WindowPtr window)
{
	WindowPtr		nextWindow, previousWindow, oldFrontWindow;
	short			layer;
	WindowPtr		iter, nextIter;
	
	if (WindowHasLayer(window))
	{
		layer=GetWindowLayer(window);
		oldFrontWindow=GetFrontWindowInLayer(layer);
		if (oldFrontWindow!=window)
		{
			nextWindow=GetWindowNext(window);			/* might be 0L */
			previousWindow=GetWindowPrevious(window);	/* guaranteed !=0L */
			SetWindowNext(previousWindow, nextWindow);
			if (nextWindow!=0L)
				SetWindowPrevious(nextWindow, previousWindow);
			SetFrontWindowInLayer(window, oldFrontWindow, layer);
		}
		
		if ((layer==kDocumentLayer) && ((iter=GetFrontWindowInLayer(kFloatLayer))!=0L))
		{
			while ((nextIter=GetWindowNext(iter))!=0L)
				iter=nextIter;
			SendBehind(window, iter);
		}
		else
		{
			SelectWindow(window);
		}
		
		if (oldFrontWindow==window)
			return FALSE;
		
		RemoveHilitePatch();
		if ((layer==kDocumentLayer) && (oldFrontWindow!=0L))
		{
			HiliteWindow(oldFrontWindow, FALSE);
		}
		HiliteWindow(window, TRUE);
		InstallHilitePatch();
	}
	else
	{
		SelectWindow(window);
	}
	
	return TRUE;
}

void SetFrontWindowInLayer(WindowPtr newWindow, WindowPtr oldFrontWindow,
	short layer)
{
	if (newWindow!=0L)
		SetWindowNext(newWindow, oldFrontWindow);
	if (oldFrontWindow!=0L)
		SetWindowPrevious(oldFrontWindow, newWindow);
	if (newWindow!=0L)
		SetWindowPrevious(newWindow, 0L);
	gFrontWindowInLayer[layer]=newWindow;
}

WindowPtr GetFrontWindowInLayer(short layer)
{
	return gFrontWindowInLayer[layer];
}

void SetWindowPrevious(WindowPtr window, WindowPtr previousWindow)
{
	((ExtendedWindowPtr)window)->previousWindow=previousWindow;
}

WindowPtr GetWindowPrevious(WindowPtr window)
{
	return ((ExtendedWindowPtr)window)->previousWindow;
}

void SetWindowNext(WindowPtr window, WindowPtr nextWindow)
{
	((ExtendedWindowPtr)window)->nextWindow=nextWindow;
}

WindowPtr GetWindowNext(WindowPtr window)
{
	return ((ExtendedWindowPtr)window)->nextWindow;
}

void SetWindowLayer(WindowPtr window, short layer)
{
	((ExtendedWindowPtr)window)->layer=layer;
}

short GetWindowLayer(WindowPtr window)
{
	return ((ExtendedWindowPtr)window)->layer;
}

Boolean WindowHasLayer(WindowPtr window)
{
	if (window==0L)
		return FALSE;
	else
		return ((ExtendedWindowPtr)window)->magic==kMagicNumber;
}

Boolean WindowIsFloat(WindowPtr window)
{
	if (window==0L)
		return FALSE;
	else
		return ((ExtendedWindowPtr)window)->layer==kFloatLayer;
}

WindowPtr GetFrontDocumentWindow(void)
{
	return GetFrontWindowInLayer(kDocumentLayer);
}

WindowPtr GetIndWindowPtr(short index)
{
	WindowPtr		w;
	short			i;
	
	for (i=NUM_LAYERS-1; i>=0; i--)
	{
		w=GetFrontWindowInLayer(i);
		while ((w!=0L) && (GetWindowIndex(w)!=index))
			w=GetWindowNext(w);
		if (w!=0L)
			return w;
	}
	
	return 0L;
}

void InstallHilitePatch(void)
{
	UniversalProcPtr	newAddress;
	
	gOldHiliteRoutine=GetToolTrapAddress(_HiliteWindow);
	newAddress=(UniversalProcPtr)StripAddress(MyHiliteWindow);
	SetToolTrapAddress(newAddress, (short)_HiliteWindow);
}

void RemoveHilitePatch(void)
{
	SetToolTrapAddress(gOldHiliteRoutine, (short)_HiliteWindow);
}

pascal void MyHiliteWindow(WindowPtr theWindow, Boolean fHilite)
{
}
