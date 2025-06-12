/*********************************************************************
 * EditRoutines.c
 *
 * Implements a linked-list / virtual memory scheme for hex editing
 *
 * HexEdit, a simple hex editor
 * copyright 1993, Jim Bumgardner
 *********************************************************************/

#include "HexEdit.h"

EditChunk	**gScrapChunk;
short		gScrapCount;
UndoRecord	gUndoRec, gRedoRec;

void LoadFile(EditWindowPtr dWin);
void UnloadFile(EditWindowPtr dWin);
EditChunk **NewChunk(long size, long addr, long filePos, short type);
EditChunk **AppendChunk(EditChunk **list, EditChunk **chunk);
void LoadChunk(EditWindowPtr dWin, EditChunk **cc);
void UnloadLeastUsedChunk(EditWindowPtr dWin);
void UnloadChunk(EditWindowPtr dWin, EditChunk	**cc, Boolean writeFlag);

// Assumes window has just been opened, file is open, fileSize field is correct
void LoadFile(EditWindowPtr dWin)
{
	EditChunk	**nc;
	long		count,chunkSize,pos;
	count = dWin->fileSize;
	pos = 0L;
	while (count) {
		if (count <= (MaxFileRAM - SlushRAM))
			chunkSize = count;
		else
			chunkSize = (MaxFileRAM - SlushRAM);
		count -= chunkSize;
		nc = NewChunk(chunkSize,pos,pos, CT_Original);
		dWin->firstChunk = AppendChunk(dWin->firstChunk, nc);
		pos += chunkSize;
	}
	dWin->curChunk = dWin->firstChunk;
}

void DisposeChunk(EditWindowPtr dWin, EditChunk **cc)
{
	if ((*cc)->loaded) {
		if ((*cc)->loaded)
			UnloadChunk(dWin,cc, false);
		DisposHandle((Handle) cc);
	}
}

void UnloadFile(EditWindowPtr dWin)
{
	EditChunk	**cc,**bc;
	cc = dWin->firstChunk;
	while (cc) {
		bc = (*cc)->next;
		DisposeChunk(dWin, cc);
		cc = bc;
	}
	dWin->firstChunk = dWin->curChunk = NULL;
}


EditChunk **NewChunk(long size, long addr, long filePos, short type)
{
	EditChunk **nc;
	nc = (EditChunk **) NewHandleClear(sizeof(EditChunk));
	if (nc == NULL) {
		ErrorAlert(ES_Caution, "Out of Memory");
		return NULL;
	}
	(*nc)->type = type;
	(*nc)->size = size;
	(*nc)->addr = addr;
	(*nc)->filePos = filePos;
	(*nc)->lastCtr = -1;
	if (type == CT_Unwritten) {
		(*nc)->loaded = true;
		(*nc)->allocSize = size;
		(*nc)->data = NewHandleClear((*nc)->allocSize);
		if ((*nc)->data == NULL) {
			ErrorAlert(ES_Caution, "Out of Memory");
			DisposHandle((Handle) nc);
			return NULL;
		}
	}
	else {
		(*nc)->loaded = false;
		(*nc)->data = NULL;
		(*nc)->allocSize = 0L;
	}
	return nc;
}

EditChunk **AppendChunk(EditChunk **list, EditChunk **chunk)
{
	if (list) {
		register EditChunk	**curChunk;
		curChunk = list;
		while ((*curChunk)->next)
			curChunk = (*curChunk)->next;
		(*curChunk)->next = chunk;
		(*chunk)->prev = curChunk;
		(*chunk)->next = NULL;
	}
	else {
		list = chunk;
		(*chunk)->next = (*chunk)->prev = NULL;
	}
	return list;
}

void SetCurrentChunk(EditWindowPtr dWin, long addr)
{
	register EditChunk **cc;
	cc = GetChunkByAddr(dWin,addr);
	dWin->curChunk = cc;
}

EditChunk **GetChunkByAddr(EditWindowPtr dWin, long addr)
{
	register EditChunk **cc;
	if (dWin->curChunk && addr >= (*dWin->curChunk)->addr)
		cc = dWin->curChunk;
	else	// Otherwise, start from beginning of chain
		cc = dWin->firstChunk;
	while (cc) {
		if (addr < (*cc)->addr+(*cc)->size)
			break;
		else {
			if ((*cc)->next)
				cc = (*cc)->next;
			else
				return cc;
		}
	}
	return cc;
}

