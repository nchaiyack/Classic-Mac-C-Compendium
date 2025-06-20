/*
	Terminal 2.2
	"Script.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main2
#endif

#include "Interp.h"
#include "Text.h"
#include "Main.h"
#include "Document.h"
#include "Procedure.h"
#include "File.h"
#include "FormatStr.h"
#include "Port.h"
#include "Macros.h"
#include "Utilities.h"

/* ----- beep() -------------------------------------------------------- */

static INTEGER SI_beep(INTEGER params[])
{
#pragma unused(params)
	SysBeep(1);
	return 0;
}

/* ----- result = pause(timeout) --------------------------------------- */

static INTEGER SI_pause(INTEGER params[])
{
	return (INTEGER)Loop((long)params[0], 0, 0);
}

/* ----- result = prompt(string, timeout) ------------------------------ */

static INTEGER SI_prompt(INTEGER params[])
{
	register INTEGER result;
	register Byte *s = (Byte *)params[0];

	CtoPstr((char *)s);
	result = (INTEGER)Loop((long)params[1], s, 1);
	PtoCstr(s);
	return result;
}

/* ----- result = nextline(string, timeout) ---------------------------- */

static INTEGER SI_nextline(INTEGER params[])
{
	register INTEGER result;
	register Byte *s = (Byte *)params[0];

	result = (INTEGER)Loop((long)params[1], s, 2);
	PtoCstr(s);
	return result;
}

/* ----- result = type(template, ...) ---------------------------------- */

static INTEGER SI_type(INTEGER params[])
{
	register INTEGER result;
	Byte s[256];

	if (params[0]) {
		SFormatStr(s, (Byte *)params[0], &params[1]);
		result = Type(s);
	} else {			/* No formatting */
		CtoPstr((char *)params[1]);
		result = Type((Byte *)params[1]);
		PtoCstr((Byte *)params[1]);
	}
	return result;
}

/* ----- result = display(template, ...) ------------------------------- */

static INTEGER SI_display(INTEGER params[])
{
	register INTEGER result;
	Byte s[256];

	if (params[0]) {
		SFormatStr(s, (Byte *)params[0], &params[1]);
		result = Display(s);
	} else {			/* No formatting */
		CtoPstr((char *)params[1]);
		result = Display((Byte *)params[1]);
		PtoCstr((Byte *)params[1]);
	}
	return result;
}

/* ----- format(string, template, ...) --------------------------------- */

static INTEGER SI_format(INTEGER params[])
{
	SFormatStr((Byte *)params[0], (Byte *)params[1], &params[2]);
	PtoCstr((Byte *)params[0]);
	return 0;
}

/* ----- result = setup(baud,data,parity,stop,port,dtr,handshake) ------ */

static INTEGER SI_setup(INTEGER params[])
{
	INTEGER err;
	register INTEGER port = params[4];
	register Boolean newport;

	if (newport = (port != -1 && port != 0 && port != 1))
		CtoPstr((char *)port);
	else
		port = 0;
	err = (INTEGER)PortSetUp((short)params[0], (short)params[1],
		(short)params[2], (short)params[3], (Byte *)port,
		(short)params[5], (short)params[6]);
	if (newport)
		PtoCstr((Byte *)port);
	return err;
}

/* ----- terminal(lEcho, rEcho, autoLF, save) -------------------------- */

static INTEGER SI_terminal(INTEGER params[])
{
	return (INTEGER)TerminalSetup((short)params[0], (short)params[1],
		(short)params[2], (short)params[3]);
}

/* ----- lecho(OnOff) -------------------------------------------------- */

static INTEGER SI_lecho(INTEGER params[])
{
	return (INTEGER)TerminalSetup((short)params[0], -1, -1, -1);
}

/* ----- recho(OnOff) -------------------------------------------------- */

static INTEGER SI_recho(INTEGER params[])
{
	return (INTEGER)TerminalSetup(-1, (short)params[0], -1, -1);
}

/* ----- autolf(OnOff) ------------------------------------------------- */

