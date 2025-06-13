/*
 *
 * Japanese Text Code Converter
 * Between Network Code and Macintosh Native Code
 *
 * Copyright (c) 1993 by Shigeru Kanemoto
 *                   and HappySize Incorporated
 *
 *
 */

#include "tcv.h"
#include <assert.h>

#ifndef	ESC
#define	ESC	'\033'
#endif	/* ESC */
#ifndef	NUL
#define NUL	'\0'
#endif	/* NUL */

#ifndef	NULL
#define NULL	0
#endif	/* NUL */

#if	0
extern char	*malloc();
extern void	free();
#endif	/* 0 */

/*
 * According to the ISO 2022, I think it is permitted
 * to use space character code when Japanese double code
 * (or other 96^n type multiple byte code) is indicated to G0.
 *
 * However, almost all Japanese terminal do not permit to
 * use it.
 */
#define	UGLYDUCK

/*
 * Many interfaces like nntp needs that a text ends with
 * newline code. If a internal text ends with a double byte char
 * and a newline, the result of this conversion will be
 * double byte char, newline and indication string.
 * This may not work with nntp. Define RESET_ON_NL to
 * add a roman indication string on the end of each line.
 */
#define	RESET_ON_NL
/*
 *
 */
typedef unsigned char	byte;


/*
 *
 * Code table sectioning
 *
 */
#define	_R	1	/* Roman */
#define	_K	2	/* Japanese Katakana */
#define	_S	4	/* Shift JIS --- Macintosh Japanese Code */
#define	_2	8	/* Shift JIS 2nd byte */
#define	_T	0	/* Other should be unknown or through */

#ifdef	UGLYDUCK
#define	_SP	_R
#else	/* UGLYDUCK */
#define	_SP	_T
#endif	/* UGLYDUCK */

#ifdef	RESET_ON_NL
#define	_NL	_R
#else	/* RESET_ON_NL */
#define	_NL	_T
#endif	/* RESET_ON_NL */


static byte	secTable[256] = {
/*0*/	_T,    _T,    _T,    _T,    _T,    _T,    _T,    _T,
	_T,    _T,    _NL,   _T,    _T,    _NL,   _T,    _T,
/*1*/	_T,    _T,    _T,    _T,    _T,    _T,    _T,    _T,
	_T,    _T,    _T,    _T,    _T,    _T,    _T,    _T,
/*2*/	_SP,   _R,    _R,    _R,    _R,    _R,    _R,    _R,
	_R,    _R,    _R,    _R,    _R,    _R,    _R,    _R,
/*3*/	_R,    _R,    _R,    _R,    _R,    _R,    _R,    _R,
	_R,    _R,    _R,    _R,    _R,    _R,    _R,    _R,
/*4*/	_R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2,
	_R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2,
/*5*/	_R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2,
	_R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2,
/*6*/	_R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2,
	_R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2,
/*7*/	_R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2,
	_R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _R|_2, _T,
/*8*/	_T|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2,
	_S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2,
/*9*/	_S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2,
	_S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2,
/*A*/	_K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2,
	_K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2,
/*B*/	_K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2,
	_K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2,
/*C*/	_K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2,
	_K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2,
/*D*/	_K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2,
	_K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2, _K|_2,
/*E*/	_S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2,
	_S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2, _S|_2,
/*F*/	_T|_2, _T|_2, _T|_2, _T|_2, _T|_2, _T|_2, _T|_2, _T|_2,
	_T|_2, _T|_2, _T|_2, _T|_2, _T|_2, _T,    _T,    _T,
};

#define	isTH(c)		(secTable[c] == _T)
#define	isRO(c)		(secTable[c] & _R)
#define	isKT(c)		(secTable[c] & _K)
#define	isSJ(c)		(secTable[c] & _S)
#define	isSJ2(c)	(secTable[c] & _2)



/*
 *
 * Code set indication state machine
 *
 */

struct indicator {
    byte	ind1;	/* 1st char of indicator string after ESC */
    byte	ind2;	/* 2nd char */
    byte	ind3;	/* 3rd char if needed (Hungul?), or NUL */
    int		(*import)();
    int		(*export)();
};

extern int	im_roman(), im_kana(), im_kanji();
extern int	ex_roman(), ex_kana(), ex_kanji();

