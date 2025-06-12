/* Screen.h */

#ifndef Included_Screen_h
#define Included_Screen_h

/* Screen module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Menus */
/* EventLoop */
/* Scrap */
/* Array */
/* Files */

/* pixel location specifier type */
/* coordinate system:  The global screen coordinates are left-right and top-bottom */
/* with the origin in the upper-left corner of the primary screen. */
typedef short OrdType;

/* limits of the OrdType value */
#define ORDTYPEMIN (-32767)
#define ORDTYPEMAX (32767)

/* window reference type */
struct WinType;
typedef struct WinType WinType;

/* specifies what the window should look like; eDialogWindow has no title */
typedef enum {eDocumentWindow EXECUTE(=-9999),eDialogWindow,eModelessDialogWindow}WinForm;

/* specifies the supported patterns to use when drawing things */
typedef enum {eWhite EXECUTE(=-91), eLightGrey, eMediumGrey, eDarkGrey, eBlack} Patterns;

/* whether there will be a close box on the window */
typedef enum {eWindowNotClosable EXECUTE(= -8888), eWindowClosable} WinCloseType;

/* whether there will be a zoom box or maximize button on the window */
typedef enum {eWindowNotZoomable EXECUTE(= -7777), eWindowZoomable} WinZoomType;

/* whether to let the system resize a window or not */
typedef enum {eWindowNotResizable EXECUTE(= -6666), eWindowResizable} WinSizingType;

/* font characteristics; powers of 2 (for bit twiddling) */
typedef enum {ePlain = 0, eBold = 1, eItalic = 2, eUnderline = 4} FontStyleType;

/* font types */
typedef int FontType;
typedef int FontSizeType;

/* bitmap type */
struct Bitmap;
typedef struct Bitmap Bitmap;

/* Initialize the screen management subsystem.  This routine must be called before */
/* any graphics routines are called.  this routine initializes the entire Level 0 */
/* module set except for some optional modules (like Network).  if it returns  */
/* False then initialization failed and the program must terminate immediately. */
MyBoolean						InitializeScreen(void);

/* close all open windows and perform any cleanup or server disconnection before */
/* the program terminates */
void								ShutdownScreen(void);

/* get size of screen.  If there are multiple screens, the result is implementation */
/* defined, but should not be counted on.  On the Macintosh, this returns only the */
/* size of the main screen.  Caveats aside, you are guarranteed that there is at */
/* least this much screen space in the form of a complete rectangle. */
OrdType							GetScreenHeight(void);
OrdType							GetScreenWidth(void);

/* how big is a window's title bar */
OrdType							WindowTitleBarHeight(WinForm WindowKind);

/* how big are the other edges of a window */
OrdType							WindowOtherEdgeWidths(WinForm WindowKind);

/* create a new window.  if WindowKind = eDocumentWindow, then the window is a */
/* standard window with a name (image is implementation defined).  In this case */
/* Zoomable determines whether there will be a "Maximize" button, and Closable */
/* determines whether there will be a "Close" button. */
/* The window returned will be considered in the "disabled" state and any */
/* objects installed in it should be disabled.  Eventually GetAnEvent will return */
/* an active window change event disabling the window previously on top and */
/* enabling this window. */
WinType*						MakeNewWindow(WinForm WindowKind, WinCloseType Closable,
											WinZoomType Zoomable, WinSizingType Resizing, OrdType Left,
											OrdType Top, OrdType Width, OrdType Height,
											void (*UpdateRoutine)(void* Refcon), void* Refcon);

/* change the size of the window.  The window will be guarranteed to be the specified */
/* size, but significant portions may not be on screen, so be careful */
void								ResizeWindow(WinType* Window, OrdType Width, OrdType Height);

/* close a window and release all associated space.  The window refnum may be reused */
/* An active window change event will be issued activating the window that is */
/* next in the stack */
void								KillWindow(WinType* Window);

