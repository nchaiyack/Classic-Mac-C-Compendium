/* 
	AddressXlation.h		
	MacTCP name to address translation routines.

    Copyright Apple Computer, Inc. 1988-91
    All rights reserved
	
*/	

#define NUM_ALT_ADDRS	4

typedef struct hostInfo {
	int	rtnCode;
	char cname[255];
	unsigned long addr[NUM_ALT_ADDRS];
};

typedef enum AddrClasses {
	A = 1,
	NS,
	CNAME = 5,
	HINFO = 13,
	MX = 15,
	lastClass = 32767
} AddrClasses; 

typedef struct HInfoRec {
	char cpuType[30];
	char osType[30];
	};

typedef struct MXRec {
	unsigned short preference;
	char exchange[255];
	};
	
typedef struct returnRec {
	int	rtnCode;
	char cname[255];
	union {
		unsigned long addr[NUM_ALT_ADDRS];
		struct HInfoRec hinfo;
		struct MXRec mx;
	} rdata;
};

typedef struct cacheEntryRecord {
	char *cname;
	unsigned short type;
	unsigned short class;
	unsigned long ttl;
	union {
		char *name;
		ip_addr addr;
	} rdata;
};

typedef pascal void (*EnumResultProcPtr)(struct cacheEntryRecord *cacheEntryRecordPtr, char *userDataPtr);

typedef pascal void (*ResultProcPtr)(struct hostInfo *hostInfoPtr, char *userDataPtr);

typedef pascal void (*ResultProc2Ptr)(struct returnRec *returnRecPtr, char *userDataPtr);

extern OSErr OpenResolver(char *fileName);

extern OSErr StrToAddr(char *hostName, struct hostInfo *hostInfoPtr, ResultProcPtr ResultProc, char *userDataPtr);

extern OSErr AddrToStr(unsigned long addr, char *addrStr);

extern OSErr EnumCache(EnumResultProcPtr enumResultProc, char *userDataPtr);

extern OSErr AddrToName(ip_addr addr, struct hostInfo *hostInfoPtr, ResultProcPtr ResultProc, char *userDataPtr);

extern OSErr HInfo(char *hostName, struct returnRec *returnRecPtr, ResultProc2Ptr resultProc, char *userDataPtr);

extern OSErr MXInfo(char *hostName, struct returnRec *returnRecPtr, ResultProc2Ptr resultProc, char *userDataPtr);

extern OSErr CloseResolver();