short GetByte(EditWindowPtr dWin, long addr)
{
	register EditChunk **cc;
	if ((cc = GetChunkByAddr(dWin,addr)) != NULL) {
		// Correct Chunk
		if (!(*cc)->loaded)
			LoadChunk(dWin,cc);
		if ((*cc)->lastCtr != dWin->useCtr) {
			// Update the Counter
			++dWin->useCtr;
			(*cc)->lastCtr = dWin->useCtr;
		}
		return (unsigned char) (*(*cc)->data)[addr - (*cc)->addr];
	}
	return -1;
}

void LoadChunk(EditWindowPtr dWin, EditChunk **cc)
{
	long	count;
	OSErr	oe;
	short	refNum;
	if ((*cc)->loaded)
		return;
	// Check if we can fit within MaxFileRam, if not, deallocate old chunks
	// until we're ok
	while (dWin->totLoaded+(*cc)->size > MaxFileRAM) {
		UnloadLeastUsedChunk(dWin);
	}
	(*cc)->data = NewHandleClear((*cc)->size);
	if ((*cc)->data == NULL) {
		ErrorAlert(ES_Caution,"Not enough memory");
		(*cc)->allocSize = 0L;
		(*cc)->loaded = false;
	}
	else {
		if ((*cc)->type == CT_Work)
			refNum = dWin->workRefNum;
		else
			refNum = dWin->refNum;
		(*cc)->allocSize = (*cc)->size;
		(*cc)->loaded = true;
		if ((oe = SetFPos(refNum, fsFromStart, (*cc)->filePos)) != noErr)
			OSErrorAlert(ES_Caution,"Seek Error",oe);
		count = (*cc)->size;
		dWin->totLoaded += (*cc)->size;
		if ((oe = FSRead(refNum, &count, *(*cc)->data)) != noErr)
			OSErrorAlert(ES_Caution,"Read Error",oe);
	}
}

void UnloadLeastUsedChunk(EditWindowPtr dWin)
{
	EditChunk	**cc,**oc=NULL;
	oc = cc = dWin->firstChunk;
	while (cc) {
		if ((*cc)->loaded && 
			(!(*oc)->loaded || (*cc)->lastCtr < (*oc)->lastCtr))
			oc = cc;
		cc = (*cc)->next;
	}
	if (oc)
		UnloadChunk(dWin, oc, true);
}

void UnloadChunk(EditWindowPtr dWin, EditChunk	**cc, Boolean writeFlag)
{
	long	count;
	OSErr	oe;

	if (cc && (*cc)->loaded && (*cc)->data) {
		if (writeFlag && (*cc)->type == CT_Unwritten) {
			// Record New Chunks in Work File
			oe = SetFPos(dWin->workRefNum, fsFromStart, dWin->workBytesWritten);
			if (oe) {
				OSErrorAlert(ES_Caution, "SetFPos", oe);
			}
			count = (*cc)->size;
			oe = FSWrite(dWin->workRefNum, &count, *(*cc)->data);
			if (oe) {
				OSErrorAlert(ES_Caution, "FSWrite", oe);
			}
			(*cc)->type = CT_Work;
			(*cc)->filePos = dWin->workBytesWritten;
			dWin->workBytesWritten += count;
		}

		dWin->totLoaded -= (*cc)->size;
		(*cc)->loaded = false;
		DisposHandle((*cc)->data);
		(*cc)->data = NULL;
		(*cc)->allocSize = 0L;
	}
}

void RewriteAddressChain(EditChunk **fc)
{
	EditChunk	**nc;
	// Rewrite Addresses of chunks starting from fc
	nc = (*fc)->next;
	while (nc) {
		(*nc)->addr = (*(*nc)->prev)->addr + (*(*nc)->prev)->size;
		nc = (*nc)->next;
	}
}

