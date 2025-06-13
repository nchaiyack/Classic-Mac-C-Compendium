/**********************************************************************

    Fn_Prototypes.h

***********************************************************************/

#include <AppleEvents.h>

/********** FnAE_Library Prototypes */
extern void   FnAE_InitAE( void );
extern void   FnAE_DoHighLevelEvent(  EventRecord      *theEvent );
extern OSErr  FnAE_GotRequiredParams( const AppleEvent *theEvent );
extern pascal OSErr FnAE_OpenApp(     AppleEvent       *theEvent,
                                      AppleEvent       *reply,
                                      long             refCon );
extern pascal OSErr FnAE_OpenDoc(     AppleEvent       *theEvent,
                                      AppleEvent       *reply,
                                      long             refCon );
extern pascal OSErr FnAE_PrintDoc(    AppleEvent       *theEvent,
                                      AppleEvent       *reply,
                                      long             refCon );
extern pascal OSErr FnAE_Quit(        AppleEvent       *theEvent,
                                      AppleEvent       *reply,
                                      long             refCon );
extern OSErr  FnAE_SendOpenAE(        FSSpec           *theDoc );

/********** FnErr_Library Prototypes */
extern void FnErr_DisplayStrID( int    stringNum,
                                int    quitFlag );
extern void FnErr_DisplayStr(   Str255 s1,
                                Str255 s2,
                                Str255 s3,
                                Str255 s4,
                                int    quitFlag );

/********** FnIO_Library Prototypes */
extern int  FnIO_OpenTextFile  ( TEHandle *te,
                                 /* results */
                                 Str255   fileName,
                                 short    *vRef,
                                 short    *refNum );
extern int  FnIO_SaveTextFile  ( TEHandle *te,
                                 Str255   fileName,
                                 short    *vRef,
                                 short    *refNum );
extern int  FnIO_SaveAsTextFile( TEHandle *te,
                                 Str255   fileName,
                                 /* results */
                                 short    *vRef,
                                 short    *refNum );
extern void FnIO_PageSetup     ( THPrint *pPrintH );
extern void FnIO_PrintTERecord ( TEHandle *te, THPrint *pPrintH );

/********** FnMisc_Library Prototypes */
extern long    FnMisc_ReadPrefs         ( int prefStrID );
extern void    FnMisc_SavePrefs         ( int prefStrID, long value );
extern Boolean FnMisc_ColorAvailability ( void );
extern int	   FnMisc_GetPixelDepth     ( GDHandle theDevice );
extern void    FnMisc_FrameButton       ( DialogPtr theDialog,
                                          short buttonID );
/********** FnTE_Library Prototypes */
extern Boolean FnTE_SetUpTEWindow  ( WindowPtr     w,
                                     Boolean       hasGrowIcon,
                                     Boolean       wordWrap,
                                     short         font,
                                     short         textSize,
                                     short         textInsetPixels,
                                     // results //
                                     TEHandle      *te,
                                     ControlHandle *vScroll );
extern void    FnTE_MaintainCursor ( WindowPtr     w,
                                     TEHandle      *te );
extern void    FnTE_UpdateWindow   ( WindowPtr     w,
                                     TEHandle      *te,
                                     Boolean       hasGrowIcon );
extern void    FnTE_DoActivate     ( WindowPtr     w,
                                     TEHandle      *te,
                                     ControlHandle *vScroll,
                                     Boolean       hasGrowIcon,
                                     Boolean       activate );
extern void    FnTE_DoKeyDown      ( char          theChar,
                                     TEHandle      *te,
                                     ControlHandle *vScroll,
                                     char          *dirty );
extern void    FnTE_DoEditMenu     ( int           item,
                                     TEHandle      *te,
                                     ControlHandle *vScroll,
                                     char          *dirty,
                                     int           cutCommand,
                                     int           copyCommand,
                                     int           pasteCommand,
                                     int           clearCommand,
                                     int           selectAllCommand );
extern void    FnTE_DoContent      ( WindowPtr     w, 
                                     EventRecord   *e,
                                     TEHandle      *te,
                                     ControlHandle *vScroll );
extern void    FnTE_DetSBarIncr    ( TEHandle      *te,
                                     ControlHandle *vScroll );
extern void    FnTE_GrowWindow     ( WindowPtr     w,
                                     TEHandle      *te,
                                     ControlHandle *vScroll,
                                     short         textInsetPixels,
                                     Point         clickLoc );
extern void   FnTE_FrameRect       ( WindowPtr     w,
                                     TEHandle      *te,
                                     short         inset );
extern void   FnTE_GetTEXT         ( short         rsrcID,
                                     TEHandle      *te,
                                     ControlHandle *vScroll );
// End of File