#include "nim message.h"
#include "dialog layer.h"
#include "sound layer.h"

#define kMessageStringsID		130
#define kMessageAlertID			150

void DisplayMessage(GameError messageToken)
{
	Str255			messageStr;
	
	DoSound(sound_error, TRUE);
	
	if (!gDisplayMessages)
		return;
	
	GetIndString(messageStr, kMessageStringsID, messageToken);
	DisplayTheAlert(kPlainAlert, kMessageAlertID, messageStr,"\p", "\p", "\p",
		(UniversalProcPtr)OneButtonFilter);
}