static INTEGER SI_autolf(INTEGER params[])
{
	return (INTEGER)TerminalSetup(-1, -1, (short)params[0], -1);
}

/* ----- save(OnOff) --------------------------------------------------- */

static INTEGER SI_save(INTEGER params[])
{
	return (INTEGER)TerminalSetup(-1, -1, -1, (short)params[0]);
}

/* ----- result = send(name) ------------------------------------------- */

static INTEGER SI_send(INTEGER params[])
{
	register INTEGER result;
	register Byte name[256];
	short volume;
	long directory;
	OSType creator, type;
	long create, modif;
	short ref;

	/* Use default folder */

	Folder(&volume, &directory);
	CtoPstr((char *)params[0]);
	memcpy(name, (Byte *)params[0], *((Byte *)params[0]) + 1);
	PtoCstr((Byte *)params[0]);

	/* See if this is a TEXT file */

	if (result = (INTEGER)InfoFile(volume, directory, name,
			&creator, &type, &create, &modif))
		goto done;
	if (type != TEXT) {
		result = 3;
		goto done;
	}

	/* Open the file, send it */

	if (!(result = (INTEGER)OpenFile(volume, directory, name, &ref)))
		result = (INTEGER)SendTextFile(ref, name);

done:
	return result;
}

/* ----- result = download(name, binary, zmodem) ----------------------- */

static INTEGER SI_download(INTEGER params[])
{
	register INTEGER result;
	register Byte *name = (Byte *)params[0];

	if (name)
		CtoPstr((char *)name);
	result = Download(name, params[1], params[2]);
	if (name)
		PtoCstr((Byte *)name);
	return result;
}

/* ----- result = upload(name, binary, zmodem) ------------------------- */

static INTEGER SI_upload(INTEGER params[])
{
	register INTEGER result;

	CtoPstr((char *)params[0]);
	result = Upload((Byte *)params[0], params[1], params[2]);
	PtoCstr((Byte *)params[0]);
	return result;
}

/* ----- seconds = time() ---------------------------------------------- */

static INTEGER SI_time(INTEGER params[])
{
#pragma unused(params)
	return Time;
}

/* ----- date(seconds, &yr, &mo, &da, &ho, &mi, &se, &dw) -------------- */

static INTEGER SI_date(INTEGER params[])
{
	DateTimeRec r;

	Secs2Date(params[0], &r);
	*(long *)params[1] = r.year;
	*(long *)params[2] = r.month;
	*(long *)params[3] = r.day;
	*(long *)params[4] = r.hour;
	*(long *)params[5] = r.minute;
	*(long *)params[6] = r.second;
	*(long *)params[7] = r.dayOfWeek;
	return 0;
}

/* ----- string = new(size) -------------------------------------------- */

static INTEGER SI_new(INTEGER params[])
{
	return (INTEGER)NewPtrClear(params[0]);
}

/* ----- free(string) -------------------------------------------------- */

static INTEGER SI_free(INTEGER params[])
{
	DisposPtr((Ptr)params[0]);
	return 0;
}

/* ----- move(source, destination, count) ------------------------------ */

static INTEGER SI_move(INTEGER params[])
{
	memmove((void *)params[1], (void *)params[0], params[2]);
	return 0;
}

/* ----- result = strcmp(s1, s2) --------------------------------------- */

static INTEGER SI_strcmp(INTEGER params[])
{
	register Byte *s1 = (Byte *)params[0];
	register Byte *s2 = (Byte *)params[1];

	while (tolower(*s1) == tolower(*s2)) {
		if (!*s1)
			break;
		s1++;
		s2++;
	}
	return (INTEGER)(tolower(*s1) - tolower(*s2));
}

/* ----- result = catalog(i,name,&type,&dsize,&rsize,&cdate,&mdate) ---- */

