
#include <Memory.h>
#include <StdIO.h>
#include <stream.h>
#include <Types.h>
#include <Strings.h>
#include <StdLib.h>
#include <PLStringFuncs.h>
#include <Aliases.h>
#include <Resources.h>
#include <Errors.h>
#include <ToolUtils.h>

Str31 gTheZone, gTheServer;
Str255 gFullpath;
FSSpec gAliasFileSpec;
short gAliasFileRefnum = -1;
AliasHandle gTargetAliasH = nil;

void PrintUsage()
{
	cerr << "### MakeFinderAlias - bad or missing parameter\n";
	cerr << "# Usage - PLookup [-z zone] [-t type] gFullpath alias-file\n";
	exit(1);
}

void FailOSErr(OSErr err, const char *msg)
{
	if (err == noErr)
		return;
	cerr << "### MakeFinderAlias, error at " << msg << "\n";
	cerr << "GetErrorText " << err;
	exit(1);
}

void FailNIL(void *p)
{
	if (!p)
		FailOSErr(-108, "??");
}

void DoMakeTargetAlias(const char *gFullpath)
{
	FailOSErr(NewAliasMinimalFromFullpath(short(strlen(gFullpath)),
											(const unsigned char*)gFullpath, gTheZone, gTheServer, &gTargetAliasH),
											"NewAlias of gFullpath");
	FailNIL(gTargetAliasH);
}

void DoMakeAliasFile(const char *aliasfilename)
{
	AliasHandle aliasH = nil;
	FailOSErr(NewAliasMinimalFromFullpath(short(strlen(aliasfilename)),
											(const unsigned char*)aliasfilename, "\p", "\p", &aliasH),
											"NewAlias of alias-file");
	FailNIL(aliasH);
	Boolean wasChanged;
	OSErr err = ResolveAlias(nil, aliasH, &gAliasFileSpec, &wasChanged);
	if (err != fnfErr)
		FailOSErr(err, "Resolve alias-file alias");
	FSpCreateResFile(&gAliasFileSpec, 'MPS ', 'TEXT', 0);
	FailOSErr(ResError(), "FSpCreate");
	gAliasFileRefnum = FSpOpenResFile(&gAliasFileSpec, fsRdWrPerm);
	if (gAliasFileRefnum == -1)
	{
		FailOSErr(ResError(), "FSpOpenResFile of alias-file");
		FailOSErr(-1111, "FSpOpenResFile of alias-file, returned -1");
	}
}

void DoAddAlias()
{
	UseResFile(gAliasFileRefnum);
	FailOSErr(ResError(), "UseResFile(gAliasFileRefnum)");
	AddResource(Handle(gTargetAliasH), 'alis', 0, "\pAlias");
	FailOSErr(ResError(), "AddResource");
	WriteResource(Handle(gTargetAliasH));
	FailOSErr(ResError(), "WriteResource");
}

void DoCloseFile()
{
	CloseResFile(gAliasFileRefnum);
	FailOSErr(ResError(), "CloseResFile(gAliasFileRefnum)");
	FInfo fndrInfo;
	FailOSErr(FSpGetFInfo(&gAliasFileSpec, &fndrInfo), "FSpGetFInfo");
	fndrInfo.fdFlags |= 1 << 15; // no constant in IM
	FailOSErr(FSpSetFInfo(&gAliasFileSpec, &fndrInfo), "FSpSetFInfo");
}

int main(int argc,char *argv[])
{
	PLstrcpy(gTheZone,				"\p*");
	PLstrcpy(gTheServer,			"\p=");
	PLstrcpy(gFullpath,			"\p=");

	StringHandle sH = GetString(-16413); // can't find it in IM: name of Macintosh
	if (sH && *sH)
		PLstrcpy(gTheServer, *sH);
	
	while ((--argc != 0) && (*++argv)[0] == '-') {
		switch ( *(argv[0]+1) ) {
		case 'z':
		case 'Z':
			if (!argc--) PrintUsage();
			PLstrcpy(gTheZone, c2pstr(*++argv));
			break;
		case 's':
		case 'S':
			if (!argc--) PrintUsage();
			PLstrcpy(gTheServer, c2pstr(*++argv));
			break;
		default:
			PrintUsage();
		}
	}
	if (argc != 2)
		PrintUsage();
	DoMakeTargetAlias(*argv);
	--argc;
	DoMakeAliasFile(*++argv);
	DoAddAlias();
	DoCloseFile();
	return 0;
}
