/*
	Terminal 2.2
	"OtherOptions.c"
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

#define B_OK		1
#define B_CANCEL	2
#define E_TEXT_CR	3	/* Capture TEXT file creator */
#define E_BIN_TYPE	4	/* Non-Macbinary file type */
#define E_BIN_CR	5	/* ... and creator */
#define E_BACKSPACE	6	/* Code for backspace key */
#define E_ESCAPE	7	/* Code for ` key */
#define C_BEEP		8	/* Control-G beeps */
#define R_CONTROL	9	/* Control key = control */
#define R_OPTION	10	/* Control key = option */
#define R_COMMAND	11	/* Control key = command */
#define T_CONTROL	12	/* Control key frame */
#define U_CONTROL	13
#define U_TITLELINE	14	/* Underline */

/* ----- Convert creator/type to string -------------------------------- */

static void ToString(long num, register Byte *str)
{
	str[0] = 4;
	memcpy((void *)(str + 1), (void *)&num, 4);
}

/* ----- Convert string to creator/type -------------------------------- */

static long FromString(register Byte *str)
{
	register short i;
	long num;

	i = str[0];
	while(i < 4)
		str[++i] = ' ';
	memcpy((void *)&num, (void *)(str + 1), 4);
	return num;
}

/* ----- Other options dialog ------------------------------------------ */

void OtherOptions(void)
{
	register DialogPtr dialog;
	long num;
	short i;
	short number;
	Byte str[256];

	CenterDialog('DLOG', DLOG_OTHER);
	if (!(dialog = GetNewDialog(DLOG_OTHER, 0, (WindowPtr)-1L)))
		return;
	SetUserItem(dialog, U_CONTROL, (ProcPtr)DrawUserFrame);
	SetUserItem(dialog, U_TITLELINE, (ProcPtr)DrawUserLine);
	ToString(Settings.textCreator, str);
	SetEText(dialog, E_TEXT_CR, str);
	ToString(Settings.binType, str);
	SetEText(dialog, E_BIN_TYPE, str);
	ToString(Settings.binCreator, str);
	SetEText(dialog, E_BIN_CR, str);
	NumToString(Settings.backspace & 0x7F, str);
	SetEText(dialog, E_BACKSPACE, str);
	NumToString(Settings.escape & 0x7F, str);
	SetEText(dialog, E_ESCAPE, str);
	SelIText(dialog, E_TEXT_CR, 0, 32767);
	SetCheck(dialog, C_BEEP, Settings.beep);
	SetRadioButton(dialog, R_CONTROL, R_COMMAND, R_CONTROL+Settings.ctrl);
	ShowWindow(dialog);
	for (;;) {
		ModalDialog(OutlineFilter, &number);
		switch(number) {
			case B_OK:
				GetEText(dialog, E_TEXT_CR, str);
				num = FromString(str);
				if (num != Settings.textCreator) {
					Settings.textCreator = num;
					Settings.dirty = TRUE;
				}
				GetEText(dialog, E_BIN_TYPE, str);
				num = FromString(str);
				if (num != Settings.binType) {
					Settings.binType = num;
					Settings.dirty = TRUE;
				}
				GetEText(dialog, E_BIN_CR, str);
				num = FromString(str);
				if (num != Settings.binCreator) {
					Settings.binCreator = num;
					Settings.dirty = TRUE;
				}
				GetEText(dialog, E_BACKSPACE, str);
				StringToNum(str, &num);
				if (num < 0 || num > 128) {
					SysBeep(1);
					SelIText(dialog, E_BACKSPACE, 0, 32767);
					break;
				}
				if (num != Settings.backspace) {
					Settings.backspace = num;
					Settings.dirty = TRUE;
				}
				GetEText(dialog, E_ESCAPE, str);
				StringToNum(str, &num);
				if (num < 0 || num > 128) {
					SysBeep(1);
					SelIText(dialog, E_ESCAPE, 0, 32767);
					break;
				}
				if (num != Settings.escape) {
					Settings.escape = num;
					Settings.dirty = TRUE;
				}
				if ((i = GetCheck(dialog, C_BEEP)) != Settings.beep) {
					Settings.beep = i;
					Settings.dirty = TRUE;
				}
				if ((i = GetRadioButton(dialog, R_CONTROL, R_COMMAND)
						- R_CONTROL) != Settings.ctrl) {
					Settings.ctrl = i;
					Settings.dirty = TRUE;
				}
				/* fall thru */
			case B_CANCEL:
				DisposDialog(dialog);
				return;
			case C_BEEP:
				ToggleCheckBox(dialog, number);
				break;
			case R_CONTROL:
			case R_OPTION:
			case R_COMMAND:
				SetRadioButton(dialog, R_CONTROL, R_COMMAND, number);
				break;
		}
	}
}
