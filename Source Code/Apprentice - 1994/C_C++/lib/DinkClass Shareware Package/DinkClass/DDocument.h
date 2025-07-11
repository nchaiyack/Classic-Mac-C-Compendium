/*
	File:		DDocument.h

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <4>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <3>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <2>	  8/9/92	MTG		merging in changes from the think C version

	To Do:
*/


// This is the Class declatation for the DDocument class
#ifndef __DDOCUMENT__
#define __DDOCUMENT__

#include "DListStuff.h"
#include "DWindow.h"

#ifdef THINK_C
	//nothing, its already included in macincludes
#else
	// for MPW
	#include <StandardFile.h>
#endif

class DEventHandler;

class DDocument : public DEventHandler
{
protected:
	
	Handle	fDataHandle;
		// a place to store data (or not, I tend not to use it)
		
	OSType	fCreator;
	OSType	fFileType;
		// The types of the existing file associated with this 
		// document.  They may be redundent, but for now, thats ok
						
	Boolean fKMNxtCalledFromWindow;
		// flag indicating when the document's KillMeNext method gets
		// called as a result of the document's window killing itself.					
public:

	StandardFileReply *fFileReply;
		// the FSSpectPtr used for system 7 savyness.

	short fFileRef;
		// to keep around if document needs to do a lot of I/O
		// otherwise it would be just as good to "get" the refNum
		// in a local variable each time I/O is done.  Its also
		// used as a flag to indicate that a save as is needed
		// this is so when mFileRef == 0

	DWindow *fDWindow;
		// refrence to the window rendering the data within the 
		// document.  It is used in support of the close document
		// flow.
		
	Boolean	fNeedToSave;
		// Not much to say about this one. Set it TRUE if data to in 
		// document has been changed.
		
	DDocument(void);
			// Default constructor, sets simple member vars to NULL.
	
	~DDocument(void);
			// Default Destructor, Makes shure that the File is closed
			// and alocated memery is freed.
		
	virtual 	DDocument* Init( Boolean OpenFromFile);
			// initialize the object if OpenFromFile is TRUE then
			// go through the standardgetfile sequence, otherwise
			// open up a blank document.

	virtual void 	AEInitDoc(FSSpec *theFSS);
			// initialize the object Open the file then
			// set up the file related members of DDocument.

	virtual 	Boolean DoSaveAs(void);
			// Closes file if open and uses sfputfile 
			// to locate and open a new file frow writting too
			
	virtual DWindow* 	MakeWindow(Boolean hasColorWindows);
			// makes whatever window needed, if computer has color
			// support then creat color quickdraw windows, otherwise 
			// make the old style windows.

	virtual Boolean WindowClosed(DWindow *deadWindow);
			// the fDwindow calls this when it gets closed
			// its intended to develope to support multi-window
			// documents in a manner similare to the Eventhandler
			// stuff in the DApplication class
			
protected:

	virtual Boolean KillMeNext(void);
		// Do LOTs of house keeping in association with Close
		// and WindowClosed
		
	short WantToSave(WindowPtr theWindow);
		// Pull up that standard dialog withe the window title
		// or something in it.
	
public:
					
	virtual void HandleMenuChoice(short menuID, short menuItem);			
	virtual void SetUpMenues(void);

protected:

	virtual void DoUndo(void);
	virtual void DoPaste(void);
	virtual void DoCopy(void);
	virtual void DoClear(void);
	virtual void DoCut(void);
	virtual void DoSellectAll(void);

 	void 			SellectFile(StandardFileReply* reply);
 		// Dose the standard ui get file stuff Supports
 		// opening existing files in the Init method
 		
	Boolean 		OpenFile(FSSpec *fileSpec);
		// opens the file, calls ReadData, and sets fRefNum
		// if its approperate.  This gets called from the Init
		// method if OpenExistingFile is true.
		
	virtual OSErr	ReadData(short refNum, long *size);
		// reads the data and places the data wherever its
		// suposed to go.  Should be over ridden to place any
		// application spacific data in the correct place, and 
		// do any filtering or processing to get the data in a
		// good format.
		
	Boolean 		SaveFile(void);
		// Save the file in place over the existing file
		// or if fRefNum do the normal save prosses
		
	virtual OSErr 	WriteData(short refNum);
		// Writes data to file, big da!
			
};// end of class declaration for DDocument


#define rWantToSave	500
#define	iYes	1
#define	iNo		3
#define	iCancel	2



#endif __DDOCUMENT__
