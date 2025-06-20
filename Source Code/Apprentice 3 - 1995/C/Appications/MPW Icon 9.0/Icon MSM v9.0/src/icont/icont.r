/*
    Commando dialog definition for the Icon Programming Language
	translator and linker (icont).
*/

#include "Cmdo.r"
#include "Types.r"

resource 'cmdo' (128) {
	{
		295,
		"Icont � Icon Programming Language Translator & Linker.  "
		"Clicking on any control will display a help message in this box.",
		{
			NotDependent {},
			MultiFiles {
				"Input File(s)�",
				"Choose the source (.icn) & icode (.u1) files you "
				"wish to include.",
				{30, 76, 54, 184},
				"Source & Ucode Files:",
				"",
				MultiInputFiles {
					{
						/*text*/  /* .u? files are not TEXT */
					},
					".icn",
					"Files ending in .icn",
					/*"All text files"*/
					"All files"
				}
			},
			Or {
				{1}
			},
			Files {
				InputOrOutputFile,
				OptionalFile {
					{117, 261, 133, 429},
					{133, 261, 152, 429},
					"Icode File Name",
					"Root name of 1st file",
					"-o",
					" (root name)",
					"This is the name to give to the executable "
					"program (icode file).  The default is the root "
					"portion of the first source or ucode file specified.",
					dim,
					"Default icode file",
					"Select existing icode file�",
					"Create new icode file�"
				},
				Additional {
					"-o",
					"",
					"",
					"",
					{	/* array TypesArray */
						text
					}
				}
			},
			NotDependent {},
			Redirection {
				StandardInput,
				{30, 210}
			},
			NotDependent {},
			Redirection {
				StandardOutput,
				{30, 338}
			},
			NotDependent {},
			Redirection {
				DiagnosticOutput,
				{68, 274}
			},
			NotDependent {},
			TextBox {
				gray,
				{20, 200, 112, 460},
				"Input and Output Redirection"
			},
			NotDependent {},
			CheckOption {
				NotSet,
				{68, 24, 84, 190},
				"Translate only",
				"-c",
				"Translate source files to create ucode files only "
				"� do not link."
			},
			NotDependent {},
			CheckOption {
				NotSet,
				{88, 24, 104, 190},
				"Suppress messages",
				"-s",
				"Suppress output of non-error, informative messages."
			},
			NotDependent {},
			CheckOption {
				NotSet,
				{108, 24, 124, 190},
				"Trace",
				"-t",
				"Give keyword &trace an initial value of -1 when "
				"program is executed."
			},
			NotDependent {},
			CheckOption {
				NotSet,
				{128, 24, 144, 240},
				"Warn undeclared",
				"-u",
				"Issue warning messages for undeclared identifiers."
			},
			NotDependent {},
			CheckOption {
				NotSet,
				{148, 24, 164, 190},
				"Preprocess only",
				"-E",
				"Write preprocessed source to standard output without "
				"translating."
			},
			NotDependent {},
			CheckOption {
				NotSet,
				{168, 24, 184, 190},
				"Keep unreferenced",
				"-fs",
				"Prevent the removal of all unreferenced declarations "
				"(equivalent to \"invocable all\")."
			},
			NotDependent {},
			RadioButtons {
				{
					{170, 230, 182, 260}, "1", "-v 1", NotSet,
							"Supress non-error output (same as -s).",
					{170, 284, 182, 314}, "2", "", Set,
							"List procedure names (the default).",
					{170, 338, 182, 368}, "3", "-v 3", NotSet,
							"Report the sizes of the icode sections "
							"(procedures, strings, and so forth).",
					{170, 392, 182, 422}, "4", "-v 4", NotSet,
							"Same as -v 3 plus list discarded globals."
				}
			},
			NotDependent {},
			TextBox {
				gray,
				{162, 200, 186, 460},
				"Verbosity of Output"
			},
			NotDependent {},
			PictOrIcon {
				Icon, 128, {24, 24, 56, 56},
			},
			NotDependent {},
			Box {
				gray,
				{18, 18, 62, 62}
			}
		},
	}
};


/*
   This is the ICON resource that creates the icon for the icon logo
   for the Icon Programming Language.
*/

resource 'ICON' (128) {
	$"0003 0000 000C 0000 001C 0000 0038 0000"
	$"0033 8000 0006 C000 000C 6000 0006 0000"
	$"0033 1800 0079 3C00 00CC 6630 00CC 6638"
	$"0278 3C9C 0630 19CC 0C00 0362 88C0 0622"
	$"8D80 0060 6730 18C0 7278 3C80 38CC 6600"
	$"18CC 6600 0079 3C00 0031 9800 0000 C000"
	$"000C 6000 0006 C000 0003 9800 0000 3800"
	$"0000 7000 0000 6000 0001 80"
};
