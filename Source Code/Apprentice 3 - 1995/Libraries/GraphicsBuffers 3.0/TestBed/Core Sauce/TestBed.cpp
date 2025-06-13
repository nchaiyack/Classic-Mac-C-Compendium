#include <Palettes.h>

#include "TestBed.h"
#include "TestBedAnim.h"

#include "GraphicsBuffers.h"
#include "SoundUtils.h"
#include "Compat.h"
#include "GameUtils.h"
#include "QDUtils.h"
#include "DialogUtils.h"
#include "KeyUtils.h"

// ---------------------------------------------------------------------------

void InitToolBox();
void InitTestBedDemo();
void SetupVariables();
void LoadSprite(short whichSprite);
void DoTestBedDemo();
void HandleEvent(EventRecord *theEvent);

// ---------------------------------------------------------------------------

enum {
	kTestBedDlog_DoItBtn = 1,
	kTestBedDlog_QuitBtn,
	
	kTestBedDlog_16bitBtn,
	kTestBedDlog_8bitBtn,
	kTestBedDlog_4bitBtn,
	kTestBedDlog_2bitBtn,
	kTestBedDlog_1bitBtn,
	
	kTestBedDlog_512x342Btn,
	kTestBedDlog_512x384Btn,
	kTestBedDlog_600x400Btn,
	kTestBedDlog_640x480Btn,
	
	kTestBedDlog_Eraser_CopyBitsBtn,
	kTestBedDlog_Eraser_BlitterBtn,

	kTestBedDlog_Drawer_CopyMaskBtn,
	kTestBedDlog_Drawer_CopyBitsTransparentBtn,
	kTestBedDlog_Drawer_CopyBitsRegionBtn,
	kTestBedDlog_Drawer_BlitterTransparentBtn,
	kTestBedDlog_Drawer_BlitterDeepMaskBtn,
	kTestBedDlog_Drawer_BlitterUnused1Btn,
	kTestBedDlog_Drawer_BlitterUnused2Btn,
	
	kTestBedDlog_Refresher_CopyBitsBtn,
	kTestBedDlog_Refresher_BlitterBtn,
	
	kTestBedDlog_SpriteBoxItem,
	kTestBedDlog_PrevSpriteBtn,
	kTestBedDlog_NextSpriteBtn,
	
	kTestBedDlog_NumSpritesEditText,
	
	kTestBedDlog_UseSoundBtn,
	
	kTestBedDlog_FrameBoxBegin = 35,
	kTestBedDlog_FrameBoxEnd = 41,
	
	kTestBedDlog_DlogID = 128
};

enum {
	kMaxNumOfSprites = 40,

	kMaxSpriteBufferWidth = 64,
	kMaxSpriteBufferHeight = kMaxSpriteBufferWidth,
	
	kSpriteRsrcType			= 'PICT',
	kSpriteMaskRsrcType		= 'MASK',
	kSpriteRegionRsrcType	= 'Rgn '
};

// ---------------------------------------------------------------------------

TestBed gTestBed;

// ---------------------------------------------------------------------------

