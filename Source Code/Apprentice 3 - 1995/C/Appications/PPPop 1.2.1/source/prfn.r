type 'PRFN' { /* Preferences resource type */
	integer;	/* version number */
	integer;	/* window top */
	integer;	/* window left */
    integer;    /* HardClose = 1, SoftClose = 0 */
    integer;	/* Sound on = 1, off = 0 */
    integer;	/* Return to finder yes = 1, no = 0  */
    longint;	/* Cumulative Time */
    integer;	/* Show Timer on = 1, no = 0 */
    integer;	/* Show session time (insead of cumulative) yes = 1 */
    integer;	/* timer window top */
    integer;	/* timer window left */
    integer;	/* AutoPosition timer = 1 */
    longint;	/* last time cumulative time rest */
    integer;	/* radio button number for timer DLG */
    integer;	/* reset day of month */
    integer;	/* spare 2 */
    longint;	/* spare 3 */
};

resource 'PRFN' (128, purgeable) {
	0x0120,
    40, 40,
    1, 1, 0,
    0, 1, 1, 
    80, 40, 1,
    0,
    8,
    1,0,0
};

/* kIconHelpString = 500; */

resource 'hfdr' (-5696, purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	{HMSTRResItem {500}}
};

resource 'STR ' (500, purgeable) {
	"Use PPPop to open and close MacPPP."
};

/*
type 'kind' {
	literal longint;
	integer;
	integer = 0;
	integer = $$CountOf(kindArray);
	wide array kindArray {
		literal longint;
		pstring;
		align word;
	};
};

resource 'kind' (128) {
	'pppO',
	0,
	{
		'PREF', "PPPop Preferences File"
	}
};
*/