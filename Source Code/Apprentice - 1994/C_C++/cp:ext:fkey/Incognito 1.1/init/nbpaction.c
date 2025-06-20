#include <Types.h>
#include <PLStringFuncs.h>
#include <Memory.h>
#include <GestaltEqu.h>
#include <time.h>
#include <errors.h>

typedef struct NameLink
{
	Str32 originalName;
	Str32 newString;
	struct NameLink *next, * previous;
} NameLink, * NameLinkPtr;

typedef struct OriginalLink
{
	Str32 originalName;
	struct OriginalLink *next;
} OriginalLink, * OriginalLinkPtr;

typedef struct
{
	NameLinkPtr registeredNames;
	OriginalLinkPtr trappedNames;
} MyGestaltRecord, * MyGestaltPtr;

static MyGestaltPtr GetGestaltInfo(void)
{
	register MyGestaltPtr	pb;
	register OSErr			error;
	
	error = Gestalt('HEID', (long *)&pb);
	if (error) pb = 0;
	return pb;
}

static NameLinkPtr GetRegisteredNames(void)
{
	register NameLinkPtr pb = nil;
	register MyGestaltPtr pc;
	
	pc = GetGestaltInfo();
	if (pc)
	{
		pb = pc->registeredNames;
	}
	return pb;
}

static OriginalLinkPtr GetMaskableNames(void)
{
	register OriginalLinkPtr	pb = nil;
	register MyGestaltPtr		pc;
	
	pc = GetGestaltInfo();
	if (pc)
	{
		pb = pc->trappedNames;
	}
	return pb;
}

/*
  Is the string one that we should mask out?
*/

static Boolean StringIsMaskable(char* theString)
{
	OriginalLinkPtr pb = GetMaskableNames();
	char* maskString;
	short result;

	if (!pb) return false;
	
	maskString = pb->originalName;

	while (true && maskString && maskString[0])
	{
		result = !PLstrcmp(theString, maskString);
		if (result)
		{
			return true;
		}
		pb = pb->next;
		if (!pb) break;
		maskString = pb->originalName;
	}
	return false;
}

/*
  Has the string already been masked out?
*/

static Boolean IsStringMasked(char* theString)
{
	NameLinkPtr pb = GetRegisteredNames();
	register char* registeredName;
	short result;

	if (!pb) return false;
	registeredName = pb->originalName;

	while (true && registeredName && registeredName[0])
	{
		result = !PLstrcmp(theString, registeredName);// check original name
		if (result)
		{
			return true;
		}
		pb = pb->next;
		if (!pb) break;
		registeredName = pb->originalName;
	}
	return false;
}

/*
  This is made 4 quark: it tries to deregister a string,
  so if the string is already masked set the string passed
  in to the mask.
*/

static void SetStringToMask(char* theString)
{
	NameLinkPtr pb = GetRegisteredNames();
	char* registeredName;
	short result;

	if (!pb) return;
	registeredName = pb->originalName;

	while (true && registeredName && registeredName[0])
	{
		if (registeredName && registeredName[0])
		{
			result = !PLstrcmp(theString, registeredName);	// check original name
		}
		else break;
		if (result)
		{
			PLstrcpy(theString, pb->newString);
			return;
		}
		pb = pb->next;
		if (!pb) break;
		registeredName = pb->originalName;
	}
	return;
}

/*
  Munge the string
*/

static void Munge(char* theString)
{
	register short i;
	register char* j;
	
	j = theString;
	i = theString[0];
	while (i)
	{
		theString[i--] = clock() + i;
	}
}

/*
  Insert the old & new strings into the mask list, & munge too
*/

static void MaskString(char* theString)
{
	NameLinkPtr pb = GetRegisteredNames();
	NameLinkPtr newLink;
	
	if (!pb) return;

	newLink = (NameLinkPtr) NewPtrSysClear(sizeof(NameLink));
	if (!newLink) return;
	PLstrcpy(newLink->originalName, theString);
	Munge(theString);
	PLstrcpy(newLink->newString, theString);

	while (true)
	{
		if (pb->next)
		{
			pb = pb->next;
		}
		else
		{
			pb->next = newLink;
			newLink->previous = pb;
			return;
		}
	}

}

/*
  removes an entry from the NameLink list
*/

static void RemoveEntry(char* theString)
{
	NameLinkPtr pb = GetRegisteredNames();
	char* str;

	if (!pb || !theString || !theString[0]) return;
	
	str = pb->newString;
	while (true && str && str[0])
	{
		if (!PLstrcmp(str, theString))
		{
			if (pb->previous)
			{
				(pb->previous)->next = pb->next;
			}
			if (pb->next)
			{
				(pb->next)->previous = pb->previous;
			}
			DisposePtr((Ptr)pb);
			return;
		}
		pb = pb->next;
		if (!pb) break;
		str = pb->newString;
	}

}

// Basically, when registering something, we ask: is the object being deregistered
// one of the ones we should mask out, and if so, has it already been munged?
// If it was already munged, we should set the string to be deregistered to the
// munged string, not the original string.

// Why? Because quark, for reasons unknown, registers, throws away the parameter
// block, then deregisters the string that it was supposed to be registered as,
// not the string it actually was registered as (like every other program).

void DoRemove(char* typeName, char* objName)
{
	if (typeName && typeName[0])
	{
		if (StringIsMaskable(typeName) && IsStringMasked(typeName))
		{
			SetStringToMask(typeName);
		}
		RemoveEntry(typeName);
	}
	
	if (objName && objName[0])
	{
		if (StringIsMaskable(objName) && IsStringMasked(objName))
		{
			SetStringToMask(objName);
		}
		RemoveEntry(objName);
	}
}

static void CheckRegister(char* typeName)
{
	if (!typeName || !typeName[0]) return;
	
	if (StringIsMaskable(typeName))				// should we worry about wiping the type?
	{
		if (!IsStringMasked(typeName))
		{
			MaskString(typeName);
		}
	}
}

void DoRegister(char* typeName, char* objName)
{
	if (typeName && typeName[0]) CheckRegister(typeName);
	if (objName && objName[0]) CheckRegister(objName);
}

//	Also does lookups, since they're basically the same.
void DoConfirm(char* typeName, char* objName)
{
	if (typeName && typeName[0])
	if (StringIsMaskable(typeName))
	{
		Munge(typeName);
	}
	
	if (objName && objName[0])
	if (StringIsMaskable(objName))
	{
		Munge(objName);
	}
}
