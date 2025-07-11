#include <stream.h>
#include <Types.h>
#include <Events.h>
#include <stdlib.h>

Boolean IsThisKeyDown(const short theKey)
{
	union
	{
		KeyMap asMap;
		Byte asBytes[16];
	};

	GetKeys(asMap);
	return asBytes[theKey >> 3] & (1 << (theKey & 0x07)) ? true : false;
} // IsThisKeyDown 

pascal Boolean IsCommandKeyDown()
{
	const short kCommandKey = 55;
	return IsThisKeyDown(kCommandKey);
} // IsCommandKeyDown 

pascal Boolean IsControlKeyDown()
{
	const short kCtlKey = 0x3B;
	return IsThisKeyDown(kCtlKey);
} // IsControlKeyDown 

pascal Boolean IsOptionKeyDown()
{
	const short kOptionKey = 58;
	return IsThisKeyDown(kOptionKey);
} // IsOptionKeyDown 

pascal Boolean IsShiftKeyDown()
{
	const short kShiftKey = 56;
	return IsThisKeyDown(kShiftKey);
} // IsShiftKeyDown 


void PrintUsage()
{
	cerr << "### IsModifierKeyDown - bad or missing parameter\n";
	cerr << "# Usage - IsModifierKeyDown [-option] [-shift] [-control] [-command]\n";
	exit(1);
}

Boolean gIsGood = true;

void CheckKey(Boolean down)
{
	if (!down)
		gIsGood = false;
}

int main(int argc,char *argv[])
{
	while ((--argc != 0) && (*++argv)[0] == '-') 
	{
		const char *p = argv[0];
		if (strcmp(p, "-option") == 0)
			CheckKey(IsOptionKeyDown());
		else if (strcmp(p, "-shift") == 0)
			CheckKey(IsShiftKeyDown());
		else if (strcmp(p, "-control") == 0)
			CheckKey(IsControlKeyDown());
		else if (strcmp(p, "-command") == 0)
			CheckKey(IsCommandKeyDown());
		else
			PrintUsage();
	}
	if (argc != 0)
		PrintUsage();
	cout << (gIsGood ? "1" : "0");
	return 0;
}