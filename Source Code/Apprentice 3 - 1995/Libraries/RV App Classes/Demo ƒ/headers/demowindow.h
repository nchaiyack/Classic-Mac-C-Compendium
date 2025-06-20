#pragma once

class demowindow : public window, public picture
{
	public:
		enum
		{
			iUndo				= 1,
			iCut				= iUndo + 2,
			iCopy,
			iPaste,
			iClear
		};
		demowindow( short rsrcID, short PICT_ID);
		demowindow( short rsrcID, const FSSpec &theFile);

		virtual ~demowindow();
		virtual void HandleUpdate();

		virtual void HandleMouseDownInContent( const EventRecord &theEvent);

		virtual void HandleMenuSelection( long selection);

		void HandleEditMenu( short item);

	private:
		virtual OSErr HandleDrop( DragReference theDragRef);
};
