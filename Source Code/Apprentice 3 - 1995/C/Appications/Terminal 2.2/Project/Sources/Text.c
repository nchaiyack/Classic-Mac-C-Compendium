/*
	Terminal 2.2
	"Text.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main2
#endif

#include "Text.h"

/* ----- Remove last character from text buffer ------------------------ */

Byte RemoveCharacter(register TextRecord *p)
{
	register Byte *buffer;
	register long new;
	register Byte c;

	buffer = p->text;
	new = p->newChar;

	if (p->length) {
		new = new ? new - 1 : p->size - 1;
		if (buffer[new] == '\015')
			return 0;				/* Cannot delete previous line */
		c = buffer[new];
		buffer[new] = '\015';
		p->newChar = new;
		--(p->length);
		return c;					/* OK, character deleted */
	}
	return 0;						/* Nothing to delete */
}

/* ----- Add new character to text buffer ------------------------------ */

void AddNewCharacter(
	register TextRecord *p,
	register Byte c)
{
	register Byte *buffer;
	register long first, new, max;

	buffer = p->text;
	first = p->firstChar;
	new = p->newChar;
	max = p->size;

	buffer[new++] = c;		/* Save character in buffer */
	if (new >= max)			/* Wrap around */
		new = 0;
	p->newChar = new;		/* Update new pointer for next character */
	if (c == '\015')			/* Cariage return */
		++(p->lines);		/* One more line */

	if (new == first) {		/* Biting in our tail */
		if (buffer[first] == '\015')
			--(p->lines);	/* One line less */
		++first;			/* We lost the oldest character */
		if (first >= max)	/* Wrap around */
			first = 0;
		p->firstChar = first;
	} else
		++(p->length);

	buffer[new++] = '\015';	/* Always keep CR here */
	if (new >= max)			/* Wrap around */
		new = 0;
	p->viewChar = new;		/* Offset to 1st character of new line */
	p->viewLine = p->lines;
}

/* ----- Find a line --------------------------------------------------- */

Boolean FindLine(
	register TextRecord *p,
	register long line,			/* Line number to look for */
	Byte *str)					/* String for result */
{
	register long offset;		/* Character offset of a character... */
	register long number;		/* ...in this line */
	register Byte *buffer;
	register Byte *s;
	register long max;
	register Byte c;
	Boolean lastLine;

	if (p->length == 0) {
		*str = 0;
		return TRUE;
	}
	buffer = p->text;
	max = p->size;
	lastLine = (line >= (p->lines - 1));

	/*
		Three line numbers with character offsets are known:
		(A)	line	0			at offset	p->firstChar
		(B)	line	p->viewLine	at offset	p->viewChar
		(C)	line	p->lines	at offset	(p->newChar + 1) % p->size

		So, first find out the one of those three line numbers
		that is the nearest to the line we are looking for.
	*/

	if (line > p->viewLine) {
		if (line > (p->viewLine + (p->lines - p->viewLine)/2)) {
			offset = p->newChar + 1;	/* (C) */
			if (offset >= max)
				offset = 0;
			number = p->lines;
		} else {
			offset = p->viewChar;		/* (B) */
			number = p->viewLine;
		}
	} else {
		if (line > p->viewLine/2) {
			offset = p->viewChar;		/* (B) */
			number = p->viewLine;
		} else {
			offset = p->firstChar;		/* (A) */
			number = 0;
		}
	}
	p->viewLine = line + 1;		/* Remember this for later */

	/* Find line */

	if (line >= number) {		/* Look forward to find line */
		line -= number;			/* Number of lines (CRs) to skip */
		while (line) {
			if (buffer[offset++] == '\015')
				line--;
			if (offset >= max)	/* Wrap around */
				offset = 0;
		}
	} else {					/* Look backwards to find line */
		line = number-line+1;	/* Number of lines (CRs) to skip */
		while (line) {
			if (offset == 0)	/* Wrap around */
				offset = max;
			if (buffer[--offset] == '\015')
				line--;
		}
		offset++;				/* We are at CR of previous line */
		if (offset >= max)		/* Wrap around */
			offset = 0;
	}

	/* Now copy line */

	s = str;
	number = 0;
	while ((c = buffer[offset++]) != '\015') {
		if (offset >= max)		/* Wrap around */
			offset = 0;
		*(++s) = c;
		++number;
	}
	if (offset >= max)			/* Wrap around */
		offset = 0;
	p->viewChar = offset;		/* Remember offset of next line */
	str[0] = number;
	return lastLine;
}
