/*
 *  Peter's Final Project -- A texture mapping demonstration
 *  � 1995, Peter Mattis
 *
 *  E-mail:
 *  petm@soda.csua.berkeley.edu
 *
 *  Snail-mail:
 *   Peter Mattis
 *   557 Fort Laramie Dr.
 *   Sunnyvale, CA 94087
 *
 *  Avaible from:
 *  http://www.csua.berkeley.edu/~petm/final.html
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "fixed.h"
#include "list.h"
#include "sys.stuff.h"
#include "scan.h"
#include "utils.h"

/*
 * Defines that affect rendering.
 * Increasing INTENSITY_MULT makes the depth falloff more dramatic.
 * Increasing SHADING_MULT makes the psuedo-Gourad effect less dramatic.
 * Both defines are only used in the texture scan conversion routines.
 */

#define INTENSITY_MULT  15
#define SHADE_MULT      50

/*
 * Define the number of spans to allocate when growing the span pool.
 */

#define SPAN_POOL_CHUNK   30000

/*
 * Define the SPAN type.
 * This type is used to keep track of which parts of the frame
 *  have been drawn.
 */

typedef struct _SPAN _SPAN, *SPAN;
struct _SPAN {
	short start;
	short end;
	SPAN next;
};

/*
 * Define the Z_LINE type.
 * This type is used in texture mapping walls.
 */

typedef struct Z_LINE Z_LINE;
struct Z_LINE {
	long u, v;
	long du, dv;
	unsigned char *table;
	BOOLEAN ready;
};

/*
 * Declare the private functions.
 */

static void scan_calc_texture (VECTOR, VECTOR, VECTOR);
static void scan_texture_points (POINTS);
static void scan_shade_points (POINTS, NUM, NUM, NUM);
static void scan_texture_horizontal8 (long, long);
static void scan_texture_vertical8 (long, long);
static void scan_shade8 (long, long, long);
static void scan_texture_horizontal24 (long, long);
static void scan_texture_vertical24 (long, long);
static void scan_shade24 (long, long, long, long, long);
static POINTS scan_calc_y_extrema (POINTS, long *, long *);
static POINTS scan_calc_x_extrema (POINTS, long *, long *);
static long *scan_calc_edge (long *, long, long, long, long);
static long *scan_calc_color (long *, long, long, long);

static void make_span_buffer (void);
static void reset_span_buffer (void);

static void make_span_pool (void);
static void grow_span_pool (void);
static void reset_span_pool (void);
static SPAN make_span (short, short);
static short span_trivial_reject (long, long);
static SPAN span_add (long, long, long);
static SPAN span_valid (SPAN, long, long);
static SPAN span_remove (SPAN, long, long);

/*
 * Keep local copies of the width and height of the frame.
 */
static long width, height;

/*
 * The "span_buffer" contains the spans of filled pixels in the frame.
 * The "span_pool" is a collection of spans that can be allocated.
 */
static SPAN *span_buffer;
static _SPAN *span_pool;

/*
 * Need something to keep track of which spans are full.
 */
static BOOLEAN *span_full;

/*
 * "span_pool_size" is the size of the span pool.
 * "span_pool_index" is the index of the next span that can be allocated.
 */
static long span_pool_size;
static long span_pool_index;

/*
 * The left and right edges of the polygon being drawn.
 * Note: Since polygons are guaranteed to be convex, only 2 edges
 *       can ever exist on a given scanline.
 */
static long *left_edges;
static long *right_edges;

/*
 * The left and right colors for usage in Gourad shading.
 */
static long *left_colors;
static long *right_colors;

/*
 * A table of intensity values.
 * The table is set up so that the first index is the intensity
 *  and the second index is a color value at that intensity.
 */
static unsigned char **intensity_table;

/*
 * The current texture and its size.
 */
static TEXTURE texture;
static long s;

/*
 * The vectors defining the current texture mapping.
 */
static VECTOR B, U, V;

static long plane_a;
static long plane_b;
static long plane_c;
static long plane_d;

/*
 * The "magic" texture mapping values.
 */
static long v1x, v1y, v1z;
static long v2x, v2y, v2z;
static long v3x, v3y, v3z;

/*
 * Different increment values.
 */
static long axi, bxi, cxi;
static long ayi, byi, cyi;
static long xi, yi;

/*
 * An array of z_lines needed for texture mapping walls.
 */
static Z_LINE *z_lines;

/*
 * Initialize the scan converter.
 */

void
scan_init ()
{
	short i, j;

	/*
	 * Get the frame buffer width and height.
	 * (Guaranteed not to change).
	 */
	width = get_frame_buffer_width ();
	height = get_frame_buffer_height ();

	/*
	 * Allocate the edge arrays.
	 */
	left_edges = (long *) ALLOC (sizeof (long) * height);
	right_edges = (long *) ALLOC (sizeof (long) * height);

	/*
	 * Allocate the color arrays.
	 */
	left_colors = (long *) ALLOC (sizeof (long) * height);
	right_colors = (long *) ALLOC (sizeof (long) * height);

	/*
	 * Allocate the intensity table.
	 */
	intensity_table = (unsigned char **) ALLOC (sizeof (unsigned char *) * 256);

	/*
	 * The intensity table is used to modify the value of a pixel in
	 *  a regular manner. The table is set up so that the ith subtable
	 *  contains values such that each value has been multiplied by (i / 255).
	 *  This means that the 255th subtable contains values between 0 and 255.
	 *  The 128th subtable contains values between 0 and 128. Evenly distributed
	 *  among the 256 entries in each subtable.
	 */
	for (i = 0; i < 256; i++)
	{
		intensity_table[i] = (unsigned char *) ALLOC (sizeof (unsigned char) * 256);

		for (j = 0; j < 256; j++)
		{
			intensity_table[i][j] = (i * (j + 1)) >> 8;
			if (intensity_table[i][j] == 0)
				intensity_table[i][j] = 1;
		}
	}

	/*
	 * Allocate the array used to keep track of constant z lines.
	 */
	z_lines = (Z_LINE *) ALLOC (sizeof (Z_LINE) * width);

	/*
	 * Make the span pool and span buffer.
	 */
	make_span_pool ();
	make_span_buffer ();
}