void main() {
	EventRecord theEvent;
	DialogPtr notImportant;
	short done;
	short itemHit;
	short eventExists;

	InitTestBedDemo();

	done = false;
	while (!done) {
		eventExists = WaitNextEvent(everyEvent, &theEvent, 10, nil);
		if (eventExists) HandleEvent(&theEvent);

		eventExists = IsDialogEvent(&theEvent);
		if (eventExists)
			eventExists = DialogSelect(&theEvent, &notImportant, &itemHit);
		if (!eventExists) itemHit = 0;

		switch(itemHit) {
			case kTestBedDlog_DoItBtn:
				/* Check number of sprites first */
				Str15 numStr;
				long numSprites;
				
				GetIText(GetDItemHdl(gTestBed.testDlog,
					kTestBedDlog_NumSpritesEditText), numStr);
				/* Get valid numeric characters only */
				if (!StrNumberOnly(numStr, false, false)) {
					SysBeep(10);
					SelIText(gTestBed.testDlog, kTestBedDlog_NumSpritesEditText,
						0, 32767);
				}
				else {
					/* Make sure # sprites in valide range */
					StringToNum(numStr, &numSprites);
					if (numSprites < 1 || numSprites > kMaxNumOfSprites) {
						SysBeep(10);
						SelIText(gTestBed.testDlog, kTestBedDlog_NumSpritesEditText,
							0, 32767);
					}
					else {
						DoTestBedDemo();
					}
				}
			break;

			case kTestBedDlog_QuitBtn:
				done = true;
			break;

			case kTestBedDlog_16bitBtn: case kTestBedDlog_8bitBtn:
			case kTestBedDlog_4bitBtn: case kTestBedDlog_2bitBtn:
			case kTestBedDlog_1bitBtn:
				SelectRadioBtn(gTestBed.testDlog, itemHit, kTestBedDlog_16bitBtn,
					kTestBedDlog_1bitBtn);

				/* 16bit doesn't support blitting with a deep mask */
				if (GetDlogCtlValue(gTestBed.testDlog, kTestBedDlog_16bitBtn)) {
					/* If this option was selected, deselect it and select
						the transparent option instead */
					if (GetDlogCtlValue(gTestBed.testDlog,
						kTestBedDlog_Drawer_BlitterDeepMaskBtn))
						SelectRadioBtn(gTestBed.testDlog,
							kTestBedDlog_Drawer_BlitterTransparentBtn,
							kTestBedDlog_Drawer_CopyMaskBtn,
							kTestBedDlog_Drawer_BlitterUnused2Btn);
					/* Disable deep mask blitter option */
					DisableDlogCtl(gTestBed.testDlog,
						kTestBedDlog_Drawer_BlitterDeepMaskBtn);
				}
				else
					/* Working on a non-16bit depth; we can use the deep mask blitter */
					EnableDlogCtl(gTestBed.testDlog,
						kTestBedDlog_Drawer_BlitterDeepMaskBtn);
				
				/* 4bit doesn't support transparent blitting */
				if (GetDlogCtlValue(gTestBed.testDlog, kTestBedDlog_4bitBtn)) {
					/* If transparent blitter option was selected,
						select deep mask option instead */
					if (GetDlogCtlValue(gTestBed.testDlog,
						kTestBedDlog_Drawer_BlitterTransparentBtn))
						SelectRadioBtn(gTestBed.testDlog,
							kTestBedDlog_Drawer_BlitterDeepMaskBtn,
							kTestBedDlog_Drawer_CopyMaskBtn,
							kTestBedDlog_Drawer_BlitterUnused2Btn);
					/* Disable transparent blitter option */
					DisableDlogCtl(gTestBed.testDlog,
						kTestBedDlog_Drawer_BlitterTransparentBtn);
				}
				else
					EnableDlogCtl(gTestBed.testDlog,
						kTestBedDlog_Drawer_BlitterTransparentBtn);
			break;

			case kTestBedDlog_512x342Btn: case kTestBedDlog_512x384Btn:
			case kTestBedDlog_600x400Btn: case kTestBedDlog_640x480Btn:
				SelectRadioBtn(gTestBed.testDlog, itemHit, kTestBedDlog_512x342Btn,
					kTestBedDlog_640x480Btn);
			break;
			
			case kTestBedDlog_Eraser_CopyBitsBtn:
			case kTestBedDlog_Eraser_BlitterBtn:
				SelectRadioBtn(gTestBed.testDlog, itemHit, kTestBedDlog_Eraser_CopyBitsBtn,
					kTestBedDlog_Eraser_BlitterBtn);
			break;

			case kTestBedDlog_Drawer_CopyMaskBtn:
			case kTestBedDlog_Drawer_CopyBitsTransparentBtn:
			case kTestBedDlog_Drawer_CopyBitsRegionBtn:
			case kTestBedDlog_Drawer_BlitterTransparentBtn:
			case kTestBedDlog_Drawer_BlitterDeepMaskBtn:
			case kTestBedDlog_Drawer_BlitterUnused1Btn:
			case kTestBedDlog_Drawer_BlitterUnused2Btn:
				SelectRadioBtn(gTestBed.testDlog, itemHit, kTestBedDlog_Drawer_CopyMaskBtn,
					kTestBedDlog_Drawer_BlitterUnused2Btn);
			break;
			
			case kTestBedDlog_Refresher_CopyBitsBtn:
			case kTestBedDlog_Refresher_BlitterBtn:
				SelectRadioBtn(gTestBed.testDlog, itemHit, kTestBedDlog_Refresher_CopyBitsBtn,
					kTestBedDlog_Refresher_BlitterBtn);
			break;

			case kTestBedDlog_NextSpriteBtn:
				if (gTestBed.curSprite == gTestBed.numSpritesAvail)
					gTestBed.curSprite = 1;
				else
					gTestBed.curSprite++;
				LoadSprite(gTestBed.curSprite);
			break;

			case kTestBedDlog_PrevSpriteBtn:
				if (gTestBed.curSprite == 1)
					gTestBed.curSprite = gTestBed.numSpritesAvail;
				else
					gTestBed.curSprite--;
				LoadSprite(gTestBed.curSprite);
			break;

			case kTestBedDlog_UseSoundBtn:
				FlipCtlValue(gTestBed.testDlog, kTestBedDlog_UseSoundBtn);
			break;
	
			default:
			break;
		}
	}
	
	/* Restore monitor depth, if necessary */
	if (gTestBed.origMonitorDepth != (**(**gTestBed.primaryDevice).gdPMap).pixelSize)
		SetMonitorDepth(gTestBed.primaryDevice, gTestBed.origMonitorDepth, true);
} // END main

