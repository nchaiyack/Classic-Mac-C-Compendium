/* RandomDotMain.h
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */

enum{
	kBetaDie = 128
};

#define NIL 0L
#define NOT !
#define kSkipGrayLine 	2
#define kScrollBarWidth	15
typedef long LongInt;
typedef short Integer;

/* ascii code definitions returned by macintosh special keys */
#define kHomeChar	1
#define kEnterChar	3
#define kEndChar	4
#define kHelpChar	5
#define kBackspaceChar	8
#define kPageUpChar	11
#define kPageDownChar	12
#define kReturnChar	13
#define kClearChar	27	/* clear and escape have same keycode */
#define kEscapeChar	27	/* clear and escape have same keycode */

/* ascii code definitions returned by arrow keys */
#define kLeftChar	28
#define kRightChar	29
#define kUpChar		30
#define kDownChar	31

/* Standard buttons : Dialogs.h defines these as all lower case., which
	is hard to read (makes them look like variables.) We make them look
	like constants.
 */
enum {
	kOK = 1,
	kCancel
};

/* kMainStrs
 */
enum {
	kPrefNameS = 1,
	kOpening1S,
	kOpening2S,
	kComputingStereogramS,
	kSaveImageS,
	kSaveStereoGramS,
	kOurHelpS
};

/* Preferences resources
 */
enum{
	kPortName = 128
};

/* convert unsigned byte to signed integer in range 0..255 */
#ifndef Length
#define Length(s)	((int) (unsigned int) ((s)[0]))
#endif /* Length */

extern StringPtr 	emptyS;
extern Integer		appResFile;		/* our own resID */
extern Integer		prefResFile;	/* preferences file resID */
extern FSSpec		prefSpec;		/* file spec for preferences file */
extern SysEnvRec	world;

void HandleEvents(void);
void DialogOnTopGoEvent(EventRecord *e);