/* get the size of the usable portion of the window */
OrdType							GetWindowHeight(WinType* Window);
OrdType							GetWindowWidth(WinType* Window);

/* Get the global coordinate location of the window */
OrdType							GetWindowXStart(WinType* Window);
OrdType							GetWindowYStart(WinType* Window);

/* Adjust the global position of the window. */
void								SetWindowPosition(WinType* Window, OrdType NewXLocation,
											OrdType NewYLocation);

/* Get what type of window it is */
WinForm							GetWindowKind(WinType* Window);

/* allow system to resize window after user clicked in some area */
void								UserGrowWindow(WinType* Window, OrdType X, OrdType Y);

/* bring window to the top of the window stack */
void								ActivateThisWindow(WinType* Window);

/* this routine helps make sure the rectangle fits on the screen.  If the rectangle */
/* already fits on the screen, X and Y will not be adjusted, but if it doesn't, some */
/* undefined adjustment will be made to ensure that the rectangle fits on the screen. */
/* If the rectangle is so large that it can't be made to fit on the screen, then */
/* the size of the window is reduced so that the window will fit on screen. */
void								MakeWindowFitOnScreen(OrdType* X, OrdType* Y,
											OrdType* Width, OrdType* Height);

/* obtain the edge of a window, conforming to the user interface */
/* guidelines of the implementation's platform */
OrdType							AlertLeftEdge(OrdType AlertWidth);
OrdType							AlertTopEdge(OrdType AlertHeight);
OrdType							DialogLeftEdge(OrdType DialogWidth);
OrdType							DialogTopEdge(OrdType DialogHeight);

/* change window's name.  name should be a null-terminated string */
void								SetWindowName(WinType* Window, char* Name);

/* invoke a window's update routine.  normal updates redraw only the part of the */
/* window that needs to be updated.  deferred updates always redraw the entire */
/* window.  they are intended to be used for redrawing a window when it isn't safe */
/* to actually perform the drawing, but they can be used to force a full redraw as */
/* soon as possible. */
void								CallWindowUpdate(WinType* Window);
void								MarkForDeferredUpdate(WinType* Window);
void								PerformDeferredUpdates(void);

/* get the refcon from the window */
void*								GetWindowRefcon(WinType* Window);

/* set the clipping rectangle for the window.  Drawing outside of this rectangle */
/* will not be change any of the window */
void								SetClipRect(WinType* Window, OrdType Left, OrdType Top,
											OrdType Width, OrdType Height);

/* constrain the clipping rectangle for the window.  The new clipping rectangle is */
/* the intersection of the specified one and the previous one. */
void								AddClipRect(WinType* Window, OrdType Left, OrdType Top,
											OrdType Width, OrdType Height);

/* returns True if any part of the specified rectangle in the window is visible. */
/* this is used for making redrawing more efficient. */
MyBoolean						IsRectVisible(WinType* Window, OrdType Left, OrdType Top,
											OrdType Width, OrdType Height);

/* Draw a line one pixel thick.  XDisp and YDisp may be negative. */
void								DrawLine(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp);

/* Draw a box with a 1 pixel thick frame.  Note that the last pixel touched */
/* is X + XDisp - 1 and Y + YDisp - 1. */
void								DrawBoxFrame(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp);

/* paint the box with the specified pattern */
void								DrawBoxPaint(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp);

/* paint the box with white */
void								DrawBoxErase(WinType* Window, OrdType X, OrdType Y,
											OrdType XDisp, OrdType YDisp);

/* And-mask the contents of the box with the pattern */
void								DrawBoxScreen(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp);

/* Draw a box, but round off the corners with circles. */
void								DrawRoundBoxFrame(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp,
											OrdType DiameterX, OrdType DiameterY);
void								DrawRoundBoxPaint(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp,
											OrdType DiameterX, OrdType DiameterY);
void								DrawRoundBoxErase(WinType* Window, OrdType X, OrdType Y,
											OrdType XDisp, OrdType YDisp, OrdType DiameterX, OrdType DiameterY);