/*
 * Reset the scan converter.
 */

void
scan_reset ()
{
	/*
	 * Reset the span pool and span buffer.
	 */
	reset_span_pool ();
	reset_span_buffer ();
}

/*
 * Finish scan conversion.
 */

void
scan_finish ()
{
	/*
	 * Nothing is done here at the present time,
	 *  but we could fill in the remaining spans
	 *  to black, or some other background color.
	 */
}

/*
 * Is scan conversion complete?
 */

short
scan_complete ()
{
	short dy;
	BOOLEAN *full_spans;
	
	/*
	 * Decide if the frame buffer is full by detemining
	 *  if every span is full.
	 */

	full_spans = &span_full[0];
	dy = height;

	if (dy > 0)
		while (dy--)
			if (!(*full_spans++))
				return FALSE;

	return TRUE;
}

/*
 * Scan convert a textured face.
 */

void
scan_texture (pts, n, to, tu, tv, t)
	POINTS pts;
	VECTOR n, to, tu, tv;
	TEXTURE t;
{
	/*
	 * Scan convert a polygon.
	 * The plane variables are needed to determine
	 *  the z value for a given screen point.
	 * "scan_calc_texture" calculates the "magic"
	 *  texture values.
	 * "scan_points" calculates the horizontal spans
	 *  this polygon contains and calls the appropriate
	 *  drawing routine based on the polygons orientation.
	 *  (ie Is it a wall or a floor).
	 */

	plane_a = my_num_to_fix (vector_x (n));
	plane_b = my_num_to_fix (vector_y (n));
	plane_c = my_num_to_fix (vector_z (n));
	plane_d = my_num_to_fix (vector_dot (n, to));

	texture = t;
	scan_calc_texture (to, tu, tv);
	scan_texture_points (pts);
}

/*
 * Scan convert a shaded face.
 */

void
scan_shade (pts, r, g, b)
	POINTS pts;
	NUM r, g, b;
{
	scan_shade_points (pts, r, g, b);
}

/*
 * Calculate the "magic" texture values.
 */

void
scan_calc_texture (to, tu, tv)
	VECTOR to, tu, tv;
{
	VECTOR V1, V2, V3;

	vector_copy (to, B);
	vector_copy (tu, U);
	vector_copy (tv, V);

	vector_cross (tu, tv, V1);
	vector_cross (to, tu, V2);
	vector_cross (tv, to, V3);

	v1x = my_num_to_fix (vector_x (V1));
	v1y = my_num_to_fix (vector_y (V1));
	v1z = my_num_to_fix (vector_z (V1));

	v2x = my_num_to_fix (vector_x (V2));
	v2y = my_num_to_fix (vector_y (V2));
	v2z = my_num_to_fix (vector_z (V2));

	v3x = my_num_to_fix (vector_x (V3));
	v3y = my_num_to_fix (vector_y (V3));
	v3z = my_num_to_fix (vector_z (V3));

	xi = float_to_fix (-2.0 /width);
	yi = float_to_fix (-2.0 /height);

	s = texture_size (texture);

	axi = fix_mul (v1x, xi);
	bxi = fix_mul (v2x, xi) * s;
	cxi = fix_mul (v3x, xi) * s;

	ayi = fix_mul (v1y, yi);
	byi = fix_mul (v2y, yi) * s;
	cyi = fix_mul (v3y, yi) * s;
}

/*
 * Scan convert a polygon for texturing.
 */

