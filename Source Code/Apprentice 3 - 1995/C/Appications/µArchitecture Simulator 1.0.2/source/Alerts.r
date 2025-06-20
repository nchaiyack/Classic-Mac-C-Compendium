/*
Copyright � 1993,1994 by Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

#define SystemSevenOrLater 1

#include "SysTypes.r"
#include "Types.r"
#include "StdAlert.r"

#define NumTextLines		3

resource 'ALRT' (200, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	200,
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

resource 'DITL' (200, purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"Save"
		},
		/* [2] */
		{ButtonTop, CancelButtonLeft, ButtonBottom, CancelButtonRight},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{ButtonTop, DontSaveButtonLeft, ButtonBottom, DontSaveButtonRight},
		Button {
			enabled,
			"Don�t Save"
		},
		/* [4] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"Save changes to the microprogram document �^0�?"
		}
	}
};

#define NumTextLines		4
#define LongestButtonName	64    // Overwrite

resource 'ALRT' (129, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	129,
	{	/* array: 4 elements */
		/* [1] */
		Cancel, visible, sound1,
		/* [2] */
		Cancel, visible, sound1,
		/* [3] */
		Cancel, visible, sound1,
		/* [4] */
		Cancel, visible, sound1
	},
	alertPositionMainScreen
};

resource 'DITL' (129, purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"Overwrite"
		},
		/* [2] */
		{ButtonTop, CancelButtonLeft, ButtonBottom, CancelButtonRight},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"A safe save is not possible, because the available free spac"
			"e is less than the size of this document. Shall I overwrite "
			"the original document?"
		}
	}
};
#define LongestButtonName	LongestBtnName    // restore

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
			"A general error ^0 has occurred; I hope you have a list of e"
			"rror codes."
		}
	}
};

#define NumTextLines		3

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
			"Sorry, this program needs features currently available "
			"only under System 7.0 or later."
		}
	}
};

#define NumTextLines		4

resource 'ALRT' (132, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	132,
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

resource 'DITL' (132, purgeable) {
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
			"Sorry, it is not possible to save onto this disk without ove"
			"rwriting the original file (no more free space available). T"
			"o be safe, save onto another disk."
		}
	}
};

#define NumTextLines		3

resource 'ALRT' (133, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	133,
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

resource 'DITL' (133, purgeable) {
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

#define NumTextLines		4

resource 'ALRT' (514, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	514,
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

resource 'DITL' (514, purgeable) {
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
			"You would assemble this program outside of the simulator mem"
			"ory. Set the value in the Assembler Preferences dialog to a "
			"more reasonable one."
		}
	}
};

#define NumTextLines		1

resource 'ALRT' (512, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	512,
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

resource 'DITL' (512, purgeable) {
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
			"I have found an invalid opcode."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (513, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	513,
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

resource 'DITL' (513, purgeable) {
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
			"I have found an opcode not present in the opcode table."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (515, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	515,
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

resource 'DITL' (515, purgeable) {
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
			"I have found a symbol which is not defined."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (135, purgeable, preload) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	135,
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

resource 'DITL' (135, purgeable, preload) {
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
			"A stack underflow error occurred (in the simulated computer,"
			" fortunately)."
		}
	}
};

#define NumTextLines		3

resource 'ALRT' (136, purgeable, preload) {
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

resource 'DITL' (136, purgeable, preload) {
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
			"Timeout error. The simulated processor is abnormally busy, p"
			"ossibly because of an infinite loop in the microprogram."
		}
	}
};

#define NumTextLines		1

resource 'ALRT' (137, purgeable, preload) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	137,
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

resource 'DITL' (137, purgeable, preload) {
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
			"The simulated program has quitted."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (516, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	516,
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

resource 'DITL' (516, purgeable) {
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
			"Operand not valid. One or two letters are allowed between ' "
			"'."
		}
	}
};

#define NumTextLines		4

resource 'ALRT' (138, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	138,
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

resource 'DITL' (138, purgeable) {
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
			"Sorry, this volume does not support �safe save�. Please save"
			" onto another volume, or save the document with another name"
			"."
		}
	}
};

#define NumTextLines		3

resource 'ALRT' (139, purgeable, preload) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	139,
	{	/* array: 4 elements */
		/* [1] */
		Cancel, visible, silent,
		/* [2] */
		Cancel, visible, silent,
		/* [3] */
		Cancel, visible, silent,
		/* [4] */
		Cancel, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (139, purgeable, preload) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"Reset"
		},
		/* [2] */
		{ButtonTop, CancelButtonLeft, ButtonBottom, CancelButtonRight},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"Do you really want to reset the Program Counter and the Stac"
			"k Pointer to their initial values? This is NOT undoable."
		}
	}
};

#define NumTextLines		2

resource 'ALRT' (140, purgeable, preload) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	140,
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

resource 'DITL' (140, purgeable, preload) {
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
			"A stack overflow error occurred (in the simulated computer, "
			"fortunately)."
		}
	}
};

#define NumTextLines		3

resource 'ALRT' (141, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	141,
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

resource 'DITL' (141, purgeable) {
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
			"Sorry, memory is full. You will find some recovered document"
			"s in the Trash after restarting your Mac."
		}
	}
};

#define NumTextLines		3

resource 'ALRT' (128, purgeable, preload) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	128,
	{	/* array: 4 elements */
		/* [1] */
		Cancel, visible, silent,
		/* [2] */
		Cancel, visible, silent,
		/* [3] */
		Cancel, visible, silent,
		/* [4] */
		Cancel, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (128, purgeable, preload) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"Reset"
		},
		/* [2] */
		{ButtonTop, CancelButtonLeft, ButtonBottom, CancelButtonRight},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"Do you really want to reset the simulator main memory to its"
			" initial state? This is NOT undoable."
		}
	}
};

#define NumTextLines		3

resource 'ALRT' (142, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	142,
	{	/* array: 4 elements */
		/* [1] */
		Cancel, visible, silent,
		/* [2] */
		Cancel, visible, silent,
		/* [3] */
		Cancel, visible, silent,
		/* [4] */
		Cancel, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (142, purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"Revert"
		},
		/* [2] */
		{ButtonTop, CancelButtonLeft, ButtonBottom, CancelButtonRight},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"Do you really want to revert to the last saved version of th"
			"is document? This is NOT undoable."
		}
	}
};

#define NumTextLines		3

resource 'ALRT' (143, purgeable) {
	{AlertTop, AlertLeft, AlertBottom, AlertRight},
	143,
	{	/* array: 4 elements */
		/* [1] */
		Cancel, visible, silent,
		/* [2] */
		Cancel, visible, silent,
		/* [3] */
		Cancel, visible, silent,
		/* [4] */
		Cancel, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (143, purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{ButtonTop, ActionButtonLeft, ButtonBottom, ActionButtonRight},
		Button {
			enabled,
			"Quit"
		},
		/* [2] */
		{ButtonTop, CancelButtonLeft, ButtonBottom, CancelButtonRight},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{TextTop, TextLeft, TextBottom, TextRight},
		StaticText {
			disabled,
			"A computation is running on the simulated processor. Do you really want to quit now?"
		}
	}
};