void DeleteSelection(EditWindowPtr dWin)
{
	EditChunk **fc,**ec,**nc,**tc;

	if (dWin->endSel == dWin->startSel)
		return;

	// Identify Starting Chunk
	fc = GetChunkByAddr(dWin, dWin->startSel);
	dWin->curChunk = fc;		// Optimize chunk searches

	// Identify Ending Chunk
	ec = GetChunkByAddr(dWin, dWin->endSel);

	// If Chunks are the same
	if (fc == ec) {
		// If chunk is unwritten
		if ((*fc)->type == CT_Unwritten) {
			// Delete Chars from Buffer
			// 12/14 JAB!!!  fixed editing bug
			BlockMove(*(*fc)->data + (dWin->endSel - (*fc)->addr),
					  *(*fc)->data + (dWin->startSel - (*fc)->addr),
					  (*fc)->size - (dWin->endSel - (*fc)->addr));
/*			BlockMove(*(*fc)->data + (dWin->endSel - (*fc)->addr),*/
/*					  *(*fc)->data + (dWin->startSel - (*fc)->addr),*/
/*					  dWin->endSel - dWin->startSel);*/
			(*fc)->size -= dWin->endSel - dWin->startSel;
		}
		else {
			UnloadChunk(dWin, fc, true);
			// Split into two chunks
			nc = NewChunk((*fc)->size - (dWin->endSel - (*fc)->addr), 
							0, 
							(*fc)->filePos + (dWin->endSel - (*fc)->addr), 
							(*fc)->type);
			(*nc)->prev = fc;
			(*nc)->next = (*fc)->next;
			if ((*nc)->next)
				(*(*nc)->next)->prev = nc;
			(*fc)->next = nc;
			(*fc)->size = dWin->startSel - (*fc)->addr;
		}
	}
	else {
		// Truncate end of first Chunk
		(*fc)->size = dWin->startSel - (*fc)->addr;
		// Unlink & Dispose Middle Chunks, If Any
		nc = (*fc)->next;
		while (nc != ec) {
			tc = (*nc)->next;
			DisposeChunk(dWin, nc);
			nc = tc;
		}
		(*ec)->prev = fc;
		(*fc)->next = ec;
		// Truncate beg of end chunk
		if ((*ec)->type == CT_Unwritten) {
			long	offset;
			offset = dWin->endSel - (*ec)->addr;
			BlockMove(*(*ec)->data, *(*ec)->data+offset, (*ec)->size - offset);
			(*ec)->size -= offset;
		}
		else {
			long	offset;
			offset = dWin->endSel - (*ec)->addr;
			UnloadChunk(dWin, ec, true);
			(*ec)->filePos += offset;
			(*ec)->size -= offset;
		}
	}

	dWin->fileSize -= (dWin->endSel - dWin->startSel);

	RewriteAddressChain(fc);

	// Modify Current Selection such that  endSel = firstSel
	dWin->endSel = dWin->startSel;
	dWin->dirtyFlag = true;
}

