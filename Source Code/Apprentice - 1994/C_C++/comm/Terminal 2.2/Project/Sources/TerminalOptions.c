/*
	Terminal 2.2
	"TerminalOptions.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Options
#endif

#include "Options.h"
#include "Strings.h"
#include "Utilities.h"
#include "Text.h"
#include "Main.h"
#include "Procedure.h"
#include "MySF.h"
#include "Document.h"

#define B_OK			1		/* "Ok" button */
#define B_CANCEL		2		/* "Cancel" button */
#define C_SAVE			3		/* Save checkbox */
#define	C_AUTOLF		4		/* Auto LF checkbox */
#define C_ECHO_LOCAL	5		/* Local echo check box */
#define C_ECHO_REMOTE	6		/* Remote echo check box */
#define T_ECHO			7		/* Frame */
#define U_ECHO			8
#define C_START_ENABLE	9		/* Enable autostart script */
#define B_START_SELECT	10		/* Select autostart script */
#define T_START_PATH	11		/* Autostart script path name */
#define T_START			12		/* Frame */
#define U_START			13
#define U_TITLELINE		15		/* Underline */

/* ----- Terminal options dialog --------------------------------------- */

void TerminalOptions(void)
{
	register DialogPtr dialog;
	short number;
	Byte str[256];
	SFReply sfr;
	short volume;
	long directory;
	Byte name[32];

	CenterDialog('DLOG', DLOG_TERMINAL);
	if (!(dialog = GetNewDialog(DLOG_TERMINAL, 0, (WindowPtr)-1L)))
		return;
	SetUserItem(dialog, U_ECHO, (ProcPtr)DrawUserFrame);
	SetUserItem(dialog, U_START, (ProcPtr)DrawUserFrame);
	SetUserItem(dialog, U_TITLELINE, (ProcPtr)DrawUserLine);
	SetCheck(dialog, C_AUTOLF, Settings.autoLF);
	SetCheck(dialog, C_ECHO_LOCAL, Settings.localEcho);
	SetCheck(dialog, C_ECHO_REMOTE, Settings.echo);
	SetCheck(dialog, C_SAVE, Settings.save);
	volume = Settings.startVolume;
	directory = Settings.startDirectory;
	if (*Settings.startName) {
		SetCheck(dialog, C_START_ENABLE, TRUE);
		memcpy(name, Settings.startName, *Settings.startName + 1);
#ifdef FULLPATH
		Pathname(str, volume, directory);
#else
		*str = 0;
#endif
		Append(str, name);
	} else {
		SetCheck(dialog, C_START_ENABLE, FALSE);
		*name = *str = 0;
	}
	SetEText(dialog, T_START_PATH, str);
	ShowWindow(dialog);
	do {
		ModalDialog(OutlineFilter, &number);
		switch(number) {
			case B_OK:
				if (!GetCheck(dialog, C_START_ENABLE)) {
					volume = 0;
					directory = 0;
					*name = 0;
				}
				if (volume != Settings.startVolume ||
					directory != Settings.startDirectory ||
					!EqualString(name, Settings.startName, FALSE, FALSE)) {
					Settings.startVolume = volume;
					Settings.startDirectory = directory;
					memcpy(Settings.startName, name, *name + 1);
					VolumeId(Settings.startVName, &Settings.startVolume);
					Settings.dirty = TRUE;
				}
				TerminalSetup(GetCheck(dialog, C_ECHO_LOCAL),
					GetCheck(dialog, C_ECHO_REMOTE),
					GetCheck(dialog, C_AUTOLF),
					GetCheck(dialog, C_SAVE));
				break;
			case B_CANCEL:
				break;
			case B_START_SELECT:
				MySFGetFile(MyString(STR_G, G_SCRIPT),
					MyString(STR_G, G_SUFFIX), 1, &TEXT,
					&sfr, 0);
				RedrawDocument();
				if (sfr.good) {
					volume = -SFSaveDisk;
					directory = CurDirStore;
					memcpy(name, sfr.fName, *sfr.fName + 1);
#ifdef FULLPATH
					Pathname(str, volume, directory);
#else
					*str = 0;
#endif
					Append(str, name);
					SetEText(dialog, T_START_PATH, str);
					SetCheck(dialog, C_START_ENABLE, TRUE);
				}
				break;
			case C_SAVE:
			case C_AUTOLF:
			case C_ECHO_LOCAL:
			case C_ECHO_REMOTE:
			case C_START_ENABLE:
				ToggleCheckBox(dialog, number);
				break;
		}
	} while (number != B_OK && number != B_CANCEL);
	DisposDialog(dialog);
}
