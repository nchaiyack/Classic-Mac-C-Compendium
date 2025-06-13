/* 	Tools Plus 2.6 -- "C / Pascal String Handles" supplemental file
 *	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *	re: Editing Fields
 *
 *	Each Tools Plus editing field uses a “string handle” (ie: a handle to a Pascal
 *	string) to store the text that is displayed in the field.  The following
 *	routines have been provided to facilitate moving C strings into Pascal Handle
 *	structures, and vice versa.
 *
 *	A Pascal string’s first byte (byte-0) is a length byte, and the string is not
 *	null-terminated. Pascal strings are limited to 255 characters.
 *
 *	NOTE: These routines do not check to determine if the destination variable
 *				is large enough to accommodate the text being copied there.  You may
 *				want to modify these routines to do such validation.
 */

#pragma options align=mac68k




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Copy a C string into a Pascal “string handle”
 */
void Cstr2PHdl(char CSource[], Handle PDest);	// (Prototype)
void Cstr2PHdl(char CSource[],								/*Source C string										*/
							 Handle PDest)									/*Handle to destination Pascal			*/
																							/*	string.													*/
	{
	short			length;														/*Source string's length						*/


	length = strlen(CSource);										/*Determine length of source string	*/
	(**PDest) = (unsigned char)length;					/*Copy length-byte into byte-0			*/
	BlockMove(CSource, (*PDest) + 1, length);		/*Copy source C string (omitting		*/
																							/*	null terminator) to the Pascal	*/
																							/*	string right after the length-	*/
	}																						/*	byte.														*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Copy the contents of a Pascal “string handle” into a C string
 */
void PHdl2Cstr(Handle PSource, char CDest[]);	//	(Prototype)
void PHdl2Cstr(Handle PSource,								/*Handle to source Pascal string		*/
							 char CDest[])									/*Destination C string							*/
	{
	short			length;														/*Source string's length						*/

	length = (unsigned char)(**PSource);				/*Determine length of source string	*/
	BlockMove((*PSource) + 1, CDest, length);		/*Copy source Pascal string					*/
																							/*	(omitting the length byte) to		*/
																							/*	the C string.										*/
	CDest[length] = '\0';												/*Null-terminate the C string				*/
	}																						/*																	*/



#pragma options align=reset