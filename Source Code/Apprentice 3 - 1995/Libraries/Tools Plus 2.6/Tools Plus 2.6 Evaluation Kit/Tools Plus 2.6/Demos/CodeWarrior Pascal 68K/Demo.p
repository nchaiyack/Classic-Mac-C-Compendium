{	Tools Plus	--	D e m o   P r o g r a m	--	(Version 2.6)	using MPW Headers														}
{	Copyright (c) 1993-94 Water's Edge Software																										}
{	Designed and programmed by Steve Makohin																										}

{	All files in this project use the Helvetica font (9pt).																									}
{	This application is written as a single source file (ie: I deliberately did NOT split it into units) to simplify				}
{	compiling for programmers that are just starting out.  It also demonstrates that programming with Tools Plus		}
{	makes source code compact and readable.																											}
{																																													}
{	If you are put off by the size of this program, please note that it is H E A V I L Y   C O M M E N T E D and it is not	}
{	optimized, both of which were done to facilitate the teaching of Tools Plus.															}
{																																													}
{	Though you will likely be able to figure out what each line of source code does, extensive comments are			}
{	provided so you won't have to constantly refer to the Tools Plus User Manual and Inside Macintosh.					}


{	R e s o u r c e s :																																						}
{	This demo uses resources that can be found in the 'Demo Resources' file.  The following describes each of the	}
{	resources and what function they perform in this application.  You can add, change, delete and view resources}
{	by using a resource editor application such as Apple's ResEdit.																			}
{																																													}
{	Type	  ID			Description																																	}
{	------		------			----------------																																	}
{	acur		128			Animated Cursor definition:  Tells Tools Plus how to animate the cursor to create a wrist			}
{								watch with a spinning second hand.  The 'acur' resource specifies the sequence of cursor		}
{								resources ('CURS') that will be used for the animation, and the time taken between steps.		}
{								See the User Manual for details.																									}
{																																													}
{	BNDL	128			Bundle Resource:  This resource tells the Finder which icons are used to depict the				}
{								application and the documents created by it.  The BNDL resource bundles the following			}
{								related resources: FREF, icl4, icl8, ICN#, and 'TP 1'																	}
{																																													}
{	cicn		- -				Color Icons (definable size):  This is the first color icon format available on color (Mac II)		}
{								Macintoshes.  Each 'cicn' includes a color image, the equivalent black and white image, and	}
{								a mask.  In this demo, all 'cicn' icons were created using 4-bit colors to ensure that they			}
{								look good when displayed on monitors set to 16-colors/grays or higher.									}
{				300-303	These color icons (hard disk, floppy, and folder) show up in the Pop-Up Menus demo beside	}
{								menu item names.																														}
{				304,305	These 'large' color icons (printer and modem) appear in 'picture Pop-Up Menus' (picture in a	}
{								Pop-Up Menu without any text).  There are 'ICON' equivalents for these items for Macs			}
{								without Color QuickDraw.																											}
{				430-441	These "tape deck" buttons are used in the Picture Buttons demo to show you the look you		}
{								can get with Tools Plus's buttons.  Each button requires two icons, one for the button's			}
{								normal state, and one for the selected (pushed in) state.  I know, they don't behave like a		}
{								real tape deck, but this is only a demo.																						}
{				460,461	This bi-directional "globe" button is used in the Picture Buttons demo.  The two icons show		}
{								the button in its normal state, and in its selected (pushed in) state.											}
{				464,465	This "Done" button is used in the Picture Buttons demo.  The two icons show the button in		}
{								its normal state, and in its selected (pushed in) state.  Even the most ordinary buttons can	}
{								look sophisticated with Tools Plus.																								}
{				520-541	These are all used in the Floating Palettes demo to create clickable icons.  You can create		}
{								professional looking palettes with various styles of buttons using Tools Plus.							}
{																																													}
{	CURS	128-134	Cursors:  These cursors are used to produce an animated cursor.  The 'acur' resource			}
{								specified which cursors are used, and the animation's speed.  See the User Manual for			}
{								details.																																		}
{																																													}
{	FREF	128			Finder Reference:  This resource is automatically created when a BNDL resource is created.	}
{								One FREF exists for each icon that the Finder references.  This demo is an application			}
{								(file type is 'APPL'), so one FREF is needed for that.  If this demo had the ability to create		}
{								three different kinds of documents, then one FREF would be required for each kind type of		}
{								document.																																	}
{																																													}
{	icl4		- -				Large (32x32 pixel) 4-bit Icons:  These icons are typically used only by the Finder to depict	}
{								an application and its related files.  However, your application can also draw these icons		}
{								with Tools Plus.  4-bit icons can have a maximum of 16 colors.													}
{				0,1,2		These color icons will be used on color monitors as a substitute for the system's Stop, Note	}
{								and Caution icons.  They give color applications a better look.													}
{				128			Application's Icon (displayed by the Finder.  The BNDL resource refers to this icon)				}
{				150			Printer icon used to demonstrate drawing icon families																}
{				151			Modem icon used to demonstrate drawing icon families																}
{			400,401		Left Arrow button used in Picture Buttons demo (part of an icl8 / icl4 / ICN# family).  One icon	}
{								is used for the button's standard position, and another for the selected (pushed in) position.	}
{			402,403		Right Arrow button used in Picture Buttons demo (similar to above)											}
{			420-425		Plus/Minus button used in Picture Buttons demo.  These 4 icons are used in a particularly		}
{								attractive button.  The top half (+) darkens when you select it, and so does the bottom (-).		}
{								Two icons (standard and selected position) are used for the top half, and two for the bottom.	}
{								These icons are part of an icl8 / icl4 / ICN# family.																		}
{			450-456		Four icons are used to draw a dual-stage button (Power-On / Power-Off) in the Picture			}
{								Buttons demo; a pair for each stage, one for the standard state and one for the selected		}
{								(pushed in) state.  An additional pair of icons ause used to portray the buttons as disabled;	}
{								one for Power-On and one for Power-Off.  These icons are part of an icl8 / icl4 / ICN# family.	}
{																																													}
{	icl8		- -				Large (32x32 pixel) 8-bit Icons:  These icons are typically used only by the Finder to depict	}
{								an application and its related files.  However, your application can also draw these icons		}
{								with Tools Plus.  8-bit icons can have a maximum of 256 colors.												}
{				1,2			These color icons will be used on color monitors as a substitute for the system's Note and		}
{								Caution icons.  They give color applications a better look.  Notice that there isn't a Stop icon	}
{								(ID=0).  The equivalent icl4 will be used instead.																		}
{				128			Application's Icon (displayed by the Finder.  The BNDL resource refers to this icon)				}
{				150			Printer icon used to demonstrate drawing icon families																}
{				151			Modem icon used to demonstrate drawing icon families																}
{			400,401		Left Arrow button used in Picture Buttons demo (part of an icl8 / icl4 / ICN# family).  One icon	}
{								is used for the button's standard position, and another for the selected (pushed in) position.	}
{			402,403		Right Arrow button used in Picture Buttons demo (similar to above)											}
{			420-425		Plus/Minus button used in Picture Buttons demo.  These 4 icons are used in a particularly		}
{								attractive button.  The top half (+) darkens when you select it, and so does the bottom (-).		}
{								Two icons (standard and selected position) are used for the top half, and two for the bottom.	}
{								These icons are part of an icl8 / icl4 / ICN# family.																		}
{			450-456		Four icons are used to draw a dual-stage button (Power-On / Power-Off) in the Picture			}
{								Buttons demo; a pair for each stage, one for the standard state and one for the selected		}
{								(pushed in) state.  An additional pair of icons ause used to portray the buttons as disabled;	}
{								one for Power-On and one for Power-Off.  These icons are part of an icl8 / icl4 / ICN# family.	}
{																																													}
{	ICN#	- -				Large (32x32 pixel) 1-bit Icons with Mask:  These icons are typically used only by the Finder	}
{								to depict an application and its related files.  However, your application can also draw these	}
{								icons with Tools Plus.  1-bit icons are black and white (no shades of gray).								}
{				128			Application's Icon (displayed by the Finder.  The BNDL resource refers to this icon)				}
{				150			Printer icon used to demonstrate drawing icon families																}
{				151			Modem icon used to demonstrate drawing icon families																}
{			400,401		Left Arrow button used in Picture Buttons demo (part of an icl8 / icl4 / ICN# family).  One icon	}
{								is used for the button's standard position, and another for the selected (pushed in) position.	}
{			402,403		Right Arrow button used in Picture Buttons demo (similar to above)											}
{			420-425		Plus/Minus button used in Picture Buttons demo.  These 4 icons are used in a particularly		}
{								attractive button.  The top half (+) darkens when you select it, and so does the bottom (-).		}
{								Two icons (standard and selected position) are used for the top half, and two for the bottom.	}
{								These icons are part of an icl8 / icl4 / ICN# family.																		}
{			450-456		Four icons are used to draw a dual-stage button (Power-On / Power-Off) in the Picture			}
{								Buttons demo; a pair for each stage, one for the standard state and one for the selected		}
{								(pushed in) state.  An additional pair of icons ause used to portray the buttons as disabled;	}
{								one for Power-On and one for Power-Off.  These icons are part of an icl8 / icl4 / ICN# family.	}
{																																													}
{	ICON	- -				Large (32x32 pixel) 1-bit Icons:  These are standard black & white icons.									}
{				0,1,2		These icons will be used as a substitute for the system's Stop, Note and Caution icons.			}
{								In this case, they are exactly the same as System 7's system icons, but they are included	}
{								in case this application runs on System 5 (where the system icons are different).  There are	}
{								color counterparts for these icons (icl4 and icl8).																		}
{			304,305		These printer and modem icons appear in 'picture Pop-Up Menus' (picture in a Pop-Up Menu	}
{								without any text) on Macs without Color QuickDraw.  There are 'cicn' equivalents for these		}
{								items for Macs with Color QuickDraw.																							}
{																																													}
{	ics4		- -				Small (16x16 pixel) 4-bit Icons:  These icons are typically used only by the Finder to depict	}
{								an application and its related files.  4-bit icons can have a maximum of 16 colors.					}
{				128			Application's Icon (displayed by the Finder.  The BNDL resource refers to this icon)				}
{																																													}
{	ics8		- -				Small (16x16 pixel) 8-bit Icons:  These icons are typically used only by the Finder to depict	}
{								an application and its related files.  8-bit icons can have a maximum of 256 colors.					}
{				128			Application's Icon (displayed by the Finder.  The BNDL resource refers to this icon)				}
{																																													}
{	ics#		- -				Small (16x16 pixel) 1-bit Icons with mask:  These icons are typically used only by the Finder	}
{								to depict an application and its related files.  1-bit icons are black & white.								}
{				128			Application's Icon (displayed by the Finder.  The BNDL resource refers to this icon)				}
{																																													}
{	PICT	- -				Pictures of any dimension, number of colors (including gray scale and black & white)				}
{				128			Black & white (1-bit) picture.  It's the black & white equivalent of resource 129.						}
{				129			Color (8-bit) picture.  It's the color equivalent of resource 128.													}
{			150-173		Complete picture suite needed for a 2-stage button. Each stage (value) can be either				}
{								or not, and includes different PICTs for screen depths of 8-bits, 4-bits, and B&W. They also	}
{								include a mask, though this is not necessary when drawing on a white background.				}
{																																													}
{	SICN	- -				Small (16x16 pixel) 1-bit Icons:  These icons are 'indexed', meaning that you can have			}
{								multiple icon images in a single 'SICN' resource.																		}
{			300-303		These color icons (hard disk, floppy, and folder) show up in the Pop-Up Menus demo beside	}
{								menu item names. They are supplied because Macs without Color QuickDraw need SICNs		}
{								(they can't use color 'cicn' icons).																								}
{			408-414		These are all used in the Picture Buttons demo to create 3D icons.  Tools Plus converts an	}
{								SICN icon into a color 3D button.																								}
{				458			These are all used in the Picture Buttons demo to create 3D icons.  Tools Plus converts an	}
{								SICN icon into a color 3D button.  Because this SICN resource has several images, each		}
{								image becomes an available 'state' (or usable value).  In this example, the user can step		}
{								through the stages by clicking on the button.																				}
{			500-519		These are all used in the Floating Palettes demo to create 3D icons.  Tools Plus converts an	}
{								SICN icon into a color 3D button.  You can create professional looking palettes with different	}
{								styles of buttons using Tools Plus.																								}
{																																													}
{	SIZE	-1				Multi-tasking info:  The 'SIZE' resource is needed if your application is going to run under		}
{								MultiFinder (System 5 and 6) or System 7.  It tells the Macintosh how your application				}
{								should behave in a multi-tasking environment.  See the User Manual for details.						}
{																																													}
{	TP 1		128			Owner Resource:  This resource is automatically created when a BNDL resource is created.	}
{								This demo's signature is 'TP 1', and as you may recall, each application must have a unique	}
{								signature code (see the User Manual for details).  That signature code is used to create an	}
{								'owner resource' whose resource type is the same as the application's signature code.			}
{																																													}
{	vers		- -				Application Version:  These resources' information is displayed in the Finder's Get Info box.	}
{				1				This information is displayed at the bottom of the Get Info box.  It includes an application		}
{								version number and the copyright info.																						}
{				2				This information is displayed at the top of the Get Info box.  It is a comment that is displayed	}
{								beneath the application's name (ie:  'Tools Plus demo').															}
{																																													}
{	WDEF		- -			Window Definition:  WDEFs define how a window looks.  If you want to open a window that is	}
{								not one of the six standard Macintosh window types, you have to create a window definition.	}
{				2000		Pallet Window:  These windows have a narrow title bar, and are typically used as 'floating		}
{								windows' (they are always active, and they float above your other windows).  This demo has	}
{								several floating pallets.																												}


{	T h e   D e m o s :																																						}
{	This demo application can be used as a framework for a new application, although you will probably want to		}
{	improve upon it to cater to your own programming style and needs.  The demo can be broken down into several	}
{	sections:																																									}
{																																													}
{			main					The main program (at the bottom of this source code) demonstrates a typical application's	}
{									startup and response to events.																								}
{																																													}
{	PrepareTheDemo	This routine creates the demos menus and opens a couple demo windows.							}
{																																													}
{		Window 1				This window demonstrates how Tools Plus works with…														}
{									• List Boxes																																}
{									• Buttons (radio buttons, check boxes and push buttons)														}
{									• Cursor Zones																															}
{																																													}
{		Window 2				This window demonstrates how Tools Plus works with…														}
{									• Editing Fields (typing, Cut, Copy, Paste, Clear, and Undo.  Tabbing to next / prev field)		}
{									• Scroll Bars																																}
{									• 'Size box' and the 'zoom box' (changing a window's size)													}
{																																													}
{		Window 3				This window demonstrates how Tools Plus works with…														}
{									• drawing icon families																												}
{									• Cursor Zones used to make icons click sensitive																	}
{																																													}
{		Window 4				This window shows off Tools Plus's cursor animation, and how Tools Plus behaves when a	}
{									watch cursor is displayed.																										}
{																																													}
{		Window 5				This window demonstrates how to write code for color drawing that is dependent on the		}
{									monitor's settings (ie: number of colors, number of shades of gray, or black & white).  It		}
{									also shows how to write code that is compatible with Macintoshes that have multiple			}
{									monitors.																																	}
{																																													}
{		Window 6				This demo shows off Tools Plus's Dynamic Alerts.  They are alert boxes that automatically	}
{									adjust to accommodate the text your application provides them.											}
{																																													}
{		Window 7				This window demonstrates Tools Plus's Pop-Up Menus and some of the features you can	}
{									incorporate into them.																												}
{																																													}
{		Window 8				This window features Tools Plus's Picture Buttons, which transform icons or pictures  into	}
{									powerful buttons.  Picture Buttons have a list of properties that you can select and				}
{									combine to create your own custom Picture Buttons.  Picture Button properties control the}
{									look and behavior of the button.																								}
{																																													}
{		Window 9				Window 9 is this application's Tool Bar.  Tools Plus lets you create a Tool Bar as easily as	}
{									any other Tools Plus window.  You can put any control you want in the Tool Bar, including	}
{									picture buttons (the most common thing), pop-up menus, and editing fields.  Demo Window}
{									8 describes picture buttons in greater detail.																			}
{																																													}
{		Window 10			Window 10 is this a floating palette.  Tools Plus lets you create a floating palette as easily	}
{									as any other Tools Plus window.  You can put any control you want in the Tool Bar,				}
{									including picture buttons (the most common thing), pop-up menus, and editing fields.			}
{									Demo Window 8 describes picture buttons in greater detail.													}
{																																													}
{		Window 11			Window 11 is this also a floating palette (like demo window 10), except this is a horizontal	}
{									palette with the title bar along the window's left side.																}


{	S o u n d   M a n a g e r :																																			}
{	This demo uses the Sound Manager just for entertainment's sake.  If this demo runs on a Mac that is running	}
{	less than System 6, simple beeps will be played instead of sounds.																		}





program Demo;
	uses
		Controls, Fonts, Lists, Menus, Packages, QuickDraw, Resources, Sound, TextEdit, ToolsPlus, ToolUtils;



																										{G l o b a l s   t y p e s   a n d   v a r i a b l e s …		}
																										{These CONSTants, TYPEs and VARiables		}
																										{	are accessible throughout this program.		}
	const
{	All these constants will make the demo's code more readable.  You don't HAVE to use them.  You can use		}
{	literals, such as 1, 2, 3… if you want to.  It all depends on your writing style and needs.										}
		DemoWindow1 = 1;																	{Window numbers for the demo…						}
		DemoWindow2 = 2;																	{																			}
		DemoWindow3 = 3;																	{																			}
		DemoWindow4 = 4;																	{																			}
		DemoWindow5 = 5;																	{																			}
		DemoWindow6 = 6;																	{																			}
		DemoWindow7 = 7;																	{																			}
		DemoWindow8 = 8;																	{																			}
		ToolBarWindow = 9;																	{																			}
		VerticalPalette = 10;																{																			}
		HorizontalPalette = 11;															{																			}
		OkButton = 1;																			{Buttons used on Demo Window 1…					}
		CancelButton = 2;																	{	When writing a real application, you can			}
		MiniButton1 = 11;																	{	give these constants meaningful names,		}
		MiniButton2 = 12;																	{	such as 'OK', 'Cancel', 'Detail', 'Summary'		}
		MiniButton3 = 13;																	{	etc.																	}
		MiniButton4 = 14;																	{																			}
		MiniButton5 = 15;																	{																			}
		CheckBox1 = 21;																		{																			}
		CheckBox2 = 22;																		{																			}
		CheckBox3 = 23;																		{																			}
		RadioButton1 = 31;																	{																			}
		RadioButton2 = 32;																	{																			}
		RadioButton3 = 33;																	{																			}
		MidiHalfMeg = 11;																	{																			}
		MidiOneMeg = 12;																	{																			}
		MidiTwoMeg = 13;																	{																			}
		LeftArrowButton = 1;																{Picture Buttons used on Window 8…					}
		RightArrowButton = 2;																{	When writing real applications, you can use	}
		LeftAlignButton = 3;																{	constants, variables, calculations, or a			}
		CenterAlignButton = 4;															{	number in place of these items.						}
		RightAlignButton = 5;																{																			}
		JustifyButton = 6;																	{																			}
		CutButton = 7;																			{																			}
		BucketButton = 8;																	{																			}
		ClipboardButton = 9;																{																			}
		PlusButton = 10;																		{																			}
		MinusButton = 11;																	{																			}
		PrinterButton = 12;																	{																			}
		ModemButton = 13;																	{																			}
		SteppingButton = 14;																{																			}
		ScrollingButton = 15;																{																			}
		GlobeButton = 16;																	{																			}
		RewindButton = 17;																	{																			}
		StopButton = 18;																		{																			}
		FastFwdButton = 19;																{																			}
		PauseButton = 20;																	{																			}
		RecordButton = 21;																	{																			}
		PlayButton = 22;																		{																			}
		PowerButton = 23;																	{																			}
		FlipButton = 24;																		{																			}
		DoneButton = 25;																	{																			}
		VPaletteMinButton = 1;															{First and last picture button used on the			}
		VPaletteMaxButton = 20;														{	vertical floating palette demo.							}
		HPaletteMinButton = 1;															{First and last picture button used on the			}
		HPaletteMaxButton = 22;														{	horizontal floating palette demo.						}
		LeftArrowIcon = 400;																{Icons and Picture used to make Picture			}
		RightArrowIcon = 404;																{	Buttons on Window 8…									}
		LeftAlignIcon = 408;																{																			}
		CenterAlignIcon = 409;															{																			}
		RightAlignIcon = 410;																{																			}
		JustifyIcon = 411;																	{																			}
		CutIcon = 412;																			{																			}
		BucketIcon = 413;																	{																			}
		ClipboardIcon = 414;																{																			}
		PlusIcon = 420;																		{																			}
		MinusIcon = 424;																		{																			}
		ScrollingIcon = 458;																{																			}
		GlobeIcon = 460;																		{																			}
		RewindIcon = 430;																	{																			}
		StopTapeIcon = 432;																{																			}
		FastFwdIcon = 434;																	{																			}
		PauseIcon = 436;																	{																			}
		RecordIcon = 438;																	{																			}
		PlayIcon = 440;																		{																			}
		PowerIcon = 450;																	{																			}
		FlipSwitchPICT = 150;																{																			}
		DoneIcon = 464;																		{																			}
		FirstVPaletteIcon = 500;															{First icon number on the vertical and horizontal}
		FirstHPaletteIcon = 520;															{	palettes.  Other icons nums are calculated.	}
		LeftList = 1;																				{List Boxes used on Demo Window 1…				}
		RightList = 2;																			{																			}
		RightBar = 1;																			{Scroll Bars used on Demo Window 2…				}
		BottomBar = 2;																		{																			}
		ApplMenu = 0;																			{Menus and Menu Items…									}
		AboutItem = 1;																			{																			}
		FileMenu = 1;																			{																			}
		CloseItem = 3;																			{																			}
		QuitItem = 5;																			{																			}
		EditMenu = 2;																			{																			}
		DemosMenu = 3;																		{																			}
		ToolBarItem = 10;																	{Item number for Tool Bar on the Demos menu	}
		PaletteDemoMenu = 16;															{Hierarchic menus use numbers 16-200				}
		popMenu1 = 1;																			{Pop-Up Menu numbers										}
		popMenu2 = 2;																			{																			}
		popMenu3 = 3;																			{																			}
		popMenu4 = 4;																			{																			}
		popMenu5 = 5;																			{																			}
		ButtonSpeedMenu = 6;															{																			}
		PrinterIcon = 150;																	{Printer and Modem icons…								}
		ModemIcon = 151;																	{																			}
		BlackAndWhitePICT = 128;														{Color and Black & White pictures…					}
		ColorPICT = 129;																		{																			}
		WatersEdgeLogo = 128;															{Logo for Water's Edge Software (icon)				}

	type
		Str30Handle = ^Str30Ptr;														{A 30-character string is defined which can be	}
		Str30Ptr = ^Str30;																	{	referenced by handle.  Editing fields use		}
		Str30 = string[30];																	{	handles to automatically maintain their text.	}

	var
		Poll: TPPollRecord;																	{Polling record to retrieve event information		}
		hField1: Str30Handle;																{Handle to 1st editing field's 30-char string			}
		hField2, hField3: StringHandle;												{Handles to 2nd and 3rd editing fields' 255-char	}
																										{	string.																}
		MidiPort: integer;																		{Serial port selected for MIDI interface (Demo 3)}
		MidiSpeed: integer;																	{MIDI interface speed (Demo 3)							}
		ToolBarGray: RGBColor;															{Color of toolbar													}
		ToolBarValue: integer;															{Value displayed in a window's tool bar (Demo 8)}
		ExitTheDemo: boolean;															{Should the demo terminate?								}






{ D r a w   T e x t   i n   a   B o x : =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =	}
{	This generic routine performs an often used task of drawing text within a specified rectangle.								}
	procedure TextInBox (Str: str255;												{The string that's going to be drawn					}
									left, top, right, bottom: integer;					{Box's co-ordinates												}
									Justification: integer;									{Left-aligned, centered, or right-aligned				}
									withBox: boolean);										{Draw a box around the text?								}
		var
			Rectangle: rect;																	{Box's co-ordinates specified as a rectangle		}
	begin
		SetRect(Rectangle, left, top, right, bottom);							{Convert the specified co-ordinates into a rect.	}
		TextBox(ptr(ord(@Str) + 1), length(Str), Rectangle, Justification);	{Draw the text in the specified rectangle	}
		if withBox then																			{If a box is needed around the text…					}
			FrameRect(Rectangle);														{	draw the rectangle around the text.				}
	end;																								{																			}





{  A c t i o n   i n   a   d e m o   W i n d o w : =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =	}
{	The following routines respond to actions made by the user in a demo window.  They are invoked by the main	}
{	event loop whenever the user does any of the following things:																				}
{		• Click a button							(doButton event)																								}
{		• Type a key									(doKeyDown or doAutoKey event, excluding those in editing fields)				}
{		• Click in an inactive editing field	(doClickField event)																						}
{		• Use a scroll bar							(doScrollBar event, excluding those in List Boxes)										}
{		• Click a line in a List Box				(doListBox)																										}
{		• Use a Pop-Up Menu					(doPopUpMenu)																								}
{		• Click in the window					(doClick, excluding clicking on other objects listed above)							}


{ C l o s e   A   W i n d o w : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine is called in response to a "doGoAway" event, the File menu's Close command, and any from some	}
{	demos that end when you click the Cancel or OK button.  Note that a doGoAway event is not generated when a}
{	desk accessory's "close" box is clicked.																												}
{		A doGoAway event indicates that the user clicked the active window's "close" box.  In a full featured				}
{	application, you may want to validate the active editing field and perform any other verifications before closing	}
{	the window.																																								}
	procedure CloseTheWindow (Window: integer);							{Window number being closed								}
	begin
		WindowClose(Window);															{Close the specified window								}
		if FirstWindowNumber = none then											{If this application does not have any open		}
																										{	windows…														}
			EnableMenu(FileMenu, CloseItem, disabled);					{Disable the File menu's Close command,			}
	end;																								{	because all the windows are closed.				}


{ A c t i o n   i n   D e m o   W i n d o w   1 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine responds to actions made by the user in demo window 1.  The actions include:									}
{		• Clicking or double-clicking a line in a List Box																									}
{		• Clicking a check box																																			}
{		• Clicking or double clicking a radio button																											}
{		• Clicking a push-button																																		}
{	Notice that if you type Enter or Return, Tools Plus automatically selects the default push button (OK).				}
	procedure ActionInWindow1;
		var
			CheckedBox: boolean;														{Was the clicked check box originally selected?}
			ButtonNum: integer;															{Button counter for stepping through as set of	}
																										{	buttons.															}
			NumberOfSounds: integer;													{Number of sounds in the System file					}
			SoundIndex: integer;															{Relative sound number going to be played		}
			hSound: handle;																	{Handle to a sound resource								}
			OSerr: integer;																	{Macintosh operating system error						}
	begin
		case Poll.What of																	{Determine what the user did (what event)…		}

																										{ B u t t o n   W a s   S e l e c t e d :	-	-	-	-	-	}
																										{This section demonstrates typical interaction	}
																										{	with check boxes, radio button groups and	}
																										{	push buttons.  Look at the radio-button			}
			doButton:																			{	group for 'double-clicking' ability.					}
				case Poll.Button.Num of													{Determine which button was selected…			}

					MiniButton1..MiniButton5:											{ M i n i - B u t t o n s :	-		-		-		-		-		}
						begin																		{The mini buttons simply play one of the sounds}
																										{	available in the System file.							}
							NumberOfSounds := CountResources('snd ');	{Determine the number of sounds available		}
							if (NumberOfSounds = none) or (SystemVersion < 6.0) then	{If there are no sounds available,	}
								Beep																{	or if the System version does not support the}
																										{	Sound Manager (older than System 6), just	}
																										{	beep the user.													}
							else																		{If there are sounds available in the System…	}
								begin																{	calculate a 'sound index' number such that	}
																										{	button 1 is the first sound, button 2 is the		}
																										{	second, etc…													}
									SoundIndex := max(1, (Poll.Button.Num - (MiniButton1 - 1)) mod NumberOfSounds);	{		}
									hSound := GetIndResource('snd ', SoundIndex);	{Load the sound rsrc from System file	}
									OSerr := SndPlay(nil, hSound, false);			{Play the sound resource (ignore the error)		}
									ReleaseResource(hSound);						{Release the resource to conserve memory		}
								end																	{																			}
						end;
																										{ C h e c k   B o x e s :	-		-		-		-		-		}
																										{When a check box is clicked, it reverses its		}
																										{	current state (If on, turn off.  If off, turn on).	}
																										{	In this example, the third check box can		}
																										{	only be selected if the second one is				}
					CheckBox1..CheckBox3:											{	selected.															}
						begin																		{																			}
							CheckedBox := ButtonIsSelected(Poll.Button.Num);{Calc if check box was originally selected	}
							CheckedBox := not CheckedBox;						{Reverse the check box's selection (if on, turn	}
							SelectButton(Poll.Button.Num, CheckedBox);	{	off.  If off, turn on.)											}
							if Poll.Button.Num = CheckBox2 then					{If the second check box was clicked…				}
								if CheckedBox then											{	enable/disable button CheckBox3.  This		}
									EnableButton(CheckBox3, enabled)			{	shows how easy it is to set up an "and"			}
								else																	{	condition with two or more check boxes.		}
									begin															{		Check box 3 is deselected and disabled		}
										EnableButton(CheckBox3, disabled);	{	when check box 2 is not selected.  When		}
										SelectButton(CheckBox3, notSelected);{	check box 2 is selected, then check box 3		}
									end																{	is enabled.														}
						end;																			{																			}

																										{ R a d i o   B u t t o n s :	-		-		-		-		-	}
																										{Radio buttons are logically organized into			}
																										{	groups, and only one radio button can be on	}
																										{	at a time.  Your application can optionally do	}
																										{	something else if a radio button is double-		}
																										{	clicked.  In this case, it is the equivalent of	}
					RadioButton1..RadioButton3:										{	selecting the radio button and clicking OK.	}
						begin																		{																			}
							for ButtonNum := RadioButton1 to RadioButton3 do	{Cycle through each button in the group…	}
								SelectButton(ButtonNum, ButtonNum = Poll.Button.Num);	{Select the clicked one, deselect}
																																{	the others.								}
							if Poll.Button.DoubleClick then							{If a radio button was double-clicked…				}
								begin																{	it means 'select this button and OK'.  Flash	}
									FlashButton(OkButton);								{	the OK button to indicate it's being selected.	}
									CloseTheWindow(DemoWindow1);				{End this demo by closing its window					}
								end																	{																			}
						end;																			{																			}

																										{ P u s h   B u t t o n s :	-		-		-		-		-		}
					CancelButton, OkButton:											{If the user clicked Cancel or OK…						}
						CloseTheWindow(DemoWindow1);							{	end this demo by closing its window.				}
				end;																					{																			}

																										{ L i s t   B o x e s :		-		-		-		-		-		}
																										{Your application may want to respond to a user}
																										{	selecting a line in a List Box.  This demo just	}
																										{	responds to double-clicks by being the			}
																										{	equivalent of selecting the line and clicking	}
																										{	OK.																	}
			doListBox:																			{																			}
				if Poll.ListBox.DoubleClick then										{If a list box was double-clicked…						}
					begin																			{	it means 'select this line and OK'.  Flash the	}
						FlashButton(OkButton);											{	OK button to indicate it's being selected.		}
						CloseTheWindow(DemoWindow1);							{End this demo by closing its window					}
					end;																				{																			}

			otherwise																				{All other events are ignored								}
		end																							{																			}
	end;


{ A c t i o n   i n   D e m o   W i n d o w   2 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -		}
{	This routine responds to actions made by the user in demo window 2.  The actions include:									}
{		• Clicking or double-clicking in an inactive editing field																						}
{		• Using Scroll Bars																																				}
{		• Tabbing to the next (or previous) field																												}
	procedure ActionInWindow2;
		var
			Field: integer;																		{Field number														}
	begin
		case Poll.What of																	{Determine what the user did (what event)…		}

																										{ I n a c t i v e   F i e l d   W a s   C l i c k e d :	-	-	}
																										{In a full-featured application, you would likely	}
																										{	validate a field for errors before proceeding	}
																										{	to the one clicked by the user.  In this demo,	}
			doClickField:																		{	no validation is done.										}
				begin																				{																			}
					SaveFieldString;															{Save the field's edited text as the field's string	}
					ClickInField;																{Process the event to activate the clicked field	}
				end;																					{	and to set the insertion point or selection		}
																										{	range as per the click.										}
	
																										{ S c r o l l   B a r   W a s   U s e d :	-	-	-	-	-	-	}
																										{Tools Plus tells your application when a scroll	}
																										{	bar is used by either clicking in the up arrow,	}
																										{	down arrow, "page up" region, "page down"		}
																										{	region, or by moving the thumb.  Your				}
																										{	application would then respond by doing		}
																										{	something.  This demo doesn't actually			}
																										{	scroll anything.												}
																										{		Note: When setting a Scroll Bar's value,		}
																										{	the minimum / maximum limit are						}
																										{	automatically adjusted (if necessary).  Here	}
																										{	we use the limits to prevent the current			}
																										{	value from being exceeded.							}
			doScrollBar:																		{																			}
				case Poll.ScrollBar.Part of												{Appropriate action is taken according to the		}
					inUpButton:																	{	part of the scroll bar that was clicked…			}
						SetScrollBarVal(Poll.ScrollBar.Num, max(GetScrollBarMin(Poll.ScrollBar.Num), GetScrollBarVal(Poll.ScrollBar.Num) - 1));
					inPageUp:
						SetScrollBarVal(Poll.ScrollBar.Num, max(GetScrollBarMin(Poll.ScrollBar.Num), GetScrollBarVal(Poll.ScrollBar.Num) - 10));
					inDownButton:
						SetScrollBarVal(Poll.ScrollBar.Num, min(GetScrollBarMax(Poll.ScrollBar.Num), GetScrollBarVal(Poll.ScrollBar.Num) + 1));
					inPageDown:
						SetScrollBarVal(Poll.ScrollBar.Num, min(GetScrollBarMax(Poll.ScrollBar.Num), GetScrollBarVal(Poll.ScrollBar.Num) + 10));
					inThumb:																		{Ignore thumb movement									}
				end;
																										{ U s e r   T y p e d   a   K e y :	-	-	-	-	-	-	-	}
																										{If the active window has an active editing field,	}
																										{	it automatically processes most key				}
																										{	strokes.  In this example, we only check for	}
																										{	the Tab key, which is used to tab to the next	}
																										{	or previous field (Shift-Tab)…							}
			doKeyDown, doAutoKey:													{																			}
				if (Poll.Key.Chr = TabKey) and not (Poll.Modifiers.CmdKey or Poll.Modifiers.OptionKey or Poll.Modifiers.ControlKey) then
					begin																			{If the Tab key was typed without the Command,}
																										{	Option or Control modifier keys, tab to the		}
																										{	next / previous field.										}
																										{		In a full-featured application, you would		}
																										{	likely validate a field for errors before				}
																										{	proceeding to the field clicked by the user.	}
																										{	In this demo, no validation is done.					}
						SaveFieldString;														{Save the field's edited text as the field's string	}
						Field := ActiveFieldNumber;									{Determine the active field number						}
						if not Poll.Modifiers.ShiftKey then							{TAB: to next field…											}
							Field := Field + 1 - 3 * ord(Field = 3)						{	Add 1.  If field=3, start at 1 again.					}
						else																			{SHIFT-TAB: to previous field…							}
							Field := Field - 1 + 3 * ord(Field = 1);						{	Subtract 1.  If field=1, start at 3.						}
						ActivateField(Field, teSelectAll);							{Select all the text in the newly activated field	}
					end;																				{																			}

			otherwise																				{All other events are ignored								}
		end																							{																			}
	end;


{ A c t i o n   i n   D e m o   W i n d o w   3 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine responds to actions made by the user in demo window 3.  The actions include:									}
{		• Clicking on an icon (Cursor Zone over the icon)																								}
{		• Clicking a radio button																																		}
	procedure ActionInWindow3;
		var
			ClickedZone: integer;															{Cursor zone clicked by user								}
			Button: integer;																	{Button number counter										}
	begin
		case Poll.What of																	{Determine what the user did (what event)…		}

																										{ U s e r   C l i c k e d   I n   T h e   W i n d o w :	-	}
																										{The only click we care about in this window is if	}
			doClick:																				{	it occurs in either of the two icons…				}
				begin																				{																			}
					ResetMouseClicks;														{Only 1st mouse-down is needed.  Ignore rest.	}
					ClickedZone := FindCursorZone(Poll.Mouse.Down[1].Where);	{Determine which cursor zone was		}
																														{	clicked by the user.							}
					if (ClickedZone <> 0) and (ClickedZone <> MidiPort) then{If a Cursor Zone was clicked, and the clicked}
						begin																		{	zone is different from the currently selected	}
																										{	MIDI port (ie: it was changed)…						}
							if ClickedZone = PrinterIcon then						{Depending on which icon was clicked, set the	}
								MidiPort := PrinterIcon									{	MidiPort variable to the selected icon.			}
							else																		{																			}
								MidiPort := ModemIcon;									{																			}
							DrawIcon(PrinterIcon, 20, 32, enabled, MidiPort = PrinterIcon);	{Redraw the printer and modem}
							DrawIcon(ModemIcon, 65, 32, enabled, MidiPort = ModemIcon);	{	icons as currently selected.}
						end																										{												}
				end;																												{												}

			doButton:																			{ U s e r   C l i c k e d   A   B u t t o n :	-	-	-	-	-	}
				begin																				{																			}
					for Button := MidiHalfMeg to MidiTwoMeg do				{Cycle through all 3 speed buttons and set only	}
						SelectButton(Button, Button = Poll.Button.Num);	{	the selected one.  Turn the others off.			}
					MidiSpeed := Poll.Button.Num;									{Keep track of the selected button						}
				end;																					{																			}

			otherwise																				{Ignore all other events										}
		end																							{																			}
	end;


{ A c t i o n   i n   D e m o   W i n d o w   4 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine responds to actions made by the user in demo window 4.  This routine is here for cosmetic reasons}
{	only, and you can get rid of it if you want.																												}
	procedure ActionInWindow4;
	begin
	end;


{ A c t i o n   i n   D e m o   W i n d o w   5 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine responds to actions made by the user in demo window 5.  The only thing the user can do is click the}
{	default OK button, which closes the window.																											}
	procedure ActionInWindow5;
	begin
		if Poll.What = doButton then													{If the user clicked a button…								}
			CloseTheWindow(DemoWindow5);										{	end this demo by closing its window.				}
	end;																								{																			}


{ A c t i o n   i n   D e m o   W i n d o w   6 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine responds to actions made by the user in demo window 6.  Dynamic Alerts take care of responding	}
{	to any action from the user, so you don't have to write any code for it.  This routine is here for cosmetic				}
{	reasons only, and you can get rid of it if you want.																									}
	procedure ActionInWindow6;
	begin
	end;


{ A c t i o n   i n   D e m o   W i n d o w   7 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - 	}
{	This routine responds to actions made by the user in demo window 7.  The only thing you can do is select an		}
{	item in a pop-up menu.																																			}
	procedure ActionInWindow7;
		const
			SilentContinueAlert = -11500;												{Button definition for a "Continue" dynamic alert	}
																										{	that doesn't beep the user.								}
		var
			AltButton: integer;																{Button number clicked in an alert box				}
	begin
		if Poll.What = doPopUpMenu then											{The only thing you can do: use a pop-up menu	}
			case Poll.Menu.Num of														{Determine which Pop-Up Menu was selected…	}

				popMenu1, popMenu5:													{Pop-Up Menus 1 and 5 use a bullet (•) to mark a}
					PopUpMark(Poll.Menu.Num, Poll.Menu.Item, DotChar);{	selected item.  Mark the new selection now.}

				popMenu2, popMenu3:													{Pop-Up Menus 2 and 3 use a check (√) to mark	}
					CheckPopUp(Poll.Menu.Num, Poll.Menu.Item, on);	{	selected items.  Mark the new selection.		}

				popMenu4:																		{This menu is a "fixed title" pop-down menu.		}
																										{	Such menus are usually used to "do some-	}
																										{	thing now".  This demo just displays an alert.	}
					AltButton := AlertBox(WatersEdgeLogo, 'Your application would do something now, such as displaying a dialog that lets the user specify formatting characteristics.', SilentContinueAlert);
			end																						{																			}
	end;


{ A c t i o n   i n   D e m o   W i n d o w   8 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -    -	}
{	There are three routines for responding to events in this window.  The first two are button value drawing				}
{	routines that display a button's value.  They are called when the button is clicked, or when the window is			}
{	refreshed.																																								}

{	This routine displays a value in the tool bar.  It is called whenever the 'plus' or 'minus' buttons are used, and		}
{	when the screen is refreshed.																																	}
	procedure ActionInWindow8_DrawToolBarValue;
		var
			theNumber: str255;																{Button's value represented as a string				}
	begin
		NumToString(ToolBarValue, theNumber);								{Convert tool bar number to a string					}
		TextFont(systemFont);															{Set the window's font to Chicago 12pt.				}
		TextSize(12);																			{																			}
		PenColorNormal;																		{PEN: 1 x 1 size, black color, patCopy mode		}
		TextInBox(theNumber, 401, 6, 439, 22, teJustCenter, true);	{Draw the value in the window's tool bar with a	}
	end;																								{	frame around it.												}

{	This routine displays the value of the 'Globe' button.  It is called whenever the 'Globe' button is used, and when}
{	the screen is refreshed.																																			}
	procedure ActionInWindow8_DrawGlobeValue;
		var
			ButtonValue: integer;															{Globe button's value											}
			theNumber: str255;																{Button's value represented as a string				}
	begin
		ButtonValue := GetPictButtonVal(GlobeButton);					{Get the 'globe' button's value							}
		NumToString(ButtonValue, theNumber);								{Convert to a string												}
		theNumber := concat(theNumber, '°');										{Add the degree symbol to the end of the string	}
		TextFont(systemFont);															{Set the window's font to Chicago 12pt.				}
		TextSize(12);																			{																			}
		TextInBox(theNumber, 408, 201, 448, 217, teJustRight, false);	{Draw the button's value below the button	}
	end;																										{																	}

{	This routine responds to actions made by the user in demo window 8.  This window contains mostly Picture		}
{	Buttons, so the response is limited to the user clicking on these buttons.  Notice that the buttons are organized}
{	into different types: simple binary clusters, multi-stage buttons, radio button clusters, etc.								}
	procedure ActionInWindow8;
		var
			theButton: integer;																{Button counter													}
	begin
		case Poll.What of																	{Determine what the user did (what event)…		}


			doPopUpMenu:																	{ P o p - U p   M e n u   W a s   S e l e c t e d :			}
				begin																				{																			}
					PopUpMark(Poll.Menu.Num, Poll.Menu.Item, '√');		{Check selected item (others are unchecked)	}
					if Poll.Menu.Num = ButtonSpeedMenu then				{If this menu regulate's the 'globe' button speed,}
						SetPictButtonAccel(GlobeButton, Poll.Menu.Item - 1);	{	set the button's acceleration curve.		}
				end;																							{																	}


			doPictButton:																		{ P i c t u r e   B u t t o n   W a s   S e l e c t e d :		}
				case Poll.Button.Num of													{This section demonstrates typical interaction	}
																										{	with check boxes, radio button groups and	}
																										{	push buttons.													}
					LeftAlignButton, CenterAlignButton, RightAlignButton, JustifyButton: {Cycle through the group and	}
						for theButton := LeftAlignButton to JustifyButton do							{	turn off the ones that weren't}
							SelectPictButton(theButton, theButton = Poll.Button.Num);			{	clicked (deselect them).		}

																										{This is the simplest case for picture buttons,	}
					PrinterButton, ModemButton:										{	clickable icons…												}
						for theButton := PrinterButton to ModemButton do		{Only 1 icon can be on, so turn the other off.	}
							SelectPictButton(theButton, theButton = Poll.Button.Num);	{														}


																										{Tape Deck buttons: Because each button		}
																										{	locks down (if required) by itself, we just		}
																										{	have to deselect any other button that may	}
																										{	be down…														}
					RewindButton, StopButton, FastFwdButton, PauseButton, RecordButton, PlayButton:	{					}
						for theButton := RewindButton to PlayButton do	{Cycle through all the buttons…							}
							if theButton <> Poll.Button.Num then					{If this button was not clicked…							}
								SelectPictButton(theButton, notSelected);	{	deselect the button.										}


					GlobeButton, -GlobeButton:										{If the globe button was selected and/or held…	}
						ActionInWindow8_DrawGlobeValue;						{	show the new value.  The negative case is		}
																										{	for the decrementing side of the button.			}


					PlusButton:																	{'Plus' button in window's tool bar:						}
						if ToolBarValue < 3200 then									{If the current value is less that 3200 (the			}
							begin																	{	upper limit)…													}
								ToolBarValue := ToolBarValue * 2;					{Double the 'zoom factor'									}
								ActionInWindow8_DrawToolBarValue;			{Redisplay the tool bar's number							}
							end;																		{																			}

					MinusButton:																{'Minus' button in window's tool bar:					}
						if ToolBarValue > 25 then										{If the current value is greater than 25 (the lower}
							begin																	{	limit)…																}
								ToolBarValue := ToolBarValue div 2;				{Half the 'zoom factor'											}
								ActionInWindow8_DrawToolBarValue;			{Redisplay the tool bar's number							}
							end;																		{																			}


					DoneButton:																{If the user clicked the 'Done' button…				}
						CloseTheWindow(DemoWindow8);							{	end this demo by closing its window.				}

					otherwise																		{Ignore all other buttons										}
				end;																					{																			}

			otherwise																				{Ignore all other kinds of events							}
		end																							{																			}
	end;																								{																			}


{ A c t i o n   i n   D e m o   W i n d o w   9 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine responds to actions made by the user in demo window 9 (the Tool Bar).  You can select an item in	}
{	Pop-Up Menu or click picture buttons, but the picture buttons don't do anything in this demo.								}
	procedure ActionInWindow9;
		var
			theButton: integer;																{Button counter													}
	begin
		if Poll.What = doPopUpMenu then											{If the pop-up menu was used…							}
			PopUpMark(Poll.Menu.Num, Poll.Menu.Item, '√')				{	check selected item (others are unchecked)	}

																										{If one of the 4 'single selection' buttons was		}
																										{	selected…														}
		else if (Poll.What = doPictButton) and (Poll.Button.Num >= LeftAlignButton) and (Poll.Button.Num <= JustifyButton) then
			for theButton := LeftAlignButton to JustifyButton do			{Cycle through the group and turn off the ones	}
				SelectPictButton(theButton, theButton = Poll.Button.Num);	{	that weren't clicked (deselect them).		}
	end;																										{																	}


{ A c t i o n   i n   D e m o   W i n d o w   10 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine responds to actions made by the user in demo window 10 (a vertical palette).  All the user can do is}
{	click a picture button, but the picture buttons don't do anything in this demo.														}
	procedure ActionInWindow10;
		var
			theButton: integer;																{Button counter													}
	begin
		if Poll.What = doPictButton then												{If the user clicked on a picture button…			}
			for theButton := VPaletteMinButton to VPaletteMaxButton do	{Cycle through the group and turn off the	}
				SelectPictButton(theButton, theButton = Poll.Button.Num);	{	ones that weren't clicked (deselect		}
	end;																										{	them).														}


{ A c t i o n   i n   D e m o   W i n d o w   11 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine responds to actions made by the user in demo window 11 (a horizontal palette).  All the user can do}
{	is click a picture button, but the picture buttons don't do anything in this demo.													}
	procedure ActionInWindow11;
		var
			theButton: integer;																{Button counter													}
	begin
		if Poll.What = doPictButton then												{If the user clicked on a picture button…			}
			for theButton := HPaletteMinButton to HPaletteMaxButton do	{Cycle through the group and turn off the	}
				SelectPictButton(theButton, theButton = Poll.Button.Num);	{	ones that weren't clicked (deselect		}
	end;																										{	them).														}










{  D r a w   D e m o   W i n d o w s '   C o n t e n t s : =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =		}
{	The following routines are used to draw a demo windows' contents.  They are invoked when a demo window is	}
{	first opened, and when a demo window needs to be refreshed.  The contents includes text and lines.  It				}
{	excludes Tools Plus objects such as Buttons, Scroll Bars, List Boxes, Editing Fields, etc.  Those objects are	}
{	refreshed automatically.																																			}


{ D r a w   a   G r o u p   B o x   w i t h   a   T i t l e : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This generic routine performs an often used task of drawing a "Group Box" such as the kind that is seen around}
{	a radio-button group.  It also draws the group's title at the top of the box.  The box is drawn using the pen's		}
{	current size, pattern and transfer mode.  The group's title is drawn using the window's current font, size, and	}
{	style.																																										}
	procedure GroupBox (Str: str255;												{Group's name														}
									left, top, right, bottom: integer);					{Co-ordinates of the group's enclosing box		}
		var
			Rectangle: rect;																	{Box's co-ordinates specified as a rectangle		}
			TextRect: rect;																	{Enclosing ractangle where box will appear		}
			Width: integer;																		{Text's width (for centering it)								}
	begin
		SetRect(Rectangle, left, top, right, bottom);							{Convert the specified co-ordinates into a rect.	}
		FrameRect(Rectangle);															{Draw the group's box											}
		if Str <> '' then																			{If a string was included as the group's title, it	}
			begin																					{	will be centered at the top…							}
				Width := StringWidth(Str);												{Calculate the text's width in pixels						}
				TextRect.left := (Rectangle.left + Rectangle.right) div 2 - (Width div 2) - 2;	{Calculate a rectangle so	}
				TextRect.right := TextRect.left + Width + 4;					{	that it fits on the top edge of the group's box. }
				TextRect.top := Rectangle.top - 8;									{	It's wide enough for the title, plus a 2 pixel		}
				TextRect.bottom := TextRect.top + 16;							{	border on each side.										}
				TextBox(ptr(ord(@Str) + 1), length(Str), TextRect, teJustCenter);	{Draw the group's title						}
			end																											{														}
	end;																													{														}


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   1 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine draws all the text and lines in demo window 1.  It is called when first creating the window and when	}
{	the window needs to be refreshed.  Note that objects that are created by Tools Plus are refreshed						}
{	automatically.																																							}
	procedure DrawWindow1Contents;
	begin
																										{ C o m m e n t s	-	-	-	-	-	-	-	-	-	-	-		}
		PenPat(qd.gray);																	{Use gray pattern for drawing framing boxes		}
		TextFont(Geneva);																	{Draw all comments using Geneva 9pt, plain		}
		TextSize(9);																			{	style.																}
		TextFace([]);																			{																			}
																										{Comments for left list box…								}
		TextInBox('This left text list uses the standard system font, just like normal list boxes.', 38, 108, 236, 140, teJustLeft, false);
																										{Comments for right list box…								}
		TextInBox('This one uses Geneva 9pt.  Each list box can have its own font.', 295, 108, 416, 150, teJustLeft, false);
																										{Comments for Cursor Zones…							}
		TextInBox('This is a "Plus Cursor" zone.  The cursor changes when entering this area.', 275, 150, 400, 189, teJustCenter, true);
		TextInBox('This is a "Cross Cursor" zone.  The cursor changes when entering this area.', 275, 193, 400, 232, teJustCenter, true);
																										{Comments for mini-buttons…							}
		TextInBox('These buttons are drawn in bold Geneva 9pt.  Many different fonts, sizes and styles can be used.', 120, 140, 240, 190, teJustLeft, false);
		PenSize(3, 3);																			{Make the pen 3x3 pixels for a fatter line			}
		GroupBox('', 14, 137, 242, 192);												{Draw a gray, fat box around the mini buttons		}
																										{Comment at bottom of the window…					}
		TextInBox('Double-click a radio button or a line in a list box to mean “select this and click ‘OK’.”  (Note that the OK button flashes.)  This OPTION is easily implemented.', 5, 290, 420, 320, teJustCenter, false);


																										{ L i s t   B o x   T i t l e s	-	-	-	-	-	-	-	-	-	}
		TextFont(systemFont);															{Set the window's font to Chicago 12pt.				}
		TextSize(12);																			{																			}
		MoveTo(5, 19);																		{Draw the word 'Left' at local co-ordinates 5,19	}
		DrawString('Left:');																	{	for the left list box.											}
		MoveTo(253, 19);																	{Draw the word 'Right' at local co-ord 253,19		}
		DrawString('Right:');																{	for the right list box.										}

																										{ R a d i o   B u t t o n   G r o u p ' s   T i t l e	-	-	}
		PenNormal;																				{Reset pen pattern to black, and pen size to 1x1}
		GroupBox('Choose', 165, 212, 231, 273);								{Draw a box around the radio button group and	}
	end;																								{	give the group a title.										}


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   2 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  	}
{	This routine draws all the text and lines in demo window 2.  It is called when first creating the window and when	}
{	the window needs to be refreshed.  Note that objects that are created by Tools Plus are refreshed						}
{	automatically.																																							}
	procedure DrawWindow2Contents;
		var
			Rectangle: rect;																	{Rectangle for drawing comments (text)				}
	begin
																										{ C o m m e n t   a b o v e   f i e l d s	-	-	-	-		}
		TextFont(Geneva);																	{Draw all comments using Geneva 9pt.				}
		TextSize(9);																			{																			}
		PenPat(qd.gray);																	{Any line drawing will be done with a gray pattern}
		TextInBox(concat('Click the "zoom box" to zoom between the standard and user co-ordinates.', ReturnKey, ReturnKey, 'The scroll bars don''t actually scroll anything in this demo.  But notice that they are automatically repositioned when you resize the window.'), 10, 2, 191, 95, teJustLeft, false);

																										{ C o m m e n t   f o r   f i r s t   f i e l d	-	-	-	-	-	}
		TextInBox('The first field is "length limited."  It accepts a maximum of 30 characters, (the length of the field), and does not scroll or require word-wrap.', 220, 15, 375, 77, teJustLeft, false);
		SetRect(Rectangle, 215, 10, 380, 82);									{Define the outline around the comment				}
		FrameRect(Rectangle);															{Draw a gray box around the comment				}
		MoveTo(215, 46);																	{Draw a gray line from the first field to the box	}
		LineTo(195, 105);																	{	that was previously completed.						}

																										{ C o m m e n t   f o r   s e c o n d   f i e l d	-	-	-	}
		TextInBox('The second field is a "single-line" editing field.  It scrolls to keep the selection in view, and does not require word wrap.', 220, 97, 375, 146, teJustLeft, false);
		SetRect(Rectangle, 215, 92, 380, 151);									{Define the outline around the comment				}
		FrameRect(Rectangle);															{Draw a gray box around the comment				}
		MoveTo(215, 126);																	{Draw a gray line from the second field to the		}
		LineTo(195, 126);																	{	box that was previously completed.				}

																										{ C o m m e n t   f o r   t h i r d   f i e l d	-	-	-	-	-	}
		TextInBox('The third field is a "multiple-line" editing field.  It scrolls to keep the selection in view, and uses word wrap to break long words.', 220, 166, 375, 215, teJustLeft, false);
		SetRect(Rectangle, 215, 161, 380, 220);								{Define the outline around the comment				}
		FrameRect(Rectangle);															{Draw a gray box around the comment				}
		MoveTo(215, 207);																	{Draw a gray line from the third field to the box	}
		LineTo(195, 148);																	{	that was previously completed.						}

																										{ C o m m e n t s   a t   b o t t o m	-	-	-	-	-	-	}
		TextInBox('Try editing the text in the fields (above).  The Edit menu automatically works with the active field (check out the Undo!)', 10, 220, 200, 260, teJustLeft, false);
		TextInBox('Notice that each editing field can have its own font!', 220, 235, 375, 260, teJustLeft, false);
	end;


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   3 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine draws all the text and lines in demo window 3.  It is called when first creating the window and when	}
{	the window needs to be refreshed.  Note that objects that are created by Tools Plus are refreshed						}
{	automatically.																																							}
	procedure DrawWindow3Contents;
	begin
		TextFont(Geneva);																	{Comment at top of window (drawn in Geneva		}
		TextSize(9);																			{	9pt)…																}
		TextInBox('This demo simulates MIDI Interface settings.  Click on an icon to select it.', 10, 100, 230, 200, teJustCenter, false);
		DrawIcon(PrinterIcon, 20, 32, enabled, MidiPort = PrinterIcon);	{Redraw the printer and modem icons as	}
		DrawIcon(ModemIcon, 65, 32, enabled, MidiPort = ModemIcon);	{	currently selected.									}
		TextFont(systemFont);															{Box and title around icon group and radio			}
		TextSize(12);																			{	button group (drawin in Chicago 12 pt.)…		}
		GroupBox('Port', 7, 17, 111, 80);											{Draw a box around the Printer and Modem			}
																										{	icons, and give the group a title.						}
		GroupBox('Speed', 128, 17, 231, 80);									{Draw a box around the interface Speed radio	}
	end;																								{	buttons, and give the group a title.					}


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   4 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - -	}
{	This routine draws all the text in demo window 4.  It is called when first creating the window and when the			}
{	window	needs to be refreshed.																																}
	procedure DrawWindow4Contents;
	begin
		MoveTo(10, 21);																		{This window has been previously set to draw	}
		DrawString('A lengthy process is being simulated.');				{	characters using Chicago 12 pt.  Simply			}
		MoveTo(10, 85);																		{	display the message.										}
		DrawString(concat('Type ', chr(CommandMark), '-. to cancel the simulation.'));	{											}
	end;																																{											}


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   5 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - -	}
{	This routine draws all the text and pictures in demo window 5.  It is called when first creating the window and		}
{	when the window needs to be refreshed.  This routine has the following unique routines:										}
{		• Color-independent drawing (doesn't care about the monitor's settings)															}
{		• Color-dependent drawing (different drawing dependent on number of colors, grays, or if black & white)			}
{		• Multiple-monitor compatibility (each monitor can have different settings)														}
{	The color PICTure looks good only in 256 colors, but it also looks good in 256 shades of gray as well as 16		}
{	shades of gray (8 bit and 4 bit gray).  So this demo decides which PICTure to draw depending on the monitor's	}
{	settings.																																									}
{		NOTE:  Tools Plus may not recognize if you change your monitor settings while running your application in	}
{	the development environment.  It will recognize the change when your application is compiled to be a stand-		}
{	alone (double-clickable) application.																														}
	procedure DrawWindow5Contents;
		var
			TheScreen: integer;															{Screen counter for multiple-screen drawing		}
			hPicture: PicHandle;															{Handle to a PICTure resource							}
			viewRect: rect;																	{Viewing rectangle for the picture						}
	begin
																										{ C o l o r   I n d e p e n d e n t   D r a w i n g :	-	-	}
																										{Drawing that doesn't care about the monitor's	}
																										{	setting can be done before or after the			}
																										{	color-dependent drawing.								}
		TextInBox(concat('This window demonstrates color-dependent, color-independent, and multiple-screen drawing.  Use the Monitors desk accessory to change the monitor’s settings.', ReturnKey, '  If you have two monitors, drag the window across so that half is on each screen.'), 10, 5, 300, 70, teJustLeft, false);

																										{ C o l o r   D e p e n d e n t   D r a w i n g :		-	-	}
																										{The following code is for drawing that depends	}
																										{	on the number of available colors, shades		}
																										{	of gray, or if the monitor is set to Black &		}
																										{	White.  It also makes the routine multiple-		}
																										{	monitor compatible.											}
		for TheScreen := 1 to NumberOfScreens do							{Repeat drawing once for each logical screen	}
			begin																					{	(monitor with different settings)…					}
				BeginUpdateScreen(TheScreen);									{Begin the update for this logical screen.  All		}
																										{	drawing is limited (clipped) to the one logical	}
																										{	screen.															}
				if ((ScreenDepth >= 8) or ((ScreenDepth = 4) and (not ScreenHasColors))) then	{Use the color pictures}
					hPicture := GetPicture(ColorPICT)								{	only if 256 colors or 16 shades of gray are		}
				else																					{	available (or better).  Otherwise, use the		}
					hPicture := GetPicture(BlackAndWhitePICT);			{	Black & White equivalent.								}
				viewRect := hPicture^^.picFrame;									{Determine the picture's framing rectangle			}
																										{The picture's rectangle is in the local window	}
																										{	co-ordinates of the application that created	}
																										{	it, so convert it to the local co-ordinates of		}
				OffsetRect(viewRect, -viewRect.left, -viewRect.top);	{	this demo window.											}
																										{Offset the picture's rectangle such that it is		}
				OffsetRect(viewRect, 30, 90);										{	drawn below the descriptive text.					}
				DrawPicture(hPicture, viewRect);									{Draw the picture in its destination rectangle	.	}
				ReleaseResource(handle(hPicture));								{Release the resource to conserve memory		}
				EndUpdateScreen;															{End the update to the current logical screen		}
			end																						{																			}
	end;																								{																			}


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   6 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - 	}
{	Dynamic Alerts take care of all drawing, so you don't have to write any code for it.  This routine is here for			}
{	cosmetic reasons only, and you can get rid of it if you want.																					}
	procedure DrawWindow6Contents;
	begin
	end;


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   7 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  	}
{	This routine draws all the text and lines in demo window 7.  It is called when first creating the window and when	}
{	the window needs to be refreshed.																															}
	procedure DrawWindow7Contents;
		var
			Rectangle: rect;																	{Rectangle for drawing comments (text)				}
	begin
																										{ C o m m e n t s   b e s i d e   P o p - U p   M e n u s}
																										{All comments use Geneva 9pt, as set when		}
																										{	this window was first opened.							}
		PenPat(qd.gray);																	{Any line drawing will be done with a gray pattern}

																										{ C o m m e n t   f o r   f i r s t   m e n u	-	-	-	-	}
		TextInBox('This is a standard Pop-Up Menu.  Nothing special here, except that it’s really easy to do.', 235, 15, 380, 51, teJustLeft, false);
		SetRect(Rectangle, 230, 12, 385, 55);									{Define the outline around the comment				}
		FrameRect(Rectangle);															{Draw a gray box around the comment				}
		MoveTo(215, 26);																	{Draw a gray line from the first menu to the box	}
		LineTo(230, 26);																		{	completed above.											}

																										{ C o m m e n t   f o r   s e c o n d   m e n u	-	-	-	}
		TextInBox('This Pop-Up Menu has icons.  But unlike System-7’s pop-ups, the selected item’s icon can appear in the pop-up box.', 235, 65, 380, 113, teJustLeft, false);
		SetRect(Rectangle, 230, 62, 385, 117);									{Define the outline around the comment				}
		FrameRect(Rectangle);															{Draw a gray box around the comment				}
		MoveTo(220, 76);																	{Draw a gray line from the second menu to the	}
		LineTo(230, 76);																		{	box completed above.										}

																										{ C o m m e n t   f o r   t h i r d   m e n u	-	-	-	-	}
		TextInBox('This Pop-Up Menu contains icons without any text.  It looks like a custom menu but it’s not.', 235, 125, 380, 161, teJustLeft, false);
		SetRect(Rectangle, 230, 122, 385, 165);								{Define the outline around the comment				}
		FrameRect(Rectangle);															{Draw a gray box around the comment				}
		MoveTo(175, 136);																	{Draw a gray line from the third menu to the box	}
		LineTo(230, 136);																	{	completed above.											}

																										{ C o m m e n t   f o r   f o u r t h   m e n u	-	-	-	}
		TextInBox('This is a “Pop-Down” menu.  It’s like an on-screen pull-down menu, and is usually used to “do something now.”', 235, 185, 380, 233, teJustLeft, false);
		SetRect(Rectangle, 230, 182, 385, 237);								{Define the outline around the comment				}
		FrameRect(Rectangle);															{Draw a gray box around the comment				}
		MoveTo(220, 196);																	{Draw a gray line from the fourth menu to the		}
		LineTo(230, 196);																	{	box completed above.										}

																										{ C o m m e n t   f o r   f i f t h   m e n u	-	-	-	-	}
		TextInBox('Pop-Up Menus can use any font and size, and can be auto-sized for a perfect look.', 235, 245, 380, 281, teJustLeft, false);
		SetRect(Rectangle, 230, 242, 385, 285);								{Define the outline around the comment				}
		FrameRect(Rectangle);															{Draw a gray box around the comment				}
		MoveTo(150, 246);																	{Draw a gray line from the fifth menu to the box	}
		LineTo(230, 246);																	{	box completed above.										}
	end;																								{																			}


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   8 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - -   -	}
{	This routine draws all the text and lines in demo window 8.  Tools Plus takes care of all the button drawing.  The	}
{	band at the top of this window demonstrates how to make a tool bar in a window.													}
	procedure DrawWindow8Contents;
		var
			TheScreen: integer;															{Screen counter for multiple-screen drawing		}
			ToolBarRect: rect;																{Rectangle for drawing toolbar							}
	begin
																										{ D r a w   T o o l B a r   B a c k g r o u n d				}
		SetRect(ToolBarRect, -1, -1, 473, 29);									{Define the toolbar's rectangle							}
																										{Draw the toolbar gray if the monitor is set to		}
																										{	4-bits or better…												}
		for TheScreen := 1 to NumberOfScreens do							{Repeat drawing once for each logical screen	}
			begin																					{	(monitor with different settings)…					}
				PenColorNormal;																{PEN: 1 x 1 size, black color, patCopy mode		}
				BeginUpdateScreen(TheScreen);									{Begin the update for this logical screen.  All		}
																										{	drawing is limited (clipped) to the one logical	}
																										{	screen.															}
				if ScreenDepth >= 4 then													{If this logical screen is set to a depth of 4-bits	}
					begin																			{	or more (16 colors/grays)…							}
						RGBForeColor(ToolBarGray);									{																			}
						PaintRect(ToolBarRect);											{Paint the toolbar the medium dark gray				}
					end;																				{																			}
				EndUpdateScreen;															{End the update to the current logical screen		}
			end;																						{																			}
		PenColorNormal;																		{PEN: 1 x 1 size, black color, patCopy mode		}
		FrameRect(ToolBarRect);														{Draw a black 1-pixel frame around the toolbar	}

																										{ B u t t o n ' s   V a l u e s										}
		ActionInWindow8_DrawToolBarValue;									{Draw the value in the window's tool bar				}
		ActionInWindow8_DrawGlobeValue;										{Draw the globe button's value							}

		TextFont(Geneva);																	{All comments are drawn using Geneva font		}
		TextSize(12);																			{One comment is drawin in Geneva 9pt…			}
		MoveTo(6, 134);																		{																			}
		DrawString('Picture Buttons are very versatile.  You define their look and behavior.');	{									}
		PenPat(qd.gray);																	{Any line drawing will be done with a gray pattern}
		MoveTo(0, 118);																		{Draw gray line dividing upper and lower part of	}
		LineTo(472, 118);																	{	the window.														}
		TextSize(9);																			{All other comments use Geneva 9pt.					}

																										{ C o m m e n t   f o r   A r r o w   B u t t o n s	-		}
		TextInBox('These buttons use an icon for each stage, giving you absolute control over their appearance.', 10, 41, 155, 80, teJustLeft, false);
		MoveTo(120, 29);																	{Draw a gray line from this comment to the			}
		LineTo(120, 40);																		{	buttons it refers to.											}

																										{ C o m m e n t   f o r   A l i g n m e n t   B u t t o n s	}
		TextInBox('These “SICN 3D” buttons are available in two sizes.  They use a single icon each.  Tools Plus does all the 3D drawing.  Activate another window and see how they look disabled.', 167, 41, 358, 110, teJustLeft, false);
		MoveTo(218, 29);																	{Draw a gray line from this comment to the			}
		LineTo(218, 40);																		{	buttons it refers to.											}
		MoveTo(322, 29);																	{																			}
		LineTo(322, 40);																		{																			}

																										{ C o m m e n t   f o r   U t i l i t y   B u t t o n s	-	-	}
		TextInBox('This pair of buttons behaves as one control.  Continuous events are generated while the mouse button is held down.', 368, 41, 471, 118, teJustLeft, false);
		MoveTo(450, 29);																	{Draw a gray line from this comment to the			}
		LineTo(450, 40);																		{	buttons it refers to.											}

																										{ C o m m e n t   f o r   P l a i n   I c o n   B u t t o n s}
		TextInBox('Use picture buttons for clickable icons.', 19, 191, 111, 216, teJustLeft, false);	{							}

																										{ C o m m e n t   f o r   T a p e   D e c k   B u t t o n s}
		TextInBox('Professional looking controls are easy!', 20, 281, 117, 306, teJustLeft, false);	{							}

																										{ C o m m e n t   f o r   S c r o l l i n g   B u t t o n		}
		TextInBox('These buttons have “values” associated with them.  You can raise/lower the value by clicking on opposite sides of the button.  The left button uses only 1 icon!', 204, 154, 398, 203, teJustLeft, false);
		MoveTo(147, 158);																	{																			}
		DrawString('Step');																	{																			}
		MoveTo(172, 158);																	{																			}
		DrawString('Scroll');																{																			}

																										{ C o m m e n t   f o r   D u a l -  S t a g e   B u t t o n}
		TextInBox('These dual-stage buttons toggle when selected.', 221, 243, 342, 268, teJustLeft, false);	{		}
	end;


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   9 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -}
{	This routine draws the contents of demo window 9 (the Tool Bar).  In this demo, the only thing the Tool Bar			}
{	needs is to be painted with the correct color (gray when the monitor is set to 4 bits or higher).								}
	procedure DrawWindow9Contents;
		var
			ToolBarRect: rect;																{Rectangle for drawing toolbar							}
	begin
																										{ P a i n t   T o o l B a r   G r a y								}
		if ScreenDepth >= 4 then															{If the main minitor is set to a depth of 4-bits or	}
			begin																					{	more (16 colors/grays), paint the tool bar…	}
				RGBForeColor(ToolBarGray);											{Set the foregound color to gray							}
				SetRect(ToolBarRect, 0, 0, 32000, 32000);					{Define the toolbar's rectangle (who cares how	}
																										{	big it really is?  All drawing is limited to the		}
																										{	Tool Bar).															}
				PaintRect(ToolBarRect);													{Paint the tool bar the medium dark gray				}
			end																						{																			}
	end;																								{																			}


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   10 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This floating palette contains only Tools Plus objects, so there is no need to draw anything in this window.  This}
{	routine is here for cosmetic reasons only, and you can get rid of it if you want.														}
	procedure DrawWindow10Contents;
	begin
	end;


{ D r a w   C o n t e n t s   o f   D e m o   W i n d o w   11 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This floating palette contains only Tools Plus objects, so there is no need to draw anything in this window.  This}
{	routine is here for cosmetic reasons only, and you can get rid of it if you want.														}
	procedure DrawWindow11Contents;
	begin
	end;










{ R e f r e s h   A   W i n d o w : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine is called in response to a "doRefresh" event.  It redraws the contents of a window.  Note that			}
{	objects such as Scroll Bars, List Boxes, Buttons and Editing Fields are updated automatically.							}
{		You should account for each of your application's windows in a routine such as this one, even if they are		}
{	modal.  Modal windows may need updating if the Mac running your application is running a screen saver.  When}
{	the screen saver is woken up, chances are that it will demand that all windows be refreshed.								}
	procedure RefreshWindow;
	begin
		CurrentWindow(Poll.Window);													{Make the affected window the current grafPort.}
																										{	That means that all drawing operations will		}
																										{	be done on that window.									}
		BeginUpdate(WindowPointer(Poll.Window));							{Restrict drawing to within the area that needs	}
																										{	refreshing.  Any drawing done outside this		}
																										{	area is ignored.												}
		case Poll.Window of																{Depending on which window needs to be			}
			DemoWindow1:																	{	refreshed, call the appropriate routine to do	}
				DrawWindow1Contents;													{	the required drawing…									}
			DemoWindow2:																	{																			}
				DrawWindow2Contents;													{																			}
			DemoWindow3:																	{																			}
				DrawWindow3Contents;													{																			}
			DemoWindow4:																	{																			}
				DrawWindow4Contents;													{																			}
			DemoWindow5:																	{																			}
				DrawWindow5Contents;													{																			}
			DemoWindow6:																	{																			}
				DrawWindow6Contents;													{																			}
			DemoWindow7:																	{																			}
				DrawWindow7Contents;													{																			}
			DemoWindow8:																	{																			}
				DrawWindow8Contents;													{																			}
			ToolBarWindow:																	{																			}
				DrawWindow9Contents;													{																			}
			VerticalPalette:																	{																			}
				DrawWindow10Contents;												{																			}
			HorizontalPalette:																{																			}
				DrawWindow11Contents;												{																			}
		end;
		EndUpdate(WindowPointer(Poll.Window));								{End the update for the window, and tell the Mac}
																										{	you've finished updating this window (ie:		}
																										{	clear the window's update region).					}
		CurrentWindowReset;																{Reset the active window to also be the				}
																										{	current window (all drawing will resume on		}
	end;																								{	the active window).											}










{  C r e a t e   D e m o   W i n d o w s : =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =	}
{	The following routines are used to create each of the demo windows and their contents.  These routines are		}
{	invoked when the user selects an item in the Demos menu.  A couple of these windows are opened					}
{	automatically by the 'PrepareTheDemo' routine when this program starts up.														}


{ C r e a t e   D e m o   W i n d o w   1 :	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	}
{	This window demonstrates how Tools Plus works with  List Boxes, Buttons (radio buttons, check boxes and		}
{	push buttons), and Cursor Zones.																															}
{		When playing around with this window, try the following things:																			}
{			• double-click a line in a List Box and notice how it translates to 'select this line and OK'								}
{			• press the Enter or Return key to activate the default OK button																	}
{			• activate another window and notice all the buttons and list boxes get disabled.  Also notice that the			}
{			   black outline around the default OK button gets dimmed out too!																}
{	TIP:  To make this window appear to fill in as quickly as possible, we don't populate the list boxes until the end!	}
	procedure OpenDemoWindow1;
	begin
																										{Open the window (it automatically shifts down	}
																										{	to compensate for an open Tool Bar, and its	}
																										{	position is 'tiled')…											}
		WindowOpen(DemoWindow1, 0, 0, 425, 320, 'Lists, Buttons and Cursor Zones', noGrowDocProc + wTile, GoAway, NotModal);
																										{ Left List Box:	-	-	-	-	-	-	-	-	-	-	-	-	-	}
		TextFont(systemFont);															{Create an empty list box.  Its font will be the		}
		TextSize(12);																			{	window's current font: Chicago 12 pt.				}
		NewListBox(LeftList, 40, 8, 220, 104, lOnlyOne);					{																			}
																										{ Right List Box:	-	-	-	-	-	-	-	-	-	-	-	-	}
		TextFont(Geneva);																	{Create a second empty list box.  This one will	}
		TextSize(9);																			{	use Geneva 9pt to show that a different			}
		NewListBox(RightList, 295, 8, 400, 104, lOnlyOne);				{	font can be used per list box.							}
																										{ B u t t o n s :	-	-	-	-	-	-	-	-	-	-	-	-	-	}
		TextFace([bold]);																	{Buttons will be drawn in bold Geneva 9pt.			}
																										{Create a set of mini-fonts using Geneva 9pt in	}
																										{	bold typeface.  This demonstrates the use		}
																										{	of a second font for buttons.							}
		NewButton(MiniButton1, 20, 150, 35, 166, '1', pushButProc + useWFont, enabled, notSelected);	{				}
		NewButton(MiniButton2, 40, 150, 55, 166, '2', pushButProc + useWFont, enabled, notSelected);	{				}
		NewButton(MiniButton3, 60, 150, 75, 166, '3', pushButProc + useWFont, enabled, notSelected);	{				}
		NewButton(MiniButton4, 80, 150, 95, 166, '4', pushButProc + useWFont, enabled, notSelected);	{				}
		NewButton(MiniButton5, 100, 150, 115, 166, '5', pushButProc + useWFont, enabled, notSelected);	{			}
																										{Create a set of check boxes using the System	}
																										{	font (Chicago 12)…											}
		NewButton(CheckBox1, 20, 220, 117, 236, 'First Choice', checkBoxProc, enabled, notSelected);	{				}
		NewButton(CheckBox2, 20, 236, 133, 252, 'Second Choice', checkBoxProc, enabled, selected);	{				}
		NewButton(CheckBox3, 20, 252, 134, 268, 'Two And Three', checkBoxProc, enabled, selected);	{				}
																										{Create a set of radio buttons using the System	}
																										{	font (Chicago 12)…											}
		NewButton(RadioButton1, 170, 220, 214, 236, 'One', radioButProc, enabled, selected);				{					}
		NewButton(RadioButton2, 170, 236, 216, 252, 'Two', radioButProc, enabled, notSelected);		{					}
		NewButton(RadioButton3, 170, 252, 226, 268, 'Three', radioButProc, enabled, notSelected);		{					}
																										{Create the 'Cancel' push-button…						}
		NewButton(CancelButton, 275, 250, 335, 270, 'Cancel', pushButProc, enabled, notSelected);	{					}
																										{Create the 'OK' push-button.  This is the			}
																										{	window's default button, and it is selected		}
																										{	automatically when you press Return or			}
		NewButton(OkButton, 350, 250, 410, 270, 'OK', DefaultButton, enabled, notSelected);				{	Enter.		}

		DrawWindow1Contents;															{Draw the rest of the objects in this demo			}
																										{	window.  All Tools Plus objects are auto-		}
																										{	matically updated when a window needs to		}
																										{	be refreshed, such as when you bring it to		}
																										{	the front to make it active.								}
																										{		Non-Tools Plus objects (ie: those you		}
																										{	create) have to be redrawn separately in		}
																										{	response to a doRefresh event, so it's a		}
																										{	good idea to keep them in a separate routine.}

	{ T i m e   c o n s u m i n g   t h i n g s …																														}
	{	The following time-consuming things were saved for the end of the routine because it makes the window		}
	{	appear to be filled in faster, particularly on slower Macs like the Plus, Classic, or LC.  You don't have to			}
	{	follow the same concept when you write your own applications.																		}
																										{ C u r s o r   T a b l e   &   Z o n e s:	-	-	-	-	-	}
		NewCursorTable(1, arrowCursor);											{Create Cursor Table number 1.  The standard	}
																										{	Arrow cursor is the default for this table.		}
		CursorZone(1, 1, plusCursor, 275, 150, 400, 189);				{Add two Cursor Zones to the Cursor Table.		}
		CursorZone(1, 2, crossCursor, 275, 193, 400, 232);				{	When the cursor is in these zones, it will		}
																										{	automatically change to the specified			}
																										{	cursor shape.													}
		UseCursorTable(1);																	{The current window will use Cursor Table no 1.	}
																										{	Note that a cursor table exists						}
																										{	independently of windows.  In fact, several	}
																										{	windows can each use the same cursor			}
																										{	table.  It is not necessary to create a cursor	}
																										{	table each time you want a window to use it.	}
																										{	You just use the UseCursorTable routine to	}
																										{	make a window use a cursor table.					}
																										{ F i l l   L i s t   B o x e s :	-	-	-	-	-	-	-	-	-	}
		DrawListBox(LeftList, off);														{Turn list box drawing off for both boxes.  The	}
		DrawListBox(RightList, off);													{	contents will be drawn later.  If you don't		}
																										{	turn list box drawing off while adding lines,		}
																										{	you will see each line being added, one at a	}
																										{	time.  This way, you see all the lines appear	}
																										{	at once.															}
		SetListBoxText(LeftList, 1, 'Bill Arlington, Shipping & Receiving');{Fill the lines in the left list box.  You can fill}
		SetListBoxText(LeftList, 2, 'Gail Force, Social Director');		{	these lines in any order you want.  Notice		}
		SetListBoxText(LeftList, 3, 'Bill Gates, Comedy Relief');		{	the longer lines are compressed (the				}
		SetListBoxText(LeftList, 4, 'Ron Martin, Art & Graphics');		{	characters are closer together) when using	}
		SetListBoxText(LeftList, 5, 'Samantha Moore, Connectivity');	{	System 7.  Tools Plus automatically			}
		SetListBoxText(LeftList, 6, 'Mike Simms, Design');				{	appends the ellipsis (…) if the line's text is		}
		SetListBoxText(LeftList, 7, 'Sam Strong, Design');				{	too long to see in the list.								}
		SetListBoxText(LeftList, 8, 'Linda Tyler, Drafting');				{																			}
		SetListBoxText(LeftList, 9, 'Margaret Victor, Drafting');		{																			}
		SetListBoxText(RightList, 1, 'Art & Graphics');						{Fill in the lines in the right list box.  The Tools	}
		SetListBoxText(RightList, 2, 'Connectivity');							{	Plus User Manual describes how you can		}
		SetListBoxText(RightList, 3, 'Comedy Relief');						{	populate a list box and have the lines in			}
		SetListBoxText(RightList, 4, 'Design');									{	alphabetic order.												}
		SetListBoxText(RightList, 5, 'Drafting');									{																			}
		SetListBoxText(RightList, 6, 'Shipping & Receiving');			{																			}
		SetListBoxText(RightList, 7, 'Social Director');						{																			}
		SetListBoxLine(LeftList, 2, on);												{Select the left list's second line by default.		}
		DrawListBox(LeftList, on);														{Turn on line drawing for both list boxes. Their	}
		DrawListBox(RightList, on);													{	contents are drawn by this process.				}
		EnableMenu(FileMenu, CloseItem, enabled);							{Enable the File menu's Close command, now	}
	end;																								{	that there's a window to close.						}


{ C r e a t e   D e m o   W i n d o w   2 :	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	}
{	This window demonstrates how Tools Plus works with  Editing Fields, Window Zooming and Scroll Bars.  Please}
{	note that although this window has scroll bars, the demo doesn't actually scroll anything.  Notice that when you}
{	click the window's zoom box, or when you resize the window by using the size box, that the scroll bars are		}
{	automatically moved to accommodate the window's new size.																				}
{		When playing around with this window, try the following things:																			}
{			• click the window's zoom box to zoom between the standard co-ordinates and the user-defined ones		}
{			• use the size box to resize the window.  Notice that the minimum and maximum size are controlled			}
{			• type in the first (length limited) field																												}
{			• copy and paste test between another application or desk accessory and one of the fields						}
	procedure OpenDemoWindow2;
		var
			userRect, stdRect: rect;														{User co-ordinates and standard co-ordinates	}
																										{	used for zooming one of the demo windows.	}
	begin
																										{ C r e a t e   t h e   W i n d o w :	-	-	-	-	-	-	-	}
																										{Open the window (it automatically shifts down	}
																										{	to compensate for an open Tool Bar, and its	}
																										{	position is 'tiled')…											}
		WindowOpen(DemoWindow2, 20, 58, 420, 338, 'Fields, Zooming and Scroll Bars', documentProc + ZoomBox + wTile, GoAway, NotModal);
		GetWindowZoom(userRect, stdRect);									{Get standard and user-defined co-ordinates		}
																										{	for Zooming the window (using zoom box)		}
		SetRect(userRect, 250, 100, 470, 320);									{Define window's 'user- defined' co- ords that		}
																										{	are used for Zooming the window when the		}
																										{	zoom-box is clicked.										}
		SetWindowZoom(userRect, stdRect);									{Set the standard and user-defined co-ordinates}
																										{	for Zooming.													}
		SetWindowSizeLimits(220, 220, 400, 280);							{Set the window's sizing limits:							}
																										{	minHoriz, minVert, maxHoriz, maxVert			}
																										{ S c r o l l   B a r s :	-	-	-	-	-	-	-	-	-	-	-	}
																										{Create a right and bottom scroll bar for the		}
																										{	window.  The last three parameters specify	}
																										{	the min limit, current value, and max limit.		}
		NewScrollBar(RightBar, scrlRightEdge, scrlTopEdge, scrlRightEdge, scrlBottomEdge, enabled, 1, 5, 10);
		NewScrollBar(BottomBar, scrlLeftEdge, scrlBottomEdge, scrlRightEdge, scrlBottomEdge, enabled, 1, 50, 100);

																										{ E d i t i n g   F i e l d s :	-	-	-	-	-	-	-	-	-	}
		hField1^^ := 'Length limited field';											{Specify the text that will be appear in each field.}
		hField2^^ := 'Single line editing field.';										{																			}
		hField3^^ := 'This is a multiple-line editing field which incorporates word wrap.';{												}

		TextFont(Monaco);																	{First field uses Monaco 9pt font, and is length	}
		TextSize(9);																			{	limited.  That means that no more than 30		}
		FieldLengthLimit(on);																{	characters (the field's limit) can be typed or	}
		NewField(1, 10, 100, 191, 111, Handle(hField1), teBoxNoCR, teJustLeft);	{	pasted in the field.					}
		FieldLengthLimit(off);																{The remaining fields are not length limited			}

		TextFont(Geneva);																	{Second field uses bold Geneva 9pt.  This field	}
		TextFace([bold]);																	{	demonstrates a single-line editing field.			}
		NewField(2, 10, 120, 191, 132, Handle(hField2), teBoxNoCR, teJustLeft);	{													}

		TextFont(systemFont);															{Third field uses Chicago 12pt.  It demonstrates}
		TextSize(12);																			{	a multiple-line editing field with word wrap.		}
		TextFace([]);																			{																			}
		NewField(3, 10, 141, 191, 189, Handle(hField3), teBoxCR, teJustLeft);	{														}

		ActivateField(1, teSelectEnd);												{Activate the first field, with the insertion point	}
																										{	at the end of the text.										}
		DrawWindow2Contents;															{Draw the rest of the objects in this demo			}
																										{	window.  All Tools Plus objects are auto-		}
																										{	matically updated when a window needs to		}
																										{	be refreshed, such as when you bring it to		}
																										{	the front to make it active.								}
																										{		Non-Tools Plus objects (ie: those you		}
																										{	create) have to be redrawn separately in		}
																										{	response to a doRefresh event, so it's a		}
																										{	good idea to keep them in a separate routine.}

		EnableMenu(FileMenu, CloseItem, enabled);							{Enable the File menu's Close command, now	}
	end;																								{	that there's a window to close.						}


{ C r e a t e   D e m o   W i n d o w   3 :	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	}
{	This window demonstrates shows how Tools Plus can draw icon families (in System 6 and System 7).  It also	}
{	provides an example how to use Cursor Zones to make icons click-sensitive.														}
{		When playing around with this window, try the following things:																			}
{			• click either of the two icons																															}
{			• if you have a color monitor, use the Monitors control panel to change the number of colors displayed		}
{			• if you have two monitors, set one to black and white and drag the window such that the icon is half on		}
{			  a color screen and half on a black and white screen.  Tools Plus draws the icon perfectly.							}
	procedure OpenDemoWindow3;
	begin																							{Open the window (it automatically shifts down	}
																										{	to compensate for an open Tool Bar, and its	}
																										{	position is 'tiled')…											}
		WindowOpen(DemoWindow3, 0, 0, 240, 142, 'Icons and Cursor Zones', noGrowDocProc + wTile, GoAway, NotModal);

																										{ C u r s o r   T a b l e   &   Z o n e s:	-	-	-	-	-	}
		NewCursorTable(2, arrowCursor);											{Create Cursor Table number 2.  The standard	}
																										{	Arrow cursor is the default for this table.		}
		CursorZone(2, PrinterIcon, arrowCursor, 20, 32, 52, 64);		{Add two Cursor Zones to the Cursor Table.		}
		CursorZone(2, ModemIcon, arrowCursor, 65, 32, 97, 64);		{	The user can click these zones, thereby		}
																										{	making the Printer and Modem icons click		}
																										{	sensitive.														}
		UseCursorTable(2);																	{The current window will use Cursor Table no 2.	}
																										{	Note that a cursor table exists						}
																										{	independently of windows.  In fact, several	}
																										{	windows can each use the same cursor			}
																										{	table.  It is not necessary to create a cursor	}
																										{	table each time you want a window to use it.	}
																										{	You just use the UseCursorTable routine to	}
																										{	make a window use a cursor table.					}

																										{Create 3 radio buttons used to select the MIDI	}
																										{	interface's speed…											}
		NewButton(MidiHalfMeg, 142, 27, 216, 39, '  .5 MHz.', radioButProc, enabled, notSelected);		{					}
		NewButton(MidiOneMeg, 142, 43, 216, 55, '1.0 MHz.', radioButProc, enabled, notSelected);		{					}
		NewButton(MidiTwoMeg, 142, 58, 216, 72, '2.0 MHz.', radioButProc, enabled, notSelected);		{					}
		SelectButton(MidiSpeed, selected);										{Set the current speed button								}

		DrawWindow3Contents;															{Draw the rest of the objects in this demo			}
																										{	window.  All Tools Plus objects are auto-		}
																										{	matically updated when a window needs to		}
																										{	be refreshed, such as when you bring it to		}
																										{	the front to make it active.								}
																										{		Non-Tools Plus objects (ie: those you		}
																										{	create) have to be redrawn separately in		}
																										{	response to a doRefresh event, so it's a		}
																										{	good idea to keep them in a separate routine.}

		EnableMenu(FileMenu, CloseItem, enabled);							{Enable the File menu's Close command, now	}
	end;																								{	that there's a window to close.						}



{ C r e a t e   D e m o   W i n d o w   4 :	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	}
{	This demo shows off several Tools Plus features:																									}
{		• Cursor animation (the wristwatch cursor spins)																								}
{		• Filtering unwanted events when the application is busy (all typing and mouse clicks are filtered, so your		}
{			application only gets Command-. events when the user wants to halt a lengthy process).							}
{		• Secondary event loop in an application																											}
{		• Background processing (an ongoing process that runs all the time)																}
{		• Progress thermometer																																		}
	procedure OpenDemoWindow4;
		const
			ThermMax = 235;																	{Thermometer's maximum value							}
		var
			ThermometerRect: rect;														{Pregress thermometer's location						}
			ThermValue: integer;															{Themometer's current value								}
			NextThermometerTime: longint;											{Next time when thermometer is updated (ticks)	}
			Done: boolean;																	{Is this demo done? (either Command-. by user	}
																										{	or thermometer is at 100%)								}
	begin
		CursorShape(watchCursor);													{Set the cursor to a wrist-watch							}
																										{Open a window to tell the user about the			}
																										{	lengthy process (automatically centered)…	}
		WindowOpen(DemoWindow4, 0, 0, 265, 100, '', dBoxProc + wCenter, NoGoAway, Modal);	{							}
		TextFont(systemFont);															{The message is displayed in Chicago 12pt.		}
		TextSize(12);																			{																			}
		DrawWindow4Contents;															{Draw this window's message								}
		SetRect(ThermometerRect, 15, 42, 250, 55);							{Define the thermometer's co-ordinates				}
		ThermValue := 0;																		{Initialize the thermometer's value (0% done)		}
		NextThermometerTime := 0;													{Force thermometer to be updated right away		}
		Done := false;																			{This demo is not completed yet							}

																										{ L o c a l   E v e n t   L o o p									}
		while not Done do																	{Keep running until we're done…							}
			if PollSystem(Poll) then														{If an event has been detected…						}
				case Poll.What of															{Determine what kind of event has occurred…	}
					doRefresh:																	{A window needs to be refreshed…						}
						RefreshWindow;														{	redraw the specified window.  A screen			}
																										{	saver may cause this event, even in a			}
																										{	modal window like this one.								}
					doKeyDown:																{If the user typed a character.  All characters	}
						Done := true;															{	other than Command-. are filtered out, so		}
																										{	we know the user has typed Command-. to		}
																										{	halt the process.												}
					otherwise																		{All other events are ignored								}
				end																					{																			}
																										{ B a c k g r o u n d   p r o c e s s i n g …				}
			else if TickCount >= NextThermometerTime then				{No event available, so if it is time to do the next}
				begin																				{	thermometer update…									}
					NextThermometerTime := TickCount + 6;					{Next update will be in 6 ticks (10 times/second)}
					ThermValue := ThermValue + 1;									{Increment the thermometer's value					}
					DrawThermometer(ThermometerRect, ThermValue, ThermMax);	{Update the progress thermometer	}
					if ThermValue >= ThermMax then								{If the thermometer reads 100%…						}
						Done := true;															{	we're done with this demo window.					}
				end;																					{																			}

		CloseTheWindow(DemoWindow4);											{End this demo by closing its window					}
		ResetCursor;																			{Get rid of the watch cursor, and set it according}
	end;																								{	to its location and relation to active window.	}





{ C r e a t e   D e m o   W i n d o w   5 :	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	}
{	This demo shows how to write code that does color drawing that is dependent on the monitor's settings (ie: the	}
{	number of colors, number of shades of gray, or black and white).  With very little additional effort, your				}
{	application can also be made to be compatible with Macintoshes that have multiple-monitors.							}
	procedure OpenDemoWindow5;
	begin																							{Open the window (it automatically shifts down	}
																										{	to compensate for an open Tool Bar, and its	}
																										{	position is 'tiled')…											}
		WindowOpen(DemoWindow5, 0, 0, 310, 208, 'Color Drawing and Multiple Monitors', noGrowDocProc + wTile, GoAway, NotModal);
																										{Create the 'OK' push-button.  This is the			}
																										{	window's default button, and it is selected		}
																										{	automatically when you press Return or			}
		NewButton(OkButton, 125, 170, 185, 190, 'OK', DefaultButton, enabled, notSelected);				{	Enter.		}


		TextFont(Geneva);																	{All text is drawn in Geneva 9pt.							}
		TextSize(9);																			{																			}
		DrawWindow5Contents;															{Draw the rest of the objects in this demo			}
																										{	window.  All Tools Plus objects are auto-		}
																										{	matically updated when a window needs to		}
																										{	be refreshed, such as when you bring it to		}
																										{	the front to make it active.								}
																										{		Non-Tools Plus objects (ie: those you		}
																										{	create) have to be redrawn separately in		}
																										{	response to a doRefresh event, so it's a		}
																										{	good idea to keep them in a separate routine.}

		EnableMenu(FileMenu, CloseItem, enabled);							{Enable the File menu's Close command, now	}
	end;																								{	that there's a window to close.						}



{ C r e a t e   D e m o   W i n d o w   6 :	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	}
{	This demo shows off Tools Plus's Dynamic Alerts.  Dynamic Alerts are self contained (you don't have to do		}
{	anything other than call it and wait for the user to respond).  Breaking from tradition that was previously			}
{	established by the 'OpenDemoWindow…' routines, this demo does everything pertaining to displaying				}
{	Dynamic Alerts, detecting user action, and responding to events.																		}
	procedure OpenDemoWindow6;
		const
			ContinueAlert = 11500;														{Button definition for a "Continue" dynamic alert	}
			SilentContinueAlert = -11500;												{Button definition for a "Continue" dynamic alert	}
																										{	that doesn't beep the user.								}
			EndAlert = -33987;																{Button definition using three custom buttons	}
		var
			AltButton: integer;																{Alert button clicked by the user							}
	begin																							{These first 2 alerts ignore which button that		}
																										{	was clicked…													}
		AltButton := AlertBox(noteIcon, concat('This is an dynamic alert box.  Dynamic Alerts automatically adjust to accommodate the text your application provides them.  In fact, they’re smart enough to recognize “carriage returns” as “new line” commands.', ReturnKey, ReturnKey, 'Click “Continue” for more examples.'), ContinueAlert);
		AltButton := AlertBox(stopIcon, concat('Icons…', ReturnKey, ReturnKey, 'You can display any icon you want in a Dynamic alert, including color icons.', ReturnKey, ReturnKey, 'AlertBox calls Tools Plus’s DrawIcon routine, so it’s smart enough to pick the right icon for your monitor’s settings.'), SilentContinueAlert);
																										{This alert displays 2 buttons: Yes and No.  If	}
																										{	the user answers Yes, the demo continues…}
		if YesAltBut = AlertBox(noteIcon, 'Do you want to continue with more examples of Dynamic Alerts?', YesNoAlert) then
			begin																					{																			}
				AltButton := AlertBox(cautionIcon, 'Dynamic Alerts can even do unusual things, like alerts with no buttons (click in this window to get rid of it).', NoButtonAlert);
				AltButton := AlertBox(ModemIcon, concat('Save modem preferences before quitting?', ReturnKey, '(Just kidding!)'), NoYesCanAlert);
				AlertButtonName(7, 'Yup!');											{This Dynamic Alert shows how you can rename}
				AlertButtonName(8, 'Wow!');											{	buttons to customize your application.  The	}
				AlertButtonName(9, 'Gnarly');											{	available options make it seem like you have	}
				AltButton := AlertBox(NoIcon, 'So, are you impressed with Dynamic Alerts?', EndAlert);	{	thousands	}
			end																						{	of customized alerts in your application.		}
	end;																								{																			}


{ C r e a t e   D e m o   W i n d o w   7 :	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	}
{	This demo demonstrates how easy it is to create and work with Pop-Up Menus.  Tools Plus gives you features	}
{	and options ordinary Pop-Up Menus don’t have.  Also, Tools Plus's Pop-Up menu's work identically across all	}
{	systems, unlike ordinary Mac Pop-Up Menus that provide some features that are exclusive to System 7.			}
	procedure OpenDemoWindow7;
	begin																							{Open the window (it automatically shifts down	}
																										{	to compensate for an open Tool Bar, and its	}
																										{	position is 'tiled')…											}
		WindowOpen(DemoWindow7, 0, 0, 390, 296, 'Pop-Up Menus', noGrowDocProc + wTile, GoAway, NotModal);	

																										{ P o p - U p   M e n u s :	-	-	-	-	-	-	-	-	-	}
	{	Example 1…																			Create a standard (default) Pop-Up Menu.  It is	}
																										{	displayed in the System Font (Chicago 12pt)	}
																										{	and only allows selection of one item at a		}
																										{	time.																}
																										{    NOTE:  If you specify the bottom of the Pop-	}
																										{	Up Menu's rectangle to be the same as the	}
																										{	top, it is be created at exactly the font's			}
		NewPopUp(popMenu1, 110, 20, 209, 20, 'Day of Week:', popupDefaultType, enabled);	{	height.					}
		PopUpMenu(popMenu1, 1, enabled, 'Sunday');						{Populate the Pop-Up Menu with days of the		}
		PopUpMenu(popMenu1, 2, enabled, 'Monday');						{	week.  At this point, no item is selected yet.	}
		PopUpMenu(popMenu1, 3, enabled, 'Tuesday!•');					{		Notice that the third item (Tuesday) is		}
		PopUpMenu(popMenu1, 4, enabled, 'Wednesday');				{	marked with a bullet character (•).  Marking	}
		PopUpMenu(popMenu1, 5, enabled, 'Thursday');					{	any item 'selects' it, and automatically			}
		PopUpMenu(popMenu1, 6, enabled, 'Friday');						{	displays it in the menu's content area.			}
		PopUpMenu(popMenu1, 7, enabled, 'Saturday');					{																			}


	{	Example 2…																			Create a second Pop-Up Menu.  This one shows}
																										{	the selected item's icon, and hides the 'down	}
																										{	arrow'.																}
		NewPopUp(popMenu2, 110, 70, 216, 70, 'Search Here:', popupIconTitle + popupNoArrow, enabled);	{			}
		PopUpMenu(popMenu2, 1, enabled, 'Desktop^44');				{Populate the Pop-Up menu with a simulated		}
		PopUpMenu(popMenu2, 2, enabled, 'Hard Disk^45');				{	path starting from the Desktop to folders.		}
		PopUpMenu(popMenu2, 3, enabled, 'Tools Plus^46');			{		Each menu item has an associated icon		}
		PopUpMenu(popMenu2, 4, enabled, 'THINK C^47');				{	(its specifier follows the '^' symbol.)  Note		}
		PopUpMenu(popMenu2, 5, enabled, 'Libraries^47');				{	that the Menu Manager automatically adds	}
		PopUpMenu(popMenu2, 6, enabled, '#Includes^47');			{	256 to the specifier to calculate the icon's		}
																										{	ID.  Example:  ^44 + 256 = 300 'cicn' ID			}
		CheckPopUp(popMenu2, 1, on);												{Select the first item by placing a check mark	}
																										{	beside it.															}


	{	Example 3…																			Create a third Pop-Up Menu that shows off how	}
																										{	easily you can create picture menus.  This		}
																										{	one uses two icons and no text.						}
		NewPopUp(popMenu3, 110, 120, 170, 154, 'Serial Port:', popupIconTitle + popupNoArrow, enabled);		{		}
		PopUpMenu(popMenu3, 1, enabled, '^48');							{Populate the Pop-Up menu with the Printer and	}
		PopUpMenu(popMenu3, 2, enabled, '^49');							{	Modem icons.													}
		CheckPopUp(popMenu3, 1, on);												{Select the first item by placing a check mark	}
																										{	beside it.															}


	{	Example 4…																			Create a fourth Pop-Up Menu that is a special	}
																										{	type of control.  This one has a fixed title,		}
																										{	and is used like an on-window pull-down			}
																										{	menu.																}
		NewPopUp(popMenu4, 110, 190, 216, 190, 'Format', popupFixedTitle, enabled);		{										}
		PopUpMenu(popMenu4, 1, enabled, 'Clear');							{Populate the menu with the selections that can}
		PopUpMenu(popMenu4, 2, enabled, 'Paragraph…');				{	be made.  Even though you can put check		}
		PopUpMenu(popMenu4, 3, enabled, 'Character…');				{	marks or other symbols beside these items,	}
		PopUpMenu(popMenu4, 4, enabled, 'Style…');						{	this demo makes them perform an action		}
																										{	just like pull-down menus.								}


	{	Example 5…																			Mini Pop-Up Menu…											}
		TextFont(Geneva);																	{Set the window's font to Geneva 9pt.  The pop-	}
		TextSize(9);																			{	up menu will use these settings.						}
																										{Create the last menu using a different (non-		}
																										{	System) font.  This feature is available on		}
																										{	all systems (unlike ordinary pop-up menus).	}
																										{	Never dim the menu's title or the selected		}
																										{	item because Geneva 9pt is unreadable			}
																										{	when its dithered on a 1-bit monitor (black		}
																										{	and white) monitor…										}
		NewPopUp(popMenu5, 110, 240, 143, 240, 'Size:', popupUseWFont + popupNoArrow + popupNeverDimTitle + popupNeverDimSelection, enabled);
		PopUpMenu(popMenu5, 1, enabled, '9');								{Populate the menu with a fictitious set of			}
		PopUpMenu(popMenu5, 2, enabled, '10');								{	available font sizes.  Once the menu is			}
		PopUpMenu(popMenu5, 3, enabled, '12!•');							{	created, you can change the window's			}
		PopUpMenu(popMenu5, 4, enabled, '14');								{	font and size without affecting the pop-up		}
		PopUpMenu(popMenu5, 5, enabled, '18');								{	menus.															}
		PopUpMenu(popMenu5, 6, enabled, '24');								{		The third item (12pt) is marked with a			}
		PopUpMenu(popMenu5, 7, enabled, '36');								{	bullet (•) to select that item.							}

		DrawWindow7Contents;															{Draw the rest of the objects in this demo			}
																										{	window.  All Tools Plus objects are auto-		}
																										{	matically updated when a window needs to		}
																										{	be refreshed, such as when you bring it to		}
																										{	the front to make it active.								}
																										{		Non-Tools Plus objects (ie: those you		}
																										{	create) have to be redrawn separately in		}
																										{	response to a doRefresh event, so it's a		}
																										{	good idea to keep them in a separate				}
																										{	routine.															}

		EnableMenu(FileMenu, CloseItem, enabled);							{Enable the File menu's Close command, now	}
	end;																								{	that there's a window to close.						}



{ C r e a t e   D e m o   W i n d o w   8 :	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	}
{	This demo shows off Tools Plus's Picture Buttons.  A variety of buttons have been created to give you a taste	}
{	of their abilities.  The top of this window includes a "window tool bar", a tool bar that is associated with a				}
{	specific window, such as the kind seen in popular word processors.																		}
{	  Picture Buttons are more versatile than most custom controls because you design their appearance and		}
{	their behavior.  It's almost like having a button-building kit at your disposal.															}
	procedure OpenDemoWindow8;
		const																						{Define constants for the various bahavior and	}
																										{	appearance specifications…							}
																										{Big 3D toolbar buttons stay down when				}
																										{	selected, lightened when selected, and			}
																										{	get whited out when disabled…						}
			BigToolBarSpec = picbutLockSelected + picbutBigSICN3D + picbutSelectLightenSICN3D + picbutDimUsingWhite;
																										{Small 3D toolbar buttons pop back up after		}
																										{	being selected, lightened when selected,		}
																										{	and get whited out when disabled…				}
			SmallToolBarSpec = picbutSelectLightenSICN3D + picbutDimUsingWhite;	{												}
																										{The Plus/Minus button pair produce repeated	}
																										{	doPictButton events while they are held			}
																										{	down, have an alternate image when				}
																										{	selected, and don't look different when			}
																										{	disabled…														}
			PlusMinusSpec = picbutRepeatEvents + picbutSelectAltImage + picbutDimNoChange;	{							}
																										{The ordinary icons (printer and modem) are		}
																										{	selected when the mouse-down occurs,			}
																										{	stay down when selected, darken when			}
																										{	selected.  When disabled, use a white			}
																										{	screen effect and preserve the border…		}
			OrdinaryIconSpec = picbutInstantEvent + picbutLockSelected + picbutSelectDarken + picbutDimUsingWhite + picbutDimLeaveBorder;
																										{These tapedeck control buttons are selected	}
																										{	when the mouse-down occurs, stay down		}
																										{	when selected and use an alternate icon.		}
																										{	They look the same disabled…						}
			TapedeckLockSpec = picbutInstantEvent + picbutLockSelected + picbutSelectAltImage + picbutDimNoChange;
																										{These tape deck controls to not lock down, and}
			TapedeckSpec = picbutSelectAltImage + picbutDimNoChange;{	they look the same disabled.					}
	begin
																										{Open the window (it automatically shifts down	}
																										{	to compensate for an open Tool Bar, and its	}
																										{	position is 'tiled')…											}
		WindowOpen(DemoWindow8, 0, 0, 472, 315, 'Picture Buttons', noGrowDocProc + wTile, GoAway, NotModal);
		ToolBarValue := 100;																{Initialize the tool bar's value								}

		DrawWindow8Contents;															{Draw all the objects in this demo window except}
																										{	for the buttons.  This is done first to create	}
																										{	a toolbar on which some buttons can be			}
																										{	placed.  All Tools Plus objects are auto-			}
																										{	matically updated when a window needs to		}
																										{	be refreshed, such as when you bring it to		}
																										{	the front to make it active.								}
																										{		Non-Tools Plus objects (ie: those you		}
																										{	create) have to be redrawn separately in		}
																										{	response to a doRefresh event, so it's a		}
																										{	good idea to keep them in a separate				}
																										{	routine.															}

																										{Create a pop-up menu in the title bar menu		}
																										{	using the window's font (Geneva 9).…			}
		if HasColorQuickDraw then														{If Color Quickdraw is available…						}
			RGBForeColor(ToolBarGray);												{	set foreground color to match the toolbar.		}
																										{	The Pop-Up Menu picks this up to remember	}
																										{	which color is behind the Pop-Up Menu.			}
		NewPopUp(popMenu1, 9, 7, 68, 7, '', popupUseWFont + popupNoArrow + popupNeverDimOutline + popupNeverDimSelection, enabled);
		PenColorNormal;																		{Restore to default colors									}
		PopUpMenu(popMenu1, 1, enabled, 'Normal!√');					{Populate the pop-up menu with some items.		}
		PopUpMenu(popMenu1, 2, disabled, '-');								{	Demo 7 gives you details about how pop-up	}
		PopUpMenu(popMenu1, 3, enabled, 'Promote');					{	menus are created.											}
		PopUpMenu(popMenu1, 4, enabled, 'Demote');						{																			}
		PopUpMenu(popMenu1, 5, enabled, 'Lateral');						{																			}

																										{Create a pop-up menu to control the 'globe'		}
																										{	button's 'acceleration curve'…						}
		NewPopUp(ButtonSpeedMenu, 406, 219, 460, 219, '∆Rate:', popupUseWFont + popupNoArrow, enabled);	
		PopUpMenu(ButtonSpeedMenu, 1, enabled, 'Linear');			{Populate the pop-up menu with the choices…	}
		PopUpMenu(ButtonSpeedMenu, 2, enabled, 'Slow');				{																			}
		PopUpMenu(ButtonSpeedMenu, 3, enabled, 'Medium!√');		{																			}
		PopUpMenu(ButtonSpeedMenu, 4, enabled, 'Fast');				{																			}



																										{C r e a t e   a l l   p i c t u r e   b u t t o n s …			}
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
		SetPictButtonSpeed(PlusButton, 2);										{Set the button speed to 2 steps per second for	}
		SetPictButtonSpeed(MinusButton, 2);									{	these two buttons.											}

		NewPictButton(PrinterButton, 28, 153, PrinterIcon, OrdinaryIconSpec, enabled, selected, 0, 0, 0);			{	}
		NewPictButton(ModemButton, 69, 153, ModemIcon, OrdinaryIconSpec, enabled, notSelected, 0, 0, 0);	{	}

	{This stepping button has nine icon images in its SICN resource.  The button's value changes automatically		}
	{	with the top 1/2 of the button incrementing the value and the bottom 1/2 decrementing it (one step each time	}
	{	the button is clicked).  This is a BID 3D button that looks 'pushed' when selected (not darker or ligher)…		}
		NewPictButton(SteppingButton, 147, 161, ScrollingIcon, picbutAutoValueChg + picbutTopBottomSplit + picbutBigSICN3D + picbutSelectPushedSICN3D, enabled, notSelected, 1, 3, 9);

	{This scrolling button has nine icon images in its SICN resource.  It keeps producing doPictButton events while}
	{	the mouse is held down.  The button's value changes automatically with the top 1/2 of the button					}
	{	incrementing the value and the bottom 1/2 decrementing it.  This is a BID 3D button that looks 'pushed' when}
	{	selected (not darker or ligher)…																															}
		NewPictButton(ScrollingButton, 170, 161, ScrollingIcon, picbutRepeatEvents + picbutAutoValueChg + picbutTopBottomSplit + picbutBigSICN3D + picbutSelectPushedSICN3D, enabled, notSelected, 1, 5, 9);

	{This is just about the most complicated button you can create.  It keeps producing doPictButton events while	}
	{	the mouse is held down.  The button's value changes automatically with the right 1/2 of the button					}
	{	incrementing the value and the left decrementing it.  The rate of change accelerates (at a medium rate) over	}
	{	time, and the button's values "wrap" (maximum + 1 = start at minimum).  An alternate icon is used when the	}
	{	button is selected.  A white screen is applied to this icon when the button is disabled.										}
		NewPictButton(GlobeButton, 410, 153, GlobeIcon, picbutRepeatEvents + picbutAutoValueChg + picbutScaleMedAccel + picbutValueWrap + picbutLeftRightSplit + picbutSelectAltImage + picbutDimUsingWhite, enabled, notSelected, -179, 0, 180);

																										{Tapedeck buttons always have 1 selected…	}
		NewPictButton(RewindButton, 17, 235, RewindIcon, TapedeckLockSpec, enabled, notSelected, 0, 0, 0);	{	}
		NewPictButton(StopButton, 48, 235, StopTapeIcon, TapedeckSpec, enabled, notSelected, 0, 0, 0);			{	}
		NewPictButton(FastFwdButton, 79, 235, FastFwdIcon, TapedeckLockSpec, enabled, notSelected, 0, 0, 0);	
		NewPictButton(PauseButton, 17, 255, PauseIcon, TapedeckSpec, enabled, notSelected, 0, 0, 0);			{	}
		NewPictButton(RecordButton, 48, 255, RecordIcon, TapedeckSpec, enabled, notSelected, 0, 0, 0);			{	}
		NewPictButton(PlayButton, 79, 255, PlayIcon, TapedeckLockSpec, enabled, notSelected, 0, 0, 0);			{	}

	{This is a dual state button (Power-On, Power-Off), so it is considered to be a "Multi-Stage" button (Off=0,		}
	{	1=On).  The button's value changes automatically when selected by the user.  The values 'wrap' to allow 1	}
	{	(on) to start back at 0 (off) again.  An alternate icon is used to depict the selected button (down position),		}
	{	and disabled button for maximum visual control…																								}
		NewPictButton(PowerButton, 173, 235, PowerIcon, picbutMultiStage + picbutAutoValueChg + picbutValueWrap + picbutSelectAltImage + picbutDimAltImage, enabled, notSelected, 0, 0, 1);

	{This is a dual state button that uses a set of PICTs. Like the button above, it is Multi-State (left=0, right=1)		}
	{	and its value changes automatically when selected by the user.  This button uses different PICTs for the		}
	{	selected state, and for the screen depth (8-bit, 4-bit, and B&W).  It also uses a mask, even though it is not	}
	{	necessary on a white background.																														}
		NewPictButton(FlipButton, 163, 277, FlipSwitchPICT, picbutAutoValueChg + picbutLeftRightSplit + picbutMultiStage + picbutUsePICTS + picbutGray4use8 + picbutSelectAltImage + picbutDimNoChange, enabled, notSelected, 0, 0, 1);
																										{This is a really simple push-button that uses an}
																										{	alternate icon when the button is selected.	}
																										{	When disabled, a white screen is overlayed	}
																										{	and the border is preserved.							}
		NewPictButton(DoneButton, 392, 280, DoneIcon, picbutSelectAltImage + picbutDimUsingWhite + picbutDimLeaveBorder, enabled, notSelected, 0, 0, 0);

		EnableMenu(FileMenu, CloseItem, enabled);							{Enable the File menu's Close command, now	}
	end;																								{	that there's a window to close.						}


{ C r e a t e   D e m o   W i n d o w   9 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -}
{	In Tools Plus, creating a Tool Bar is as easy as opening a window.  Tools Plus makes sure the Tools Bar is		}
{	always at the front, and always active.  This Tool Bar contains a Pop-Up Menu and several Picture Buttons		}
{	(they are simply a duplicate of the 'window tool bar' seen in demo 8.																		}
{	  Tools Plus automatically hides the Tool Bar when your application is suspended, and displays it again when	}
{	your application resumes (when running under MultiFinder™ or System 7).															}
{	  This routine is used to both open and close the application's Tool Bar.  Note that all open windows can			}
{	optionally be shifted down when a Tool Bar is opened to make sure that no windows are hidden behind the Tool	}
{	Bar.  You can also tell Tools Plus to offset future windows downward to account for the Tool Bar's space on the}
{	monitor.																																									}
	procedure OpenDemoWindow9;
		const																						{These constants describe the behavior and		}
																										{	look of picture buttons in the Tool Bar.			}
																										{	Demo 8 provides more detail about picture		}
																										{	buttons. All the buttons here have been			}
																										{	duplicated from Demo 8…								}
			BigToolBarSpec = picbutLockSelected + picbutBigSICN3D + picbutSelectLightenSICN3D + picbutDimUsingWhite;
			SmallToolBarSpec = picbutSelectLightenSICN3D + picbutDimUsingWhite;	{												}
	begin
																										{ O p e n   T o o l   B a r …										}
		if not WindowIsOpen(ToolBarWindow) then							{If the Tool Bar is not open…								}
			begin																					{																			}
				ToolBarOpen(ToolBarWindow, 26, tbShiftWindows + tbOffsetNewWindows);{Open a Tool Bar that is 26	}
																										{	pixels high.  Shift all open windows down as	}
																										{	the Tool Bar opens, and as future windows		}
																										{	are opened, offset them downward to				}
																										{	compensate for the open Tool Bar.					}

				RenameItem(DemosMenu, 10, 'Hide Tool Bar');				{Rename the menu item										}

				DrawWindow9Contents;													{Paint the Tool Bar gray.  Picture buttons will be	}
																										{	placed on top of the gray.  All Tools Plus		}
																										{	objects are automatically updated when a		}
																										{	window needs to be refreshed.						}
																										{		Non-Tools Plus objects (ie: those you		}
																										{	create) have to be redrawn separately in		}
																										{	response to a doRefresh event, so it's a		}
																										{	good idea to keep them in a separate				}
																										{	routine.															}
				if HasColorQuickDraw then												{If Color Quickdraw is available…						}
					RGBForeColor(ToolBarGray);										{	set foreground color to match the toolbar.		}
																										{	The Pop-Up Menu picks this up to remember	}
																										{	which color is behind the Pop-Up Menu.			}
				TextFont(Geneva);															{The pop-up menu will use Geneva 9pt				}
				TextSize(9);																	{																			}
																										{Create the pop-up menu…									}
				NewPopUp(popMenu1, 9, 7, 68, 7, '', popupUseWFont + popupNoArrow + popupNeverDimOutline + popupNeverDimSelection, enabled);
				PenColorNormal;																{Restore to default colors									}
				PopUpMenu(popMenu1, 1, enabled, 'Normal!√');			{Populate the pop-up menu with some items.		}
				PopUpMenu(popMenu1, 2, enabled, 'Section');				{	Demo 7 gives you details about how pop-up	}
				PopUpMenu(popMenu1, 3, enabled, 'Bullet');					{	menus are created.											}
				PopUpMenu(popMenu1, 4, enabled, 'Table');					{																			}

																										{C r e a t e   a l l   p i c t u r e   b u t t o n s …			}
																										{	These are the same picture buttons you saw	}
																										{	described in detail in Demo 8…						}
				NewPictButton(LeftArrowButton, 103, 2, LeftArrowIcon, picbutSelectAltImage + picbutDimNoChange, enabled, notSelected, 0, 0, 0);
				NewPictButton(RightArrowButton, 126, 2, RightArrowIcon, picbutSelectAltImage + picbutDimNoChange, enabled, notSelected, 0, 0, 0);
				NewPictButton(LeftAlignButton, 172, 2, LeftAlignIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
				NewPictButton(CenterAlignButton, 195, 2, CenterAlignIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
				NewPictButton(RightAlignButton, 218, 2, RightAlignIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
				NewPictButton(JustifyButton, 241, 2, JustifyIcon, BigToolBarSpec, enabled, notSelected, 0, 0, 0);
				NewPictButton(CutButton, 288, 3, CutIcon, SmallToolBarSpec, enabled, notSelected, 0, 0, 0);
				NewPictButton(BucketButton, 311, 3, BucketIcon, SmallToolBarSpec, enabled, notSelected, 0, 0, 0);
				NewPictButton(ClipboardButton, 334, 3, ClipboardIcon, SmallToolBarSpec, enabled, notSelected, 0, 0, 0);
			end


		else																							{ C l o s e   T o o l   B a r …									}
			begin																					{																			}
				WindowClose(ToolBarWindow);										{Close the Tool Bar's window (all open windows	}
																										{	automatically move up.									}
				RenameItem(DemosMenu, 10, 'Show Tool Bar');			{Rename the menu item										}
			end																						{																			}
	end;																								{																			}


{ C r e a t e   D e m o   W i n d o w   10 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -		}
{	In Tools Plus, creating a floating palette (windoid) is as easy as opening a window.  Tools Plus makes sure the	}
{	palette if opened behind the Tool Bar (if there is one) and in front of standard windows (if there are any).			}
{	Palettes are always active.																																		}
{	  Tools Plus automatically hides palettes when your application is suspended, and displays them when your	}
{	application resumes (when running under MultiFinder™ or System 7).																	}
	procedure OpenDemoWindow10;
		var
			theButton: integer;																{Button number counter										}
			theIcon: integer;																	{Icon number counter											}
			thePoint: point;																	{Top left corner for button									}

		const																						{All picture buttons on this floating palette are	}
																										{	small SICN 3D icons (colorized by Tools			}
																										{	Plus).  They respond instantly when clicked.	}
			SmallButtonSpec = picbutInstantEvent + picbutSelectDarkenSICN3D;	{													}
	begin
																										{ O p e n   V e r t i c a l   P a l e t t e …					}
																										{Open the floating palette (it automatically			}
																										{	shifts down to compensate for an open Tool	}
																										{	Bar)…																}
		WindowOpen(VerticalPalette, 450, 40, 495, 229, '', paletteProc, GoAway, NotModal);	{								}

																										{C r e a t e   a l l   p i c t u r e   b u t t o n s …			}
																										{	Use some basic math to create the picture		}
																										{	buttons for this palette.  All the picture			}
																										{	buttons are buttons are sequentially				}
																										{	numbered, and so is their placement on this	}
																										{	palette.															}
		theIcon := FirstVPaletteIcon;													{Start with using the first icon in this palette		}
		for theButton := VPaletteMinButton to VPaletteMaxButton do	{Cycle through all buttons in this palette…		}
			begin																						{																		}
				thePoint.h := ord(not odd(theButton)) * 23 - 1;				{Horizontally, odd numbered buttons are at 0		}
																										{	and even numbered buttons are at 23 pixels.	}
				thePoint.v := ((theButton - 1) div 2) * 19 - 1;					{Two buttons fit side-by side horizontally, so		}
																										{	after each two buttons, shift down 19 pixels.	}
																										{Create the picture button…								}
				NewPictButton(theButton, thePoint.h, thePoint.v, theIcon, SmallButtonSpec, enabled, notSelected, 0, 0, 0);
				theIcon := theIcon + 1;													{Increment the icon for the next picture button	}
			end																						{																			}
	end;																								{																			}


{ C r e a t e   D e m o   W i n d o w   11 : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -		}
{	In Tools Plus, creating a floating palette (windoid) is as easy as opening a window.  Tools Plus makes sure the	}
{	palette if opened behind the Tool Bar (if there is one) and in front of standard windows (if there are any).			}
{	Palettes are always active.																																		}
{	  Tools Plus automatically hides palettes when your application is suspended, and displays them when your	}
{	application resumes (when running under MultiFinder™ or System 7).																	}
	procedure OpenDemoWindow11;
		var
			theButton: integer;																{Button number counter										}
			theIcon: integer;																	{Icon number counter											}
			thePoint: point;																	{Top left corner for button									}
			ButtonsPerRow: integer;														{Number of buttons per row on this palette			}
			ButtonInRow: integer;															{Button number in current row (1 to x)					}

		const																						{All picture buttons on this floating palette			}
																										{	respond instantly when clicked.  The image	}
																										{	darkens when selected, is dithered when		}
																										{	disabled (with it border left unaffected)…		}
			SmallButtonSpec = picbutInstantEvent + picbutSelectDarken + picbutDimUsingWhite + picbutDimLeaveBorder;
	begin
																										{ O p e n   H o r i z o n t a l   P a l e t t e …				}
																										{Open the floating palette (it automatically			}
																										{	shifts down to compensate for an open Tool	}
																										{	Bar)…																}
		WindowOpen(HorizontalPalette, 239, 260, 508, 299, '', altPaletteProc, GoAway, NotModal);	{						}

																										{C r e a t e   a l l   p i c t u r e   b u t t o n s …			}
																										{	Use some basic math to create the picture		}
																										{	buttons for this palette.  All the picture			}
																										{	buttons are buttons are sequentially				}
																										{	numbered, and so is their placement on this	}
																										{	palette.  Double borders between groups of	}
																										{	buttons are created by placing the buttons	}
																										{	further apart.													}
		theIcon := FirstHPaletteIcon;													{Start with using the first icon in this palette		}
		for theButton := HPaletteMinButton to HPaletteMaxButton do	{Cycle through all buttons in this palette…		}
			begin																						{																		}
																										{Determine number of picture buttons per row	}
				ButtonsPerRow := (HPaletteMaxButton - HPaletteMinButton + 1) div 2;{	(with two rows on this palette).}
				if theButton <= ButtonsPerRow then								{Calculate the button's relative number in this	}
					ButtonInRow := theButton											{	row (i.e. the first button in this row, second,	}
				else																					{	third, etc.)  This number is used to calculate	}
					ButtonInRow := theButton - ButtonsPerRow;				{	horizontal button spacing.								}
																										{Horizontally, each button is 24 pixels apart		}
																										{	from the previous one.  There are borders at	}
																										{	the end of the 4th, 9th and 10th button, so		}
																										{	add an extra 2 pixels between those				}
																										{	buttons…														}
				thePoint.h := (ButtonInRow - 1) * 24 + (2 * ord(ButtonInRow > 4)) + (2 * ord(ButtonInRow > 9)) + (2 * ord(ButtonInRow > 10)) - 1;
				thePoint.v := ord(theButton > ButtonsPerRow) * 20 - 1;	{Half of the buttons fit on the top row, and the	}
																										{	second half of them fit on the bottom row.		}
																										{	The bottom row is 20 pixels lower than the		}
																										{	first.																	}
																										{Create the picture button…								}
				NewPictButton(theButton, thePoint.h, thePoint.v, theIcon, SmallButtonSpec, enabled, notSelected, 0, 0, 0);
				theIcon := theIcon + 1;													{Increment the icon for the next picture button	}
			end																						{																			}
	end;																								{																			}










{  R e s p o n s e s   t o   E v e n t s : =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =		}
{	The following routines are invoked in response to events that were detected in the main event loop.					}


{ U s e r   C l i c k e d   I n a c t i v e   W i n d o w : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - 	}
{	This routine is called in response to a "doChgWindow" event.  Note that clicking between the active window		}
{	and a desk accessory, or between two desk accessories does not generate a doChgWindow event.					}
{		A doChgWindow event is generated only when the user tries to activate an inactive window belonging to		}
{	your application.  In a full featured application, you may want to validate the active editing field and perform		}
{	any other verifications before activating the window.																								}
	procedure ChangeWindow;
	begin
		ActivateWindow(Poll.Window);												{Activate the requested window							}
	end;


{ C h o o s e   A   M e n u : -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -	}
{	This routine is called in response to a "doMenu" event.  A doMenu event indicates that the user selected a		}
{	menu or typed a Command-key equivalent for a menu.  Note that a doMenu event is not generated if a desk		}
{	accessory's menu is selected.  The Edit menu's Undo, Cut, Copy, Paste, and Clear items automatically			}
{	interact with the active editing field without you having to do anything.																	}
	procedure MenuSelection;
		var
			theButton: integer;																{Button that was clicked in a Dynamic Alert		}
	begin
		case Poll.Menu.Num of
			ApplMenu:																			{ A p p l e  M e n u :	-	-	-	-	-	-	-	-	-	-	-	}
																										{The only item in the Apple menu that generates}
																										{	an event is the "About…" item.  All we do is	}
																										{	display a Dynamic Alert box…						}
				theButton := AlertBox(NoIcon, concat('T o o l s   P l u s   D e m o   A p p l i c a t i o n', ReturnKey, ReturnKey, 'Water’s Edge Software', ReturnKey, 'PO Box 70022', ReturnKey, '2441 Lakeshore Road West', ReturnKey, 'Oakville, Ontario', ReturnKey, 'Canada, L6L 6M9', ReturnKey, ReturnKey, '(416) 219-5628', ReturnKey, ReturnKey, '(Click this window to continue)'), NoButtonAlert);


			FileMenu:																				{ F i l e   M e n u :	-	-	-	-	-	-	-	-	-	-	-	-	}
				case Poll.Menu.Item of													{Determine the item selected within this menu…}
					CloseItem:																	{ Close…																}
						if FirstStdWindowNumber <> 0 then						{If a standard window is open…							}
							CloseTheWindow(FirstStdWindowNumber)			{	close the front most standard window.			}
						else if FirstPaletteNumber <> 0 then						{If a flating palette is open…								}
							CloseTheWindow(FirstPaletteNumber);				{	close the front most palette.							}

					QuitItem:																		{ Quit…																}
						ExitTheDemo := true;												{	set the 'exit the demo' flag to true to indicate	}
				end;																					{	we're ready to return to the Finder.					}


			DemosMenu:																		{ D e m o s   M e n u :	-	-	-	-	-	-	-	-	-	-	}
				begin																				{When selecting an item from the Demos menu,	}
																										{	it means 'activate this demo window' if the		}
																										{	window is already open, and 'open this demo	}
																										{	window' if it isn't open.										}

					if Poll.Menu.Item = ToolBarItem then							{If user selected the Hide/Show Tool Bar item…	}
						OpenDemoWindow9												{	Open or closes (hide) the Tool Bar					}
					else																				{																			}
						begin																		{																			}
							ActivateWindow(Poll.Menu.Item);						{Try to activate the window number that				}
																										{	corresponds to the menu's item number.		}
																										{	This will bring the window to the front.				}
							if not WindowIsOpen(Poll.Menu.Item) then		{If the specified window is not open, it indicates	}
																										{	the requested demo window is not open yet.	}
																										{	So open the requested demo…						}
								case Poll.Menu.Item of									{Determine the item selected within this menu	}
									1:																	{	and open the specified demo window.				}
										OpenDemoWindow1;								{																			}
									2:																	{																			}
										OpenDemoWindow2;								{																			}
									3:																	{																			}
										OpenDemoWindow3;								{																			}
									4:																	{																			}
										OpenDemoWindow4;								{																			}
									5:																	{																			}
										OpenDemoWindow5;								{																			}
									6:																	{																			}
										OpenDemoWindow6;								{																			}
									7:																	{																			}
										OpenDemoWindow7;								{																			}
									8:																	{																			}
										OpenDemoWindow8;								{																			}
								end																	{																			}
						end																			{																			}
				end;																					{																			}


			PaletteDemoMenu:																{ P a l e t t e   D e m o s   M e n u :	-	-	-	-	-	-	}
				begin																				{The 'Palettes' menu is hierarchic, so you treat	}
																										{	it like a separate pull-down menu.  When		}
																										{	selecting an item from the Palettes Demos		}
																										{	menu, it means 'activate this palette window'	}
																										{	if the window is already open, and 'open this	}
																										{	palette window' if it isn't open.							}
					ActivateWindow(Poll.Menu.Item + VerticalPalette - 1);	{Try to activate the palette number that			}
																										{	corresponds to the menu's item number.		}
																										{	This will bring the palette to the front.				}
					if not WindowIsOpen(Poll.Menu.Item + VerticalPalette - 1) then	{If the specified palette is not open	}
																															{	then display the palette…			}
						case Poll.Menu.Item of											{Determine the item selected within this menu	}
							1:																			{	and open the specified palette window.			}
								OpenDemoWindow10;										{																			}
							2:																			{																			}
								OpenDemoWindow11;										{																			}
						end																			{																			}
				end																					{																			}
		end;

		if not ExitTheDemo then															{If the user has not quit the application…			}
			MenuHilite(0);																		{	turn off the highlighted menu in the menu		}
																										{	bar.  This shows the user that the					}
																										{	application has responded to the menu			}
	end;																								{	selection.														}










{ P r e p a r a t i o n s : =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =	}
{	This routine does all the "set up" work for the demo, such as creating the menus and introducing the demo.		}
{	In a full-featured application, you would probably open documents that were double-clicked from the Finder.		}
	procedure PrepareTheDemo;
		const
			QuitAlert = 11700;																{Button definition for a 'Quit' Dynamic Alert		}
			SilentContinueAlert = -11500;												{Button definition for a "Continue" dynamic alert	}
																										{	that doesn't beep the user.								}
		var
			AltButton: integer;																{Button number clicked in an alert box				}
	begin
		SetNullTime(1, maxNullTime);													{Be a good neighbor to other applications by		}
																										{	using fewer CPU cycles.									}
		CursorShape(watchCursor);													{Change the cursor to a wrist watch					}
		MidiPort := ModemIcon;															{By default, select the modem port for a MIDI		}
		MidiSpeed := MidiOneMeg;														{	interface running at 1 MHz. (demo window 3)	}
		ToolBarGray.red := integer(52428);										{Define the toolbar color as a medium dark gray	}
		ToolBarGray.green := integer(52428);									{																			}
		ToolBarGray.blue := integer(52428);										{																			}

																										{ M e n u s :	-	-	-	-	-	-	-	-	-	-	-	-	-	-	}
		AppleMenu('About The Demo…');											{Create the Apple menu with an "About…" item.	}
																										{	You can now work with anything in the Apple	}
																										{	menu without having to write code for it!			}
		Menu(FileMenu, 0, enabled, 'File');											{File menu:	create menu name (menu 1)			}
		Menu(FileMenu, 1, disabled, 'New/N');									{			create 'New' item   (1st item)					}
		Menu(FileMenu, 2, disabled, 'Open/O');									{			create 'Open' item   (2nd item)					}
		Menu(FileMenu, 3, disabled, 'Close/W');									{			create 'Close' item   (3rd item)					}
		Menu(FileMenu, 4, disabled, mDividingLine);							{			4th item is a dividing line							}
		Menu(FileMenu, 5, enabled, 'Quit/Q');									{			create 'Quit' item (5th item)						}
		Menu(EditMenu, 0, enabled, 'Edit');										{Edit menu:	create menu name (menu 2)			}
		Menu(EditMenu, 1, disabled, 'Undo/Z');									{			The Undo, Cut, Copy, Paste, and			}
		Menu(EditMenu, 2, disabled, mDividingLine);							{			Clear items are all disabled.  They			}
		Menu(EditMenu, 3, disabled, 'Cut/X');									{			are automatically enabled as					}
		Menu(EditMenu, 4, disabled, 'Copy/C');									{			required when working with editing			}
		Menu(EditMenu, 5, disabled, 'Paste/V');								{			fields.  Your application can also			}
		Menu(EditMenu, 6, disabled, 'Clear');										{			manually enable/disable these items		}
		Menu(EditMenu, 7, disabled, mDividingLine);							{			when working with other objects				}
		Menu(EditMenu, 8, disabled, 'Show Clipboard…');					{			such as graphics.									}
																										{				The Edit menu is needed for				}
																										{			desk accessories and editing fields.		}
																										{			More items can be added to the Edit		}
																										{			menu, and your application has full			}
																										{			control of these items.							}
		Menu(DemosMenu, 0, enabled, 'Demos');								{Demos:	The 'Demos' menu lets you					}
		Menu(DemosMenu, 1, enabled, 'Lists, Buttons and Cursor Zones/1');		{			open any of the windows		}
		Menu(DemosMenu, 2, enabled, 'Fields, Zooming and Scroll Bars/2');		{			that were created to			}
		Menu(DemosMenu, 3, enabled, 'Icons and Cursor Zones/3');					{			demonstrate a few of Tools	}
		Menu(DemosMenu, 4, enabled, 'Cursor Animation/4');								{			Plus's features.					}
		Menu(DemosMenu, 5, enabled, 'Color Drawing and Multiple Monitors/5');	{														}
		Menu(DemosMenu, 6, enabled, 'Dynamic Alert Boxes/6');							{														}
		Menu(DemosMenu, 7, enabled, 'Pop-Up Menus/7');									{														}
		Menu(DemosMenu, 8, enabled, 'Picture Buttons/8');									{														}
		Menu(DemosMenu, 9, disabled, mDividingLine);										{														}
		Menu(DemosMenu, 10, enabled, 'Show Tool Bar');										{														}
		Menu(DemosMenu, 11, enabled, 'Floating Palettes (Windoids)');	{This item will become an hierarchic menu	}
		Menu(PaletteDemoMenu, 1, enabled, 'Vertical Orientation');		{Create an hierarchic menu by defining the		}
		Menu(PaletteDemoMenu, 2, enabled, 'Horizontal Orientation');	{	menu's items.												}
		AttachMenu(DemosMenu, 11, PaletteDemoMenu);				{Attach the hierarchic 'palettes' menu to the		}
																										{	11th item in the 'Demos' pull-down menu.		}
		UpdateMenuBar;																		{Draw the menu bar with all its menu names		}

																										{ E d i t i n g   F i e l d s :	-	-	-	-	-	-	-	-	-	}
		hField1 := Str30Handle(NewStrHandle(30));							{Allocate memory for the editing fields' strings.	}
		hField2 := NewStrHandle(255);												{	Tools Plus fields reference their related			}
		hField3 := NewStrHandle(255);												{	string by this handle.										}

																										{Display an alert that tells the user how to use	}
																										{	the demo…														}
		AltButton := AlertBox(WatersEdgeLogo, concat('Welcome to the Tools Plus demo.', ReturnKey, ReturnKey, 'Use the “Demos” menu to explore each of the demos prepared for you.  Feel free to play around with the various objects.', ReturnKey, ReturnKey, 'Click “Continue” to start the first demo.'), SilentContinueAlert);
		OpenDemoWindow1;																{Open the first demo window								}
		ResetCursor;																			{Get rid of the wrist watch cursor by resetting it	}
																										{	to its proper shape (Tools Plus figures out		}
																										{	its proper shape depending on where it is on	}
	end;																								{	the screen).														}










{ M a i n   P r o g r a m : =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =   =		}
{	The main program demonstrates the detection of, and response to events.  Tools Plus events are more usable	}
{	than those obtained from The Event Manager.																										}
begin
	InitGraf(@qd.thePort);																{Standard Mac ToolBox initialization					}
	InitFonts;																						{	required by all applications.							}
	InitWindows;																				{																			}
	InitMenus;																					{																			}
	TEInit;																							{																			}
	InitDialogs(nil);																			{																			}
	MaxApplZone;																				{Expand application zone to its limit					}

	if not InitToolsPlus(0, 12, UseColor) then									{Initialize Tools Plus with 0 extra handle blocks	}
		ExitToShell;																				{	and a maximum of 11 open windows.  Use		}
																										{	Color QuickDraw if it’s available.  If					}
																										{	initialization fails, return to the Finder.			}
	PrepareTheDemo;																		{Prepare the demo by creating the menus and	}
																										{	opening a sample window.								}

	ExitTheDemo := false;																	{The demo is not over yet									}
	while not ExitTheDemo do															{Keep polling for an event until the Quit item is	}
																										{	selected in the File menu…								}
		if PollSystem(Poll) then															{If an event has been detected…						}
			case Poll.What of																{Determine what kind of event has occurred…	}
				doMenu:																			{User selected a menu item…								}
					MenuSelection;															{	execute that menu item.									}
				doChgWindow:																{User clicked on an inactive window…				}
					ChangeWindow;															{	activate the window the user clicked.				}
				doRefresh:																		{A window needs to be refreshed…						}
					RefreshWindow;															{	redraw the specified window.							}
				doGoAway:																		{User clicked a window's 'close box'…				}
					CloseTheWindow(Poll.Window);									{	close the affected window.								}
																										{The user has clicked a button, typed a key,		}
																										{	clicked in an inactive field, used a scroll			}
																										{	bar, clicked a line in a list box, used a Pop-	}
																										{	Up Menu, or clicked in the window (not on an	}
																										{	object)…															}
				doButton, doPictButton, doKeyDown, doAutoKey, doClickField, doScrollBar, doListBox, doPopUpMenu, doClick:
					if Poll.Window <> none then										{If the event is applied to an open window…		}
						begin																		{	(ignore events if no window is open)				}
							CurrentWindow(Poll.Window);								{Make the affected window current.  It will be the}
																										{	target of actions that are made in response	}
																										{	to the user's activity in the affected window.	}
							case Poll.Window of											{Determine which window is affected and call the}
								1:																		{	appropriate routine to respond to the user's	}
									ActionInWindow1;										{	activity (ie: call a window-handler routine).		}
								2:																		{																			}
									ActionInWindow2;										{																			}
								3:																		{																			}
									ActionInWindow3;										{																			}
								4:																		{																			}
									ActionInWindow4;										{																			}
								5:																		{																			}
									ActionInWindow5;										{																			}
								6:																		{																			}
									ActionInWindow6;										{																			}
								7:																		{																			}
									ActionInWindow7;										{																			}
								8:																		{																			}
									ActionInWindow8;										{																			}
								9:																		{																			}
									ActionInWindow9;										{																			}
								10:																	{																			}
									ActionInWindow10;										{																			}
								11:																	{																			}
									ActionInWindow11;										{																			}
							end																		{																			}
						end;																			{																			}
				otherwise																			{All other events are ignored								}
			end																						{																			}

		else																							{If this demo was to perform on-going					}
			;																							{	operations, perhaps even when suspended,	}
																										{	that code would go here.									}

	CursorShape(watchCursor);														{Change the cursor to a wrist watch as the demo}
																										{	quits to the Finder.  Visually, it looks more		}
																										{	professional to 'look busy' while the				}
end.																									{	application quits.												}
 ¿Eu       ‘Ej       P  P  Ë|¶ì·ˇ¸ê î!ˇ¿êÅ \É· \Äb  Äc (  AÇ Ä ÄÇ  ÅÑ H  `   Ä H8! @|¶É·ˇ¸NÄ  Et       Et       ,Eu       4Ej    *   p  †  
ã|¶ì·ˇ¸ê î!ˇ¿êÅ \É· \Ä $(  AÇ Ä Äü (Åü $H  `   ÄÇ  ÄÑ (  AÇ Ä Ä¢  ÅÖ H  `   Ä H8! @|¶É·ˇ¸NÄ  Eu       0Et       8Et       LEu       TEj    +   L     à|¶ì·ˇ¸ê î!ˇ¿êa XÉ· XÄ (  AÇ Ä Äü  Åü H  `   Ä H8! @|¶É·ˇ¸NÄ  Eu       0Ej    ,   L  P  0|¶ì·ˇ¸ê î!ˇ¿êa XÉ· XÄ (  AÇ Ä Äü Åü H  `   Ä H8! @|¶É·ˇ¸NÄ  Eu       0Ej    -   h  †  ‘|¶ì·ˇ¸ì¡ˇ¯ê î!ˇ¿êa XÉ· XÄ (  AÇ ,Ä Åü H  `   |~xÄü (  AÇ Äø ì≈  Ä H8! @|¶É·ˇ¸É¡ˇ¯NÄ  Eu       0Ek    /       ∂8`  NÄ  Ek    0     "  8`  ∞d  8`  NÄ  Ek    2     L  p8`  ∞e  8`  NÄ  Ek    3   8  v  ﬂ|¶ê î!ˇ¿êÅ \H  `   ÄÅ \êd  H  `   Ä H8! @|¶NÄ  Eu    4   Eu        Ek    6   l  ®  ]|¶ì·ˇ¸ê î!ˇ∞êa hêÅ lÄa h8Å 8H  |x„4,  @Ç $Äa 8H  `   ÄÅ lêd  H  `   |x„˚xÄ X8! P|¶É·ˇ¸NÄ  Eu        Eu    7   8Eu       HEk    9  ƒ    .|¶ì·ˇ¸ì¡ˇ¯ì°ˇÙê î!ˇ∞êa hêÅ lê° pê¡ tê· xë |ë! Ä;¿  ;‡  Äa h( @Ç ;¿˝ïƒ4,  @Ç4H  `   |}xH  `   |~x≈4,  @Ç î,  @Ç å8` 0H  `   |x(  @Ç ,H  `   |c4,  AÇ H  `   |~4BÄ ;¿ˇî∆4,  @Ç (8`  8Ä  Â˚xH  `   ê  H  `   |~x«4,  AÇ „˚xH  `   »4,  @Çt8` 0H  `   |x(  @Ç ,H  `   |c4,  AÇ H  `   |~4BÄ ;¿ˇî…4,	  @Ç(ÅA lë_ Åa pë ÅÅ |ëü Äb  8Ä  8†  Ê˚xÄ· tH  `   ê  H  `   |~x√4,  @Ç ºÄ  ÄÇ  H  `   H  `   |~xƒ4,  @Ç tÄ  ÄÇ  H  `   H  `   |~x≈4,  @Ç LÄ  8ü H  |~x∆4,  @Ç 0Ä· xTÁ˛(  AÇ  Ä  8Ä  H  `   H  `   |~x»4,  AÇ Ä  H  `   9   ë?   4,
  AÇ „˚xH  `   ;‡  (  AÇ Å BÄ 9`  ÅÅ Äël  √ÛxÄ X8! P|¶É·ˇ¸É¡ˇ¯É°ˇÙNÄ  Eu       XEu       dEu    :   àEu    ;   úEu    ;   ∞Eu    <   ‹Eu       ËEu    =  Eu    :  Eu    ;  0Eu    ;  DEt    >  |Eu    ?  êEu      úEt    @  ∏Eu    A  ºEu      ƒEt    B  ‡Eu    C  ‰Eu      ÏEu      Eu    D  8Eu      @Eu    E  \Eu    =  |Ek    K   ÿ  ,  á|¶ì·ˇ¸ì¡ˇ¯ì°ˇÙê î!ˇ∞|}xêÅ l;‡  (  AÇ ê£Îx8Å 8H  |x„4,  @Ç tÄa 8H  `   |~xH  `   |x‰4,  @Ç LÄæ (  AÇ Ä¡ lÄ˛ ê«  Äa 8H  `   H  `   |xË4,  @Ç √ÛxH  `   „˚xÄ X8! P|¶É·ˇ¸É¡ˇ¯É°ˇÙNÄ  Eu       4Eu    
   LEu       XEu    E   åEu       îEu    =   ∞Ek    L   h  ˆ  X|¶ì·ˇ¸ì¡ˇ¯ê î!ˇ¿êa XH  `   |~xH  `   |x„4,  @Ç √ÛxÄÅ XH  |x„˚xÄ H8! @|¶É·ˇ¸É¡ˇ¯NÄ  Eu       Eu       $Eu       DEk    M   @  \  |¶ê î!ˇ¿H  `   ,  AÇ 8`  H  `   H  `   Ä H8! @|¶NÄ  Eu       Eu    N    Eu       (Ek    O   ¥  à  w|¶ì·ˇ¸ì¡ˇ¯ì°ˇÙê î!ˇ∞H  `   |}xH  `   |x„4,  @Ç \£ÎxH  `   |~xH  `   |x‰4,  @Ç 4Äæ ,<¿Ä 8∆ˇˇ|0 @Ç ;‡˝ïÁ4,  @Ç Å ,9 ë ,„˚xÄ X8! P|¶É·ˇ¸É¡ˇ¯É°ˇÙNÄ  Eu       Eu       $Eu    
   @Eu       LEk    P   ¨  	  —|¶ì·ˇ¸ì¡ˇ¯ì°ˇÙê î!ˇ∞H  `   |}xH  `   |x„4,  @Ç T£ÎxH  `   |~xH  `   |x‰4,  @Ç ,Äæ ,,  @Ç ;‡˝ïÊ4,  @Ç Ä˛ ,8Áˇˇê˛ ,„˚xÄ X8! P|¶É·ˇ¸É¡ˇ¯É°ˇÙNÄ  Eu       Eu       $Eu    
   @Eu       LEk    Q   `  	¥   "|¶ì·ˇ¸ê î!ˇ∞êa hÄa h8Å 8H  |x„4,  @Ç Äa 8H  `   H  `   |x„˚xÄ X8! P|¶É·ˇ¸NÄ  Eu       Eu    R   4Eu       <Ek    S   §  
   “|¶ì·ˇ¸ì¡ˇ¯ê î!ˇ∞êa h|ü#xÄa h8Å 8H  |~x√4,  @Ç PH  `   ÄÅ 8| @Ç 8† ∞ø  BÄ 0Äa 8H  `   Tc>(  @Ç 8¿ ∞ﬂ  BÄ 8‡  ∞ˇ  H  `   Ä X8! P|¶É·ˇ¸É¡ˇ¯NÄ  Eu       $Eu       8Eu    T   \Eu       ÑEk    V  ƒ  
¶  "|¶ì·ˇ¸ì¡ˇ¯ì°ˇÙìÅˇê î!ˇ∞êa h∞Å n|º+xH  `   |}xH  `   |x„4,  @ÇXH  `   |~xH  `   |x‰4,  @Ç4Äa h8Å 8H  |xÂ4,  @Ç†¡ n, AÇ H@Ä ,  @Ä úBÄ ¯, @Ä BÄ Ä· 8| AÇ ‰Äa 8H  `   H  `   |xBÄ »Å 8|Ë @Ç ;‡˝ïÈ4,	  @Ç ¨Äa 88Ä  H  `   H  `   |xÍ4,
  @Ç ÑÅa 8| @Ç xÉ„xH  `   BÄ hÅÅ 8|Ë @Ç ;‡˝ï„4,  @Ç LÄa 88Ä H  `   H  `   |x‰4,  @Ç $Ä° 8| @Ç É„xH  `   BÄ ;‡˝ï„˚xÄ X8! P|¶É·ˇ¸É¡ˇ¯É°ˇÙÉÅˇNÄ  Eu       (Eu       4Eu       LEu       XEu       xEu    R   ƒEu       ÃEu    D   Eu      Eu    R  0Eu    D  `Eu      hEu    R  êEk    W   \  $  &Y|¶ì·ˇ¸ê î!ˇ¿êa X∞Å ^ê° `H  |x„4,  @Ç Äa X†Å ^Ä° `H  |x„˚xÄ H8! @|¶É·ˇ¸NÄ  Eu    P   Eu    V   <Ek    X   4  ê  ',|¶ê î!ˇ¿êa XH  `   ÄÅ Xêd  8`  Ä H8! @|¶NÄ  Eu    Y   Ek    [   h  ¬  '†|¶ì·ˇ¸ì¡ˇ¯ê î!ˇ¿êa XêÅ \ê° `Äa XH  `   |xÄa \ÄÅ `H  |~x„˚xH  `   √ÛxÄ H8! @|¶É·ˇ¸É¡ˇ¯NÄ  Eu    \   $Eu    S   8Eu    \   DEk    ^   ∞  
<  (~|¶ì·ˇ¸ì¡ˇ¯ì°ˇÙìÅˇê î!ˇ∞||x|ù#xÉ„xH  `   |~xÉ„x§Îx8° 8H  |x„4,  @Ç 0†Å 8, AÇ ;‡˝ïÂ4,  @Ç £Îx†Å 88†  H  √ÛxH  `   „˚xÄ X8! P|¶É·ˇ¸É¡ˇ¯É°ˇÙÉÅˇNÄ  Eu    \   (Eu    [   @Eu    V   |Eu    \   ÑEk    _   p  
ﬁ  )¸|¶ì·ˇ¸ì¡ˇ¯ê î!ˇ¿|~x(  AÇ Äb  BÄ 8`  H  `   H  `   |x„4,  @Ç ÄÇ  ìƒ  „˚xÄ H8! @|¶É·ˇ¸É¡ˇ¯NÄ  Et    `    Eu    a   ,Eu       4Et       LEk    c  $  6  *·|¶ì·ˇ¸ì¡ˇ¯ì°ˇÙìÅˇê î!ˇ∞êa h|ù#x|º+xò¡ wÄa h8Å 8H  |x„4,  @Ç ºÄa 8H  `   |~xH  `   |x‰4,  @Ç îà° w(  AÇ HÄa 8(  AÇ ÄÇ  BÄ 8Ä  H  `   H  `   |xÊ4,  @Ç Pìæ ìû BÄ DÄa 8(  AÇ ÄÇ  BÄ 8Ä  H  `   H  `   |xÁ4,  @Ç ìæ ìû  „˚xÄ X8! P|¶É·ˇ¸É¡ˇ¯É°ˇÙÉÅˇNÄ  Eu       4Eu    
   LEu       XEt    d   àEu    e   îEu       úEt    f   ÃEu    g   ÿEu       ‡El      ˇˇˇˇ  )Eo
   j   Ä               Ev d   c    Evd   k   En
   f   Ä               Ev d   +    Evd   k   En
   d   Ä               Ev d   ,    Evd   k   Eo
   l   Ä               Ev d   _    Evd   k   En
   `   Ä               Ev d       Evd   k   Eo
   m   Ä               Ev d   ^    Evd   k   Eo
   n   Ä               Ev d   [    Evd   k   Eo
   o   Ä               Ev d   X    Evd   k   Eo
   p   Ä               Ev d   W    Evd   k   Eo
   q   Ä               Ev d   V    Evd   k   Eo
   r   Ä               Ev d   S    Evd   k   Eo
   s   Ä               Ev d   Q    Evd   k   Eo
   t   Ä               Ev d   P    Evd   k   Eo
   u   Ä               Ev d   O    Evd   k   Eo
   v   Ä               Ev d   M    Evd   k   Eo
   w   Ä               Ev d   L    Evd   k   Eo
   x   Ä               Ev d   K    Evd   k   Eo
   y   Ä               Ev d   9    Evd   k   En
   B   Ä               Ev d   *    Evd   k   En
   @   Ä               Ev d       Evd   k   En
   >   Ä               Ev d   -    Evd   k   Eo
   z   Ä               Ev d   6    Evd   k   Eo
   {   Ä               Ev d   3    Evd   k   Eo
   |   Ä               Ev d   2    Evd   k   Eo
   }   Ä               Ev d   0    Evd   k   Eo
   ~   Ä               Ev d   /    Evd   k   En   f   Ä           Ev
d   f    En   d   Ä           Ev
d   d    En   `   Ä           Ev
d   `    En   B   Ä           Ev
d   B    En   @   Ä           Ev
d   @    En   >   Ä           Ev
d   >    En      Ä           Evd       Eo   k    Ä       Eh  SYMH  <   	                          é     ê   @  ≤   H  «   L  Ë   X      \  %   h  ;   l  N   t  jˇˇˇˇ  {    	          
ˇˇˇ˝           ∂     ≈ˇˇˇˇ  ÷    
          	ˇˇˇ¸           	     	   (  	$ˇˇˇˇ  	=    
      X      e   \       	q     	ë   (  	Ø   4  	ª   D  	Ÿˇˇˇˇ  	Ë    	      h      e   l              
       8       
#      
_   ,  
   4  
Æ   <  
‹   H  
˜   T     `     p  S   |  c   å  ü   ò  ∞   ¨  Ó   ∏     Ã  [   ‹  ß   Ë  ≤     —ˇˇˇˇ  Ê          x                         ˇˇˇ˚    8        
     
     
C   (  
a   <  
àˇˇˇˇ  
â       d   \   ˇˇˇ˙             
Ω     
ø     
‰   $  
˝   8  8   H  Z   \  Öˇˇˇˇ  Ü       d   \   ˇˇˇ˙             Ø     ±     ÷   $     8  -ˇˇˇˇ  .       d   X   ˇˇˇ˙             V     X     }   $  ñ   8  —ˇˇˇˇ  “       d   X   ˇˇˇ˙             ˘     ˚     /   (  E   <  s   H  é   P  ≥ˇˇˇˇ  ¥       d   X   .   d        ˇˇˇ˙            ˛ˇˇˇˇ           P     ]ˇˇˇˇ  n    1ˇˇˇ¯           ø     Ãˇˇˇˇ  ›    1ˇˇˇ¯           !     #      Bˇˇˇˇ  [    5ˇˇˇ˝   \       ó     ∑   (  ’   4  „   H     T  ˇˇˇˇ  ,    	      h   8ˇˇˇ˝   l              
       8       ¡   4  ˇ   8  
   <     H  >   L  Y   X  ç   d  §   p  ª   Ñ  ˛   î  H   ú  Y   ƒ  ã   –  õ   Ë  —   Ù  Ô     ˚        R  (  ú  0  ≠  X  ﬂ     ˚  ˚   ‚s7.5!!Ctrl!Pnls!Fldr!ContentsSyThreadsTimed-68K.πa   MMPRMMCC  MMPRMMCC                       ´âíº  ¬  ›THINK Project ManagerThread Library DocumentationThreadLibrary-10d4.6ThreadsTestThreadsTest-TC.πTITTIES.WAVtitties.zipTrack       J  	Monaco                             ( 
¬Ä ( 
¬Ä                         (  Å⁄       ˛            ˛     0                                                 
                                           &     MPCC:Inter  cool        	  	    P      8  0   H> ê ˙$ iê       Ôíh                 ( ‰5     ˙$  º   Î†Î®                »     ˇˇ   ﬁ     ˇˇ   Ó     ˇˇ  Æ     ˇˇ  π     »ˇˇ  ƒ    ˇˇ   T    Project Version     33ˇˇ’d       @’gP      f   ’d’e     ﬁ`¿&»   P  ¯                 ThreadLibrary.c                                ´±·¸ˇ 
´ï¡¿ˇ ´ï¡¿ˇ ´ï¡¿mB ´ï¡¿mB ´ï¡¿mB ´ª˛ymB ™ÃΩmB ´ï¡¿mB 
´ï¡¿mB ´ï¡¿mB ´ï¡¿mB    ƒ∑ √∑   Ü[6E∑[hÅsƒ˚»eNcK\åT>. GÒÿAbout Tools Plus™…t   ttrottxt  ttrottxt    @                  ´¥Ω    É ≈=c[\õpTåM◊Ç[U⁄[gVZ[PÇgﬁ¥Jg} YOX„^¯alisgx¯ˇ˚ˇgbÄ gZ¯oßgR¯ ˇ¸LgX¯¯ˆgP¯Û
g:Äof%u Ïg8¯fı
g*¯¯˜g"$Û  4Ç4Ç     ◊⁄ ˇ ˇ˛   H   H       ◊⁄      
     ◊⁄ òÄÓ     ◊⁄         H   H                    F≈     ˇˇˇˇˇˇ ÓÓÓÓÓÓ ›››››› ÃÃÃÃÃÃ ªªªªªª ™™™™™™ ôôôôôô àààààà wwwwww 	ffffff 
UUUUUU DDDDDD 333333 
"""" "[Display using Geneva 12]



The Tools Plus Advantage
~~~~~~~~~~~~~~~~~~~~

  There are many advantages to using Tools Plus when you are writing a Macintosh application:




Tools Plus routines work seamlessly with System 5 and System 6 (when running under Finder and MultiFinder), System 7, and Power Macintoshes (in emulation or native mode).


Tools Plus library routines are reusable, self-maintaining components.  They promote code reusability by being reusable themselves, and provide a solid appli a     found       	ÿ  ÿ   ñ ˛  ˛  ˛  ˛  ˛  ˛  ˛    Icon
           @ ±‡±‡    II            @ ±‡±‡                  ´õ∑≠      
n  à  ã˝  à  ã˝  à  ã˝  à  ã˝  à  ã˝  à  ˇˇ      ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ  ˇˇ                                         @ˇ¸Ä  Ä  Ä  Ä  Ä}úÄ}úÄˆ˛Äˆ˛Å€wÅ€wÉÄ„ÇÉÄ„Çá¡¬á¡¬éÄ‚éÄ‚Ä  Ä  Ä  ˇˇˇ˛                              ¯  ¸  ?˛  ˇˇ¸ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇˇ˛ˇˇ ˛MPCCˇˇˇ˛ˇˇ  cool        ~  ö   P      ò  0  RuntimeI⁄                                 0  (
Tools PlusêÄ                             0  (MainÀÚË                                     !p ˘ùÿ\      !`]†            [†       
                 ƒ –                                  à  H d               MWCRuntime.Lib                                 F H e               InterfaceLib                                   F H d               ANSI C.PPC.Lib                          
       F H d