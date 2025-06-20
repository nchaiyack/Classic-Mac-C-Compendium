/****
 * CHyperCuberDoc.c
 *
 *	Document methods for a typical application.
 *
 *  Copyright � 1990 Symantec Corporation.  All rights reserved.
 *
 ****/

#include "CControlsDirector.h"
#include "CEnhancedWindow.h"
#include "CHyperCuberDoc.h"
#include "CHyperCuberPane.h"
#include "CHyperCuberPrefs.h"
#include "CGraphic.h"

#include <CSizeBox.h>

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"

#include <string.h>
#include <stdio.h>
#include <fstream.h>
#include <strstrea.h>

#define	WINDHyperCuber		500		/* Resource ID for WIND template */

//===================================== Globals =====================================\\

extern CApplication 		*gApplication;	/* The application */
extern CBartender			*gBartender;	/* The menu handling object */
extern CClipboard			*gClipboard;	/* The clipboard */
extern CDesktop				*gDesktop;		/* The enclosure for all windows */
extern CBureaucrat			*gGopher;		/* The current boss in the chain of command */
extern OSType				gSignature;		/* The application's signature */
extern CError				*gError;		/* The global error handler */

extern CHyperCuberPrefs		*gPrefs;		//  The preferences

extern Boolean				menubar_hidden;	//  TRUE if the menubar is currently hidden
extern short				menubar_height;	//  The height of the menubar (if any)

extern Boolean				drawing_disabled;//  TRUE if drawing is disabled


//=============================== Procedure Prototypes ===============================\\

extern void verify_window_rect (Rect *bounds);
extern "C" void show_menubar(void);
extern "C" void hide_menubar(void);
extern void CreateNCube(long n, char *filename);
extern void adjust_offscreen_pixmap(CGrafPort *color_port, Rect *bounds);
void HideControlDirector(CObject *director);
void ShowControlDirector(CObject *director);


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::IHyperCuberDoc
//| 
//| Purpose: Initialize a HyperCuberdocument.
//|
//| Parameters: superclass parameters
//|_________________________________________________________

void CHyperCuberDoc::IHyperCuberDoc(CApplication *aSupervisor, Boolean printable)
{

	CDocument::IDocument(aSupervisor, printable);	//  Initialize this as a Document

	graphic = (CGraphic *) new(CGraphic);			//  Create a Graphic
	((CGraphic*) graphic)->IGraphic();				//  Initialize the Graphic
								
	controls_directors = new(CList);				//  Set up the controls list
	controls_directors->IList();
	
	BecomeGopher(TRUE);
	
}	//==== CHyperCuberDoc::IHyperCuberDoc() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::Dispose
//|
//| Purpose: Dispose of a HyperCuberdocument
//|
//| Parameters: none
//|_________________________________________________________

void CHyperCuberDoc::Dispose()

{

	((CEnhancedWindow *) itsWindow)->
		GetRect(&gPrefs->prefs.graphics_window_position);//  Save the graphics window position

	graphic->Dispose();									//  Get rid of the graphic
	
	controls_directors->Dispose();						//  Get rid of the controls list.  Note that the
														//   actual controls directors have already been
														//   disposed by CApplication's Quit method.

	inherited::Dispose();				//  Dispose of it as a Document

}	//==== CHyperCuberDoc::Dispose() ====\\


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::DoCommand
//|
//| Purpose: This handles document commands
//|
//| Paramters: command: the command to do
//|__________________________________________

void CHyperCuberDoc::DoCommand(long command)

