/****************************************************************************
*
*  HCGestalt XFCN v1.1.1
*
*  All source code © 1992-94 Best Before Yesterday (Alex Metcalf)
*  All rights reserved
*
*
*  This source code is being distributed to allow new programmers to
*  see how to get started with writing an external for Hypercard.
*  I know it's not perfect code (I'm sure people can suggest ways to
*  optimize it), but I think it's a good example of how you can receive
*  parameters, as well as return a result to Hypercard.
*
*  This code is for THINK C. Make a project, and include the files:
*  "ANSI-A4", "HyperXLib", and "MacTraps". You'll probably need all three
*  of these when you write your own external.
*
*  Please distribute only unmodified versions of this code.
*  Contact me if you wish to use this XFCN in your stacks.
*
*  Alex Metcalf
*  alex@metcalf.demon.co.uk
*
*  UK Fax:        (0570) 45636
*  US/Canada Fax: 011 44 570 45636
*
****************************************************************************/


/*****
*
* A quick note about memory:
*
* The most common problem in Hypercard externals occurs when you don't
* "tidy up after yourself". This means that you allocate some memory
* somewhere, and then don't get rid of it when you leave. Not good! Although
* this only causes problems when the user calls your XCMD lots of times,
* it's much better to find the leak and plug it. The way to check for a
* leak is to make an empty stack, put your XCMD or XFCN in the stack, and
* put the following into the card script:
*
* on idle
*    MyXCMD "parameter 1"
*    put the heapspace
* end idle
*
* or if it's an XFCN:
*
* on idle
*    get MyXFCN "parameter 1"
*    put the heapspace
* end idle
*
* Then move the mouse over the button and watch the number in the msg box.
* If it's going down, you have a memory leak! Check through your code for
* any times you use NewPtr() or NewHandle(), and make sure you're disposing
* them if you have to. I've only used NewPtr once in this code, and I didn't
* dispose it because I'm giving it back to Hypercard. Another way you can
* lose memory is if you use a toolbox routine which allocates memory, and
* you don't bother to get rid of it.
*
*****/


#include <HyperXCmd.h>
#include <gestaltEqu.h>

/*****
*
* If you want to use global variables in your code, then you need
* to include:
*
* #include <SetUpA4.h>
*
* Then, right at the beginning of main(), type in "SetUpA4 ();". Then, right
* at the end of main(), type in "RestoreA4 ();". You'll then be able to use
* global variables.
*
* Note you'll also need to put RestoreA4 right before any "return" commands
* you have in your main().
* 
*****/


/*****
*
* To return a result to Hypercard, you need to give Hypercard a handle
* to a C string which you have made.
*
*****/

void ReturnValue (XCmdPtr paramPtr, char *tStringPtr)
{
	Handle	tStringHandle;
	
	tStringHandle = NewHandle (256);
	if (tStringHandle == nil)
	{
		SysBeep (1);
		return;
	}
	
	MoveHHi (tStringHandle);
	HLock (tStringHandle);
	strcpy ((char *) *tStringHandle, tStringPtr);
	
	paramPtr->returnValue = tStringHandle;
}

/*****
*
* UpperCase is a little routine I made to convert the user's parameters
* into upper case. Otherwise, if they used a parameter "one" and you only
* checked for "One", for example, they'd wonder why it wasn't working.
*
* I think there's a command in C for converting a string to upper case,
* but I didn't know that when I wrote this. :-)
*
*****/

UpperCase (char *tString)
{
	short		i, tLength;
	
	tLength = strlen (tString);
	for (i=0;i<=tLength;i++)
	{
		if ( (tString[i] >= 'a') && (tString[i] <= 'z') )
		{
			tString[i] -= 32;
		}
	}
}


/*****
*
* Great little piece of code to check for a system version you require.
* For example, if your XCMD needs System 7.0.1 or later, replace the
* 604 with 701. This XFCN, obviously, requires System 6.0.4 or later.
*
*****/


Boolean SystemOK (void)
{
	OSErr 		myStatus;
	SysEnvRec	myEnvData;
	
	myStatus = SysEnvirons (1, &myEnvData);
	if ((myStatus != noErr) ||
		(myEnvData.systemVersion < 0x604))
	{
		return false;
	} else
	{
		return true;
	}
}

/*****
*
* Used for checking for a Gestalt feature.
*
*****/

