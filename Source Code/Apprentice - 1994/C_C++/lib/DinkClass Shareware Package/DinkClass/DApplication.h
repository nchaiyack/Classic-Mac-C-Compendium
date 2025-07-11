/*
	File:		DApplication.h

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <4>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <3>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <2>	 9/20/92	MTG		Bringing the C++ code up todate with the THINK C version

	To Do:
*/


// This is the class declaration of the DApplication class.
#ifndef __DAPPLICATION__
#define __DAPPLICATION__

#include "DListStuff.h"
#include "DEventHandler.h"
class DDocument; // forward declaration 

class DApplication : public DEventHandler
{

public:

	Boolean fInBackground;

	DApplication(void);
	~DApplication(void);

protected:

	unsigned long	fSleepVal;
	RgnHandle fCursorRgn;
	DEventHandler*	fTarget;
	
	DIterator*	fIterator;
			// an iterator for use in place of the local iterators
			// which have to be created and destroyed in tight loops ( like in Idle procs)
	
	DList*  fEventHandlers;
			// This list is needed because I'm not using a doubly linked list to
			// implement the message passing scheam within the DEventHandler class
			// and to clean up when quitting the application
			
	DList* fDeadHandlers;
			// This list is needed because a) the signely linked list shceem and
			// b) the fact that more than one event handler could died in one
			// event loop pass.
			
	Boolean FlushDeadHandlers(void);
			// Gets called from the gApplications nullEvent handleing
			// routeen, if the list of deadHandlers is not empty.  It
			// frees/deletes all the elements in that list.

// 
// The Following members and meber functions are in support of the 
// enent handling scheem implemented within this class library.
// These are the First Crack event handling methods
//
	virtual void MouseDown(EventRecord *theEvent);
		// Supports menu selections and window selections
		// when mouseDown happens in a inactive window or passes the 
		// handleMouseDown message down the fTarget chain.
		
	virtual void NullEvent(EventRecord *theEvent);
	virtual void ActivateEvt(EventRecord *theEvent);


	virtual void AutoKey(EventRecord *theEvent);
	virtual void KeyDown(EventRecord *theEvent);					

	virtual void DiskEvt(EventRecord *theEvent);

	virtual void HighLevelEvent(EventRecord *theEvent);
	virtual void OSEvent(EventRecord *theEvent);
	virtual void UpdateEvt(EventRecord *theEvent);
	
public:
	
	Boolean fDone;
	Boolean InitApp(void);
		// set up lists and put any members into a 
		// safe state before the rest of the application starts roling.
	
	Boolean InstalHandler( DEventHandler *addMe);
	Boolean RemoveHandler( DEventHandler *killMe);
		// Take killMe out of the list of fEventHandlers and updates
		// and refrences to the killMe instance to point to killMe->nextHandler
		// It also inserts killMe into the list of fDeadHandlers which will get 
		// flushed when a NULL event comes around.  It couldn't be deleted
		// otherwise because of the possible reslut in deleting
		// the dead object while one of its methods on the stack.
			
	virtual void EventLoop(void);
		// parses the events to the first crack handlers.
		// virtual because in the Comunication tool box 
		// demo I needed to call tmidle every time through the
		// event loop....
		
	Boolean CleanUp(void);	
		// goes through and has all the fEventHandlers kill them selves
		// and frees up any used memory
	
	virtual void HandleMenuChoice(short menuID, short menuItem);			
	virtual void SetUpMenues(void);
	
protected:
	
	virtual void ClearMenus(void);

// This next section are the functions supporting the multiple
// documents and file handleing for the DApplication class.

public:

	OSType		fCreator;
	OSType		fClipType;
	OSType		fMainFileType;
			// set these members in the main function!!

	Handle	fClipData;
		// access is needed for the Document objects to do the copy
		// and past methods.
	
	virtual DDocument* MakeDDoc(Boolean OpenFromFile);
		// instaniates a new DDocument object and calls
		// it initialization function which will look 
		// for a disk file to read in the data from if
		// OpenFromFile is true.

	virtual int GetFileType(OSType *typeList);
		// fills the SFTypeList array and returns the number of Items
		// the number of Items is limited to 4.  The OS can do better BUT
		// to SFTypeList is defined for 4 items and its easyer than worrying
		// about if the pointer passed to this function has been alocated correctly.
		
private:
	
	void	GetClipFromSystem(void);
	void	GiveClipToSystem(void);
	
public:
	Handle	GetClipCopyFromApp(OSType *type);
		// allocates memory the caller needs to maintain.
		
	void	GiveDataToApp(Handle data, OSType type);
		// deletes existing clip and copies data into a new
		// handle
	
};//end of class declaration

#define SLEEPVAL 9
#define BACKGROUNDSLEEP 0x0FFFFFFF
#define kDITop 0x0050
#define kDILeft 0x0070

#define	rMenuBarID	128
#define rAppleMenu	128
#define	rFileMenu	129
#define	rAboutIDBox	128

#define iAbout	1

#define	iNew	1
#define	iOpen	2
#define	iClose		4
#define	iSave		5
#define	iSaveAs		6
#define	iPageSetup	8
#define	iPrint		9
#define	iQuit	11

#define	rEditMenu		130
#define	iUndo		1
#define	iCut		3
#define	iCopy		4
#define iPaste		5
#define	iClear		6
#define	iSelectAll	8

#endif __DAPPLICATION__