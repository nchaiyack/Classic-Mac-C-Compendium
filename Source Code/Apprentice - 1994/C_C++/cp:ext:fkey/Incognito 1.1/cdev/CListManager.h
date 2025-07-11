#ifndef __CLISTMANAGER__
#define __CLISTMANAGER__

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif

#ifndef __LISTS__
#include <Lists.h>
#endif

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

class CListManager : public SingleObject
{
	public:
	
				CListManager(short dialogItem, DialogPtr theDialog, short width = 1, short height = 0);
				~CListManager();

		void	AddItem(StringPtr theItem, Cell theCell);
		void	RemoveItem(Cell theCell);
		void	RemoveItem(StringPtr theItem);
		Boolean GetItem(StringPtr theData, Cell &theCell);
		void	SetItem(StringPtr theString, Cell theCell);
		Boolean InsertItem(StringPtr theItem);
		
		Boolean	FindItem(StringPtr theItem, Cell &theCell);
		Boolean	FindNextSelection(Cell &theCell);

		/*
			iterators. Call GetFirstItem to get the cell
			and data of the first item; call GetNextItem to
			iterate over the rest. They return true if the item is
			real.
		*/
		Boolean	GetFirstItem(StringPtr theItem, Cell &theCell);
		Boolean	GetNextItem(StringPtr theItem, Cell &theCell);
																
		void	DeselectList();
		void	SelectItem(StringPtr theItem);
		
		Boolean	IsSelection();
		Boolean	IsSelection(Cell &theCell);					// is an item selected
		Boolean	IsItemPresent(StringPtr theItem);
		Boolean	IsItemSelected(Cell &theCell);
		Boolean	IsItemSelected(StringPtr theItem);
		Boolean	GetCurrentSelection(StringPtr theItem);
		void	SelectItem(Cell &theCell);
		void	DeselectItem(Cell &theCell);
		void	DeleteSelection(void);

		void	Update();
		void	Activate();
		void	Deactivate();
		void	AutoScroll();
		void	Empty();
		void	DoDraw(Boolean drawStuff = true);
		Boolean	IsEmpty() {return !GetHeight();}
		Boolean	Click(Point thePoint, short modifiers);
	
		inline	short	GetHeight();
		inline	short	GetWidth();
	
		void	SetList(ListHandle theList) {fTheList = theList;}
		Boolean BinarySearchInsensitive(StringPtr theItem, Cell &theCell);
		ListHandle	GetList() {return fTheList;}

	protected:
	
		ListHandle	fTheList;

		/*
			Compare function for LFind speediness. Override if you want, since
			this only checks the whole string. In theory, you could use this 2
			implement user-typed selections.
		*/
		short	CompareInfo(Cell theCell, short offset, short length, StringPtr theData);
		
		/*
			BinarySearch returns true if the item isn't in the list, false if it
			is. Kinda backwards, but hey. It's the MAIN searching procedure, used
			both by FindItem and InsertItem. Speedy, I guess. It was just nicer to
			have this binary search instead of Ye Olde Linear search that LSearch
			uses.
		*/
		Boolean	BinarySearch(StringPtr theItem, Cell &theCell);


};
#endif