{

	CHyperCuberPane *main_pane =
					(CHyperCuberPane *) itsMainPane;

	if (command > cmdShowControlsWindowBase)					//  Show or hide a controls window
		{
		
		long dimension = command - cmdShowControlsWindowBase;	//  Find dimension controlled by window
		
		CControlsDirector *director =
					(CControlsDirector *) controls_directors->
											NthItem(dimension);	//  Find the director of window	
		
		if (gPrefs->prefs.controls_window_visible[dimension])	//  Show or hide the window
			director->HideWindow();
		else
			director->ShowWindow();

		gPrefs->prefs.controls_window_visible[dimension] = 
			!gPrefs->prefs.controls_window_visible[dimension];	//  Reverse visibility of window in prefs

		return;													//  Done with this command

		}

	switch (command) {

		case cmdMono:
		
			main_pane->StereoMode = mono;
			main_pane->UpdateGraphicsPanes();			//  Adjust panes to new config., and redraw
			break;
		
		case cmdTwoImageStereo:
		
			main_pane->StereoMode = two_image_stereo;
			main_pane->graphic->SwitchToStereo();		//  Prepare graphic for switch
			main_pane->UpdateGraphicsPanes();			//  Adjust panes to new config.
			main_pane->Refresh();						//  Redraw the object
			break;
		
		case cmdTwoColorStereo:
		
			main_pane->StereoMode = two_color_stereo;
			main_pane->graphic->SwitchToStereo();		//  Prepare graphic for switch
			main_pane->UpdateGraphicsPanes();			//  Adjust panes to new config.
			main_pane->Refresh();						//  Redraw the object
			break;
		
		case cmdAntialias:
		
			main_pane->fAntialias =
							!main_pane->fAntialias;		//  Toggle Antialiasing
			main_pane->Refresh();						//  Redraw the object
			break;
		
		case cmdCopy:
		
			PicHandle object_pict;
			CreatePICT(object_pict);
			
			gClipboard->EmptyGlobalScrap();					//  Put the Picture in the clipboard
			gClipboard->PutGlobalScrap('PICT',
							(Handle) object_pict);

			KillPicture(object_pict);						//  Get rid of the picture

			break;

		case cmdToggleMenuBar:								//  Toggle the menubar
			
			if (menubar_hidden)
				{
				show_menubar();								//  Show the menubar
				
				gBartender->SetCmdText(cmdToggleMenuBar,	//  Change menu text to "Hide Menu Bar"
										"\pHide Menu Bar");	//  Not that anyone will ever see this....
				}
			else
				{
				hide_menubar();								//  Hide the menubar

				LongRect window_rect_long;					//  Update the window which was under bar
				Rect window_rect;
				itsWindow->Prepare();
				itsWindow->GetFrame(&window_rect_long);
				LongToQDRect(&window_rect_long, &window_rect);
				window_rect.bottom = window_rect.top + menubar_height + 1;
				InvalRect(&window_rect);

				gBartender->SetCmdText(cmdToggleMenuBar,	//  Change menu text to "Show Menu Bar"
										"\pShow Menu Bar");
				
				}
			break;
			
		case cmdToggleFullScreen:							//  Make this window full-screen
			
			if (fFullScreen)
				{
				if (menubar_hidden)
					DoCommand(cmdToggleMenuBar);			//  There must be a menu bar if the window's
															//   not full screen.

				((CEnhancedWindow *) itsWindow)->
						Place(&normal_window_size);			//  Restore former window size
				fFullScreen = FALSE;						//  It's no longer full screen

				}

			else
				{
				((CEnhancedWindow *) itsWindow)->
						GetRect(&normal_window_size);		//  Remember the current window position
				((CEnhancedWindow *) itsWindow)->
						MakeFullScreen();					//  Make the window full-screen
				fFullScreen = TRUE;							//  It's now a full-screen window

				}
			break;

		default:	inherited::DoCommand(command);

			break;

	}
	
}	//==== CHyperCuberDoc::DoCommand() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::UpdateMenus
//|
//| Purpose: This method is called when the user clicks in the menu
//|          bar.  It enables all menu items which should be enabled
//|          when a HyperCuberDoc exists.
//|
//| Parameters: none
//|___________________________________________________________________