// ---------------------------------------------------------------------------

void InitTestBedDemo()
/*
	Here we do the one-time init stuff; eg loading in the resources,
	setting up the dialog box, allocating our buffers, etc.
*/
{
	short monitorHeight, monitorWidth;

	InitToolBox();
	SetCursor(*GetCursor(watchCursor));
	CheckEnviron();
	
	/* No color quickdraw? Get outta here! */
	if (!gEnviron.hasCQD) ExitToShell();

	gTestBed.primaryDevice = GetMainDevice();
	gTestBed.monitorBounds = (**gTestBed.primaryDevice).gdRect;
	gTestBed.origMonitorDepth = (**(**gTestBed.primaryDevice).gdPMap).pixelSize;
	
	/* Can only handle 4, 8, and 16 bit color; all others we quit on */
	if (gTestBed.origMonitorDepth < 4 || gTestBed.origMonitorDepth > 16) {
		SysBeep(10);
		ExitToShell();
	}
	
	gTestBed.testDlog = GetNewDialog(kTestBedDlog_DlogID, NULL, (WindowPtr)-1L);
	SetPort(gTestBed.testDlog);
	
	/* Setup buttons and stuff. Ooh la la! */
	/* Disable some items by default */
	DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_2bitBtn);
	DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_1bitBtn);
	DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_Drawer_BlitterUnused1Btn);
	DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_Drawer_BlitterUnused2Btn);
	
	/* Disable some more items, if necessary */
	/* First, see if certain bit-depths are available */
	if (!HasDepth(gTestBed.primaryDevice, 16, 1 << gdDevType, 1))
		DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_16bitBtn);
	if (!HasDepth(gTestBed.primaryDevice, 8, 1 << gdDevType, 1))
		DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_8bitBtn);
	if (!HasDepth(gTestBed.primaryDevice, 4, 1 << gdDevType, 1))
		DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_4bitBtn);

	/* Now see if certain screen sizes are available */
	monitorHeight = gTestBed.monitorBounds.bottom - gTestBed.monitorBounds.top;
	monitorWidth = gTestBed.monitorBounds.right - gTestBed.monitorBounds.left;
	if (monitorHeight < 342 || monitorWidth < 512)
		DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_512x342Btn);
	if (monitorHeight < 384 || monitorWidth < 512)
		DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_512x384Btn);
	if (monitorHeight < 400 || monitorWidth < 600)
		DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_600x400Btn);
	if (monitorHeight < 480 || monitorWidth < 640)
		DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_640x480Btn);
	
	/* Setup some preset values. We use FlipCtlValue() as a cheap
	   'n quick alternative to SetCtlValue. Because all the controls
	   by default have a value of 0, by flipping the values we'll
	   set them to 1 */
	switch(gTestBed.origMonitorDepth) {
		case 16: FlipCtlValue(gTestBed.testDlog, kTestBedDlog_16bitBtn); break;
		case 8: FlipCtlValue(gTestBed.testDlog, kTestBedDlog_8bitBtn); break;
		case 4: FlipCtlValue(gTestBed.testDlog, kTestBedDlog_4bitBtn); break;
		default:
			if (HasDepth(gTestBed.primaryDevice, 16, 1 << gdDevType, 1))
				FlipCtlValue(gTestBed.testDlog, kTestBedDlog_16bitBtn);
			else if (HasDepth(gTestBed.primaryDevice, 8, 1 << gdDevType, 1))
				FlipCtlValue(gTestBed.testDlog, kTestBedDlog_8bitBtn);
			else if (HasDepth(gTestBed.primaryDevice, 4, 1 << gdDevType, 1))
				FlipCtlValue(gTestBed.testDlog, kTestBedDlog_4bitBtn);
		break;
	} // END switch

	if (monitorWidth >= 640 && monitorHeight >= 480)
		FlipCtlValue(gTestBed.testDlog, kTestBedDlog_640x480Btn);
	else if (monitorWidth >= 600 && monitorHeight >= 400)
		FlipCtlValue(gTestBed.testDlog, kTestBedDlog_600x400Btn);
	else if (monitorWidth >= 512 && monitorHeight >= 384)
		FlipCtlValue(gTestBed.testDlog, kTestBedDlog_512x384Btn);
	else
		FlipCtlValue(gTestBed.testDlog, kTestBedDlog_512x342Btn);

	FlipCtlValue(gTestBed.testDlog, kTestBedDlog_Eraser_CopyBitsBtn);
	FlipCtlValue(gTestBed.testDlog, kTestBedDlog_Drawer_CopyMaskBtn);
	FlipCtlValue(gTestBed.testDlog, kTestBedDlog_Refresher_CopyBitsBtn);
	/* Disable blitter deep mask option if we're in 16bit */
	if (GetDlogCtlValue(gTestBed.testDlog, kTestBedDlog_16bitBtn))
		DisableDlogCtl(gTestBed.testDlog, kTestBedDlog_Drawer_BlitterDeepMaskBtn);

	SelIText(gTestBed.testDlog, kTestBedDlog_NumSpritesEditText, 0, 32767);
	ShowWindow(gTestBed.testDlog);
	
	/* Preset some variables */
	gTestBed.demoWindow = NULL;

	gTestBed.bufferDepth = gTestBed.origMonitorDepth;

	/* Setup buffers */
	(void)InitGraphicsBuffers();

	SetRect(&gTestBed.bufferBounds, 0, 0, 640, 480);
	if (NewGraphicsBuffer(&gTestBed.bkgndBuffer, 0,
		&gTestBed.bufferBounds, noNewDevice, true) != noErr)
		ExitToShell();
	if (NewGraphicsBuffer(&gTestBed.dockBuffer, 0,
		&gTestBed.bufferBounds, noNewDevice, true) != noErr)
		ExitToShell();

	SetRect(&gTestBed.spriteBufferBounds, 0, 0,
		kMaxSpriteBufferWidth, kMaxSpriteBufferHeight);
	if (NewGraphicsBuffer(&gTestBed.spriteBuffer, 0,
		&gTestBed.spriteBufferBounds, noNewDevice, true) != noErr)
		ExitToShell();
	if (NewGraphicsBuffer(&gTestBed.spriteMaskDeepBuffer, 0,
		&gTestBed.spriteBufferBounds, noNewDevice, true) != noErr)
		ExitToShell();
	if (NewGraphicsBuffer(&gTestBed.spriteMaskBuffer, 1,
		&gTestBed.spriteBufferBounds, 0, false) != noErr)
		ExitToShell();

	GetDItemRect(gTestBed.testDlog, kTestBedDlog_SpriteBoxItem, &gTestBed.curSpriteBox);

	/* Preset some master sprite values */
	gTestBed.masterSprite.buffer = gTestBed.spriteBuffer;
	gTestBed.masterSprite.region = NULL;

	/* Load in sprites */
	gTestBed.numSpritesAvail = Count1Resources('PICT');
	gTestBed.curSprite = 1;
	//LoadSprite(gTestBed.curSprite);

	InitSoundUtils();

	SetCursor(&qd.arrow);
} // END InitTestBedDemo

