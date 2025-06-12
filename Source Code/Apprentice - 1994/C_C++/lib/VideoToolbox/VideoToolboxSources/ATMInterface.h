/*
 * ATMInterface.h
 *
 * Version 1.01
 *
 * Adobe Type Manager is a trademark 
 * of Adobe Systems Incorporated.
 */

#define int16 short
#define int32 long

#define _H_ATMInterface

#include <Devices.h>
#include <Types.h>
typedef struct
	{
	Fixed a, b, c, d, tx, ty;
	} FixedMatrix;

#define ATMProcs3Version 3
typedef struct
	{
	long version;
	int16 (*fontAvailable)(int16 family,
		 int16 style);
	int16 (*showText)(char *text, 
		int16 length, FixedMatrix *matrix);
	int16 (*xyshowText)(char *text, 
		int16 length, FixedMatrix *matrix,
		Fixed *displacements);
	} ATMProcs3;

#define ATMProcsStatusCode 0

/*
	initATM initializes ATMInterface, returns 
	1 if and only if ATM is available and
	the procsets are correctly initialized
*/
int16 initATM(void);

/* 
	fontAvailableATM returns 1 if and 
	only if ATM can image the specified 
	family and style 
*/
int16 fontAvailableATM(int16 family, 
	int16 style);

/* 
	showTextATM shows length characters
	starting at text transformed by 
	the specified matrix 
	Returns the number of characters not shown */
/* 	Matrix maps one point character space to device space, */
/* relative to current pen position */
/* Matrix's tx and ty components are updated */
int16 showTextATM(char *text, int16 length, 
	FixedMatrix *matrix);

/*
	Show length characters starting 
	at text transformed by the specified 
	matrix.
	Matrix maps one point character space 
	to device space, relative to current pen position.
	Matrix's tx and ty components are updated.
	Character x and y widths are specified 
	by displacements.
	Returns the number of characters not shown.
*/
int16 xyshowTextATM(char *text, int16 length, 
	FixedMatrix *matrix, Fixed *displacements);
/* 
*	end listing of ATMInterface.h 
*
*/
