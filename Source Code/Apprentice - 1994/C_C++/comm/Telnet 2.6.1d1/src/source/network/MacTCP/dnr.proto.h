short AppleOpenOurRF(short VRefNum, long DirID);
short OpenOurRF(void);
OSErr OpenResolver(char *fileName);
OSErr CloseResolver(void);
OSErr StrToAddr(char *hostName, struct hostInfo *rtnStruct, long resultproc, char *userDataPtr);
OSErr AddrToStr(unsigned long addr, char *addrStr);
OSErr EnumCache(long resultproc, char *userDataPtr);
OSErr AddrToName(unsigned long addr, struct hostInfo *rtnStruct, long resultproc, char *userDataPtr);
