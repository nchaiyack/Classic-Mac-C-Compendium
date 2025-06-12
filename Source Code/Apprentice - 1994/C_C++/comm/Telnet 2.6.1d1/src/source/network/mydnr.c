/*
*   Util.c
*   utility library for use with the Network kernel
*
*   version 2, full session layer, TK started 6/17/87
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1992,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*/

#ifdef MPW
#pragma segment DNR
#endif

#include <OSUtils.h>
#include <Folders.h>
#include <ctype.h>

#include "TelnetHeader.h"
#include "InternalEvents.h"
#include "bkgr.proto.h"
#include "AddressXlation.h"
#include "telneterrors.h"
#include "netevent.proto.h"
#include "mydnr.proto.h"
#include "Connections.proto.h"

extern long MyA5;

typedef	struct {
	short
		screen;
	long
		MyA5;
	OSErr
		theError;
	Str63
		hostname;
	struct hostInfo
		*hinfo;
}	DNRDelayStruct;

pascal void DNRDone(struct hostInfo *hostInfoPtr, DNRDelayStruct *info);
pascal void DNRDoneInit(struct hostInfo *hostInfoPtr, DNRDelayStruct *info);
PROTO_UPP(DNRDoneInit, Result);

/**********************************************************************
 * DotToNum - turn an address in dotted decimal into an internet address
 * returns True if the conversion was successful.  This routine is
 * somewhat limited, in that it will accept only four octets, and does
 * not permit the abbreviated forms for class A and B networks. <- Steve Dorner
 **********************************************************************/
Boolean DotToNum(BytePtr string,ip_addr *nPtr)
{
  unsigned long	address=0;
  short			b=0;
  BytePtr 		cp;
  short			dotcount=0;
  
  /*
   * allow leading spaces
   */
  for (cp=string+1;cp<=string+*string;cp++) if (*cp!=' ') break;
  
  /*
   * the address
   */
  for (;cp<=string+*string;cp++)
  {
    if (*cp=='.')
    {
      if (++dotcount > 3) return (FALSE); /* only 4 octets allowed */
      address <<= 8;
      address |= b;
      b=0;
    }
    else if (isdigit(*cp))
    {
      b *= 10;
      b += (*cp - '0');
      if (b>255) return (FALSE);          /* keep it under 256 */
    }
    else if (*cp==' ')                    /* allow trailing spaces */
      break;
    else
      return (FALSE);                     /* periods or digits ONLY */
  }
  
  /*
   * final checks, assignment
   */
  if (dotcount!=3) return (FALSE);
  address <<= 8;
  address |= b;
  *nPtr = (ip_addr) address;
  return(TRUE);
}

/**************************************************************************/
/*	For FTP to look up the transfer options to use when running */
Boolean	TranslateIPtoDNSname(ip_addr ipnum, StringPtr machineName)
{
	#pragma unused (ipnum, machineName)
	// NEED TO IMPLEMENT THIS BUGG
	return(FALSE);

}

/*********************************************************************/
pascal void DNRDone(struct hostInfo *hostInfoPtr, DNRDelayStruct *info)
{
	#pragma unused (hostInfoPtr)
	netputevent(USERCLASS,DOMAIN_DONE,info->screen, (long)info);
}

SIMPLE_UPP(DNRDoneInit, Result);
pascal void DNRDoneInit(struct hostInfo *hostInfoPtr, DNRDelayStruct *info)
{
	long saveA5;
	DNRDelayStruct *mptr;
	struct hostInfo *hptr;
	
	mptr = info;
	hptr = hostInfoPtr;

#ifndef	__powerpc__
	saveA5 = SetA5(mptr->MyA5);
#endif

	DNRDone(hptr, mptr);
	
#ifndef	__powerpc__
	SetA5(saveA5);
#endif
}

OSErr	DoTheDNR(StringPtr hostname, short window)
{
	DNRDelayStruct		*Info;
	struct hostInfo		*HInfo;
	ip_addr				ip;
	
	Info = (DNRDelayStruct *)NewPtrClear(sizeof(DNRDelayStruct));
	HInfo = (struct hostInfo *)NewPtrClear(sizeof(struct hostInfo));
	
	if ((Info == NULL) || (HInfo == NULL)) return(memFullErr);
	
	Info->screen = window;
#ifndef __powerpc__
	Info->MyA5 = MyA5;
#endif
	Info->hinfo = HInfo;
		
	BlockMove(hostname, Info->hostname, Length(hostname)+1);
	
	if (DotToNum(hostname, &ip)) {
		Info->hinfo->rtnCode = noErr;
		Info->hinfo->addr[0] = ip;
		DNRDone(HInfo, Info);
		return(noErr);
		}
		
	PtoCstr(Info->hostname);
	Info->theError = StrToAddr((char *)Info->hostname, HInfo,
								DNRDoneInitUPP, (Ptr)Info);
	
	if ((Info->theError != cacheFault) && (Info->theError != inProgress))
		DNRDone(HInfo, Info);
	
	return(noErr);
}

//	The event queue routines send us the screen number that DNRDone sent.  We demangle
//	this mess of data and call CompleteConnectionOpening to do all of the port and screen
//	stuff that we shouldn't know about at this level.  We are merely a non-interrupt level
//	flow control point. (i.e. I would do this from DNRDone, but that's interrupt time)
void	HandleDomainDoneMessage(short screen, long data2)
{
	DNRDelayStruct	*MyData = (DNRDelayStruct *)data2;
	ip_addr			the_IP;
	OSErr			theErr;		// The error, if any
	
	the_IP = MyData->hinfo->addr[0];
	theErr = MyData->hinfo->rtnCode;
	
	CompleteConnectionOpening(screen, the_IP, theErr, MyData->hinfo->cname);

	//	We also dispose of the DNR memory allocations
		
	DisposePtr((Ptr)MyData->hinfo);
	DisposePtr((Ptr)MyData);
}