// Assumes selection point is already 0 chars wide...
void InsertCharacter(EditWindowPtr dWin, short charCode)
{
	EditChunk **fc,**ec,**nc,**tc;

	// !! Remember Current State for Undo


	// Insert Character Into List
	//	Identify current chunk - optimize so that if char is between
	//		chunks, pick the unwritten one of the two...

	// Identify Starting Chunk
	fc = GetChunkByAddr(dWin, dWin->startSel);

	//	Identify current chunk - optimize so that if char is between
	//	chunks, pick the unwritten one of the two... - this way, if I keep typing
	//  characters, I won't generate a bunch of 1 byte chunks.
	if (dWin->startSel - (*fc)->addr == 0 &&
		(*fc)->prev && (*fc)->type != CT_Unwritten &&
		(*(*fc)->prev)->type == CT_Unwritten) {
		fc = (*fc)->prev;
	}
	dWin->curChunk = fc;		// Optimize chunk searches

	//	If current chunk is not unwritten
	if ((*fc)->type != CT_Unwritten) {
		// Unload it
		UnloadChunk(dWin, fc, true);

		if (dWin->startSel > (*fc)->addr) {

			// Split into two chunks
			if (dWin->startSel < (*fc)->addr + (*fc)->size) {
				ec = NewChunk((*fc)->size - (dWin->startSel - (*fc)->addr), 
								0, 
								(*fc)->filePos + (dWin->startSel - (*fc)->addr), 
								(*fc)->type);
				(*ec)->prev = fc;
				(*ec)->next = (*fc)->next;
				if ((*ec)->next)
					(*(*ec)->next)->prev = ec;
				(*fc)->next = ec;
			}
			else
				ec = (*fc)->next;

			(*fc)->size = dWin->startSel - (*fc)->addr;
		}
		else {
			ec = fc;
			fc = (*fc)->prev;
		}

		// Add New unwritten chunk in middle with 0 size
		nc = NewChunk(0,0,0,CT_Unwritten);
		if (fc) {
			(*fc)->next = nc;
			(*nc)->addr = (*fc)->addr + (*fc)->size;
		}
		else
			dWin->firstChunk = nc;
		if (ec)
			(*ec)->prev = nc;
		(*nc)->prev = fc;
		(*nc)->next = ec;
		// current chunk = new chunk
		dWin->curChunk = nc;
		fc = nc;
	}

	//	Expand Ptr if Necessary
	if ((*fc)->allocSize <= (*fc)->size) {
		(*fc)->allocSize += AllocIncr;		// !! consider expanding as size goes up
		SetHandleSize((*fc)->data,(*fc)->allocSize);
	}

	// Make Room for Character if necessary
	if (dWin->startSel < (*fc)->addr + (*fc)->size)
		BlockMove(*(*fc)->data + (dWin->startSel - (*fc)->addr), 
				  *(*fc)->data + (1+(dWin->startSel - (*fc)->addr)), 
				  (*fc)->addr + (*fc)->size - dWin->startSel);

	//	Insert Char into buffer
	(*(*fc)->data)[dWin->startSel - (*fc)->addr] = charCode;

	//	Update Fields in this chunk
	(*fc)->size++;
	dWin->fileSize++;

	// Set Dirty Flag
	dWin->dirtyFlag = true;

	//	Update addr fields of following chunks
	RewriteAddressChain(fc);

	// Increment current Selection
	dWin->startSel++;
	dWin->endSel++;


	// Update Display
	ScrollToSelection(dWin,dWin->startSel,true, false);
}

void ReleaseEditScrap(EditWindowPtr dWin, EditChunk ***scrap)
{
	EditChunk	**cc,**bc;
	cc = *scrap;
	while (cc) {
		bc = (*cc)->next;
		DisposeChunk(dWin, cc);
		cc = bc;
	}
	*scrap = NULL;
}

// High Level Copy
void CopySelection(EditWindowPtr dWin)
{
	CopyOperation(dWin, &gScrapChunk);
	if (gScrapChunk) {
		// Copy to Desk Scrap
		ZeroScrap();
		HLock((*gScrapChunk)->data);
		PutScrap((*gScrapChunk)->size, 'TEXT', *(*gScrapChunk)->data);
		HUnlock((*gScrapChunk)->data);
		gScrapCount = ScrapInfo.scrapCount;
		(*gScrapChunk)->lastCtr = 0;	// Flag as internal
	}
}



void CopyOperation(EditWindowPtr dWin, EditChunk ***scrapChunk)
{
	EditChunk	**fc,**ec,**nc,**tc;
	// Unload current scrap
	ReleaseEditScrap(dWin, scrapChunk);

	// Copy current selection into scrapChunk
	// Identify Starting Chunk
	fc = GetChunkByAddr(dWin, dWin->startSel);
	dWin->curChunk = fc;		// Optimize chunk searches

	// Identify Ending Chunk
	ec = GetChunkByAddr(dWin, dWin->endSel);

	// If Chunks are the same
	nc = NewChunk(dWin->endSel - dWin->startSel,
				  0,
				  0,
				  CT_Unwritten);
	if (nc == NULL)
		return;

	*scrapChunk = nc;

	if (fc == ec) {
		LoadChunk(dWin, fc);
		BlockMove(*(*fc)->data + (dWin->startSel - (*fc)->addr),
				  *(*nc)->data,
				  (*nc)->size);
	}
	else {
		// First Chunk to End
		tc = fc;
		LoadChunk(dWin, tc);
		BlockMove(*(*tc)->data + (dWin->startSel - (*tc)->addr),
				  *(*nc)->data,
				  (*tc)->size - (dWin->startSel - (*tc)->addr));
		tc = (*tc)->next;

		// Middle Chunks, If Any
		while (tc != ec) {
			LoadChunk(dWin, tc);
			BlockMove(*(*tc)->data,
					  *(*nc)->data + ((*tc)->addr - dWin->startSel),
					  (*tc)->size);
			tc = (*tc)->next;
		}

		// Last Chunk
		LoadChunk(dWin, tc);
		BlockMove(*(*tc)->data,
				  *(*nc)->data + ((*tc)->addr - dWin->startSel),
				  dWin->endSel - (*tc)->addr);
	}
}

