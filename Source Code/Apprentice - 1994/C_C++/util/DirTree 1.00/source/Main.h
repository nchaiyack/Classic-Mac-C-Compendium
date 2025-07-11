/* Main.h */

#ifndef Included_Main_h
#define Included_Main_h

#include "Screen.h"
#include "Files.h"
#include "Scroll.h"

struct Node;
typedef struct Node Node;

struct AliasVec;
typedef struct AliasVec AliasVec;

Node*							ListFromDirectory(FileSpec* Where);
Node*							ListFromDirectoryStart(FileSpec* Where);
void							DisposeList(Node* List);
Node*							FindMatchingItem(Node* List, FileSpec* Target,
										OrdType* OutX, OrdType* OutY);
void							ResolveAliases(Node* List);
void							DisposeAliasList(void);
void							FillInSizeFields(Node* List, OrdType FrameX, OrdType FrameY,
										OrdType* WidthOut, OrdType* HeightOut);
void							RedrawArea(Node* List);
void							RedrawAreaStart(MyBoolean DoAliases);
void							DeselectAll(Node* List);
MyBoolean					MouseDownSelect(Node* List, OrdType X, OrdType Y);
void							HScrollHook(long Parameter, ScrollType How, void* DontCare);
void							VScrollHook(long Parameter, ScrollType How, void* DontCare);
void							UpdateProcedure(void* Refcon);
void							JumpSymbolicLink(void);
void							ReturnToPreviousLink(void);
void							HiliteLinksToCurrent(void);
void							ShowSpecifiedItem(Node* Item);
void							AdvanceToNext(void);
void							AdvanceToPrevious(void);
void							EnterSubLevel(void);
void							ExitLevel(void);
void							GoUp(MyBoolean JumpToTop);
void							GoDown(MyBoolean JumpToBottom);
void							Find(void);
void							ShowFileInfoForSelection(Node* Item);

#endif
