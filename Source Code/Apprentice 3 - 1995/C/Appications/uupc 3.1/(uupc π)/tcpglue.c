#include "dcp.h"

#include	"MacTCPCommonTypes.h"
#include	"AddressXlation.h"
#include	"GetMyIPAddr.h"
#include	"TCPPB.h"
#include	"UDPPB.h"

#include	"TCP.h"

static	TCPNotifyProc	asrProc			= nil;
static	TCPiopb			pb;
static	StreamPtr		stream;
static	char			streamBuf[4096];
static	long			streamBufLen	= sizeof(streamBuf);
static	Ptr				streamBufPtr;
static	ip_addr			localIP			= cAnyIP;
static	ip_port			localPort		= cAnyPort;
static	ip_addr			remoteIP		= cAnyIP;
static	ip_port			remotePort		= 540; /* uucp listener daemon */
static	WDS(1)			wds;

pascal void	StrToAddrResultProc(aHostInfo, userdata)	/* utility routine for StrToAddr */
	struct hostInfo	*aHostInfo;
	Ptr				userdata;
{
	/* simply watch the aHostInfo.rtnCode! */
}

OSErr TCPDotAddress(char *dotAddress, ip_addr *ipAddress) {
	auto	OSErr				osErr;
	auto	struct hostInfo		aHostInfo;			/* a data structure for the DNR function */

	osErr = OpenResolver((char *) 0);
	if (osErr) {
		return osErr;
	}
	
	/* ask the DNR function to get the IP address */
	StrToAddr(dotAddress, &aHostInfo, (ResultProcPtr) StrToAddrResultProc, (Ptr) 0);
	
	/* wait for the address information or some error other than cacheFault to occur */
	while (cacheFault == aHostInfo.rtnCode)
		;
	
	osErr = CloseResolver();
	if (osErr) {
		return osErr;
	}

	/* if it was an error there isn't much more we can do here but let the caller know */
	if (noErr != aHostInfo.rtnCode) {
		osErr = aHostInfo.rtnCode;
		return osErr;
	}
	
	/* use the first IP address for this host */
	*ipAddress = aHostInfo.addr[0];
				
	return osErr;
}

OSErr open_tcp_uucp()
{
	OSErr osErr;
	osErr = _TCPInit();
	if (osErr = noErr) {
		osErr = _TCPCreate(&pb, &stream, streamBuf, streamBufLen, (TCPNotifyProc) asrProc, (Ptr) nil,  (TCPIOCompletionProc) nil, false);
		if (noErr == osErr) {
			osErr = TCPDotAddress(flds[FLD_PHONE], &remoteIP);
			if (noErr == osErr) {
				osErr = _TCPActiveOpen(&pb, stream, remoteIP, remotePort, &localIP, &localPort, (Ptr) nil, (TCPIOCompletionProc) nil, false);
			}
		}
	}
	return osErr;
}