void CHyperCuberDoc::UpdateMenus(void)
{

	inherited::UpdateMenus();

	gBartender->EnableCmd(cmdCopy);						//  Enable the Copy menu item  
	gBartender->EnableCmd(cmdStereoMode);				//  Enable the Stereo mode submenu 
	gBartender->EnableCmd(cmdAntialias);				//  Enable the Antialias menu item 
	gBartender->EnableCmd(cmdToggleFullScreen);			//  Enable the Full Screen menu item 

	if (fFullScreen)
		gBartender->EnableCmd(cmdToggleMenuBar);		//  Enable Toggle Menu Bar... if full screen

	CHyperCuberPane *main_pane =
				(CHyperCuberPane *) itsMainPane;

	gPrefs->Lock(TRUE);									//  Lock down the prefs
	PrefsStruct *prefs = &(gPrefs->prefs);				//  Dereference prefs

	long i;
	for (i = 3; i <= graphic->dimension; i++)			//  Check items in Windows menu	
		{
		gBartender->EnableCmd(
						cmdShowControlsWindowBase + i);	//  Enable the controls window command
		gBartender->CheckMarkCmd(
					cmdShowControlsWindowBase + i,
					prefs->controls_window_visible[i]);	//  Check it if the controls window is visible
		}
						
	gBartender->EnableCmd(cmdMono);						//  Enable the Stereo Mode submenu if Stereo is on
	gBartender->EnableCmd(cmdTwoColorStereo);			//  Enable the Two-Color Stereo menu item 
	gBartender->EnableCmd(cmdTwoImageStereo);			//  Enable the Two-Image Stereo menu item 

	gBartender->CheckMarkCmd(cmdMono,					//  Check or uncheck Mono
				(main_pane->StereoMode == mono));
	gBartender->CheckMarkCmd(cmdTwoImageStereo,			//  Check or uncheck Two-Image Stereo
		(main_pane->StereoMode == two_image_stereo));
	gBartender->CheckMarkCmd(cmdTwoColorStereo,			//  Check or uncheck Two-Color Stereo
		(main_pane->StereoMode == two_color_stereo));
	gBartender->CheckMarkCmd(cmdAntialias,				//  Check or uncheck Antialiasing
						main_pane->fAntialias);
	gBartender->CheckMarkCmd(cmdToggleFullScreen,		//  Check or uncheck Full-Screen
						fFullScreen);

	gPrefs->Lock(FALSE);						//  Unlock the prefs

}	//==== CHyperCuberDoc::UpdateMenus() ====\\




//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::Activate
//|
//| Purpose: Activate this director, and show its controls windows
//|
//| Parameters: none
//|__________________________________________________________________

void CHyperCuberDoc::Activate(void)
{   

	CDocument::Activate();									//  Call superclass
	
#define WINDOWS_MENU_ID	130
#define BASE_WINDOWS_MENU_ITEMS	3

	long i;
	for (i = 3; i <= graphic->dimension; i++)				//  Add menu items to show/hide control windows
		{
		char menu_item_text[30];
		sprintf(menu_item_text, "%dD Controls", i);
		gBartender->InsertMenuCmd(cmdShowControlsWindowBase + i,
						CtoPstr(menu_item_text),
						WINDOWS_MENU_ID,
						BASE_WINDOWS_MENU_ITEMS + i - 3);	//  Add "nD Controls" to the end of the menu
		}
		
	if (!itsWindow) return;									//  Don't do anything with windows if we're just
															//    creating this document
	
	for (i = 3; i <= graphic->dimension; i++)
		{
		CControlsDirector *director = (CControlsDirector *) controls_directors->NthItem(i);

		if (gPrefs->prefs.controls_window_visible[i])
			director->GetWindow()->ShowFloat();				//  Show the window if it should be visible
	
		}

	BecomeGopher(TRUE);

}	//==== CHyperCuberDoc::Activate() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::Deactivate
//|
//| Purpose: Deactivate this director, and hide its controls windows
//|
//| Parameters: none
//|__________________________________________________________________

void CHyperCuberDoc::Deactivate(void)
{   

	CDocument::Deactivate();							//  Call superclass

	long i;
	for (i = 3; i <= graphic->dimension; i++)			//  Remove menu items to show/hide control windows
		gBartender->RemoveMenuCmd(cmdShowControlsWindowBase + i);

	if (!itsWindow)
		return;											//  Don't try to dispose of controls 
														//    windows if we're closing document

	for (i = 3; i <= graphic->dimension; i++)
		{
		CControlsDirector *director =
					(CControlsDirector *)
						controls_directors->NthItem(i);

		if (gPrefs->prefs.controls_window_visible[i])
			director->GetWindow()->HideFloat();			//  Hide the window if it is visible
		}

}	//==== CHyperCuberDoc::Deactivate() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::Suspend
//|
//| Purpose: Suspend this director (switching to another process)
//|
//| Parameters: none
//|__________________________________________________________________

void CHyperCuberDoc::Suspend(void)
{   

	inherited::Suspend();
	
	itsWindow->Deactivate();
	
}	//==== CHyperCuberDoc::Suspend() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::Resume
//|
//| Purpose: Resume this director (switching from another process)
//|
//| Parameters: none
//|__________________________________________________________________

