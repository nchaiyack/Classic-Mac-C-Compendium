/*	Rez file for balloon document "MacRTrace Balloons" */
/*	Created Mon, Jan 16, 1995 at 9:11 PM */

#include "Types.r"
#include "BalloonTypes.r"

resource 'hmnu' (1003,"Edit", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSTRResItem { 0,0,0,0 },			/* item -1:Default balloon for Edit */
	{

		HMStringResItem {		/* Menu Title */
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
			5000,3,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 8:Preferences… */
			5000,4,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (1002,"File", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSTRResItem { 0,0,0,0 },			/* item -1:Default balloon for File */
	{

		HMStringResItem {		/* Menu Title */
			5000,5,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 1:Open scene file */
			5000,6,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Save image */
			5000,7,
			5000,8,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 4:Quit */
			5000,9,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (1004,"Windows", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSTRResItem { 0,0,0,0 },			/* item -1:Default balloon for Windows */
	{

		HMStringResItem {		/* Menu Title */
			5000,10,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 1:Show Options Window */
			5000,11,
			5000,12,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Show Status Window */
			5000,13,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 3:Hide Log Window */
			5000,14,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 4:Show Animation Window */
			5000,15,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 5:Show Image Window */
			5000,16,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hdlg' (5000,"About...", purgeable) {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,

		HMStringResItem {		/* Default balloon for About... */
			{0,0},
			{0,0,0,0},
			5000,17,
			0,0,
			0,0,
			0,0
		},
	{

		HMStringResItem {		/* item 1:Okay! */
			{0,0},
			{0,0,0,0},
			5000,18,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:More… */
			{0,0},
			{0,0,0,0},
			5000,19,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 5:Secret Animation */
			{0,0},
			{0,0,0,0},
			5000,20,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hdlg' (5001,"Animation Dialog", purgeable) {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,

		HMStringResItem {		/* Default balloon for Animation Dialog */
			{0,0},
			{0,0,0,0},
			5000,21,
			0,0,
			0,0,
			0,0
		},
	{

		HMStringResItem {		/* item 1:Okay */
			{0,0},
			{0,0,0,0},
			5000,22,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Cancel */
			{0,0},
			{0,0,0,0},
			5000,23,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 5:t Start */
			{0,0},
			{0,0,0,0},
			5000,24,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 6:t End */
			{0,0},
			{0,0,0,0},
			5000,25,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 10:t Step */
			{0,0},
			{0,0,0,0},
			5000,26,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 14:Eye x */
			{0,0},
			{0,0,0,0},
			5000,27,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 15:Eye y */
			{0,0},
			{0,0,0,0},
			5000,28,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 16:Eye z */
			{0,0},
			{0,0,0,0},
			5000,29,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 21:Up x */
			{0,0},
			{0,0,0,0},
			5000,30,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 22:Up y */
			{0,0},
			{0,0,0,0},
			5000,31,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 23:Up z */
			{0,0},
			{0,0,0,0},
			5000,32,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 28:Look x */
			{0,0},
			{0,0,0,0},
			5000,33,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 29:Look y */
			{0,0},
			{0,0,0,0},
			5000,34,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 30:Look z */
			{0,0},
			{0,0,0,0},
			5000,35,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 33:Horiz Angle Aperature */
			{0,0},
			{0,0,0,0},
			5000,36,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 36:Vert Angle Aperature */
			{0,0},
			{0,0,0,0},
			5000,37,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hrct' (5000,"Animation Dialog", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{

		HMStringResItem {		/* Animation Dialog */
			{0,0},
			{0,0,0,0},
			5000,38
		}
	}
};

resource 'hdlg' (5002,"Options Dialog", purgeable) {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,

		HMStringResItem {		/* Default balloon for Options Dialog */
			{0,0},
			{0,0,0,0},
			5000,39,
			0,0,
			0,0,
			0,0
		},
	{

		HMTEResItem {		/* item 1:Render */
			{0,0},
			{0,0,0,0},
			5000,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Defaults */
			{0,0},
			{0,0,0,0},
			5000,40,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 4:Image Width */
			{0,0},
			{0,0,0,0},
			5000,41,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 6:Image Height */
			{0,0},
			{0,0,0,0},
			5000,42,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 8:Aliasing Threshold */
			{0,0},
			{0,0,0,0},
			5000,43,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 10:Aliasing Threshold Menu */
			{0,0},
			{0,0,0,0},
			5000,44,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 12:Shading Threshold */
			{0,0},
			{0,0,0,0},
			5000,45,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 14:Shading Threshold Menu */
			{0,0},
			{0,0,0,0},
			5000,46,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 15:Ambient Threshold */
			{0,0},
			{0,0,0,0},
			5000,47,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 17:Ambient Threshold Menu */
			{0,0},
			{0,0,0,0},
			5000,48,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 18:Eye Separation */
			{0,0},
			{0,0,0,0},
			5000,49,
			5000,50,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMTEResItem {		/* item 20:Eye Separation Menu */
			{0,0},
			{0,0,0,0},
			5001,
			5002,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 22:Ambient Samples */
			{0,0},
			{0,0,0,0},
			5001,1,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 24:Ambient Levels */
			{0,0},
			{0,0,0,0},
			5001,2,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 26:Shading Levels */
			{0,0},
			{0,0,0,0},
			5001,3,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 28:Cluster Size */
			{0,0},
			{0,0,0,0},
			5001,4,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 30:Focal Aperature */
			{0,0},
			{0,0,0,0},
			5001,5,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMTEResItem {		/* item 32:Focal Distance */
			{0,0},
			{0,0,0,0},
			5003,
			5004,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 33:Focal Distance: */
			{0,0},
			{0,0,0,0},
			5001,6,
			0,0,
			5001,7,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 36:Intersect adjust */
			{0,0},
			{0,0,0,0},
			5001,8,
			0,0,
			5001,9,
			0,0
		},
		HMStringResItem {		/* item 37:Only correct normals inside */
			{0,0},
			{0,0,0,0},
			5001,10,
			0,0,
			5001,11,
			0,0
		},
		HMStringResItem {		/* item 38:Use jittered sampling */
			{0,0},
			{0,0,0,0},
			5001,12,
			0,0,
			5001,13,
			0,0
		},
		HMStringResItem {		/* item 39:Intersect all objects */
			{0,0},
			{0,0,0,0},
			5001,14,
			0,0,
			5001,15,
			0,0
		},
		HMStringResItem {		/* item 40:Animate... */
			{0,0},
			{0,0,0,0},
			5001,16,
			0,0,
			5001,17,
			0,0
		},
		HMTEResItem {		/* item 41:Correct texture normals */
			{0,0},
			{0,0,0,0},
			5005,
			0,0,
			5006,
			0,0
		},
		HMStringResItem {		/* item 42:Translucence Menu */
			{0,0},
			{0,0,0,0},
			5001,18,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 44:Sampling menu */
			{0,0},
			{0,0,0,0},
			5001,19,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 46:View Menu */
			{0,0},
			{0,0,0,0},
			5001,20,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 48:Shading Menu */
			{0,0},
			{0,0,0,0},
			5001,21,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 50:Antialiasing Menu */
			{0,0},
			{0,0,0,0},
			5001,22,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMTEResItem {		/* item 52:Backfaces Menu */
			{0,0},
			{0,0,0,0},
			5007,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMTEResItem {		/* item 54:Walk Mode Menu */
			{0,0},
			{0,0,0,0},
			5008,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hrct' (5001,"Options Dialog", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{

		HMStringResItem {		/* Options Dialog */
			{0,0},
			{0,0,0,0},
			5001,23
		}
	}
};

resource 'hdlg' (5003,"Preferences", purgeable) {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,

		HMStringResItem {		/* Default balloon for Preferences */
			{0,0},
			{0,0,0,0},
			5001,24,
			0,0,
			0,0,
			0,0
		},
	{

		HMStringResItem {		/* item 1:Okay */
			{0,0},
			{0,0,0,0},
			5001,25,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 3:Allow background tasks */
			{0,0},
			{0,0,0,0},
			5001,26,
			0,0,
			5001,27,
			0,0
		},
					HMSkipItem { },
		HMTEResItem {		/* item 5:Time Between Backgrounds */
			{0,0},
			{0,0,0,0},
			5009,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 9:Show image as it is rendered */
			{0,0},
			{0,0,0,0},
			5001,28,
			0,0,
			5001,29,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 11:Show Status window for each render */
			{0,0},
			{0,0,0,0},
			5001,30,
			0,0,
			5001,31,
			0,0
		},
		HMStringResItem {		/* item 12:Hide Options window during render */
			{0,0},
			{0,0,0,0},
			5001,32,
			0,0,
			5001,33,
			0,0
		},
		HMStringResItem {		/* item 13:Show About window at program startup */
			{0,0},
			{0,0,0,0},
			5001,34,
			0,0,
			5001,35,
			0,0
		},
		HMStringResItem {		/* item 14:Use Textures */
			{0,0},
			{0,0,0,0},
			5001,36,
			0,0,
			5001,37,
			0,0
		},
		HMStringResItem {		/* item 15:Textures inside objects in scene file */
			{0,0},
			{0,0,0,0},
			5001,38,
			5001,39,
			5001,40,
			0,0
		},
		HMStringResItem {		/* item 16:Textures after objects in scene file */
			{0,0},
			{0,0,0,0},
			5001,41,
			5001,42,
			5001,43,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 18:Max Objects */
			{0,0},
			{0,0,0,0},
			5001,44,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 20:Max Lights */
			{0,0},
			{0,0,0,0},
			5001,45,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 22:Max Surfaces */
			{0,0},
			{0,0,0,0},
			5001,46,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 24:Max CSG Objects */
			{0,0},
			{0,0,0,0},
			5001,47,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 26:Defaults */
			{0,0},
			{0,0,0,0},
			5001,48,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 27:Revert */
			{0,0},
			{0,0,0,0},
			5001,49,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hrct' (5002,"Preferences", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{

		HMStringResItem {		/* Preferences Dialog */
			{0,0},
			{0,0,0,0},
			5001,50
		}
	}
};

resource 'hdlg' (5004,"Status Dialog", purgeable) {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,

		HMStringResItem {		/* Default balloon for Status Dialog */
			{0,0},
			{0,0,0,0},
			5002,1,
			0,0,
			0,0,
			0,0
		},
	{

		HMStringResItem {		/* item 1:Done reading sc… */
			{0,0},
			{0,0,0,0},
			0,0,
			5002,2,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
		HMStringResItem {		/* item 4:Status Bar */
			{0,0},
			{0,0,0,0},
			0,0,
			5002,3,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 6:Memory Bar */
			{0,0},
			{0,0,0,0},
			0,0,
			5002,4,
			0,0,
			0,0
		}
	}
};

resource 'hrct' (5003,"Log", purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{

		HMStringResItem {		/* Log Window */
			{0,0},
			{0,0,0,0},
			5002,5
		}
	}
};



resource 'hwin' (5000) {
	HelpMgrVersion,
	0,
	{
		5003,	'hrct',	3,	"Log",
		5004,	'hdlg',	13,	"Status Dialog",
		5003,	'hdlg',	11,	"Preferences",
		5002,	'hrct',	11,	"Preferences",
		5002,	'hdlg',	14,	"Options Dialog",
		5001,	'hrct',	14,	"Options Dialog",
		5001,	'hdlg',	16,	"Animation Dialog",
		5000,	'hrct',	16,	"Animation Dialog",
		5000,	'hdlg',	8,	"About..."
	}
};

resource 'STR#' (5000, "Balloon Help String 1") {
	{
		/* [1] Menu Title, Normal */
			"This menu allows you to copy the current rendered image to "
			"the Clipboard, or to change the preferences.",
		/* [2] Copy, Normal */
			"Choose this to copy the contents of the Image window to the"
			" clipboard.",
		/* [3] Copy, Grayed */
			"This allows you to copy the image to the clipboard. It is d"
			"immed because there is no complete rendered image.",
		/* [4] Preferences…, Normal */
			"Choose this to change the program preferences (changes to p"
			"references survive between sessions).",
		/* [5] Menu Title, Normal */
			"This menu allows you to open scene  description files, to s"
			"ave rendered images, and to quit MacRTrace.",
		/* [6] Open scene file, Normal */
			"Choose this to read a new scene description file (ending in"
			" .sff) into memory.",
		/* [7] Save image, Normal */
			"This allows you to save the picture in the Image window to "
			"disk.",
		/* [8] Save image, Grayed */
			"This allows you to have a rendered image. It is grayed beca"
			"use there is currently no complete rendered image.",
		/* [9] Quit, Normal */
			"Choose this to quit MacRTrace.",
		/* [10] Menu Title, Normal */
			"Use this menu to show or hide MacRTrace windows.",
		/* [11] Show Options Window, Normal */
			"Choose this to show or hide the window containing the optio"
			"ns for the scene in memory.",
		/* [12] Show Options Window, Grayed */
			"This allows you to show or hide the window containing the o"
			"ptions for the scene in memory. It is dimmed because there i"
			"s current no scene in memory.",
		/* [13] Show Status Window, Normal */
			"Choose this to show or hide the Status window.",
		/* [14] Hide Log Window, Normal */
			"Click here to show or hide the Log window.",
		/* [15] Show Animation Window, Normal */
			"Choose this to show or hide the Animation window.",
		/* [16] Show Image Window, Normal */
			"Choose this to show or hide the Image window.",
		/* [17] Default balloon for About..., Normal */
			"This window displays information about MacRTrace.",
		/* [18] Okay!, Normal */
			"Click here to close this window.",
		/* [19] More…, Normal */
			"Click here to switch between the pages of information.",
		/* [20] Secret Animation, Normal */
			"Click here....",
		/* [21] Default balloon for Animation Dialog, Normal */
			"Use this window to change the animation parameters.",
		/* [22] Okay, Normal */
			"Click here to accept these settings and close this window.",
		/* [23] Cancel, Normal */
			"Click here to close this window and turn off animation.",
		/* [24] t Start, Normal */
			"This is the starting bound for the variable t.",
		/* [25] t End, Normal */
			"This is the ending bound for the variable t.",
		/* [26] t Step, Normal */
			"This is the number of steps the variable t takes, getting f"
			"rom the starting bound to the ending bound.",
		/* [27] Eye x, Normal */
			"This is the x coordinate of the Eye point (the position of "
			"the eye, or camera). It may be a function of t.",
		/* [28] Eye y, Normal */
			"This is the y coordinate of the Eye point (the position of "
			"the eye, or camera). It may be a function of t.",
		/* [29] Eye z, Normal */
			"This is the z coordinate of the Eye point (the position of "
			"the eye, or camera). It may be a function of t.",
		/* [30] Up x, Normal */
			"This is the x coordinate of the Up vector (direction which "
			"appears to be up). It may be a function of t.",
		/* [31] Up y, Normal */
			"This is the y coordinate of the Up vector (direction which "
			"appears to be up). It may be a function of t.",
		/* [32] Up z, Normal */
			"This is the z coordinate of the Up vector (direction which "
			"appears to be up). It may be a function of t.",
		/* [33] Look x, Normal */
			"This is the x coordinate of the Look point (the point we’re"
			" looking at). It may be a function of t.",
		/* [34] Look y, Normal */
			"This is the y coordinate of the Look point (the point we’re"
			" looking at). It may be a function of t.",
		/* [35] Look z, Normal */
			"This is the z coordinate of the Look point (the point we’re"
			" looking at). It may be a function of t.",
		/* [36] Horiz Angle Aperature, Normal */
			"This is the horizontal angle aperature. It may be a functio"
			"n of t.",
		/* [37] Vert Angle Aperature, Normal */
			"This is the vertical angle aperature. It may be a function "
			"of t.",
		/* [38] Animation Dialog, Normal */
			"Use this window to change the animation settings.",
		/* [39] Default balloon for Options Dialog, Normal */
			"Use this window to change the rendering options.",
		/* [40] Defaults, Normal */
			"Click here to set the rendering options to their default va"
			"lues.",
		/* [41] Image Width, Normal */
			"This is the width in pixels of the rendered image.",
		/* [42] Image Height, Normal */
			"This is the height in pixels of the rendered image.",
		/* [43] Aliasing Threshold, Normal */
			"This controls image quality through pixel supersampling. Th"
			"e range is 0.1 (poor) through 0.03 (best).",
		/* [44] Aliasing Threshold Menu, Normal */
			"Click here to choose the Aliasing threshold from a menu.",
		/* [45] Shading Threshold, Normal */
			"This controls image quality through shading rays propagatio"
			"n. The range is 0.01 (poor) through 0.001 (best).",
		/* [46] Shading Threshold Menu, Normal */
			"Click here to choose the Shading threshold from a menu.",
		/* [47] Ambient Threshold, Normal */
			"This controls the image quality through ambient rays distri"
			"bution and caching. The range is 0.01 (poor) through 0.00001"
			" (best). 0 means no ambient threshold.",
		/* [48] Ambient Threshold Menu, Normal */
			"Click here to choose the Ambient threshold from a menu.",
		/* [49] Eye Separation, Normal */
			"This controls the separation between the eyes in Left or Ri"
			"ght eye view. The interpretation of this number depends on t"
			"he selection in the menu to the right.",
		/* [50] Eye Separation, Grayed */
			" This controls the separation between the eyes in Left or R"
			"ight eye view. It is dimmed because the View is Normal (no s"
			"tereo separation)."
	}
};

resource 'STR#' (5001, "Balloon Help String 2") {
	{
		/* [1] Ambient Samples, Normal */
			"This defines the maximum number of distributed rays to be u"
			"sed in ambient lighting calculations. Use low values.",
		/* [2] Ambient Levels, Normal */
			"This is the number of shading levels (shading tree depth) i"
			"n which ambient lighting calculations will be done through r"
			"ay distribution. Use low values.",
		/* [3] Shading Levels, Normal */
			"This is the maximum shading tree depth. When a scene has tr"
			"ansparent or reflexive objects, it may be important to lower"
			" this parameter, or else the tracing never stops. In most ca"
			"sts, there should be no problem allowing it to be large.",
		/* [4] Cluster Size, Normal */
			"This is the number of objects in a cluster (clusters are us"
			"ed to group objects during rendering). Use a low value for s"
			"parse scenes, and a high value for dense scenes.",
		/* [5] Focal Aperature, Normal */
			"This is the focal aperature of the camera which is taking t"
			"he picture. 0.0 is a pinhole camera. If this is non-zero, th"
			"ere is depth of field, so adaptive supersampling antialiasin"
			"g will not work very well.",
		/* [6] Focal Distance:, Normal */
			"Click here to use the value in the Focal Distance field as "
			"the focal distance.",
		/* [7] Focal Distance:, Checked */
			"Click here to use the distance from the Look point to the E"
			"ye point as the focal distance (ignoring the value in the Fo"
			"cal Distance field).",
		/* [8] Intersect adjust, Normal */
			"Click here to prevents some problems with self-intersection"
			"s. If the scene has text objects, you should click here to t"
			"urn this on.",
		/* [9] Intersect adjust, Checked */
			"Click here turn off an option whch prevents some problems w"
			"ith self-intersections. If the scene has text objects, you s"
			"hould keep this on.",
		/* [10] Only correct normals inside, Normal */
			"Click here to correct surface normals, so that they point a"
			"gainst the incidence ray. You should turn this on if you are"
			" using “correct” objects.",
		/* [11] Only correct normals inside, Checked */
			"Click here to turn off the correction of surface normals. Y"
			"ou should keep this on if you are using “correct” objects.",
		/* [12] Use jittered sampling, Normal */
			"Click here to turn on jittered sampling. Sometimes jittered"
			" sampling produces better images in scenes with small tricky"
			" details.",
		/* [13] Use jittered sampling, Checked */
			"Click here to turn off jittered sampling. Sometimes jittere"
			"d sampling produces better images in scenes with small trick"
			"y details.",
		/* [14] Intersect all objects, Normal */
			"Click here, if you’re using adaptive supersampling antialia"
			"sing mode, to test all scene objects while rendering each pi"
			"xel. This will slow things down considerably, but may render"
			" very small objects more accurately.",
		/* [15] Intersect all objects, Checked */
			"Click here, if you’re using adaptive supersampling antialia"
			"sing mode, to test only the objects found at the corners of "
			"and inside each pixel. This will speed things up considerabl"
			"y, but may render very small objects less accurately.\$0D",
		/* [16] Animate..., Normal */
			"Click here to turn animation off. When animation is off, th"
			"e values in the Animation window are used to generate a seri"
			"es of image frames for the animation.",
		/* [17] Animate..., Checked */
			"Click here to turn animation off. When animation is off, th"
			"e values in the Animation window are ignored, and a single i"
			"mage is generated each render.",
		/* [18] Translucence Menu, Normal */
			"Choose an option from this menu to control the generation o"
			"f shadow rays through non-opaque objects. If a scene has tra"
			"nslucent objects, full or partial shadow rays should be used"
			" for the most realistic image.",
		/* [19] Sampling menu, Normal */
			"Choose an option from this menu to select the amount of sup"
			"ersampling that occurs during a render. Higher supersampling"
			" results in much better-looking pictures, but in slower rend"
			"ering times.",
		/* [20] View Menu, Normal */
			"Choose from this menu to change the stereoscopic setting. Y"
			"ou can render from the left eye’s view point, or from the ri"
			"ght eye’s view point. If Normal use used, no stereo offset t"
			"akes place. The amount of offset is determined by the Eye Se"
			"paration option.",
		/* [21] Shading Menu, Normal */
			"Choose an option from this menu to change the shading model"
			". The Strauss model is slower.",
		/* [22] Antialiasing Menu, Normal */
			"Choose an option from this menu to change the antialiasing "
			"mode. Antialiasing eliminates the “jaggies” in a picture. No"
			"rmal supersampling antialiasing should be used with non-zero"
			" focal aperatures.",
		/* [23] Options Dialog, Normal */
			"This window contains the rendering options for the scene cu"
			"rrently in memory. These options will be in effect the next "
			"time an image is rendered",
		/* [24] Default balloon for Preferences, Normal */
			"Use this window to change the preferences",
		/* [25] Okay, Normal */
			"Click here to accept these preferences and close this windo"
			"w.",
		/* [26] Allow background tasks, Normal */
			"Click here to turn off background tasks. This will slow ren"
			"dering, but will make it possible to use MacRTrace and run b"
			"ackground programs while rendering.",
		/* [27] Allow background tasks, Checked */
			"Click here to turn off background tasks. This will speed re"
			"ndering, but will make it impossible to do anything else unt"
			"il the render is complete.",
		/* [28] Show image as it is rendered, Normal */
			"Click here if you want the Image window to appear automatic"
			"ally each render.",
		/* [29] Show image as it is rendered, Checked */
			"Click here if you don’t want the Image window to appear aut"
			"omatically each render.",
		/* [30] Show Status window for each render, Normal */
			"Click here if you want the Status window to appear automati"
			"cally each render.",
		/* [31] Show Status window for each render, Checked */
			"Click here if you don’t want the Status window to appear au"
			"tomatically each render.",
		/* [32] Hide Options window during render, Normal */
			"Click here if you want the Options window to disappear auto"
			"matically each render.",
		/* [33] Hide Options window during render, Checked */
			"Click here if you don’t want the Options window to disappea"
			"r automatically each render.",
		/* [34] Show About window at program startup, Normal */
			"Click here if you want to see the About window each time yo"
			"u start MacRTrace.",
		/* [35] Show About window at program startup, Checked */
			"Click here if you don’t want to see the About window each t"
			"ime you start MacRTrace.",
		/* [36] Use Textures, Normal */
			"Click here if you want to use texture mapping during render"
			"s. Using textures slows rendering.",
		/* [37] Use Textures, Checked */
			"Click here if you don’t want to use texture mapping during "
			"renders. Using textures slows rendering.",
		/* [38] Textures inside objects in scene file, Normal */
			"Click here if textures are defined inside objects in scene "
			"files (.sff files).",
		/* [39] Textures inside objects in scene file, Grayed */
			"This option lets you specify that textures are defined insi"
			"de objects in scene files (.sff files). It is grayed now bec"
			"ause texures are disabled.",
		/* [40] Textures inside objects in scene file, Checked */
			"Textures are currently defined inside objects in scene file"
			"s (.sff files).",
		/* [41] Textures after objects in scene file, Normal */
			"Click here if textures are defined after objects in scene f"
			"iles (.sff files).",
		/* [42] Textures after objects in scene file, Grayed */
			"This option lets you specify that textures are defined afte"
			"r objects in scene files (.sff files). It is grayed now beca"
			"use texures are disabled.",
		/* [43] Textures after objects in scene file, Checked */
			"Textures are currently defined after objects in scene files"
			" (.sff files).",
		/* [44] Max Objects, Normal */
			"This is the maximum number of objects permitted in a scene.",
		/* [45] Max Lights, Normal */
			"This is the maximum number of lights permitted in a scene.",
		/* [46] Max Surfaces, Normal */
			"This is the maximum number of surfaces (surface characteris"
			"tics) permitted in a scene.",
		/* [47] Max CSG Objects, Normal */
			"This is the maximum depth of CSG objects permitted in a sce"
			"ne.",
		/* [48] Defaults, Normal */
			"Click here to change all options in this window to their de"
			"fault values.",
		/* [49] Revert, Normal */
			"Click here to revert to the preferences which were in effec"
			"t when this window was opened.",
		/* [50] Preferences Dialog, Normal */
			"Use this window to change the preferences."
	}
};

resource 'STR#' (5002, "Balloon Help String 3") {
	{
		/* [1] Default balloon for Status Dialog, Normal */
			"This window indicates what the program is doing and how muc"
			"h memory is available.",
		/* [2] Done reading sc…, Grayed */
			"This text indicates what MacRTrace is currently doing.",
		/* [3] Status Bar, Grayed */
			"This bar indicates the amount of time left in the current t"
			"ask. If the amount of time remaining is unknown, the bar jus"
			"t moves a dark strip across itself.",
		/* [4] Memory Bar, Grayed */
			"This bar indicates the amount of memory currently used by M"
			"acRTrace. When the entire bar is dark, no more memory is ava"
			"ilable. You can increase the amount of available memory by i"
			"ncreasing MacRTrace’s memory partition using Get Info... fro"
			"m the Finder.",
		/* [5] Log Window, Normal */
			"This window contains the low-level output of the ray tracin"
			"g engine."
	}
};

data 'TEXT' (5000, "Balloon Help String 4") {
		/* Render, Normal */
			"Click here to begin a render. If the button reads “Render,”"
			" the render will begin immediately, and a single image will "
			"be generated. If it reads “Render Next,” the render will beg"
			"in as soon as the scene has been read. If it reads “Render &"
			" Save,” an animation sequence will be rendered and saved to "
			"the disk."
};

data 'TEXT' (5001, "Balloon Help String 5") {
		/* Eye Separation Menu, Normal */
			"This menu affects the interpretation of the Eye Separation "
			"number. If “actual” is chosen, the number is the actual dist"
			"ance between eyes. If “% dist” is chosen, the number is a pe"
			"rcentage, and the distance between eyes is that percentage o"
			"f the distance from the Eye point to the Look point."
};

data 'TEXT' (5002, "Balloon Help String 6") {
		/* Eye Separation Menu, Grayed */
			"This menu affects the interpretation of the Eye Separation "
			"number. It is dimmed now because the View is Normal (no ster"
			"eo separation)."
};

data 'TEXT' (5003, "Balloon Help String 7") {
		/* Focal Distance, Normal */
			"This is the focal distance of the camera which is taking th"
			"e picture. To use the distance from the Eye point to the Loo"
			"k point as the focal distance, uncheck the checkbox to the l"
			"eft."
};

data 'TEXT' (5004, "Balloon Help String 8") {
		/* Focal Distance, Grayed */
			"This is used to specify the focal distance of the camera wh"
			"ich is taking the picture. It is not currently being used (a"
			"nd is dimmed) because the checkbox to the left is unchecked."
			" Instead,  the distance from the Eye point to the Look point"
			" is currently being used as the focal distance."
};

data 'TEXT' (5005, "Balloon Help String 9") {
		/* Correct texture normals, Normal */
			"Click here to turn on texture normal correction. When this "
			"option is on, MacRTrace corrects texture normals when textur"
			"es that modify normals are used, as they may sometimes creat"
			"e strange surface effects. This tends to happen if the scale"
			" of the normal perturbation is large."
};

data 'TEXT' (5006, "Balloon Help String 10") {
		/* Correct texture normals, Checked */
			"Click here to turn off texture normal correction. When this"
			" option is on, MacRTrace corrects texture normals when textu"
			"res that modify normals are used, as they may sometimes crea"
			"te strange surface effects. This tends to happen if the scal"
			"e of the normal perturbation is large."
};

data 'TEXT' (5007, "Balloon Help String 11") {
		/* Backfaces Menu, Normal */
			"Choose an option from this menu to control the removal of b"
			"ackface polygons. No Removal, is slower but always generates"
			" correct images. Full removal is faster but can result in in"
			"correct images. Partial Removal is a good compromise."
};

data 'TEXT' (5008, "Balloon Help String 12") {
		/* Walk Mode Menu, Normal */
			"Choose from this menu to change the pixel rendering order. "
			"With Serpent, the image is scanned top to bottom in a serpen"
			"tine manner. With Hilbert, pixels are scanned with a fractal"
			" walk, which takes more memory, but maximizes pixel coherenc"
			"y."
};

data 'TEXT' (5009, "Balloon Help String 13") {
		/* Time Between Backgrounds, Normal */
			"This is the number of ticks (1/60 second) between backgroun"
			"d tasks. Larger values result in faster rendering, but also "
			"make MacRTrace’s and background programs’ user interfaces mo"
			"re jerky and less responsive."
};

data 'TEXT' (5010, purgeable) {
			"This program creates photo-realistic images and animations "
			"of three-dimensional scenes."
};

resource 'hfdr' (-5696, purgeable) {	/* Help balloon for application icon */
	HelpMgrVersion, hmDefaultOptions, 0, 0, /* header information */
	{
	HMTEResItem { 5010 }
	}
};

