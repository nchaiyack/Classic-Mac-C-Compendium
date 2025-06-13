//
// UScreenFader - v1.2
//
// (c) rp&A Inc. - 1995
// Written by: Ramin Firoozye'
//
// This has been tested with MetroWorks CodeWarrior 6 under
// PowerPlant 1.1 under System 7.1.
//
// This is UScreenFader, a screen fader class that allows you to perform
// a cool Gamma "fade-to-black" and "fade-from-black" effect. 
// The first pass on this class used color lookup table (clut) fading 
// which worked fine, except that it needed extra care and feeding when you 
// ran it in anything other than 8-bit color mode. This version works under
// all color modes, as long as your display cards support Gamma Tables
// (and most Mac color displays do).
//
//			Rev		Version		Date		Who		Desc.
// Edit:	001		1.0			6/19/95		rf		clut version.
//			002		1.1			6/20/95		rf		gamma version.
//			003		1.2			6/20/95		rf		Powerbook modification.
//
// Please return all comments and bug-fixes to me at rpa@netcom.com or
// CIS:70751,252.
//
// For legalities and usage, please refer to the enclosed README file.
//
#include <GestaltEqu.h>
#include <Traps.h>
#include <Memory.h>
#include <Video.h>
#include "UScreenFader.h"

// This is an internal structure used to keep information on each display
struct CScreen {
		Boolean			mIsValid;	// display has all capabilities we need
		GDHandle 		mGDevice;	// GDevice for this table (1 per screen)
		short 			mBufSize;	// total gamma table size
		GammaTblHandle	mOriginal;	// original gamma table (save for restoring)
		GammaTblHandle	mCurrent;	// room for current one...
		short 			mDataSize;	// Size of actual data
		CntrlParam		mControl;	// control param block for set/get gamma.
};
typedef struct CScreen CScreen;


// Trap for GetDeviceList
const unsigned short kGetDeviceListTrapNum = 0xAA29;

//
// ctor - we do an awful lot of stuff here, so we don't have to make the user
// call a separate "init" function. The dtor cleans memory allocations.
// The ctor ultimately sets the mInit flag to true. Without this flag, all the
// other functions are no-ops.
//
UScreenFader::UScreenFader()
{
GDHandle aGDevice;
CScreen	*one;
GammaTblPtr	aTable;
short	bufSize, dataSize, currentIndex;
OSErr status;

  mInit = false;		// Start with not initialized - assume the worst.
  mDeviceCount = 0;
  mMainIndex = 0;

// Check to see that we support GetDeviceList, if not, punt...
  if (::NGetTrapAddress(kGetDeviceListTrapNum, ToolTrap) ==
		::NGetTrapAddress(_Unimplemented, ToolTrap)) 
		return;
//
// We do this in two passes. The first one counts the number of devices with
// support for gamma tables. We then allocate an array of pointers to CScreen
// structures. The second one loads up the array. We do this so when we're doing
// multi-monitor fades, we can run things a little faster.
//

// Pass 1:
  for(aGDevice = ::GetDeviceList(); 
  	  aGDevice != nil;
  	  aGDevice = ::GetNextDevice(aGDevice)) {
  	  // Let's test and see if it even supports gamma tables...
  	  // A device that is a screen device and has on-board drivers qualifies.
		if (!(::TestDeviceAttribute(aGDevice, screenDevice) && 
				::TestDeviceAttribute(aGDevice, noDriver))) {  
		mDeviceCount++;
	}
  }
  
  // Now make room for this many pointers to CScreen's.
  mScreens = (CScreen **) ::NewPtrClear((Size) (mDeviceCount * sizeof(CScreen *)));
 
// Pass 2: OK, now let's load it up... Note: we test each device to find out
// which one is also the "main" device. This is done in case we are called 
// to do a fade on the main screen.

  currentIndex = 0;	
  for(aGDevice = ::GetDeviceList(); 
  	  aGDevice != nil;
  	  aGDevice = ::GetNextDevice(aGDevice)) {
  	  // We need to test this again in case the first loop skipped a display.
 		if (!(::TestDeviceAttribute(aGDevice, screenDevice) && 
				::TestDeviceAttribute(aGDevice, noDriver))) { 
			// Now we go about building our structure for this device.
			one = (CScreen *) ::NewPtrClear(sizeof(CScreen));
			one->mGDevice = aGDevice;

			// Let's see if this is the main screen
			if (::TestDeviceAttribute(aGDevice, mainScreen))
			  mMainIndex = currentIndex;

			// Let's fetch its Gamma table. We only get a pointer to the buffer.
			// We are actually going to make copies for later use.
			one->mControl.csCode = cscGetGamma;
			one->mControl.ioCRefNum = (*aGDevice)->gdRefNum;
			(* (Ptr *) &(one->mControl.csParam)) = (Ptr) &aTable;	
			status = PBStatus((ParmBlkPtr) &(one->mControl), false);
			// On certain B&W machines versions, they pass the above attribute
			// test, yet still fail to support the cscGetGamma call. We don't
			// actually punt because this may be a multi-display machine, with
			// some good and some bad displays. We just mark the entry as
			// invalid and continue on. We just make sure we deallocate
			// only what we've actually allocated in the dtor.
			if (status) // getgamma failed... 
			  one->mIsValid = false;
			else {
			// The buffer we allocate is actually the GammaTbl structure plus
			// room for the formula data plus the actual gamma data.
			// We squirrel away the size of the gamma data for later use.
			  dataSize = (aTable->gChanCnt * aTable->gDataCnt * aTable->gDataWidth) / 8;
			  bufSize = sizeof(GammaTbl) + aTable->gFormulaSize + dataSize;
			  one->mBufSize = bufSize;
			  one->mDataSize = dataSize;
			  ::PtrToHand((Ptr) aTable, (Handle *) &(one->mOriginal), (long) bufSize);
			  ::PtrToHand((Ptr) aTable, (Handle *) &(one->mCurrent), (long) bufSize);
			  one->mIsValid = true; // but continue for other screens
			}
			// Now add it to the array and go back for more
			mScreens[currentIndex++] = one;
		} else
		  return;	// bad news...
	}
	mInit = true;
}

