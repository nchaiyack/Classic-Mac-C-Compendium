/* mz 2/8/93 */
/* Displays the "More Info dlg" from main app menu.
*/

extern	EventRecord		Event;

#define	MOREINFODIALOG	141
#define DONEBUTTON 1

void	MoreInfo()
{
	DialogPtr	MoreInfoDialog;
	DialogPtr	DialogHit;
	int			ItemHit;
	
	MoreInfoDialog = GetNewDialog(MOREINFODIALOG,0,(WindowPtr)-1);
	ShowWindow(MoreInfoDialog);
	ItemHit = 0;
	do
	{	
		if(GetNextEvent(everyEvent,&Event))
		{	if(IsDialogEvent(&Event) && Event.what != keyDown)
				if(!DialogSelect(&Event,&DialogHit,&ItemHit))
					ItemHit=0;
			
		}
	} while(!ItemHit);
	DisposDialog(MoreInfoDialog);
	Event.what = 0;
	Event.modifiers = btnState;
}
