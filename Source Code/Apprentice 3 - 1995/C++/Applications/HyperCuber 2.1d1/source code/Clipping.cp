//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains clipping algorithms
//|________________________________________________________________________________



//=============================== Prototypes ===============================\\

Boolean clip_line(long *p1h, long *p1v, long *p2h, long *p2v, Rect *clip_rect);

long intersect_vert(long start_h, long start_v, long end_h, long end_v, long line_h);
long intersect_horiz(long start_h, long start_v, long end_h, long end_v, long line_v);
Boolean left_side(long start_h, long start_v, long end_h, long end_v,
					long point_h, long point_v);


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure left_side
//|
//| Purpose: Find which side of a ray a point is on
//|
//| Parameters: point: point to classify
//|             ray_start: start point of ray
//|             ray_end:   endpoint of ray
//|             returns TRUE if point is on the left, looking from ray_start to ray_end
//|_____________________________________________________________________________________

Boolean left_side(long start_h, long start_v, long end_h, long end_v,
					long point_h, long point_v)
{

	long dot_product = (end_v - start_v) * (point_h - start_h) +
						(start_h - end_h) * (point_v - start_v);

	return (dot_product < 0);

}	//==== left_side() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure intersect_horiz
//|
//| Purpose: Find intersection of a line segment and a horizontal line
//|
//| Parameters: seg_start: start of line segment
//|             seg_end:   end of line segment
//|             line_v:    vertical coordinate of line
//|             returns:   horizontal coordinate of intersection
//|_____________________________________________________________________________________

long intersect_horiz(long start_h, long start_v, long end_h, long end_v, long line_v)
{

	return (start_h +
				(line_v - start_v)*(end_h - start_h) /
					(end_v - start_v));

}	//==== intersect_horiz() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure intersect_vert
//|
//| Purpose: Find intersection of a line segment and a vertical line, using
//|          Nicholl-Lee-Nicholl algorithm.
//|
//| Parameters: seg_start: start of line segment
//|             seg_end:   end of line segment
//|             line_v:    horizontal coordinate of line
//|             returns:   vertical coordinate of intersection
//|_____________________________________________________________________________________

long intersect_vert(long start_h, long start_v, long end_h, long end_v, long line_h)
{

	return (start_v +
				(line_h - start_h)*(end_v - start_v) /
					(end_h - start_h));

}	//==== intersect_vert() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure clip_line
//|
//| Purpose: Clip a line segment to a rectangle
//|
//| Parameters: start:     start of line segment; received start of clipped segment
//|             end:       end of line segment; received end of clipped segment
//|             clip_rect: rectangle to clip to
//|             returns TRUE if line was partly inside rect, FALSE if entirely outside
//|_____________________________________________________________________________________

