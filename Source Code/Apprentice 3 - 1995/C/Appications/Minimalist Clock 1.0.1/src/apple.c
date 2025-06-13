/* ----------------------------------------------------------------------
apple.c
---------------------------------------------------------------------- */

#include	"the_defines.h"
#include	"the_globals.h"
#include	"the_prototypes.h"

/* ----------------------------------------------------------------------
DoMenuApple
---------------------------------------------------------------------- */
void DoMenuApple(theItem)
int theItem;
{
	switch (theItem)
	{
		case APPLE_ABOUT:
			AppleAbout();
			break;
		default:
			AppleDA(theItem);
			break;
	}
	
}

/* ----------------------------------------------------------------------
AppleAbout
---------------------------------------------------------------------- */
void AppleAbout()
{
	DialogPtr myDialog;
	short	itemHit;
	Boolean aboutDone = false;
	Boolean modaling;

	myDialog = GetNewDialog(DLOG_ABOUT,0,(WindowPtr) -1);
	if (myDialog)
	{
		do
		{
			ShowWindow(myDialog);
			ModalDialog(NIL,&itemHit);
			switch(itemHit)
			{
				case DLOG_ABOUT_OK:
					aboutDone = true;
					break;
			}
		}
		while (!aboutDone);
		DisposeDialog(myDialog);
	}
}

/* ----------------------------------------------------------------------
AppleDA
---------------------------------------------------------------------- */
void AppleDA(theItem)
int theItem;
{
	Str255	accName;
	int		accNumber;

	GetItem(gAppleMenu,theItem,accName);
	accNumber = OpenDeskAcc(accName);

}
