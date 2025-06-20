/*
	Terminal 2.2
	"Text.h"
*/

typedef struct {
	Byte	*text;		/* Pointer to text buffer */
	long	size;		/* Size of text buffer */
	long	firstChar;	/* Offset of first character in text */
	long	newChar;	/* Offset of next character location */
	long	length;		/* Length of text */
	long	lines;		/* Number of lines in text */
	long	viewChar;	/* Offset of character corresponding... */
	long	viewLine;	/* ...to this line */
} TextRecord;

void AddNewCharacter (TextRecord *, Byte);
Boolean	FindLine (TextRecord *, long, Byte *);
Byte RemoveCharacter (TextRecord *);
