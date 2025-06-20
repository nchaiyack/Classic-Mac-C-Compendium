/*/
     Project Arashi: VAText.h
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 22:13
     Created: Friday, December 21, 1990, 20:17

     Copyright � 1990-1992, Juri Munkki
/*/

/*
**	Prototypes and defines for public vector animation kit routines.
**	These includes are for use with the vector text drawing routines.
*/

#define	MAXSTROKES	10

typedef	struct
{
	unsigned	char	a;			/*	First point of line stroke.			*/
	unsigned	char	b;			/*	Second endpoint.					*/
}	VAStroke;

typedef struct
{
	int			width;					/*	Width of character in gridunits		*/
	int			height;					/*	Height of character in gridunits	*/
	int			nStrokes;				/*	Number of strokes in mark			*/
	VAStroke	strokes[MAXSTROKES];	/*	Table of strokes for drawing mark	*/
}	VAMark;

typedef	int	VAMarkTranslator[256];
typedef	enum {	VALeft, VARight, VACenter, VAStarWars	}	VATextStyle;

typedef struct
{
	Fixed				hScale;
	Fixed				vScale;
	VAMark				**marks;
	VAMarkTranslator	*translation;
	VATextStyle			
}	