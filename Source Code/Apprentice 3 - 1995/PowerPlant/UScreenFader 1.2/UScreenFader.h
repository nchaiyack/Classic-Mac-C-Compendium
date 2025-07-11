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
//			002		1.1			6/22/95		rf		gamma version.
//			003		1.2			6/20/95		rf		Powerbook modification.
//
// Please return all comments and bug-fixes to me at rpa@netcom.com or
// CIS:70751,252.
//
// For legalities and usage, please refer to the enclosed README file.
//
#pragma once

#include <QuickDraw.h>	// For GammaTbl and GDevice
#include <Files.h>		// For CntrlParam.

struct CScreen;			// Forward reference
//
// This is the main class.
// Just create an instance of this class and call FadeToBlack or FadeFromBlack
// for the desired effect. FadeToBlack takes screen to black. FadeFromBlack
// returns it to its original state. Use FadeLevel to explicitly dim to a certain
// level.
//
class UScreenFader {
public:
		const enum { kAllScreens = -1, kMainScreen = 0, kBlack = 0, kNormal = 100 };
		
		UScreenFader();
		~UScreenFader();
		int FadeLevel(short aPercent=100, short aScrIdx=kAllScreens);
		void FadeToBlack(short aIncrement=4, short aScrIdx=kAllScreens);
		void FadeFromBlack(short aIncrement=4, short aScrIdx=kAllScreens);

private:	// These are internal to the class. Leave them alone.
		Boolean			mInit;			// Properly ctor'ed object.
		short			mDeviceCount;	// # of items in CScreen array
		short			mMainIndex;	 	// index for the "Main" screen
		struct CScreen	**mScreens; 	// array of CScreen's
};