//
// MONITOR.h		verson 1.0
//
// Header file for the MONITOR class in C++.
//
// Public Domain.
// Written May 1995 by Kenneth Worley.
//
// Requires System 7 or later.
//
// The fading code in this class was derived from the public domain work
// of N. Jonas Englund, Mark Womack, and Macneil Shonle in the clut fade
// project. You can contact them at: AOL: MarkWomack,
// Internet: markwomack@aol.com and Macneil Shonle - AOL: MacneilS,
// Internet: macneils@aol.com
//
// You can contact me at AOL: KNEworley, Internet: KNEworley@aol.com
// I'm a freelance programmer. Send me work!
//
// Here's the general idea behind this class:
//
// Each instance of the MONITOR class represents one graphics device
// (monitor) and the methods called in that object affect only that one
// monitor. If a system has multiple monitors, an object should be
// created for each which the application keeps track of.
//
// This class manipulates monitors in the following ways:
//	� Change color/gray mode
//	� Change bit depth (number of colors displayed)
//	� Tells you what the monitor's maximum color depth is
//	� Does a clut (color table) fade to black or a completely different clut
//
// Limitations:
//	� Clut fade routines don't seem to work on PowerBooks.
//	� Clut fades only work on "indexed" monitors. That is, they work on
//		monitors that (at this point) are set to 256 colors (8 bits) or
//		less. At those depths, a color table (clut) is used and each
//		pixel in the pixel map holds an index into the table rather than
//		an actual color.
//	� Depth and mode setting routines require System 7 or later (I think).
//
// Features:
//	� The depth and mode changing routines work on any Mac and will do
//		nothing if the requested bit depth or mode is not available.
//	� Multiple monitors may be faded either in unison, or not, to the
//		same destination clut or not, etc.
//
// Ugh! What's a clut?
// -------------------
// A 'clut' or color look-up table is used by your Mac to decide which
// colors to display on screen. When your monitor is set to 4 bit color
// (16 colors), those 16 colors don't always have to be the same 16
// colors. If you wanted to display an image that was mostly different
// shades of green, you could use a color table with 16 different shades
// of green rather than the standard 16 system colors. This type of
// thing is manipulated through the Palette Manager.
//
// Only one color table may be used at a time on any one monitor. That's
// why you may see things in the background turning weird colors when
// certain graphics are displayed on your Mac. The color table or clut
// has been changed to better display the picture in the foreground, but
// that makes everything in the background look funny because their
// pixel maps have indexes into the regular "system" default clut, not
// the one being used for the picture.
//
// At 8 bits and below, each pixel in the pixel map contains a number
// that represents an index into the current color table.
//
// Oh! So why aren't cluts used above 256 colors (8 bit)?
// ------------------------------------------------------
// Well, the next step above 8 bit color is 16 bit color. Since 16 bits
// is enough to represent thousands of different colors, a color table
// is not needed to "tune" the screen to different graphics. Everything
// pretty much looks good using one of the thousands of colors available.
// In other words, once you have thousands of colors to work with, the
// extra complexity of going through a color table is not worth it.
//
// At 16 bits and above, each pixel in the pixel map contains the
// representation of an actual RGB color.
//

#ifndef MONITOR_h
#define MONITOR_h

#define kMaxColors		256		// max indexed colors we can handle

class MONITOR
{
	public:
	
		GDHandle		myDevice;
		CTabHandle		myClut;
		CTabHandle		mySavedClut;
		Boolean			fading;
	
	protected:
	
		// These 'deltas' are used for fading to another clut
		
		long		redDeltas[kMaxColors];
		long		blueDeltas[kMaxColors];
		long		grnDeltas[kMaxColors];
		
		CTabHandle		destClut;	// 'destination' clut
		
	public:
	
				MONITOR( GDHandle aDevice );	// CONSTRUCTOR
				MONITOR( void );				// CONSTRUCTOR
		virtual	~MONITOR( void );	// DESTRUCTOR
		
		void	IMonitor( GDHandle aDevice );
		
	private:
	
		short			steps;
	
	public:
	
		void			FadeToClut( CTabHandle dest, short stepsToTake );
		
		void			FadeStep( void );
		void			FinishFade( void );
		void			AbortFade( void );
		
		Boolean			SaveCurrentClut( void );
		void			FadeToBlack( short stepsToTake );
		void			FadeToColor( RGBColor *aColor, short stepsToTake );
		void			FadeToSaved( short stepsToTake );
		
		void			FadeToClutSync( CTabHandle dest, short stepsToTake );
		void			FadeToBlackSync( short stepsToTake );
		void			FadeToColorSync( RGBColor *aColor, short stepsToTake );
		void			FadeToSavedSync( short stepsToTake );
		
		short			MaxScreenDepth( void );
		
		short			GetScreenDepth( void );
		short			GetScreenMode( void );
		void			SetScreenDepth( short newDepth );
		void			SetScreenMode( short newMode );
		void			SetScreenModeDepth( short newDepth, short newMode );
};

#endif //MONITOR_h
