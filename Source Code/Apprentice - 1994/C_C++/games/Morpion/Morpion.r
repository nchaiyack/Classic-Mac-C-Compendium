#define systemSevenOrLater	1
#include "Types.r"
#include "SysTypes.r"

// amount of room you have to work with in your fader dialog
#define areaHeight 				180
#define areaWidth 				310

// slider control resource
type 'SCTL' {
	integer=$$Countof(ValueStringPairs);
	Array ValueStringPairs {
		integer;	// display string if value >= this value
		pstring;	// string to display
	};
};

// screen depth preferences
#define noFlags		0x0000
#define colorDevice	0x0001
type 'DEEP' {
	integer = $$CountOf(DepthPreferencesList);
	array DepthPreferencesList {
		integer;			// preferred depth
		integer;			// whichFlags
		integer;			// flags
		};
};

#define squareWaveSynth	1
#define waveTableSynth	3
#define sampledSynth	5
type 'Chnl' {
	integer;
};

// dialog control list
type 'DCTL' {
	integer = $$CountOf(DCTLArray);
	array DCTLArray {
		integer;			// item number - zero for expressing cluster ranges
		integer;			// minValue for grouped items
		integer;			// maxValue for grouped items
		integer;			// setting index, 0-19 for shorts, 20-23 for strings
		};
};

// default settings
type 'DFLT' {
	array Shorts {
		integer;			// range of shorts
		};
		
	array Strings {
		pstring;
		};
};

type 'HELP' {
	string;
};

// --------------------------------------------------------------------------------------------------

// application name
resource 'STR ' (-16396, purgeable) {
	"DarkSide of the Mac 4.2"
};

// version info
resource 'vers' (1, purgeable) {
	0x01, 0x00, release, 0x00, verFrance,
	"1.0",
	"1.0, by François Pottier, 10/1993"
};

resource 'vers' (2, purgeable) {
	0x04, 0x02, release, 0x00, verUS,
	"4.2",
	"(for DarkSide of the Mac 4.2)"
};

resource 'DITL' (5000, purgeable) {
	{
	{ 5, 5, 20, areaWidth-5 },
		StaticText {enabled, "5 In A Row"},
		
	{ 5, areaWidth-33, 5+32, areaWidth-2 },
		Icon {enabled, 5000},
	
	{ 25, 15, 25+30, areaWidth-15 },
	 	Control {enabled, 5000+1},
	 	
 	{ 65, 15, 65+30, areaWidth-15 },
	 	Control {enabled, 5000+2},

	};
};

resource 'CNTL' (5001, purgeable) {
	{0, 0, 0+30, areaWidth-20},
	20,
	visible,
	60,
	0,
	16*200,
	5001,				// STR# ID of the names
	"Playing speed: "
};

resource 'CNTL' (5002, purgeable) {
	{0, 0, 0+30, areaWidth-20},
	16,
	visible,
	48,
	6,
	16*200,
	5002,				// STR# ID of the names
	"Square size: "
};

// strings filled in for the first control
resource 'SCTL' (5001) {
	{
	0,	"Painfully slow";
	10, "Very slow";
	20,	"Slow";
	30, "Rather slow";
	40,	"Normal";
	50, "Fast";
	60,	"Very fast";
	};
};

// strings filled in for the second control
resource 'SCTL' (5002) {
	{
	6,  "Tiny";
	12, "Small";
	16,	"Normal";
	24, "Large";
	36,	"Very large";
	48, "Huge";
	};
};

// resource to control the DITL above
resource 'DCTL' (5000) {
	{
	// item 1		about ID		ignored			ignored
	1,				5000,			0,				0;
	
	// item 2		about ID		ignored			ignored
	2, 				5000,			0, 				0;
	
	// item 3		res control		ignored			first short
	3,				3,				0,				0;
	
	// item 4		res control		ignored			second short
	4,				3,				0,				1;
	};
};

// default values for our fader
resource 'DFLT' (5000) {
	{40, 16},
	{};
};

resource 'HELP' (5000) {
	"5 In A Row, by François Pottier - 10/1993.\n\n"
	"Watch the computer play against itself!\n\n"
	"This fader was written in THINK C 5.0 and is placed into the public domain. "
	"It should help you create your own faders in THINK C."
};

resource 'Chnl' (0) {
	sampledSynth;
};
