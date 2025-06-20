/****************************************************************/
/*																*/
/* Password.h													*/
/*																*/
/* A couple routines to set and get a password from the user.	*/
/* The password is displayed as bullets:						*/
/*																*/
/* SetPassword			Verifies an old password and asks for	*/
/*						a new password.							*/
/* AskPassword			Asks for a password.					*/
/*																*/
/* Version 1.0 � June 1994										*/
/* �1994 T&T Software � by Tad Woods � 70312,3552 on CompuServe	*/
/*																*/
/* You may use these routines in your own applications.			*/
/* They have been tested but the author will not be				*/
/* responsible for any unforseen damage caused by using			*/
/* these routines. Send problem reports or comments to			*/
/* the address above.											*/
/*																*/
/****************************************************************/

#pragma once


/****************************************************************/
/*																*/
/* SetPassword													*/
/*																*/
/* <-> password			On entry, contains the old password		*/
/*						which is required to change the 		*/
/*						password, or an empty string.			*/
/*						On exit, contains the new password		*/
/*						if the function returns 1.				*/
/*																*/
/* Returns:  0 User pressed Cancel.								*/
/*			 1 The password is successfully changed.			*/
/*			-1 The old password was not verified correctly.		*/
/*			-2 The new password was not typed the same twice.	*/
/*																*/
/****************************************************************/

pascal short SetPassword(Str32 password);



/****************************************************************/
/*																*/
/* AskPassword													*/
/*																*/
/*  -> password			Contains the password.					*/
/*																*/
/* Returns:  0 User pressed Cancel.								*/
/*			 1 The password is successfully entered.			*/
/*			-1 The old password was not valid.					*/
/*																*/
/****************************************************************/

pascal short AskPassword(Str32 password);
