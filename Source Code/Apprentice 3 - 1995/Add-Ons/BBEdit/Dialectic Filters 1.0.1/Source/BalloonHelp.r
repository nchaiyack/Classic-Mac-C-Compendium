/* BalloonHelp.r
 */
 
#include <BalloonTypes.r>
#include <Types.r>

resource 'hdlg' (128, "Balloon Help 1", purgeable)
{
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,
	HMSkipItem {},
	{
		HMStringResItem { {0, 0}, {0, 0, 0, 0}, 128, 1, 0, 0, 0, 0, 0, 0 },
		HMStringResItem { {0, 0}, {0, 0, 0, 0}, 128, 2, 0, 0, 0, 0, 0, 0 },
		HMStringResItem { {0, 0}, {0, 0, 0, 0}, 128, 3, 0, 0, 0, 0, 0, 0 },
		HMStringResItem { {0, 0}, {0, 0, 0, 0}, 128, 4, 0, 0, 0, 0, 0, 0 },
		HMStringResItem { {1, 1}, {0, 0, 0, 0}, 0, 0, 128, 5, 0, 0, 0, 0 },
	}
};

resource 'hdlg' (129, "Balloon Help 2", purgeable)
{
	HelpMgrVersion,
	5,
	hmDefaultOptions,
	0,
	3,
	HMSkipItem {},
	{
		HMStringResItem { {8, 1}, {0, 0, 0, 0}, 128, 6, 0, 0, 128, 6, 0, 0 },
		HMStringResItem { {8, 1}, {0, 0, 0, 0}, 128, 7, 0, 0, 128, 7, 0, 0 },
		HMStringResItem { {8, 1}, {0, 0, 0, 0}, 128, 8, 0, 0, 128, 8, 0, 0 },
		HMStringResItem { {8, 1}, {0, 0, 0, 0}, 128, 9, 0, 0, 128, 9, 0, 0 },
		HMStringResItem { {8, 1}, {0, 0, 0, 0}, 128, 10, 0, 0, 128, 10, 0, 0 },
		HMStringResItem { {8, 1}, {0, 0, 0, 0}, 128, 11, 0, 0, 128, 11, 0, 0 },
		HMStringResItem { {8, 1}, {0, 0, 0, 0}, 128, 12, 0, 0, 128, 12, 0, 0 },
		HMStringResItem { {8, 1}, {0, 0, 0, 0}, 128, 13, 0, 0, 128, 13, 0, 0 },
		HMStringResItem { {8, 1}, {0, 0, 0, 0}, 128, 14, 0, 0, 128, 14, 0, 0 },
	}
};


resource 'STR#' (128, "Balloon Help Strings")
{
	{ 
		"Apply the filter to the selected text in your document.";
		"Close the dialog box without applying a filter, leaving your document unchanged.";
		"Display information about this version of the Dialectic Filters.";
		"Toggle balloon help on and off.";
		"This is just a separation line, silly. What did you think it was?";
		"English as spoken by the Swedish Chef on the Muppet Show. Bork! Bork! Bork!";
		"English as spoken by Elmer Fudd (“I’m hunting wabbits!”) in the Bugs Bunny cartoons.";
		"Middle English as found in the writings of Chaucer.";
		"A writing style founded in the cracker subculture of the 80s.";
		"Owhay oday ouyay escribeday Igpay Atinlay?";
		"Rotate the alphabet 13 positions to the right. Rot13 is used to encrypt and decrypt Usenet postings.";
		"The dots and dashes of telegraphic Morse code.";
		"English as it sounds when spoken underwater (don’t ask).";
		"The “secret” language you developed when you were 10 by putting ‘op’ before each vowel sound.";
	}
};


resource 'STR#' (129, "Error Messages")
{
	{
		"Dialectic Filters can only run if used with BBEdit (or BBEdit Lite) version 2.0 or later.";
		"Dialectic Filters cannot run on this kind of window. Try opening a regular document.";
		"Dialectic Filters can only run if you select some text first.";
		"Balloon Help is only available if you are running System 7.0 or later.";
		"Dialectic Filters could not get the window contents from BBEdit. Try restarting the application.";
		"Dialectic Filters could not allocate enough memory. Try closing some documents or quitting other running applications.";
		"A really weird internal inconsistency has occurred. Please contact the author and let him know you got this error message.";
	}
};


resource 'STR#' (130, "About Box Strings")
{
	{
		"Dialectic filter code written by Mark Pilgrim, Merrimac Software Group, ©1994.";
		"BBEdit Extension code and filter glue written by Leo Breebaart, Kronto Software, ©1994.";
		"This extension is freeware, as is the separately available source code. Feel free to send comments or questions to Leo Breebaart at the following Internet address: leo@cp.tn.tudelft.nl.";
	}
};