//
// This is the dtor for UScreenFader. We first reset the screen back to 100%
// value in case the caller forgot (don't want any "accidental" black screens
// now do we?
// We then run through and delete all the allocated buffers... We don't do
// anything if the ctor failed (mInit is false).
//
UScreenFader::~UScreenFader()
{
CScreen	*one = NULL;
short idx = 0;

	if (mInit) {		// we successfully inited something
		FadeLevel(100);	// First let's put it back to 100% level...
		// Now loop down the list and deallocate all the memory...
		for (idx = 0; idx < mDeviceCount; idx++) {
		  one = mScreens[idx];
		  // Don't deallocate GammaTbl data if this display did not support
		  // gamma tables...
		  if (one->mIsValid == true) {
		    ::DisposeHandle((Handle) one->mOriginal);
		    ::DisposeHandle((Handle) one->mCurrent);
		  }
		  ::DisposePtr((Ptr) one);
		}
	if (mScreens != NULL)
	  ::DisposePtr((Ptr) mScreens); // now the array goes
	}
}
/*
 * This is the main method here. Use it to set the FadeLevel to a percentage
 * level. 0 means solid black, 100 means not black. To get a ramping effect
 * going use the FadeToBlack and FadeFromBlack method.
 *
 * Usage:
 *     object.FadeLevel(short aPercent, short aScrIdx)
 *
 * aPercent = percentage fade level, where 0 is black and 100 is non-black.
 * aScrIdx = the index for the screen (in case there's more than one).
 *           index is a 1-based value that matches what you see in the "monitors"
 *           control panel. To simultaneously fade ALL screens, use the constant
 *           UScreenFader::kAllScreens. To fade only the "main" screen (as defined
 *           in the monitors control panel) use the constant 
 *           UScreenFader::kMainScreen. aScrIdx is a default parameter that can
 *           be skipped. It defaults to kAllScreens.
 */
