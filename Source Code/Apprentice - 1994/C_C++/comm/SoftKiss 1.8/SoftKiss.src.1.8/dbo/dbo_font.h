/*
 * dbo_font.h - debugging print system
 * by Aaron Wohl
 * Public domain
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15208
 * home: 412-731-6159
 * work: 412-268-5032
 */

#define DBO_CHARSET_SIZE 	(128)
#define DBO_LINE_HEIGHT  	(10)
#define DBO_LINE_VOFFSET	 (8)
#define DBO_CHAR_PIXEL_WIDTH (8)

#ifndef DBO_FONT_EXTERN
#define DBO_FONT_EXTERN extern
#endif

DBO_FONT_EXTERN unsigned char dbo_draw_font[DBO_CHARSET_SIZE][DBO_LINE_HEIGHT];
