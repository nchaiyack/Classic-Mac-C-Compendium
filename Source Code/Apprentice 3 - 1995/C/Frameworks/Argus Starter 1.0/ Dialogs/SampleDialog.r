/**********************************************************************

    Segment: SampleDialogResources.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>
#include <BalloonTypes.r>

resource 'CNTL' (500, "Popup", purgeable) {
	{90, 40, 109, 282},
	popupTitleLeftJust,
	visible,
	80,  /* pixel width of title */
	500, /* MENU resource ID */
	popupMenuCDEFProc,
	0,   /* reference value */
	"Database:"
};

resource 'MENU' (500, "Popup Items") {
	500,  /* menu ID */
	textMenuProc,
	allEnabled,
	enabled,
	"Popup",
	{
		"Yellow Pages", noIcon, nokey, noMark, plain,
		"White Pages", noIcon, nokey, noMark, plain,
		"Personal Database", noIcon, nokey, noMark, plain
	}
};

resource 'DLOG' (500, "Sample Dialog", purgeable) {
	{46, 10, 195, 460},
	movableDBoxProc,
	invisible,
	noGoAway,
	0x0,
	500,
	"Sample Dialog"
};

resource 'DITL' (500, "Sample Dialog", purgeable) {
	{	/* array DITLarray: 11 elements */
		/* [1] */
		{119, 382, 139, 440},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{119, 311, 139, 369},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{20, 30, 36, 86},
		StaticText {
			disabled,
			"Name"
		},
		/* [4] */
		{50, 30, 66, 86},
		StaticText {
			disabled,
			"Phone"
		},
		/* [5] */
		{20, 90, 36, 288},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{50, 90, 66, 288},
		EditText {
			enabled,
			""
		},
		/* [7] */
		{20, 340, 38, 446},
		RadioButton {
			enabled,
			"Male"
		},
		/* [8] */
		{40, 340, 58, 446},
		RadioButton {
			enabled,
			"Female"
		},
		/* [9] */
		{60, 350, 76, 425},
		StaticText {
			disabled,
			"_______"
		},
		/* [10] */
		{80, 340, 98, 446},
		CheckBox {
			enabled,
			"Married"
		},
		/* [11] */
		{90, 40, 109, 282},
		Control {
			enabled,
			500
		},
		
		{0, 0, 0, 0},
		HelpItem {
		  disabled,
		  HMScanhdlg
		  {500}
		}
	}
};

// End of File