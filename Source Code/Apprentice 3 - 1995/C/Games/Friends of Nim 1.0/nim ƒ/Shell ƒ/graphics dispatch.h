#ifndef __GRAPHICS_DISPATCH_H__
#define __GRAPHICS_DISPATCH_H__

enum DispatchError		/* return codes from window dispatch procedures */
{
	kSuccess=0,			/* message handled, no further processing please */
	kFailure,			/* message not handled, use default action if any */
	kCancel,			/* message refused, cancel action (only good with kClose) */
	kPassThrough		/* returned by floating window; pass message to front document window */
};

typedef short		DispatchError;

#ifdef __cplusplus
extern "C" {
#endif

extern	DispatchError SetupWindowDispatch(short index, WindowRef theWindow);
extern	DispatchError ShutdownWindowDispatch(WindowRef theWindow);
extern	DispatchError OpenWindowDispatch(WindowRef theWindow);
extern	DispatchError CloseWindowDispatch(WindowRef theWindow);
extern	DispatchError DisposeWindowDispatch(WindowRef theWindow);
extern	DispatchError ChangeDepthDispatch(WindowRef theWindow);
extern	DispatchError DrawWindowDispatch(WindowRef theWindow, short theDepth);
extern	DispatchError CopybitsDispatch(WindowRef theWindow, WindowRef offscreenWindow);
extern	DispatchError IdleWindowDispatch(WindowRef theWindow, Point mouseLoc);
extern	DispatchError ActivateWindowDispatch(WindowRef theWindow);
extern	DispatchError DeactivateWindowDispatch(WindowRef theWindow);
extern	DispatchError GrowWindowDispatch(WindowRef theWindow);
extern	DispatchError ZoomWindowDispatch(WindowRef theWindow);
extern	DispatchError GetGrowSizeDispatch(WindowRef theWindow, Rect *sizeRect);
extern	DispatchError KeyDownDispatch(WindowRef theWindow, unsigned char theChar);
extern	DispatchError MouseDownDispatch(WindowRef theWindow, Point thePoint);
extern	DispatchError UndoDispatch(WindowRef theWindow);
extern	DispatchError CutDispatch(WindowRef theWindow);
extern	DispatchError CopyDispatch(WindowRef theWindow);
extern	DispatchError PasteDispatch(WindowRef theWindow);
extern	DispatchError ClearDispatch(WindowRef theWindow);
extern	DispatchError SelectAllDispatch(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
