/* RandomDotRes.h - this file is read both by the c program and by the Rez compiler.
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#define kCreator	'rs�D'
#define kPrefType	'rs�D'

#define kMBAR 128

/* menu id numbers
 */
#define kAppleMenu	128
#define kFileMenu	(kAppleMenu+1)
#define kEditMenu	(kFileMenu+1)
#define kRandomDotMenu	(kEditMenu+1)

/* dialog template id numbers
 */
#define rWin			128
#define	rAbout			(rWin+1)
#define rRandomDotCongig	(rAbout+1)
#define rError			(rRandomDotCongig+1)
#define rProgress		(rError+1)
#define rHelp			(rProgress+1)

/* STR# id numbers
 */
#define kMainStrs			128
#define kErrorStrs			(kMainStrs+1)
