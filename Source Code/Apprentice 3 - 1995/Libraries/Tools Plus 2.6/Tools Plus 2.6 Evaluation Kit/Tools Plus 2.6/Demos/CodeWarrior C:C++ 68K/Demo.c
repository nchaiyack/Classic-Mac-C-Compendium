/***********************************************************************************
 *	Tools Plus	--	D e m o   P r o g r a m	--	(Version 2.6)
 *	Copyright (c) 1994-95 Water's Edge Software
 *	Designed and programmed by Steve Makohin
 *
 *	All files in this project use the Monaco 9pt.
 *	This application is written as a single source file (ie: I deliberately did NOT
 *	split it into units) to simplify compiling for programmers that are just
 *	starting out.  It also demonstrates that programming with Tools Plus makes
 *	source code compact and readable.
 *
 *	If you are put off by the size of this program, please note that it is
 *	H E A V I L Y   C O M M E N T E D and it is not optimized, both of which were
 *	done to facilitate the teaching of Tools Plus.
 *
 *	Though you will likely be able to figure out what each line of source code does,
 *	extensive comments are provided so you won't have to constantly refer to the
 *	Tools Plus User Manual and Inside Macintosh.


 *	R e s o u r c e s :
 *	This demo uses resources that can be found in the "Demo Resources" file.  The
 *	following describes each of the resources and what function they perform in this
 *	application.  You can add, change, delete and view resources by using a resource
 *	editor application such as Apple's ResEdit.
 *
 *	Type     ID    Description
 *	----  -------  -----------
 *	acur		128	   Animated Cursor definition:  Tells Tools Plus how to animate the
 *                 cursor to create a wrist watch with a spinning second hand.  The
 *                 'acur' resource specifies the sequence of cursor resources
 *                 ('CURS') that will be used for the animation, and the time taken
 *                 between steps.  See the User Manual for details.
 *
 *	BNDL		128    Bundle Resource:  This resource tells the Finder which icons are
 *                 used to depict the application and the documents created by it.
 *                 The BNDL resource bundles the following related resources: FREF,
 *                 icl4, icl8, ICN#, and 'TP 1'
 *
 *	cicn		- -		 Color Icons (definable size):  This is the first color icon format
 *								 available on color (Mac II) Macintoshes.  Each 'cicn' includes a
 *								 color image, the equivalent black and white image, and a mask.  In
 *								 this demo, all 'cicn' icons were created using 4-bit colors to
 *								 ensure that they look good when displayed on monitors set to
 *								 16-colors/grays or higher.
 *				300-303  These color icons (hard disk, floppy, and folder) show up in the
 *								 Pop-Up Menus demo beside menu item names.
 *				304,305	 These "large" color icons (printer and modem) appear in "picture
 *								 Pop-Up Menus" (picture in a Pop-Up Menu without any text).  There
 *								 are 'ICON' equivalents for these items for Macs without Color
 *								 QuickDraw.
 *				430-441	 These "tape deck" buttons are used in the Picture Buttons demo to
 *								 show you the look you can get with Tools Plus's buttons.  Each
 *								 button requires two icons, one for the button's normal state, and
 *								 one for the selected (pushed in) state.  I know, they don't behave
 *								 like a real tape deck, but this is only a demo.
 *				460,461  This bi-directional "globe" button is used in the Picture Buttons
 *								 demo.  The two icons show the button in its normal state, and in
 *								 its selected (pushed in) state.
 *				464,465	 This "Done" button is used in the Picture Buttons demo.  The two
 *								 icons show the button in its normal state, and in its selected
 *								 (pushed in) state.  Even the most ordinary buttons can look
 *								 sophisticated with Tools Plus.
 *				520-541	 These are all used in the Floating Palettes demo to create
 *								 clickable icons.  You can create professional looking palettes
 *								 with various styles of buttons using Tools Plus.
 *
 *	CURS	128-134  Cursors:  These cursors are used to produce an animated cursor.
 *                 The 'acur' resource specified which cursors are used, and the
 *                 animation's speed.  See the User Manual for details.
 *
 *	FREF		128    Finder Reference:  This resource is automatically created when a
 *                 BNDL resource is created.  One FREF exists for each icon that the
 *                 Finder references.  This demo is an application (file type is
 *                 'APPL'), so one FREF is needed for that.  If this demo had the
 *                 ability to create three different kinds of documents, then one
 *                 FREF would be required for each kind type of document.
 *
 *	icl4		- -		 Large (32x32 pixel) 4-bit Icons:  These icons are typically used
 *                 only by the Finder to depict an application and its related
 *                 files.  However, your application can also draw these icons with
 *                 Tools Plus.  4-bit icons can have a maximum of 16 colors.
 *					0,1,2	 These "large" color icons appear in "picture Pop-Up Menus"
 *								 (picture in a Pop-Up Menu without any text).  There are 'ICON'
 *								 equivalents for these items for Macs without Color QuickDraw.
 *								 These color icons will be used on color monitors as a substitute
 *                 for the system's Stop, Note and Caution icons.  They give color
 *                 applications a better look.
 *					128		 Application's Icon (displayed by the Finder.  The BNDL resource
 *                 refers to this icon)
 *					150		 Printer icon used to demonstrate drawing icon families
 *					151		 Modem icon used to demonstrate drawing icon families
 *				400,401	 Left Arrow button used in Picture Buttons demo (part of an icl8 /
 *								 icl4 / ICN# family).  One icon is used for the button's standard
 *								 position, and another for the selected (pushed in) position.
 *				402,403	 Right Arrow button used in Picture Buttons demo (similar to above)
 *				420-425	 Plus/Minus button used in Picture Buttons demo.  These 4 icons are
 *								 used in a particularly attractive button.  The top half (+)
 *								 darkens when you select it, and so does the bottom (-). Two icons
 *								 (standard and selected position) are used for the top half, and
 *								 two for the bottom. These icons are part of an icl8 / icl4 / ICN#
 *								 family.
 *				450-456	 Four icons are used to draw a dual-stage button (Power-On /
 *								 Power-Off) in the Picture Buttons demo; a pair for each stage, one
 *								 for the standard state and one for the selected (pushed in) state.
 *								 An additional pair of icons ause used to portray the buttons as
 *								 disabled; one for Power-On and one for Power-Off.  These icons are
 *								 part of an icl8 / icl4 / ICN# family.
 *
 *	icl8		- -		 Large (32x32 pixel) 8-bit Icons:  These icons are typically used
 *                 only by the Finder to depict an application and its related
 *                 files.  However, your application can also draw these icons with
 *                 Tools Plus.  8-bit icons can have a maximum of 256 colors.
 *					1,2    These color icons will be used on color monitors as a substitute
 *                 for the system's Note and Caution icons.  They give color
 *                 applications a better look.  Notice that there isn't a Stop icon
 *                 (ID=0).  The equivalent icl4 will be used instead.
 *					128		 Application's Icon (displayed by the Finder.  The BNDL resource
 *                 refers to this icon)
 *					150		 Printer icon used to demonstrate drawing icon families
 *					151		 Modem icon used to demonstrate drawing icon families
 *				400,401	 Left Arrow button used in Picture Buttons demo (part of an icl8 /
 *								 icl4 / ICN# family).  One icon is used for the button's standard
 *								 position, and another for the selected (pushed in) position.
 *				402,403	 Right Arrow button used in Picture Buttons demo (similar to above)
 *				420-425	 Plus/Minus button used in Picture Buttons demo.  These 4 icons are
 *								 used in a particularly attractive button.  The top half (+)
 *								 darkens when you select it, and so does the bottom (-). Two icons
 *								 (standard and selected position) are used for the top half, and
 *								 two for the bottom. These icons are part of an icl8 / icl4 / ICN#
 *								 family.
 *				450-456	 Four icons are used to draw a dual-stage button (Power-On /
 *								 Power-Off) in the Picture Buttons demo; a pair for each stage, one
 *								 for the standard state and one for the selected (pushed in) state.
 *								 An additional pair of icons ause used to portray the buttons as
 *								 disabled; one for Power-On and one for Power-Off.  These icons are
 *								 part of an icl8 / icl4 / ICN# family.
 *
 *	ICN#		- -		 Large (32x32 pixel) 1-bit Icons with Mask:  These icons are
 *                 typically used only by the Finder to depict an application and
 *                 its related files.  However, your application can also draw these
 *                 icons with Tools Plus.  1-bit icons are black and white (no
 *                 shades of gray).
 *					128		 Application's Icon (displayed by the Finder.  The BNDL resource
 *                 refers to this icon)	
 *					150		 Printer icon used to demonstrate drawing icon families
 *					151		 Modem icon used to demonstrate drawing icon families
 *				400,401	 Left Arrow button used in Picture Buttons demo (part of an icl8 /
 *								 icl4 / ICN# family).  One icon is used for the button's standard
 *								 position, and another for the selected (pushed in) position.
 *				402,403	 Right Arrow button used in Picture Buttons demo (similar to above)
 *				420-425	 Plus/Minus button used in Picture Buttons demo.  These 4 icons are
 *								 used in a particularly attractive button.  The top half (+)
 *								 darkens when you select it, and so does the bottom (-). Two icons
 *								 (standard and selected position) are used for the top half, and
 *								 two for the bottom. These icons are part of an icl8 / icl4 / ICN#
 *								 family.
 *				450-456	 Four icons are used to draw a dual-stage button (Power-On /
 *								 Power-Off) in the Picture Buttons demo; a pair for each stage, one
 *								 for the standard state and one for the selected (pushed in) state.
 *								 An additional pair of icons ause used to portray the buttons as
 *								 disabled; one for Power-On and one for Power-Off.  These icons are
 *								 part of an icl8 / icl4 / ICN# family.
 *
 *	ICON		- -		 Large (32x32 pixel) 1-bit Icons:  These are standard black &
 *                 white icons.
 *					0,1,2  These icons will be used as a substitute for the system's Stop,
 *                 Note and Caution icons.  In this case, they are exactly the same
 *                 as System 7's system icons, but they are included in case this
 *                 application runs on System 5 (where the system icons are
 *                 different).  There are color counterparts for these icons (icl4
 *                 and icl8).
 *
 *				304,305	 These printer and modem icons appear in "picture Pop-Up Menus"
 *								 (picture in a Pop-Up Menu without any text) on Macs without Color
 *								 QuickDraw.  There are 'cicn' equivalents for these items for Macs
 *								 with Color QuickDraw.
 *
 *	ics4		- -		 Small (16x16 pixel) 4-bit Icons:  These icons are typically used
 *                 only by the Finder to depict an application and its related
 *                 files.  4-bit icons can have a maximum of 16 colors.
 *					128		 Application's Icon (displayed by the Finder.  The BNDL resource
 *                 refers to this icon)
 *
 *	ics8		- -		 Small (16x16 pixel) 8-bit Icons:  These icons are typically used
 *                 only by the Finder to depict an application and its related
 *                 files.  8-bit icons can have a maximum of 256 colors.
 *					128		 Application's Icon (displayed by the Finder.  The BNDL resource
 *                 refers to this icon)
 *
 *	ics#		- -		 Small (16x16 pixel) 1-bit Icons:  These icons are typically used
 *                 only by the Finder to depict an application and its related
 *                 files.  1-bit icons are black & white.
 *					128		 Application's Icon (displayed by the Finder.  The BNDL resource
 *                 refers to this icon)
 *
 *	PICT		- -		 Pictures of any dimension, number of colors (including gray scale
 *                 and black & white)
 *					128		 Black & white (1-bit) picture.  It's the black & white equivalent
 *                 of resource 129
 *					129		 Color (8-bit) picture.  It's the color equivalent of resource
 *                 128.
 *				150-173	 Complete picture suite needed for a 2-stage button. Each stage
 *								 (value) can be either or not, and includes different PICTs for
 *								 screen depths of 8-bits, 4-bits, and B&W. They also include a
 *								 mask, though this is not necessary when drawing on a white
 *								 background.
 *
 *  SICN		- -		 Small (16x16 pixel) 1-bit Icons:  These icons are "indexed",
 *								 meaning that you can have multiple icon images in a single 'SICN'
 *								 resource.
 *				300-303	 These color icons (hard disk, floppy, and folder) show up in the
 *								 Pop-Up Menus demo beside menu item names. They are supplied
 *								 because Macs without Color QuickDraw need SICNs (they can't use
 *								 color 'cicn' icons).
 *				408-414  These are all used in the Picture Buttons demo to create 3D icons.
 *								 Tools Plus converts an SICN icon into a color 3D button.
 *					458		 These are all used in the Picture Buttons demo to create 3D icons.
 *								 Tools Plus converts an SICN icon into a color 3D button.  Because
 *								 this SICN resource has several images, each image becomes an
 *								 available "state" (or usable value).  In this example, the user
 *								 can step through the stages by clicking on the button.
 *				500-519	 These are all used in the Floating Palettes demo to create 3D
 *								 icons.  Tools Plus converts an SICN icon into a color 3D button.
 *								 You can create professional looking palettes with different styles
 *								 of buttons using Tools Plus.
 *
 *	SIZE		-1     Multi-tasking info:  The 'SIZE' resource is needed if your
 *                 application is going to run under MultiFinder (System 5 and 6)
 *                 or System 7.  It tells the Macintosh how your application should
 *                 behave in a multi-tasking environment.  See the User Manual for
 *                 details.
 *
 *	TP 1		128		 Owner Resource:  This resource is automatically created when a
 *                 BNDL resource is created. This demo's signature is 'TP 1', and as
 *                 you may recall, each application must have a unique signature
 *                 code (see the User Manual for details).  That signature code is
 *                 used to create an "owner resource" whose resource type is the
 *                 same as the application's signature code.
 *
 *	vers		- -		 Application Version:  These resources' information is displayed
 *                 in the Finder's Get Info box.
 *					1      This information is displayed at the bottom of the Get Info box.
 *                 It includes an application version number and the copyright info.
 *					2      This information is displayed at the top of the Get Info box.  It
 *                 is a comment that is displayed beneath the application's name
 *                 (ie: "Tools Plus demo").
 *
 *	WDEF		- -		 Window Definition:  WDEFs define how a window looks.  If you want
 *                 to open a window that is not one of the six standard Macintosh
 *                 window types, you have to create a window definition.
 *					2000	 Pallet Window:  These windows have a narrow title bar, and are
 *                 typically used as "floating windows" (they are always active, and
 *                 they float above your other windows).  This demo has several
 *                 floating pallets.


 *	T h e   D e m o s :
 *	This demo application can be used as a framework for a new application, although
 *  you will probably want to improve upon it to cater to your own programming style
 *  and needs.  The demo can be broken down into several sections:
 *
 *		main					The main program demonstrates a typical application's startup
 *                  and response to events.
 *
 *	PrepareTheDemo	This routine creates the demos menus and opens a couple demo
 *                  windows.
 *
 *		Window 1			This window demonstrates how Tools Plus works with�
 *										� List Boxes
 *										� Buttons (radio buttons, check boxes and push buttons)
 *										� Cursor Zones
 *
 *		Window 2			This window demonstrates how Tools Plus works with�
 *										� Editing Fields (typing, Cut, Copy, Paste, Clear, and Undo.
 *										  Tabbing to next / prev field)
 *										� Scroll Bars
 *										� "Size box" and the "zoom box" (changing a window's size)
 *
 *		Window 3			This window demonstrates how Tools Plus works with�
 *										� drawing icon families
 *										� Cursor Zones used to make icons click sensitive
 *
 *		Window 4			This window shows off Tools Plus's cursor animation, and how
 *									Tools Plus behaves when a watch cursor is displayed.
 *
 *		Window 5			This window demonstrates how to write code for color drawing
 *									that is dependent on the monitor's settings (ie: number of
 *									colors, number of shades of gray, or black & white).  It also
 *									shows how to write code that is compatible with Macintoshes that
 *									have multiple monitors.
 *
 *		Window 6			This demo shows off Tools Plus's Dynamic Alerts.  They are alert
 *									boxes that automatically adjust to accommodate the text your
 *									application provides them.
 *
 *		Window 7			This window demonstrates Tools Plus's Pop-Up Menus and some of
 *									the features you can incorporate into them.
 *
 *		Window 8			This window features Tools Plus's Picture Buttons, which
 *									transform icons or pictures into powerful buttons.  Picture
 *									Buttons have a list of properties that you can select and combine
 *									to create your own custom Picture Buttons.  Picture Button
 *									properties control the look and behavior of the button.
 *
 *		Window 9			Window 9 is this application's Tool Bar.  Tools Plus lets you
 *									create a Tool Bar as easily as any other Tools Plus window.  You
 *									can put any control you want in the Tool Bar, including picture
 *									buttons (the most common thing), pop-up menus, and editing fields.
 *									Demo Window 8 describes picture buttons in greater detail.
 *
 *		Window 10			Window 10 is this a floating palette.  Tools Plus lets you create
 *									a floating palette as easily as any other Tools Plus window.
 *									You can put any control you want in the Tool Bar, including
 *									picture buttons (the most common thing), pop-up menus, and
 *									editing fields.  Demo Window 8 describes picture buttons in
 *									greater detail.
 *
 *		Window 11			Window 11 is this also a floating palette (like demo window 10),
 *									except this is a horizontal palette with the title bar along the
 *									window's left side.





 *	S o u n d   M a n a g e r :
 *	This demo uses the Sound Manager just for entertainment's sake.  If this demo
 *	runs on a Mac that is running less than System 6, simple beeps will be played
 *	instead of sounds.


 **********************************************************************************/



																	/* I n c l u d e d   C o d e										*/
#include "String.h"								/*	ANSI C string manipulation									*/
#include "Sound.h"								/*	Macintosh Sound Manager, used to play a few	*/
																	/*	  sounds, just for fun.											*/
#include "ToolsPlus.h"						/*	Tools Plus header file.  Each program or		*/
																	/*	  unit that makes reference to a Tools Plus	*/
																	/*	  #define, struct, or routine needs to use	*/
																	/*	  this header.															*/
#include "PascalStrHandles.c"			/*	Quick 'n' Dirty routines for copying a C		*/
																	/*		string to a Pascal "String Handle" and		*/
																	/*		vice versa.																*/



																	/* D e f i n e s �															*/
																	/* These constants, types and variables are			*/
																	/*	 accessible throughout this program.				*/