// ---------------------------------------------------------------------------

void SetupVariables()
/*
	Get all the options user has set for demo and put 'em into
	the global gTestBed structure. This is called every time prior
	to beginning each demo run.
*/
{
	register DialogPtr testDlog = gTestBed.testDlog;
	long numero;
	short dlogItem;
	short demoDepth;
	Str15 str;

	/* Setup rect of window */
	gTestBed.windowBounds.left = gTestBed.windowBounds.top = 0;
	dlogItem = GetRadioBtn(testDlog, kTestBedDlog_512x342Btn, kTestBedDlog_640x480Btn);

	switch(dlogItem) {
		case kTestBedDlog_512x342Btn:
			gTestBed.windowBounds.right = 512;
			gTestBed.windowBounds.bottom = 342;
		break;
		
		case kTestBedDlog_512x384Btn:
			gTestBed.windowBounds.right = 512;
			gTestBed.windowBounds.bottom = 384;
		break;
		
		case kTestBedDlog_600x400Btn:
			gTestBed.windowBounds.right = 640;
			gTestBed.windowBounds.bottom = 400;
		break;
		
		case kTestBedDlog_640x480Btn:
			gTestBed.windowBounds.right = 640;
			gTestBed.windowBounds.bottom = 480;
		break;
		
		default:
			gTestBed.windowBounds.right = 512;
			gTestBed.windowBounds.bottom = 342;
		break;
	} // END switch
	
	CenterRect(&gTestBed.windowBounds, &gTestBed.monitorBounds);
	
	/* Get demo depth and set monitor depth */
	dlogItem = GetRadioBtn(testDlog, kTestBedDlog_16bitBtn, kTestBedDlog_1bitBtn);
	switch(dlogItem) {
		case kTestBedDlog_16bitBtn: demoDepth = 16; break;
		case kTestBedDlog_8bitBtn:  demoDepth = 8;  break;
		case kTestBedDlog_4bitBtn:  demoDepth = 4;  break;
		case kTestBedDlog_2bitBtn:  demoDepth = 2;  break;
		case kTestBedDlog_1bitBtn:  demoDepth = 1;  break;
	} // END switch

	/* Now update demo depth: monitor depth & buffer depth */
	if (demoDepth != gTestBed.bufferDepth) {
		SetMonitorDepth(gTestBed.primaryDevice, demoDepth, true);
		gTestBed.bufferDepth = demoDepth;
		
		numero = (long)UpdateGraphicsBuffer(gTestBed.bkgndBuffer, 0,
			&gTestBed.bufferBounds, 0, true);
		if (numero != noErr) {
			SysBeep(10); DebugStr("\p Bkgnd buffer");
		}
		numero = (long)UpdateGraphicsBuffer(gTestBed.dockBuffer, 0,
			&gTestBed.bufferBounds, 0, true);
		if (numero != noErr) {
			SysBeep(10); DebugStr("\pDock buffer");
		}
		numero = (long)UpdateGraphicsBuffer(gTestBed.spriteBuffer, 0,
			&gTestBed.spriteBufferBounds, 0, true);
		if (numero != noErr) {
			SysBeep(10); DebugStr("\p Sprite buffer");
		}
		numero = (long)UpdateGraphicsBuffer(gTestBed.spriteMaskDeepBuffer, 0,
			&gTestBed.spriteBufferBounds, 0, true);
		if (numero != noErr) {
			SysBeep(10); DebugStr("\p Sprite deep buffer");
		}
		
	}
	LoadSprite(gTestBed.curSprite);

	/* Determine eraser function */
	dlogItem = GetRadioBtn(testDlog, kTestBedDlog_Eraser_CopyBitsBtn,
		kTestBedDlog_Eraser_BlitterBtn);
	switch(dlogItem) {
		case kTestBedDlog_Eraser_CopyBitsBtn:
			gTestBed.eraser = CopyGraphicsBuffer;
		break;
		case kTestBedDlog_Eraser_BlitterBtn:
			switch(demoDepth) {
				case 16: gTestBed.eraser = BlitGraphicsBuffer_16bit; break;
				case 8:  gTestBed.eraser = BlitGraphicsBuffer_8bit;  break;
				case 4:  gTestBed.eraser = BlitGraphicsBuffer_4bit;  break;
				default: gTestBed.eraser = CopyGraphicsBuffer;		 break;
			}
		break;
	} // END switch
	
	/* Determine drawer function */
	dlogItem = GetRadioBtn(testDlog, kTestBedDlog_Drawer_CopyMaskBtn,
		kTestBedDlog_Drawer_BlitterUnused2Btn);
	switch(dlogItem) {
		case kTestBedDlog_Drawer_CopyMaskBtn:
			gTestBed.drawer = (ImageDrawer)CopyGraphicsBufferMask;
			gTestBed.animationMethod = kCopyMaskMethod;
		break;

		case kTestBedDlog_Drawer_CopyBitsTransparentBtn:
			gTestBed.drawer = (ImageDrawer)CopyGraphicsBufferTransparent;
			gTestBed.animationMethod = kCopyBitsTransparentMethod;
		break;

		case kTestBedDlog_Drawer_CopyBitsRegionBtn:
			gTestBed.drawer = (ImageDrawer)CopyGraphicsBufferRegion;
			gTestBed.animationMethod = kCopyBitsRegionMethod;
		break;

		case kTestBedDlog_Drawer_BlitterTransparentBtn:
			switch(demoDepth) {
				case 16:
					gTestBed.drawer = (ImageDrawer)BlitGraphicsBuffer_Transparent16bit;
				break;
				case 8:
					gTestBed.drawer = (ImageDrawer)BlitGraphicsBuffer_Transparent8bit;
				break;
				case 4:
					// No transparent blitter for 4bit available.
				break;
				default:
					gTestBed.drawer = (ImageDrawer)CopyGraphicsBuffer;
				break;
			}
			gTestBed.animationMethod = kBlitterTransparentMethod;
		break;

		case kTestBedDlog_Drawer_BlitterDeepMaskBtn:
			switch(demoDepth) {
				// Note: 16-bit deep mask blitter not supported
				case 8:
					gTestBed.drawer = (ImageDrawer)BlitGraphicsBuffer_Mask8bit;
				break;
				case 4:
					gTestBed.drawer = (ImageDrawer)BlitGraphicsBuffer_Mask4bit;
				break;
				default:
					gTestBed.drawer = (ImageDrawer)CopyGraphicsBuffer;
				break;
			}
			gTestBed.animationMethod = kBlitterDeepMaskMethod;
		break;

		default:
			gTestBed.drawer = (ImageDrawer)CopyGraphicsBufferMask;
			gTestBed.animationMethod = kCopyMaskMethod;
		break;
	} // END switch

	/* Determine refresher function */
	dlogItem = GetRadioBtn(testDlog, kTestBedDlog_Refresher_CopyBitsBtn,
		kTestBedDlog_Refresher_BlitterBtn);
	switch(dlogItem) {
		case kTestBedDlog_Refresher_CopyBitsBtn:
			gTestBed.refresher = CopyGraphicsBuffer;
		break;
		case kTestBedDlog_Refresher_BlitterBtn:
			switch(demoDepth) {
				case 16: gTestBed.refresher = BlitGraphicsBuffer_16bit; break;
				case 8:  gTestBed.refresher = BlitGraphicsBuffer_8bit;  break;
				case 4:  gTestBed.refresher = BlitGraphicsBuffer_4bit;  break;
				default: gTestBed.refresher = CopyGraphicsBuffer;		break;
			}
		break;
	} // END switch

	/* Find number of sprites to use in demo */
	/* Note: no error checking; we're expecting a valid number string */
	GetIText(GetDItemHdl(testDlog, kTestBedDlog_NumSpritesEditText), str);
	StringToNum(str, &numero);
	gTestBed.numSpritesInDemo = numero;
	
	/* Use bkgnd sound? */
	gTestBed.useBkgndSound = GetDlogCtlValue(testDlog, kTestBedDlog_UseSoundBtn);

	/* Now create the window, as needed */
	if (gTestBed.demoWindow) DisposeWindow(gTestBed.demoWindow);

	gTestBed.demoWindow = NewCWindow(NULL, &gTestBed.windowBounds, "\p", false,
		plainDBox, (WindowPtr)-1L, false, 0);
} // END SetupVariables

