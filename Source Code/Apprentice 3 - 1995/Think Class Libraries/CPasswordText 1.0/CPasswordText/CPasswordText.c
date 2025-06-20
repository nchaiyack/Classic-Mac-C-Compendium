/******************************************************************************
 CPasswordText.c

	A subclass of CDialogText for entering hidden password text. Password 
	text is limited to 100 characters currently, but there is no limitation
	checking. This class replaced TextEdit's draw hook with one that 
	draws bullets.
	
    AUTHOR: Andrew_Gilmartin@Brown.Edu
	REVISION: 2

******************************************************************************/

#include "CPasswordText.h"


/******************************************************************************
 IPasswordText
 
 	Initialize a CPasswordText object. The user's password is hidden by
 	displaying a bullet for each character. This is done by replacing
 	TextEdit draw hook.
******************************************************************************/

void CPasswordText::IPasswordText
	( CView *anEnclosure
	, CView *aSupervisor
	, short aWidth
	, short aHeight
	, short aHEncl
	, short aVEncl
	, SizingOption aHSizing
	, SizingOption aVSizing
	, short aLineWidth )
{
	/*
		Since I am only replacing the draw hook, we need to fake out
		text edit so that its own width and hit testing calls find the
		right characters. Since the only displayed character is a 
		bullet TE should think all characters have the bullet's 
		width. I use a monospace font to accumplish this.
	*/

	TextFont( monaco ); // Perhaps I should have a RestoreEnvironment() method...

	IDialogText
		( anEnclosure
		, aSupervisor
		, aWidth
		, aHeight
		, aHEncl
		, aVEncl
		, aHSizing
		, aVSizing
		, aLineWidth );
	
    InstallTEHooks();

} /* IPasswordText */



/******************************************************************************
 IViewTemp
 
 	Initialize a CPasswordText object from resource template (DlTx). See 
 	comments in IPasswordText().
******************************************************************************/

void CPasswordText::IViewTemp
	( CView *anEnclosure
	, CBureaucrat *aSupervisor
	, Ptr viewData )
{
	TextFont( monaco );

	inherited::IViewTemp( anEnclosure, aSupervisor, viewData );
	
    InstallTEHooks();

} /* IViewTemp */



/******************************************************************************
 InstallTEHooks

	Replace TextEdit's draw hook with one that only draws bullets. 
******************************************************************************/

void CPasswordText::InstallTEHooks( void )
{
	void MyTEDrawHook( void );
    ProcPtr teHook;

    teHook = (ProcPtr) StripAddress( MyTEDrawHook );
    TECustomHook( intDrawHook, &teHook, macTE );

} /* InstallTEHooks */



/*============================================================================
 MyTEDrawHook

	This routine is called any time the various components of a line are drawn.
	The appropriate font, face, and size characteristics have already been set
	into the current port by the time this routine is called.

	On entry:	D0  offset into text (word)
				D1  length of text to draw (word)
				A0  pointer to text to draw (long)
				A3  pointer to the locked TextEdit record (long)
				A4  handle to the locked TextEdit record (long)

	Thanks to Chris Wysocki (wysocki@husc.harvard.edu) for example code.
============================================================================*/

static char* gBulletString = /* 100 bullets */
	"ееееееееее" "ееееееееее" "ееееееееее" "ееееееееее" "ееееееееее"
	"ееееееееее" "ееееееееее" "ееееееееее" "ееееееееее" "ееееееееее";

static void MyTEDrawHook()
{
    asm {
    	move.l a0,-(sp)            ; save a0 (Why doesn't pea work?)
        movea.l gBulletString,a0   ; get address of bullet string
        move.l a0,-(sp);           ; push ptr to text
        move.w #0,-(sp)            ; push offset into text
        move.w  d1,-(sp)           ; push length of text to draw (Hope <= 100)
        _DrawText                  ; draw the text
        move.l (sp)+,a0            ; restore a0
    }

} /* MyTEDrawHook */



