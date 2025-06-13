
#ifndef __DNRLIB__
#define __DNRLIB__

#ifndef __MACTCPCOMMONTYPES__
#include <MacTCPCommonTypes.h>
#endif
#ifndef __ADDRESSXLATION__
#include <AddressXlation.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

OSErr	dnrNameToAddr(char* name, ip_addr* addr, ResultProcPtr proc, char* usr);
OSErr	dnrAddrToName(ip_addr addr, char* name, ResultProcPtr proc, char* usr);
OSErr	dnrDotsToAddr(char* dots, ip_addr* addr);
OSErr	dnrAddrToDots(ip_addr addr, char* dots);

#ifdef __cplusplus
}
#endif

#endif