/* This array should be ordered as TCV_???? index above */
static struct indicator	indTable[] = {
    { 0x28, 0x42, NUL, im_roman, ex_roman },	/* TCV_USRO */
    { 0x28, 0x49, NUL, im_kana,  ex_kana  },	/* TCV_JPKT */
    { 0x28, 0x4a, NUL, im_roman, ex_roman },	/* TCV_JPRO */
    { 0x24, 0x40, NUL, im_kanji, ex_kanji },	/* TCV_JPK1 */
    { 0x24, 0x42, NUL, im_kanji, ex_kanji }	/* TCV_JPK2 */
};
#define	NUM_INDICATOR		(sizeof (indTable) / sizeof (struct indicator))
#define	DEF_INDICATOR	TCV_JPRO	/* first state of indication */



/*
 *
 * The state machine parts
 *
 */

/* Which code set is now indicated by the network code. */
static int
im_whichnext(pp)
    byte	**pp;
{
    int		i;
    struct indicator	*tp;

    for (i = 0, tp = &indTable[0]; i < NUM_INDICATOR; i++, tp++) {
	if ((*pp)[1] == tp->ind1 && (*pp)[2] == tp->ind2
	 && (tp->ind3 == NUL || (*pp)[3] == tp->ind3)) {
	    (*pp) += 1 + (tp->ind3 == NUL ? 2 : 3);	/* 1 for ESC */
	    return i;
	}
    }
    return TCV_UNKNOWN;	/* not found */
}

/* importing roman */
static int
im_roman(tcv, pp)
    TextCodeConverter	*tcv;
    byte		**pp;
{
    int			i, r;
    byte		c;

    if (pp == (byte **)NULL)
	return 0;

    while ((*pp)[0] != NUL) {
	c = (*pp)[0];
	if (c == ESC
	 && (i = im_whichnext(pp)) != TCV_UNKNOWN && i != tcv->index) {
	    tcv->index = i;
	    return 0;
	}

	if ((r = (*tcv->backend)(tcv, c, NUL)) < 0)
	    return r;
	(*pp)++;
    }
    return 0;
}

/* importing japanese katakana */
static int
im_kana(tcv, pp)
    TextCodeConverter	*tcv;
    byte		**pp;
{
    int			i, r;
    byte		c;

    if (pp == (byte **)NULL)
	return 0;

    while ((*pp)[0] != NUL) {
	c = (*pp)[0];
	if (c == ESC
	 && (i = im_whichnext(pp)) != TCV_UNKNOWN && i != tcv->index) {
	    tcv->index = i;
	    return 0;
	}

	if ((r = (*tcv->backend)(tcv, (isKT(c) ? c+0x80 : c), NUL)) < 0)
	    return r;
	(*pp)++;
    }
    return 0;
}

/* importing japanese kanji */
static int
im_kanji(tcv, pp)
    TextCodeConverter	*tcv;
    byte		**pp;
{
    int			i, r;
    byte		c, c2;

    if (pp == (byte **)NULL)
	return 0;

    while ((*pp)[0] != NUL) {
	c = (*pp)[0];
	if (c == ESC
	 && (i = im_whichnext(pp)) != TCV_UNKNOWN && i != tcv->index) {
	    tcv->index = i;
	    return 0;
	}

	c2 = (*pp)[1];
	if (isRO(c) && isRO(c2)) {
	    c |= 0x80;
	    c2 |= 0x80;
	    if (c & 0x01) {
		c = c / 2 + (c < 0xdf ? 0x31 : 0x71);
		c2 = c2 - (c2 >= 0xe0 ? 0x60 : 0x61);
	    } else {
		c = c / 2 + (c < 0xdf ? 0x30 : 0x70);
		c2 = c2 - 2;
	    }
	    (*pp)++;
	} else
	    c2 = NUL;

	if ((r = (*tcv->backend)(tcv, c, c2)) < 0)
	    return r;
	(*pp)++;
    }
    return 0;
}


/* Put appropriate indication sequence. */
void
ex_changenext(tcv)
    TextCodeConverter	*tcv;
{
    struct indicator	*tp;

    tp = &indTable[tcv->index];
    (*tcv->backend)(tcv, ESC, tp->ind1);
    (*tcv->backend)(tcv, tp->ind2, tp->ind3);
    tcv->lastindex = tcv->index;
}

