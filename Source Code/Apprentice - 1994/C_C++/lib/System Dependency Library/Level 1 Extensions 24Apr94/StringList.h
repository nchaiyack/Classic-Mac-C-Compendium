/* StringList.h */

#ifndef Included_StringList_h
#define Included_StringList_h

/* StringList module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* Memory */
/* DataMunging */
/* Scroll */
/* EventLoop */
/* Array */

/* StringList provides a scrolling list of text strings which can be selected. */

#include "Screen.h"
#include "EventLoop.h"

struct StringListRec;
typedef struct StringListRec StringListRec;

#define StringListDoAllowMultipleSelection (True)
#define StringListDontAllowMultipleSelection (False)

/* forward declaration for Array */
struct ArrayRec;

/* allocate a new, empty string list.  Title is null terminated */
StringListRec*		NewStringList(WinType* Window, OrdType X, OrdType Y,
										OrdType Width, OrdType Height, FontType Font, FontSizeType FontSize,
										MyBoolean AllowMultipleSelection, char* Title);

/* dispose the string list and all of the items in it */
void							DisposeStringList(StringListRec* List);

/* find out where the string list is located */
OrdType						GetStringListXLoc(StringListRec* List);
OrdType						GetStringListYLoc(StringListRec* List);
OrdType						GetStringListWidth(StringListRec* List);
OrdType						GetStringListHeight(StringListRec* List);

/* what font is being used to display the string list */
FontType					GetStringListFont(StringListRec* List);

/* what point size is being used to display the string list */
FontSizeType			GetStringListFontSize(StringListRec* List);

/* how many lines of text are visible in the string list */
long							GetStringListNumVisibleLines(StringListRec* List);

/* change the location of the string list */
void							SetStringListLoc(StringListRec* List, OrdType X, OrdType Y,
										OrdType Width, OrdType Height);

/* change the font being used to display the string list */
void							SetStringListFontInfo(StringListRec* List, FontType Font,
										FontSizeType FontSize);

/* do a complete redraw of the string list */
void							RedrawStringList(StringListRec* List);

/* return a count of the number of selected items in the string list */
long							GetStringListHowManySelectedItems(StringListRec* List);

/* return an Array containing a list of the References of all selected items */
struct ArrayRec*	GetListOfSelectedItems(StringListRec* List);

/* add a new element to the string list.  String is the string to be added to the */
/* list, and OurReference is the reference pointer that identifies the item.  */
/* BeforeThisReference is the item to insert it before.  if it is NIL then the */
/* item is appended to the list.  NIL can be passed for the name if the name isn't */
/* known yet.  Returns True if insertion was successful. */
MyBoolean					InsertStringListElement(StringListRec* List, char* String,
										void* BeforeThisReference, void* OurReference, MyBoolean Redraw);

/* change the name of a string list element associated with the specified reference */
void							ChangeStringListElementName(StringListRec* List, char* NewName,
										void* Reference);

/* remove an element from the string list */
void							RemoveStringListElement(StringListRec* List, void* Reference,
										MyBoolean Redraw);

/* see if the specified location is within the string list box */
MyBoolean					StringListHitTest(StringListRec* List, OrdType X, OrdType Y);

/* do a mouse down in the string list to select items.  returns True if it was */
/* a double click. */
MyBoolean					StringListMouseDown(StringListRec* List, OrdType X, OrdType Y,
										ModifierFlags Modifiers);

/* select (hilite) the specified element in the list */
void							SelectStringListElement(StringListRec* List, void* Reference);

/* deselect the specified element in the list */
void							DeselectStringListElement(StringListRec* List, void* Reference);

/* enable the scrollbar display in the list */
void							EnableStringList(StringListRec* List);

/* disable the scrollbar display in the list */
void							DisableStringList(StringListRec* List);

/* deselect all of the items in the string list that are selected */
void							DeselectAllStringListElements(StringListRec* List);

/* make sure selection is visible in the window */
void							MakeStringListSelectionVisible(StringListRec* List);

#endif
