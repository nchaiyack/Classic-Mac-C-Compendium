/*
	Terminal 2.2
	"FormatStr.c"

	In the spirit of printf() but using pascal strings.

	Format specifiers begin with the character % and include zero or more
	of the following conversion specification elements:

	% [justify flag] [0 pad character] [field size] [type of variable]

	Justify flag (optional):
		-	Left justify output in field, pad on right (default is to
			right justify).
	Pad character (optional):
		0	Use zero rather than space (default) for the pad character.
	Type of variable:
		i	int
		l	long
		c	single character
		s	pascal string
		a	character array
		%	print a %, no argument used
		
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main2
#endif

Byte *FormatStr(Byte *string, Byte *template);
Byte *SFormatStr(Byte *string, Byte *template, long *param);

static void MoveBytes(
	register Byte *source,
	register Byte **destination,
	register Byte *limit,
	register short count)
{
	while (*destination <= limit && count--)
		*(*destination)++ = *source++;
}

static void MoveBlanks(
	register Byte **destination,
	register Byte *limit,
	register short count,
	register Byte pad)
{
	if (count <= 0)
		return;
	while (*destination <= limit && count--)
		*(*destination)++ = pad;
}

Byte *FormatStr(
	Byte *result,			/* Result string */
	Byte *template)			/* Template string */
{
	va_list ap;
	Byte *p = result + 1;
	Byte *max = template + *template;
	Byte num[30];
	short fieldWidth;
	Byte pad;
	Boolean leftJustify;

	va_start(ap, template);

	while (template < max) {
		template++;
		if (*template == '%') {
			template++;
			fieldWidth = 0;
			leftJustify = FALSE;
			pad = ' ';
			if (*template == '-') {
				template++;
				leftJustify = TRUE;
			}
			if (*template >= '0' && *template <= '9') {
				if (*template == '0')
					pad = '0';
				do
					fieldWidth = 10*fieldWidth + (*template++ & 0x0F);
				while (*template >= '0' && *template <= '9');
			}
			switch (*template) {
				case 'a':			/* Character array */
					MoveBytes(va_arg(ap, Byte *), &p, result + 255, fieldWidth);
					break;
				case 'c':			/* Character */
					{
						Byte c;

						if (!leftJustify)
							MoveBlanks(&p, result + 255, fieldWidth - 1, pad);
#ifdef applec
						/* In MPW C char is passed in 4 bytes on the stack! */
						c = va_arg(ap, long);
#else
						/* In THINK C char is passed in 2 bytes on the stack! */
						c = va_arg(ap, short);
#endif
						MoveBytes(&c, &p, result + 255, 1);
						if (leftJustify)
							MoveBlanks(&p, result + 255, fieldWidth - 1, pad);
						}
					break;
				case 's':			/* String */
					{
						register Byte *sp;

						sp = va_arg(ap, Byte *);
						if (!leftJustify)
							MoveBlanks(&p, result + 255, fieldWidth - *sp,
								pad);
						MoveBytes(sp + 1, &p, result + 255, *sp);
						if (leftJustify)
							MoveBlanks(&p, result + 255, fieldWidth - *sp,
								pad);
					}
					break;
				case 'i':			/* int */
#ifdef applec
					/* In MPW C short is passed in 4 bytes on the stack! */
#else
					NumToString(va_arg(ap, short), num);
					goto integer;
#endif
				case 'l':			/* long */
					NumToString(va_arg(ap, long), num);
				integer:
					if (!leftJustify)
						MoveBlanks(&p, result + 255, fieldWidth - *num,
							pad);
					MoveBytes(num + 1, &p, result + 255, *num);
					if (leftJustify)
						MoveBlanks(&p, result + 255, fieldWidth - *num,
							pad);
					break;
				default:
					*p++ = *template;
			}
		} else						/* Copy character */
			*p++ = *template;
	}
	va_end(pa);
	*result = p - result - 1;
	return result;
}

Byte *SFormatStr(				/* Special version for scripts */
	Byte *result,				/* Result string (Pascal string) */
	register Byte *template,	/* Template string (C-string) */
	long params[])				/* Array with parameters (char, long or C-string) */
{
	Byte *p = result + 1;
	register long *s = params;
	Byte num[30];
	short fieldWidth;
	Byte pad;
	Boolean leftJustify;

	while (*template) {
		if (*template == '%') {
			template++;
			if (!*template)
				break;
			fieldWidth = 0;
			leftJustify = FALSE;
			pad = ' ';
			if (*template == '-') {
				template++;
				if (!*template)
					break;
				leftJustify = TRUE;
			}
			if (*template >= '0' && *template <= '9') {
				if (*template == '0')
					pad = '0';
				do
					fieldWidth = 10*fieldWidth + (*template++ & 0x0F);
				while (*template >= '0' && *template <= '9');
				if (!*template)
					break;
			}
			switch (*template) {
				case 'c':			/* Character */
					if (!leftJustify)
						MoveBlanks(&p, result + 255, fieldWidth - 1, pad);
					MoveBytes((Byte *)s + 3, &p, result + 255, 1);
					if (leftJustify)
						MoveBlanks(&p, result + 255, fieldWidth - 1, pad);
					s++;
					break;
				case 's':			/* C-string */
					{
						register Byte *sp;

						sp = (Byte *)*s;
						CtoPstr((char *)sp);
						if (!leftJustify)
							MoveBlanks(&p, result + 255, fieldWidth - *sp,
								pad);
						MoveBytes(sp + 1, &p, result + 255, *sp);
						if (leftJustify)
							MoveBlanks(&p, result + 255, fieldWidth - *sp,
								pad);
						PtoCstr((Byte *)sp);
					}
					s++;
					break;
				case 'i':			/* int */
					NumToString(*s, num);
					s++;
					if (!leftJustify)
						MoveBlanks(&p, result + 255, fieldWidth - *num,
							pad);
					MoveBytes(num + 1, &p, result + 255, *num);
					if (leftJustify)
						MoveBlanks(&p, result + 255, fieldWidth - *num,
							pad);
					break;
				default:
					*p++ = *template;
			}
		} else						/* Copy character */
			*p++ = *template;
		template++;
	}
done:
	*result = p - result - 1;
	return result;
}
