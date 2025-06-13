THIS CODE IS UNFINISHED AND UNTESTED

class scrollInfo
{
	public:
		scrollInfo( Fixed size_of_doc, Fixed size_of_window, Fixed scale = fixed1);
		//
		// Note: calling either of the 'Magnify' members may change the current
		// scroll setting. Use operator()
		//
		void Magnify( Fixed magnfication);	// relative to center of visible part
		void Magnify( Fixed magnification, Fixed xCenter, Fixed yCenter);

		Fixed operator()() const;	// returns current scroll setting
		
		Fixed maxScroll() const;
		//
		// member to call when the window changes in size:
		//
		void AdjustScroll( Fixed newWindowSize);
		//
		// Instead of up/down you can read left/right or whatever you want
		//
		void PageDown();
		void PageUp();
		void goDown( Fixed amount);
		void goUp( Fixed amount);
		
	private:
		Fixed KeepInBounds( Fixed newValue) const;

		Fixed scale;
		const Fixed docSize;
		Fixed currentSize;		// == FixedMult( scale, docSize)
		Fixed windowSize;
		
		Fixed currentScroll;
		Fixed maximumScroll;	// max( 0, currentSize - windowSize)
		Fixed pageScroll;		// changes when window resizes
};

inline void scrollInfo::PageDown()
{
	goDown( pageScroll);
}

inline void scrollInfo::PageUp()
{
	goUp( pageScroll);
}
