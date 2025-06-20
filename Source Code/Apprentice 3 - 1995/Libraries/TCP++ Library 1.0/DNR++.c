
#include "DNR++.h"

#include <Menus.h>
#include <Devices.h>
#include <Events.h>
#include <StdLib.h>
#include <String.h>

pascal void __DNRDone(struct hostInfo* info, char* usrPtr);

OSErr
dnrNameToAddr(char* name, ip_addr* addr, ResultProcPtr proc, char* usrPtr)
{
	OSErr			theErr;
	ResultProcPtr	done;
	struct hostInfo	hinfo;
	
	if (proc)
		done = proc;
	else
		done = &__DNRDone;

	theErr = OpenResolver(nil);
	
	if (theErr == noErr) 
		theErr = StrToAddr(name, &hinfo, done, usrPtr);
		
	if ((theErr == noErr) || (theErr == cacheFault)) {
		while (hinfo.rtnCode == cacheFault)
			SystemTask();
			
		CloseResolver();
		
		if (hinfo.rtnCode == noErr)
			*addr = hinfo.addr[0];
			
		theErr = (OSErr) hinfo.rtnCode;
	}

	return theErr;
}

OSErr
dnrAddrToName(ip_addr addr, char* name, ResultProcPtr proc, char* usrPtr)
{
	OSErr			theErr;
	ResultProcPtr	done;
	struct hostInfo	hinfo;
	int				i;
	
	if (proc)
		done = proc;
	else
		done = &__DNRDone;
	
	theErr = OpenResolver(nil);
	
	if (theErr == noErr) 
		theErr = AddrToName(addr, &hinfo, done, usrPtr);
	
	if ((theErr == noErr) || (theErr == cacheFault)) {
		while (hinfo.rtnCode == cacheFault)
			SystemTask();
		
		CloseResolver();
		
		if (hinfo.rtnCode == noErr) {
			i = strlen(hinfo.cname) - 1;
			if (hinfo.cname[i] == '.')
				hinfo.cname[i] = 0;

			strcpy(name, hinfo.cname);
		}
		
		theErr = (OSErr) hinfo.rtnCode;
	}
	
	return theErr;
}

OSErr
dnrDotsToAddr(char* dots, ip_addr* addr)
{
	OSErr			theErr;
	struct hostInfo	hinfo;
	
	theErr = OpenResolver(nil);
	
	if (theErr == noErr) 
		theErr = StrToAddr(dots, &hinfo, &__DNRDone, nil);
		
	if (theErr == noErr)
		*addr = hinfo.addr[0];

	return theErr;
}

OSErr
dnrAddrToDots(ip_addr addr, char* dots)
{
	return AddrToStr(addr, dots);
}

pascal void
__DNRDone(struct hostInfo* , char* )
{
	// do nothing by default
}
