/*
 * menu.h
 */

#pragma once
#ifndef menu_h
# define menu_h

# ifndef window_h
#  include "window.h"
# endif /* window.h */

enum { APPLE_MENU = 128 , FILE_MENU , FONT_MENU , SIZE_MENU , CLOCK_MENU } ;
enum { ABOUT_ITEM = 1 } ;
enum { NEW_ITEM = 1 , OPEN_ITEM , CLOSE_ITEM , QUIT_ITEM = CLOSE_ITEM + 2 } ;
enum { SMALLER_ITEM = 1 , LARGER_ITEM , SIZE_DELIMITER , FIRST_SIZE_ITEM = LARGER_ITEM + 2 } ;
enum { SECONDS_ITEM = 1 , SMOOTH_ITEM , ANTI_ALIAS_ITEM } ;


void DoMenu ( EventRecord * event ) ;
void SelectMenu ( long selection ) ;
void DispatchMenu ( short menu , short item ) ;

void UpdateMenus ( void ) ;
void UpdateWinMenus ( WindowPtr wp , DefWindowRec * rec ) ;

void MakeMenus ( void ) ;
void SizeMenuOutlines ( short fontNum ) ;

void DisableMenu ( short menu ) ;
void EnableMenu ( short menu ) ;
void DisableAll ( void ) ;

void DisableCmd ( short menu , short item ) ;
void EnableCmd ( short menu , short item ) ;
void CheckCmd ( short menu , short item , Boolean checked ) ;
void CheckStr ( short menu , unsigned char * str , Boolean checked ) ;

#endif /* menu.h */