static INTEGER SI_catalog(INTEGER params[])
{
	register short err;
	HParamBlockRec p;
	short volume;
	long directory;

	if (params[0] == 0)		/* File name is given */
		CtoPstr((char *)params[1]);
	Folder(&volume, &directory);
	memset(&p, 0, sizeof(p));
	p.ioParam.ioVRefNum = volume;
	p.fileParam.ioDirID = directory;
	p.ioParam.ioNamePtr = (StringPtr)params[1];	/* File name */
	p.fileParam.ioFDirIndex = params[0];		/* Index */
	err = PBHGetFInfo(&p, FALSE);
	PtoCstr((Byte *)params[1]);
	if (err)
		return err;
	*((long *)params[2]) = p.fileParam.ioFlFndrInfo.fdType;	/* Type */
	*((long *)params[3]) = p.fileParam.ioFlLgLen;	/* Data fork length */
	*((long *)params[4]) = p.fileParam.ioFlRLgLen;	/* Resource fork */
	*((long *)params[5]) = p.fileParam.ioFlCrDat;	/* Creation date */
	*((long *)params[6]) = p.fileParam.ioFlMdDat;	/* Modification date */
	return 0;
}

/* ----- result = getcts() --------------------------------------------- */

static INTEGER SI_getcts(INTEGER params[])
{
#pragma unused(params)
	return SerialCTS();
}

/* ----- result = getdcd() --------------------------------------------- */

static INTEGER SI_getdcd(INTEGER params[])
{
#pragma unused(params)
	return SerialDCD();
}

/* ----- setdtr(onoff) ------------------------------------------------- */

static INTEGER SI_setdtr(INTEGER params[])
{
	SerialDTR(params[0]);
	return 0;
}

/* ----- protocol(crc, k, batch, timeout, binary, b) ------------------- */

static INTEGER SI_protocol(INTEGER params[])
{
	return TransferSetup(params[0], params[1], params[2], params[3]);
}

/* ----- xyparms(crc, k, batch, timeout) ------------------------------- */

static INTEGER SI_xyparms(INTEGER params[])
{
	return XYModemSetup(params[0] != 0, params[1], params[2], params[3]);
}

/* ----- zparms(escctl,timeout,retries,buffer,packet,window,crcq) -----  */

static INTEGER SI_zparms(INTEGER params[])
{
	return ZModemSetup(params[0] != 0, params[1], params[2], params[3],
		params[4], params[5], params[6]);
}

/* ----- err = macrol(name) -------------------------------------------- */

static INTEGER SI_macrol(INTEGER params[])	/* Load macro file */
{
	register short err;

	CtoPstr((char *)params[0]);
	err = LoadMacros(Settings.scriptVolume, Settings.scriptDirectory,
		(Byte *)params[0]);
	PtoCstr((Byte *)params[0]);
	return err;
}

/* ----- err = macrox(i, op, name) ------------------------------------- */

static INTEGER SI_macrox(INTEGER params[])	/* Execute macro */
{
	switch (params[1]) {
		case 0:	/* Send */
		case 1:	/* Display */
			break;
		case 2:	/* Get name */
			NameMacro(params[0] + DOMACRO1, (Byte *)params[2]);
			PtoCstr((Byte *)params[2]);
		default:
		return 0;
	}
	return DoMacro(params[0] + DOMACRO1, params[1] != 0);
}

/* ----- text(prompt, ldelay, cdelay) ---------------------------------- */

static INTEGER SI_text(INTEGER params[])
{
	return TextsendSetup((params[0] < 0) ? 0 : (Byte *)params[0],
		params[1], params[2]);
}

/* ----- i = val(str) -------------------------------------------------- */

static INTEGER SI_val(INTEGER params[])	/* Convert string to integer */
{
	INTEGER i;

	CtoPstr((char *)params[0]);
	StringToNum((Byte *)params[0], &i);
	PtoCstr((Byte *)params[0]);
	return i;
}

/* ----- err = capture(onoff, name) ------------------------------------ */