void CutSelection(EditWindowPtr dWin)
{
	RememberOperation(dWin, EO_Cut, &gUndoRec);
	CopyOperation(dWin, &gScrapChunk);		// Copy into paste buffer
	DeleteSelection(dWin);
	dWin->dirtyFlag = true;
	ScrollToSelection(dWin,dWin->startSel,true, false);
}

// High Level Paste
void PasteSelection(EditWindowPtr dWin)
{
	RememberOperation(dWin, EO_Paste, &gUndoRec);
	PasteOperation(dWin, gScrapChunk);
	dWin->dirtyFlag = true;
	ScrollToSelection(dWin,dWin->startSel,true, false);
}

Boolean HexConvertScrap(EditWindowPtr dWin, EditChunk **scrapChunk)
{
	Handle	rh=NULL;
	Ptr		sp,dp,esp;
	short	val;
	Boolean	loFlag;

	rh = NewHandle((*scrapChunk)->size);
	if (rh == NULL) {
		ErrorAlert(ES_Caution, "Not enough memory");
		return false;
	}
	HLock(rh);
	HLock((*scrapChunk)->data);
	sp = *(*scrapChunk)->data;
	esp = sp + (*scrapChunk)->size;
	dp = *rh;
	loFlag = false;
	for (; sp < esp; ++sp) {
		if (*sp == '0' && *(sp+1) == 'x') {
			loFlag = 0;
			++sp;
			continue;
		}
		if (isspace(*sp) || ispunct(*sp)) {
			loFlag = 0;
			continue;
		}
		if (*sp >= '0' && *sp <= '9')
			val = *sp - '0';
		else if (*sp >= 'A' && *sp <= 'F')
			val = 0x0A + (*sp - 'A');
		else if (*sp >= 'a' && *sp <= 'f')
			val = 0x0A + (*sp - 'a');
		else
			goto HexError;
		if (loFlag) {
			*(dp-1) = (*(dp-1) << 4) | val;
			loFlag = 0;
		}			
		else {
			*dp = val;
			++dp;
			loFlag = 1;
		}
	}
	if (dp - *rh == 0)
		goto HexError;
	(*scrapChunk)->size = dp - *rh;
	HUnlock(rh);
	HUnlock((*scrapChunk)->data);
	BlockMove(*rh, *(*scrapChunk)->data, (*scrapChunk)->size);
	DisposHandle(rh);
	(*scrapChunk)->lastCtr = 0;		// Mark as Internal
	return true;
HexError:
	HUnlock(rh);
	HUnlock((*scrapChunk)->data);
	ErrorAlert(ES_Caution, "Only valid Hex values may be pasted here");
	DisposHandle(rh);
	return false;
}