// ---------------------------------------------------------------------------

short GetResourceID(Handle theResource);
short GetResourceID(Handle theResource) {
	short resID;
	ResType resType;
	Str63 resName;
	
	GetResInfo(theResource, &resID, &resType, resName);
	return(resID);
} // END GetResourceID

void LoadSprite(short whichSprite)
/*
	Load in all the resources associated with the sprite:
	PICT, PICT mask, RGN region, etc.
	
	Argument whichSprite is resource index into PICT rsrcs,
	from 1 to the number of PICTs.
*/
{
	PicHandle spritePic;
	GWorldPtr saveWorld; GDHandle saveDev;
	Rect picRect;
	Rect sourceRect;
	short spriteID;

	GetGWorld(&saveWorld, &saveDev);
	
	/* Get sprite pic & draw into sprite buffer */
	spritePic = (PicHandle)Get1IndResource(kSpriteRsrcType, whichSprite);
	if (spritePic == NULL) {
		SysBeep(10); return;
	}
	spriteID = GetResourceID((Handle)spritePic);
	SetGraphicsBuffer(gTestBed.spriteBuffer);
	picRect = (**spritePic).picFrame;
	OffsetRect(&picRect, -picRect.left, -picRect.top);
	HLock((Handle)spritePic);
	DrawPicture(spritePic, &picRect);
	HUnlock((Handle)spritePic);
	HPurge((Handle)spritePic);

	gTestBed.masterSprite.offBounds = picRect;
	
	/* Get sprite mask and draw into both deep & shallow sprite masks */
	spritePic = (PicHandle)Get1Resource(kSpriteMaskRsrcType, spriteID);
	if (spritePic == NULL) {
		SysBeep(10); return;
	}
	SetGraphicsBuffer(gTestBed.spriteMaskBuffer);
	DrawPicture(spritePic, &picRect);
	SetGraphicsBuffer(gTestBed.spriteMaskDeepBuffer);
	HLock((Handle)spritePic);
	DrawPicture(spritePic, &picRect);
	/* The deep mask blitter requires an "inverse" mask: where we
	   want to copy the pixels we want white, where we don't want
	   to copy we want black pixels. So invert the mask */
	InvertRect(&picRect);
	HUnlock((Handle)spritePic);
	HPurge((Handle)spritePic);
	
	/* Get region mask */
	/* Each sprite should really have a copy of their own mask region. */
	/* However, since all of our sprites in our demo will be the same,
	   we can be lazy and give them all the same region. We'll place a
	   reference to this region in the master sprite */
	if (gTestBed.masterSprite.region != NULL) {
		HUnlock((Handle)gTestBed.masterSprite.region);
		HPurge((Handle)gTestBed.masterSprite.region);
		gTestBed.masterSprite.region = NULL;
	}
	gTestBed.masterSprite.region = (RgnHandle)Get1Resource(kSpriteRegionRsrcType, spriteID);
	if (spritePic == NULL) {
		SysBeep(10); return;
	}
	MoveHHi((Handle)gTestBed.masterSprite.region);
	HLock((Handle)gTestBed.masterSprite.region);
	HNoPurge((Handle)gTestBed.masterSprite.region);
	
	SetGWorld(saveWorld, saveDev);

	SetPort(gTestBed.testDlog);
	EraseRect(&gTestBed.curSpriteBox);
	FrameBorderBlack(gTestBed.testDlog, kTestBedDlog_SpriteBoxItem);
	sourceRect = picRect;
	CenterRect(&picRect, &gTestBed.curSpriteBox);
	CopyGraphicsBuffer2Window(gTestBed.spriteBuffer, gTestBed.testDlog,
		&sourceRect, &picRect);
} // END LoadSprite