void CHyperCuberDoc::Resume(void)
{   

	inherited::Resume();
	
	itsWindow->Activate();
	
}	//==== CHyperCuberDoc::Resume() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::DoKeyDown
//|
//| Purpose: This is called when the user presses a key
//|
//| Parameters: theChar: char code of the key pressed
//|             keyCode: key code of the key pressed
//|             macEvent: keydown event
//|______________________________________________________________

void CHyperCuberDoc::DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent)
{

	gPrefs->Lock(TRUE);

	short modifiers = (macEvent->modifiers &
		(cmdKey | optionKey | shiftKey | controlKey | alphaLock));//  Get the modifiers
	key_control_struct *key_controls =
					gPrefs->prefs.key_controls;					//  Get the key controls array
	
	long i;
	for (i = 0; i <= gPrefs->prefs.num_key_controls; i++)		//  Loop through all key controls
		{
		if ((key_controls[i].key_code == keyCode) &&
			(key_controls[i].modifiers == modifiers))			//  Check if we should use this
			{
			key_control_struct key_control = key_controls[i];	//  Get the key control record
			
			CControlsDirector *controls_director =
					(CControlsDirector *)
						controls_directors->
							NthItem(key_control.dimension);		//  Find the controls
			
			controls_director->OffsetScrollBar(
					key_control.angle, key_control.increment);	// Update the scroll bar
			}
		}
	
	gPrefs->Lock(FALSE);

}	//==== CHyperCuberDoc::DoKeyDown() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::DoAutoKey
//|
//| Purpose: This is called when the a key repeats
//|
//| Parameters: theChar: char code of the key pressed
//|             keyCode: key code of the key pressed
//|             macEvent: keydown event
//|______________________________________________________________

void CHyperCuberDoc::DoAutoKey(char theChar, Byte keyCode, EventRecord *macEvent)
{

	DoKeyDown(theChar, keyCode, macEvent);		//  Same as DoKeyDown

}	//==== CHyperCuberDoc::DoAutoKey() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::NewFile
//|
//| Purpose: Create a new window for the new document
//|
//| Parameters: none
//|_________________________________________________________

void CHyperCuberDoc::NewFile(void)
{   

#define THREE_COLOR_HYPERCUBE_ID	128

	NewFileFromTEXT(THREE_COLOR_HYPERCUBE_ID);		//  Display a three-color hypercube

}	//==== CHyperCuberDoc::NewFile() ====\\


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::NewNCube
//|
//| Purpose: Create a new n-dimensional cube
//|
//| Parameters: dimension: dimension of the n-cube
//|_________________________________________________________

void CHyperCuberDoc::NewNCube(long dimension)
{

	char filename[10];

	if (dimension == 3)
		strcpy(filename, "Cube");				//  Set the filename to describe to the object
		
	else if (dimension == 4)
		strcpy(filename, "Hypercube");
	
	else
		sprintf(filename, "%d-Cube", dimension);

	CreateNCube(dimension, filename);			//  Create an n-cube file
	OpenFileFromFilename(filename);				//  Open the file
	short err = remove(filename);				//  Delete the file

}	//==== CHyperCuberDoc::NewNCube() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::OpenFile
//|
//| Purpose: Open the file the user selected, and create a new window for it
//|
//| Parameters: macSFReply: the file information
//|__________________________________________________________________________

void CHyperCuberDoc::OpenFile(SFReply *macSFReply)

{
	
	short working_dir_id = macSFReply->vRefNum;		//  Set the default directory to the input file's
	SetVol ((StringPtr) NULL, working_dir_id);		//    directory

	char	filename[32];							//  Get the filename
	strcpy (filename, PtoCstr(macSFReply->fName));
	
	OpenFileFromFilename(filename);					//  Open file

}	//==== OpenFile() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::OpenFileFromFilename
//|
//| Purpose: Open the file, given the filename
//|
//| Parameters: filename: the filename
//|__________________________________________________________________________

