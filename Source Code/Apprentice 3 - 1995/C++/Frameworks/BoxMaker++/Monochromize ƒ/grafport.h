#pragma once

class grafport
{
	public:
		~grafport();
		void use() const;
		void get_rect( Rect *rect) const;
		void get_pixH( PixMapPtr *pix) const;
		void copyfrom( const grafport &source, const short mode = srcCopy,
										const RgnHandle maskRgn = 0) const;

		void copyfrom( const grafport &source, const Rect &origRect, const Rect &destRect,
						const short mode = srcCopy, const RgnHandle maskRgn = 0) const;

		void copyfrom( const grafport &source, const Rect &origRect,
						const short mode = srcCopy, const RgnHandle maskRgn = 0) const;

		void copyfrom( const grafport &source, const grafport &mask,
						const short mode = srcCopy, const RgnHandle maskRgn = 0) const;

		void copyfrom( const grafport &source, const grafport &mask, const Rect &origRect,
							const Rect &maskRect, const Rect &destRect,
						const short mode = srcCopy, const RgnHandle maskRgn = 0) const;

		void scroll( short dh, short dv) const;

		PicHandle getPICT() const;
		
		void cls_black() const;
		void cls_white() const;
		void invert() const;

		void setentries( short start, short count, ColorSpec *theTable) const;
		void SetColorTable( short resID) const;
		void SetColorTable( CTabHandle theColorTable) const;
		
	protected:
		static const grafport *currentport;
		
		GWorldPtr    myGWorldPtr;
		GDHandle     myGDHandle;
		
		Rect         myRect;
		PixMapPtr    myPix;		
};

inline void grafport::get_rect( Rect *rect) const
{
	*rect   = myRect;
}

inline void grafport::get_pixH( PixMapPtr *pix) const
{
	*pix = myPix;
}

inline void grafport::copyfrom( const grafport &source, const short mode, const RgnHandle maskRgn) const
{
	use();
	CopyBits( (BitMapPtr)(source.myPix), (BitMapPtr)myPix,
					&(source.myRect), &myRect, mode, maskRgn);
}

inline void grafport::cls_black() const
{
	use();
	PaintRect( &myRect);
}

inline void grafport::cls_white() const
{
	use();
	EraseRect( &myRect);
}

inline void grafport::invert() const
{
	use();
	InvertRect( &myRect);
}