static void
scan_texture_points (pts)
	POINTS pts;
{
	POINTS last_pt = points_last (pts);
	POINTS y_min_pt, tmp_pts;
	POINT cur_pt, next_pt, prev_pt;
	long y_min, y_max;
	long x_min, x_max;
	long *temp_edge;
	long dy;

	/*
	 * Calculate the x extrema and do a trivial
	 *  rejection if this polygon degenerates into
	 *  a vertical line.
	 */
	scan_calc_x_extrema (pts, &x_min, &x_max);
	if (x_min == x_max)
		return;

	/*
	 * Calculate the y extrema and do a trivial
	 *  rejection if this polygon degenerates into
	 *  a horizontal line.
	 */
	y_min_pt = scan_calc_y_extrema (pts, &y_min, &y_max);
	if (y_min == y_max)
		return;

	/*
	 * Check to see that this polygon has some visible
	 *  portion.
	 */
	if (span_trivial_reject (y_min, y_max))
		return;

	/*
	 * Make the points list circular.
	 */
	set_points_rest (last_pt, pts);
	set_points_prev (pts, last_pt);

	/*
	 * Calculate the "left_edges" for this polygon.
	 * This is done by using a modified line drawing
	 *  algorithm to determine the x-coordinate for
	 *  every y-coordinate on the line.
	 * Note: Because it is unknown whether or not
	 *       the points are specified clockwise or
	 *       counter-clockwise in the "pts" list, the
	 *       "left_edges" calculated here may in fact
	 *       be the "right_edges" of the polygon. This
	 *       is something to keep in mind.
	 */

	tmp_pts = y_min_pt;
	temp_edge = left_edges;
	while (points_prev (tmp_pts) != y_min_pt)
	{
		cur_pt = points_first (tmp_pts);
		prev_pt = points_first (points_prev (tmp_pts));
		tmp_pts = points_prev (tmp_pts);

		dy = point_y (prev_pt) - point_y (cur_pt);
		if (dy < 0)
			break;

		if (dy > 0)
		{
			temp_edge = scan_calc_edge (temp_edge, point_x (cur_pt), point_x (prev_pt),
				point_y (cur_pt), point_y (prev_pt));
		}
	}

	/*
	 * Calculate the "right_edges" for this polygon.
	 */

	tmp_pts = y_min_pt;
	temp_edge = right_edges;
	while (points_rest (tmp_pts) != y_min_pt)
	{
		cur_pt = points_first (tmp_pts);
		next_pt = points_first (points_rest (tmp_pts));
		tmp_pts = points_rest (tmp_pts);

		dy = point_y (next_pt) - point_y (cur_pt);
		if (dy < 0)
			break;

		if (dy > 0)
		{
			temp_edge = scan_calc_edge (temp_edge, point_x (cur_pt), point_x (next_pt),
				point_y (cur_pt), point_y (next_pt));
		}
	}

	/*
	 * Call the appropriate drawing routine based on two
	 *  considerations.
	 * 1. What is the depth of the screen?
	 * 2. What is the orientation of the polygon?
	 * Walls are drawn by the scan_texture_vertical routines
	 *  and floor and ceilings are drawn by the
	 *  scan_texture_horizontal routines.
	 */
	switch (get_frame_buffer_pixel ())
	{
	case 1:
		if (v1x == 0)
			scan_texture_horizontal8 (y_min, y_max);
		else if (v1y == 0)
			scan_texture_vertical8 (y_min, y_max);
		break;
	case 4:
		if (v1x == 0)
			scan_texture_horizontal24 (y_min, y_max);
		else if (v1y == 0)
			scan_texture_vertical24 (y_min, y_max);
		break;
	}

	/*
	 * Make the points list non-circular.
	 * This is needed so that disposal of the list
	 *  works properly.
	 */
	set_points_rest (last_pt, NULL);
	set_points_prev (pts, NULL);
}

/*
 * Scan convert a polygon for shading.
 */

static void
scan_shade_points (pts, r, g, b)
	POINTS pts;
	NUM r, g, b;
{
	POINTS last_pt = points_last (pts);
	POINTS y_min_pt, tmp_pts;
	POINT cur_pt, next_pt, prev_pt;
	long y_min, y_max;
	long x_min, x_max;
	long *temp_edge;
	long *temp_color;
	long dy;

	/*
	 * Calculate the x extrema and do a trivial
	 *  rejection if this polygon degenerates into
	 *  a vertical line.
	 */
	scan_calc_x_extrema (pts, &x_min, &x_max);
	if (x_min == x_max)
		return;

	/*
	 * Calculate the y extrema and do a trivial
	 *  rejection if this polygon degenerates into
	 *  a horizontal line.
	 */
	y_min_pt = scan_calc_y_extrema (pts, &y_min, &y_max);
	if (y_min == y_max)
		return;

	/*
	 * Check to see that this polygon has some visible
	 *  portion.
	 */
	if (span_trivial_reject (y_min, y_max))
		return;

	/*
	 * Make the points list circular.
	 */
	set_points_rest (last_pt, pts);
	set_points_prev (pts, last_pt);

	/*
	 * Calculate the "left_edges" for this polygon.
	 * This is done by using a modified line drawing
	 *  algorithm to determine the x-coordinate for
	 *  every y-coordinate on the line.
	 * Note: Because it is unknown whether or not
	 *       the points are specified clockwise or
	 *       counter-clockwise in the "pts" list, the
	 *       "left_edges" calculated here may in fact
	 *       be the "right_edges" of the polygon. This
	 *       is something to keep in mind.
	 *
	 * The difference between this and the texture scan
	 *  converter is that we also keep track off intensity
	 *  values along edges.
	 */

	tmp_pts = y_min_pt;
	temp_edge = left_edges;
	temp_color = left_colors;
	while (points_prev (tmp_pts) != y_min_pt)
	{
		cur_pt = points_first (tmp_pts);
		prev_pt = points_first (points_prev (tmp_pts));
		tmp_pts = points_prev (tmp_pts);

		dy = point_y (prev_pt) - point_y (cur_pt);
		if (dy < 0)
			break;

		if (dy > 0)
		{
			temp_edge = scan_calc_edge (temp_edge, point_x (cur_pt), point_x (prev_pt),
				point_y (cur_pt), point_y (prev_pt));
			
			temp_color = scan_calc_color (temp_color, 
				my_num_to_fix (my_mul (point_intensity (cur_pt),
									   my_float_to_num (255.0))),
				my_num_to_fix (my_mul (point_intensity (prev_pt),
									   my_float_to_num (255.0))),
				dy);
		}
	}

	/*
	 * Calculate the "right_edges" for this polygon.
	 */

	tmp_pts = y_min_pt;
	temp_edge = right_edges;
	temp_color = right_colors;
	while (points_rest (tmp_pts) != y_min_pt)
	{
		cur_pt = points_first (tmp_pts);
		next_pt = points_first (points_rest (tmp_pts));
		tmp_pts = points_rest (tmp_pts);

		dy = point_y (next_pt) - point_y (cur_pt);
		if (dy < 0)
			break;

		if (dy > 0)
		{
			temp_edge = scan_calc_edge (temp_edge, point_x (cur_pt), point_x (next_pt),
				point_y (cur_pt), point_y (next_pt));
			
			temp_color = scan_calc_color (temp_color, 
				my_num_to_fix (my_mul (point_intensity (cur_pt),
									   my_float_to_num (255.0))),
				my_num_to_fix (my_mul (point_intensity (next_pt),
									   my_float_to_num (255.0))),
				dy);
		}
	}

	/*
	 * Call the appropriate drawing routine based on the depth.
	 */
	switch (get_frame_buffer_pixel ())
	{
	case 1:
		scan_shade8 (y_min, y_max, 65536);
	case 4:
		scan_shade24 (y_min, y_max, 
			my_num_to_fix (r), 
			my_num_to_fix (g),
			my_num_to_fix (b));
		break;
	}

	/*
	 * Make the points list non-circular.
	 * This is needed so that disposal of the list
	 *  works properly.
	 */
	set_points_rest (last_pt, NULL);
	set_points_prev (pts, NULL);
}

