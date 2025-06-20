/*
Copyright � 1993,1994 by Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

#define NumTextLines		2

resource 'ALRT' (201, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	201,
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

resource 'DITL' (201, purgeable) {
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
			"You cannot save over an existing document; please try again."
		}
	}
};

#define NumTextLines		6

resource 'ALRT' (202, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	202,
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

resource 'DITL' (202, purgeable) {
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
			"The document �^0� has been successfully saved. "
			"You may print this document with SimpleText or your favorite "
			"text editing application."
		}
	}
};

#define NumTextLines		5

resource 'ALRT' (203, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	203,
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

resource 'DITL' (203, purgeable) {
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
			"The document �^0� has NOT been successfully saved, "
			"because the disk is full. "
			"Try saving onto another disk."
		}
	}
};

#define NumTextLines		5

resource 'ALRT' (204, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	204,
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

resource 'DITL' (204, purgeable) {
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
			"The document �^0� has NOT been successfully saved, "
			"because of an error ^1. "
			"Try saving onto another disk."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (205, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	205,
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

resource 'DITL' (205, purgeable) {
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
			"Could not open the document for writing. "
			"Try saving onto another disk."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (206, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	206,
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

resource 'DITL' (206, purgeable) {
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
			"Could not create the document. "
			"Try saving it with another name."
		}
	}
};

resource 'STR ' (300, purgeable) {
	"Save registration form as:"
};

resource 'STR ' (301, purgeable) {
	"Registration"
};

resource 'STR ' (302, purgeable) {
	"0123456789\$7F"
};

resource 'STR#' (1100, purgeable) {
	{
		/* [1] ok */
			"To save the registration form in a text document, click here.",
		/* [2] cancel */
			"To cancel, click here.",
		/* [3] Name */
			"Enter your full name.",
		/* [4] Company */
			"Enter your company name, if applicable.",
		/* [5] Address */
			"Enter your address.",
		/* [6] City */
			"Enter your city.",
		/* [7] State */
			"Enter your state.",
		/* [8] ZIP */
			"Enter your ZIP code.",
		/* [9] Country */
			"Enter your country name.",
		/* [10] e-mail */
			"Enter your electronic mail addresses. This is important because you can obtain "
			"upgrades from me via e-mail. "
			"(i.e. Internet, America Online, Compuserve, AppleLink, MCI Mail, etc.)",
		/* [11] Phone */
			"Enter your phone number, if you wish.",
		/* [12] FAX */
			"Enter your fax phone number, if you wish.",
		/* [13] copies */
			MACRO_COPIES,
		/* [14] from */
			"Enter how you heard of this software, and how you obtained it. "
			"(i.e. a friend, a Mac User Group, a Bulletin Board System, a CD-ROM, "
			"a magazine, an Internet site, etc.)",
		/* [15] suggestions */
			"Enter your comments about this software. Let me know your suggestions, problems, "
			"anything you could think of.",
		/* [16] price */
			"Here appears the sum due for this software. The more copies you "
			"buy, the less you pay per copy.",
	}
};