char * CheckMyBit (long *tFeature, short tTestBit)
{
	if (BitTst (tFeature, 31-tTestBit) == true)
	{
		return "true";
	} else
	{
		return "false";
	}
}

/*****
*
* A small but very important bit of code, which reads in the parameters
* that the programmer gives you. For the first parameter, use 'params[0]',
* for the second use 'params[1]', and so on. You can check the number
* of parameters that the user gave you by checking "paramPtr->paramCount".
*
*****/

void LoadParams (XCmdPtr paramPtr, char *tSelectorType)
{
	strcpy (tSelectorType, *(paramPtr->params[0]));
}

/*****
*
* The majority of this code deals with the Gestalt Manager.
* If you're only interested in the 'core' of the XCMD, skip
* right down to the bottom, to main().
*
* I've removed most of the checks, but I've left one or two so you
* get an idea of what's happening.
*
*****/

void HCGAddressingMode (char *tReturnStr)
{
	long	tFeature;
	int		tError;
	
	tError = Gestalt (gestaltAddressingModeAttr, &tFeature);
	if (tError != noErr)
	{
		strcpy (tReturnStr, "Error");
		return;
	}
	strcpy (tReturnStr, CheckMyBit (&tFeature,
		gestalt32BitAddressing));
	
	if (strcmp (tReturnStr, "true") == 0)
	{
		strcpy (tReturnStr, "32 Bit Addressing");
	} else
	{
		strcpy (tReturnStr, "24 Bit Addressing");
	}
}

void HCGMachineType (char *tReturnStr)
{
	long	tFeature;
	int		tError;
	
	tError = Gestalt (gestaltMachineType, &tFeature);
	if (tError != noErr)
	{
		strcpy (tReturnStr, "Error");
		return;
	}
	GetIndString ((StringPtr) tReturnStr,
		kMachineNameStrID, tFeature);
	PtoCstr ((StringPtr) tReturnStr);
}

void HCGROMSize (char *tReturnStr)
{
	long	tFeature;
	int		tError;
	
	tError = Gestalt (gestaltROMSize, &tFeature);
	if (tError != noErr)
	{
		strcpy (tReturnStr, "Error");
		return;
	}
	NumToString (tFeature, (StringPtr) tReturnStr);
	PtoCstr ((StringPtr) tReturnStr);
}

void GetGestalt (char *selector, char *tReturnStr)
{
	UpperCase (selector);
	
	if (strcmp (selector, "ADDRESSING MODE")==0)
	{
		HCGAddressingMode (tReturnStr);
		return;
	}
	
	if (strcmp (selector, "MACHINE TYPE")==0)
	{
		HCGMachineType (tReturnStr);
		return;
	}
	
	strcpy (tReturnStr, "Error: selector type unknown.");
}

/*****
*
* main() is the first bit of code that is executed when the user calls your
* external. The only parameter is something called an XCmdPtr: this is the
* information that Hypercard passes you, such as what parameters the user
* entered and stuff like that. Don't lose it! You need the same information
* to give a result back to the user. If you want to know more about it,
* look in the header "HyperXCmd.h".
*
* I've started by checking the system version to make sure Gestalt's available.
* I then used paramPtr->paramCount to make sure that they only entered one
* parameter: if they didn't, then I tell them how to do it properly! Then,
* LoadParams () loads  the parameter, and GetGestalt goes away and figures
* out what they want. If any of my Gestalt stuff returned an error message,
* then I convert it to something more meaningful for the user. Finally, I
* use ReturnValue to give the result back to the user. Simple!
*
*****/
	
pascal void main (XCmdPtr paramPtr)
{
	char	tReturnStr[256], selector[256];
	
	*tReturnStr = 0;
	
	if (!SystemOK ())
	{
		strcpy (tReturnStr, "Error: HCGestalt requires System 6.0.4"
			" or later.");
		ReturnValue (paramPtr, tReturnStr);
		return;
	}

	if ( !(paramPtr->paramCount==1) )
	{
		strcpy (tReturnStr, "Syntax: HCGestalt <selector type>");
		ReturnValue (paramPtr, tReturnStr);
		return;
	}
	
	LoadParams (paramPtr, selector);
	GetGestalt (selector, tReturnStr);
	
	if (strcmp(tReturnStr, "Error")==0)
	{
		strcpy (tReturnStr, "Error: Gestalt Manager returned"
			" an error.");
	}
	
	ReturnValue (paramPtr, tReturnStr);
}

/*****
*
* The end!
*
*****/