/*
 * Scan a "horizontal" polygon for texturing.
 */

static void
scan_texture_horizontal8 (y_min, y_max)
	long y_min, y_max;
{
	PIXEL8 *p, *t;
	PIXEL8 *ta;
	SPAN cur_span;
	SPAN next_span;
	unsigned char *table;
	long ts, mask;
	long dx, dy;
	long *left, *right;
	long a, b, c;
	long bi, ci;
	long u, v;
	long x, y;
	long intensity;

	/*
	 * Get the frame buffer address and offset to
	 *  the first scanline.
	 */
	p = get_frame_buffer_address ();
	p += y_min * width;

	/*
	 * Check the "middle" of the polygon to see which
	 *  edges are which. We can't know ahead of time!
	 */
	dy = (y_max - y_min) >> 1;
	if (left_edges[dy] < right_edges[dy])
	{
		left = left_edges;
		right = right_edges;
	}
	else
	{
		left = right_edges;
		right = left_edges;
	}

	/*
	 * Get the texture address and its size log 2.
	 * Also, a mask is used to speed up overflow handling.
	 */
	ta = texture_address (texture);
	ts = texture_size_log2 (texture);
	mask = texture_size (texture) - 1;

	/*
	 * Need to keep track of y.
	 */
	y = float_to_fix (1 - (y_min * 2.0) / height);

	/*
	 * For each scanline...
	 */
	dy = y_max - y_min;
	while (dy--)
	{
		/*
		 * Get a pointer to the start of the line and
		 *  increment to the start of the next line.
		 */
		t = p + *left;
		p += width;

		/*
		 * Find the width of the span on the current line.
		 * This should never be less than 0. (Though stranger
		 *  things have happened.) It may be 0, though.
		 */
		dx = *right - *left;
		if (dx > 0)
		{
			/*
			 * Add the span to the given span buffer. The span
			 *  that is returned is the areas we are allowed to
			 *  draw in.
			 */
			cur_span = span_add (*left, *right, y_max - dy - 1);

			/*
			 * If something was returned...
			 */
			if (cur_span)
			{
				/*
				 * Calculate texture values for this line.
				 */
				x = float_to_fix (1 - (*left * 2.0) /width);
				a = fix_mul (v1x, x) + fix_mul (v1y, y) + v1z;
				b = (fix_mul (v2x, x) + fix_mul (v2y, y) + v2z) * s;
				c = (fix_mul (v3x, x) + fix_mul (v3y, y) + v3z) * s;

				/*
				 * The polygon is "horizontal" so z is constant
				 *  across a scanline. (ie "ai" is 0). This means
				 *  we can do the costly divides outside the inner
				 *  loop.
				 */
				b = fix_div (b, a);
				c = fix_div (c, a);

				bi = fix_div (bxi, a);
				ci = fix_div (cxi, a);

				/*
				 * Calculate the z distance this scanline is from the viewer.
				 * It'll be the same for the whole scanline.
				 */
				intensity = fix_mul (plane_a, x) + fix_mul (plane_b, y) - plane_c;
				intensity = fix_div (plane_d, intensity);

				/*
				 * This is a little hack, that makes it look nice.
				 * Clamp the value between 0 and 255.
				 */
				intensity = fix_to_int (intensity * INTENSITY_MULT);
				if (intensity < 0)
					intensity = -intensity;
				if (intensity > 255)
					intensity = 255;

				/*
				 * The farther something is away, the darker it gets, so...
				 */
				table = intensity_table[255 - intensity];

				/*
				 * This is just something I noticed. (It's the psuedo-
				 *  Gouraud shading).
				 * "a" depends on the orientation of the polygon
				 *  with respect to the viewer. That is, if the
				 *  polygon is nearly on edge, "a" gets smaller.
				 *  Conversely, if the polygon is viewed straight
				 *  on "a" get larger.
				 */
				a = fix_to_int (a * SHADE_MULT);
				if (a < 0)
					a = -a;
				if (a > 255)
					a = 255;

				/*
				 * Use the previous depth table to find a new table
				 *  based on the value of "a".
				 */
				table = intensity_table[table[a]];

				/*
				 * "t" points to the beginning of the original
				 *  span for this scanline. We need to increment
				 *  it and the texture values to get to the
				 *  start of the first span we can draw into.
				 */
				if (cur_span->next)
					dx = 0;
				dx = cur_span->start - *left;
				
				t += dx;
				b += bi * dx;
				c += ci * dx;

				/*
				 * While there are more spans...
				 */
				while (cur_span)
				{
					dx = cur_span->end - cur_span->start;
					if (dx < 0)
						dx = 0;

					/*
					 * For the width of this span calculate
					 *  the u and v texture vals and use the
					 *  table chosen above to modify the
					 *  texture color.
					 * Remember to increment.
					 */
					while (dx--)
					{
						u = fix_to_int (b) & mask;
						v = fix_to_int (c) & mask;

						*t++ = table[*(ta + (u << ts) + v)];

						b += bi;
						c += ci;
					}

					/*
					 * There is probably (almost definately)
					 *  some distance between the end of the
					 *  current span and the start of the next.
					 *  This means we have to increment the
					 *  texture vals appropriately.
					 */
					next_span = cur_span->next;
					if (next_span)
						dx = next_span->start - cur_span->end;
					else
						dx = *right - cur_span->end;

					cur_span = next_span;

					if (dx < 0)
						dx = 0;
					
					t += dx;
					b += bi * dx;
					c += ci * dx;
				}
			}
		}

		/*
		 * Increment the left and right edge pointers.
		 */
		left++;
		right++;

		/*
		 * Increment y.
		 */
		y += yi;
	}
}

