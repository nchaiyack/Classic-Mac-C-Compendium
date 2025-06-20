/* Menu.h
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */

/* kFileMenu -  items
 */
enum {
	kOpenI = 1,
	kCloseI,
	kSaveI = kCloseI + kSkipGrayLine,
	kSaveAsI,
	kPageSetupI = kSaveAsI + kSkipGrayLine,
	kPrintI,
	kQuitI = kPrintI + kSkipGrayLine
};

/* kEditMenu -  items
 */
enum {
	kUndoI = 1,
	kCutI = kUndoI + kSkipGrayLine,
	kCopyI,
	kPasteI,
	kClearI
};

/* kRandomDotMenu - items
 */
enum{
	kGrayI = 1,
	kBWStereoI,
	kGrayStereoI,
	kShimmerI
};

void DoQuit(void);
void DoOpen(void);
void DoClose(void);

void DoCloseWin(WindowPtr win);

void GoMenu(LongInt item);
void InitHelpItem(void);
