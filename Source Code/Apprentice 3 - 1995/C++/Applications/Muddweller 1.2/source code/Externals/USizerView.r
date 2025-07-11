#ifndef __TYPES.R__
#include "Types.r"
#endif

#ifndef __MacAppTypes__
#include "MacAppTypes.r"
#endif

#define cSizeViews			2300

/* Cursor resource IDs  */
#define kVertSizingCursor	1300
#define kHorzSizingCursor	1301

resource 'cmnu' (1001) {
	1001,
	textMenuProc,
	allEnabled,
	enabled,
	"Buzzwords",
	{
/* [1] */	"Pane Resize", noIcon, noKey, noMark, plain, cSizeViews
	}
};

/* Vertical Pane Sizing Cursor */
resource 'CURS' (kVertSizingCursor) {
	$"0000 0000 0380 0380 0380 0BA0 1FF0 2388"
	$"4384 2388 1FF0 0BA0 0380 0380 0380 0000",
	$"0000 07C0 07C0 07C0 0FE0 1FF0 3FF8 7FFC"
	$"FFFE 7FFC 3FF8 1FF0 0FE0 07C0 07C0 07C0",
	{8, 7}
};

/* Horizontal Pane Sizing Cursor */
resource 'CURS' (kHorzSizingCursor) {
	$"00 00 00 00 01 00 02 80 04 40 0C 60 04 40 7F FC"
	$"7F FC 7F FC 04 40 0C 60 04 40 02 80 01",
	$"00 00 01 00 03 80 07 C0 0F E0 1F F0 FF FE FF FE"
	$"FF FE FF FE FF FE 1F F0 0F E0 07 C0 03 80 01",
	{8, 7}
};

