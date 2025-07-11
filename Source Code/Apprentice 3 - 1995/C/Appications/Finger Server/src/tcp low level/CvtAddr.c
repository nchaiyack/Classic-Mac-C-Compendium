#include <Types.h>
#include "MacTCPCommonTypes.h"
#include "AddressXLation.h"
#include "CvtAddr.h"
#include <string.h>

pascal void DNRResultProc(struct hostInfo *hInfoPtr,char *userDataPtr);

/*	ConvertStringToAddr is a simple call to get a host's IP number, given the name
	of the host.
*/

OSErr ConvertStringToAddr(char *name,unsigned long *netNum)
{
	struct hostInfo hInfo;
	OSErr result;
	char done = 0x00;
	extern Boolean gCancel;

	if ((result = OpenResolver(nil)) == noErr) {
		result = StrToAddr(name,&hInfo,DNRResultProc,&done);
		if (result == cacheFault)
			while (!done)
				; /* wait for cache fault resolver to be called by interrupt */
		CloseResolver();
		if ((hInfo.rtnCode == noErr) || (hInfo.rtnCode == cacheFault)) {
			*netNum = hInfo.addr[0];
			strcpy(name,hInfo.cname);
			name[strlen(name)-1] = '\0';
			return noErr;
		}
	}
	*netNum = 0;

	return result;
}


/*	This is the completion routine used for name-resolver calls.
	It sets the userDataPtr flag to indicate the call has completed.
*/

pascal void DNRResultProc(struct hostInfo *hInfoPtr,char *userDataPtr)
{
#pragma unused (hInfoPtr)

	*userDataPtr = 0xff;	/* setting the use data to non-zero means we're done */
}