/* circles */
void								DrawCircleFrame(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp);
void								DrawCirclePaint(WinType* Window, Patterns Pattern,
											OrdType X, OrdType Y, OrdType XDisp, OrdType YDisp);
void								DrawCircleErase(WinType* Window, OrdType X, OrdType Y,
											OrdType XDisp, OrdType YDisp);

/* fill a triangle */
void								DrawTrianglePaint(WinType* Window, Patterns Pattern, OrdType X1,
											OrdType Y1, OrdType X2, OrdType Y2, OrdType X3, OrdType Y3);

/* Get the ID of a heavier screen font (Macintosh == Chicago) */
FontType						GetUglyFont(void);

/* Get the ID of the default screen font (Macintosh == Geneva) */
FontType						GetScreenFont(void);

/* Get the ID of the normal monospaced font, usually courier or monaco */
FontType						GetMonospacedFont(void);

/* Get the ID of the named font.  If no such font exists, then it is an error */
FontType						GetFontByName(char* Name);

/* Get the total number of pixels high a line is using the specified font */
OrdType							GetFontHeight(FontType FontID, FontSizeType PointSize);

/* return a heap pointer containing the name of the font, null terminated */
/* the heap pointer can be released with ReleasePtr. */
char*								GetNameOfFont(FontType FontID);

/* get number of fonts */
long								GetNumAvailableFonts(void);

/* get the FontType of an indexed font. indices are from 0 to GetNumAvailableFonts - 1 */
FontType						GetIndexedFont(long FontIndex);

/* find the total number of pixels long the string of text is */
OrdType							LengthOfText(FontType Font, FontSizeType PointSize, char* Text,
											long Length, FontStyleType FontStyle);

/* draw a line of text */
void								DrawTextLine(WinType* Window, FontType Font, FontSizeType PointSize,
											char* Text, long Length, OrdType X, OrdType Y,
											FontStyleType FontStyle);

/* draw a line of text, but with white background and black letters */
void								InvertedTextLine(WinType* Window, FontType Font,
											FontSizeType PointSize, char* Text, long Length,
											OrdType X, OrdType Y, FontStyleType FontStyle);

/* move the specified rectangle of of pixels. XDisplacement and YDisplacement */
/* positive mean to the right and down.  Area opened up is erased with white. */
/* no area outside of the rectangle is touched. */
void								ScrollArea(WinType* Window, OrdType Left, OrdType Top, OrdType Width,
											OrdType Height, OrdType XDisplacement, OrdType YDisplacement);

/* convert a raw packed-byte list of data (upper bit of each byte is leftmost */
/* on the screen) to an internal bitmap */
Bitmap*							MakeBitmap(char* RawData, OrdType Width, OrdType Height,
											long BytesPerRow);

/* dispose of the bitmap made by MakeBitmap */
void								DisposeBitmap(Bitmap* TheBitmap);

/* copy the bitmap to the area specified. */
void								DrawBitmap(WinType* Window, OrdType X, OrdType Y, Bitmap* TheBitmap);

/* logical-or the bitmap onto the window */
void								OrBitmap(WinType* Window, OrdType X, OrdType Y, Bitmap* TheBitmap);

/* Bit-clear the bitmap onto the window:  Where the bitmap is set, the */
/* window will be erased; otherwise the window will be untouched */
void								BicBitmap(WinType* Window, OrdType X, OrdType Y, Bitmap* TheBitmap);

/* duplicate the bitmap */
Bitmap*							DuplicateBitmap(Bitmap* Original);

/* logical-or the first bitmap onto the second.  sizes must be the same */
void								BitmapOrIntoBitmap(Bitmap* NotChanged, Bitmap* IsChanged);

/* logical-and the first bitmap onto the second.  sizes must be the same */
void								BitmapAndIntoBitmap(Bitmap* NotChanged, Bitmap* IsChanged);

#endif
