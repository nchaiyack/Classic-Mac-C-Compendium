/* ZoomCode.h -
	Note: this code is from develop magazine, issue 17. It is referreed by Apple and
	may be freely used in Macintosh programs.
 */
typedef void (*CalcIdealDocumentSizeProcPtr)(WindowPtr theWindow, Rect *idealContentSize);

void ZoomTheWindow(WindowPeek theWindow, short zoomState,
					CalcIdealDocumentSizeProcPtr calcRoutine);