/* exporting roman */
static int
ex_roman(tcv, pp)
    TextCodeConverter	*tcv;
    byte		**pp;
{
    byte		c;
    int			r;

    if (indTable[tcv->lastindex].export != ex_roman)
	ex_changenext(tcv);
    if (pp == (byte **)NULL)
	return 0;

    while ((*pp)[0] != NUL) {
	c = (*pp)[0];

	if (isKT(c)) {
	    tcv->index = TCV_JPKT;	/* katakana */
	    return 0;
	}
	if (isSJ(c)) {
	    tcv->index = TCV_JPK2;	/* kanji should use the newer code */
	    return 0;
	}
	/* RO and UN need no conversion */

	if ((r = (*tcv->backend)(tcv, c, NUL)) < 0)
	    return r;
	(*pp)++;
    }
    return 0;
}

static int
ex_kana(tcv, pp)
    TextCodeConverter	*tcv;
    byte		**pp;
{
    byte		c;
    int			r;

    if (indTable[tcv->lastindex].export != ex_kana)
	ex_changenext(tcv);
    if (pp == (byte **)NULL)
	return 0;

    while ((*pp)[0] != NUL) {
	c = (*pp)[0];

	if (isRO(c)) {
	    tcv->index = TCV_JPRO;	/* roman */
	    return 0;
	}
	if (isSJ(c)) {
	    tcv->index = TCV_JPK2;	/* kanji should use the newer code */
	    return 0;
	}
	if (isKT(c))
	    c -= 0x80;
	/* UN needs no conversion */

	if ((r = (*tcv->backend)(tcv, c, NUL)) < 0)
	    return r;
	(*pp)++;
    }
    return 0;
}

static int
ex_kanji(tcv, pp)
    TextCodeConverter	*tcv;
    byte		**pp;
{
    byte		c, c2;
    int			r;

    if (indTable[tcv->lastindex].export != ex_kanji)
	ex_changenext(tcv);
    if (pp == (byte **)NULL)
	return 0;

    while ((*pp)[0] != NUL) {
	c = (*pp)[0];

	if (isRO(c)) {
	    tcv->index = TCV_JPRO;	/* roman */
	    return 0;
	}
	if (isKT(c)) {
	    tcv->index = TCV_JPKT;	/* kanji should use the newer code */
	    return 0;
	}
	if (isSJ(c) && (c2 = (*pp)[1], isSJ2(c2))) {
	    if (c2 >= 0x9f) {
		c = (c*2 - (c >= 0xe0 ? 0xe0 : 0x60)) & 0x7f;
		c2 = c2 - 0x7e;
	    } else {
		c = (c*2 - (c >= 0xe0 ? 0xe1 : 0x61)) & 0x7f;
		c2 = (c2 + (c2 >= 0x7f ? 0x60 : 0x61)) & 0x7f;
	    }
	    (*pp)++;
	} else
	    c2 = NUL;
	/* UN needs no conversion */

	if ((r = (*tcv->backend)(tcv, c, c2)) < 0)
	    return r;
	(*pp)++;
    }
    return 0;
}



/*
 *
 * Default frontend and backend
 *
 */

static int	/* -1 if error */
defFrontend(tcv, pp)
    TextCodeConverter	*tcv;
    byte		**pp;
{
    *pp = (byte *)tcv->inData;
    tcv->inData = (TCVClientData)NULL;
    return 0;
}

static int	/* -1 if error */
defBackend(tcv, c1, c2)
    TextCodeConverter	*tcv;
    byte		c1, c2;
{
    byte		*p;

    p = (byte *)tcv->outData;
    *p++ = c1;
    if (c2 != NUL)
	*p++ = c2;
    tcv->outData = (TCVClientData)p;
    return 0;
}


/*
 *
 * Create a new converter
 *
 * dir: tcvImport to convert from Network code to Macintosh code
 *      tcvExport to convert from Macintosh code to Network code
 *
 * frontend, backend:
 *      pointer to function as below. NULL to use default functions.
 *
 * int (*frontend)(TextCodeConverter *tcv, char **pp)
 *      will return a porinter to a string to be converted in (*pp),
 *      will return 0 if no error, negative value if error.
 *
 * int (*backend)(TextCodeConverter *tcv, char c1, char c2)
 *      will output c1, and c2 if c2 is not '\0',
 *      will return 0 if no error, negative value if error.
 *
 */
