
#include	<stdio.h>
#include	<string.h>
//#include	<unix.h>

#include	<MacTCPCommonTypes.h>
#include	<GetMyIPAddr.h>
#include	<TCPPB.h>
#include	<UDPPB.h>
#include	<AddressXlation.h>
#include	<MiscIPPB.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>

#include "tcpglue.h"
#include "socket.internal.h"





main()
{

	printf("Hello World\n");
	
	xNetMask();
	xIPAddr();
}