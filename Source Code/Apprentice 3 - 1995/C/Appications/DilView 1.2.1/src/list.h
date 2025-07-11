/* list.h */

#include <QDOffScreen.h>

#ifndef __dil_structs__
 #include "structs.h"
#endif

#define k_list_buffer_depth 	1
#define k_list_row_height 		15
#define k_list_item_offset_h 	(k_list_row_height)
#define k_list_item_offset_v 	(k_list_row_height - 3)

GWorldPtr GetGListPort( void );
void SetMarkedFlag( Boolean value );
Boolean GetMarkedFlag( void );
void myInitList( void );
void myDisposeList( void );
void DrawList( void );
short GetNumListCols( void );
short GetNumListRows( void );
short GetNumListItems( void );
Rect *GetCheckBoxRect( Rect *rect, short row, short col );
Rect *GetItemRect( Rect *rect, short row, short col );
Rect *GetListRect( Rect *rect );
dil_rec *GetFirstDil( void );
dil_rec *GetNextDil( dil_rec *dil );
dil_rec *GetNextMarkedDil( dil_rec *dil );
dil_rec *GetPrevDil( dil_rec *dil );
void AddDilToList( dil_rec *dil );
void ListClick( Point localPt );
void SetCurToHead( void );