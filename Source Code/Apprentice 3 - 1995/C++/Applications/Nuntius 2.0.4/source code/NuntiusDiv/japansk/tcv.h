#ifndef	_TCV_H
#define	_TCV_H

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

/* Client data such as string, file, network... */
typedef unsigned long	TCVClientData;

/* Code Conversion Direction */
typedef enum {
    tcvImport,		/* Import into Macintosh */
    tcvExport,		/* Export from Macintosh */
} TCVDirection;

/* State holder */
typedef struct {
    int			index;	/* index to the indicator indTable[] */
    int			lastindex;
    TCVDirection	dir;
    TCVClientData	inData, outData;
    int			(*frontend)();
	/*
	 * frontend(TextCodeConverter *tcv, char **pp)
	 */
    int			(*backend)();
	/*
	 * frontend(TextCodeConverter *tcv, char c1, char c2)
	 */
} TextCodeConverter;

/* Code set symbols */
#define	TCV_UNKNOWN	(-1)
#define	TCV_USRO	0	/* US Roman ISO 646 */
#define	TCV_JPKT	1	/* Japanese Katakana */
#define	TCV_JPRO	2	/* Japanese Roman */
#define	TCV_JPK1	3	/* Japanese Kanji JIS C 6226-1978 */
#define	TCV_JPK2	4	/* Japanese Kanji JIS X 0208-1983/90 */
/* This numbers are the index of indTable[] array below. */

#endif	/* _TCV_H */