static void
scan_texture_vertical8 (y_min, y_max)
	long y_min, y_max;
{
	PIXEL8 *p, *t;
	PIXEL8 *ta;
	Z_LINE *cols;
	SPAN cur_span;
	SPAN next_span;
	long dx, dy;
	long *left, *right;
	long ts, mask;
	long a, b, c;
	long x, y;
	long u, v;
	long intensity;

	/*
	 * Get the frame buffer address and offset to
	 *  the first scanline.
	 */
	p = get_frame_buffer_address ();
	p += y_min * width;

	/*
	 * Check the "middle" of the polygon to see which
	 *  edges are which. We can't know ahead of time!
	 */
	dy = (y_max - y_min) >> 1;
	if (left_edges[dy] < right_edges[dy])
	{
		left = left_edges;
		right = right_edges;
	}
	else
	{
		left = right_edges;
		right = left_edges;
	}

	/*
	 * Initialize the "z_lines" array.
	 */
	for (dx = 0; dx < width; dx++)
		z_lines[dx].ready = FALSE;

	/*
	 * Get the texture address and its size log 2.
	 * Also, a mask is used to speed up overflow handling.
	 */
	ta = texture_address (texture);
	ts = texture_size_log2 (texture);
	mask = texture_size (texture) - 1;

	/*
	 * Need to keep track of y.
	 */
	y = float_to_fix (1 - (y_min * 2.0) /height);

	/*
	 * For each scanline...
	 */
	dy = y_max - y_min;
	while (dy--)
	{
		/*
		 * Get a pointer to the start of the line and
		 *  increment to the start of the next line.
		 */
		t = p + *left;
		p += width;
		
		/*
		 * Find the width of the span on the current line.
		 * This should never be less than 0. (Though stranger
		 *  things have happened.) It may be 0, though.
		 */
		dx = *right - *left;
		if (dx > 0)
		{
			/*
			 * Add the span to the given span buffer. The span
			 *  that is returned is the areas we are allowed to
			 *  draw in.
			 */
			cur_span = span_add (*left, *right, y_max - dy - 1);

			if (cur_span)
			{
				/*
				 * Get a pointer to the start of the "z_lines"
				 *  for this scanline. Also calculate x for
				 *  the start of this scanline.
				 */
				cols = &z_lines[*left];
				x = float_to_fix (1 - (*left * 2.0) / width);
				
				/*
				 * "t" points to the beginning of the original
				 *  span for this scanline. We need to increment
				 *  it and the texture values to get to the
				 *  start of the first span we can draw into.
				 */
				if (cur_span->next)
					dx = 0;
				dx = cur_span->start - *left;
				if (dx < 0)
					dx = 0;

				while (dx--)
				{
					if (cols->ready)
						cols->u += cols->du;
	
					t++;
					cols++;
					x += xi;
				}
				
				/*
				 * While there are more spans...
				 */
				while (cur_span)
				{
					dx = cur_span->end - cur_span->start;
					if (dx < 0)
						dx = 0;
	
					while (dx--)
					{
						/*
						 * If this column isn't ready, then we need to
						 *  do some initialization. This is basically
						 *  the same as what was done for the "horizontal"
						 *  scan conversion, except that this time "a"
						 *  is constant down a column. Actually, its even
						 *  better than that this time. "v" is also constant.
						 */
						if (!cols->ready)
						{
							cols->ready = TRUE;
	
							a = fix_mul (v1x, x) + fix_mul (v1y, y) + v1z;
							b = (fix_mul (v2x, x) + fix_mul (v2y, y) + v2z) * s;
							c = (fix_mul (v3x, x) + fix_mul (v3y, y) + v3z) * s;
	
							cols->u = fix_div (b, a);
							cols->v = fix_to_int (fix_div (c, a)) & mask;
							cols->du = fix_div (byi, a);
	
							intensity = fix_mul (plane_a, -x) + fix_mul (plane_b, y) - plane_c;
							intensity = fix_div (plane_d, intensity);
	
							intensity = fix_to_int (intensity * INTENSITY_MULT);
							if (intensity < 0)
								intensity = -intensity;
							if (intensity > 255)
								intensity = 255;
	
							cols->table = intensity_table[255 - intensity];
	
							a = fix_to_int (a * SHADE_MULT);
							if (a < 0)
								a = -a;
							if (a > 255)
								a = 255;
	
							cols->table = intensity_table[cols->table[a]];
						}
	
						u = fix_to_int (cols->u) & mask;
						v = cols->v;
	
						cols->u += cols->du;
	
						*t++ = cols->table[*(ta + (u << ts) + v)];
						cols++;
						x += xi;
					}
	
					/*
					 * There is probably (almost definately)
					 *  some distance between the end of the
					 *  current span and the start of the next.
					 *  This means we have to increment the
					 *  texture vals appropriately.
					 */
					next_span = cur_span->next;
					if (next_span)
						dx = next_span->start - cur_span->end;
					else
						dx = *right - cur_span->end;
	
					cur_span = next_span;
	
					if (dx < 0)
						dx = 0;
	
					while (dx--)
					{
						if (cols->ready)
							cols->u += cols->du;
	
						t++;
						cols++;
						x += xi;
					}
				}
			}
		}

		/*
		 * Increment the left and right edge pointers.
		 */
		left++;
		right++;

		/*
		 * Increment y.
		 */
		y += yi;
	}
}

