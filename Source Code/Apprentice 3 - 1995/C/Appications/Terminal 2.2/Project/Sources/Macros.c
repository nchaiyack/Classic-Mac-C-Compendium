/*
	Terminal 2.2
	"Macros.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main2
#endif

#include "Strings.h"
#include "Utilities.h"
#include "MySF.h"
#include "Text.h"
#include "Main.h"
#include "Document.h"
#include "Port.h"
#include "Macros.h"

#define MAXMACROS	10	/* Maximal number of macros */
#define MAXNAME		30	/* Maximal size of macro name */
#define MACRO_EOF	(-1)
#define MACRO_NAME	(-2)

typedef struct {
	union {
		long offset;	/* Relative offset to macro text */
		Byte *ptr;
	} x;
	long length;		/* Length of macro text */
} MINFO;

/*
	The global variable "MacrosText" is a handle to a structure in
	memory described as:

	struct {
		MINFO info[MAXMACROS];
		Byte data[...];
	}

	The handle is locked all the time. "info.offset" is the relative
	offset of the macros text, relative to the beginning of the
	structure. "info.length" is the length, in bytes, of the macro text.
	If a macro is not used its offset and length are set to zero.

	A complete set of macros (up to 10 macros) is stored in a macro file
	(TEXT file with ".m" suffix) in the following format:

	\M<macro name>
	<macro text>
	\M<macro name>
	<macro text>
	...

	The macro name must start with "\M" or "\m" and is terminated by the
	next return character. The macro text follows the macro name and is
	terminated by the beginning of the next macro name or by the end of
	the file. To include control characters into the macro text use a
	backslash followed by the character (upper and lower case are
	identical), e.g. control-C becomes "\C". You cannot use "\M" because
	this starts a new macro name, simply use the return character
	(control-M is the same as carriage return). To use the backslash
	itself use "\\". The last return character, the one just before the
	next macro name is not included in the macro text.
*/

/* ----- Static data --------------------------------------------------- */

static long Textsize;
static Byte *Text;

/* ----- Get next character -------------------------------------------- */

static short NextChar(void)
{
	register short b;

	if (Textsize <= 0)
		return MACRO_EOF;
	--Textsize;
	if ((b = *Text++) != '\\')
		return b;
	if (Textsize <= 0)
		return MACRO_EOF;
	--Textsize;
	b = *Text++;
	if (b >= 'a' && b <= 'z')
		b -= 'a' - 'A';
	switch (b) {
		case 'M':
			return MACRO_NAME;
		case '\\':
			return '\\';
	}
	return b - '@';
}

/* ----- Parse macros file --------------------------------------------- */

enum {
	WAITNAME,
	WAITEON,
	WAITEOT
};
	
static long ParseMacros(
	Byte *text,
	long size,
	register Byte *macros)		/* If 0 only calculate length */
{
	register MINFO *p;
	register Byte *q;
	Byte name[MAXNAME + 1];
	register short state;
	register short item;
	register short c;
	MenuHandle mh;

	Text = text;
	Textsize = size;
	p = (MINFO *)macros;
	if (macros) {
		if (!(mh = GetMenu(MACRO)))
			return 0;
		for (item = 0; item < MAXMACROS; ++item) {
			SetItem(mh, item + DOMACRO1, "\p ");
			DisableItem(mh, item + DOMACRO1);
			p[item].x.offset = 0;
			p[item].length = 0;
		}
	}
	q = (Byte *)(p + MAXMACROS) - 1;
	state = WAITNAME;
	item = 0;

	while ((c = NextChar()) != MACRO_EOF) {
		switch (state) {
			case WAITNAME:		/* Waiting for beginning of name */
				if (c == MACRO_NAME) {
					if (macros)
						*name = 0;	/* Prepare to make pascal string */
					state = WAITEON;
				}
				break;
			case WAITEON:		/* Waiting for end of name */
				switch (c) {
					case '\015':
						if (macros && item < MAXMACROS) {
							SetItem(mh, item + DOMACRO1, name);
							EnableItem(mh, item + DOMACRO1);
							p[item].x.ptr = q + 1;
						}
						state = WAITEOT;
						break;
					default:
						if (macros && *name < MAXNAME) {
							++(*name);
							name[*name] = c;
						}
				}
				break;
			case WAITEOT:		/* Waiting for end of macro text */
				switch (c) {
					case MACRO_NAME:
						if (macros && item < MAXMACROS) {
							*name = 0;	/* Prepare to make pascal string */
							if (q > p[item].x.ptr)
								p[item].length = q - p[item].x.ptr;
							else {
								p[item].length = 0;
								p[item].x.offset = 0;
								DisableItem(mh, item + DOMACRO1);
							}
							++item;
						}
						state = WAITEON;
						break;
					default:
						++q;
						if (macros && item < MAXMACROS)
							*q = c;
						break;
				}
		}
	}

	/* Last macro text */

	if (state == WAITEOT) {
		if (macros && item < MAXMACROS) {
			if (q > p[item].x.ptr)
				p[item].length = q - p[item].x.ptr;
			else {
				p[item].length = 0;
				p[item].x.offset = 0;
				DisableItem(mh, item + DOMACRO1);
			}
		}
	}

	/* Make relative offsets */

	if (macros)
		for (item = 0; item < MAXMACROS; ++item)
			if (p[item].x.offset)
				p[item].x.offset -= (long)macros;

	return q - macros + 1;	/* Add one spare byte at the end */
}

