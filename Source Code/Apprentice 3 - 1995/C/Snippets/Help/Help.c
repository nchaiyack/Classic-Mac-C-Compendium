/* =============================================================
   ======================= Help.c ==============================
   =============================================================
   
   �1994 by Thomas Reed
   Written using Metrowerks CodeWarrior 4
   
   Please do not distribute modified versions of this code.
   
   If you decide to use this code in a program without making significant
   changes, I don't ask for much -- just mention my name in the About box or
   in a Read Me file (since About box real estate can be valuable!  ;-)
   
   To use Help.c, simply add it to your project.  (Don't use #include,
   choose Add... from the Source menu.)
   
   Here's what you need to do to use it:
   
   1)  #include the header "Help.h" in your project.
   
   2)  Put a 'STR#' resource into the project's resource fork.  Then,
       change the kUtilStrings constant to the ID number of this resource.
   
   3)  Insert as the first string in the resource the title you'd like
       displayed in the title bar of the Help window.
       
   4)  Put a 'TEXT' resource into the project's resource, and type in the
       text you want to appear in the Help window's scrollable text field.
       
   5)  If the ID of this 'TEXT' resource isn't 128, change the kHelpTextID
       constant to the correct ID.
       
   6)  When you want the Help window to be displayed, call DoHelp(?).
       DoHelp will handle all the events until the user clicks the Done
       button or presses return.  If you want to play a sound when the
       help window appears, pass it's ID to DoHelp, otherwise pass 0.  Also,
       you must add the following line to the prefix of your program:
       (Choose Options & go to Prefix)
       
         #define USE_HELP_SOUNDS 1
       
       Lastly, you must include my Sounds.c library in your program.
       
       You must also pass a pointer to an update procedure that must be
       defined like this:
       
         void (*UpdateProc) (EventRecord *event)
       
   That's all there is to it!  Hope you like it!
   
   ==============================================================
   
   ============================================================== */

#include "Help.h"

pascal MyAction(ControlHandle myControl, short partCode);

#define NIL  0L
#define MAXLONG 0x7FFFFFFF

#define kEnter		(char) 0x03
#define kReturn		(char) 0x0D

#define kUtilStrings		128
#define kHelpWTitleString	1

#define kHelpTextID			128

#define kHelpSound			128

pascal MyAction(ControlHandle myControl, short partCode)
{
  short			oldCtlVal, newCtlVal, temp, lHeight, i, maxVal;
  long			numLines;
  TEHandle		textH;
  TEStyleHandle	styleH;

  if ((partCode > 0) && (partCode != inThumb))
  {
    textH = (TEHandle) (*myControl)->contrlRfCon;
    styleH = GetStylHandle(textH);
    oldCtlVal = GetCtlValue(myControl);
    switch (partCode)
    {
      case inUpButton: 
        i = oldCtlVal - 1;
        temp = 0;
        if (i < 1)
          i = 1;
        else
          temp = (*(*styleH)->lhTab)[i - 1].lhHeight;
        break;
      case inDownButton: 
        i = oldCtlVal + 1;
        temp = 0;
        if (i > GetCtlMax(myControl))
          i = GetCtlMax(myControl);
        else
          temp = (*(*styleH)->lhTab)[i - 2].lhHeight;
        break;
      case inPageUp: 
        temp = 0;
        i = oldCtlVal - 2;
        while ((i >= 0) && (temp + (*(*styleH)->lhTab)[i].lhHeight <= (*textH)->viewRect.bottom - (*textH)->viewRect.top))
        {
          temp = temp + (*(*styleH)->lhTab)[i].lhHeight;
          i--;
        }
        i = i + 2;
        break;
      case inPageDown: 
        temp = 0;
        i = oldCtlVal - 1;
        maxVal = GetCtlMax(myControl);
        while ((i < maxVal - 1) && (temp + (*(*styleH)->lhTab)[i].lhHeight <= (*textH)->viewRect.bottom - (*textH)->viewRect.top))
        {
          temp = temp + (*(*styleH)->lhTab)[i].lhHeight;
          i++;
        }
        i++;
        break;
    }  /* end of switch */
    SetCtlValue(myControl, i);
    newCtlVal = i;
    lHeight = 0;
    if (oldCtlVal < newCtlVal)
      lHeight = -temp;
    else
      lHeight = temp;
    TEScroll(0, lHeight, textH);
  }  /* end if */
}  /* end MyAction */

