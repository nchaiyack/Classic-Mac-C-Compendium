/*****************************************************************************************************

	DialogUtils.h - A plethora of useful routines for dealing with dialogs.  This include some
					routines for easier access to dialog stuff, as well as a couple of
					generic dialogs that can be used in apps.
					
					- Remember to include a file called DialogUtils.rsrc in compiling this.
					
			©1994 Paul Rademacher
				  Dept. Of Computer Science
				  West Virginia University
				  paulr@cs.wvu.edu
			
			1/5/94

*****************************************************************************************************/

#ifndef _DIALOGUTILS_
#define _DIALOGUTILS_


/********************* Definitions *****************/

#ifndef _KEYCODES_
#define _KEYCODES_
#define kEnter				0x03
#define kTab				0x09
#define kBackspace			0x08
#define kReturn				0x0D
#define kLeftArrow			0x1C
#define kRightArrow			0x1D
#define kUpArrow			0x1E
#define kDownArrow			0x1F
#define kSpaceBar			0x31
#define	kShiftKey			0x0200
#define kEscape				27
#endif

#define rSaveChangesDLOG		200
#define rDualChoiceDLOG			201
#define rPromptForStringDLOG	202
#define rTextLengthALRT			203

#define rDialogUtilsText		200

#define kMoveToFront		(WindowPtr)-1L

#define kDisableControl		(short) 255
#define kEnableControl		0
#define kButtonPress		1

#define	kPushButton			ctrlItem + btnCtrl
#define kRadioButton		ctrlItem + radCtrl
#define kCheckmark			ctrlItem + chkCtrl

#ifndef kEntireText
#define kEntireText			32767	/* Used as maxSel in SelIText() */
#endif

#ifndef NOT
#define	NOT	!
#define AND	&&
#define OR	||
#endif

enum SaveChangesEnum
{
	dSaveChangesOK = 1,
	dSaveChangesCancel,
	dSaveChangesNo
};

enum DualChoiceEnum
{
	dDualChoiceOK = 1,
	dDualChoiceCancel,
	dDualChoiceText
};

enum PromptForStringEnum
{
	dPromptForStringOK = 1,
	dPromptForStringCancel,
	dPromptForStringPrompt,
	dPromptForStringText,
	dPromptForStringNumberOfItems
};


enum DialogUtilsTextEnum
{
	kDialogUtilsTextOK = 1,
	kDialogUtilsTextNumberOfItems
};

/****************** Prototypes *********************/

				/* Dialog Manager access routines */
				
Handle 		getDItemHandle(DialogPtr dlog, short item);
void		setDItemText( DialogPtr dlog, short item, Str255 text );
void		getDItemText( DialogPtr dlog, short item, StringPtr text );
short		getDItemValue( DialogPtr dlog, short item );
void		setDItemValue( DialogPtr dlog, short item, short value );
Rect		getDItemRect( DialogPtr dlog, short item );
short		getDItemType( DialogPtr dlog, short item );


				/* User-Interface shortcuts */

void		fakeMouseClick( DialogPtr dPtr, short itemNum );
void		highlightDefault( DialogPtr dPtr, short itemNum );
void		enableDItem( DialogPtr dlog, short item );
void		disableDItem( DialogPtr dlog, short item );
void		drawTextInUserItem( DialogPtr dlog, short item, StringPtr text );
void		drawDItemDottedBoxText( DialogPtr dlog, short item, StringPtr text );
void		drawDottedBoxText( Rect *rect, StringPtr text );
void		setAlternatingDefaultItems( DialogPtr dlog, short item );

			
				/* Generic dialogs */

short		doSaveChanges( StringPtr actionText, Str255 docName );
short		doDualChoice( const StringPtr prompt, const short defaultButton, const StringPtr okText );
short		promptForString( StringPtr prompt, StringPtr defaultText, short minLength, 
				short maxLength, StringPtr dest );
				
				
				/* Filter procs for the generic dialogs */
				
pascal Boolean	genericFilterProc( DialogPtr dlog,EventRecord *event,short *itemHit );

#endif

