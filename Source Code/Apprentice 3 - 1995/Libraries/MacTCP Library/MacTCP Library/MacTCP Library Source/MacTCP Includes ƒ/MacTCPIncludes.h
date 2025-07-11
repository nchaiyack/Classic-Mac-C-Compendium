/*
	MacTCPIncludes.h
	
	Generic file for including all of the MacTCP headers.

	01/28/94: 1.0 completed - dn
	06/12/95: 1.01 Added some common typedefs - dn
*/

#pragma once

#ifndef __H_MacTCPIncludes__
#define __H_MacTCPIncludes__

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

#include <MacTCPCommonTypes.h>
#include <GetMyIPAddr.h>
#include <AddressXlation.h>
#include <TCPPB.h>
#include <UDPPB.h>
#include <IPPB.h>

#include <MacTCPConst.h>
#include <MacTCPExtras.h>
#include <MacTCP.h>
#include <DNR.h>
#include <TCP.h>
#include <UDP.h>
#include <IP.h>

#endif /* __H_MacTCPIncludes__ */