void DoHelp(short soundID, void (*UpdateProc) (EventRecord *event))
{
  WindowPtr			helpWindow, whichWindow;
  Rect				tempRect, txRect;
  short				top, left, i, lHeight, temp;
  Handle			myText;
  TEHandle			textH;
  TEStyleHandle		styleH;
  StScrpHandle		styleScrp;
  ControlHandle		scrollControl, buttonControl, whichControl;
  EventRecord		myEvent;
  short				controlPart, oldCtlVal, newCtlVal;
  long				numLines;
  Point				thePt;
  ProcPtr			myProc;
  Boolean			done;
  long				numTicks;
  Str255			title;
  short				whereClicked, radius;
  char				key;
  
  GDHandle			oldDevice;
  CGrafPtr			oldPort;
      
  GetGWorld(&oldPort, &oldDevice);

  myProc = (ProcPtr) &MyAction;
  left = (qd.screenBits.bounds.right - 350) / 2;
  top = (qd.screenBits.bounds.bottom - 250) / 2;
  SetRect(&tempRect, left, top, left + 350, top + 262);
  GetIndString(title, kUtilStrings, kHelpWTitleString);
  helpWindow = NewWindow(NIL, &tempRect, title, TRUE, noGrowDocProc, (WindowPtr) -1, TRUE, 0);
  SetPort(helpWindow);
  myText = GetResource('TEXT', kHelpTextID);
  SetRect(&txRect, 0, 0, qd.thePort->portRect.right - 15, qd.thePort->portRect.bottom);
  textH = TEStylNew(&txRect, &txRect);
  InsetRect(&(*textH)->destRect, 3, 3);
  InsetRect(&(*textH)->viewRect, 3, 3);
  styleScrp = (StScrpHandle) GetResource('styl', kHelpTextID);
  TEStylInsert(*myText, SizeResource(myText), styleScrp, textH);
  styleH = GetStylHandle(textH);
  SetRect(&tempRect, txRect.right, txRect.top - 1, txRect.right + 16, txRect.bottom + 1);
  temp = 0;
  i = 1;
  while ((i - 1 >= 0) && (temp + (*(*styleH)->lhTab)[i - 1].lhHeight <= (*textH)->viewRect.bottom - (*textH)->viewRect.top))
  {
    temp = temp + (*(*styleH)->lhTab)[i - 1].lhHeight;
    i++;
  }
  numLines = i;
  scrollControl = NewControl(helpWindow, &tempRect, "\p", TRUE, 1, 1, (*textH)->nLines - numLines + 2, scrollBarProc, 0);
  (*scrollControl)->contrlRfCon = (long) textH;
  if (i < 0)
    HiliteControl(scrollControl, 255);
  
  #ifdef USE_HELP_SOUNDS
    if (soundID != 0)
    {
      InitChan();
      PlayResFromDisk(soundID, 30);
    }
  #endif

  done = FALSE;
  while (!done)
  {
    #ifdef USE_HELP_SOUNDS
      if (soundID != 0)
        IdleSounds();
    #endif
    if (GetNextEvent(everyEvent, &myEvent))
      switch (myEvent.what)
      {
        case keyDown:
        case autoKey:
          key = myEvent.message & charCodeMask;
          if (((key == 'w') || (key == 'W')) && (myEvent.modifiers & cmdKey))
            done = TRUE;
          else
            SysBeep(1);
          break;
        case mouseDown:
          whereClicked = FindWindow(myEvent.where, &whichWindow);
          if (whichWindow == helpWindow)
            switch (whereClicked)
            {
              case inContent:
                thePt = myEvent.where;
                GlobalToLocal(&thePt);
                controlPart = FindControl(thePt, helpWindow, &whichControl);
                if (controlPart == inThumb)
                {
                  oldCtlVal = GetCtlValue(scrollControl);
                  controlPart = TrackControl(whichControl, thePt, NIL);
                  newCtlVal = GetCtlValue(scrollControl);
                  lHeight = 0;
                  if (oldCtlVal < newCtlVal)
                    for (i = oldCtlVal - 1; i <= newCtlVal - 2; i++)
                      lHeight = lHeight - (*(*styleH)->lhTab)[i].lhHeight;
                  else
                    for (i = newCtlVal - 1; i <= oldCtlVal - 2; i++)
                      lHeight = lHeight + (*(*styleH)->lhTab)[i].lhHeight;
                  TEScroll(0, lHeight, textH);
                }
                else
                  if (controlPart > 0)
                    controlPart = TrackControl(whichControl, thePt, NewControlActionProc(myProc));
                break;
              case inDrag:
                DragWindow(helpWindow, myEvent.where, &qd.screenBits.bounds);
                break;
              case inGoAway:
                if (TrackGoAway(helpWindow, myEvent.where))
                  done = TRUE;
                break;
              default:
                SysBeep(1);
            }  /* end switch */
          break;
        case updateEvt:
          if ((WindowPtr) myEvent.message == helpWindow)
          {
            PenNormal();
            BeginUpdate(helpWindow);
            TEUpdate(&(*textH)->viewRect, textH);
            DrawControls(helpWindow);
            EndUpdate(helpWindow);
          }
          else
            UpdateProc(&myEvent);
          break;
      }  /* end switch */
  }  /* end while */
  DisposeControl(scrollControl);
  TEDispose(textH);
  DisposeWindow(helpWindow);
  ReleaseResource(myText);
  ReleaseResource((Handle) styleScrp);
  
  SetGWorld(oldPort, oldDevice);
  
  #ifdef USE_HELP_SOUNDS
    if (soundID != 0)
      KillSounds();
  #endif
}