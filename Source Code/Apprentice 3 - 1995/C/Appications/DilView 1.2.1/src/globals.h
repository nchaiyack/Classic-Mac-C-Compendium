/* globals.h */

#ifndef __dil_structs__
 #include "structs.h"
#endif

short GetScale( void );
float GetScalePercent( void );
void SetScale( short new_scale );

GrafPtr GetGWindowPort( void );
void SetGWindowPort( CWindowPtr new_port );

dil_rec *GetCurDil( void );
void SetCurDil( dil_rec *dil );