void PasteOperation(EditWindowPtr dWin, EditChunk **scrapChunk)
{
	EditChunk **fc,**ec,**nc;

	// Hex Pasting Mode for Outside Pastes
	if (dWin->editMode == EM_Hex && (*gScrapChunk)->lastCtr == 1) {
		if (!HexConvertScrap(dWin,scrapChunk))
			return;
	}

	// Create duplicate scrap attached to nc->nec
	nc = NewChunk((*scrapChunk)->size,
					0,
					0,
					CT_Unwritten);
	if (nc == NULL)
		return;

	BlockMove(*(*scrapChunk)->data,
			  *(*nc)->data,
			  (*nc)->size);

	DeleteSelection(dWin);
	// Insert paste buffer into selStart

	fc = GetChunkByAddr(dWin, dWin->startSel);
	if ((*fc)->addr < dWin->startSel) {
		// Split 'em up
		// Unload it
		UnloadChunk(dWin, fc, true);

		// Split into two chunks
		if (dWin->startSel < (*fc)->addr + (*fc)->size) {
			ec = NewChunk((*fc)->size - (dWin->startSel - (*fc)->addr), 
							0, 
							(*fc)->filePos + (dWin->startSel - (*fc)->addr), 
							(*fc)->type);
			(*ec)->prev = fc;
			(*ec)->next = (*fc)->next;
			if ((*ec)->next)
				(*(*ec)->next)->prev = ec;
		}
		else
			ec = (*fc)->next;

		(*fc)->next = ec;
		(*fc)->size = dWin->startSel - (*fc)->addr;
	}
	else {
		ec = fc;
		fc = (*fc)->prev;
	}

	// Insert fc->nc->ec
	if (fc) {
		(*fc)->next = nc;
		(*nc)->prev = fc;
		(*nc)->addr = (*fc)->addr + (*fc)->size;
	}
	else {
		dWin->firstChunk = nc;
		(*nc)->addr = 0L;
	}

	if (ec) {
		(*nc)->next = ec;
		(*ec)->prev = nc;
	}

	// Correct addresses
	RewriteAddressChain(nc);

	// Reset Selection
	dWin->startSel = dWin->endSel = (*nc)->addr + (*nc)->size;

	// Update other stuff
	dWin->fileSize += (*scrapChunk)->size;
	dWin->dirtyFlag = true;
}

void ClearSelection(EditWindowPtr dWin)
{
	RememberOperation(dWin, EO_Clear, &gUndoRec);
	DeleteSelection(dWin);
	dWin->dirtyFlag = true;
	ScrollToSelection(dWin, dWin->startSel, true, false);
}

// Remember current state for Undo of following operation
void RememberOperation(EditWindowPtr dWin, short opType, UndoRecord *ur)
{
	// Forget Last stuff
	if (ur == &gRedoRec) {
		// Reset menu text to Redo
		Str31	menuStr;
		GetItem(gEditMenu, EM_Undo, menuStr);
		if (menuStr[1] == 'R')
			BlockMove("Un", &menuStr[1], 2);
		else
			BlockMove("Re", &menuStr[1], 2);
		SetItem(gEditMenu, EM_Undo, menuStr);
	}
	else {
		Str31	undoStr;
		Str31	menuStr;
		GetIndString(undoStr, UndoSTRs, opType);
		BlockMove("\pUndo ", menuStr, 6);
		BlockMove(&undoStr[1], &menuStr[6], undoStr[0]);
		menuStr[0] += undoStr[0];
		SetItem(gEditMenu, EM_Undo, menuStr);
	}

	ReleaseEditScrap(dWin, &ur->undoScrap);
	// Clear Undo Stuff
	ur->undoScrap = NULL;
	ur->type = opType;
	ur->startSel = dWin->startSel;
	ur->endSel = dWin->endSel;
	ur->fileSize = dWin->fileSize;
	ur->window = dWin;
	CopyOperation(dWin, &ur->undoScrap);
	(*ur->undoScrap)->lastCtr= 0;
	dWin->lastTypePos = -1;	// Clear Special Editing Modes
	dWin->loByteFlag = false;
}

void UndoOperation()
{
	EditWindowPtr	dWin = gUndoRec.window;

	if (gUndoRec.type == 0)
		return;
	if (dWin != (EditWindowPtr) FrontWindow())
		SelectWindow((WindowPtr) dWin);

	switch (gUndoRec.type) {
	case EO_Typing:
	case EO_Paste:
	case EO_Insert:
		dWin->startSel = gUndoRec.startSel;
		dWin->endSel = dWin->fileSize - (gUndoRec.fileSize - gUndoRec.endSel);
		RememberOperation(dWin, EO_Delete, &gRedoRec);
		DeleteSelection(dWin);
		PasteOperation(dWin, gUndoRec.undoScrap);
		break;
	case EO_Cut:
	case EO_Clear:
	case EO_Delete:
		dWin->startSel = dWin->endSel = gUndoRec.startSel;
		RememberOperation(dWin, EO_Insert, &gRedoRec);
		PasteOperation(dWin, gUndoRec.undoScrap);
		break;
	}

	ReleaseEditScrap(dWin, &gUndoRec.undoScrap);
	gUndoRec = gRedoRec;
	gRedoRec.undoScrap = NULL;

	ScrollToSelection(dWin,dWin->startSel,true, false);
}

