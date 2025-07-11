/*
 * sed.r
 *
 * For GNU sed
 *
 * Franklin Chen
 * Wed, Jan 19, 1994	Version 2.03
 */

#include "SysTypes.r"
#include "Cmdo.r"

resource 'vers' (1, purgeable) {
	0x2,
	0x3,
	release,
	0x0,
	verUS,
	"2.03",
	"2.03, � FSF, Inc. 1989-93\nMPW port by Fr"
	"anklin Chen"
};

resource 'vers' (2, purgeable) {
	0x2,
	0x3,
	release,
	0x0,
	verUS,
	"2.03",
	"GNU for MPW"
};

resource 'cmdo' (128, "Sed") {
	{	/* array dialogs: 1 elements */
		/* [1] */
		289,
		"sed -- GNU sed is a batch stream editor.",
		{	/* array itemArray: 8 elements */
			/* [1] */
			NotDependent {

			},
			CheckOption {
				NotSet,
				{34, 349, 49, 461},
				"Help",
				"-h",
				"[--help] Print usage information on diagnostic output."
			},
			/* [2] */
			NotDependent {

			},
			CheckOption {
				NotSet,
				{51, 349, 65, 461},
				"Version",
				"-V",
				"[--version] Print version information on diagnostic output."
			},
			/* [3] */
			NotDependent {

			},
			CheckOption {
				NotSet,
				{71, 349, 85, 461},
				"Quiet",
				"-n",
				"[--quiet, --silent] No default output; don't write out a line"
				" unless an explicit print command is given."
			},
			/* [4] */
			NotDependent {

			},
			MultiRegularEntry {
				"Script",
				{125, 15, 145, 60},
				{98, 64, 174, 458},
				{	/* array DefEntryList: 0 elements */
				},
				"-e",
				"[--expression=script] Type in lines of a"
				" script to execute.  Enclose each line in quotes to"
				" protect from the shell."
			},
			/* [5] */
			NotDependent {

			},
			Files {
				InputFile,
				OptionalFile {
					{47, 42, 64, 115},
					{65, 34, 87, 121},
					"Script File",
					"",
					"-f ",
					"",
					"[--file=scriptfile] The name of a file c"
					"ontaining a sed script to compile and ex"
					"ecute.",
					dim,
					"Standard Input",
					"Select a script file�",
					""
				},
				Additional {
					"",
					"",
					"",
					"",
					{	/* array TypesArray: 1 elements */
						/* [1] */
						text
					}
				}
			},
			/* [6] */
			NotDependent {

			},
			MultiFiles {
				"Input Files�",
				"Select files to process.",
				{21, 20, 41, 137},
				"Input files to process:",
				"",
				MultiInputFiles {
					{	/* array MultiTypesArray: 1 elements */
						/* [1] */
						text
					},
					"",
					"",
					""
				}
			},
			/* [7] */
			NotDependent {

			},
			Redirection {
				StandardOutput,
				{14, 176}
			},
			/* [8] */
			NotDependent {

			},
			Redirection {
				DiagnosticOutput,
				{51, 176}
			}
		}
	}
};

