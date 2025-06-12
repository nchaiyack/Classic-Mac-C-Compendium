/*
 * interface to dnr.c
 * created by Aaron Wohl / n3liw+@cmu.edu
 * to allow dnr.c to be used with think c
 */

/*
 * changes by Aaron Wohl / n3liw+@cmu.edu
 * support for think c
 */
TrapType GetTrapType(unsigned long theTrap);
Boolean dnr_TrapAvailable(unsigned long trap);
void GetSystemFolder(short *vRefNumP, long *dirIDP);
void GetCPanelFolder(short *vRefNumP, long *dirIDP);
short SearchFolderForDNRP(long targetType, long targetCreator, short vRefNum, long dirID);
short OpenOurRF(void);
OSErr OpenResolver(char *fileName);
OSErr CloseResolver(void);
OSErr StrToAddr(char *hostName,struct hostInfo *rtnStruct,long resultproc,char *userDataPtr);
OSErr EnumCache(long resultproc,char *userDataPtr);
OSErr AddrToStr(unsigned long addr,char *addrStr);									
OSErr AddrToName(unsigned long addr,struct hostInfo *rtnStruct,long resultproc,char *userDataPtr);									
OSErr HInfo(char *hostName,struct returnRec *returnRecPtr,long resultProc,char *userDataPtr);
OSErr MXInfo(char *hostName,struct returnRec *returnRecPtr,long resultProc,char *userDataPtr);
