/*	Rez file for balloon document "HyperCuber Balloons" */
/*	Created Mon, May 2, 1994 at 2:28 PM */

#include "Types.r"
#include "BalloonTypes.r"

resource 'hmnu' (3,"Edit", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	HMSkipItem { },
	{

		HMStringResItem {		/* Edit Menu */
			5000,1,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 4:Copy */
			5000,2,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 8:Show Clipboard */
			5000,3,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (2,"File", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	HMSkipItem { },
	{

		HMStringResItem {		/* File Menu */
			5000,4,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 1:New N-Cube */
			5000,5,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Open… */
			5000,6,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 3:Close */
			5000,7,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 4:Save As PICT file */
			5000,8,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 6:Quit */
			5000,9,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (128,"Options", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	HMSkipItem { },
	{

		HMStringResItem {		/* Options Menu */
			5000,10,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 1:Antialias Lines */
			5000,11,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 4:Colors… */
			5000,12,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 5:Keyboard Controls… */
			5000,13,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 6:Mouse Controls… */
			5000,14,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (129,"Viewing Mode", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	HMSkipItem { },
	{
		HMStringResItem {		/* Menu title messages */
			0,0,		/*  Normal */
			0,0,		/*  Grayed */
			0,0,		/*  Checked */
			0,0,		/*  Other */
		},
		HMStringResItem {		/* item 1:Mono */
			5000,15,
			0,0,
			5000,16,
			0,0
		},
		HMStringResItem {		/* item 2:Two-Image Stereo */
			5000,17,
			0,0,
			5000,18,
			0,0
		},
		HMStringResItem {		/* item 3:Two-Color Stereo */
			5000,19,
			0,0,
			5000,20,
			0,0
		}
	}
};

resource 'hmnu' (130,"Windows", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	HMSkipItem { },
	{

		HMStringResItem {		/* Windows Menu */
			5000,21,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 1:Hide Menubar */
			5000,22,
			5000,23,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Full-Screen */
			5000,24,
			0,0,
			5000,25,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 4:3D Controls */
			5000,26,
			0,0,
			5000,27,
			0,0
		},
		HMStringResItem {		/* item 5:4D Controls */
			5000,28,
			0,0,
			5000,29,
			0,0
		},
		HMStringResItem {		/* item 6:5D Controls */
			5000,30,
			0,0,
			5000,31,
			0,0
		},
		HMStringResItem {		/* item 7:6D Controls */
			5000,32,
			0,0,
			5000,33,
			0,0
		},
		HMStringResItem {		/* item 8:7D Controls */
			5000,34,
			0,0,
			5000,35,
			0,0
		}
	}
};

resource 'hrct' (5000,"", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{

		HMStringResItem {		/* Angle Scroll Bar */
			{0,0},
			{0,0,0,0},
			5000,36,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Colors Background */
			{0,0},
			{0,0,0,0},
			5000,37,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Colors Cancel */
			{0,0},
			{0,0,0,0},
			5000,38,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Colors Default */
			{0,0},
			{0,0,0,0},
			5000,39,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Colors Left Eye */
			{0,0},
			{0,0,0,0},
			5000,40,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Colors Okay */
			{0,0},
			{0,0,0,0},
			5000,41,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Colors Right Eye */
			{0,0},
			{0,0,0,0},
			5000,42,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Image */
			{0,0},
			{0,0,0,0},
			5000,43,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Key Add */
			{0,0},
			{0,0,0,0},
			5000,44,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Key Cancel */
			{0,0},
			{0,0,0,0},
			5000,45,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Key Defaults */
			{0,0},
			{0,0,0,0},
			5000,46,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Key Edit */
			{0,0},
			{0,0,0,0},
			5000,47,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Key Okay */
			{0,0},
			{0,0,0,0},
			5000,48,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Key Remove */
			{0,0},
			{0,0,0,0},
			5000,49,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* KeyEd Angle */
			{0,0},
			{0,0,0,0},
			5000,50,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* KeyEd Cancel */
			{0,0},
			{0,0,0,0},
			5001,1,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* KeyEd Dimension */
			{0,0},
			{0,0,0,0},
			5001,2,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* KeyEd Direction */
			{0,0},
			{0,0,0,0},
			5001,3,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* KeyEd Increment */
			{0,0},
			{0,0,0,0},
			5001,4,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* KeyEd Key */
			{0,0},
			{0,0,0,0},
			5001,5,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* KeyEd Okay */
			{0,0},
			{0,0,0,0},
			5001,6,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Mouse Add */
			{0,0},
			{0,0,0,0},
			5001,7,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Mouse Cancel */
			{0,0},
			{0,0,0,0},
			5001,8,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Mouse Defaults */
			{0,0},
			{0,0,0,0},
			5001,9,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Mouse Edit */
			{0,0},
			{0,0,0,0},
			5001,10,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Mouse Okay */
			{0,0},
			{0,0,0,0},
			5001,11,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Mouse Remove */
			{0,0},
			{0,0,0,0},
			5001,12,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Angle */
			{0,0},
			{0,0,0,0},
			5001,13,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Cancel */
			{0,0},
			{0,0,0,0},
			5001,14,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Command */
			{0,0},
			{0,0,0,0},
			5001,15,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Control */
			{0,0},
			{0,0,0,0},
			5001,16,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Dimension */
			{0,0},
			{0,0,0,0},
			5001,17,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Direction */
			{0,0},
			{0,0,0,0},
			5001,18,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Multiplier */
			{0,0},
			{0,0,0,0},
			5001,19,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Okay */
			{0,0},
			{0,0,0,0},
			5001,20,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Option */
			{0,0},
			{0,0,0,0},
			5001,21,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* MouseEd Shift */
			{0,0},
			{0,0,0,0},
			5001,22,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* N-Cube Dimension */
			{0,0},
			{0,0,0,0},
			5001,23,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* N-Cube Okay */
			{0,0},
			{0,0,0,0},
			5001,24,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* NCube Cancel */
			{0,0},
			{0,0,0,0},
			5001,25,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* Perspective Scroll Bar */
			{0,0},
			{0,0,0,0},
			5001,26,
			0,0,
			0,0,
			0,0
		}
	}
};



resource 'STR#' (5000, "Balloon Help String 1") {
	{
		/* [1] Edit Menu, Normal */
			"Use this menu to copy a picture of the object to the clipbo"
			"ard, or to view the clipboard.",
		/* [2] Copy, Normal */
			"This copies the current view of the object to the clipboard"
			". You can then paste the picture into another program.",
		/* [3] Show Clipboard, Normal */
			"This shows the current contents of the Clipboard.",
		/* [4] File Menu, Normal */
			"Use this menu to open and view objects, to save a picture o"
			"f an object, or to exit HyperCuber.",
		/* [5] New N-Cube, Normal */
			"This creates and displays a new monochrome n-cube.",
		/* [6] Open…, Normal */
			"This opens and views an object.",
		/* [7] Close, Normal */
			"This closes the current object and all related windows.",
		/* [8] Save As PICT file, Normal */
			"This saves the current view as a picture (in a PICT file).",
		/* [9] Quit, Normal */
			"This quits HyperCuber.",
		/* [10] Options Menu, Normal */
			"Use this menu to change the colors used to draw the object,"
			" and to set up keyboard and mouse shortcuts to control the s"
			"croll bars. ",
		/* [11] Antialias Lines, Normal */
			"This makes line segments look much better, but also slows d"
			"own display.",
		/* [12] Colors…, Normal */
			"This lets you change the colors used to draw the object.",
		/* [13] Keyboard Controls…, Normal */
			"This lets you set up keyboard shortcuts so you can control "
			"the scroll bars from the keyboard.",
		/* [14] Mouse Controls…, Normal */
			"This lets you set up mouse shortcuts so you can control the"
			" scroll bars by clicking and dragging in the object window.",
		/* [15] Mono, Normal */
			"This shows the object as a single image.",
		/* [16] Mono, Checked */
			"This is the current viewing mode. It shows the object as a "
			"single image.",
		/* [17] Two-Image Stereo, Normal */
			"This shows the object as two images side-by-side, each view"
			"ed from a slightly different angle. If you look at the image"
			" cross-eyed, the object will appear to be three-dimensional.",
		/* [18] Two-Image Stereo, Checked */
			"This is the current viewing mode. It shows the object as tw"
			"o images side-by-side, each viewed from a slightly different"
			" angle. If you look at the image cross-eyed, the object will"
			" appear to be three-dimensional.",
		/* [19] Two-Color Stereo, Normal */
			"This shows the object as two colored images superimposed, e"
			"ach viewed from a slightly different angle. If you look at t"
			"he image with special glasses, the object will appear to be "
			"three-dimensional.",
		/* [20] Two-Color Stereo, Checked */
			"This is the current viewing mode. It shows the object as tw"
			"o colored images superimposed, from slightly different angle"
			"s. If you look at the image with special glasses, the object"
			" will appear to be three-dimensional.",
		/* [21] Windows Menu, Normal */
			"Use this menu to show or hide controls windows, and to make"
			" the object window as large as possible.",
		/* [22] Hide Menubar, Normal */
			"This hides the menu bar. The menu bar will re-appear when y"
			"ou click at the top of the screen.",
		/* [23] Hide Menubar, Grayed */
			"This would hide the menu bar, but your window must be full-"
			"screen before you can hide the menu bar.",
		/* [24] Full-Screen, Normal */
			"This makes the window take up the entire screen.",
		/* [25] Full-Screen, Checked */
			"This shrink the window to the size it was before you made i"
			"t full-screen.",
		/* [26] 3D Controls, Normal */
			"This shows the 3D Controls window.",
		/* [27] 3D Controls, Checked */
			"This hides the 3D Controls window.",
		/* [28] 4D Controls, Normal */
			"This shows the 4D Controls window.",
		/* [29] 4D Controls, Checked */
			"This hides the 4D Controls window.",
		/* [30] 5D Controls, Normal */
			"This shows the 5D Controls window.",
		/* [31] 5D Controls, Checked */
			"This hides the 5D Controls window.",
		/* [32] 6D Controls, Normal */
			"This shows the 6D Controls window.",
		/* [33] 6D Controls, Checked */
			"This hides the 7D Controls window.",
		/* [34] 7D Controls, Normal */
			"This shows the 7D Controls window.",
		/* [35] 7D Controls, Checked */
			"This hides the 7D Controls window.",
		/* [36] Angle Scroll Bar, Normal */
			"This scroll bar controls the viewpoint angle named to the l"
			"eft. Scrolling this bar will change the angle. The number to"
			" the left is the current value of this angle.",
		/* [37] Colors Background, Normal */
			"The color used to draw the background of the image. Click i"
			"n the color box to change the background color.",
		/* [38] Colors Cancel, Normal */
			"Click this to cancel this without changing any colors.",
		/* [39] Colors Default, Normal */
			"Click this to change all colors to their default values.",
		/* [40] Colors Left Eye, Normal */
			"Shows the color currently used to draw the left eye image, "
			"when viewing in two-color stereo. Click in the color box to "
			"change the left eye color.",
		/* [41] Colors Okay, Normal */
			"Click this to accept these colors.",
		/* [42] Colors Right Eye, Normal */
			"The color currently used to draw the right eye image, when "
			"viewing in two-color stereo. Click in the color box to chang"
			"e the right eye color.",
		/* [43] Image, Normal */
			"This is the image. Click and drag here to invoke mouse cont"
			"rols.",
		/* [44] Key Add, Normal */
			"Click here to add a new key control to the end of the list.",
		/* [45] Key Cancel, Normal */
			"Click here to cancel without changing the key controls.",
		/* [46] Key Defaults, Normal */
			"Click here to change to the default set of key controls.",
		/* [47] Key Edit, Normal */
			"Click here to edit the selected key control.",
		/* [48] Key Okay, Normal */
			"Click here to accept these key controls.",
		/* [49] Key Remove, Normal */
			"Click here to remove the selected key control.",
		/* [50] KeyEd Angle, Normal */
			"The angle to change."
	}
};

resource 'STR#' (5001, "Balloon Help String 2") {
	{
		/* [1] KeyEd Cancel, Normal */
			"Click here to cancel this without changing this control.",
		/* [2] KeyEd Dimension, Normal */
			"The dimension of the angle to change.",
		/* [3] KeyEd Direction, Normal */
			"The direction to change the angle.",
		/* [4] KeyEd Increment, Normal */
			"The amount by which to change the angle.",
		/* [5] KeyEd Key, Normal */
			"The key and modifiers to press to invoke this key control. "
			"To change this, click here and press the new key combination"
			".",
		/* [6] KeyEd Okay, Normal */
			"Click here to accept this control.",
		/* [7] Mouse Add, Normal */
			"Click here to add a new mouse control to the end of the lis"
			"t.",
		/* [8] Mouse Cancel, Normal */
			"Click here to cancel without changing the mouse controls.",
		/* [9] Mouse Defaults, Normal */
			"Click here to change to the default set of mouse controls.",
		/* [10] Mouse Edit, Normal */
			"Click here to edit the selected mouse control.",
		/* [11] Mouse Okay, Normal */
			"Click here to accept these mouse controls.",
		/* [12] Mouse Remove, Normal */
			"Click here to remove the selected mouse control.",
		/* [13] MouseEd Angle, Normal */
			"The angle to change.",
		/* [14] MouseEd Cancel, Normal */
			"Click here to cancel this without changing this mouse contr"
			"ol.",
		/* [15] MouseEd Command, Normal */
			"This is checked if you should hold down the command (clover"
			"leaf) key while dragging to invoke this control. Click here "
			"to change this.",
		/* [16] MouseEd Control, Normal */
			"This is checked if you should hold down the control key whi"
			"le dragging to invoke this control. Click here to change thi"
			"s.",
		/* [17] MouseEd Dimension, Normal */
			"The dimension of the angle to change.",
		/* [18] MouseEd Direction, Normal */
			"If this is horizontal, the angle will track the mouse’s hor"
			"izontal movements. If it is vertical, the angle will track t"
			"he mouse’s vertical movements.",
		/* [19] MouseEd Multiplier, Normal */
			"This is multiplied by the mouse movements when computing ho"
			"w much the angle should change. To make the angle change mor"
			"e quickly, increase the magnitude of this number. To reverse"
			" the angle’s changes, reverse the sign of this number.",
		/* [20] MouseEd Okay, Normal */
			"Click here to accept this mouse control.",
		/* [21] MouseEd Option, Normal */
			"This is checked if you should hold down the option key whil"
			"e dragging to invoke this control. Click here to change this"
			".",
		/* [22] MouseEd Shift, Normal */
			"This is checked if you should hold down the shift key while"
			" dragging to invoke this control. Click here to change this.",
		/* [23] N-Cube Dimension, Normal */
			"Type the dimension of the n-cube you want to create. For in"
			"stance, to create a four-dimensional cube, type 4 here.",
		/* [24] N-Cube Okay, Normal */
			"Click this to create the n-cube.",
		/* [25] NCube Cancel, Normal */
			"Click this if you do not want to create an n-cube.",
		/* [26] Perspective Scroll Bar, Normal */
			"This scroll bar controls the perspective parameter named to"
			" the left. Scrolling this bar will change the parameter. The"
			" number to the left is the current value of this perspective"
			" parameter."
	}
};

data 'TEXT' (5000, purgeable) {
			"This program displays and rotates objects of higher dimensi"
			"ons."
};

resource 'hfdr' (-5696, purgeable) {	/* Help balloon for application icon */
	HelpMgrVersion, hmDefaultOptions, 0, 0, /* header information */
	{
	HMTEResItem { 5000 }
	}
};