void CHyperCuberDoc::OpenFileFromFilename(char *filename)
{
	
	ifstream fs;
	fs.open(filename);									//  Open the file
	long g;
	fs >> *graphic;										//  Read the graphic
	fs.close();											//  Close the file

	BuildGraphicsWindow();								//  Build a graphics window for this file
	itsWindow->SetTitle(CtoPstr(filename));				//  Set the window name to the filename
	PtoCstr((unsigned char *) filename);

	controls_directors->Append((CObject *) NULL);		// Add null items for dimensions 1 and 2
	controls_directors->Append((CObject *) NULL);
	
	long i;
	for (i = 3; i <= graphic->dimension; i++)
		{
		CControlsDirector *controls_director;
		controls_director = new(CControlsDirector);		//  Create a controls director for this
		controls_director->IControlsDirector(this,
				(CHyperCuberPane *) itsMainPane, i);	//    dimension

		controls_directors->Append(controls_director);	//  Add this director to the list
		}

	itsWindow->Select();								//  Make the window active

	graphic->Project(graphic->dimension);				//  Do all projections for graphic

	Rect no_change = {0, 0, 0, 0};
	CHyperCuberPane *main_pane =
				(CHyperCuberPane *) itsMainPane;
	main_pane->AdjustToEnclosure(&no_change);			//  Recalc graphics panes (and draw object)

}	//==== OpenFileFromFilename() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::NewFileFromTEXT
//|
//| Purpose: Get a new document, with graphic defined from a TEXT resource
//|
//| Parameters: TEXT_id: the resource id of the TEXT resouce
//|__________________________________________________________________________

void CHyperCuberDoc::NewFileFromTEXT(short TEXT_id)
{
	
	Handle TEXT_handle = GetResource('TEXT', TEXT_id);	//  Get the TEXT resource
	short string_length = GetHandleSize(TEXT_handle);	//  Get the length
	SetHandleSize(TEXT_handle, string_length+1);		//  Add an extra byte for null terminator
	HLock(TEXT_handle);
	
	char *text_string = *TEXT_handle;					//  Get a C string
	text_string[string_length] = '\0';					//  Add a terminator
	
	strstream s;
	s << text_string;									//  Write the text string to the stream
	s >> *graphic;										//  Read the graphic from the stream

	DisposHandle(TEXT_handle);							//  Dispose of the TEXT resource in memory	

	BuildGraphicsWindow();								//  Build a graphics window for this file
	itsWindow->SetTitle("\pHyperCube");					//  Set the window name

	controls_directors->Append((CObject *) NULL);		// Add null items for dimensions 1 and 2
	controls_directors->Append((CObject *) NULL);
	
	long i;
	for (i = 3; i <= graphic->dimension; i++)
		{
		CControlsDirector *controls_director;
		controls_director = new(CControlsDirector);		//  Create a controls director for this
		controls_director->IControlsDirector(this,
				(CHyperCuberPane *) itsMainPane, i);	//    dimension

		controls_directors->Append(controls_director);	//  Add this director to the list
		}

	itsWindow->Select();								//  Make the window active

	graphic->Project(graphic->dimension);				//  Do all projections for graphic

	Rect no_change = {0, 0, 0, 0};
	CHyperCuberPane *main_pane =
				(CHyperCuberPane *) itsMainPane;
	main_pane->AdjustToEnclosure(&no_change);			//  Recalc graphics panes (and draw object)

}	//==== CHyperCuberDoc::NewFileFromTEXT() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::BuildGraphicsWindow
//|
//| Purpose: Builds the graphics window
//|
//| Parameters: none
//|_________________________________________________________

void CHyperCuberDoc::BuildGraphicsWindow (void)

{

#define	TWO_IMAGE		128
#define	TWO_COLOR		129

	itsWindow = new(CEnhancedWindow);					//  Create the window
	((CEnhancedWindow *) itsWindow)->
				IEnhancedWindow(WINDHyperCuber, FALSE,
					gDesktop, this);
	itsWindow->helpResID = WINDOW_HELP_RES;

	Rect size_rect;										//  Limit size of window to size of
	gDesktop->GetBounds(&size_rect);					//  desktop and make sure it's always
	size_rect.left = 50;								//  large enough to be visible.
	size_rect.top = 50;
	itsWindow->SetSizeRect(&size_rect);

	((CEnhancedWindow *) itsWindow)->
		PlaceWithVerify(
			&gPrefs->prefs.graphics_window_position);	//  Place the window in the default position
														//  (but not offscreen)
		
	CHyperCuberPane	*theMainPane;
	theMainPane = new(CHyperCuberPane);					//  Setup the main pane
    theMainPane->IHyperCuberPane(itsWindow, this,
    							100, 100, 10, 10,
    							sizELASTIC, sizELASTIC);
	itsMainPane = theMainPane;							//  Make this the main pane
	itsGopher = theMainPane;							//  Make it the gopher
	theMainPane->FitToEnclosure(TRUE, TRUE);			//  Expand it to fill the window
	theMainPane->SetWantsClicks(TRUE);					//  Let the controls get clicks
	theMainPane->helpResIndex = kImage;					//  Link to Balloon Help

	short window_width, window_height;
	theMainPane->GetLengths(&window_width,				//  Get width and height of window
								&window_height);

	CSizeBox *size_box = new(CSizeBox);					//  Add a size box in lower right
	size_box->ISizeBox(itsWindow, this);

	short pane_width, pane_height;
	theMainPane->GetLengths(&pane_width, &pane_height);
	
    theMainPane->graphic = graphic;						//  Link the main pane to its graphic
	graphic->pane = theMainPane;						//  Link the graphic to the main pane

	Rect pane_bounds = {0, 0, pane_height, pane_width};
	adjust_offscreen_pixmap(&theMainPane->OffscreenPort,//  Make offscreen bitmap large enough
									&pane_bounds);
	
}	//==== CHyperCuberDoc::BuildGraphicsWindow() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::DoSaveAs
//|
//| Purpose: Save the graphic as a PICT file
//|
//| Parameters: macSFReply: the file to save to
//|_________________________________________________________

