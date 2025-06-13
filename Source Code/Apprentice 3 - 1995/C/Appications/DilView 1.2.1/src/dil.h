/* dil.h */

#ifndef __dil_structs__
 #include "structs.h"
#endif

// boolean: scale to window then buffer, or scale to window then copy window to buffer
//  bad if window isn't entirely visible (ie part offscreen)
#define slower_but_better 0

#define k_dil_buffer_depth 8
#define k_scaled_dil_buffer_depth 8

void myInitDil( void );
void myDisposeDil( void );
void DrawDil( void );
void LoadDilIntoDilBuffer( dil_rec *dil );
Rect *GetDilRect( Rect *rect );
void DrawDilInRect( dil_rec *dil, Rect *rect );
dil_rec *NewDilRec( void );
dil_rec *GifToDil( FSSpec *spec );
