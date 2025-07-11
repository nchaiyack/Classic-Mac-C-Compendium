/* RandomDotWinCommands.c
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include "RandomDotMain.h"
#include "RandomDotRes.h"

#include "RandomDotWinCommands.h"
#include "Error.h"
#include "RandomDot.h"
#include "RandomDotWin.h"
#include "Menu.h"
#include "Utils.h"

/* DoRandomDotSave - 
 */
void DoRandomDotSave(void){
	RandomDotWindowPtr	win;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
		if(NOT (bSaved & win->flags)){
			DoRandomDotSaveAs();
		}
	}
}

/* RandomDotPicHandle - return whole window as a pict.
 */
static PicHandle RandomDotPicHandle(RandomDotWindowPtr win){
	RgnHandle	saveClip;
	PicHandle	ph;

	saveClip = NewRgn();
	GetClip(saveClip);
	ClipRect(&win->frame);
	ph = OpenPicture(&win->frame);
	RandomDotCopybitsWin(win, &win->frame);
	ClosePicture();
	SetClip(saveClip);
	DisposeRgn(saveClip);
	return ph;
}

/* DoRandomDotSaveAs - 
 */
void DoRandomDotSaveAs(void){
	OSErr				errCode, modCode;
	Integer				ref;
	LongInt				len;
	RandomDotWindowPtr	win;
	StandardFileReply	reply;
	Str255				s;
	PicHandle			ph;
	FInfo				fInfo;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
		GetIndString(s, kMainStrs, (bShowGray == (win->flags & kShow)) ? kSaveImageS : kSaveStereoGramS );
		StandardPutFile(s, win->fs.name, &reply);
		if(reply.sfGood){
			modCode = -1;
			errCode = noErr;
			ref = -1;
			if(NIL == (ph = RandomDotPicHandle(win)) || NIL == *ph || EmptyRect(&(**ph).picFrame)){
				errCode = memFullErr;
			}else{
				if(NOT reply.sfReplacing){
					errCode = modCode = FSpCreate(&reply.sfFile, 'ttxt', 'PICT', reply.sfScript);
					if(noErr == errCode){ errCode = FSpOpenDF(&reply.sfFile, fsRdWrPerm, &ref); }
				}else{
					errCode = FSpGetFInfo(&reply.sfFile, &fInfo);
					if(noErr == errCode && fInfo.fdType != 'PICT'){ errCode = ePictOnly; }
					if(noErr == errCode){ errCode = FSpOpenDF(&reply.sfFile, fsRdWrPerm, &ref); }
					if(noErr == errCode){ errCode = modCode = SetEOF(ref, 0); }
				}

				/* we've got an empty pict file.
				 */
				BlockClear(s, sizeof(Str255));
				len = 256;
				if(noErr == errCode){ errCode = FSWrite(ref, &len, s); }
				if(noErr == errCode){ errCode = FSWrite(ref, &len, s); }
				if(noErr == errCode){
					len = GetHandleSize((Handle) ph);
					HLock((Handle) ph);
					errCode = FSWrite(ref, &len, *ph); 
					HUnlock((Handle) ph);
				}
				if(-1 != ref){
					FSClose(ref);
					ref = -1;
					FlushVol(NIL, reply.sfFile.vRefNum);
				}

				/* an error occurred after the file was created/zeroed. dump the file.
				 */
				if(noErr != errCode && noErr == modCode){	
					FSpDelete(&reply.sfFile);
				}
				if(noErr == errCode){
					win->fs = reply.sfFile;
					SetWTitle((WindowPtr) win, reply.sfFile.name);
				}
				KillPicture(ph);
			}
			TellError(errCode);
		}
	}
}


/* SetRandomDotMenu - 
	o set the menu
	o save the flags, then update them
	o if they've changed, update the image to match,
	o and redraw the image.
 */
void SetRandomDotMenu(Integer item){
	static Boolean	inside = FALSE;
	MenuHandle	mh;
	RandomDotWindowPtr	win;
	Integer		i;
	LongInt		flags;
	Rect		r;

	if(inside){
		return;
	}
	inside = TRUE;
	mh = GetMHandle(kRandomDotMenu);
	for(i = kGrayI ; i <= kShimmerI; i++){
		CheckItem(mh, i, item == i);
	}
	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
		flags = win->flags;
		switch(item){
		case kGrayI:		win->flags &= ~kShow; win->flags |= bShowGray;		break;
		case kBWStereoI:	win->flags &= ~kShow; win->flags |= bShowBWStereo;	break;
		case kGrayStereoI:	win->flags &= ~kShow; win->flags |= bShowGrayStereo;break;
		case kShimmerI:		win->flags &= ~kShow; win->flags |= bShowGrayStereo|bIsShimmer;break;
		}

		if(flags != win->flags){
			switch(item){
			case kGrayI:
				RandomDotPalette();
				break;
			case kBWStereoI:
				RandomDotPalette();
				if(bIsGray & win->flags){
					win->flags &= ~bIsGray;
					ComputeAutoStereogram(win->grayImage, win->stereoImage, FALSE);
				}
				break;
			case kGrayStereoI:
			case kShimmerI:
				if(item == kGrayStereoI){
					RandomDotPalette();
				}else{
					ShimmerDotPalette();
				}
				if(NOT (bIsGray & win->flags)){
					win->flags |= bIsGray;
					ComputeAutoStereogram(win->grayImage, win->stereoImage, TRUE); 
				}
				break;
			}
			SetPort((WindowPtr) win);
			GetContentsRect(&r);
			InvalRect(&r);
		}
	}
	inside = FALSE;
}

/* DoRandomDotGrayState - 
 */
void DoRandomDotGrayState(void){
	RandomDotWindowPtr	win;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
		SetRandomDotMenu(kGrayI);
	}
}

/* DoRandomDotBWStereoState - 
 */
void DoRandomDotBWStereoState(void){
	RandomDotWindowPtr	win;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
		SetRandomDotMenu(kBWStereoI);
	}
}

/* DoRandomDotGrayStereoState - 
 */
void DoRandomDotGrayStereoState(void){
	RandomDotWindowPtr	win;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
		SetRandomDotMenu(kGrayStereoI);
	}
}

/* DoRandomDotShimmer - 
 */
void DoRandomDotShimmer(void){
	RandomDotWindowPtr	win;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
		SetRandomDotMenu(kShimmerI);
	}
}

/* DoRandomDotCut - 
 */
void DoRandomDotCut(void){
	RandomDotWindowPtr	win;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
		DoRandomDotCopy();
	}
}

/* DoRandomDotCopy - 
 */
void DoRandomDotCopy(void){
	OSErr				errCode;
	RandomDotWindowPtr	win;
	PicHandle			ph;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
		errCode = noErr;
		if(NIL == (ph = RandomDotPicHandle(win)) || NIL == *ph || EmptyRect(&(**ph).picFrame)){
			errCode = memFullErr;
		}else{
			ZeroScrap();
			HLock((Handle) ph);
			errCode = PutScrap(GetHandleSize((Handle) ph), 'PICT', (Ptr) *ph);
			HUnlock((Handle) ph);
			KillPicture(ph);
		}
		TellError(errCode);
	}
}

/* DoRandomDotPaste - 
 */
void DoRandomDotPaste(void){
	RandomDotWindowPtr	win;

	if(NIL != (win = (RandomDotWindowPtr) FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort((WindowPtr) win);
	}
}