static void
scan_shade8 (y_min, y_max, intensity)
	long y_min, y_max, intensity;
{
	PIXEL8 *p, *t;
	SPAN cur_span;
	SPAN next_span;
	unsigned char *table;
	long dx, dy, dc;
	long color;
	long *left, *right;
	long *left_c, *right_c;

	p = get_frame_buffer_address ();
	p += y_min * width;

	dy = (y_max - y_min) >> 1;
	if (left_edges[dy] < right_edges[dy])
	{
		left = left_edges;
		right = right_edges;
		left_c = left_colors;
		right_c = right_colors;
	}
	else
	{
		left = right_edges;
		right = left_edges;
		left_c = right_colors;
		right_c = left_colors;
	}

	dy = y_max - y_min;
	while (dy--)
	{
		t = p + *left;
		p += width;

		dx = *right - *left;
		if (dx > 0)
		{
			cur_span = span_add (*left, *right, y_max - dy - 1);

			if (cur_span)
			{
				color = *left_c;
				dc = (*right_c - *left_c) /dx;

				if (cur_span->next)
					dx = 0;
				dx = cur_span->start - *left;

				while (dx--)
				{
					t++;
					color += dc;
				}

				while (cur_span)
				{
					dx = cur_span->end - cur_span->start;
					if (dx < 0)
						dx = 0;

					while (dx--)
					{
						*t++ = fix_to_int (fix_mul (color, intensity));
						color += dc;
					}

					next_span = cur_span->next;
					if (next_span)
						dx = next_span->start - cur_span->end;
					else
						dx = *right - cur_span->end;

					cur_span = next_span;

					while (dx--)
					{
						t++;
						color += dc;
					}
				}
			}
		}

		left++;
		right++;
		left_c++;
		right_c++;
	}
}

static void
scan_texture_horizontal24 (y_min, y_max)
	long y_min, y_max;
{
	PIXEL24 *p, *t;
	PIXEL24 *ta, pix;
	SPAN cur_span;
	SPAN next_span;
	unsigned char *table;
	long ts, mask;
	long dx, dy;
	long *left, *right;
	long a, b, c;
	long bi, ci;
	long u, v;
	long x, y;
	long intensity;

	p = get_frame_buffer_address ();
	p += y_min * width;

	dy = (y_max - y_min) >> 1;
	if (left_edges[dy] < right_edges[dy])
	{
		left = left_edges;
		right = right_edges;
	}
	else
	{
		left = right_edges;
		right = left_edges;
	}

	ta = texture_address (texture);
	ts = texture_size_log2 (texture);
	mask = texture_size (texture) - 1;

	y = float_to_fix (1 - (y_min * 2.0) /height);

	dy = y_max - y_min;
	while (dy--)
	{
		t = p + *left;
		p += width;

		dx = *right - *left;
		if (dx > 0)
		{
			cur_span = span_add (*left, *right, y_max - dy - 1);

			if (cur_span)
			{
				x = float_to_fix (1 - (*left * 2.0) /width);
				a = fix_mul (v1x, x) + fix_mul (v1y, y) + v1z;
				b = (fix_mul (v2x, x) + fix_mul (v2y, y) + v2z) * s;
				c = (fix_mul (v3x, x) + fix_mul (v3y, y) + v3z) * s;

				b = fix_div (b, a);
				c = fix_div (c, a);

				bi = fix_div (bxi, a);
				ci = fix_div (cxi, a);

				intensity = fix_mul (plane_a, x) + fix_mul (plane_b, y) - plane_c;
				intensity = fix_div (plane_d, intensity);

				intensity = fix_to_int (intensity * INTENSITY_MULT);
				if (intensity < 0)
					intensity = -intensity;
				if (intensity > 255)
					intensity = 255;

				table = intensity_table[255 - intensity];

				a = fix_to_int (a * SHADE_MULT);
				if (a < 0)
					a = -a;
				if (a > 255)
					a = 255;

				table = intensity_table[table[a]];

				if (cur_span->next)
					dx = 0;
				dx = cur_span->start - *left;

				while (dx--)
				{
					t++;
					b += bi;
					c += ci;
				}

				while (cur_span)
				{
					dx = cur_span->end - cur_span->start;
					if (dx < 0)
						dx = 0;

					while (dx--)
					{
						u = fix_to_int (b) & mask;
						v = fix_to_int (c) & mask;

						pix = *(ta + (u << ts) + v);
						*t = table[pix & 0xFF];
						pix >>= 8;
						*t += table[pix & 0xFF] << 8;
						pix >>= 8;
						*t += table[pix & 0xFF] << 16;
						t++;

						b += bi;
						c += ci;
					}

					next_span = cur_span->next;
					if (next_span)
						dx = next_span->start - cur_span->end;
					else
						dx = *right - cur_span->end;

					cur_span = next_span;

					if (dx < 0)
						dx = 0;

					while (dx--)
					{
						t++;
						b += bi;
						c += ci;
					}
				}
			}
		}

		left++;
		right++;

		y += yi;
	}
}