Boolean clip_line(long *p1h, long *p1v, long *p2h, long *p2v, Rect *clip_rect)
{

	register long h1 = *p1h;
	register long v1 = *p1v;
	register long h2 = *p2h;
	register long v2 = *p2v;
	register short left = clip_rect->left;
	register short right = clip_rect->right;
	register short top = clip_rect->bottom;
	register short bottom = clip_rect->top;
	
	if (h1 > right)
		{														//  p1 is right of clip
		if (h2 > right)
			return FALSE;										//  both points are right of clip
			
		if (v1 > top)
			{													//  p1 is in NE section
			
			if (v2 > top)
				return FALSE;									//  both points are above clip
			
			if (left_side(h1, v1, right, bottom, h2, v2))		//  both points outside clip
				return FALSE;
			
			if (left_side(left, top, h1, v1, h2, v2))			//  both points outside clip
				return FALSE;
			
			if (left_side(right, top, left, bottom, h1, v1))
				{												//  p1 is in lower right of NE
				if (left_side(h1, v1, left, bottom, h2, v2))
					{
					if (v2 < bottom)							//  If p2 is outside clip, clip it
						{
						*p2v = bottom;
						*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
						}
					
					*p1v = intersect_vert(h1, v1, h2, v2, right);
					*p1h = right;
					}

				else if (left_side(h1, v1, right, top, h2, v2))
					{
					if (h2 < left)								//  If p2 is outside clip, clip it
						{
						*p2h = left;
						*p2v = intersect_vert(h1, v1, h2, v2, left);
						}
					
					*p1v = intersect_vert(h1, v1, h2, v2, right);
					*p1h = right;
					}

				else
					{
					if (h2 < left)								//  If p2 is outside clip, clip it
						{
						*p2h = left;
						*p2v = intersect_vert(h1, v1, h2, v2, left);
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, top);
					*p1v = top;
					}
				
				}
			else
				{												//  p1 is in upper left of NE
				if (left_side(h1, v1, right, top, h2, v2))
					{
					if (v2 < bottom)							//  If p2 is outside clip, clip it
						{
						*p2v = bottom;
						*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
						}
					
					*p1v = intersect_vert(h1, v1, h2, v2, right);
					*p1h = right;
					}

				else if (left_side(h1, v1, left, bottom, h2, v2))
					{
					if (v2 < bottom)							//  If p2 is outside clip, clip it
						{
						*p2v = bottom;
						*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, top);
					*p1v = top;
					}


				else
					{
					if (h2 < left)								//  If p2 is outside clip, clip it
						{
						*p2h = left;
						*p2v = intersect_vert(h1, v1, h2, v2, left);
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, top);
					*p1v = top;
					}
				
				}
			}
					
		else if (v1 < bottom)
			{													//  p1 is in SE section
			
			if (v2 < bottom)
				return FALSE;									//  both points are below clip
			
			if (left_side(right, top, h1, v1, h2, v2))			//  both points outside clip
				return FALSE;
			
			if (left_side(h1, v1, left, bottom, h2, v2))		//  both points outside clip
				return FALSE;
			
			if (left_side(right, bottom, left, top, h1, v1))
				{												//  p1 is in lower left of SE
				if (left_side(h1, v1, left, top, h2, v2))
					{
					if (h2 < left)								//  If p2 is outside clip, clip it
						{
						*p2h = left;
						*p2v = intersect_vert(h1, v1, h2, v2, left);
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p1v = bottom;
					}

				else if (left_side(h1, v1, right, bottom, h2, v2))
					{
					if (v2 > top)								//  If p2 is outside clip, clip it
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, top);
						*p2v = top;
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p1v = bottom;
					}

				else
					{
					if (v2 > top)								//  If p2 is outside clip, clip it
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, top);
						*p2v = top;
						}
					
					*p1v = intersect_vert(h1, v1, h2, v2, right);
					*p1h = right;
					}
				
				}
			else
				{												//  p1 is in upper right of SE
				if (left_side(h1, v1, right, bottom, h2, v2))
					{
					if (h2 < left)								//  If p2 is outside clip, clip it
						{
						*p2h = left;
						*p2v = intersect_vert(h1, v1, h2, v2, left);
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p1v = bottom;
					}

				else if (left_side(h1, v1, left, top, h2, v2))
					{
					if (h2 < left)								//  If p2 is outside clip, clip it
						{
						*p2h = left;
						*p2v = intersect_vert(h1, v1, h2, v2, left);
						}
					
					*p1v = intersect_vert(h1, v1, h2, v2, right);
					*p1h = right;
					}


				else
					{
					if (v2 > top)								//  If p2 is outside clip, clip it
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, top);
						*p2v = top;
						}
										
					*p1v = intersect_vert(h1, v1, h2, v2, right);
					*p1h = right;
					}
				
				}
			}
					
		else
			{													//  p1 is in E section
			
			if (left_side(right, top, h1, v1, h2, v2))			//  both points outside clip
				return FALSE;
			
			if (left_side(h1, v1, right, bottom, h2, v2))		//  both points outside clip
				return FALSE;
			
			if (left_side(left, top, h1, v1, h2, v2))
				{
				if (v2 > top)									//  If p2 is outside clip, clip it
					{
					*p2h = intersect_horiz(h1, v1, h2, v2, top);
					*p2v = top;
					}
				}
				
			else if (left_side(h1, v1, left, bottom, h2, v2))
				{
				if (v2 < bottom)								//  If p2 is outside clip, clip it
					{
					*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p2v = bottom;
					}
				}
				
			else
				{
				if (h2 < left)									//  If p2 is outside clip, clip it
					{
					*p2v = intersect_vert(h1, v1, h2, v2, left);
					*p2h = left;
					}
				}
		
			*p1v = intersect_vert(h1, v1, h2, v2, right);
			*p1h = right;
			}
		}

	else if (h1 < left)
		{														//  p1 is left of clip
		if (h2 < left)
			return FALSE;										//  both points are left of clip
			
		if (v1 > top)
			{													//  p1 is in NW section
			
			if (v2 > top)
				return FALSE;									//  both points are above clip
			
			if (left_side(left, bottom, h1, v1, h2, v2))		//  both points outside clip
				return FALSE;
			
			if (left_side(h1, v1, right, top, h2, v2))			//  both points outside clip
				return FALSE;
			
			if (left_side(left, top, right, bottom, h1, v1))
				{												//  p1 is in upper right of NW
				if (left_side(h1, v1, right, bottom, h2, v2))
					{
					if (h2 > right)								//  If p2 is outside clip, clip it
						{
						*p2h = right;
						*p2v = intersect_vert(h1, v1, h2, v2, right);
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, top);
					*p1v = top;
					}

				else if (left_side(h1, v1, left, top, h2, v2))
					{
					if (v2 < bottom)							//  If p2 is outside clip, clip it
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
						*p2v = bottom;
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, top);
					*p1v = top;
					}

				else
					{
					if (v2 < bottom)							//  If p2 is outside clip, clip it
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
						*p2v = bottom;
						}
					
					*p1v = intersect_vert(h1, v1, h2, v2, left);
					*p1h = left;
					}
				
				}
			else
				{												//  p1 is in lower left of NW
				if (left_side(h1, v1, left, top, h2, v2))
					{
					if (h2 > right)								//  If p2 is outside clip, clip it
						{
						*p2h = right;
						*p2v = intersect_vert(h1, v1, h2, v2, right);
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, top);
					*p1v = top;
					}

				else if (left_side(h1, v1, right, bottom, h2, v2))
					{
					if (h2 > right)								//  If p2 is outside clip, clip it
						{
						*p2h = right;
						*p2v = intersect_vert(h1, v1, h2, v2, right);
						}
					
					*p1v = intersect_vert(h1, v1, h2, v2, left);
					*p1h = left;
					}


				else
					{
					if (v2 < bottom)							//  If p2 is outside clip, clip it
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
						*p2v = bottom;
						}
										
					*p1v = intersect_vert(h1, v1, h2, v2, left);
					*p1h = left;
					}
				
				}
			}
					
		else if (v1 < bottom)
			{													//  p1 is in SW section
			
			if (v2 < bottom)
				return FALSE;									//  both points are below clip
			
			if (left_side(right, bottom, h1, v1, h2, v2))		//  both points outside clip
				return FALSE;
			
			if (left_side(h1, v1, left, top, h2, v2))			//  both points outside clip
				return FALSE;
			
			if (left_side(left, bottom, right, top, h1, v1))
				{												//  p1 is in upper left of SW
				if (left_side(h1, v1, right, top, h2, v2))
					{
					if (v2 > top)								//  If p2 is outside clip, clip it
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, top);
						*p2v = top;
					}
					
					*p1v = intersect_vert(h1, v1, h2, v2, left);
					*p1h = left;
					}

				else if (left_side(h1, v1, left, bottom, h2, v2))
					{
					if (h2 > right)								//  If p2 is outside clip, clip it
						{
						*p2h = right;
						*p2v = intersect_vert(h1, v1, h2, v2, right);
						}
					
					*p1v = intersect_vert(h1, v1, h2, v2, left);
					*p1h = left;
					}

				else
					{
					if (h2 > right)								//  If p2 is outside clip, clip it
						{
						*p2h = right;
						*p2v = intersect_vert(h1, v1, h2, v2, right);
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p1v = bottom;
					}
				
				}
			else
				{												//  p1 is in lower right of SW
				if (left_side(h1, v1, left, bottom, h2, v2))
					{
					if (v2 > top)								//  If p2 is outside clip, clip it
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, top);
						*p2v = top;
						}
					
					*p1v = intersect_vert(h1, v1, h2, v2, left);
					*p1h = left;
					}

				else if (left_side(h1, v1, right, top, h2, v2))
					{
					if (v2 > top)								//  If p2 is outside clip, clip it
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, top);
						*p2v = top;
						}
					
					*p1h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p1v = bottom;
					}


				else
					{
					if (h2 > right)								//  If p2 is outside clip, clip it
						{
						*p2v = intersect_vert(h1, v1, h2, v2, right);
						*p2h = right;
						}
										
					*p1h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p1v = bottom;
					}
				
				}
			}
		
		else
			{													//  p1 is in W section
			
			if (left_side(left, bottom, h1, v1, h2, v2))		//  both points outside clip
				return FALSE;
			
			if (left_side(h1, v1, left, top, h2, v2))			//  both points outside clip
				return FALSE;
			
			if (left_side(h1, v1, right, top, h2, v2))
				{
				if (v2 > top)									//  If p2 is outside clip, clip it
					{
					*p2h = intersect_horiz(h1, v1, h2, v2, top);
					*p2v = top;
					}
				}

			else if (left_side(right, bottom, h1, v1, h2, v2))
				{
				if (v2 < bottom)								//  If p2 is outside clip, clip it
					{
					*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p2v = bottom;
					}
				}

			else
				{
				if (h2 > right)									//  If p2 is outside clip, clip it
					{
					*p2v = intersect_vert(h1, v1, h2, v2, right);
					*p2h = right;
					}
				}
				
			*p1v = intersect_vert(h1, v1, h2, v2, left);
			*p1h = left;
			}
		
		}

	else
		{														//  Point is horizontally in clip
		
		if (v1 > top)
			{													//  Point is in N section
			
			if (v2 > top)										//  both points above clip
				return (FALSE);
			
			if (left_side(h1, v1, right, top, h2, v2))
				return (FALSE);									//  both points outside clip
			
			if (left_side(left, top, h1, v1, h2, v2))
				return (FALSE);									//  both points outside clip
			
			if (left_side(h1, v1, right, bottom, h2, v2))
				{
				if (h2 > right)									//  If p2 is outside clip, clip it
					{
					*p2v = intersect_vert(h1, v1, h2, v2, right);
					*p2h = right;
					}
				}
		
			else if (left_side(left, bottom, h1, v1, h2, v2))
				{
				if (h2 < left)									//  If p2 is outside clip, clip it
					{
					*p2v = intersect_vert(h1, v1, h2, v2, left);
					*p2h = left;
					}
				}

			else
				{
				if (v2 < bottom)								//  If p2 is outside clip, clip it
					{
					*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p2v = bottom;
					}
				}

			*p1h = intersect_horiz(h1, v1, h2, v2, top);
			*p1v = top;
			
			}
		
		else if (v1 < bottom)
			{													//  Point is in S section
			
			if (v2 < bottom)									//  both points below clip
				return (FALSE);
			
			if (left_side(h1, v1, left, bottom, h2, v2))
				return (FALSE);									//  both points outside clip
			
			if (left_side(right, bottom, h1, v1, h2, v2))
				return (FALSE);									//  both points outside clip
			
			if (left_side(h1, v1, left, top, h2, v2))
				{
				if (h2 < left)									//  If p2 is outside clip, clip it
					{
					*p2v = intersect_vert(h1, v1, h2, v2, left);
					*p2h = left;
					}
				}
		
			else if (left_side(right, top, h1, v1, h2, v2))
				{
				if (h2 > right)									//  If p2 is outside clip, clip it
					{
					*p2v = intersect_vert(h1, v1, h2, v2, right);
					*p2h = right;
					}
				}

			else
				{
				if (v2 > top)									//  If p2 is outside clip, clip it
					{
					*p2h = intersect_horiz(h1, v1, h2, v2, top);
					*p2v = top;
					}
				}

			*p1h = intersect_horiz(h1, v1, h2, v2, bottom);
			*p1v = bottom;
			
			}
		
		else
			{													//  Point is in clip
			
			if (v2 > top)
				{												//  Point2 is above clip
				
				if (h2 > right)
					{											//  Point2 is in NE section
					
					if (left_side(h1, v1, right, top, h2, v2))
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, top);
						*p2v = top;
						}
					
					else
						{
						*p2v = intersect_vert(h1, v1, h2, v2, right);
						*p2h = right;
						}
					
					}

				else if (h2 < left)
					{											//  Point2 is in NW section
					if (left_side(h1, v1, left, top, h2, v2))
						{
						*p2v = intersect_vert(h1, v1, h2, v2, left);
						*p2h = left;
						}
					
					else
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, top);
						*p2v = top;
						}
					
					}

				else											//  Point2 is in N section
					{
					*p2h = intersect_horiz(h1, v1, h2, v2, top);
					*p2v = top;
					}
				
				}
				
			else if (v2 < bottom)
				{												//  Point2 is below clip
				if (h2 > right)
					{											//  Point2 is in SE section
					if (left_side(h1, v1, right, bottom, h2, v2))
						{
						*p2v = intersect_vert(h1, v1, h2, v2, right);
						*p2h = right;
						}
					
					else
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
						*p2v = bottom;
						}
					}

				else if (h2 < left)
					{											//  Point2 is in SW section
					if (left_side(h1, v1, left, bottom, h2, v2))
						{
						*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
						*p2v = bottom;
						}
						
					else
						{
						*p2v = intersect_vert(h1, v1, h2, v2, left);
						*p2h = left;
						}
					}
					
				else											//  Point2 is in N section
					{
					*p2h = intersect_horiz(h1, v1, h2, v2, bottom);
					*p2v = bottom;
					}
				
				}
			else
				{												//  Point2 is vertically in clip
				
				if (h2 > right)
					{
					*p2v = intersect_vert(h1, v1, h2, v2, right);	//  Point2 is in E section
					*p2h = right;
					}

				else if (h2 < left)
					{
					*p2v = intersect_vert(h1, v1, h2, v2, left);	//  Point2 is in W section
					*p2h = left;
					}

				//  else Point2 is in clip
				
				}
			
			}
		
		}
		
	return TRUE;

}	//==== clip_line() ====\\
