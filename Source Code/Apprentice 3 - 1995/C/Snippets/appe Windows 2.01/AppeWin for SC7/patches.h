// File "patches.h" - 

#ifndef ____PATCHES_HEADER_____
#define ____PATCHES_HEADER_____

// * **************************************************************************** * //
// * **************************************************************************** * //

// Setup calling information for the routines we need
typedef pascal void (*ExitToShellProcPtr)(void);
typedef pascal WindowPtr (*NewWindowProcPtr)(Ptr wStorage, Rect *bounds, StringPtr title,
		Boolean vis, short procID, WindowPtr behind, Boolean close, long refCon);

// * **************************************************************************** * //
// * **************************************************************************** * //
// Function Prototypes

void PatchExitToShell(void);
pascal void MyExitToShell(void);

void PatchNewWindow(void);
pascal WindowPtr MyNewWindow(Ptr wStorage, Rect *bounds, StringPtr title, 
		Boolean vis, short procID, WindowPtr behind, Boolean close, long refCon);

#endif  ____PATCHES_HEADER_____
