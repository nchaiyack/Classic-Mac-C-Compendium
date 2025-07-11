/*
	$Workfile:   TSMHelper.r  $
	$Revision:   1.0  $

	Just standard run-of-the-mill resources for the app that handles our pallette.
	Nothing special here.

	� 1993 CE Software, Inc.  All rights reserved.

	WHEN	WHO		WHAT

�����
	
�����
*/

#include "types.r"
#include "systypes.r"

resource 'vers' (1) {
	0x01,0x00,development, 0x01, verUS,
	"v1.0d1",
	"TSMHelper v1.0d1\n�1993 CE Software, Inc."
};
	
type 'TSM+' AS 'STR ';

resource 'TSM+' (0) {
	"TSMHelper 1.0 �1993 CE Software, Inc."
};

resource 'BNDL' (128) { 'TSM+', 0, { 'ICN#', { 0, 128 }; 'FREF', { 0, 128 } } };

resource 'FREF' (128, purgeable) { 'INIT', 0, "" };

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"7FFF FFF0 8000 0008 8000 0008 8000 0008 81C0 1C08 8238 0208 8207 0208 8200 E208 A1C0 1C08 D000 000A 9000 000D 11F3 2209"
		$"1044 B601 1044 2A01 1043 2201 1040 A201 1044 A201 1043 2201 1000 0001 1000 0001 1000 0009 91C0 1C0D D238 020A A207 0208"
		$"8200 E208 81C0 1C08 8000 0008 8000 0008 8000 0008 8000 0008 8000 0008 7FFF FFF0",
		/* [2] */
		$"7FFF FFF0 FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8 DFFF FFFA 9FFF FFFF 1FFF FFFF"
		$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF 9FFF FFFF DFFF FFFA FFFF FFF8"
		$"FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8 7FFF FFF0"
	}
};

