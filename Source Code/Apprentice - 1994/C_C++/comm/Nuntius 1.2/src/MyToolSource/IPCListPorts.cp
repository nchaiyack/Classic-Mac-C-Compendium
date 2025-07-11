#include <Memory.h>
#include <stdio.h>
#include <PPCToolbox.h>
#include <StdLib.h>
#include <PLStringFuncs.h>
#include <Strings.h>

void CheckOS(OSErr err, const char *msg)
{
	if (!err)
		return;
	fprintf(stderr, "### Error: %s\n", msg);
	fprintf(stderr, "### ErrorCode = %ld\n", long(err));
	exit(1);
}

int main()
{
	CheckOS(PPCInit(), "PPCInit()");
//	Debugger();
	const short maxNumPorts = 100;
	PortInfoRec *buffer = (PortInfoRec *)NewPtr(maxNumPorts * sizeof(PortInfoRec));
	if (!buffer)
		CheckOS(-108, "no mem for buffer");
	PPCPortRec pr;
	PLstrcpy(pr.name, "\p=");
	pr.nameScript = 0;
	pr.portKindSelector = ppcByString;
	PLstrcpy(pr.u.portTypeStr, "\p=");

	LocationNameRec ln;
	ln.locationKindSelector = ppcNBPLocation;
	PLstrcpy(ln.u.nbpEntity.zoneStr, "\pPrinter net");
	PLstrcpy(ln.u.nbpEntity.objStr,  "\pDelfi");
	PLstrcpy(ln.u.nbpEntity.typeStr, "\pPPCToolbox");

	IPCListPortsPBRec pb;
	pb.ioCompletion = nil;
	pb.startIndex = 0;
	pb.requestCount = maxNumPorts;
	pb.portName = &pr;
	pb.locationName = &ln;
	pb.bufferPtr = buffer;
	CheckOS(IPCListPortsSync(&pb), "IPCListPortsSync");
	short num = pb.actualCount;
	fprintf(stderr, "Num ports = %ld\n", long(num));
	PortInfoRec *p = buffer;
	for (short i = 1; i <= num; ++i, ++p)
	{
		p2cstr(p->name.name);
		fprintf(stderr, "Port #%2ld: auth = %ld  '%s'", i, long(p->authRequired), p->name.name);
		if (p->name.portKindSelector == ppcByString)
		{
			p2cstr(p->name.u.portTypeStr);
			fprintf(stderr, "  type = '%s'\n", p->name.u.portTypeStr);
		}
		else
			fprintf(stderr, "  by creator&type\n");
	}
	return 0;
}