static void
scan_texture_vertical24 (y_min, y_max)
	long y_min, y_max;
{
	PIXEL24 *p, *t;
	PIXEL24 *ta, pix;
	Z_LINE *cols;
	SPAN cur_span;
	SPAN next_span;
	long dx, dy;
	long *left, *right;
	long ts, mask;
	long a, b, c;
	long x, y;
	long u, v;
	long intensity;

	p = get_frame_buffer_address ();
	p += y_min * width;

	dy = (y_max - y_min) >> 1;
	if (left_edges[dy] < right_edges[dy])
	{
		left = left_edges;
		right = right_edges;
	}
	else
	{
		left = right_edges;
		right = left_edges;
	}

	for (dx = 0; dx < width; dx++)
		z_lines[dx].ready = FALSE;

	ta = texture_address (texture);
	ts = texture_size_log2 (texture);
	mask = texture_size (texture) - 1;

	y = float_to_fix (1 - (y_min * 2.0) /height);

	dy = y_max - y_min;
	while (dy--)
	{
		t = p + *left;
		p += width;

		cols = &z_lines[*left];
		x = float_to_fix (1 - (*left * 2.0) /width);
		dx = *right - *left;

		if (dx > 0)
		{
			cur_span = span_add (*left, *right, y_max - dy - 1);

			if (cur_span)
			{
				if (cur_span->next)
					dx = 0;
				dx = cur_span->start - *left;
			}

			if (dx < 0)
				dx = 0;

			while (dx--)
			{
				if (cols->ready)
				{
					cols->u += cols->du;
				}

				t++;
				cols++;
				x += xi;
			}

			while (cur_span)
			{
				dx = cur_span->end - cur_span->start;
				if (dx < 0)
					dx = 0;

				while (dx--)
				{
					if (!cols->ready)
					{
						cols->ready = TRUE;

						a = fix_mul (v1x, x) + fix_mul (v1y, y) + v1z;
						b = (fix_mul (v2x, x) + fix_mul (v2y, y) + v2z) * s;
						c = (fix_mul (v3x, x) + fix_mul (v3y, y) + v3z) * s;

						cols->u = fix_div (b, a);
						cols->v = fix_to_int (fix_div (c, a)) & mask;
						cols->du = fix_div (byi, a);

						intensity = fix_mul (plane_a, -x) + fix_mul (plane_b, y) - plane_c;
						intensity = fix_div (plane_d, intensity);

						intensity = fix_to_int (intensity * INTENSITY_MULT);
						if (intensity < 0)
							intensity = -intensity;
						if (intensity > 255)
							intensity = 255;

						cols->table = intensity_table[255 - intensity];

						a = fix_to_int (a * SHADE_MULT);
						if (a < 0)
							a = -a;
						if (a > 255)
							a = 255;

						cols->table = intensity_table[cols->table[a]];
					}

					u = fix_to_int (cols->u) & mask;
					v = cols->v;

					cols->u += cols->du;

					pix = *(ta + (u << ts) + v);
					*t = cols->table[pix & 0xFF];
					pix >>= 8;
					*t += cols->table[pix & 0xFF] << 8;
					pix >>= 8;
					*t += cols->table[pix & 0xFF] << 16;
					t++;

					cols++;
					x += xi;
				}

				next_span = cur_span->next;
				if (next_span)
					dx = next_span->start - cur_span->end;
				else
					dx = *right - cur_span->end;

				cur_span = next_span;

				if (dx < 0)
					dx = 0;

				while (dx--)
				{
					if (cols->ready)
					{
						cols->u += cols->du;
					}

					t++;
					cols++;
					x += xi;
				}
			}
		}

		left++;
		right++;

		y += yi;
	}
}

static void
scan_shade24 (y_min, y_max, red, green, blue)
	long y_min, y_max;
	long red, green, blue;
{
	PIXEL24 *p, *t;
	PIXEL24 r, g, b;
	SPAN cur_span;
	SPAN next_span;
	unsigned char *table;
	long dx, dy, dc;
	long color;
	long *left, *right;
	long *left_c, *right_c;

	p = get_frame_buffer_address ();
	p += y_min * width;

	dy = (y_max - y_min) >> 1;
	if (left_edges[dy] < right_edges[dy])
	{
		left = left_edges;
		right = right_edges;
		left_c = left_colors;
		right_c = right_colors;
	}
	else
	{
		left = right_edges;
		right = left_edges;
		left_c = right_colors;
		right_c = left_colors;
	}

	dy = y_max - y_min;
	while (dy--)
	{
		t = p + *left;
		p += width;

		dx = *right - *left;
		if (dx > 0)
		{
			cur_span = span_add (*left, *right, y_max - dy - 1);

			if (cur_span)
			{
				color = *left_c;
				dc = (*right_c - *left_c) /dx;

				if (cur_span->next)
					dx = 0;
				dx = cur_span->start - *left;

				while (dx--)
				{
					t++;
					color += dc;
				}

				while (cur_span)
				{
					dx = cur_span->end - cur_span->start;
					if (dx < 0)
						dx = 0;

					while (dx--)
					{
						r = fix_to_int (fix_mul (color, red)) << 16;
						g = fix_to_int (fix_mul (color, green)) << 8;
						b = fix_to_int (fix_mul (color, blue));
						*t++ = r + g + b;

						color += dc;
					}

					next_span = cur_span->next;
					if (next_span)
						dx = next_span->start - cur_span->end;
					else
						dx = *right - cur_span->end;

					cur_span = next_span;

					while (dx--)
					{
						t++;
						color += dc;
					}
				}
			}
		}

		left++;
		right++;
		left_c++;
		right_c++;
	}
}

static POINTS
scan_calc_y_extrema (pts, y_min, y_max)
	POINTS pts;
	long *y_min, *y_max;
{
	POINT pt;
	POINTS tmp;

	*y_min = height + 1;
	*y_max = -1;

	while (pts)
	{
		pt = points_first (pts);

		if (point_y (pt) < *y_min)
		{
			*y_min = point_y (pt);
			tmp = pts;
		}
		if (point_y (pt) > *y_max)
			*y_max = point_y (pt);

		pts = points_rest (pts);
	}

	return tmp;
}

