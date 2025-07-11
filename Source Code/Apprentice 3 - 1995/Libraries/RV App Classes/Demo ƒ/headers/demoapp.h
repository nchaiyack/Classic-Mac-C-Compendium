#pragma once

enum
{
	kNewWindowItem	= 1,
	kOpenItem		= kNewWindowItem + 2,
	kCloseItem,
	kSaveItem,
	kSaveAsItem,
	kPageSetupItem	= kSaveAsItem + 2,
	kPrintItem,
	kQuitItem		= kPrintItem + 2
};

class demoapp : public application, public dragreceiver
{
	public:
		demoapp();

		Boolean HandleAppleMenu( short theItem);
		Boolean HandleFileMenu( short theItem);

	protected:
		virtual Boolean HandleMenuSelection( long selection);
		
	private:
		virtual OSErr DragEnterWindow( WindowPtr theWindow, DragReference theDragRef);
		virtual OSErr DragInWindow( WindowPtr theWindow, DragReference theDragRef);
		virtual OSErr DragLeaveWindow( WindowPtr theWindow, DragReference theDragRef);

		virtual OSErr theRealReceiveHandler( WindowPtr theWindow, DragReference theDragRef);
};

class OAPP_Handler : private appleeventhandler
{
	public:
		OAPP_Handler();
	
	private:
		virtual OSErr theRealHandler( AppleEvent *theAppleEvent, AppleEvent *reply);
};

class ODOC_PDOC_Handler : private appleeventhandler
{
	public:
		ODOC_PDOC_Handler( AEEventID theAEEventID);
	
	private:
		virtual OSErr theRealHandler( AppleEvent *theAppleEvent, AppleEvent *reply);
		
		const AEEventID theID;
};

class QUIT_Handler : private appleeventhandler
{
	public:
		QUIT_Handler();
	
	private:
		virtual OSErr theRealHandler( AppleEvent *theAppleEvent, AppleEvent *reply);
};
