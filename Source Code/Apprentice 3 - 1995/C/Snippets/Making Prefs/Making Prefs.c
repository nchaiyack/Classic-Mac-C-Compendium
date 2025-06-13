/****************************************************************************************
Did you ever wonder how to make prefrence files or just how to save to resources in 
general? Me too. Well this file in 'C' contains two functions one that creates a 
preference file in the the preference folder and saves prefs to it. The other reads the 
prefs back. I have tried to document to code extensivly to make it as clear as possible 
and easy to learn from. If you have an questions e-mail me on America Online at Bernard256
Also be sure to check out the Macintosh Development Forum on America Online the guys there
answer all questions. Feel free to distrubute this to all your friends or post it on BBS's,
ftp's etc... By the way you would probably have best luck reading this with Think C or Codewarrior
because I used hard returns. The code here only works in System 7 but if you know 
something about files and resources you could make it Sys 6 compatible. For more on
Files see Inside Macintosh: Files or for resources Inside Macintosh: More Mac ToolBox
****************************************************************************************/

#include "stdio.h"
#include "Folders.h"
#include "string.h"

FSSpec mySpec;
long myDirID;
short myResRef, gARef;
short myVRef;

void ReadPref(void);
void SavePref(void);

struct prefs /*this is the setting struct as well as a handle to it because you save 
			   resources using Handles. You'll see*/
{
	Boolean havingFun;
	Boolean registered;
	Boolean soundOn;
	char *Names[5][256];/*high score names array*/
	int Scores[5];/*high score numbers array*/
}settings,**setHan;

void main(void)
{
	//Call these functions in the beginning of your application
	gARef = CurResFile();/*keep a permanant refrence to you apps resource fork so we can 
					   always go back to it after we are done fooling around with the 
					   pref files resource fork*/
	setHan = (struct prefs **)NewHandle(sizeof(settings));/*initalize the Handle*/
	SavePref();
	ReadPref();
}
void ReadPref(void)
{
	int err,i;
	OSErr myErr;
	
	myErr = FindFolder(0x8000, 'pref', FALSE, &myVRef, &myDirID);/*Make sure you are in 
																   the preference folder
																   of the startup volume*/
	if(myErr == noErr)/*your cool*/
		myErr = FSMakeFSSpec(myVRef, myDirID, "\pMyPrefs",&mySpec);
	if(myErr==fnfErr)/*you've still created the FSSpec but you don't have a file to attach
					   it to so you can't read the prefs so we must call SavePrefs() 
					   because besides saving prefs SavePrefs() creates the file*/
	{
		SavePref();
	}
	else /*otherwise you have the pref file and can just read from it*/
	{
		myResRef = FSpOpenResFile(&mySpec,3);/*open the file's resource fork*/
		setHan = (struct prefs **)GetResource('????',128);/*'????' stands for the kind of
															resource you are saving to. 
															Just like you normally use 
															'WIND' or 'MENU' resources now
															you are going to use a 
															resource type that you have 
															created in the SavePref()
															function. Anyway we are making
															the Handle setHan equal to the
															Handle of resource id 128. We
															coerce it to fit the same struct
															as setHan*/
		settings.havingFun=(**setHan).havingFun;/*now we set our settings equal to the saved ones*/
		settings.registered=(**setHan).registered;
		settings.soundOn=(**setHan).soundOn;
		for(i=0;i<5;i++)
		{
			settings.Scores[i] =(**setHan).Scores[i];
			strcpy(*settings.Names[i],*(**setHan).Names[i]);
		}
		UseResFile(gARef);/*set the curren resource fork back to your programs*/
	}
}
void SavePref(void)
{
	int err,i;
	OSErr myErr;
	
	myErr = FindFolder(0x8000, 'pref', FALSE, &myVRef, &myDirID);/*Make sure you are in 
																   the preference folder
																   of the startup volume*/
	if(myErr == noErr)
		myErr = FSMakeFSSpec(myVRef, myDirID, "\pMy Prefs",&mySpec);
	if(myErr==fnfErr)/*the file doesn't exist although the FSSpec has been created so now
					   we have to make the file if the file does exist we can just skip
					   ovet the next section and go directly to the writing*/
	{
		FSpCreateResFile(&mySpec, '????','pref',-1);/*create a file of type 'pref' with 
													  the creator being whatever 4 letter
													  code you put in place of ????*/
		myResRef = FSpOpenResFile(&mySpec,3);		/*Get a refrence to this file's 
													  resource fork*/
	}
	UseResFile(myResRef);/*and use this file's resource fork as the current fork*/
	(**setHan).havingFun=settings.havingFun;/*Now save your application's setting to the
											  resource handle*/
	(**setHan).registered=settings.registered;
	(**setHan).soundOn=settings.soundOn;
	for(i=0;i<5;i++)
		{
			(**setHan).Scores[i] =settings.Scores[i];
			strcpy(*(**setHan).Names[i],*settings.Names[i]);
		}
	if(myErr==fnfErr)/*if we have just created the pref file we have to use the add 
						resource command to add a resource to the resource fork*/
	{
		AddResource((Handle)setHan,'????',128, "\pPrefs");/*???? is the type (like we
															talked about before) and 128
															is the id and Prefs the name
															The first paramter is a handle
															the the struct cause add 
															resource creates a resource 
															out of a handle (thats why
															we made a handle)*/
	}
	else /*if we are calling it just to update the file we have already made the just
		   call Changed resource and it knows which ones*/
	{
		ChangedResource((Handle)setHan);
  		UpdateResFile(myResRef);/*update the file's resource fork*/
  	}

	UseResFile(gARef);/*set the current resource fork back to your applications*/
}