static POINTS
scan_calc_x_extrema (pts, x_min, x_max)
	POINTS pts;
	long *x_min, *x_max;
{
	POINT pt;
	POINTS tmp;

	*x_min = width + 1;
	*x_max = -1;

	while (pts)
	{
		pt = points_first (pts);

		if (point_x (pt) < *x_min)
		{
			*x_min = point_x (pt);
			tmp = pts;
		}
		if (point_x (pt) > *x_max)
			*x_max = point_x (pt);

		pts = points_rest (pts);
	}

	return tmp;
}

static long *
scan_calc_edge (buf, xs, xe, ys, ye)
	long *buf;
	long xs, xe;
	long ys, ye;
{
	long dx, dy;
	long error, inc;

	dx = xe - xs;
	dy = ye - ys;

	if (ABS (dx) > ABS (dy))
	{
		if (dx < 0)
		{
			inc = -1;
			dx = -dx;
		}
		else
		{
			inc = 1;
		}

		error = -dx /2;
		while (xs != xe)
		{
			error += dy;
			if (error > 0)
			{
				error -= dx;
				*buf++ = xs;
			}

			xs += inc;
		}
	}
	else
	{
		error = -dy /2;
		if (dx < 0)
		{
			dx = -dx;
			inc = -1;
		}
		else
		{
			inc = 1;
		}

		while (ys++ < ye)
		{
			*buf++ = xs;

			error += dx;
			if (error > 0)
			{
				error -= dy;
				xs += inc;
			}
		}
	}

	return buf;
}

static long *
scan_calc_color (buf, is, ie, dy)
	long *buf, is, ie, dy;
{
	long di;

	di = (ie - is) / dy;

	while (dy--)
	{
		*buf++ = is;
		is += di;
	}

	return buf;
}

static void
make_span_buffer ()
{
	span_buffer = (SPAN *) ALLOC (sizeof (SPAN) * height);
	span_full = (BOOLEAN *) ALLOC (sizeof (BOOLEAN) * height);
}

static void
reset_span_buffer ()
{
	short i;

	for (i = 0; i < height; i++)
	{
		span_buffer[i] = make_span (0, width);
		span_full[i] = FALSE;
	}
}

static void
make_span_pool ()
{
	span_pool_size = SPAN_POOL_CHUNK;
	span_pool = (_SPAN *) ALLOC (sizeof (_SPAN) * span_pool_size);
}

static void
grow_span_pool ()
{
}

static void
reset_span_pool ()
{
	span_pool_index = 0;
}

static SPAN
make_span (xs, xe)
	short xs, xe;
{
	SPAN span;

	if (span_pool_index >= span_pool_size)
		fatal_error ("Ran out of spans");

	span = &span_pool[span_pool_index++];
	span->start = xs;
	span->end = xe;
	span->next = NULL;

	return span;
}

static short
span_trivial_reject (ys, ye)
	long ys, ye;
{
	short dy;
	BOOLEAN *full_spans;

	full_spans = &span_full[ys];
	dy = ye - ys;

	if (dy > 0)
		while (dy--)
			if (!(*full_spans++))
				return FALSE;

	return TRUE;
}

static SPAN
span_add (xs, xe, y)
	long xs, xe, y;
{
	SPAN valid_span;

	if ((y < 0) || (y >= height) || span_full[y])
		return NULL;

	valid_span = span_valid (span_buffer[y], xs, xe);
	if (!valid_span)
		return NULL;

	span_buffer[y] = span_remove (span_buffer[y], xs, xe);

	if (!span_buffer[y])
		span_full[y] = TRUE;

	return valid_span;
}

static SPAN
span_valid (span, xs, xe)
	SPAN span;
	long xs, xe;
{
	SPAN valid_span;
	SPAN temp_span;

	valid_span = NULL;
	while (span)
	{
		if ((span->end >= xs) && (span->start <= xe))
		{
			if (valid_span)
			{
				temp_span->next = make_span (MAX (span->start, xs), MIN (span->end, xe));
				temp_span = temp_span->next;
			}
			else
			{
				valid_span = make_span (MAX (span->start, xs), MIN (span->end, xe));
				temp_span = valid_span;
			}
		}

		span = span->next;
	}

	return valid_span;
}

static SPAN
span_remove (span, xs, xe)
	SPAN span;
	long xs, xe;
{
	SPAN cur_span;
	SPAN prev_span;
	SPAN temp_span;
	short major_hack = 0;

	prev_span = NULL;
	cur_span = span;
	while (cur_span)
	{
		if ((xe >= cur_span->start) &&
			(xs <= cur_span->end))
		{
			if ((xe >= cur_span->end) &&
				(xs <= cur_span->start))
			{
				if (prev_span)
				{
					prev_span->next = cur_span->next;
					cur_span = prev_span;
				}
				else
				{
					span = cur_span->next;
					major_hack = 1;
				}
			}
			else if (xe >= cur_span->end)
			{
				cur_span->end = xs;
			}
			else if (xs <= cur_span->start)
			{
				cur_span->start = xe;
			}
			else
			{
				temp_span = make_span (MIN (xe, cur_span->end),
				MAX (xe, cur_span->end));

				temp_span->next = cur_span->next;
				cur_span->end = xs;
				cur_span->next = temp_span;
				cur_span = cur_span->next;
			}
		}

		if (major_hack)
			major_hack = 0;
		else
			prev_span = cur_span;
		cur_span = cur_span->next;
	}

	return span;
}