TextCodeConverter *
TCVnew(dir, frontend, backend)
    TCVDirection	dir;
    int			(*frontend)();
    int			(*backend)();
{
    TextCodeConverter	*tcv;

    tcv = (TextCodeConverter *)malloc(sizeof (TextCodeConverter));
#if	0
    if (tcv == (TextCodeConverter *)NULL)
	panic();
#endif	/* 0 */

    tcv->index = DEF_INDICATOR;
    tcv->lastindex = DEF_INDICATOR;
    tcv->dir = dir;
    tcv->frontend = (frontend ? frontend : defFrontend);
    tcv->backend = (backend ? backend: defBackend);
    return tcv;
}

/*
 *
 * Destroy and free a converter
 *
 */
void
TCVdestroy(tcv)
    TextCodeConverter	*tcv;
{
    free((void *)tcv);
}


/*
 *
 * Start conversion
 * Reset the indication to the default
 *
 */
int
TCVstart(tcv)
    TextCodeConverter	*tcv;
{
    tcv->index = DEF_INDICATOR;
    return 0;
}

/*
 *
 * End conversion
 * Indicate to the default indication if needed
 *
 */
int
TCVend(tcv, inData, outData)
    TextCodeConverter	*tcv;
    TCVClientData	inData, outData;
{
    int			r;

    tcv->inData = inData;
    tcv->outData = outData;
    tcv->index = DEF_INDICATOR;

    if (tcv->dir == tcvImport)
	r = (*(indTable[DEF_INDICATOR]).import)(tcv, (byte **)NULL);
    else
	r = (*(indTable[DEF_INDICATOR]).export)(tcv, (byte **)NULL);
    return (r < 0 ? r : (*tcv->backend)(tcv, NUL, NUL));
}

/*
 *
 *
 * The main processing
 * The default frontend/backend does conversion
 * from string (char *)inData to (char *)outData.
 *
 *
 */
int
TCVdoit(tcv, inData, outData)
    TextCodeConverter	*tcv;
    TCVClientData	inData, outData;
{
    byte	*p;
    int		r;

    tcv->inData = inData;
    tcv->outData = outData;

    /* Just in case... */
    if (tcv->index < 0 || tcv->index >= NUM_INDICATOR)
	tcv->index = DEF_INDICATOR;

    if (tcv->dir == tcvImport) {
	for (;;) {
	    if ((r = (*tcv->frontend)(tcv, &p)) < 0)
		return r;
	    if (p == (byte *)NULL)
		break;

	    while (*p != NUL) {
		if ((r = (*indTable[tcv->index].import)(tcv, &p)) < 0)
		    return r;
		assert(tcv->index >= 0 && tcv->index < NUM_INDICATOR);
	    }
	}
    } else {
	for (;;) {
	    if ((r = (*tcv->frontend)(tcv, &p)) < 0)
		return r;
	    if (p == (byte *)NULL)
		break;

	    while (*p != NUL) {
		if ((r = (*indTable[tcv->index].export)(tcv, &p)) < 0)
		    return r;
		assert(tcv->index >= 0 && tcv->index < NUM_INDICATOR);
	    }
	}
    }
    return (*tcv->backend)(tcv, NUL, NUL);
}


#ifdef	TCV_TEST
#include <stdio.h>

/*
 * Test routine for unix
 */
void
main(ac, av)
    int		ac;
    char	**av;
{
//    char	*ibuf[500];
//    char	*obuf[500];
    char	ibuf[500];
    char	obuf[500];
    TextCodeConverter	*tcv;

    tcv = TCVnew((ac==2&&av[1][0]=='e' ? tcvExport : tcvImport), NULL, NULL);

    TCVstart(tcv);
    while (fgets(ibuf, sizeof (ibuf), stdin) != (char *)NULL) {
	if (TCVdoit(tcv, (TCVClientData)ibuf, (TCVClientData)obuf) < 0)
	    exit(1);
	fputs(obuf, stdout);
    }
    if (TCVend(tcv, (TCVClientData)ibuf, (TCVClientData)obuf) < 0)
	exit(1);
    fputs(obuf, stdout);

    exit(0);
}
#endif	/* TCV_TEST */

