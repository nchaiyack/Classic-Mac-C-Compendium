#ifndef __GRAPHICS_DISPATCH_H__
#define __GRAPHICS_DISPATCH_H__

enum DispatchError		/* return codes from window dispatch procedures */
{
	kSuccess=0,			/* message handled, no further processing please */
	kFailure,			/* message not handled, use default action if any */
	kCancel,			/* message refused, cancel action (only good with kClose) */
	kPassThrough		/* returned by floating window; pass message to front document window */
};

enum DispatchError SetupWindowDispatch(short index, WindowPtr theWindow);
enum DispatchError ShutdownWindowDispatch(short index);
enum DispatchError OpenWindowDispatch(short index);
enum DispatchError CloseWindowDispatch(short index);
enum DispatchError DisposeWindowDispatch(short index);
enum DispatchError ChangeDepthDispatch(short index);
enum DispatchError DrawWindowDispatch(short index, short theDepth);
enum DispatchError CopybitsDispatch(short index, WindowPtr offscreenWindow);
enum DispatchError IdleWindowDispatch(short index, Point mouseLoc);
enum DispatchError ActivateWindowDispatch(short index);
enum DispatchError DeactivateWindowDispatch(short index);
enum DispatchError GrowWindowDispatch(short index);
enum DispatchError ZoomWindowDispatch(short index);
enum DispatchError GetGrowSizeDispatch(short index, Rect *sizeRect);
enum DispatchError KeyDownDispatch(short index, unsigned char theChar);
enum DispatchError MouseDownDispatch(short index, Point thePoint);
enum DispatchError UndoDispatch(short index);
enum DispatchError CutDispatch(short index);
enum DispatchError CopyDispatch(short index);
enum DispatchError PasteDispatch(short index);
enum DispatchError ClearDispatch(short index);
enum DispatchError SelectAllDispatch(short index);
enum DispatchError ReceiveDragDispatch(short index);

#endif
