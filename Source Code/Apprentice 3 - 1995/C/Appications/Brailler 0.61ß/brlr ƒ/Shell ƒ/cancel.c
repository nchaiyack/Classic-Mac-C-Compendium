#include "cancel.h"
#include "main.h"

#define TheCancelKey	'.'

Boolean DealWithOtherPeople(void)
{
	/* this is just a small useful function to see if the user has cancelled */
	/* a lengthy operation with command-period; could come in handy, I suppose, */
	/* in a somewhat bizarre set of circumstances... */
	/* Note that this procedure will break under AUX */
	/* Note also that this returns TRUE if there has been no attempt to cancel */
	
	Boolean			foundEvent;
	EvQElPtr		eventQPtr;
	QHdrPtr			eventQHdr;
	char			thisChar;
	long			isCmdKey;
	
	foundEvent=FALSE;
	eventQHdr=GetEvQHdr();
	eventQPtr=(EvQElPtr)(eventQHdr->qHead);
	while ((eventQPtr!=0L) && (!foundEvent))
	{
		if (eventQPtr->evtQWhat==keyDown)
		{
			thisChar=(char)((eventQPtr->evtQMessage)&charCodeMask);
			isCmdKey=(eventQPtr->evtQModifiers)&cmdKey;
			if (isCmdKey!=0L)
				foundEvent=(thisChar==TheCancelKey);
		}
		if (!foundEvent)
			eventQPtr=(EvQElPtr)(eventQPtr->qLink);
	}

	while (HandleSingleEvent(FALSE)) {};
	
	return !foundEvent;
}
