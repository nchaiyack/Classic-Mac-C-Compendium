/*
Copyright � 1993,1994 by Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/


#define NumTextLines		2

resource 'ALRT' (130, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	130,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound2,
		/* [2] */
		OK, visible, sound2,
		/* [3] */
		OK, visible, sound2,
		/* [4] */
		OK, visible, sound2
	},
	alertPositionMainScreen
};

resource 'DITL' (130, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"A general error ^0 has occurred; I hope you have a list of "
			"error codes."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (131, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	131,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	alertPositionMainScreen
};

resource 'DITL' (131, purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"Quit"
		},
		/* [2] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"Sorry, this application needs System 7.0 or later."
		}
	}
};

#define NumTextLines		3

resource 'ALRT' (134, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	134,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound2,
		/* [2] */
		OK, visible, sound2,
		/* [3] */
		OK, visible, sound2,
		/* [4] */
		OK, visible, sound2
	},
	alertPositionMainScreen
};

resource 'DITL' (134, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"An error ^0 has occurred while processing an Apple Event; I "
			"hope you have a list of error codes."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (136, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	136,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound3,
		/* [2] */
		OK, visible, sound3,
		/* [3] */
		OK, visible, sound3,
		/* [4] */
		OK, visible, sound3
	},
	alertPositionMainScreen
};

resource 'DITL' (136, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"Quit"
		},
		/* [2] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"Sorry, an error ^0 has occurred while initializing my windows."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (137, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	137,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound3,
		/* [2] */
		OK, visible, sound3,
		/* [3] */
		OK, visible, sound3,
		/* [4] */
		OK, visible, sound3
	},
	alertPositionMainScreen
};

resource 'DITL' (137, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"Quit"
		},
		/* [2] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"Sorry, an error ^0 has occurred while installing the Apple Event handlers."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (500, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	500,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (500, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"Sorry, you cannot do this now. "
			"Please dismiss the dialog box first."
		}
	}
};

#define NumTextLines		4

resource 'ALRT' (501, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	501,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (501, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"Sorry, you cannot do this now. "
			"Please dismiss the dialog box first, "
			"or try dragging the item onto the frontmost window."
		}
	}
};

resource 'STR ' (320, purgeable) {
	"Fabrizio Oddone <gspnx@di.unito.it>"
};

resource 'STR ' (321, purgeable) {
	"http://www.di.unito.it/pub/WWW/www_student/apple/FabrizioOddone/"
};

resource 'STR#' (1200, purgeable) {
	{
		/* [1] e-mail */
			"Copies the author�s e-mail address; "
			"paste it in your favorite electronic mail application.",
		/* [2] WWW */
			"Copies the author�s World Wide Web Home Page address "
			"(in the Universal Resource Locator style). "
			"Use it with applications such as MacWeb, Mosaic, NetScape."
	}
};