Boolean	CHyperCuberDoc::DoSaveAs(SFReply *macSFReply)
{

#if 0
	static frame = 0;
	sprintf((char *) macSFReply->fName, "f%d", frame++);	//  Save picture to frame file.  This
	CtoPstr((char *) macSFReply->fName);					//   code can be used to create movies.
#endif


	FSSpec spec;
	short error;
	error = FSMakeFSSpec(macSFReply->vRefNum, 0, 		//  Create FSSpec for PICT file
						macSFReply->fName, &spec);

	PicHandle object_pict;
	CreatePICT(object_pict);

	short file_refnum;
	error = FSpDelete(&spec);							//  Create PICT file
	error = FSpCreate(&spec, 'ttxt', 'PICT',
						iuSystemScript);
	error = FSpOpenDF(&spec, fsWrPerm, &file_refnum);
	
	long header_size = 512;								//  Write empty PICT file header
	Handle header_handle = NewHandleClear(header_size);
	HLock(header_handle);
	error = FSWrite(file_refnum, &header_size,
						*header_handle);
	DisposHandle(header_handle);
	
	long pict_size = GetHandleSize(						//  Write the PICT
						(Handle) object_pict);	
	HLock((Handle) object_pict);
	error = FSWrite(file_refnum, &pict_size,
						*object_pict);
	
	error = FSClose(file_refnum);						//  Close the PICT file

	KillPicture(object_pict);							//  Get rid of the picture

	return TRUE;

}	//==== CHyperCuberDoc::DoSaveAs() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberDoc::CreatePICT
//|
//| Purpose: Create a PICT from the graphic
//|
//| Parameters: hypercube_pict: receives the PICT
//|_________________________________________________________

void CHyperCuberDoc::CreatePICT(PicHandle& hypercube_pict)
{

	LongRect frame_rect_long;				
	OpenCPicParams pict_params;
	CHyperCuberPane *main_pane =
				(CHyperCuberPane *) itsMainPane;
	main_pane->GetFrame(&frame_rect_long);			//  Get the frame in frame coordinates
							
	main_pane->FrameToWindR(&frame_rect_long,		//  Convert frame to window coordinates
						&(pict_params.srcRect));
	pict_params.hRes =								//  72dps resolution
				pict_params.vRes = 0x00480000;
	pict_params.version = -2;						//  Version 2 picture
	
	main_pane->Prepare();							//  Prepare to draw to graphics frame
	hypercube_pict = OpenCPicture(&pict_params);	//  Start recording a Picture
							
	
	if (!main_pane->fAntialias)
		main_pane->fDrawOffscreen = FALSE;			//  If we're not antialiasing, create
													//    an object-type picture.  Else
													//    create a bitmap.
	
	main_pane->Draw((Rect *) NULL);					//  Redraw the object

	main_pane->fDrawOffscreen = TRUE;

	ClosePicture();									//  Stop recording the Picture
	
	RGBColor rgb_white = {0xFFFF, 0xFFFF, 0xFFFF};	//  Restore colors
	RGBColor rgb_black = {0x0000, 0x0000, 0x0000};
	RGBForeColor(&rgb_black);
	RGBBackColor(&rgb_white);
			
}	//==== CHyperCuberDoc::CreatePICT() ====\\
