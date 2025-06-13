/*	Rez file for balloon document "VoicePicker Balloons" */
/*	Created Fri, Jun 23, 1995 at 18:39 */

#include "Types.r"
#include "BalloonTypes.r"

resource 'hdlg' (5000,"DITL 5000") {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing dialog items */
	{

		HMStringResItem {		/* item 1:OK */
			{0,0},
			{0,0,0,0},
			5001,1,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Cancel */
			{0,0},
			{0,0,0,0},
			5001,2,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 3:Voices Popup */
			{0,0},
			{0,0,0,0},
			0,0,
			0,0,
			5001,3,
			0,0
		},
		HMSkipItem { },			/* item 4:Message */
		HMStringResItem {		/* item 5:gender */
			{0,0},
			{0,0,0,0},
			0,0,
			5001,5,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 6:age */
			{0,0},
			{0,0,0,0},
			0,0,
			5001,6,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 7:Language */
			{0,0},
			{0,0,0,0},
			0,0,
			5001,7,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 8:Region */
			{0,0},
			{0,0,0,0},
			0,0,
			5001,8,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 9:Rate */
			{0,0},
			{0,0,0,0},
			0,0,
			5001,9,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 10:Pitch */
			{0,0},
			{0,0,0,0},
			0,0,
			5001,10,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 11:Rate Dial */
			{0,0},
			{0,0,0,0},
			5001,11,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 12:Pitch Dial */
			{0,0},
			{0,0,0,0},
			5001,12,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 13:Try */
			{0,0},
			{0,0,0,0},
			5001,13,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 14:Stop */
			{0,0},
			{0,0,0,0},
			5001,14,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 15:Sample */
			{0,0},
			{0,0,0,0},
			5001,15,
			0,0,
			0,0,
			0,0
		},
		HMSkipItem { },			/* item 16:Gender: */
		HMSkipItem { },			/* item 17:Age: */
		HMSkipItem { },			/* item 18:Language: */
		HMSkipItem { },			/* item 19:Region: */
		HMSkipItem { },			/* item 20:Rate: */
		HMSkipItem { },			/* item 21:Pitch: */
		HMSkipItem { }			/* item 22:DITLÉ Item 22 */
	}
};

resource 'STR#' (5001, "Balloon Help String 1") {
	{
		/* [1] OK, Normal */
			"Click here to close the dialog and accept the current voice"
			" settings.",
		/* [2] Cancel, Normal */
			"Click here to close the dialog without changing the voice s"
			"ettings.",
		/* [3] Voices Popup, Normal */
			"Choose one of the voices available on this computer.",
		/* [4] Voices Popup, Grayed */
			"There are no voices available on this computer.",
		/* [5] gender, Normal */
			"This is the gender of the current voice.\$0DVoices may be Ma"
			"le, Female and Neuter.",
		/* [6] age, Normal */
			"This is the age of the current voice.\$0D",
		/* [7] Language, Normal */
			"This is the language code of the current voice.",
		/* [8] Region, Normal */
			"This is the region code of the current voice.",
		/* [9] Rate, Normal */
			"This is the rate setting of the current voice.\$0DYou may ch"
			"ange this value with the dial on the right.",
		/* [10] Pitch, Normal */
			"This is the pitch setting of the current voice.\$0DYou may c"
			"hange this value with the dial on the right.",
		/* [11] Rate Dial, Normal */
			"Use this dial to change the rate of the current voice.",
		/* [12] Pitch Dial, Normal */
			"Use this dial to change the pitch of the current voice.",
		/* [13] Try, Normal */
			"Click this button to try out the current voice.",
		/* [14] Stop, Normal */
			"Click this button to stop speaking.",
		/* [15] Sample, Normal */
			"This is where the description of the current voice is displ"
			"ayed.\$0DYou may type in here some sample text."
	}
};