/*	All these constants will make the demo's code more readable.  You don't	HAVE	*/
/*	to use them.  You can use literals, such as 1, 2, 3� if you want to.  It all	*/
/*	depends on your writing style and needs.																			*/
#define		DemoWindow1				1				/*Window numbers for the demo�								*/
#define		DemoWindow2				2				/*																						*/
#define		DemoWindow3				3				/*																						*/
#define		DemoWindow4				4				/*																						*/
#define		DemoWindow5				5				/*																						*/
#define		DemoWindow6				6				/*																						*/
#define		DemoWindow7				7				/*																						*/
#define		DemoWindow8				8				/*																						*/
#define		ToolBarWindow			9				/*																						*/
#define		VerticalPalette		10			/*																						*/
#define		HorizontalPalette	11			/*																						*/
#define		OkButton					1				/*Buttons used on Demo Window 1�							*/
#define		CancelButton			2				/*	When writing a real application,					*/
#define		MiniButton1				11			/*	you can give these constants							*/
#define		MiniButton2				12			/*	meaningful names, such as "OK",						*/
#define		MiniButton3				13			/*	"Cancel", "Detail", "Summary" etc.				*/
#define		MiniButton4				14			/*																						*/
#define		MiniButton5				15			/*																						*/
#define		CheckBox1					21			/*																						*/
#define		CheckBox2					22			/*																						*/
#define		CheckBox3					23			/*																						*/
#define		RadioButton1			31			/*																						*/
#define		RadioButton2			32			/*																						*/
#define		RadioButton3			33			/*																						*/
#define		MidiHalfMeg				11			/*																						*/
#define		MidiOneMeg				12			/*																						*/
#define		MidiTwoMeg				13			/*																						*/
#define		LeftArrowButton		1				/*Picture Buttons used on Window 8�						*/
#define		RightArrowButton	2				/*	When writing real applications, you can		*/
#define		LeftAlignButton		3				/*	use constants, variables, calculations,		*/
#define		CenterAlignButton	4				/*	or a number in place of these items.			*/
#define		RightAlignButton	5				/*																						*/
#define		JustifyButton			6				/*																						*/
#define		CutButton					7				/*																						*/
#define		BucketButton			8				/*																						*/
#define		ClipboardButton		9				/*																						*/
#define		PlusButton				10			/*																						*/
#define		MinusButton				11			/*																						*/
#define		PrinterButton			12			/*																						*/
#define		ModemButton				13			/*																						*/
#define		SteppingButton		14			/*																						*/
#define		ScrollingButton		15			/*																						*/
#define		GlobeButton				16			/*																						*/
#define		RewindButton			17			/*																						*/
#define		StopButton				18			/*																						*/
#define		FastFwdButton			19			/*																						*/
#define		PauseButton				20			/*																						*/
#define		RecordButton			21			/*																						*/
#define		PlayButton				22			/*																						*/
#define		PowerButton				23			/*																						*/
#define		FlipButton				24			/*																						*/
#define		DoneButton				25			/*																						*/
#define		VPaletteMinButton 1				/*First and last picture button used on the		*/
#define		VPaletteMaxButton 20			/*	vertical floating palette demo.						*/
#define		HPaletteMinButton 1				/*First and last picture button used on the		*/
#define		HPaletteMaxButton 22			/*	horizontal floating palette demo.					*/
#define		LeftArrowIcon			400			/*Icons and pictures used to make Picture			*/
#define		RightArrowIcon		404			/*	Buttons on Window 8�											*/
#define		LeftAlignIcon			408			/*																						*/
#define		CenterAlignIcon		409			/*																						*/
#define		RightAlignIcon		410			/*																						*/
#define		JustifyIcon				411			/*																						*/
#define		CutIcon						412			/*																						*/
#define		BucketIcon				413			/*																						*/
#define		ClipboardIcon			414			/*																						*/
#define		PlusIcon					420			/*																						*/
#define		MinusIcon					424			/*																						*/
#define		ScrollingIcon			458			/*																						*/
#define		GlobeIcon					460			/*																						*/
#define		RewindIcon				430			/*																						*/
#define		StopTapeIcon			432			/*																						*/
#define		FastFwdIcon				434			/*																						*/
#define		PauseIcon					436			/*																						*/
#define		RecordIcon				438			/*																						*/
#define		PlayIcon					440			/*																						*/
#define		PowerIcon					450			/*																						*/
#define		FlipSwitchPICT		150			/*																						*/
#define		DoneIcon					464			/*																						*/
#define		FirstVPaletteIcon 500			/*First icon number on the vert. and horiz.		*/
#define		FirstHPaletteIcon 520			/*	palettes. Other icons nums are calculated.*/
#define		LeftList					1				/*List Boxes used on Demo Window 1�						*/
#define		RightList					2				/*																						*/
#define		RightBar					1				/*Scroll Bars used on Demo Window 2�					*/
#define		BottomBar					2				/*																						*/
#define		ApplMenu					0				/*Menus and Menu Items�												*/
#define		AboutItem					1				/*																						*/
#define		FileMenu					1				/*																						*/
#define		CloseItem					3				/*																						*/
#define		QuitItem					5				/*																						*/
#define		EditMenu					2				/*																						*/
#define		DemosMenu					3				/*																						*/
#define		ToolBarItem				10			/*Item number for Tool Bar on the Demos menu	*/
#define		PaletteDemoMenu 	16			/*Hierarchic menus use numbers 16-200					*/
#define		popMenu1					1				/*Pop-Up Menu numbers													*/
#define		popMenu2					2				/*																						*/
#define		popMenu3					3				/*																						*/
#define		popMenu4					4				/*																						*/
#define		popMenu5					5				/*																						*/
#define		ButtonSpeedMenu		6				/*																						*/



#define		PrinterIcon				150			/*Printer and Modem icons�										*/
#define		ModemIcon					151			/*																						*/
#define		BlackAndWhitePICT	128			/*Color and Black & White pictures�						*/
#define		ColorPICT					129			/*																						*/
#define		WatersEdgeLogo		128			/*Logo for Water's Edge Software (icons)			*/



																				/* T y p e   D e f i n i t i o n s �			*/
typedef		unsigned char			Str30[31];	/*A 30-character string										*/



																				/* G l o b a l   R o u t i n e s �				*/
void PrepareTheDemo (void);							/* Prepare (initialize) the demo appl.		*/
void CloseTheWindow (short Window);			/* Close a window													*/
void ActionInWindow1 (void);						/* Respond to user's action in window 1		*/
void ActionInWindow2 (void);						/* Respond to user's action in window 2		*/
void ActionInWindow3 (void);						/* Respond to user's action in window 3		*/
void ActionInWindow4 (void);						/* Respond to user's action in window 4		*/
void ActionInWindow5 (void);						/* Respond to user's action in window 5		*/
void ActionInWindow6 (void);						/* Respond to user's action in window 6		*/
void ActionInWindow7 (void);						/* Respond to user's action in window 7		*/
void ActionInWindow8 (void);						/* Respond to user's action in window 8		*/
void ActionInWindow8_DrawToolBarValue (void);	/* Supporting routines for drawing	*/
void ActionInWindow8_DrawGlobeValue (void);		/*		objects in window 8.					*/
void ActionInWindow9 (void);						/* Respond to user's action in window 9		*/
void ActionInWindow10 (void);						/* Respond to user's action in window 10	*/
void ActionInWindow11 (void);						/* Respond to user's action in window 11	*/
																				/* Draw text in a specified rectangle�		*/
void TextInBox (char Str[], short left, short top, short right, short bottom, short Justification, Boolean withBox);
void stoa (short n, char s[]);					/* Convert a short to a C string					*/
																				/* Draw a group box with a title�					*/
void GroupBox (Str255 Str, short left, short top, short right, short bottom);	/*	*/
void DrawWindow1Contents (void);				/* Draw the contents of window 1					*/
void DrawWindow2Contents (void);				/* Draw the contents of window 2					*/
void DrawWindow3Contents (void);				/* Draw the contents of window 3					*/
void DrawWindow4Contents (void);				/* Draw the contents of window 4					*/
void DrawWindow5Contents (void);				/* Draw the contents of window 5					*/
void DrawWindow6Contents (void);				/* Draw the contents of window 6					*/
void DrawWindow7Contents (void);				/* Draw the contents of window 7					*/
void DrawWindow8Contents (void);				/* Draw the contents of window 8					*/
void DrawWindow9Contents (void);				/* Draw the contents of window 9					*/
void DrawWindow10Contents (void);				/* Draw the contents of window 10					*/
void DrawWindow11Contents (void);				/* Draw the contents of window 11					*/
void OpenDemoWindow1 (void);						/* Open Demo Window 1											*/
void OpenDemoWindow2 (void);						/* Open Demo Window 2											*/
void OpenDemoWindow3 (void);						/* Open Demo Window 3											*/
void OpenDemoWindow4 (void);						/* Open Demo Window 4											*/
void OpenDemoWindow5 (void);						/* Open Demo Window 5											*/
void OpenDemoWindow6 (void);						/* Open Demo Window 6											*/
void OpenDemoWindow7 (void);						/* Open Demo Window 7											*/
void OpenDemoWindow8 (void);						/* Open Demo Window 8											*/
void OpenDemoWindow9 (void);						/* Open Demo Window 9											*/
void OpenDemoWindow10 (void);						/* Open Demo Window 10										*/
void OpenDemoWindow11 (void);						/* Open Demo Window 11										*/
void RefreshWindow (void);							/* Respond to a doRefresh event						*/
void ChangeWindow (void);								/* Respond to a doChgWindow event					*/
void MenuSelection (void);							/* Respond to a doMenu event							*/





																				/* G l o b a l   V a r i a b l e s �			*/
TPPollRecord	Poll;											/*Polling record to retrieve event info		*/
Handle				hField1;									/*Handle to 1st editing field's 30-char		*/
																				/*	string.																*/
Handle				hField2, hField3;					/*Handles to 2nd and 3rd editing fields'	*/
																				/*	255-char string.											*/
short					MidiPort;									/*Serial port selected for MIDI						*/
																				/*	interface (Demo 3).										*/
RGBColor			ToolBarGray;							/*Color of toolbar												*/
short					ToolBarValue;							/*Value displayed in a window's tool bar	*/
																				/*	(Demo 8)															*/
short					MidiSpeed;								/*MIDI interface speed (Demo 3)						*/
Boolean				ExitTheDemo;							/*Should the demo terminate?							*/





/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * M a i n   P r o g r a m :
 *	The main program demonstrates the detection of, and response to events.
 *	Tools Plus events are more usable than those obtained from The Event Manager.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 void main(void)																
	{
	InitGraf(&qd.thePort);										/*Standard Mac ToolBox initialization	*/
	InitFonts();															/*	required by all applications.			*/
	InitWindows();														/*																		*/
	InitMenus();															/*																		*/
	TEInit();																	/*																		*/
	InitDialogs(0L);													/*																		*/
	MaxApplZone();														/*Expand application zone to its limit*/

	if (!InitToolsPlus(5, 11, UseColor))			/*Initialize Tools Plus with "5"			*/
		ExitToShell();													/*	handle blocks, and a maximum of		*/
																						/*	"11" open windows.  Use Color			*/
																						/*	QuickDraw if it�s available.  If	*/
																						/*	initialization fails, return to		*/
																						/*	the Finder.												*/
	PrepareTheDemo();													/*Prepare the demo by creating the		*/
																						/*	menus and opening a sample window.*/

	ExitTheDemo = false;											/*The demo is not over yet						*/
	while (!ExitTheDemo)											/*Keep polling for an event until the	*/
		{																				/*	Quit item is selected in the File	*/
																						/*	menu�															*/
		if (PollSystem(&Poll)) 									/*If an event has been detected�			*/
			{																			/*																		*/
   		switch (Poll.What)										/*Determine what kind of event has		*/
   			{																		/*	occurred�													*/
				case doMenu: 												/*User selected a menu item�					*/
					MenuSelection();									/*	execute that menu item.						*/
					break;														/*																		*/
				case doChgWindow: 									/*User clicked on an inactive window�	*/
					ChangeWindow(); 									/*	activate the window the user			*/
					break;														/*	clicked.													*/
				case doRefresh: 										/*A window needs to be refreshed�			*/
					RefreshWindow();									/*	redraw the specified window.			*/
					break;														/*																		*/
				case doGoAway: 											/*User clicked a window's "close box"�*/
					CloseTheWindow(Poll.Window);			/*	close the affected window.				*/
					break;														/*																		*/
				case doButton: 		case doPictButton:/*The user has clicked a button, typed*/
				case doKeyDown: 	case doAutoKey:		/*	a key, clicked in an inactive			*/
				case doClickField:case doScrollBar:	/*	field, used a scroll bar, clicked	*/
				case doListBox: 	case doPopUpMenu:	/*	a line in a list box, used a Pop-	*/
				case doClick:												/*	Up Menu, or clicked in the window	*/
																						/*	(not on an object)�								*/
					if (Poll.Window != none)					/*If the event is applied to an open	*/
						{																/*	window (ignore events if no window*/
																						/*	is open)�													*/
       			CurrentWindow(Poll.Window);			/*Make the affected window current.		*/
       																			/*	It will be the target of actions	*/
       																			/*	that are made in response to the	*/
       																			/*	user's activity in the affected		*/
       																			/*	window.														*/
						switch (Poll.Window)						/*Determine which window is affected	*/
							{															/*	and call the appropriate routine	*/
							case 1: 											/*	to respond to the user's activity	*/
       					ActionInWindow1();					/*	 (ie: call a window-handler				*/
       					break;											/*	routine).													*/
							case 2: 											/*																		*/
       					ActionInWindow2();					/*																		*/
       					break;											/*																		*/
							case 3: 											/*																		*/
       					ActionInWindow3();					/*																		*/
       					break;											/*																		*/
							case 4: 											/*																		*/
       					ActionInWindow4();					/*																		*/
       					break;											/*																		*/
							case 5: 											/*																		*/
       					ActionInWindow5();					/*																		*/
       					break;											/*																		*/
							case 6: 											/*																		*/
       					ActionInWindow6();					/*																		*/
       					break;											/*																		*/
							case 7: 											/*																		*/
       					ActionInWindow7();					/*																		*/
       					break;											/*																		*/
							case 8: 											/*																		*/
       					ActionInWindow8();					/*																		*/
       					break;											/*																		*/
							case 9: 											/*																		*/
       					ActionInWindow9();					/*																		*/
       					break;											/*																		*/
							case 10: 											/*																		*/
       					ActionInWindow10();					/*																		*/
       					break;											/*																		*/
							case 11: 											/*																		*/
       					ActionInWindow11();					/*																		*/
       					break;											/*																		*/
							}															/*																		*/
						}																/*																		*/
       			break;													/*																		*/
       			
    		default:														/*All other events are ignored by			*/
 					break;														/*	this demo.												*/
  			}																		/*																		*/
			}																			/*																		*/
		else																		/*If this demo was to perform on-going*/
   		{																			/*	operations, perhaps even when			*/
   																					/*	suspended, that code would go			*/
   		}																			/*	here.															*/
		}																				/*																		*/

 	CursorShape(watchCursor);									/*Change the cursor to a wrist watch	*/
 																						/*	as the demo quits to the Finder.	*/
 																						/*	Visually, it looks more						*/
 																						/*	professional to "look busy" while	*/
 	}																					/*	the application quits.						*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * P r e p a r a t i o n s :
 *	This routine does all the "set up" work for the demo, such as creating the
 *	menus and introducing the demo. In a full-featured application, you would
 *	probably open documents that were double-clicked from the Finder.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void PrepareTheDemo (void)
	{
	#define			QuitAlert						 11700		/*Button definition for a "Quit"			*/
																						/*	Dynamic Alert.										*/
	#define			SilentContinueAlert	-11500		/*Button definition for a "Continue"	*/
																						/*	dynamic alert that doesn't beep		*/
																						/*	the user.													*/


  SetNullTime(1, maxNullTime);							/*Be a good neighbor to other appls by*/
																						/*	using fewer CPU cycles.						*/
	CursorShape(watchCursor);									/*Change the cursor to a wrist watch	*/
  MidiPort = ModemIcon;											/*By default, select the modem port		*/
  MidiSpeed = MidiOneMeg;										/*	for a MIDI interface running at		*/
  																					/*	1 MHz. (demo window 3)						*/
  ToolBarGray.red = 52428;									/*Define the toolbar color as a medium*/
  ToolBarGray.green = 52428;								/*	dark gray.												*/
  ToolBarGray.blue = 52428;									/*																		*/

																						/* M e n u s :	-	-	-	-	-	-	-	-	-	-	-	*/
  AppleMenu("\pAbout The Demo�");						/*Create the Apple menu with an				*/
  																					/*	"About�" item.  You can now work	*/
  																					/*	with anything in the Apple menu		*/
  																					/*	without having to write code for	*/
  																					/*	it!																*/
  Menu(FileMenu, 0, enabled, "\pFile");			/*File menu:create menu name (menu 1)	*/
  Menu(FileMenu, 1, disabled, "\pNew/N");		/*			create "New" item (1st item)	*/
  Menu(FileMenu, 2, disabled, "\pOpen/O");	/*			create "Open" item (2nd item)	*/
  Menu(FileMenu, 3, disabled, "\pClose/W");	/*			create "Close" item (3rd item)*/
  Menu(FileMenu, 4, disabled,mDividingLine);/*			4th item is a dividing line		*/
  Menu(FileMenu, 5, enabled, "\pQuit/Q");		/*			create "Quit" item (5th item)	*/
  Menu(EditMenu, 0, enabled, "\pEdit");			/*Edit menu: create menu name (menu 2)*/
  Menu(EditMenu, 1, disabled, "\pUndo/Z");	/*			The Undo, Cut, Copy, Paste,		*/
  Menu(EditMenu, 2, disabled,mDividingLine);/*			and Clear items are all				*/
  Menu(EditMenu, 3, disabled, "\pCut/X");		/*			disabled.  They are						*/
  Menu(EditMenu, 4, disabled, "\pCopy/C");	/*			automatically enabled as			*/
  Menu(EditMenu, 5, disabled, "\pPaste/V");	/*			required when working with		*/
  Menu(EditMenu, 6, disabled, "\pClear");		/*			editing fields.  Your					*/
  Menu(EditMenu, 7, disabled,mDividingLine);/*			application can also manually	*/
  Menu(EditMenu, 8, disabled, "\pShow Clipboard�");	/*	enable/disable these items*/
  																					/*			when working with other				*/
  																					/*			objects such as graphics.			*/
																						/*				The Edit menu is needed for	*/
																						/*			desk accessories and editing	*/
																						/*			fields.  More items can be		*/
																						/*			added to the Edit menu, and		*/
																						/*			your application has full			*/
																						/*			control of these items.				*/
  Menu(DemosMenu, 0, enabled, "\pDemos");		/*Demos:	The "Demos" menu lets you		*/
  Menu(DemosMenu, 1, enabled, "\pLists, Buttons and Cursor Zones/1");	/*	open any*/
  Menu(DemosMenu, 2, enabled, "\pFields, Zooming and Scroll Bars/2");	/*	of the	*/
  Menu(DemosMenu, 3, enabled, "\pIcons and Cursor Zones/3");					/*	windows	*/
  Menu(DemosMenu, 4, enabled, "\pCursor Animation/4");								/*	that		*/
  Menu(DemosMenu, 5, enabled, "\pColor Drawing and Multiple Monitors/5");	/*	were*/
  Menu(DemosMenu, 6, enabled, "\pDynamic Alert Boxes/6");							/*	created	*/
	Menu(DemosMenu, 7, enabled, "\pPop-Up Menus/7");		/*to demonstrate a few of		*/
	Menu(DemosMenu, 8, enabled, "\pPicture Buttons/8");	/*	Tools Plus's features.	*/
  Menu(DemosMenu, 9, disabled, mDividingLine);				/*													*/
  Menu(DemosMenu, 10, enabled, "\pShow Tool Bar");		/*													*/
																					  /*This item will become an hierarchic	*/
																					  /*	menu�															*/
  Menu(DemosMenu, 11, enabled, "\pFloating Palettes (Windoids)");		/*						*/
																					  /*Create an hierarchic menu by				*/
																					  /*	defining the menu items�					*/
  Menu(PaletteDemoMenu, 1, enabled, "\pVertical Orientation");		/*							*/
  Menu(PaletteDemoMenu, 2, enabled, "\pHorizontal Orientation");	/*							*/
  AttachMenu(DemosMenu, 11, PaletteDemoMenu);/*Attach the hierarchic "palettes"		*/
  																					/*	menu to the 11th item in the			*/
  																					/*	"Demos" pull-down menu.						*/
  UpdateMenuBar();													/*Draw the menu bar with all its menu	*/
  																					/*	names.														*/

																						/* E d i t i n g   F i e l d s :	-	-	*/
  hField1 = (Handle)NewStrHandle(30);				/*Allocate memory for the editing			*/
  hField2 = (Handle)NewStrHandle(255);			/*	fields' strings.  Tools Plus			*/
  hField3 = (Handle)NewStrHandle(255);			/*	fields reference their related		*/
  																					/*	string by this handle.						*/

																						/*Display an alert that tells the user*/
																						/*	how to use the demo�							*/
	AlertBox(WatersEdgeLogo, "\pWelcome to the Tools Plus demo.\r\rUse the �Demos� menu to explore each of the demos prepared for you.  Feel free to play around with the various objects.\r\rClick �Continue� to start the first demo.", SilentContinueAlert);
  OpenDemoWindow1();												/*Open the first demo window					*/
  ResetCursor();														/*Get rid of the wrist watch cursor		*/
  																					/*	by resetting it to its proper			*/
  																					/*	shape (Tools Plus figures out its	*/
  																					/*	proper shape depending on where		*/
  }																					/*	it is on the screen).							*/