/* ----- Get macro name ------------------------------------------------ */

void NameMacro(
	register short item,
	register Byte *name)
{
	register MenuHandle mh;

	*name = 0;
	if (!(MacrosText && (mh = GetMenu(MACRO)) &&
			item >= DOMACRO1 && item < DOMACRO1 + MAXMACROS))
		return;
	GetItem(mh, item, name);
	return;
}

/* ----- Execute macro (or cancel macro) ------------------------------- */

short DoMacro(
	register short item,
	Boolean display)
{
	register Byte *buf;
	register long size;
	short err;

	{
		register MINFO *p;

		item -= DOMACRO1;
		if (item < 0 || item >= MAXMACROS)
			return FINE;
		p = (MINFO *)(*MacrosText);
		p += item;
		if (!p->x.offset || !p->x.offset)
			return FINE;
		buf = (Byte *)(*MacrosText) + p->x.offset;
		size = p->length;
	}

	if (display) {
		NewCharacters(buf, size, FALSE);
		return FINE;
	}

	if (Transfer)
		return FINE;
	if (Sending) {		/* Cancel by reselecting menu item */
		SerialAbort();
		Sending = FALSE;
		return FINE;
	}

	item += DOMACRO1;
	Sending = TRUE;
	Control_X = FALSE;
	SetItemStyle(GetMenu(MACRO), item, ACTIVE);
	/* SerialHandshake(Settings.handshake); */
	do {
		register Byte *buffer;
		register long count;
		Byte save;

		buffer = buf;
		if (size && Settings.chardelay) {
			++buf;
			--size;
		} else {
			while (size && *buf != '\015') {
				++buf;
				--size;
			}
			if (size) {
				++buf;
				--size;
			}
		}
		count = buf - buffer;
		if (!count)
			break;
		while (Busy)
			;
		save = *buf;	/* May be replaced by LF */
		if (buffer[count-1] == '\015') {
			if (Settings.autoLF)
				buffer[count++] = '\012';
			SerialSend(buffer, count, &Busy);
			if (Settings.localEcho)
				NewCharacters(buffer, count, FALSE);
			if (Settings.linedelay || Settings.prompt[0])
				Loop(Settings.linedelay,(Byte *)Settings.prompt,1);
			else
				CheckEvents();
		} else {
			SerialSend(buffer, count, &Busy);
			if (Settings.localEcho)
				NewCharacters(buffer, count, FALSE);
			if (Settings.chardelay)
				Loop(Settings.chardelay, 0, 0);
			else
				CheckEvents();
		}
		while (Busy)
			;
		*buf = save;
	} while (Sending);

	err = Sending ? FINE : (Control_X ? ABORT : CANCEL);
	SetItemStyle(GetMenu(MACRO), item, 0);
	Sending = FALSE;
	Control_X = FALSE;
	/* SerialHandshake(0); */
	return err;
}

/* ----- Load macros file ---------------------------------------------- */

short LoadMacros(
	short volume,
	long directory,
	Byte *name)		/* If NIL use dialog to select */
{
	register Handle h;
	short ref;
	short err;
	SFReply sfr;
	long size;

	/* Get file name if not yet specified, open the file */

	if (name)
		err = OpenFile(volume, directory, name, &ref);
	else {
		MySFGetFile(MyString(STR_G,G_MACRO), MyString(STR_G,G_MSUFFIX),
			1, &TEXT, &sfr, 0);
		RedrawDocument();
		if (!sfr.good)
			return noErr;
		err = OpenFile(sfr.vRefNum, 0, (Byte *)sfr.fName, &ref);
	}
	if (err)
		return err;

	/* Get rid of previous macros */

	if (MacrosText) {
		DisposHandle(MacrosText);
		MacrosText = 0;
	}

	/* Read new macros into memory and close the file */

	GetEOF(ref, &size);
	if (h = NewHandle(size)) {
		HLock(h);
		if (err = FSRead(ref, &size, *h))
			DisposHandle(h);
	} else
		err = memFullErr;
	FSClose(ref);
	if (err)
		return err;

	/* Parse the macros and save them */

	if (MacrosText = NewHandle(ParseMacros((Byte *)*h, size, 0))) {
		HLock(MacrosText);
		ParseMacros((Byte *)*h, size, (Byte *)*MacrosText);
		HUnlock(MacrosText);
		DisposHandle(h);
		MoveHHi(MacrosText);
		HLock(MacrosText);
		err = noErr;
	} else {
		DisposHandle(h);
		err = memFullErr;
	}
	return err;
}
