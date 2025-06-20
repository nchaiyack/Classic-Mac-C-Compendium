/* Half_bits

	This routine is for scaling a bitmap by a factor of 1/2.
	
	Each pixel in the destination is determined by a group of 4 pixels
	in the source.  If two adjacent source pixels are black, then
	the destination pixel will be black.
	
	CopyBits makes the destination pixel black if any of the source
	pixels is black.
*/

#define		nil		0L
#include <MacProto.h>

void Half_bits(
	BitMap	*src_bits,
	BitMap	*dst_bits,
	Rect	*src_rect,
	Point	dst_topLeft )
{
	BitMap	half_band1;
	register int	row;
	int		num_bands;
	Rect	s_rect, d_rect;
	GrafPtr	save_port;
	GrafPort	band_port;
	Pattern		ll_lr_pat, ul_pat, ur_pat;

	GetPort( &save_port );
	
	/* Create an offscreen GrafPort for a 32-px-deep band. */
	OpenPort( &band_port );
	/*
		The default visRgn is screenBits.bounds, which is
		too small for printing.
	*/
	CopyRgn( band_port.clipRgn, band_port.visRgn );
	band_port.portBits.rowBytes = (src_rect->right - src_rect->left) / 8;
	/* Make sure it's a multiple of 4 */
	band_port.portBits.rowBytes = 4 *
		( (band_port.portBits.rowBytes + 3) / 4 );
	SetRect( &band_port.portBits.bounds,
		src_rect->left, 0, src_rect->right, 32 );
	band_port.portBits.baseAddr =
		NewPtr( 32 * band_port.portBits.rowBytes );
	band_port.portRect = band_port.portBits.bounds;
	PenMode( notSrcBic );	/* AND the paint pattern with the bitmap */
	
	half_band1.rowBytes = band_port.portBits.rowBytes / 2;
	/* Make sure it's a multiple of 4 */
	half_band1.rowBytes =
		4 * ( (half_band1.rowBytes + 3) / 4 );
	half_band1.bounds.left = half_band1.bounds.top = 0;
	half_band1.bounds.right =
		(band_port.portBits.bounds.right - band_port.portBits.bounds.left)
		/ 2;
	half_band1.bounds.bottom = 16;
	half_band1.baseAddr = NewPtr( 16 * half_band1.rowBytes );
	
	if ( (band_port.portBits.baseAddr != nil) &&
		(half_band1.baseAddr != nil)  )
	{
		StuffHex( ll_lr_pat, "\p00FF00FF00FF00FF" );
		StuffHex( ul_pat, "\pAA00AA00AA00AA00" );
		StuffHex( ur_pat, "\p5500550055005500" );
		num_bands = (src_rect->bottom - src_rect->top) / 32;
		s_rect = *src_rect;
		s_rect.bottom = s_rect.top + 32;
		topLeft(d_rect) = dst_topLeft;
		d_rect.bottom = d_rect.top + 16;
		d_rect.right = d_rect.left + 
			(src_rect->right - src_rect->left) / 2;
		for (row = 0; row < num_bands; row++)
		{
			CopyBits( src_bits, &band_port.portBits,
				&s_rect, &band_port.portBits.bounds,
				srcCopy, nil );
			PenPat( &ul_pat );
			PaintRect(  &band_port.portBits.bounds );
			CopyBits( &band_port.portBits, &half_band1,
				&band_port.portBits.bounds, &half_band1.bounds,
				srcCopy, nil );
			CopyBits( src_bits, &band_port.portBits,
				&s_rect, &band_port.portBits.bounds,
				srcCopy, nil );
			PenPat( &ur_pat );
			PaintRect(  &band_port.portBits.bounds );
			CopyBits( &band_port.portBits, &half_band1,
				&band_port.portBits.bounds, &half_band1.bounds,
				notSrcBic, nil );
			/* Now half_band1 contains ul & ur */
			
			CopyBits( src_bits, &band_port.portBits,
				&s_rect, &band_port.portBits.bounds,
				srcCopy, nil );
			PenPat( &ll_lr_pat );
			PaintRect(  &band_port.portBits.bounds );
			CopyBits( &band_port.portBits, &half_band1,
				&band_port.portBits.bounds, &half_band1.bounds,
				srcOr, nil );
			
			CopyBits( &half_band1, dst_bits,
				&half_band1.bounds, &d_rect, srcCopy, nil );
			OffsetRect( &s_rect, 0, 32 );
			OffsetRect( &d_rect, 0, 16 );
		}
		DisposPtr( band_port.portBits.baseAddr );
		DisposPtr( half_band1.baseAddr );
	}
	else
	{
		topLeft(s_rect) = dst_topLeft;
		s_rect.right = s_rect.left + (src_rect->right - src_rect->left) /2;
		s_rect.bottom = s_rect.top + (src_rect->bottom - src_rect->top) /2;
		CopyBits( src_bits, dst_bits, src_rect, &s_rect, srcCopy, nil );
	}
	
	SetPort( save_port );
	ClosePort( &band_port );
}