int UScreenFader::FadeLevel(short aPercent, short aScrIdx)
{
OSErr status;
unsigned long i, num;
unsigned char *aSrcData, *aDestData;
CScreen *one = NULL;
short idx, start, end;
GammaTblPtr	aSrcTable, aDestTable;
CTabHandle aCTab;
GDHandle oldG;

	// Lots of reasons why we may fail:
	// 1: We didn't initialize properly
	// 2: The parameters they sent down was outside the accepted value
	// 3: We couldn't find any suitable devices
	//
	if (!mInit || (aPercent < 0) || (aPercent > 100) || mDeviceCount == 0 ||
		aScrIdx < kAllScreens || aScrIdx > mDeviceCount) 
		return(-1);
	
	// If they pass down a kAllScreens, we fade out every screen. If they give
	// us a specific index (1-based), we only do that screen... If they give
	// us kMainScreen, we only do the main screen (whichever that might be).
	
	if (aScrIdx == kAllScreens) {
	  start = 0;
	  end = mDeviceCount;
	} else
	  if (aScrIdx == kMainScreen) {
	    start = mMainIndex;
	    end = start + 1;
	  } else {
     start = aScrIdx - 1;
     end = aScrIdx;
 	 }
	
	// Start the process. We have a start and ending screen index.
    for (idx = start; idx < end; idx++) {
    //
    // We obtain the CScreen item. To save some time, the original table was
    // copied in the ctor, with a duplicate stored in mCurrent. We calculate percent
    // changes of the data values from the original onto current. Then set the
    // gamma table to this newly recalculated version. Setting the value to 100
    // effectively copies the original table into the current one and resets
    // everything back to normal.
    // NOTE: If the entry is marked as invalid, i.e. display doesn't support gamma
    // tables, we just punt. But we continue looping in case there is a mixed
    // display environment where some displays may work and others may not.
    //
      one = mScreens[idx];
      if (one->mIsValid) {
        aSrcTable = (*(one->mOriginal));
        aDestTable = (*(one->mCurrent));
	    aSrcData = (unsigned char *) (aSrcTable->gFormulaData + aSrcTable->gFormulaSize);
	    aDestData = (unsigned char *) (aDestTable->gFormulaData + aDestTable->gFormulaSize);
	    for (i = 0; i < one->mDataSize; i++) {
		  num = aSrcData[i];
		  num = (num * aPercent) / 100;
		  aDestData[i] = num;
	    }
	//
	// Now let's set the gamma levels with the current one and recalculate the 
	// color palette.
	// 
	    one->mControl.csCode = cscSetGamma;
	    one->mControl.ioCRefNum = (*(one->mGDevice))->gdRefNum;
	    (* (Ptr *) &(one->mControl.csParam)) = (Ptr) one->mCurrent;
	    status = PBControl((ParmBlkPtr) &(one->mControl), 0);
	// Nowet the current GDevice and change its color table entry
	// all at once to the newly calculated ones (based on the new table).
        oldG = ::GetGDevice();
	    ::SetGDevice(one->mGDevice);
 	    aCTab = (*(*(one->mGDevice))->gdPMap)->pmTable;
	    ::SetEntries (0, (*aCTab)->ctSize, (*aCTab)->ctTable);
	    ::SetGDevice(oldG);
	  }
	}
	return(status);
}

/*
 * Use these two methods to get a ramping effect. The value aIncrement is the
 * increment step to take when going from full screen to all black. The smaller
 * the value, the less flicker and smoother a fade effect, however, it also
 * takes longer. Default (defined in UScreenFader.h is 4). 
 *
 * Valid values for aScrIdx are described above.
 */
void UScreenFader::FadeToBlack(short aIncrement, short aScrIdx)
{
short i;

	for(i = kNormal; i >= kBlack; i-=aIncrement) 
	  FadeLevel(i, aScrIdx);
}

/* 
 * Use this one to come back from full black level.
 * As with above, the increment is for each progressive step from the previous
 * value. The smaller the value, the less flicker and smoother at the expense
 * of time. 
 *
 * Valid values for aScrIdx are described above.
 */
void UScreenFader::FadeFromBlack(short aIncrement, short aScrIdx)
{
short i;

	for(i = kBlack; i <= kNormal; i+=aIncrement) 
	  FadeLevel(i, aScrIdx);
}