/***********************************************************************************
 ***********************************************************************************
 *  A c t i o n   i n   a   d e m o   W i n d o w :
 *	The following routines respond to actions made by the user in a demo window.
 *	They are invoked by the main event loop whenever the user does any of the
 *	following things:
 *		� Click a button											(doButton event)
 *		� Type a key													(doKeyDown or doAutoKey event,
 *																					 excluding those in editing fields)
 *		� Click in an inactive editing field	(doClickField event)
 *		� Use a scroll bar										(doScrollBar event, excluding those
 *																					 in List Boxes)	
 *		� Click a line in a List Box					(doListBox)
 *		� Use a Pop-Up Menu										(doPopUpMenu)
 *		� Click in the window									(doClick, excluding clicking on other
 *																					 objects listed above)
 */




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C l o s e   A   W i n d o w :
 *	This routine is called in response to a "doGoAway" event, the File menu's Close
 *	command, and any from some demos that end when you click the Cancel or OK
 *	button.  Note that a doGoAway event is not generated when a desk accessory's
 *	"close" box is clicked.
 *		A doGoAway event indicates that the user clicked the active window's "close
 *	box".  In a full featured application, you may want to validate the active
 *	editing field and perform any other verifications before closing the window.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CloseTheWindow (short Window)					/*Window number being closed					*/
	{
	WindowClose(Window);											/*Close the specified window					*/
	if (FirstWindowNumber() == none)					/*If this application does not have		*/
																						/*	any open windows�									*/
		EnableMenu(FileMenu, CloseItem, disabled);	/*Disable the File menu's Close		*/
																								/*	command, because all the			*/
	}																							/*	windows are closed.						*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   1 :
 *	This routine responds to actions made by the user in demo window 1.  The actions
 *	include:
 *		� Clicking or double-clicking a line in a List Box
 *		� Clicking a check box
 *		� Clicking or double clicking a radio button
 *		� Clicking a push-button
 *	Notice that if you type Enter or Return, Tools Plus automatically selects the
 *	default push button (OK).
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow1 (void)
	{
	Boolean			CheckedBox;										/*Was the clicked check box originally*/
																						/*	selected?													*/
  short				ButtonNum;										/*Button counter for stepping through */
  																					/*	as set of buttons.								*/
  short				NumberOfSounds;								/*Number of sounds in the System file	*/
  short				SoundIndex;										/*Relative sound number going to be		*/
  																					/*	played.														*/
  Handle			hSound;												/*Handle to a sound resource					*/
  short				OSerr;												/*Macintosh operating system error		*/


  switch (Poll.What)												/*Determine what the user did (what		*/
  	{																				/*	event)�														*/

		case doButton: 				 			/* B u t t o n   W a s   S e l e c t e d :	- - -	*/
																/*This section demonstrates typical interaction		*/
																/*	with check boxes, radio button groups and			*/
																/*	push buttons.  Look at the radio-button group	*/
  															/*	for "double-clicking" ability.								*/

    	switch (Poll.Button.Num) {						/*Determine which button was selected�*/
		    case MiniButton1:case MiniButton2:	/* M i n i - B u t t o n s :	-		-		*/
		    case MiniButton3:case MiniButton4:	/*The mini buttons simply play one of	*/
		    case MiniButton5:										/*	the sounds available in the System*/
      																			/*	file.															*/
					NumberOfSounds = CountResources('snd ');	/*Determine the number of			*/
																										/*	sounds available.					*/														
					if ((NumberOfSounds == none) || (SystemVersion() < 6.0))	/*If there are*/
						Beep();													/*	no sounds available, or if the		*/
																						/*	System version does not support		*/
																						/*	the Sound Manager (older than			*/
																						/*	System 6), just beep the user.		*/
					else															/*If there are sounds available in the*/
						{																/*	System, calculate a "sound index"	*/
																						/*	number such that button 1 is the	*/
																						/*	first sound, button 2 is the			*/
																						/*	second, etc�											*/
       			SoundIndex = max(1, (Poll.Button.Num - (MiniButton1 - 1)) % NumberOfSounds);
      			hSound = GetIndResource('snd ', SoundIndex);/*Load the sound resource	*/
      																									/*	from the System file	*/
      																			/*Play the sound resource (ignore	the	*/
     				OSerr = SndPlay(nil, (SndListHandle)hSound, false);	/*	error code)		*/
      			ReleaseResource(hSound);				/*Release the resource to conserve		*/
       																			/*	memory.														*/
      			}																/*																		*/
      		break;														/*																		*/
      	
      	
				case CheckBox1:case CheckBox2:			/* C h e c k   B o x e s :	-		-		-	*/
				case CheckBox3:											/*When a check box is clicked, it			*/
																						/*	reverses its current state (If on,*/
																						/*	turn off.  If off, turn on). In		*/
																						/*	this example, the third check box	*/
																						/*	can only be selected if the second*/
																						/*	one is selected.									*/
					CheckedBox = ButtonIsSelected(Poll.Button.Num);	/*Determine if check box*/
																						/*	was originally selected.					*/
       		CheckedBox = !CheckedBox;					/*Reverse the check box's selection		*/
       		SelectButton(Poll.Button.Num, CheckedBox);	/*	(if on, turn	off.  If	*/
       																								/*	off, turn on.)					*/
					if (Poll.Button.Num == CheckBox2)	/*If the second check box was clicked�*/
						{																/*	enable/disable button CheckBox3.	*/
       			if (CheckedBox)									/*	This shows how easy it is to set	*/
       				EnableButton(CheckBox3, enabled);/*	up an "and" condition with two	*/
       																			/*	or more check boxes.							*/
       			else														/*		Check box 3 is deselected and		*/
       				{															/*	disabled when check box 2 is not	*/
       				EnableButton(CheckBox3, disabled);		/*	selected.  When check box	*/
       				SelectButton(CheckBox3, notSelected);	/*	2 is selected, then check	*/
     			 	 	}																			/*	box 3 is enabled.					*/
						}																				/*														*/
					break;																		/*														*/
					
					
				case RadioButton1:case RadioButton2:/* R a d i o   B u t t o n s :	-		-	*/
				case RadioButton3:									/*Radio buttons are logically					*/
																						/*	organized into groups, and only		*/
																						/*	one radio button can be on at a		*/
																						/*	time.  Your application can				*/
																						/*	optionally do something else if a	*/
																						/*	radio button is double- clicked.	*/
																						/*	In this case, it is the equivalent*/
																						/*	of selecting the radio button and	*/
																						/*	clicking OK.											*/
					for (ButtonNum = RadioButton1; ButtonNum <= RadioButton3; ButtonNum++)
																						/*Cycle through each button in the		*/
																						/*	group�														*/
       			SelectButton(ButtonNum, ButtonNum == Poll.Button.Num);	/*Select the	*/
       																			/*	clicked one, deselect the others.	*/
       		if (Poll.Button.DoubleClick)			/*If a radio button was double-clicked*/
      			{																/*	it means "select this button and	*/
      			FlashButton(OkButton);					/*	OK".  Flash the OK button to			*/
      			CloseTheWindow(DemoWindow1);		/*	indicate it's being selected.			*/
       			}																/*	End this demo by closing its			*/
      		break;														/*	window.														*/


				case CancelButton:case OkButton:		/* P u s h   B u t t o n s :	-		-		*/
    			CloseTheWindow(DemoWindow1);			/*If the user clicked Cancel or OK�		*/
      																			/*	end this demo by closing its			*/
      		break;														/*	window.														*/

			}																			/*																		*/
			break;																/* (end of doButton responses)				*/


		case doListBox: 				 		/* L i s t   B o x e s :	- - - - - - - - - - - -	*/
																/*Your application may want to respond to a user	*/
																/*	selecting a line in a List Box.  This demo		*/
																/*	just responds to double-clicks by being the		*/
																/*	equivalent of selecting the line and clicking	*/
																/*	OK.																						*/
    	if (Poll.ListBox.DoubleClick)					/*If a list box was double-clicked�		*/
     		{																		/*	it means "select this line and		*/
     		FlashButton(OkButton);							/*	OK".  Flash the OK button to			*/
     																				/*	indicate it's being selected. End	*/
      	CloseTheWindow(DemoWindow1);				/*	this demo by closing its window.	*/
     		}																		/*																		*/
     	break;																/*																		*/

		default:;																/*All other events are ignored in this*/
  	}																				/*	demo.															*/
  }																					/*																		*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   2 :
 *	This routine responds to actions made by the user in demo window 2.  The
 *	actions include:
 *		� Clicking or double-clicking in an inactive editing field
 *		� Using Scroll Bars
 *		� Tabbing to the next (or previous) field
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow2 (void)
	{
  short				Field;												/*Field number												*/


  switch (Poll.What)												/*Determine what the user did (what		*/
  	{																				/*	event)�														*/

 		case doClickField:					/* I n a c t i v e   F i e l d   C l i c k e d :	*/
																/*In a full-featured application, you would likely*/
																/*	validate a field for errors before proceeding	*/
																/*	to the one clicked by the user.  In this demo,*/
																/*	no validation is done.												*/
    														/*												*/
			SaveFieldString();										/*Save the field's edited text as the	*/
																						/*	field's string.										*/
			ClickInField();												/*Process the event to activate the		*/
																						/*	clicked field and to set the			*/
																						/*	insertion point or selection range*/
			break;																/*	as per the click.									*/


		case doScrollBar:						/* S c r o l l   B a r   W a s   U s e d :	-	-	-	*/
																/*Tools Plus tells your application when a scroll	*/
																/*	bar is used by either clicking in the up			*/
																/*	arrow, down arrow, "page up" region, "page		*/
																/*	down" region, or by moving the thumb.  Your		*/
																/*	application would then respond by doing				*/
																/*	something.  This demo doesn't actually scroll	*/
																/*	anything.																			*/
																/*		Note: When setting a Scroll Bar's value,		*/
																/*	the minimum / maximum limit are automatically	*/
																/*	adjusted (if necessary).  Here we use the			*/
																/*	limits to prevent the current value from			*/
																/*	being exceeded.																*/
    	switch (Poll.ScrollBar.Part)					/*Appropriate action is taken					*/
    		{																		/*	according to the part of the			*/
    																				/*	scroll bar that was clicked�			*/
    		case inUpButton: 										/*																		*/
    		  SetScrollBarVal(Poll.ScrollBar.Num, max(GetScrollBarMin(Poll.ScrollBar.Num), GetScrollBarVal(Poll.ScrollBar.Num) - 1));
    		  break;														/*																		*/
    		case inPageUp:											/*																		*/
      		SetScrollBarVal(Poll.ScrollBar.Num, max(GetScrollBarMin(Poll.ScrollBar.Num), GetScrollBarVal(Poll.ScrollBar.Num) - 10));
					break;														/*																		*/
     		case inDownButton:									/*																		*/
      		SetScrollBarVal(Poll.ScrollBar.Num, min(GetScrollBarMax(Poll.ScrollBar.Num), GetScrollBarVal(Poll.ScrollBar.Num) + 1));
					break;														/*																		*/
     		case inPageDown:										/*																		*/
      		SetScrollBarVal(Poll.ScrollBar.Num, min(GetScrollBarMax(Poll.ScrollBar.Num), GetScrollBarVal(Poll.ScrollBar.Num) + 10));
					break;														/*																		*/
				case inThumb:												/*Ignore thumb movement								*/
    			break;														/*																		*/
    		}																		/*																		*/
    	break;																/*	(end of scroll bar response)			*/


		case doKeyDown:case doAutoKey:/* U s e r   T y p e d   a   K e y :	-	-	-	-	-	*/
																	/*If the active window has an active editing		*/
																	/*	field, it automatically processes most key	*/
																	/*	strokes.  In this example, we only check		*/
																	/*	for the Tab key, which is used to tab to		*/
																	/*	the next or previous field (Shift-Tab)�			*/
			if ((Poll.Key.Chr == TabKey) && !(Poll.Modifiers.Bits.CmdKey || Poll.Modifiers.Bits.OptionKey || Poll.Modifiers.Bits.ControlKey))
				{																		/*If the Tab key was typed without the*/
																						/*	Command, Option or Control				*/
																						/*	modifier keys, tab to the next /	*/
																						/*	previous field.										*/
																						/*		In a full-featured application,	*/
																						/*	you would likely validate a field	*/
																						/*	for errors before proceeding to		*/
																						/*	the field clicked by the user. In	*/
																						/*	this demo, no validation is done.	*/
      	SaveFieldString();									/*Save the field's edited text as the	*/
      																			/*	field's string.										*/
      	Field = ActiveFieldNumber();				/*Determine the active field number		*/
      	if (!Poll.Modifiers.Bits.ShiftKey)	/*TAB: to next field�									*/
       		Field = Field + 1 - 3 * (Field == 3);			/*Add 1.  If field=3, start		*/
       																							/*	at 1 again.								*/
     		else																/*SHIFT-TAB: to previous field�				*/
					Field = Field - 1 + 3 * (Field == 1);			/*Subtract 1.  If field=1,		*/
																										/*	start at 3.								*/
     		ActivateField(Field, teSelectAll);	/*Select all the text in the newly		*/
     																				/*	activated field.									*/
				}																		/*																		*/
			break;																/*																		*/
				
		default:;																/*All other events are ignored				*/
 		}																				/*																		*/
 	}																					/*																		*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   3 :
 *	This routine responds to actions made by the user in demo window 3.  The
 *	actions include:
 *		� Clicking on an icon (Cursor Zone over the icon)
 *		� Clicking a radio button
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow3 (void)
	{
  short				ClickedZone;									/*Cursor zone clicked by user					*/
  short				Button;												/*Button number counter								*/


  switch (Poll.What)												/*Determine what the user did (what		*/
  	{																				/*	event)�														*/

 		case doClick:								/* U s e r   C l i c k e d   I n   T h e					*/
 																/*		W i n d o w :																*/
																/*The only click we care about in this window is	*/
																/*	if it occurs in either of the two icons�			*/
    	ResetMouseClicks();										/*Only 1st mouse-down is needed.			*/
    																				/*	Ignore the rest.									*/
			ClickedZone = FindCursorZone(Poll.Mouse.Down[0].Where);	/*Determine which		*/
																						/*	cursor zone was clicked by the		*/
																						/*	user.															*/
     	if ((ClickedZone != 0) && (ClickedZone != MidiPort))	/*If a Cursor Zone		*/
				{																		/*	was clicked, and the clicked zone	*/
     																				/*	is different from the currently		*/
     																				/*	selected MIDI port (ie: it was		*/
     																				/*	changed)�				*/
				if (ClickedZone == PrinterIcon)			/*Depending on which icon was clicked,*/
					MidiPort = PrinterIcon;						/*	set the MidiPort variable to the	*/
																						/*	selected icon.										*/
       	else																/*																		*/
       		MidiPort = ModemIcon;							/*																		*/
       																			/*Redraw the printer and modem icons	*/
       																			/*	as currently selected�						*/
				DrawIcon(PrinterIcon, 20, 32, enabled, MidiPort == PrinterIcon);	/*			*/
				DrawIcon(ModemIcon, 65, 32, enabled, MidiPort == ModemIcon);			/*			*/
				}																		/*																		*/
			break;																/*																		*/


		case doButton:							/* U s e r   C l i c k e d   A   B u t t o n :	-	*/
			for (Button = MidiHalfMeg; Button<=MidiTwoMeg; Button++)	/*Cycle through		*/
	      SelectButton(Button, Button == Poll.Button.Num);				/*	all 3 speed		*/
	      																		/*	buttons and set only the selected	*/
	      																		/*	one.  Turn the others off.				*/
			MidiSpeed = Poll.Button.Num;					/*Keep track of the selected button		*/
     	break;																/*																		*/

		default:;																/*Ignore all other events							*/
  	}																				/*																		*/
	}																					/*																		*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   4 :
 *	This routine responds to actions made by the user in demo window 4. This routine
 *	is here for cosmetic reasons only, and you can get rid of it if you want.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow4 (void)
	{
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   5 :
 *	This routine responds to actions made by the user in demo window 5.  The only
 *	thing the user can do is click the default OK button, which closes the window.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow5 (void)
	{
  if (Poll.What == doButton)								/*If the user clicked a button�				*/
		CloseTheWindow(DemoWindow5);						/*	end this demo by closing its			*/
	}																					/*	window.														*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   6 :
 *	This routine responds to actions made by the user in demo window 6.  Dynamic
 *	Alerts take care of responding to any action from the user, so you don't have
 *	to write any code for it.  This routine is here for cosmetic reasons only, and
 *	you can get rid of it if you want.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow6 (void)
	{
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   7 :
 *	This routine responds to actions made by the user in demo window 7.  The only
 *	thing you can do is select an item in a pop-up menu.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow7 (void)
	{
	#define			SilentContinueAlert	-11500		/*Button definition for a "Continue"	*/
																						/*	dynamic alert that doesn't beep		*/
																						/*	the user.													*/	
	
  if (Poll.What == doPopUpMenu)							/*The only thing you can do: use a		*/
  	{																				/*	pop-up menu.											*/
  
		switch (Poll.Menu.Num)									/*Determine which Pop-Up Menu was			*/
			{																			/*	selected�													*/

    	case popMenu1: case popMenu5: 				/*Pop-Up Menus 1 and 5 use a bullet		*/
    																				/*	(�) to mark a selected item.			*/
				PopUpMark(Poll.Menu.Num, Poll.Menu.Item, DotChar);	/*Mark new selection	*/
				break;																							/*										*/
				
			case popMenu2: case popMenu3: 				/*Pop-Up Menus 2 and 3 use a check (�)*/
																						/*	to mark	a selected item.					*/
				CheckPopUp(Poll.Menu.Num, Poll.Menu.Item, on);	/*Mark the new selection	*/
				break;																					/*												*/

			case popMenu4: 												/*This menu is a "fixed title" pop-		*/
																						/*	down menu. Such menus are usually	*/
																						/*	used to "do some-thing now". This */
																						/*	demo just displays an alert.			*/
				AlertBox(WatersEdgeLogo, "\pYour application would do something now, such as displaying a dialog that lets the user specify formatting characteristics.", SilentContinueAlert);
				break;															/*																		*/
			}																			/*																		*/
		}																				/*																		*/
	}																					/*																		*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   8 :
 *	There are three routines for responding to events in this window.  The first two
 *	are button value drawing routines that display a button's value.  They are
 *	called when the button is clicked, or when the window is refreshed.

 *	This routine displays a value in the tool bar.  It is called whenever the "plus"
 *	or "minus" buttons are used, and when the screen is refreshed.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow8_DrawToolBarValue (void)
	{
	char				theNumber[5];									/*Button's value, string format				*/
	
	
	stoa(ToolBarValue, theNumber);						/*Convert tool bar number to a string	*/
  TextFont(systemFont);											/*Set window's font to Chicago 12pt.	*/
  TextSize(12);															/*																		*/
  PenColorNormal();													/*PEN: 1x1 size, black, patCopy mode	*/
    																				/*Draw the value in the window's tool	*/
    																				/*	bar with a frame around it�				*/
  TextInBox(theNumber, 401, 6, 439, 22, teJustCenter, true);	/*									*/
 	}																														/*									*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	This routine displays the value of the "Globe" button.  It is called whenever
 *	the "Globe" button is used, and when the screen is refreshed.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow8_DrawGlobeValue (void)
	{
	short				ButtonValue;									/*Globe button's value								*/
	char				theNumber[6];									/*Button's value, string format				*/


	ButtonValue = GetPictButtonVal(GlobeButton);	/*Get the "globe" button's value	*/
  stoa(ButtonValue, theNumber);							/*Convert to a string									*/
	strcat(theNumber, "�");										/*Add degree symbol to end of string	*/
  TextFont(systemFont);											/*Set window's font to Chicago 12pt.	*/
  TextSize(12);															/*																		*/
  																					/*Draw value below the "globe" button	*/
  TextInBox(theNumber, 408, 201, 448, 217, teJustRight, false);	/*								*/
	}																															/*								*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  This routine responds to actions made by the user in demo window 8.  This
 *	window contains mostly Picture Buttons, so the response is limited to the user
 *	clicking on these buttons.  Notice that the buttons are organized into different
 *	types: simple binary clusters, multi-stage buttons, radio button clusters, etc.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow8 (void)
	{
	short				theButton;										/*Button counter											*/


  switch (Poll.What)												/*Determine what the user did (what		*/
		{																				/*	type of event)�										*/


		case doPopUpMenu:								/*P o p - U p   M e n u   S e l e c t e d :		*/
			PopUpMark(Poll.Menu.Num, Poll.Menu.Item, '�');	/*Check selected item				*/
																											/*	(others are unchecked)	*/
			if (Poll.Menu.Num == ButtonSpeedMenu)	/*If this menu regulate's the "globe"	*/
																						/*	button speed�											*/
				SetPictButtonAccel(GlobeButton, Poll.Menu.Item - 1);	/*Set the button's	*/
																															/*acceleration curve*/
			break;																									/*									*/

		case doPictButton:						/* P i c t u r e   B u t t o n   S e l e c t e d*/
    	switch (Poll.Button.Num)		/* Determine which picture button was selected�	*/
    		{
																						/*This section demonstrates typical		*/
																						/*	interaction with check boxes,			*/
																						/*	radio button groups and push			*/
																						/*	buttons.													*/
				case LeftAlignButton: case CenterAlignButton:	/*													*/
				case RightAlignButton: case JustifyButton:		/*													*/
																						/*Cycle through the group and turn off*/
																						/*	the one that weren't clicked			*/
																						/*	(deselect them)�									*/
					for (theButton = LeftAlignButton; theButton<=JustifyButton; theButton++)
						SelectPictButton(theButton, theButton == Poll.Button.Num);	/*				*/
					break;																												/*				*/

																						/*This is the simplest case for				*/
																						/*	picture buttons, clickable icons�	*/
				case PrinterButton: case ModemButton:/*																		*/
																						/*Only 1 icon can be on, so turn the	*/
																						/*	other one off�										*/		
					for (theButton = PrinterButton; theButton<=ModemButton; theButton++) /*	*/
						SelectPictButton(theButton, theButton == Poll.Button.Num);				 /*	*/
					break;																															 /*	*/

																						/*Tape Deck buttons: Because each			*/
				case RewindButton: case StopButton:		/*	button locks down (if required)	*/
				case FastFwdButton: case PauseButton:	/*	by itself, we just have to			*/
				case RecordButton: case PlayButton:		/*	deselect any other button that	*/
																							/*	may be down�										*/
																							/*Cycle through all the buttons�		*/
					for (theButton = RewindButton; theButton<=PlayButton; theButton++)	/*	*/
						if (theButton != Poll.Button.Num)	/*If this button was not clicked�		*/
							SelectPictButton(theButton, notSelected);	/*	deselect the button.	*/
					break;																				/*												*/	


				case GlobeButton: case -GlobeButton:/*If the globe button was selected		*/
					ActionInWindow8_DrawGlobeValue();	/*	and/or held,show the new value.		*/
																						/*	The negative case is for the			*/
																						/*	decrementing side of the button.	*/
					break;														/*																		*/


				case PlusButton: 										/*"Plus" button in window's tool bar:	*/
					if (ToolBarValue < 3200) {				/*If the current value is less that		*/
																						/*	3200 (the upper limit)�						*/
						ToolBarValue = ToolBarValue * 2;/*Double the "zoom factor"						*/
						ActionInWindow8_DrawToolBarValue();	/*Redisplay the tool bar's number	*/
					}																			/*																*/
					break;																/*																*/

				case MinusButton:										/*"Minus" button in window's tool bar:*/
					if (ToolBarValue > 25) {					/*If the current value is greater than*/
																						/*	25 (the lower limit)�							*/
						ToolBarValue = ToolBarValue / 2;/*Half the "zoom factor"							*/
						ActionInWindow8_DrawToolBarValue();	/*Redisplay the tool bar's number	*/
					}																			/*																*/
					break;																/*																*/


				case DoneButton: 										/*If user clicked the "Done" button�	*/
					CloseTheWindow(DemoWindow8);			/*	end this demo by closing window.	*/
					break;														/*																		*/

				default:														/*Ignore all other buttons						*/
					break;														/*																		*/
				}																		/*																		*/

			default:;															/*Ignore all other events							*/
			}																			/*																		*/
	}																					/*																		*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   9 :
 *	This routine responds to actions made by the user in demo window 9 (the Tool
 *	Bar).  You can select an item in Pop-Up Menu or click picture buttons, but the
 *	picture buttons don't do anything in this demo.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow9 (void)
	{
	short				theButton;										/*Button counter											*/
	
		
  if (Poll.What == doPopUpMenu)							/*If the pop-up menu was used�				*/
		PopUpMark(Poll.Menu.Num, Poll.Menu.Item, '�');	/*	check selected item				*/
																										/*		(others are unchecked)	*/

																						/*If one of the 4 "single selection"	*/
																						/*	buttons was selected�							*/
  else if ((Poll.What == doPictButton) && (Poll.Button.Num >= LeftAlignButton) && (Poll.Button.Num <= JustifyButton)) {
																						/*Cycle through the group and turn off*/
																						/*	the ones that weren't clicked			*/
																						/*	(deselect them)�									*/
		for (theButton = LeftAlignButton; theButton <= JustifyButton; theButton++){		
    	SelectPictButton(theButton, (theButton == Poll.Button.Num));
			}
		}
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   10 :
 *	This routine responds to actions made by the user in demo window 10 (a vertical
 *	palette).  All the user can do is click a picture button, but the picture
 *	buttons don't do anything in this demo.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow10 (void)
	{
	short				theButton;										/*Button counter											*/
	
		
  if (Poll.What == doPictButton) {					/*If user clicked on a picture button�*/		
																						/*Cycle through the group and turn off*/
																						/*	the ones that weren't clicked			*/
																						/*	(deselect them)�									*/
		for (theButton = VPaletteMinButton; theButton <= VPaletteMaxButton; theButton++){		
    	SelectPictButton(theButton, (theButton == Poll.Button.Num));
    	}
		}
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * A c t i o n   i n   D e m o   W i n d o w   11 :
 *	This routine responds to actions made by the user in demo window 11 (a
 *	horizontal palette).  All the user can do is click a picture button, but the
 *	picture buttons don't do anything in this demo.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ActionInWindow11 (void)
	{
	short				theButton;										/*Button counter											*/
	
		
  if (Poll.What == doPictButton) {					/*If user clicked on a picture button�*/		
																						/*Cycle through the group and turn off*/
																						/*	the ones that weren't clicked			*/
																						/*	(deselect them)�									*/
		for (theButton = HPaletteMinButton; theButton <= HPaletteMaxButton; theButton++){		
    	SelectPictButton(theButton, (theButton == Poll.Button.Num));
    	}
		}
	}







/***********************************************************************************
 ***********************************************************************************
 *  D r a w   D e m o   W i n d o w s '   C o n t e n t s :
 *	The following routines are used to draw a demo windows' contents.  They are
 *	invoked when a demo window is first opened, and when a demo window needs to be
 *	refreshed.  The contents includes text and lines.  It excludes Tools Plus
 *	objects such as Buttons, Scroll Bars, List Boxes, Editing Fields, etc.  Those
 *	objects are refreshed automatically.
 */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   T e x t   i n   a   B o x :
 *	This generic routine performs an often used task of drawing text within a
 *	specified rectangle.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void TextInBox (char Str[],									/*The string that's going to be drawn	*/
       short left, short top, short right, 	/*Box's co-ordinates									*/
       short bottom,												/*																		*/
       short Justification,									/*Left-aligned, centered, or right-		*/
       																			/*	aligned.													*/
       Boolean withBox)											/*Draw a box around the text?					*/
	{
	Rect				Rectangle;										/*Box's co-ordinates specified as a		*/
  																					/*	rectangle.												*/


  SetRect(&Rectangle, left, top, right, bottom);	/*Convert the specified co-			*/
  																								/*	ordinates into a rectangle.	*/
  TextBox(Str, strlen(Str), &Rectangle, Justification);	/*Draw the text in the		*/
  																											/*	specified rectangle.	*/
  if (withBox)															/*If a box is needed around the text�	*/
		FrameRect(&Rectangle);									/*	draw the rectangle around the			*/
	}																					/*	text.															*/



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C o n v e r t   a   s h o r t   t o   a   s t r i n g :
 *	This generic routine converts a short number into a C string which can then be
 *	drawn on a window.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void stoa (short theShort, char theString[])
	{
	short sign;																/*The short's sign (positive/negative)*/
	short ByteCount;													/*Byte counter in string							*/
	short ReverseByte;												/*Reverse byte counter in string			*/
	short theChar;														/*Single character in the string			*/

	if ((sign = theShort) < 0)								/*Take note of the short's sign				*/			
		theShort = -theShort;										/*Make the number positive						*/

	ByteCount = 0;														/*Initialize the bytes counter				*/	
	do{																				/*Generate digits in reverse order�		*/
		theString[ByteCount++] = theShort % 10 + '0';	/*Get the next digit						*/
	} while ((theShort /= 10) > 0);									/*Delete digit									*/
	if (sign < 0)															/*If sign is negative�								*/
		theString[ByteCount++] = '-';						/*	add a minus sign.									*/
	theString[ByteCount] = '\0';							/*Null-terminate the C string					*/
		
																						/*Reverse the string into the proper	*/
																						/*	sequence of digits�								*/
	for (ByteCount = 0, ReverseByte = strlen(theString)-1; ByteCount < ReverseByte; ByteCount++, ReverseByte--){
		theChar = theString[ByteCount];
		theString[ByteCount] = theString[ReverseByte];
		theString[ReverseByte] = theChar;
	}		
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   a   G r o u p   B o x   w i t h   a   T i t l e :
 *	This generic routine performs an often used task of drawing a "Group Box" such
 *	as the kind that is seen around a radio-button group.  It also draws the
 *	group's title at the top of the box.  The box is drawn using the pen's current
 *	size, pattern and transfer mode.  The group's title is drawn using the window's
 *	current font, size, and style.	
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 void GroupBox (Str255 Str,									/*Group's name (Pascal string)				*/
       short left, short top, short right,	/*Co-ordinates of the group's					*/
       short bottom)												/*	enclosing box.										*/
	{
	Rect				Rectangle;										/*Box's co-ordinates specified as a		*/
																						/*	rectangle.												*/
	Rect				TextRect;											/*Enclosing ractangle where box will	*/
																						/*	appear.														*/
	short					Width;											/*Text's width (for centering it)			*/


  SetRect(&Rectangle, left, top, right, bottom);	/*Convert the specified co-			*/
  																								/*	ordinates into a rectangle.	*/
  FrameRect(&Rectangle);										/*Draw the group's box								*/
  if (Str[0] != 0)													/*If a string was included as the			*/
  	{																				/*	group's title, it will be centered*/
  																					/*	at the top�												*/
  	Width = StringWidth(Str);								/*Calculate the text's width in				*/
  																					/*	pixels.														*/
    TextRect.left = (Rectangle.left + Rectangle.right) / 2 - (Width / 2) - 2;	/*	*/
    TextRect.right = TextRect.left + Width + 4;	/*	Calculate a rectangle so that	*/
    TextRect.top = Rectangle.top - 8;						/*	it fits on the top edge of the*/
    TextRect.bottom = TextRect.top + 16;				/*	group's box.  It's wide enough*/
    																						/*	for the title, plus a 2 pixel	*/
    																						/*	border on each side.					*/
    TextBox(&Str[1], Str[0], &TextRect, teJustCenter);	/*Draw the group's title	*/
		}																										/*												*/
	}																											/*												*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   1 :
 *	This routine draws all the text and lines in demo window 1.  It is called when
 *	first creating the window and when the window needs to be refreshed.  Note that
 *	objects that are created by Tools Plus are refreshed automatically.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow1Contents (void)
	{
																						/* C o m m e n t s	-	-	-	-	-	-	-	-	-	*/
  PenPat(&qd.gray);													/*Use gray pattern for drawing framing*/
  																					/*	boxes.														*/
  TextFont(geneva);													/*Draw all comments using Geneva 9pt,	*/
  TextSize(9);															/*	plain style.											*/
  TextFace(0);															/*																		*/
																						/*Comments for left list box�					*/
  TextInBox("This left text list uses the standard system font, just like normal list boxes.", 38, 108, 236, 140, teJustLeft, false);
																						/*Comments for right list box�				*/
  TextInBox("This one uses Geneva 9pt.  Each list box can have its own font.", 295, 108, 416, 150, teJustLeft, false);
																						/*Comments for Cursor Zones�					*/
  TextInBox("This is a \"Plus Cursor\" zone.  The cursor changes when entering this area.", 275, 150, 400, 189, teJustCenter, true);
  TextInBox("This is a \"Cross Cursor\" zone.  The cursor changes when entering this area.", 275, 193, 400, 232, teJustCenter, true);
																						/*Comments for mini-buttons�					*/
  TextInBox("These buttons are drawn in bold Geneva 9pt.  Many different fonts, sizes and styles can be used.", 120, 140, 240, 190, teJustLeft, false);
  PenSize(3, 3);														/*Make the pen 3x3 pixels for a fatter*/
  																					/*	line.															*/
  GroupBox("\p", 14, 137, 242, 192);				/*Draw a gray, fat box around the mini*/
  																					/*	buttons.													*/
																						/*Comment at bottom of the window�		*/
  TextInBox("Double-click a radio button or a line in a list box to mean �select this and click �OK�.�  (Note that the OK button flashes.)  This OPTION is easily implemented.", 5, 290, 420, 320, teJustCenter, false);


																						/* L i s t   B o x   T i t l e s	-	-	*/
  TextFont(systemFont);											/*Set the window's font to Chicago		*/
  TextSize(12);															/*	12pt.															*/
  MoveTo(5, 19);														/*Draw the word "Left" at local co-		*/
  DrawString("\pLeft:");										/*	ordinates 5,19 for the left list	*/
  																					/*	box.															*/
  MoveTo(253, 19);													/*Draw the word "Right" at local co-	*/
  DrawString("\pRight:");										/*	ordinates 253,19 for the right		*/
  																					/*	list box.													*/

																						/* R a d i o   B u t t o n						*/
																						/*		G r o u p ' s   T i t l e	-	- -	*/
  PenNormal();															/*Reset pen pattern to black, and pen	*/
  																					/*	size to 1x1.											*/
  GroupBox("\pChoose", 165, 212, 231, 273);	/*Draw a box around the radio button	*/
  }																					/*	group and give the group a title.	*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   2 :
 *	This routine draws all the text and lines in demo window 2.  It is called when
 *	first creating the window and when the window needs to be refreshed.  Note that
 *	objects that are created by Tools Plus are refreshed automatically.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow2Contents (void)
	{
	Rect						Rectangle;								/*Rectangle for drawing comments			*/
																						/*	(text).														*/
  																					

																						/* Comment above fields	-	-	-	-	-	-	-	*/
  TextFont(geneva);													/*Draw all comments using Geneva 9pt.	*/
  TextSize(9);															/*																		*/
  PenPat(&qd.gray);													/*Any line drawing will be done with	*/
  																					/*	a gray pattern.										*/
  TextInBox("Click the \"zoom box\" to zoom between the standard and user co-ordinates.\r\rThe scroll bars don't actually scroll anything in this demo.  But notice that they are automatically repositioned when you resize the window.", 10, 2, 191, 95, teJustLeft, false);

																						/* Comment for 1st field	-	-	-	-	-	-	*/
  TextInBox("The first field is \"length limited.\"  It accepts a maximum of 30 characters, (the length of the field), and does not scroll or require word-wrap.", 220, 15, 375, 77, teJustLeft, false);
  SetRect(&Rectangle, 215, 10, 380, 82);		/*Define the outline around the				*/
  																					/*	comment.													*/
  FrameRect(&Rectangle);										/*Draw a gray box around the comment	*/
  MoveTo(215, 46);													/*Draw a gray line from the first			*/
  LineTo(195, 105);													/*	field to the box that was					*/
  																					/*	previously completed.							*/

																						/* Comment for 2nd field	-	-	-	-	-	-	*/
  TextInBox("The second field is a \"single-line\" editing field.  It scrolls to keep the selection in view, and does not require word wrap.", 220, 97, 375, 146, teJustLeft, false);
  SetRect(&Rectangle, 215, 92, 380, 151);		/*Define the outline around the				*/
  																					/*	comment.													*/
  FrameRect(&Rectangle);										/*Draw a gray box around the comment	*/
  MoveTo(215, 126);													/*Draw a gray line from the second		*/
  LineTo(195, 126);													/*	field to the box that was					*/
  																					/*	previously completed.							*/

																						/* Comment for 3rd field	-	-	-	-	-	-	*/
  TextInBox("The third field is a \"multiple-line\" editing field.  It scrolls to keep the selection in view, and uses word wrap to break long words.", 220, 166, 375, 215, teJustLeft, false);
  SetRect(&Rectangle, 215, 161, 380, 220);	/*Define the outline around the				*/
  																					/*	comment.													*/
  FrameRect(&Rectangle);										/*Draw a gray box around the comment	*/
  MoveTo(215, 207);													/*Draw a gray line from the third			*/
  LineTo(195, 148);													/*	field to the box that was					*/
  																					/*	previously completed.							*/

																						/* Comments at bottom	-	-	-	-	-	-	-	-	*/
  TextInBox("Try editing the text in the fields (above).  The Edit menu automatically works with the active field (check out the Undo!)", 10, 220, 200, 260, teJustLeft, false);
  TextInBox("Notice that each editing field can have its own font!", 220, 235, 375, 260, teJustLeft, false);
  }




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   3 :
 *	This routine draws all the text and lines in demo window 3.  It is called when
 *	first creating the window and when the window needs to be refreshed.  Note that
 *	objects that are created by Tools Plus are refreshed automatically.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow3Contents (void)
	{
  TextFont(geneva);													/*Comment at top of window (drawn in	*/
  TextSize(9);															/*	Geneva 9pt).											*/
  TextInBox("This demo simulates MIDI Interface settings.  Click on an icon to select it.", 10, 100, 230, 200, teJustCenter, false);
  DrawIcon(PrinterIcon, 20, 32, enabled, MidiPort == PrinterIcon);	/*Redraw the	*/
  DrawIcon(ModemIcon, 65, 32, enabled, MidiPort == ModemIcon);			/*	printer		*/
  																					/*	and modem icons as currently			*/
  																					/*	selected.													*/
  TextFont(systemFont);											/*Box and title around icon group and	*/
  TextSize(12);															/*	radio button group (drawin in			*/
  																					/*	Chicago 12 pt.)�									*/
  GroupBox("\pPort", 7, 17, 111, 80);				/*Draw a box around the Printer and		*/
  																					/*	Modem icons, and give the group a	*/
  																					/*	title.														*/
  GroupBox("\pSpeed", 128, 17, 231, 80);		/*Draw a box around the interface			*/
  																					/*	Speed radio buttons, and give the	*/
  }																					/*	group a title.										*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   4 :
 *	This routine draws all the text in demo window 4.  It is called when first
 *	creating the window and when the window needs to be refreshed.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow4Contents (void)
	{
  MoveTo(10, 21);														/*This window has been previously set	*/
  																					/*	to draw characters using Chicago	*/
  																					/*	12 pt.														*/
  DrawString("\pA lengthy process is being simulated.");	/*Display the message		*/
  MoveTo(10, 85);														/*																		*/
  DrawString("\pType \x11-. to cancel the simulation.");	/*Note: \x11 is the			*/
	}																												/*	Command character.	*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   5 :
 *	This routine draws all the text and pictures in demo window 5.  It is called
 *	when first creating the window and when the window needs to be refreshed.  This
 *	routine has the following unique routines:
 *		� Color-independent drawing (doesn't care about the monitor's settings)
 *		� Color-dependent drawing (different drawing dependent on number of colors,
 *		  grays, or if black & white)
 *		� Multiple-monitor compatibility (each monitor can have different settings)
 *	The color PICTure looks good only in 256 colors, but it also looks good in 256
 *	shades of gray as well as 16 shades of gray (8 bit and 4 bit gray).  So this
 *	demo decides which PICTure to draw depending on the monitor's settings.
 *		NOTE:  Tools Plus may not recognize if you change your monitor settings while
 *	running your application in the development environment.  It will recognize the
 *	change when your application is compiled to be a stand- alone (double-clickable)
 *	application.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow5Contents (void)
	{
  short				TheScreen;										/*Screen counter for multiple-screen	*/
  																					/*	drawing.													*/
	PicHandle		hPicture;											/*Handle to a PICTure resource				*/
	Rect				viewRect;											/*Viewing rectangle for the picture		*/


																						/*Color Independent Drawing:	- - - -	*/
																						/*Drawing that doesn't care about the	*/
																						/*	monitor's setting can be done			*/
																						/*	before or after the color-				*/
																						/*	dependent drawing.								*/
  TextInBox("This window demonstrates color-dependent, color-independent, and multiple-screen drawing.  Use the Monitors desk accessory to change the monitor�s settings.\r  If you have two monitors, drag the window across so that half is on each screen.", 10, 5, 300, 70, teJustLeft, false);

																						/*Color Dependent Drawing: - - - - - -*/
																						/*The following code is for drawing		*/
																						/*	that depends on the number of			*/
																						/*	available colors, shades of gray,	*/
																						/*	or if the monitor is set to Black	*/
																						/*	& White.  It also makes the				*/
																						/*routine multiple-monitor compatible.*/
  for (TheScreen = 1; TheScreen <= NumberOfScreens(); TheScreen++)	/*Repeat			*/
  	{																				/*	drawing once for each logical			*/
  																					/*	screen (monitor with different		*/
  																					/*	settings)�												*/
    BeginUpdateScreen(TheScreen);						/*Begin the update for this logical		*/
    																				/*	screen.  All drawing is limited		*/
    																				/*	(clipped) to the one logical			*/
    																				/*	screen.														*/
    if ((ScreenDepth() >= 8) || ((ScreenDepth() == 4) && (!ScreenHasColors())))
			hPicture = GetPicture(ColorPICT);			/*Use the color pictures only if 256	*/
																						/*	colors or 16 shades of gray are		*/
    else																		/*	available (or better).  Otherwise,*/
			hPicture = GetPicture(BlackAndWhitePICT);	/*	use the Black & White					*/
																								/*	equivalent.										*/
    viewRect = (*hPicture)->picFrame;				/*Determine the picture's framing			*/
    																				/*	rectangle.												*/
																						/*The picture's rectangle is in the		*/
																						/*	local window co-ordinates of the	*/
																						/*	application that created it, so		*/
																						/*	convert it to the local co-				*/
																						/*	ordinates of this demo window�		*/
    OffsetRect(&viewRect, -viewRect.left, -viewRect.top);		/*										*/
		OffsetRect(&viewRect, 30, 90);					/*Offset the picture's rectangle such	*/
																						/*	that it is drawn below the				*/
																						/*	descriptive text.									*/
    DrawPicture(hPicture, &viewRect);				/*Draw the picture in its destination	*/
    																				/*	rectangle.												*/
    ReleaseResource((Handle)hPicture);			/*Release the resource to conserve		*/
    																				/*	memory.														*/
    EndUpdateScreen();											/*End the update to the current				*/
    }																				/*	logical screen.										*/
	}																					/*																		*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   6 :
 *	Dynamic Alerts take care of all drawing, so you don't have to write any code
 *	for it.  This routine is here for cosmetic reasons only, and you can get rid
 *	of it if you want.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow6Contents (void)
	{
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   7 :
 *	This routine draws all the text and lines in demo window 7.  It is called when
 *	first creating the window and when the window needs to be refreshed.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow7Contents (void)
	{
	Rect						Rectangle;								/*Rectangle for drawing comments			*/
																						/*	(text).														*/

																						/* Comment for Pop-Up Menus	-	-	-	-	-	*/
																						/*All comments use Geneva 9pt, as set	*/
																						/*	when this window was first opened.*/
  PenPat(&qd.gray);													/*Any line drawing will be done with a*/
  																					/*	gray pattern.											*/

																						/* Comment for 1st Pop-Up Menu-	-	-	-	*/
  TextInBox("This is a standard Pop-Up Menu.  Nothing special here, except that it�s really easy to do.", 235, 15, 380, 51, teJustLeft, false);
  SetRect(&Rectangle, 230, 12, 385, 55);		/*Define outline around the comment		*/
  FrameRect(&Rectangle);										/*Draw a gray box around the comment	*/
  MoveTo(215, 26);													/*Draw a gray line from the first menu*/
  LineTo(230, 26);													/*	to the box completed above.				*/

																						/* Comment for 2nd Pop-Up Menu-	-	-	-	*/
  TextInBox("This Pop-Up Menu has icons.  But unlike System-7�s pop-ups, the selected item�s icon can appear in the pop-up box.", 235, 65, 380, 113, teJustLeft, false);
  SetRect(&Rectangle, 230, 62, 385, 117);		/*Define outline around the comment		*/
  FrameRect(&Rectangle);										/*Draw a gray box around the comment	*/
  MoveTo(220, 76);													/*Draw a gray line from the second		*/
  LineTo(230, 76);													/*	menu to the box completed above.	*/

																						/* Comment for 3rd Pop-Up Menu-	-	-	-	*/
  TextInBox("This Pop-Up Menu contains icons without any text.  It looks like a custom menu but it�s not.", 235, 125, 380, 161, teJustLeft, false);
  SetRect(&Rectangle, 230, 122, 385, 165);	/*Define outline around the comment		*/
  FrameRect(&Rectangle);										/*Draw a gray box around the comment	*/
  MoveTo(175, 136);													/*Draw a gray line from the third menu*/
  LineTo(230, 136);													/*	to the box completed above.				*/

																						/* Comment for 4th Pop-Up Menu-	-	-	-	*/
  TextInBox("This is a �Pop-Down� menu.  It�s like an on-screen pull-down menu, and is usually used to �do something now.�", 235, 185, 380, 233, teJustLeft, false);
  SetRect(&Rectangle, 230, 182, 385, 237);	/*Define outline around the comment		*/
  FrameRect(&Rectangle);										/*Draw a gray box around the comment	*/
  MoveTo(220, 196);													/*Draw a gray line from the fourth		*/
  LineTo(230, 196);													/*	menu to the box completed above.	*/

																						/* Comment for 5th Pop-Up Menu-	-	-	-	*/
  TextInBox("Pop-Up Menus can use any font and size, and can be auto-sized for a perfect look.", 235, 245, 380, 281, teJustLeft, false);
  SetRect(&Rectangle, 230, 242, 385, 285);	/*Define outline around the comment		*/
  FrameRect(&Rectangle);										/*Draw a gray box around the comment	*/
  MoveTo(150, 246);													/*Draw a gray line from the fifth menu*/
  LineTo(230, 246);													/*	to the box box completed above.		*/
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   8 :
 *	This routine draws all the text and lines in demo window 8.  Tools Plus takes
 *	care of all the button drawing.  The band at the top of this window demonstrates
 *	how to make a tool bar in a window.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow8Contents (void)
	{
	short						TheScreen;								/*Screen counter, multi-screen drawing*/
	Rect						ToolBarRect;							/*Rectangle for drawing toolbar				*/


																						/*D r a w   T o o l B a r							*/
																						/*		B a c k g r o u n d �						*/
	SetRect(&ToolBarRect, -1, -1, 473, 29);		/*Define the toolbar's rectangle			*/
																						/*Draw the toolbar gray if the monitor*/
																						/*	is set to 4-bits or better�				*/
	for (TheScreen = 1; TheScreen<=NumberOfScreens(); TheScreen++)	/*Repeat drawing*/
		{																				/*	once for each logical screen			*/
  																					/*	(monitor with different settings)�*/
		PenColorNormal();												/*PEN: 1 x 1 size, black, patCopy mode*/
    BeginUpdateScreen(TheScreen);						/*Begin the update for this logical		*/
    																				/*	screen.  All drawing is limited		*/
    																				/*	(clipped) to the one logical			*/
    																				/*	screen.														*/
    if (ScreenDepth() >= 4)									/*If this logical screen is set to a	*/
    	{																			/*	depth of 4-bits or more (16 colors*/
    																				/*	or grays)�												*/
      RGBForeColor(&ToolBarGray);						/*																		*/
      PaintRect(&ToolBarRect);							/*Paint the toolbar the medium dark		*/
    	}																			/*	gray.															*/
		EndUpdateScreen();											/*End the update to the current				*/
	}																					/*	logical screen										*/
  PenColorNormal();													/*PEN: 1 x 1 size, black, patCopy mode*/
  FrameRect(&ToolBarRect);									/*Draw a black 1-pixel frame around		*/
  																					/*	the toolbar.											*/

																						/*B u t t o n ' s   V a l u e s				*/
  ActionInWindow8_DrawToolBarValue();				/*Draw value in the window's tool bar	*/
  ActionInWindow8_DrawGlobeValue();					/*Draw the globe button's value				*/

  TextFont(geneva);													/*All comments drawn using Geneva font*/
  TextSize(12);															/*One comment is drawin in Geneva 9pt�*/
  MoveTo(6, 134);														/*																		*/
  DrawString("\pPicture Buttons are very versatile.  You define their look and behavior.");
  PenPat(&qd.gray);													/*Any line drawing will be done with a*/
  																					/*	gray pattern											*/
  MoveTo(0, 118);														/*Draw gray line dividing upper and		*/
  LineTo(472, 118);													/*	lower part of the window.					*/
  TextSize(9);															/*All other comments use Geneva 9pt.	*/

																						/* C o m m e n t   f o r   A r r o w	*/
																						/*		B u t t o n s �									*/
  TextInBox("These buttons use an icon for each stage, giving you absolute control over their appearance.", 10, 41, 155, 80, teJustLeft, false);
  MoveTo(120, 29);													/*Draw a gray line from this comment	*/
  LineTo(120, 40);													/*	to the buttons it refers to.			*/

																						/* C o m m e n t   f o r							*/
																						/*	A l i g n m e n t   B u t t o n s�*/
  TextInBox("These �SICN 3D� buttons are available in two sizes.  They use a single icon each.  Tools Plus does all the 3D drawing.  Activate another window and see how they look disabled.", 167, 41, 358, 110, teJustLeft, false);
  MoveTo(218, 29);													/*Draw a gray line from this comment	*/
  LineTo(218, 40);													/*	to the buttons it refers to.			*/
  MoveTo(322, 29);													/*																		*/
  LineTo(322, 40);													/*																		*/

																						/* C o m m e n t   f o r							*/
																						/*	U t i l i t y   B u t t o n s �		*/
  TextInBox("This pair of buttons behaves as one control.  Continuous events are generated while the mouse button is held down.", 368, 41, 471, 118, teJustLeft, false);
  MoveTo(450, 29);													/*Draw a gray line from this comment	*/
  LineTo(450, 40);													/*	to the buttons it refers to.			*/

																						/* C o m m e n t   f o r   P l a i n	*/
																						/*	I c o n   B u t t o n s �					*/
  TextInBox("Use picture buttons for clickable icons.", 19, 191, 111, 216, teJustLeft, false);

																						/* C o m m e n t   f o r   T a p e		*/
																						/*	D e c k   B u t t o n s �					*/
  TextInBox("Professional looking controls are easy!", 20, 281, 117, 306, teJustLeft, false);

																						/* C o m m e n t   f o r							*/
																						/*	S c r o l l i n g   B u t t o n �	*/
  TextInBox("These buttons have �values� associated with them.  You can raise/lower the value by clicking on opposite sides of the button.  The left button uses only 1 icon!", 204, 154, 398, 203, teJustLeft, false);
  MoveTo(147, 158);													/*																		*/
  DrawString("\pStep");											/*																		*/
  MoveTo(172, 158);													/*																		*/
  DrawString("\pScroll");										/*																		*/

																						/* C o m m e n t   f o r   D u a l -	*/
																						/*	S t a g e   B u t t o n	�					*/
	TextInBox("These dual-stage buttons toggle when selected.", 221, 243, 342, 268, teJustLeft, false);
}																						/*																		*/		




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   9 :
 *	This routine draws the contents of demo window 9 (the Tool Bar).  In this demo,
 *	the only thing the Tool Bar needs is to be painted with the correct color (gray
 *	when the monitor is set to 4 bits or higher).
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow9Contents (void)
	{
	Rect						ToolBarRect;							/*Rectangle for drawing toolbar				*/


																						/* P a i n t   T o o l B a r   G r a y*/
  if (ScreenDepth() >= 4)										/*If main minitor is set to a depth of*/
  	{																				/*	4-bits or more (16 colors/grays),	*/
  																					/*	paint the tool bar�								*/
    RGBForeColor(&ToolBarGray);							/*Set the foregound color to gray			*/
    SetRect(&ToolBarRect, 0, 0, 32000, 32000);/*Define the toolbar's rectangle		*/
    																				/*	(who cares how big it really is?	*/
    																				/*	All drawing is limited to the Tool*/
    																				/*	Bar).															*/
    PaintRect(&ToolBarRect);								/*Paint tool bar the medium dark gray	*/
		}
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   10 :
 *	This floating palette contains only Tools Plus objects, so there is no need to
 *	draw anything in this window. This routine is here for cosmetic reasons only,
 *	and you can get rid of it if you want.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow10Contents (void)
	{
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * D r a w   C o n t e n t s   o f   D e m o   W i n d o w   11 :
 *	This floating palette contains only Tools Plus objects, so there is no need to
 *	draw anything in this window. This routine is here for cosmetic reasons only,
 *	and you can get rid of it if you want.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DrawWindow11Contents (void)
	{
	}










/***********************************************************************************
 ***********************************************************************************
/*  C r e a t e   D e m o   W i n d o w s :
 *	The following routines are used to create each of the demo windows and their
 *	contents.  These routines are invoked when the user selects an item in the
 *	Demos menu.  A couple of these windows are opened automatically by the
 *	"PrepareTheDemo" routine when this program starts up.
 */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   1 :
 *	This window demonstrates how Tools Plus works with  List Boxes, Buttons (radio
 *	buttons, check boxes and push buttons), and Cursor Zones.
 *		When playing around with this window, try the following things:
 *			� double-click a line in a List Box and notice how it translates to "select
 *				this line and OK"
 *			� press the Enter or Return key to activate the default OK button
 *			� activate another window and notice all the buttons and list boxes get
 *				disabled.  Also notice that the black outline around the default OK button
 *				gets dimmed out too!
 *	TIP:  To make this window appear to fill in as quickly as possible, we don't
 *				populate the list boxes until the end!
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow1 (void)
	{																					/*Open the window (it automatically		*/
																						/*	shifts down to compensate for an	*/
																						/*	open Tool Bar, and its position is*/
																						/*	"tiled")�													*/
	WindowOpen(DemoWindow1, 0, 0, 425, 320, "\pLists, Buttons and Cursor Zones", noGrowDocProc + wTile, GoAway, NotModal);
																						/* Left List Box:	-	-	-	-	-	-	-	-	-	-	*/
  TextFont(systemFont);											/*Create an empty list box.  Its font	*/
  TextSize(12);															/*	will be the window's current font:*/
  NewListBox(LeftList, 40, 8, 220, 104, lOnlyOne);	/*	Chicago 12 pt.						*/
																						/* Right List Box:-	-	-	-	-	-	-	-	-	-	*/
  TextFont(geneva);													/*Create a second empty list box.			*/
  TextSize(9);															/*	This one will use Geneva 9pt to		*/
  																					/*	show that a different font can be	*/
  NewListBox(RightList, 295, 8, 400, 104, lOnlyOne);	/*	used per list box.			*/
																						/* B u t t o n s :-	-	-	-	-	-	-	-	-	-	*/
  TextFace(bold);														/*Buttons will be drawn in bold				*/
  																					/*	Geneva 9pt.												*/
																						/*Create a set of mini-fonts using		*/
																						/*	Geneva 9pt in bold typeface.  This*/
																						/*	demonstrates the use of a second	*/
																						/*	font for buttons.									*/
  NewButton(MiniButton1, 20, 150, 35, 166, "\p1", pushButProc + useWFont, enabled, notSelected);
  NewButton(MiniButton2, 40, 150, 55, 166, "\p2", pushButProc + useWFont, enabled, notSelected);
  NewButton(MiniButton3, 60, 150, 75, 166, "\p3", pushButProc + useWFont, enabled, notSelected);
  NewButton(MiniButton4, 80, 150, 95, 166, "\p4", pushButProc + useWFont, enabled, notSelected);
  NewButton(MiniButton5, 100, 150, 115, 166, "\p5", pushButProc + useWFont, enabled, notSelected);
																						/*Create a set of check boxes using		*/
																						/*	the System font (Chicago 12)�			*/
  NewButton(CheckBox1, 20, 220, 117, 236, "\pFirst Choice", checkBoxProc, enabled, notSelected);
  NewButton(CheckBox2, 20, 236, 133, 252, "\pSecond Choice", checkBoxProc, enabled, selected);
  NewButton(CheckBox3, 20, 252, 134, 268, "\pTwo And Three", checkBoxProc, enabled, selected);
																						/*Create a set of radio buttons using	*/
																						/*	the System font (Chicago 12)�			*/
  NewButton(RadioButton1, 170, 220, 214, 236, "\pOne", radioButProc, enabled, selected);
  NewButton(RadioButton2, 170, 236, 216, 252, "\pTwo", radioButProc, enabled, notSelected);
  NewButton(RadioButton3, 170, 252, 226, 268, "\pThree", radioButProc, enabled, notSelected);
																						/*Create the "Cancel" push-button�		*/
  NewButton(CancelButton, 275, 250, 335, 270, "\pCancel", pushButProc, enabled, notSelected);
																						/*Create the "OK" push-button.  This	*/
																						/*	is the window's default button,		*/
																						/*	and it is selected automatically	*/
																						/*	when you press Return or Enter�		*/
  NewButton(OkButton, 350, 250, 410, 270, "\pOK", DefaultButton, enabled, notSelected);

  DrawWindow1Contents();										/*Draw the rest of the objects in this*/
  																					/*	demo window.  All Tools Plus			*/
  																					/*	objects are auto matically updated*/
  																					/*	when a window needs to be					*/
  																					/*	refreshed, such as when you bring	*/
  																					/*	it to the front to make it active.*/
																						/*		Non-Tools Plus objects (ie:			*/
																						/*	those you create) have to be			*/
																						/*	redrawn separately in response to	*/
																						/*	a doRefresh event, so it's a good	*/
																						/*	idea to keep them in a separate		*/
																						/*	routine.													*/

	/* T i m e   c o n s u m i n g   t h i n g s �																	*/
	/*	The following time-consuming things were saved for the end of the routine		*/
	/*	because it makes the window appear to be filled in faster, particularly on	*/
	/*	slower Macs like the Plus, Classic, or LC.  You don't have to follow the		*/
	/*	same concept when you write your own applications.													*/
																						/* Cursor Tables & Zones:	-	-	-	-	-	-	*/
  NewCursorTable(1, arrowCursor);						/*Create Cursor Table number 1.  The	*/
  																					/*	standard Arrow cursor is the			*/
  																					/*	default for this table.						*/
  																					
  CursorZone(1, 1, plusCursor, 275, 150, 400, 189);		/*Add two Cursor Zones to		*/
  CursorZone(1, 2, crossCursor, 275, 193, 400, 232);	/*	the Cursor Table.		When*/
  																					/*	the cursor is in these zones, it	*/
  																					/*	will automatically change to the	*/
  																					/*	specified cursor shape.						*/
  UseCursorTable(1);												/*The current window will use Cursor	*/
  																					/*	Table no 1.  Note that a cursor		*/
  																					/*	table exists independently of			*/
  																					/*	windows.  In fact, several windows*/
  																					/*	can each use the same cursor			*/
  																					/*	table.  It is not necessary to		*/
  																					/*	create a cursor table each time		*/
  																					/*	you want a window to use it.  You */
  																					/*	just use the UseCursorTable				*/
  																					/*	routine to make a window use a		*/
  																					/*	cursor table.											*/
  																					
																						/* Fill List Boxes:	-	-	-	-	-	-	-	-	-	*/
  DrawListBox(LeftList, off);								/*Turn list box drawing off for both	*/
  DrawListBox(RightList, off);							/* boxes.  The	contents will be drawn*/
  																					/*	later.  If you don't turn list box*/
  																					/*	drawing off while adding lines,		*/
																						/*	you will see each line being			*/
																						/*	added, one at a time.  This way,	*/
																						/*	you see all the lines appear at		*/
																						/*	once.															*/
																						/*Fill the lines in the left list box.*/
																						/*	You can fill these lines in any		*/
																						/*	order you want.  Notice the longer*/
																						/*	lines are compressed (the					*/
																						/*	characters are closer together)		*/
																						/*	when using System 7.  Tools Plus	*/
																						/*	automatically appends the ellipsis*/
																						/*	(�) if the line's text is too long*/
																						/*	to see in the list.  Fill in the	*/
																						/*	lines in the right list box.  The	*/
																						/*	Tools Plus User Manual describes	*/
																						/*	how you can populate a list box		*/
																						/*	and have the lines in alphabetic	*/
																						/*	order.														*/
  SetListBoxText(LeftList, 1, "\pBill Arlington, Shipping & Receiving");	/*			*/
  SetListBoxText(LeftList, 2, "\pGail Force, Social Director");	/*								*/
  SetListBoxText(LeftList, 3, "\pBill Gates, Comedy Relief");		/*								*/
  SetListBoxText(LeftList, 4, "\pRon Martin, Art & Graphics");	/*								*/
  SetListBoxText(LeftList, 5, "\pSamantha Moore, Connectivity");/*								*/
  SetListBoxText(LeftList, 6, "\pMike Simms, Design");					/*								*/
  SetListBoxText(LeftList, 7, "\pSam Strong, Design");					/*								*/
  SetListBoxText(LeftList, 8, "\pLinda Tyler, Drafting");				/*								*/
  SetListBoxText(LeftList, 9, "\pMargaret Victor, Drafting");		/*								*/
  SetListBoxText(RightList, 1, "\pArt & Graphics");							/*								*/
  SetListBoxText(RightList, 2, "\pConnectivity");								/*								*/
  SetListBoxText(RightList, 3, "\pComedy Relief");							/*								*/
  SetListBoxText(RightList, 4, "\pDesign");											/*								*/
  SetListBoxText(RightList, 5, "\pDrafting");										/*								*/
  SetListBoxText(RightList, 6, "\pShipping & Receiving");				/*								*/
  SetListBoxText(RightList, 7, "\pSocial Director");						/*								*/
  SetListBoxLine(LeftList, 2, on);					/*Select the left list's second line	*/
  																					/*	by default.												*/
  DrawListBox(LeftList, on);								/*Turn on line drawing for both list	*/
  DrawListBox(RightList, on);								/*	boxes. Their contents are drawn by*/
  																					/*	this process.											*/
  EnableMenu(FileMenu, CloseItem, enabled);	/*Enable the File menu's Close				*/
  																					/*	command, now that there's a window*/
  }																					/*	to close.													*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   2 :
 *	This window demonstrates how Tools Plus works with  Editing Fields, Window
 *	Zooming and Scroll Bars.  Please note that although this window has scroll bars,
 *	the demo doesn't actually scroll anything.  Notice that when you click the
 *	window's zoom box, or when you resize the window by using the size box, that
 *	the scroll bars are automatically moved to accommodate the window's new size.
 *		When playing around with this window, try the following things:
 *			� click the window's zoom box to zoom between the standard co-ordinates
 *				and the user-defined ones
 *			� use the size box to resize the window.  Notice that the minimum and
 *				maximum size are controlled
 *			� type in the first (length limited) field
 *			� copy and paste test between another application or desk accessory and
 *				one of the fields
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow2 (void)
	{
  Rect				userRect, stdRect;						/*User co-ordinates and standard co-	*/
  																					/*	ordinates used for zooming one of	*/
  																					/*	the demo windows.									*/

	
																						/* C r e a t e   t h e   W i n d o w:	*/
																						/*Open the window (it automatically		*/
																						/*	shifts down to compensate for an	*/
																						/*	open Tool Bar, and its position is*/
																						/*	"tiled")�													*/
  WindowOpen(DemoWindow2, 20, 58, 420, 338, "\pFields, Zooming and Scroll Bars", documentProc + ZoomBox + wTile, GoAway, NotModal);
  GetWindowZoom(&userRect, &stdRect);				/*Get the standard and user-defined		*/
  																					/*	co-ordinates for Zooming the			*/
  																					/*	window (using zoom box)						*/
  SetRect(&userRect, 250, 100, 470, 320);		/*Define the window's "user- defined"	*/
  																					/*	co-ords that are used for Zooming	*/
  																					/*	the window when the zoom-box is		*/
  																					/*	clicked.													*/
  SetWindowZoom(&userRect, &stdRect);				/*Set the standard and user-defined		*/
  																					/*	co-ordinates for Zooming.					*/
  SetWindowSizeLimits(220, 220, 400, 280);	/*Set the window's sizing limits:			*/
																						/*	minHoriz, minVert, maxHoriz,			*/
																						/*	maxVert.													*/
																						/* Scroll Bars:	-	-	-	-	-	-	-	-	-	-	-	*/
  NewScrollBar(RightBar, scrlRightEdge, scrlTopEdge, scrlRightEdge, scrlBottomEdge, enabled, 1, 5, 10);
  NewScrollBar(BottomBar, scrlLeftEdge, scrlBottomEdge, scrlRightEdge, scrlBottomEdge, enabled, 1, 50, 100);
																					  /*	scroll bar for the window.  The		*/
																					  /*	last three parameters specify the	*/
																					  /*	min limit, current value, and max	*/
																					  /*	limit.														*/

																						/* E d i t i n g   F i e l d s :	-	-	*/
  																					/*Specify the text that will be appear*/
  																					/*	in each field.										*/
	Cstr2PHdl("Length limited field", hField1);				/*														*/
	Cstr2PHdl("Single line editing field.", hField2);	/*														*/	
	Cstr2PHdl("This is a multiple-line editing field which incorporates word wrap.", hField3);

  TextFont(monaco);													/*First field uses Monaco 9pt font,		*/
  TextSize(9);															/*	and is length limited.  That means*/
  FieldLengthLimit(on);											/*	that no more than 30 characters		*/
    																				/*	(the field's limit) can be typed	*/
  																					/*	or pasted in the field.						*/
  NewField(1, 10, 100, 191, 111, hField1, teBoxNoCR, teJustLeft);	/*							*/
  FieldLengthLimit(off);										/*The remaining fields are not length	*/
 																						/*	limited.													*/

  TextFont(geneva);													/*Second field uses bold Geneva 9pt.	*/
 	TextFace(bold);														/*	This field demonstrates a single-	*/
 																						/*	line editing field.								*/
  NewField(2, 10, 120, 191, 132, hField2, teBoxNoCR, teJustLeft);	/*							*/

  TextFont(systemFont);											/*Third field uses Chicago 12pt.  It	*/
  TextSize(12);															/*	demonstrates a multiple-line			*/
  TextFace(0);															/*	editing field with word wrap.			*/
  NewField(3, 10, 141, 191, 189, hField3, teBoxCR, teJustLeft);	/*								*/

  ActivateField(1, teSelectEnd);						/*Activate the first field, with the	*/
  																					/*	insertion point at the end of the	*/
  																					/*	text.															*/
  DrawWindow2Contents();										/*Draw the rest of the objects in this*/
  																					/*	demo window.  All Tools Plus			*/
  																					/*	objects are auto matically updated*/
  																					/*	when a window needs to be					*/
  																					/*	refreshed, such as when you bring	*/
  																					/*	it to the front to make it active.*/
																						/*		Non-Tools Plus objects (ie:			*/
																						/*	those you create) have to be			*/
																						/*	redrawn separately in response to	*/
																						/*	a doRefresh event, so it's a good	*/
																						/*	idea to keep them in a separate		*/
																						/*	routine.													*/
  EnableMenu(FileMenu, CloseItem, enabled);	/*Enable the File menu's Close				*/
  																					/*	command, now that there's a window*/
  }																					/*	to close.													*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   3 :
 *	This window demonstrates shows how Tools Plus can draw icon families (in
 *	System 6 and System 7).  It also provides an example how to use Cursor Zones
 *	to make icons click-sensitive.
 *		When playing around with this window, try the following things:
 *			� click either of the two icons
 *			� if you have a color monitor, use the Monitors control panel to change
 *				the number of colors displayed
 *			� if you have two monitors, set one to black and white and drag the window
 *				such that the icon is half on a color screen and half on a black and
 *				white screen.  Tools Plus draws the icon perfectly.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow3 (void)
	{																					/*Open the window (it automatically		*/
																						/*	shifts down to compensate for an	*/
																						/*	open Tool Bar, and its position is*/
																						/*	"tiled")�													*/
  WindowOpen(DemoWindow3, 0, 0, 240, 142, "\pIcons and Cursor Zones", noGrowDocProc + wTile, GoAway, NotModal);

																						/* Cursor Table & Zones:-	-	-	-	-	-	-	*/
  NewCursorTable(2, arrowCursor);						/*Create Cursor Table number 2.  The	*/
  																					/*	standard Arrow cursor is the			*/
  																					/*	default for this table.						*/
  CursorZone(2, PrinterIcon, arrowCursor, 20, 32, 52, 64);	/*Add two Cursor Zones*/
  CursorZone(2, ModemIcon, arrowCursor, 65, 32, 97, 64);		/*	to the Cursor			*/
  																					/*	Table.  The user can click these	*/
  																					/*	zones, thereby making the Printer	*/
  																					/*	and Modem icons click sensitive.	*/
  UseCursorTable(2);												/*The current window will use Cursor	*/
  																					/*	Table no 2.  Note that a cursor		*/
  																					/*	table exists independently of			*/
  																					/*	windows.  In fact, several windows*/
  																					/*	can each use the same cursor			*/
  																					/*	table.  It is not necessary to		*/
  																					/*	create a cursor table each time		*/
  																					/*	you want a window to use it.  You	*/
  																					/*	just use the UseCursorTable				*/
  																					/*	routine to make a window use a		*/
  																					/*	cursor table.											*/

																						/*Create 3 radio buttons used to			*/
																						/*	select the MIDI interface's speed�*/
  NewButton(MidiHalfMeg, 142, 27, 216, 39, "\p  .5 MHz.", radioButProc, enabled, notSelected);
  NewButton(MidiOneMeg, 142, 43, 216, 55, "\p1.0 MHz.", radioButProc, enabled, notSelected);
  NewButton(MidiTwoMeg, 142, 58, 216, 72, "\p2.0 MHz.", radioButProc, enabled, notSelected);
  SelectButton(MidiSpeed, selected);				/*Set the current speed button				*/

  DrawWindow3Contents();										/*Draw the rest of the objects in this*/
  																					/*	demo window.  All Tools Plus			*/
  																					/*	objects are auto matically updated*/
  																					/*	when a window needs to be					*/
  																					/*	refreshed, such as when you bring	*/
  																					/*	it to the front to make it active.*/
																						/*		Non-Tools Plus objects (ie:			*/
																						/*	those you create) have to be			*/
																						/*	redrawn separately in response to	*/
																						/*	a doRefresh event, so it's a good	*/
																						/*	idea to keep them in a separate		*/
																						/*	routine.													*/

  EnableMenu(FileMenu, CloseItem, enabled);	/*Enable the File menu's Close				*/
  																					/*	command, now that there's a window*/
  }																					/*	to close.													*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   4 :
 *	This demo shows off several Tools Plus features:
 *		� Cursor animation (the wristwatch cursor spins)
 *		� Filtering unwanted events when the application is busy (all typing and mouse
 *		  clicks are filtered, so your application only gets Command-. events when the
 *			user wants to halt a lengthy process).
 *		� Secondary event loop in an application
 *		� Background processing (an ongoing process that runs all the time)
 *		� Progress thermometer
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow4 (void)
	{
	#define			ThermMax				235						/*Thermometer's maximum value					*/

	Rect				ThermometerRect;							/*Pregress thermometer's location			*/
	short				ThermValue;										/*Themometer's current value					*/
  long				NextThermometerTime;					/*Next thermometer update time (ticks)*/
  Boolean			Done;													/*Is this demo done? (either Command-.*/
  																					/*	by user or thermometer is at 100%)*/


  CursorShape(watchCursor);									/*Set the cursor to a wrist-watch			*/
																						/*Open a window to tell the user about*/
																						/*	the lengthy process (automatically*/
																						/*	centered)�												*/
  WindowOpen(DemoWindow4, 0, 0, 265, 100, "\p", dBoxProc + wCenter, NoGoAway, Modal);
  TextFont(systemFont);											/*The message is displayed in Chicago	*/
  TextSize(12);															/*	12pt.															*/
  DrawWindow4Contents();										/*Draw this window's message					*/

  SetRect(&ThermometerRect, 15, 42, 250, 55);/*Define thermometer's co-ordinates	*/
  ThermValue = 0;														/*Initialize thermometer's value (0%)	*/
  NextThermometerTime = 0;									/*Force thermometer update right away	*/
  Done = false;															/*This demo is not completed yet			*/

																						/* L o c a l   E v e n t   L o o p		*/
  while (!Done) {														/*Keep running until we're done�			*/
		if (PollSystem(&Poll)) {								/*If an event has been detected�			*/
  	  switch (Poll.What)										/*Determine what kind of event has		*/
   		 	{																		/*	occurred�													*/
				case doRefresh: 										/*A window needs to be refreshed�			*/
					RefreshWindow();									/*	redraw the specified window.  A		*/
					break;														/*	screen saver may cause this event,*/
																						/*	even in a modal window like this	*/
																						/*	one.															*/
				case doKeyDown:											/*If the user typed a character.  All	*/
					Done = true;											/*	characters other than Command-.		*/
					break;														/*	are filtered out, so we know the	*/
																						/*	user has typed Command-. to halt	*/
																						/*	the process.											*/
				default:														/*All other events are ignored by			*/
 					break;														/*	this demo.												*/
  			}																		/*																		*/
			}																			/* B a c k g r o u n d   processing�	*/
		else if (TickCount() >= NextThermometerTime)	/*No event available, so if it	*/
			{																						/*	is time to do the next			*/
																									/*	thermometer update�					*/
			NextThermometerTime = TickCount() + 6;			/*Next update will be in 6			*/
																									/*	ticks (10 times/second)			*/
			ThermValue = ThermValue + 1;								/*Increment thermometer's value	*/
			DrawThermometer(&ThermometerRect, ThermValue, ThermMax);/*Update thermometer*/
			if (ThermValue >= ThermMax)						/*If the thermometer reads 100%�			*/
				Done = true;												/*	we're done with this demo window.	*/
    	}																			/*																		*/
    }																				/*																		*/

  CloseTheWindow(DemoWindow4);							/*End this demo by closing its window	*/
  ResetCursor();														/*Get rid of the watch cursor, and set*/
  																					/*	it according to its location and	*/
	}																					/*	relation to active window.				*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   5 :
 *	This demo shows how to write code that does color drawing that is dependent on
 *	the monitor's settings (ie: the number of colors, number of shades of gray, or
 *	black and white).  With very little additional effort, your application can
 *	also be made to be compatible with Macintoshes that have multiple-monitors.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow5 (void)
	{																					/*Open the window (it automatically		*/
																						/*	shifts down to compensate for an	*/
																						/*	open Tool Bar, and its position is*/
																						/*	"tiled")�													*/
  WindowOpen(DemoWindow5, 0, 0, 310, 208, "\pColor Drawing and Multiple Monitors", noGrowDocProc + wTile, GoAway, NotModal);
																						/*Create the "OK" push-button.  This	*/
																						/*	is the window's default button,		*/
																						/*	and it is selected automatically	*/
																						/*	when you press Return or Enter�		*/
  NewButton(OkButton, 125, 170, 185, 190, "\pOK", DefaultButton, enabled, notSelected);


  TextFont(geneva);													/*All text is drawn in Geneva 9pt.		*/
  TextSize(9);															/*																		*/
  DrawWindow5Contents();										/*Draw the rest of the objects in this*/
  																					/*	demo window.  All Tools Plus			*/
  																					/*	objects are auto matically updated*/
  																					/*	when a window needs to be					*/
  																					/*	refreshed, such as when you bring	*/
  																					/*	it to the front to make it active.*/
																						/*		Non-Tools Plus objects (ie:			*/
																						/*	those you create) have to be			*/
																						/*	redrawn separately in response to	*/
																						/*	a doRefresh event, so it's a good	*/
																						/*	idea to keep them in a separate		*/
																						/*	routine.													*/
  EnableMenu(FileMenu, CloseItem, enabled);	/*Enable the File menu's Close				*/
  																					/*	command, now that there's a window*/
  }																					/*	to close.													*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   6 :
 *	This demo shows off Tools Plus's Dynamic Alerts.  Dynamic Alerts are self
 *	contained (you don't have to do anything other than call it and wait for the
 *	user to respond).  Breaking from tradition that was previously established by
 *	the "OpenDemoWindow�" routines, this demo does everything pertaining to
 *	displaying Dynamic Alerts, detecting user action, and responding to events.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow6 (void)
	{
	#define			ContinueAlert				 11500		/*Button definition for a "Continue"	*/
																						/*	dynamic alert.										*/
	#define			SilentContinueAlert	-11500		/*Button definition for a "Continue"	*/
																						/*	dynamic alert that doesn't beep		*/
																						/*	the user.													*/
	#define			EndAlert						-33987		/*Button definition using three				*/
																						/*	custom buttons.										*/

	char				Msg[256];											/*Long message displayed in a dynamic	*/
																						/*	alert box (too long to include as	*/
																						/*	a single line of source code).		*/
																						/*	Note that this is a C string!			*/


																						/*The message for the first alert is	*/
																						/*	too long to include in a single		*/
																						/*	line of source code, to we can		*/
																						/*	assign a value to the Msg					*/
																						/*	variable�													*/
  strcpy(Msg, "This is an dynamic alert box.  Dynamic Alerts automatically adjust to accommodate the text your application provides them.  In fact, they�re smart enough to recognize �carriage returns� as �new line� commands.\r\rClick �Continue� for more examples.");
																						/*These first 2 alerts ignore which		*/
																						/*	button that was clicked�					*/
 	AlertBox(noteIcon, C2PStr(Msg), ContinueAlert);	/*															*/
  AlertBox(stopIcon, "\pIcons�\r\rYou can display any icon you want in a Dynamic alert, including color icons.\r\rAlertBox calls Tools Plus�s DrawIcon routine, so it�s smart enough to pick the right icon for your monitor�s settings.", SilentContinueAlert);

																						/*This alert displays 2 buttons: Yes	*/
																						/*	and No.  If the user answers Yes,	*/
																						/*	the demo continues�								*/
  if (YesAltBut == AlertBox(noteIcon, "\pDo you want to continue with more examples of Dynamic Alerts?", YesNoAlert))
		{																				/*																		*/
    AlertBox(cautionIcon, "\pDynamic Alerts can even do unusual things, like alerts with no buttons (click in this window to get rid of it).", NoButtonAlert);
    AlertBox(ModemIcon, "\pSave modem preferences before quitting?\r(Just kidding!)", NoYesCanAlert);
    AlertButtonName(7, "\pYup!");						/*This Dynamic Alert shows how you can*/
    AlertButtonName(8, "\pWow!");						/*	rename buttons to customize your	*/
    AlertButtonName(9, "\pGnarly");					/*	application.  The available				*/
    																				/*	options make it seem like you			*/
    																				/*	have thousands of	customized			*/
    																				/*	alerts in your application.				*/
		AlertBox(NoIcon, "\pSo, are you impressed with Dynamic Alerts?", EndAlert);/*	*/
		}																				/*																		*/
	}																					/*																		*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   7 :
 *  This demo demonstrates how easy it is to create and work with Pop-Up Menus.
 *	Tools Plus gives you features and options ordinary Pop-Up Menus don�t have.
 *	Also, Tools Plus's Pop-Up menu's work identically across all systems, unlike
 *	ordinary Mac Pop-Up Menus that provide some features that are exclusive to
 *	System 7.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow7 (void)
	{																					/*Open the window (it automatically		*/
																						/*	shifts down to compensate for an	*/
																						/*	open Tool Bar, and its position is*/
																						/*	"tiled")�													*/
  WindowOpen(DemoWindow7, 0, 0, 390, 296, "\pPop-Up Menus", noGrowDocProc + wTile, GoAway, NotModal);		

																						/* P o p - U p   M e n u s :	-	-	-	-	*/
	/*	Example 1�														  Create a standard (default) Pop-Up	*/
																						/*	Menu.  It is displayed in the			*/
																						/*	System Font (Chicago 12pt) and		*/
																						/*	only allows selection of one item	*/
																						/*	at a time.												*/
																						/*    NOTE: If you specify the bottom	*/
																						/*	of the Pop- Up Menu's rectangle to*/
																						/*	be the same as the top, it is be	*/
																						/*	created at exactly the font's			*/
																						/*	height�														*/
  NewPopUp(popMenu1, 110, 20, 209, 20, "\pDay of Week:", popupDefaultType, enabled);
  PopUpMenu(popMenu1, 1, enabled, "\pSunday");		/*Populate the Pop-Up Menu with	*/
  PopUpMenu(popMenu1, 2, enabled, "\pMonday");		/*	days of the week.  At this	*/
  PopUpMenu(popMenu1, 3, enabled, "\pTuesday!�");	/*	point, no item is selected	*/
  PopUpMenu(popMenu1, 4, enabled, "\pWednesday");	/*	yet.	Notice that the third	*/
  PopUpMenu(popMenu1, 5, enabled, "\pThursday");	/*	item (Tuesday) ismarked with*/
  PopUpMenu(popMenu1, 6, enabled, "\pFriday");		/*	a bullet character (�).			*/
  PopUpMenu(popMenu1, 7, enabled, "\pSaturday");	/*	Marking any item "selects"	*/
  																								/*	it, and automatically				*/
  																								/*	displays it in the menu's		*/
  																								/*	content area.								*/


	/*	Example 2�														  Create a second Pop-Up Menu.  This	*/
																						/*	one shows the selected item's	icon*/
																						/*	and hides the "down arrow".				*/
  NewPopUp(popMenu2, 110, 70, 216, 70, "\pSearch Here:", popupIconTitle + popupNoArrow, enabled);
  PopUpMenu(popMenu2, 1, enabled, "\pDesktop^44");		/*Populate the Pop-Up menu	*/
  PopUpMenu(popMenu2, 2, enabled, "\pHard Disk^45");	/*	with a simulated path		*/
  PopUpMenu(popMenu2, 3, enabled, "\pTools Plus^46");	/*	starting from the				*/
  PopUpMenu(popMenu2, 4, enabled, "\pCodeWarrior^47");/*	Desktop to folders.	Each*/
  PopUpMenu(popMenu2, 5, enabled, "\pLibraries^47");	/*	menu item has an				*/
  PopUpMenu(popMenu2, 6, enabled, "\p#Includes^47");	/*	associated icon (its		*/
																											/*	specifier follows the		*/
																						/*	"^" symbol.)  Notethat the Menu		*/
																						/*	Manager automatically adds 256 to	*/
																						/*	the specifier to calculate the		*/
																						/*	icon'sID.													*/
																						/*	Example:  ^44+256=300 'cicn' ID		*/
  CheckPopUp(popMenu2, 1, on);							/*Select the first item by placing a	*/
																						/*	check mark beside it.							*/


	/*	Example 3�														Create a third Pop-Up Menu that shows	*/
																						/*	off how easily you can create			*/
																						/*	picture menus.  This one uses two	*/
																						/*	icons and no text.								*/
  NewPopUp(popMenu3, 110, 120, 170, 154, "\pSerial Port:", popupIconTitle + popupNoArrow, enabled);
  PopUpMenu(popMenu3, 1, enabled, "\p^48");	/*Populate the Pop-Up menu with the		*/
  PopUpMenu(popMenu3, 2, enabled, "\p^49");	/*	Printer and Modem icons.					*/
  CheckPopUp(popMenu3, 1, on);							/*Select the first item by placing a	*/
																						/*	check mark beside it.							*/


	/*	Example 4�														Create a fourth Pop-Up Menu that is a	*/
																						/*	special type of control.  This one*/
																						/*	has a fixed title, and is used		*/
																						/*	like an on-window pull-down menu.	*/
  NewPopUp(popMenu4, 110, 190, 216, 190, "\pFormat", popupFixedTitle, enabled);
  PopUpMenu(popMenu4, 1, enabled, "\pClear");			/*Populate the menu with the		*/
  PopUpMenu(popMenu4, 2, enabled, "\pParagraph�");/*	selections that can be made.*/
  PopUpMenu(popMenu4, 3, enabled, "\pCharacter�");/*	Even though you can put			*/
  PopUpMenu(popMenu4, 4, enabled, "\pStyle�");		/*	check marks or other symbols*/
  																								/*	beside these items, this		*/
  																					/*	demo makes them perform an action	*/
																						/*	just like pull-down menus.				*/


	/*	Example 5�														Mini Pop-Up Menu�											*/
  TextFont(geneva);													/*Set the window's font to Geneva 9pt.*/
  TextSize(9);															/*	The pop-up menu will use these		*/
  																					/*	settings.													*/
																						/*Create the last menu using a				*/
																						/*	different (non- System) font. This*/
																						/*	feature is available on	 all			*/
																						/*	systems (unlike ordinary pop-up		*/
																						/*	menus). Never dim the menu's title*/
																						/*	or the selected item because			*/
																						/*	Geneva 9pt is unreadable when its	*/
																						/*	dithered on a 1-bit monitor (black*/
																						/*	and white) monitor�								*/
  NewPopUp(popMenu5, 110, 240, 143, 240, "\pSize:", popupUseWFont + popupNoArrow + popupNeverDimTitle + popupNeverDimSelection, enabled);
  PopUpMenu(popMenu5, 1, enabled, "\p9");		/*Populate the menu with a fictitious	*/
  PopUpMenu(popMenu5, 2, enabled, "\p10");	/*	set of available font sizes.  Once*/
  PopUpMenu(popMenu5, 3, enabled, "\p12!�");/*	the menu is created, you can			*/
  PopUpMenu(popMenu5, 4, enabled, "\p14");	/*	change the window's font and size	*/
  PopUpMenu(popMenu5, 5, enabled, "\p18");	/*	without affecting the pop-up			*/
  PopUpMenu(popMenu5, 6, enabled, "\p24");	/*	menus.														*/
  PopUpMenu(popMenu5, 7, enabled, "\p36");	/*	The third item (12pt) is marked		*/
  																					/*	with a bullet (�) to select that	*/
  																					/*	item.															*/

  DrawWindow7Contents();										/*Draw the rest of the objects in	this*/
  																					/*	demo window.  All Tools Plus			*/
  																					/*	objects are auto matically updated*/
  																					/*	when a window needs to be					*/
  																					/*	refreshed, such as when you bring	*/
  																					/*	it to the front to make it active.*/
																						/*		Non-Tools Plus objects (ie:			*/
																						/*	those you create) have to be			*/
																						/*	redrawn separately in response to	*/
																						/*	a doRefresh event, so it's a good	*/
																						/*	idea to keep them in a separate		*/
																						/*	routine.													*/
  EnableMenu(FileMenu, CloseItem, enabled);	/*Enable the File menu's Close				*/
  																					/*	command, now that there's a window*/
  }																					/*	to close.													*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   8 :
 *	This demo shows off Tools Plus's Picture Buttons.  A variety of buttons have
 *	been created to give you a taste of their abilities.  The top of this window
 *	includes a "window tool bar", a tool bar that is associated with a specific
 *	window, such as the kind seen in popular word processors.
 *	��Picture Buttons are more versatile than most custom controls because you
 *	design their appearance and their behavior.  It's almost like having a button-
 *	building kit at your disposal.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow8 (void)
	{																					/*Define constants for the various		*/
																						/*	bahavior and appearance						*/
																						/*	specifications�										*/
																						/*Big 3D toolbar buttons stay down		*/
																						/*	when selected, lightened when			*/
																						/*	selected, and get whited out when	*/
																						/*	disabled�													*/
	#define			BigToolBarSpec			picbutLockSelected + picbutBigSICN3D + picbutSelectLightenSICN3D + picbutDimUsingWhite
																						/*Small 3D toolbar buttons pop back up*/
																						/*	after being selected, lightened		*/
																						/*	when selected, and get whited out	*/
																						/*	when disabled�										*/
	#define			SmallToolBarSpec		picbutSelectLightenSICN3D + picbutDimUsingWhite
																						/*The Plus/Minus button pair produce	*/
																						/*	repeated doPictButton events while*/
																						/*	they are held down, have an				*/
																						/*	alternate image when selected, and*/
																						/*	don't look different when					*/
																						/*	disabled�													*/
	#define			PlusMinusSpec				picbutRepeatEvents + picbutSelectAltImage + picbutDimNoChange
																						/*The ordinary icons (printer and			*/
																						/*	modem) are selected when the			*/
																						/*	mouse-down occurs, stay down when	*/
																						/*	(selected, darken when selected.	*/
																						/*	When disabled, use a white screen	*/
																						/*	effect and preserve the border�		*/
	#define			OrdinaryIconSpec		picbutInstantEvent + picbutLockSelected + picbutSelectDarken + picbutDimUsingWhite + picbutDimLeaveBorder
																						/*These tapedeck control buttons are	*/
																						/*	selected when the mouse-down			*/
																						/*	occurs, stay down when selected		*/
																						/*	and use an alternate icon. They		*/
																						/*	look the same disabled�						*/
	#define			TapedeckLockSpec	picbutInstantEvent + picbutLockSelected + picbutSelectAltImage + picbutDimNoChange
																						/*These tape deck controls to not lock*/
																						/*	down, and they look the same			*/
																						/*	disabled�													*/
	#define			TapedeckSpec			picbutSelectAltImage + picbutDimNoChange	/*			*/


																						/*Open the window (it automatically		*/
																						/*	shifts down to compensate for an	*/
																						/*	open Tool Bar, and its position is*/
																						/*	"tiled")�													*/
  WindowOpen(DemoWindow8, 0, 0, 472, 315, "\pPicture Buttons", noGrowDocProc + wTile, GoAway, NotModal);	
  ToolBarValue = 100;												/*Initialize the tool bar's value			*/

  DrawWindow8Contents();										/*Draw all the objects in this demo		*/
  																					/*	window except for the buttons.		*/
  																					/*	This is done first to create a		*/
  																					/*	toolbar on which some buttons can	*/
  																					/*	be placed.  All Tools Plus objects*/
  																					/*	are automatically updated when a	*/
  																					/*	window needs to be refreshed, such*/
  																					/*	as when you bring it to the front	*/
  																					/*	to make it active.								*/
  																					/*		Non-Tools Plus objects (ie:			*/
  																					/*	those you create) have to be			*/
  																					/*	redrawn separately in response to	*/
  																					/*	a doRefresh event, so it's a good	*/
  																					/*	idea to keep them in a separate		*/
  																					/*	routine.													*/

																						/*Create a pop-up menu in the title		*/
																						/*	bar menu using the window's font	*/
																						/*	(Geneva 9)�												*/
	if (HasColorQuickDraw())									/*If Color Quickdraw is available�		*/
		RGBForeColor(&ToolBarGray);							/*	set foreground color to match the	*/
																						/*	toolbar. The Pop-Up Menu picks		*/
																						/*	this up to remember which color is*/
																						/*	behind the Pop-Up Menu.						*/
  NewPopUp(popMenu1, 9, 7, 68, 7, "\p", popupUseWFont + popupNoArrow + popupNeverDimOutline + popupNeverDimSelection, enabled);
  PenColorNormal();													/*Restore to default colors						*/
  PopUpMenu(popMenu1, 1, enabled, "\pNormal!�");	/*Populate the pop-up menu with	*/
  PopUpMenu(popMenu1, 2, disabled, "\p-");				/*	some items.	Demo 7 gives you*/
  PopUpMenu(popMenu1, 3, enabled, "\pPromote");		/*	details about how pop-up		*/
  PopUpMenu(popMenu1, 4, enabled, "\pDemote");		/*	menus are created.					*/
  PopUpMenu(popMenu1, 5, enabled, "\pLateral");		/*															*/

																						/*Create a pop-up menu to control the	*/
																						/*	 "globe" button's "acceleration		*/
																						/*	curve"�														*/
  NewPopUp(ButtonSpeedMenu, 406, 219, 460, 219, "\p�Rate:", popupUseWFont + popupNoArrow, enabled);	
  PopUpMenu(ButtonSpeedMenu, 1, enabled, "\pLinear");	/*Populate the pop-up menu	*/
  PopUpMenu(ButtonSpeedMenu, 2, enabled, "\pSlow");		/*	with the choices�				*/
  PopUpMenu(ButtonSpeedMenu, 3, enabled, "\pMedium!�");/*													*/
  PopUpMenu(ButtonSpeedMenu, 4, enabled, "\pFast");		/*													*/



																						/*C r e a t e   a l l   p i c t u r e	*/
																						/*	b u t t o n s �										*/
  NewPictButton(LeftArrowButton, 103, 3, LeftArrowIcon, picbutSelectAltImage + picbutDimNoChange, enabled, notSelected, 0, 0, 0);
  NewPictButton(RightArrowButton, 126, 3, RightArrowIcon, picbutSelectAltImage + picbutDimNoChange, enabled, notSelected, 0, 0, 0);

  NewPictButton(LeftAlignButton, 172, 3, LeftAlignIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(CenterAlignButton, 195, 3, CenterAlignIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(RightAlignButton, 218, 3, RightAlignIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(JustifyButton, 241, 3, JustifyIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
  
  NewPictButton(CutButton, 288, 4, CutIcon, SmallToolBarSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(BucketButton, 311, 4, BucketIcon, SmallToolBarSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(ClipboardButton, 334, 4, ClipboardIcon, SmallToolBarSpec, enabled, notSelected, 0, 0, 0);

  NewPictButton(PlusButton, 441, 5, PlusIcon, PlusMinusSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(MinusButton, 441, 15, MinusIcon, PlusMinusSpec, enabled, notSelected, 0, 0, 0);
  SetPictButtonSpeed(PlusButton, 2);				/*Set the button speed to 2 steps per	*/
  SetPictButtonSpeed(MinusButton, 2);				/*	second for these two buttons.			*/

  NewPictButton(PrinterButton, 28, 153, PrinterIcon, OrdinaryIconSpec, enabled, selected, 0, 0, 0);
  NewPictButton(ModemButton, 69, 153, ModemIcon, OrdinaryIconSpec, enabled, notSelected, 0, 0, 0);

	/*	This stepping button has nine icon images in its SICN resource.  The				*/
	/*	button's value changes automatically with the top 1/2 of the button					*/
	/*	incrementing the value and the bottom 1/2 decrementing it (one step each		*/
	/*	time the button is clicked). This is a BID 3D button that looks "pushed"		*/
	/*	when selected (not darker or ligher)�																				*/
  NewPictButton(SteppingButton, 147, 161, ScrollingIcon, picbutAutoValueChg + picbutTopBottomSplit + picbutBigSICN3D + picbutSelectPushedSICN3D, enabled, notSelected, 1, 3, 9);

	/*	This scrolling button has nine icon images in its SICN resource.  It keeps	*/
	/*	producing doPictButton events while the mouse is held down.  The button's		*/
	/*	value changes automatically with the top 1/2 of the button incrementing the	*/
	/*	value and the bottom 1/2 decrementing it.  This is a BID 3D button that			*/
	/*	looks "pushed" when selected (not darker or ligher)�												*/
  NewPictButton(ScrollingButton, 170, 161, ScrollingIcon, picbutRepeatEvents + picbutAutoValueChg + picbutTopBottomSplit + picbutBigSICN3D + picbutSelectPushedSICN3D, enabled, notSelected, 1, 5, 9);

	/*	This is just about the most complicated button you can create.  It keeps		*/
	/*	producing doPictButton events while the mouse is held down.  The button's		*/
	/*	value changes automatically with the right 1/2 of the button incrementing		*/
	/*	the value and the left decrementing it.  The rate of change accelerates (at	*/
	/*	a medium rate) over time, and the button's values "wrap" (maximum + 1 =			*/
	/*	start at minimum).  An alternate icon is used when the button is selected.	*/
	/*	A white screen is applied to this icon when the button is disabled.					*/
  NewPictButton(GlobeButton, 410, 153, GlobeIcon, picbutRepeatEvents + picbutAutoValueChg + picbutScaleMedAccel + picbutValueWrap + picbutLeftRightSplit + picbutSelectAltImage + picbutDimUsingWhite, enabled, notSelected, -179, 0, 180);

																						/*Tapedeck buttons always have 1			*/
																						/*	selected�													*/
  NewPictButton(RewindButton, 17, 235, RewindIcon, TapedeckLockSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(StopButton, 48, 235, StopTapeIcon, TapedeckSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(FastFwdButton, 79, 235, FastFwdIcon, TapedeckLockSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(PauseButton, 17, 255, PauseIcon, TapedeckSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(RecordButton, 48, 255, RecordIcon, TapedeckSpec, enabled, notSelected, 0, 0, 0);
  NewPictButton(PlayButton, 79, 255, PlayIcon, TapedeckLockSpec, enabled, notSelected, 0, 0, 0);

	/*	This is a dual state button (Power-On, Power-Off), so it is considered to		*/
	/*	be a "Multi-Stage" button (Off=0, 1=On). The button's value changes					*/
	/*	automatically when selected by the user.  The values "wrap" to allow 1 (on)	*/
	/*	to start back at 0 (off) again.  An alternate icon is used to depict the		*/
	/*	selected button (down position), and disabled button for maximum visual			*/
	/*	control�																																		*/
  NewPictButton(PowerButton, 173, 235, PowerIcon, picbutMultiStage + picbutAutoValueChg + picbutValueWrap + picbutSelectAltImage + picbutDimAltImage, enabled, notSelected, 0, 0, 1);


	/*	This is a dual state button that uses a set of PICTs. Like the button above,*/
	/*	it is Multi-State (left=0, right=1) and its value changes automatically when*/
	/*	selected by the user.  This button uses different PICTs for the selected		*/
	/*	state, and for the screen depth (8-bit, 4-bit, and B&W). It also uses a			*/
	/*	mask, even though it is not necessary on a white background.								*/
  NewPictButton(FlipButton, 163, 277, FlipSwitchPICT, picbutAutoValueChg + picbutLeftRightSplit + picbutMultiStage + picbutUsePICTS + picbutGray4use8 + picbutSelectAltImage + picbutDimNoChange, enabled, notSelected, 0, 0, 1);

																						/*This is a really simple push-button	*/
																						/*	that uses an alternate icon when	*/
																						/*	the button is selected. When			*/
																						/*	disabled, a white screen is				*/
																						/*	overlayed and the border is				*/
																						/*	preserved�												*/
  NewPictButton(DoneButton, 392, 280, DoneIcon, picbutSelectAltImage + picbutDimUsingWhite + picbutDimLeaveBorder, enabled, notSelected, 0, 0, 0);

  EnableMenu(FileMenu, CloseItem, enabled);	/*Enable the File menu's Close				*/
  																					/*	command, now that there's a window*/
  }																					/*	to close.													*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   9 :
 *	In Tools Plus, creating a Tool Bar is as easy as opening a window.  Tools Plus
 *	makes sure the Tools Bar is always at the front, and always active.  This Tool
 *	Bar contains a Pop-Up Menu and several Picture Buttons (they are simply a
 *	duplicate of the "window tool bar" seen in demo 8.
 *		Tools Plus automatically hides the Tool Bar when your application is
 *	suspended, and displays it again when your application resumes (when running
 *	under MultiFinder� or System 7).
 *		This routine is used to both open and close the application's Tool Bar.  Note
 *	that all open windows can optionally be shifted down when a Tool Bar is opened
 *	to make sure that no windows are hidden behind the Tool Bar.  You can also tell
 *	Tools Plus to offset future windows downward to account for the Tool Bar's space
 *	on the monitor.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow9 (void)
	{																					/*These constants describe the behavior	*/
																						/*	and look of picture buttons in the	*/
																						/*	Tool Bar. Demo 8 provides more			*/
																						/*	detail about picture buttons. All		*/
																						/*	the buttons here have been					*/
																						/*	duplicated from Demo 8�							*/
	#define			BigToolBarSpec			picbutLockSelected + picbutBigSICN3D + picbutSelectLightenSICN3D + picbutDimUsingWhite
	#define			SmallToolBarSpec		picbutSelectLightenSICN3D + picbutDimUsingWhite


																						/* O p e n   T o o l   B a r �					*/
	if (!WindowIsOpen(ToolBarWindow))					/*If the Tool Bar is not open�					*/
		{																				/*																			*/
																						/*Open a Tool Bar that is 26 pixels			*/
																						/*	high.  Shift all open windows down	*/
																						/*	as the Tool Bar opens, and as future*/
																						/*	windows are opened, offset them			*/
																						/*	downward to compensate for the open	*/
																						/*	Tool Bar�														*/
		ToolBarOpen(ToolBarWindow, 26, tbShiftWindows + tbOffsetNewWindows);	/*				*/

    RenameItem(DemosMenu, 10, "\pHide Tool Bar");	/*Rename the menu item						*/

    DrawWindow9Contents();									/*Paint the Tool Bar gray.  Picture			*/
																						/*	buttons will be placed on top of the*/
																						/*	gray.  All Tools Plus objects are		*/
																						/*	automatically updated when a window	*/
																						/*	needs to be refreshed.							*/
																						/*		Non-Tools Plus objects (ie: those	*/
																						/*	you create) have to be redrawn			*/
																						/*	separately in response to a					*/
																						/*	doRefresh event, so it's a good idea*/
																						/*	to keep them in a separate routine.	*/
		if (HasColorQuickDraw())								/*If Color Quickdraw is available�			*/
			RGBForeColor(&ToolBarGray);						/*	set foreground color to match the		*/
																						/*	toolbar. The Pop-Up Menu picks this	*/
																						/*	up to remember which color is behind*/
																						/*	the Pop-Up Menu.										*/
		TextFont(geneva);												/*The pop-up menu will use Geneva 9pt		*/
    TextSize(9);														/*																			*/
																						/*Create the pop-up menu�								*/
    NewPopUp(popMenu1, 9, 7, 68, 7, "\p", popupUseWFont + popupNoArrow + popupNeverDimOutline + popupNeverDimSelection, enabled);
    PenColorNormal();												/*Restore to default colors							*/
    PopUpMenu(popMenu1, 1, enabled, "\pNormal!�");	/*Populate the pop-up menu with	*/
    PopUpMenu(popMenu1, 2, enabled, "\pSection");		/*	some items.	Demo 7 gives you*/
    PopUpMenu(popMenu1, 3, enabled, "\pBullet");		/*	details about how pop-up		*/
    PopUpMenu(popMenu1, 4, enabled, "\pTable");			/*	menus are created.					*/

																						/*C r e a t e   a l l   p i c t u r e		*/
																						/*	b u t t o n s �											*/
																						/*These are the same picture buttons you*/
																						/*	saw described in detail in Demo 8�	*/
		NewPictButton(LeftArrowButton, 103, 2, LeftArrowIcon, picbutSelectAltImage + picbutDimNoChange, enabled, notSelected, 0, 0, 0);
    NewPictButton(RightArrowButton, 126, 2, RightArrowIcon, picbutSelectAltImage + picbutDimNoChange, enabled, notSelected, 0, 0, 0);
    NewPictButton(LeftAlignButton, 172, 2, LeftAlignIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
    NewPictButton(CenterAlignButton, 195, 2, CenterAlignIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
    NewPictButton(RightAlignButton, 218, 2, RightAlignIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
    NewPictButton(JustifyButton, 241, 2, JustifyIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
    NewPictButton(CutButton, 288, 3, CutIcon, SmallToolBarSpec, enabled, notSelected, 0, 0, 0);
    NewPictButton(BucketButton, 311, 3, BucketIcon, SmallToolBarSpec, enabled, notSelected, 0, 0, 0);
    NewPictButton(ClipboardButton, 334, 3, ClipboardIcon, SmallToolBarSpec, enabled, notSelected, 0, 0, 0);
		}


  else	{																		/* C l o s e   T o o l   B a r �				*/
    WindowClose(ToolBarWindow);							/*Close the Tool Bar's window (all open	*/
																						/*	windows automatically move up.			*/
    RenameItem(DemosMenu, 10, "\pShow Tool Bar");	/*Rename the menu item						*/
   	}
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   10 :
 *	In Tools Plus, creating a floating palette (windoid) is as easy as opening a
 *	window.  Tools Plus makes sure the palette if opened behind the Tool Bar (if
 *	there is one) and in front of standard windows (if there are any).  Palettes
 *	are always active.
 *		Tools Plus automatically hides palettes when your application is suspended,
 *	and displays them when your application resumes (when running under MultiFinder�
 *	or System 7).
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow10 (void)
	{																					/*All picture buttons on this floating	*/
																						/*	palette are small SICN 3D icons			*/
																						/*	(colorized by Tools Plus).  They		*/
																						/*	respond instantly when clicked�			*/
	#define			SmallButtonSpec		picbutInstantEvent + picbutSelectDarkenSICN3D

	short				theButton;										/*Button number counter									*/
	short				theIcon;											/*Icon number counter										*/
	Point				thePoint;											/*Top left corner for button						*/

																						/* O p e n   V e r t i c a l						*/
																						/*		P a l e t t e �										*/
																						/*Open the floating palette (it					*/
																						/*	automatically shifts down to				*/
																						/*	compensate for an open Tool Bar)�		*/
  WindowOpen(VerticalPalette, 450, 40, 495, 229, "\p", paletteProc, GoAway, NotModal);

																						/*C r e a t e   a l l   p i c t u r e		*/
																						/*	b u t t o n s �											*/
																						/*Use some basic math to create the			*/
																						/*	picture buttons for this palette.		*/
																						/*	All the picture buttons are buttons	*/
																						/*	are sequentially numbered, and so is*/
																						/*	their placement on this palette.		*/
  theIcon = FirstVPaletteIcon;							/*Start with using the first icon in		*/
																						/*	this palette.												*/
																						/*Cycle through all buttons in palette�	*/
  for (theButton = VPaletteMinButton; theButton <= VPaletteMaxButton; theButton++)
  	{																				/*																			*/
		thePoint.h = ((theButton -1) % 2) * 23 - 1;	/*Horizontally, odd numbered buttons*/
																								/*	are at 0 and even numbered			*/
																								/*	buttons are at 23 pixels.				*/
    thePoint.v = ((theButton - 1) / 2) * 19 - 1;/*Two buttons fit side-by side			*/
    																				/*	horizontally, so after each two			*/
    																				/*	buttons, shift down 19 pixels.			*/
																						/*Create the picture button�						*/
		NewPictButton(theButton, thePoint.h, thePoint.v, theIcon, SmallButtonSpec, enabled, notSelected, 0, 0, 0);
    theIcon = theIcon + 1;									/*Increment the icon for the next				*/
    }																				/*	picture button.											*/
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C r e a t e   D e m o   W i n d o w   11 :
 *	In Tools Plus, creating a floating palette (windoid) is as easy as opening a
 *	window.  Tools Plus makes sure the palette if opened behind the Tool Bar (if
 *	there is one) and in front of standard windows (if there are any).  Palettes
 *	are always active.
 *		Tools Plus automatically hides palettes when your application is suspended,
 *	and displays them when your application resumes (when running under MultiFinder�
 *	or System 7).
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OpenDemoWindow11 (void)
	{																					/*All picture buttons on this floating	*/
																						/*	palette respond instantly when			*/
																						/*	clicked.  The image darkens when		*/
																						/*	selected, is dithered when disabled	*/
																						/*	(with it border left unaffected)�		*/
	#define			HPaletteButtonSpec	picbutInstantEvent + picbutSelectDarken + picbutDimUsingWhite + picbutDimLeaveBorder

	short				theButton;										/*Button number counter									*/
	short				theIcon;											/*Icon number counter										*/
	Point				thePoint;											/*Top left corner for button						*/
	short				ButtonsPerRow;								/*Number of buttons per row on palette	*/
	short				ButtonInRow;									/*Button number in current row (1 to x)	*/


																						/* O p e n   H o r i z o n t a l				*/
																						/*		P a l e t t e �										*/
																						/*Open the floating palette (it					*/
																						/*	automatically shifts down to				*/
																						/*	compensate for an open Tool Bar)�		*/
	WindowOpen(HorizontalPalette, 239, 260, 508, 299, "\p", altPaletteProc, GoAway, NotModal);

																						/*C r e a t e   a l l   p i c t u r e		*/
																						/*	b u t t o n s �											*/
																						/*Use some basic math to create the			*/
																						/*	picture buttons for this palette.		*/
																						/*	All the picture buttons are buttons	*/
																						/*	are sequentially numbered, and so is*/
																						/*	their placement on this palette.		*/
																						/*	Double borders between groups of		*/
																						/*	buttons are created by placing the	*/
																						/*	buttons further apart.							*/
  theIcon = FirstHPaletteIcon;							/*Start with using the first icon in		*/
  																					/*	this palette.												*/
  																					/*Cycle through all buttons in palette�	*/
  for (theButton = HPaletteMinButton; theButton <= HPaletteMaxButton; theButton++)
  	{																				/*																			*/
																						/*Calc num. of picture buttons per row	*/
																						/*	(with two rows on this palette)�		*/
    ButtonsPerRow = (HPaletteMaxButton - HPaletteMinButton + 1) / 2;	/*						*/
    if (theButton <= ButtonsPerRow)					/*Calculate the button's relative number*/
			ButtonInRow = theButton;							/*	in this row (i.e. the first button	*/
		else																		/*	in this row, second, third, etc.)  	*/
			ButtonInRow = theButton - ButtonsPerRow;/*	This number is used to calculate	*/
																						/*	horizontal button spacing.					*/
																						/*Horizontally, each button is 24 pixels*/
																						/*	apart from the previous one. There	*/
																						/*	are borders at the end of the 4th,	*/
																						/*	9th and 10th button, so add an extra*/
																						/*	2 pixels between those buttons�			*/
    thePoint.h = (ButtonInRow - 1) * 24 + (2 * (ButtonInRow > 4)) + (2 * (ButtonInRow > 9)) + (2 * (ButtonInRow > 10)) - 1;
    thePoint.v = (theButton > ButtonsPerRow) * 20 - 1;	/*Half of the buttons fit on*/
    																				/*	the top row, and the second half of	*/
    																				/*	them fit on the bottom row. The			*/
    																				/*	bottom row is 20 pixels lower than	*/
    																				/*	the first.													*/
																						/*Create the picture button�						*/
    NewPictButton(theButton, thePoint.h, thePoint.v, theIcon, HPaletteButtonSpec, enabled, notSelected, 0, 0, 0);
    theIcon = theIcon + 1;									/*Increment the icon for the next				*/
    }																				/*	picture button.											*/
	}





/***********************************************************************************
 ***********************************************************************************
 *  R e s p o n s e s   t o   E v e n t s :
 *	The following routines are invoked in response to events that were detected in
 *	the main event loop.
 */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * R e f r e s h   A   W i n d o w :
 *	This routine is called in response to a "doRefresh" event.  It redraws the
 *	contents of a window.  Note that objects such as Scroll Bars, List Boxes,
 *	Buttons and Editing Fields are updated automatically.
 *		You should account for each of your application's windows in a routine such
 *	as this one, even if they are modal.  Modal windows may need updating if the Mac
 *	running your application is running a screen saver.  When the screen saver is
 *	woken up, chances are that it will demand that all windows be refreshed.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void RefreshWindow (void)
	{
  CurrentWindow(Poll.Window);								/*Make the affected window the current*/
  																					/*	grafPort. That means that all			*/
  																					/*	drawing operations will be done		*/
  																					/*	on that window.										*/
  BeginUpdate(WindowPointer(Poll.Window));	/*Restrict drawing to within the area	*/
  																					/*	that needs refreshing.  Any				*/
  																					/*	drawing done outside this area is	*/
  																					/*	ignored.													*/
  switch (Poll.Window)											/*Depending on which window needs to	*/
  	{																				/*	be refreshed, call the appropriate*/
		case DemoWindow1: 											/*	routine to do the required				*/
    	DrawWindow1Contents();								/*	drawing�													*/
    	break;																/*																		*/
		case DemoWindow2: 											/*																		*/
			DrawWindow2Contents();								/*																		*/
    	break;																/*																		*/
		case DemoWindow3: 											/*																		*/
			DrawWindow3Contents();								/*																		*/
    	break;																/*																		*/
		case DemoWindow4: 											/*																		*/
			DrawWindow4Contents();								/*																		*/
    	break;																/*																		*/
		case DemoWindow5: 											/*																		*/
			DrawWindow5Contents();								/*																		*/
    	break;																/*																		*/
		case DemoWindow6: 											/*																		*/
			DrawWindow6Contents();								/*																		*/
    	break;																/*																		*/
		case DemoWindow7: 											/*																		*/
			DrawWindow7Contents();								/*																		*/
    	break;																/*																		*/
		case DemoWindow8: 											/*																		*/
			DrawWindow8Contents();								/*																		*/
    	break;																/*																		*/
    case ToolBarWindow:  										/*																		*/
			DrawWindow9Contents();								/*																		*/
    	break;																/*																		*/
		case VerticalPalette: 									/*																		*/
			DrawWindow10Contents();								/*																		*/
    	break;																/*																		*/
		case HorizontalPalette: 								/*																		*/
			DrawWindow11Contents();								/*																		*/
    	break;																/*																		*/
   }																				/*																		*/
	EndUpdate(WindowPointer(Poll.Window));		/*End the update for the window, and	*/
																						/*	tell the Mac you've finished			*/
																						/*	updating this window (ie: clear		*/
																						/*	the window's update region).			*/
  CurrentWindowReset();											/*Reset the active window to also be	*/
  																					/*	the current window (all drawing		*/
  }																					/*	will resume on the active window).*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * U s e r   C l i c k e d   I n a c t i v e   W i n d o w :
 *	This routine is called in response to a "doChgWindow" event.  Note that
 *	clicking between the active window and a desk accessory, or between two desk
 *	accessories does not generate a doChgWindow event. 	A doChgWindow event is
 *	generated only when the user tries to activate an inactive window belonging
 *	to your application.  In a full featured application, you may want to validate
 *	the active editing field and perform any other verifications before activating
 *	the window.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ChangeWindow (void)
	{
  ActivateWindow(Poll.Window);							/*Activate the requested window				*/
	}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C h o o s e   A   M e n u :
 *	This routine is called in response to a "doMenu" event.  A doMenu event
 *	indicates that the user selected a menu or typed a Command-key equivalent for
 *	a menu.  Note that a doMenu event is not generated if a desk accessory's menu
 *	is selected.  The Edit menu's Undo, Cut, Copy, Paste, and Clear items
 *	automatically interact with the active editing field without you having to do
 *	anything.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MenuSelection (void)
	{
	switch (Poll.Menu.Num)
		{
		case ApplMenu:													/* A p p l e  M e n u :	-	-	-	-	-	-	-	*/
																						/*The only item in the Apple menu that*/
																						/*	generates an event is the "About�"*/
																						/*	item.  All we do is display a			*/
																						/*	Dynamic Alert box�								*/
			AlertBox(NoIcon, "\pT o o l s   P l u s   D e m o   A p p l i c a t i o n\r\rWater�s Edge Software\rPO Box 70022\r2441 Lakeshore Road West\rOakville, Ontario\rCanada, L6L 6M9\r\r(416) 219-5628\r\r(Click this window to continue)", NoButtonAlert);
			break;																/*																		*/

		case FileMenu: 													/* F i l e   M e n u :	-	-	-	-	-	-	-	*/
			switch (Poll.Menu.Item)								/*Determine the item selected within	*/
				{																		/*	this menu�												*/
				case CloseItem: 										/* Close�															*/
     			if (FirstStdWindowNumber() != 0)					/*If standard window is open�	*/
						CloseTheWindow(FirstStdWindowNumber());	/*	close front std window.		*/
					else if (FirstPaletteNumber() != 0)				/*If flating palette is open�	*/
						CloseTheWindow(FirstPaletteNumber());		/*	close the front palette.	*/
					break;																		/*														*/
				case QuitItem: 											/* Quit�															*/
					ExitTheDemo = true;								/*	set the "exit the demo" flag to		*/
					break;														/*	true to indicate we're ready to		*/
				}																		/*	return to the Finder.							*/
			break;																/*																		*/
				
		case DemosMenu:													/* D e m o s   M e n u :	-	-	-	-	-	-	*/
			{																			/*When selecting an item from the			*/
																						/*	Demos menu, it means "activate		*/
																						/*	this demo window" if the window		*/
																						/*	is already open, and "open this		*/
																						/*	demo window" if it isn't open.		*/

			if (Poll.Menu.Item == ToolBarItem)		/*If user selected the Hide/Show Tool */
				OpenDemoWindow9();									/*	Bar item,	open or closes (hide)		*/
			else																	/*	the Tool Bar.											*/
				{																		/*																		*/


				ActivateWindow(Poll.Menu.Item);			/*Try to activate the window number		*/
																						/*	that corresponds to the menu's		*/
																						/*	item number. This will bring the	*/
																						/*	window to the front.							*/
				if (!WindowIsOpen(Poll.Menu.Item))	/*If the specified window is not open,*/
					{																	/*	it indicates the requested demo		*/
																						/*	window is not open yet. So open		*/
																						/*	the requested demo�								*/
					switch (Poll.Menu.Item)						/*Determine the item selected within	*/
						{																/*	this menu	and open the specified	*/
						case 1: 												/*	demo window.											*/
							OpenDemoWindow1();						/*																		*/
							break;												/*																		*/
						case 2: 												/*																		*/
							OpenDemoWindow2();						/*																		*/
							break;												/*																		*/
						case 3: 												/*																		*/
							OpenDemoWindow3();						/*																		*/
							break;												/*																		*/
						case 4: 												/*																		*/
							OpenDemoWindow4();						/*																		*/
							break;												/*																		*/
						case 5: 												/*																		*/
							OpenDemoWindow5();						/*																		*/
							break;												/*																		*/
						case 6: 												/*																		*/
							OpenDemoWindow6();						/*																		*/
							break;												/*																		*/
						case 7: 												/*																		*/
							OpenDemoWindow7();						/*																		*/
							break;												/*																		*/
						case 8: 												/*																		*/
							OpenDemoWindow8();						/*																		*/
							break;												/*																		*/
						} 															/*																		*/
					} 																/*																		*/
				} 																	/*																		*/
			} 																		/*																		*/
			break;																/*																		*/

		case PaletteDemoMenu: 									/* P a l e t t e   D e m o s   M e n u*/
			{																			/*The "Palettes" menu is hierarchic,	*/
																						/*	so you treat it like a separate		*/
																						/*	pull-down menu. When selecting an	*/
																						/*	item from the Palettes Demos menu,*/
																						/*	it means "activate this palette		*/
																						/*	window" if the window is already	*/
																						/*	open, and "open this palette			*/
																						/*	window" if it isn't open.					*/
			ActivateWindow(Poll.Menu.Item + VerticalPalette - 1);	/*Try to activate the	*/
																						/*	palette number that corresponds to*/
																						/*	the menu's item number. This will	*/
																						/*	bring the palette to the front.		*/
			if (!WindowIsOpen(Poll.Menu.Item + VerticalPalette - 1)) /*If the specified	*/
				{																		/*	palette is not open then display	*/
																						/*	the palette�											*/
				switch (Poll.Menu.Item)							/*Determine the item selected within	*/
					{																	/*	this menu and open the specified	*/
					case 1:														/*	palette window.										*/
						OpenDemoWindow10();							/*																		*/
						break;													/*																		*/
					case 2: 													/*																		*/
						OpenDemoWindow11();							/*																		*/
						break;													/*																		*/
					}																	/*																		*/
				}																		/*																		*/
			}																			/*																		*/
			break;																/*																		*/
		}


	if (!ExitTheDemo)													/*If the user has not quit the app�		*/
		MenuHilite(0);													/*	turn off the highlighted menu in	*/
																						/*	the menu bar.  This shows the user*/
																						/*	that the application has responded*/
	}																					/*	to the menu selection.						*/