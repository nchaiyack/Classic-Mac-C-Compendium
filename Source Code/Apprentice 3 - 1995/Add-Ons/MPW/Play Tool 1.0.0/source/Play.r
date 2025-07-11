/*
	Play.r - commando resource file

	Play file�
*/

#include	"Cmdo.r"
#include	"SysTypes.r"


resource 'cmdo' (128) {
	{
		245,			/* Height of dialog */
		"Plays a System 7 Sound file.",
		{
			Or {{-4}}, MultiFiles {
				"Sounds to play�",
				"Select the sounds to play.",
				{36, 35, 56, 200},
				"Sounds to play:",
				"",
				MultiInputFiles {
					{'sfil'},
					FilterTypes,
					"Only System 7 Sounds",
					"All files",
				}
			},
			Or {{-3}}, Redirection {
				StandardInput,
				{85, 30}
			},
			notDependent {}, Redirection {
				StandardOutput,
				{85, 180}
			},
			notDependent {}, Redirection {
				DiagnosticOutput,
				{85, 330}
			},
			notDependent {}, TextBox {
				gray,
				{80, 25, 125, 450},
				"Redirection"
			},
		}
	}
};

resource 'vers' (1) {
	0x1,
	0x0,
	release,
	0x0,
	verUS,
	"v1.0.0 �1994, G.E. Allen",
	"Play v1.0.0, a Tool for MPW\n�1994, Gregory E. Allen"
};

resource 'vers' (2) {
	0x1,
	0x0,
	release,
	0x0,
	verUS,
	"v1.0.0 <gallen@arlut.utexas.edu>",
	"v1.0.0 <gallen@arlut.utexas.edu>"
};
