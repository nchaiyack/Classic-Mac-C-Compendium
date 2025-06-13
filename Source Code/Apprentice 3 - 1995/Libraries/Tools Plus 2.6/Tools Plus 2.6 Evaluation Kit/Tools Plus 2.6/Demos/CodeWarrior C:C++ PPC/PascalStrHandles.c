/* 	Tools Plus 2.6 -- "C / Pascal String Handles" supplemental file
 *	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *	re: Editing Fields
 *
 *	Each Tools Plus editing field uses a “string handle” (ie: a handle to a Pascal
 *	string) to store the text that is displayed in the field.  The following
 *	routines have been provided to facilitate moving C strings into Pascal Handle
 *	structures, and vice versa.
 *
 *	A Pascal string’s first byte (byte-0) is a length byte, and the string is not
 *	null-terminated. Pascal strings are limited to 255 characters.
 *
 *	NOTE: These routines do not check to determine if the destination variable
 *				is large enough to accommodate the text being copied there.  You may
 *				want to modify these routines to do such validation.
 */

#pragma options align=mac68k




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Copy a C string into a Pascal “string handle”
 */
void Cstr2PHdl(char CSource[], Handle PDest);	// (Prototype)
void Cstr2PHdl(char CSource[],								/*Source C string										*/
							 Handle PDest)									/*Handle to destination Pascal			*/
																							/*	string.													*/
	{
	short			length;														/*Source string's length						*/


	length = strlen(CSource);										/*Determine length of source string	*/
	(**PDest) = (unsigned char)length;					/*Copy length-byte into byte-0			*/
	BlockMove(CSource, (*PDest) + 1, length);		/*Copy source C string (omitting		*/
																							/*	null terminator) to the Pascal	*/
																							/*	string right after the length-	*/
	}																						/*	byte.														*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Copy the contents of a Pascal “string handle” into a C string
 */
void PHdl2Cstr(Handle PSource, char CDest[]);	//	(Prototype)
void PHdl2Cstr(Handle PSource,								/*Handle to source Pascal string		*/
							 char CDest[])									/*Destination C string							*/
	{
	short			length;														/*Source string's length						*/

	length = (unsigned char)(**PSource);				/*Determine length of source string	*/
	BlockMove((*PSource) + 1, CDest, length);		/*Copy source Pascal string					*/
																							/*	(omitting the length byte) to		*/
																							/*	the C string.										*/
	CDest[length] = '\0';												/*Null-terminate the C string				*/
	}																						/*																	*/



#pragma options align=resettton requires two icons, one for the button's normal state, and
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
 *		Window 1			This window demonstrates how Tools Plus works with…
 *										• List Boxes
 *										• Buttons (radio buttons, check boxes and push buttons)
 *										• Cursor Zones
 *
 *		Window 2			This window demonstrates how Tools Plus works 