// ---------------------------------------------------------------------------

void DoTestBedDemo() {
	SetupVariables();

	/* Hide dialog */
	HideWindow(gTestBed.testDlog);
	ShowWindow(gTestBed.demoWindow);
	SetPort(gTestBed.demoWindow);
	FillRect(&gTestBed.demoWindow->portRect, &qd.black);
	
	Animate();

	HideWindow(gTestBed.demoWindow);
	ShowWindow(gTestBed.testDlog);
	SelIText(gTestBed.testDlog, kTestBedDlog_NumSpritesEditText,
		0, 32767);
} // END DoTestBedDemo

// ---------------------------------------------------------------------------

void HandleEvent(EventRecord *theEvent) {
	switch(theEvent->what) {
		case updateEvt:
			LoadSprite(gTestBed.curSprite);
			for (short i = kTestBedDlog_FrameBoxBegin; i <= kTestBedDlog_FrameBoxEnd; i++) {
				FrameBorderDotted(gTestBed.testDlog, i);
			}
		break;
		
		case keyDown: case autoKey:
			char theKey = theEvent->message & charCodeMask;
			//if (theKey == 13) DoTestBedDemo();
		break;
	} // END switch
} // END HandleEvent

// ---------------------------------------------------------------------------

void InitToolBox() {
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
} // END InitToolBox