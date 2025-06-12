#pragma load "precompile"
#include "MACTCPCommonTypes.h"
#include "rslv.h"
#include "fsu.h"

main()
{
	OSErr errCode;
	short	refNum;
	char name[256];
	unsigned long addr;
	char		str[256];

	errCode = OpenDriver("\p.IPP", &refNum);
	printf("OpenDriver return code = %d\n", errCode);
	if (errCode) return 1;
	errCode = rslv_Init(refNum);
	printf("rslv_Init return code = %d\n", errCode);
	if (errCode) return 1;
	while (true) {
		printf("Enter domain name:\n");
		if (!gets(name) || !*name) break;
		c2pstr(name);
		errCode = rslv_Resolve(name, nil, &addr);
		printf("rslv_Resolve return code = %d\n", errCode);
		if (errCode == noErr) {
			printf("IP address = %s\n", rslv_DottedDecimal(addr, str));
		};
	};
}