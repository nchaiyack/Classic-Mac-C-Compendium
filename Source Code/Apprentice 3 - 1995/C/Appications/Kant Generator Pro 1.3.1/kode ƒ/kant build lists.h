#ifndef __MY_BUILD_LISTS_H__
#define __MY_BUILD_LISTS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitTheBuildLists(void);
extern	ListHandle GetReferenceListHandle(void);
extern	ListHandle GetInstantListHandle(void);
extern	void GetReferenceListRect(Rect *theRect);
extern	void GetInstantListRect(Rect *theRect);
extern	Boolean ReferenceListActiveQQ(void);
extern	Boolean InstantListActiveQQ(void);
extern	Boolean HighlightedReferenceInterestingQQ(void);
extern	Boolean SetHighlightedReferenceInteresting(Boolean isInteresting);
extern	void SetReferenceListActive(Boolean redraw);
extern	void SetInstantListActive(Boolean redraw);
extern	void SetMaxReferenceDisplay(short numRefs, Boolean redraw);
extern	void SetHighlightedReference(short refNum);
extern	void SetHighlightedInstant(short instantNum);
extern	short GetNumberOfReferences(void);
extern	short GetNumberOfInstants(void);
extern	short GetNumberOfVisibleReferences(void);
extern	short GetNumberOfVisibleInstants(void);
extern	void GetIndReferenceName(Str255 theStr, short index);
extern	void GetIndInstantName(Str255 theStr, short index);
extern	short GetHighlightedReference(void);
extern	short GetHighlightedInstant(void);
extern	void GetHighlightedReferenceName(Str255 theStr);
extern	void GetHighlightedInstantName(Str255 theStr);
extern	Boolean DeleteHighlightedReference(void);
extern	Boolean DeleteHighlightedInstantiation(void);
extern	void AddReferenceToList(Str255 theStr, Boolean isInteresting, Boolean autoSelect);
extern	void AddInstantToList(Str255 theStr, Boolean autoSelect);
extern	void EditReferenceInList(short index, Str255 newStr, Boolean isInteresting);
extern	void EditInstantInList(short index, Str255 newStr);
extern	void SetBuildListRects(WindowRef theWindow, short hSpace, short vSpace, short headerHeight);
extern	void CreateBuildLists(WindowRef theWindow);
extern	OSErr BuildReferenceListHandle(FSSpec theFS, Boolean redraw);
extern	OSErr BuildInstantListHandle(FSSpec theFS, Boolean redraw);
extern	void UpdateBuildLists(WindowRef theWindow);
extern	void ResizeBuildLists(WindowRef theWindow, short hSpace, short vSpace, short headerHeight);
extern	void GetGrowSizeTheLists(Rect *sizeRect, short hSpace, short vSpace, short headerHeight);
extern	void DisposeBuildLists(void);

#ifdef __cplusplus
}
#endif

#endif