static INTEGER SI_capture(INTEGER params[])	/* Capture file on/off */
{
	register DocumentPeek window = TerminalWindow;
	register short err;
	register Byte *name;
	short volume;
	long directory;
	short r;
	OSType creator, type;
	long create, modif;

	Folder(&volume, &directory);
	name = (Byte *)params[1];
	switch (params[0]) {
		case 0:				/* Capture off */
			if (!window->file)
				return 1;	/* Was not open */
			err = (window->length) ?
				FSWrite(window->file, &window->length, window->record) : 0;
			FSClose(window->file);
			FlushVol(0, window->volume);
			window->file = window->volume = 0;
			window->length = 0;
			SetItemStyle(GetMenu(FILE), CAPTURE, 0);
			return err;
		case 1:				/* Capture on, new file */
			if (window->file)
				return 2;	/* Still on */
			CtoPstr((char *)name);
			DeleteFile(volume, directory, name);
			if (!(err = CreateFile(volume, directory, name,
						Settings.textCreator, TEXT)) &&
					!(err = OpenFile(volume, directory, name, &r))) {
				window->file = r;
				window->volume = volume;
				SetItemStyle(GetMenu(FILE), CAPTURE, ACTIVE);
			}
			PtoCstr((Byte *)name);
			return err;
		case 2:				/* Capture on, append */
			if (window->file)
				return 2;	/* Still on */
			CtoPstr((char *)name);
			if (err = InfoFile(volume, directory, name,
					&creator, &type, &create, &modif)) {
				/* File does not yet exist */
				if ((err = CreateFile(volume, directory, name,
							Settings.textCreator, TEXT)) ||
						(err = OpenFile(volume, directory, name, &r)))
					goto back;
			} else {
				/* File already exists */
				if (type != TEXT) {
					err = 3;	/* Must be a TEXT file */
					goto back;
				}
				if (err = OpenFile(volume, directory, name, &r))
					goto back;
				if (err = SetFPos(r, fsFromLEOF, 0)) {
					FSClose(r);
					goto back;
				}
			}
			window->file = r;
			window->volume = volume;
			SetItemStyle(GetMenu(FILE), CAPTURE, ACTIVE);
back:
			PtoCstr(name);
			return err;
		default:			/* Illegal parameter */
			return 9;
	}
}

/* ----- err = nextbuff(char *buff, int count, int timeout) ------------ */

static INTEGER SI_nextbuff(INTEGER params[])
{
	return (INTEGER)LoopBuffer((long)params[2],(Byte *)params[0],params[1]);
}

/* ----- Script intrinsic functions ------------------------------------ */

INTRINSIC Intrinsics[] = {
	(Byte *)"autolf",	SI_autolf,
	(Byte *)"beep",		SI_beep,
	(Byte *)"capture",	SI_capture,
	(Byte *)"catalog",	SI_catalog,
	(Byte *)"date",		SI_date,
	(Byte *)"display",	SI_display,
	(Byte *)"download",	SI_download,
	(Byte *)"format",	SI_format,
	(Byte *)"free",		SI_free,
	(Byte *)"getcts",	SI_getcts,
	(Byte *)"getdcd",	SI_getdcd,
	(Byte *)"lecho",	SI_lecho,
	(Byte *)"macrol",	SI_macrol,
	(Byte *)"macrox",	SI_macrox,
	(Byte *)"move",		SI_move,
	(Byte *)"new",		SI_new,
	(Byte *)"nextbuff",	SI_nextbuff,
	(Byte *)"nextline",	SI_nextline,
	(Byte *)"pause",	SI_pause,
	(Byte *)"prompt",	SI_prompt,
	(Byte *)"protocol",	SI_protocol,
	(Byte *)"recho",	SI_recho,
	(Byte *)"save",		SI_save,
	(Byte *)"send",		SI_send,
	(Byte *)"setdtr",	SI_setdtr,
	(Byte *)"setup",	SI_setup,
	(Byte *)"stack",	SI_stack,		/* Defined in Interp.c */
	(Byte *)"strcmp",	SI_strcmp,
	(Byte *)"terminal",	SI_terminal,
	(Byte *)"text",		SI_text,
	(Byte *)"time",		SI_time,
	(Byte *)"type",		SI_type,
	(Byte *)"upload",	SI_upload,
	(Byte *)"val",		SI_val,
	(Byte *)"xyparms",	SI_xyparms,
	(Byte *)"zparms",	SI_zparms,
	0,			0
};
