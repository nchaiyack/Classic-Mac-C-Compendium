/*****
 *
 *	Grant's CGI Framework
 *		(Common Grant Interface :-)
 *		by Grant Neufeld
 *		http://arpp1.carleton.ca/grant/mac/cgi.html
 *
 *	CGI.c
 *
 *	Standard functions for cgi applications.
 *
 *	You must call InitCGIUtil in your application startup.
 *	You must install CGIAEHandle as the event handler for the WWW½sdoc apple event
 *	You must write the function:
 *		void MyCGIProcess ( CGIHdl theCGIHdl )
 *		which is where you will, guess what, do your application specific processing
 *		of the cgi stuff.
 *
 *	Do not call any functions begining with lower case 'cgi' - you can use any of the
 *	others.
 *
 *	watch the homepage for future upgrades
 *
 *	notice of upgrades will be posted to macwwwtool@arpp1.carelton.ca
 *		see http://arpp1.carleton.ca/list/macwwwtool.html for details
 *
 *
 *	Copyright ©1995 by Grant Neufeld
 *
 *	http://arpp1.carleton.ca/grant/
 *	gneufeld@ccs.carleton.ca
 *	grant@acm.org
 *
 *	This source may be freely used as long as the copyright notice is kept in the source.
 *	I ask that you let me know of any enhancements (read: bug fixes) to this code.
 *	I would also like copies of (or discounts on) anything you produce this with, please.
 *
 *	See the License and Limited Warranty Agreement for all the legal stuff.
 *
 *****/

#include <stdlib.h>
#include <string.h>

#include "globals.h"

#include "DebugUtil.h"
#include "MemoryUtil.h"
#include "StringUtil.h"

/* CGI.h processes differently for CGI.c, this is controlled by defining __CGISegment__ */
#define __CGISegment__	1
#include "CGI.h"
#undef	__CGISegment__


/***  CONSTANT DECLARATIONS  ***/

#define kHTTPHeaderStrs		3000
#define kHTTPHeaderOK		1
#define kHTTPHeaderRedirect	2
#define kHTTPHeaderErr		3


/***  LOCAL FUNCTION PROTOTYPES  ***/

void	cgiDisposeHandle		( CGIHdl );

OSErr	cgiAEGetParamString		( AppleEvent *, AEKeyword, char **,		 char *, long );
OSErr	cgiAEGetParamShort		( AppleEvent *, AEKeyword, short *,		 char *, long );
OSErr	cgiAEGetParamHTTPMethod	( AppleEvent *, AEKeyword, HTTPMethod *, char *, long );


/***  FUNCTIONS  ***/

/* This initialization function MUST be called in the startup sequence of your application */
void
InitCGIUtil ( void )
{
	GetIndString	( gHTTPHeaderOK, kHTTPHeaderStrs, kHTTPHeaderOK );
	P2CStr			( gHTTPHeaderOK );
	gHTTPHeaderOKSize = strlen ( (char *)gHTTPHeaderOK );
	
	GetIndString	( gHTTPHeaderRedirect, kHTTPHeaderStrs, kHTTPHeaderRedirect );
	P2CStr			( gHTTPHeaderRedirect );
	gHTTPHeaderRedirectSize = strlen ( (char *)gHTTPHeaderRedirect );
	
	GetIndString	( gHTTPHeaderErr, kHTTPHeaderStrs, kHTTPHeaderErr );
	P2CStr			( gHTTPHeaderErr );
	gHTTPHeaderErrSize = strlen ( (char *)gHTTPHeaderErr );
} /* InitCGIUtil */


/**  FORM FIELDS  **/
#pragma mark -

/* The separator '&' separates individual fields.
	The delimiter '=' delimits the name and value in a field.
	For example: "Field 1=some stuff&Another Field=more stuff&Last Field=no stuff"
	Means that there are 3 fields with names "Field 1", "Another Field" and "Last Field"
	
	The function returns an array of field records with the last containing null values.
	I don't think this should really be called outside this file - I'll have to
	figure that out, though - so use it at your own risk. */
	/* ¥¥¥ I should add error reporting */
CGIFormField *
CGIFormFieldsFromArgs ( char *theString, long *count, short *outErr )
{
	CGIFormField *	theFields;
	long			totalStrSize;
	long			totalFields;
	long			nameSize;
	long			valueSize;
	long			currentField;
	char *			theStringPtr;
	char *			fieldSeparator;
	char *			fieldDelimiter;
	
	my_assert ( theString != nil, "\pCGIFormArgs: nil string" );
	
	theFields = nil;
	
	/* don't return number of fields until function is successful */
	*count			= nil;
	
	totalStrSize	= strlen ( theString );
	
	/* the total number of fields is the number of separator characters + 1 */
	totalFields		= StringCountChar ( theString, kCGIFormFieldSeparator ) + 1;
	
	if ( totalFields == 1 )
	{
		/* the case where there were no separator characters is special,
			test for a field delimiter to confirm that the string passed does
			indeed contain field information */
		fieldDelimiter = strchr ( theString, kCGIFormFieldDelimiter );
		
		if ( fieldDelimiter == nil )
		{
			/* string does not contain field data */
			*outErr = 1;
			
			goto Exit_Fail;
		}
	}
	
	theFields = (CGIFormField *) MyNewPtr ( ((totalFields + 1) * sizeof(CGIFormField)), outErr );
	
	if ( theFields == nil )
	{
		/* memory didn't allocate */
		goto Exit_Fail;
	}
	
	/* set the name and value of the last field in the array to nil */
	(theFields[totalFields]).name	= nil;
	(theFields[totalFields]).value	= nil;
	
	theStringPtr	= theString;
	
	for ( currentField = nil; currentField < totalFields; currentField++ )
	{
		fieldDelimiter	= strchr ( theStringPtr, kCGIFormFieldDelimiter );
		fieldSeparator	= strchr ( theStringPtr, kCGIFormFieldSeparator );
		
		/* if there is a field delimiter, and it is before any field separator */
		if ( (fieldDelimiter != nil) && ((fieldSeparator > fieldDelimiter) || (fieldSeparator == nil)) )
		{
			/* field name */
			/* the size of the name string is the difference between the begining of the
				field and the position of the field delimiter */
			nameSize = fieldDelimiter - theStringPtr;
			
			/* allocate the name string */
			(theFields[currentField]).name = (char *) MyNewPtr ( nameSize + 1, outErr );
			
			if ( (theFields[currentField]).name == nil )
			{
				/* memory didn't allocate */
				(theFields[currentField]).value = nil;
				
				goto Exit_Fail;
			}
			
			/* copy the field name */
			BlockMove ( theStringPtr, (theFields[currentField]).name, nameSize );
			/* null terminate the end of the name string */
			((theFields[currentField]).name)[nameSize] = nil;
			/* convert the url encoded text to a normal string */
			CGIDecodeSpecialChars ( (theFields[currentField]).name );
			
			/* field value */
			if ( fieldSeparator != nil )
			{
				valueSize = fieldSeparator - (fieldDelimiter + 1);
			}
			else
			{
				valueSize = strlen ( fieldDelimiter + 1 );
			}
			
			(theFields[currentField]).value = (char *) MyNewPtr ( (valueSize + 1), outErr );
			
			if ( (theFields[currentField]).value == nil )
			{
				/* memory didn't allocate */
				DisposePtr ( (theFields[currentField]).name );
				(theFields[currentField]).name = nil;
				
				goto Exit_Fail;
			}
			
			BlockMove ( fieldDelimiter + 1, (theFields[currentField]).value, valueSize );
			((theFields[currentField]).value)[valueSize] = nil;
			CGIDecodeSpecialChars ( (theFields[currentField]).value );
			
			theStringPtr = fieldSeparator + 1;
		}
		else
		{
			/* invalid data encountered */
			*outErr = 2;
			
			goto Exit_Fail;
		}
	}
	
	/* assign the return parameters values */
	*count	= totalFields;
	*outErr	= nil;
	
	return theFields;
	
	
	Exit_Fail:
	
	if ( theFields != nil )
	{
		/* release allocated memory */
		CGIFormFieldsDispose ( theFields );
	}
	
	return nil;
} /* CGIFormFieldsFromArgs */


/* Returns a pointer to the first form field record, in the given fieldArray,
	that's name matches the supplied field name */
CGIFormField *
CGIFormFieldsFindRecord ( CGIFormField *fieldArray, char *fieldName )
{
	long	currentField;
	
	/* look til we find something or we hit the end */
	for ( currentField = nil; (fieldArray[currentField]).name != nil; currentField++ )
	{
		if ( strcmp((fieldArray[currentField]).name, fieldName) == nil )
		{
			/* found a match, so we're done */
			return &(fieldArray[currentField]);
		}
	}
	
	/* didn't find a match */
	return nil;
} /* CGIFormFieldsFindRecord */


/* Deallocate memory for theFields array.
	I don't think this should be called outside this file - use at your own risk */
void
CGIFormFieldsDispose ( CGIFormField *theFields )
{
	long	offset;
	
	my_assert ( theFields != nil, "\pCGIFormFieldsDispose: null field array pointer" );
	
	for ( offset = nil; (theFields[offset]).name != nil; offset++ )
	{
		if ( (theFields[offset]).name != nil )
		{
			/* if there's a name string, deallocate its memory */
			DisposePtr ( (Ptr)((theFields[offset]).name) );
		}
		
		if ( (theFields[offset]).value != nil )
		{
			/* if there's a value string, deallocate its memory */
			DisposePtr ( (Ptr)((theFields[offset]).value) );
		}
	}
	
	DisposePtr ( (Ptr)theFields );
} /* CGIFormFieldsDispose */


/**  CHARACTER CODING  **/
#pragma mark -

/* replaces instances of percent signs (%) followed by an ASCII char value
	with the actual character.
	This function modifies theString parameter! */
void
CGIDecodeSpecialChars ( char *theString )
{
	int				read;
	int				write;
	unsigned char	theChar;
	unsigned char	highOrder;
	unsigned char	lowOrder;
	Boolean			isValid;
	
	my_assert ( theString != nil, "\pCGIDecodeSpecialChars: nil string" );
	
	read	= nil;
	write	= nil;
	
	while ( theString[read] != nil )
	{
		switch ( theString[read] )
		{
			case '%':
				/* a percent symbol begins a hex char block (%## where ## is the hex value) */
				
				isValid = true;
				
				/* determine high order hex character */
				if ( (theString[read+1] >= 'A') && (theString[read+1] <= 'F') )
				{
					/* uppercase A-F convert to 10-15 */
					highOrder = theString[read+1] - 'A' + 10;
				}
				else if ( (theString[read+1] >= 'a') && (theString[read+1] <= 'f') )
				{
					/* lowercase a-f convert to 10-15 */
					highOrder = theString[read+1] - 'a' + 10;
				}
				else if ( (theString[read+1] >= '0') && (theString[read+1] <= '9') )
				{
					/* character digits 0-9 convert to decimal 0-9 */
					highOrder = theString[read+1] - '0';
				}
				else
				{
					/* Illegal character! Can't convert from hex */
					isValid = false;
				}
				
				/* Multiply high order hex digit by 16 */
				highOrder *= 16;
				
				/* determine low order hex character */
				if ( (theString[read+2] >= 'A') && (theString[read+2] <= 'F') )
				{
					/* uppercase A-F convert to 10-15 */
					lowOrder = (theString[read+2] - 'A' + 10);
				}
				else if ( (theString[read+2] >= 'a') && (theString[read+2] <= 'f') )
				{
					/* lowercase a-f convert to 10-15 */
					lowOrder = (theString[read+2] - 'a' + 10);
				}
				else if ( (theString[read+1] >= '0') && (theString[read+1] <= '9') )
				{
					/* character digits 0-9 convert to decimal 0-9 */
					lowOrder = (theString[read+2] - '0');
				}
				else
				{
					/* Illegal character! Can't convert from hex */
					isValid = false;
				}
				
				theChar = highOrder + lowOrder;
				
				if ( isValid )
				{
					isValid = (theChar >= 0) && (theChar < 256);
				}
				
				if ( isValid )
				{
					/* if theChar is valid, write it out */
					
					if ( theChar == 10 )
					{
						/* don't write newline */
						write--;
					}
					else
					{
						theString[write] = theChar;
					}
					
					/* Increment read past the two digits of the hex code */
					read += 2;
				}
				else
				{
					/* invalid hex character code, just write out the percent symbol */
					theString[write] = theString[read];
				}
				break;
			
			case '+':
				/* Plus symbols convert to space */
				theString[write] = ' ';
				break;
				
			case 10:
				/* ignore line feeds, we only need carriage returns (13) */
				write--;
				break;
				
			default:
				/* write out the character */
				theString[write] = theString[read];
				break;
		}
			
		read++;
		write++;
	}
	
	/* terminate the string */
	theString[write] = '\0';
} /* CGIDecodeSpecialChars */


#define kCGIEncodeExtraChars		16

/* %hex encode all non-alphanumeric characters
	theString parameter is not modified */
char *
CGIEncodeSpecialChars ( char *theString )
{
	char *	theResult;
	long	strSize;		/* size of the source string */
	long	tempSize;		/* current size of the result */
	long	strOffset;		/* pointer to current read position in the source string */
	long	resultOffset;	/* pointer to current write position in the result string */
	
	strSize		= strlen ( theString );
	tempSize	= strSize + kCGIEncodeExtraChars;
	theResult	= (char *) MyNewPtr ( (tempSize + 1), nil );
	
	for ( strOffset = resultOffset = nil; strOffset < strSize; strOffset++, resultOffset++ )
	{
		if ( resultOffset > tempSize )
		{
			/* the current size of the result string isn't big enough, so grow it */
			tempSize += kCGIEncodeExtraChars;
			SetPtrSize ( (Ptr)theResult, tempSize + 1 );
		}
		
		if ( ((theString[strOffset] >= '0') && (theString[strOffset] <= '9')) ||
			((theString[strOffset] >= 'A') && (theString[strOffset] <= 'Z')) ||
			((theString[strOffset] >= 'a') && (theString[strOffset] <= 'z')) )
		{
			/* if the character is alphanumeric just copy it */
			theResult[resultOffset] = theString[strOffset];
		}
		else
		{
			/* if the character is not alphanumeric, hex encode it */
			
			if ( (resultOffset + 2) > tempSize )
			{
				/* the current size of the result string isn't big enough, so grow it */
				tempSize += kCGIEncodeExtraChars;
				SetPtrSize ( (Ptr)theResult, tempSize + 1 );
			}
		
			/* hex encode character */
			CGICharToHex ( theString[strOffset], &(theResult[resultOffset]) );
			/* add the extra two characters for hex encoding to the result offset */
			resultOffset += 2;
		}
	}
	
	/* increment the offset to accomodate the string terminator */
	resultOffset++;
	
	if ( resultOffset > tempSize )
	{
		/* the current size of the result string isn't big enough, so grow it */
		tempSize += kCGIEncodeExtraChars;
		SetPtrSize ( (Ptr)theResult, tempSize + 1 );
	}
	
	/* terminate the string */
	theResult[resultOffset] = nil;
	
	if ( resultOffset != tempSize )
	{
		/* shrink the memory for the result string to exactly match it's needs */
		SetPtrSize ( (Ptr)theResult, resultOffset + 1 );
	}
	
	return theResult;
} /* CGIEncodeSpecialChars */


/* Converts a character to percent-hex encoding and writes that to the string */
void
CGICharToHex ( unsigned char theChar, char *theString )
{
	unsigned char	chrChunk;
	
	/* start off with a percent symbol to indicate a hex character code */
	theString[0] = '%';
	
	/* mask to get the high 4 bits, then shift them into the lower 4 bits */
	chrChunk = ( theChar & 0xF0 ) >> 4;
	
	if ( chrChunk > 9 )
	{
		/* chrChunk is a value between A and F in hex */
		theString[1] = ( chrChunk - 10 ) + 'A';
	}
	else
	{
		/* chrChunk is a value between 0 and 9 in hex */
		theString[1] = chrChunk + '0';
	}
	
	/* mask to get the low 4 bits */
	chrChunk = theChar & 0x0F;
	
	if ( chrChunk > 9 )
	{
		/* chrChunk is a value between A and F in hex */
		theString[2] = ( chrChunk - 10 ) + 'A';
	}
	else
	{
		/* chrChunk is a value between 0 and 9 in hex */
		theString[2] = chrChunk + '0';
	}
} /* CGICharToHex */


#pragma mark -

/*  */
void
cgiDisposeHandle ( CGIHdl theCGIHdl )
{
	if ( (*theCGIHdl)->path_args != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->path_args) );
	}
	if ( (*theCGIHdl)->http_search_args != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->http_search_args) );
	}
	if ( (*theCGIHdl)->username != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->username) );
	}
	if ( (*theCGIHdl)->password != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->password) );
	}
	if ( (*theCGIHdl)->from_user != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->from_user) );
	}
	if ( (*theCGIHdl)->client_address != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->client_address) );
	}
	if ( (*theCGIHdl)->post_args != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->post_args) );
	}
	if ( (*theCGIHdl)->server_name != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->server_name) );
	}
	if ( (*theCGIHdl)->script_name != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->script_name) );
	}
	if ( (*theCGIHdl)->content_type != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->content_type) );
	}
	if ( (*theCGIHdl)->referer != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->referer) );
	}
	if ( (*theCGIHdl)->user_agent != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->user_agent) );
	}
	if ( (*theCGIHdl)->action != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->action) );
	}
	if ( (*theCGIHdl)->action_path != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->action_path) );
	}
	if ( (*theCGIHdl)->client_ip != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->client_ip) );
	}
	if ( (*theCGIHdl)->full_request != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->full_request) );
	}
	
	if ( (*theCGIHdl)->responseData != nil )
	{
		DisposePtr ( (Ptr)((*theCGIHdl)->responseData) );
	}
	
	if ( (*theCGIHdl)->formFields != nil )
	{
		CGIFormFieldsDispose ( (*theCGIHdl)->formFields );
	}
} /* cgiDisposeHandle */


/**  APPLE EVENT SUPPORT  **/
#pragma mark -

#pragma segment AppleEvent
/* AppleEvent Handler for the CGI WWW½-sdoc event */
pascal OSErr
CGIAESearchDoc ( AppleEvent *theAppleEvent, AppleEvent *TheReply, long Reference )
{
	OSErr		theErr;
	DescType	actualType;
	Ptr			tempBuffer;
	CGIHdl		theCGIHdl;
	short		fieldError;
	
	theCGIHdl	= (CGIHdl) MyNewHandleClear ( sizeof(CGIrecord), &theErr );
	
	if ( theCGIHdl == nil )
	{
		return theErr;
	}
	
	/* store references to the apple event and reply records */
	(*theCGIHdl)->appleEvent	= theAppleEvent;
	(*theCGIHdl)->replyEvent	= TheReply;
	
	tempBuffer = MyNewPtr ( kCGIParamMaxSize, &theErr );
	
	if ( tempBuffer == nil )
	{
		DisposeHandle ( (Handle)theCGIHdl );
		
		return theErr;
	}
	
	actualType = (DescType) 'char';
	
	HLockHi ( (Handle)theCGIHdl );
	
	/* '----' - direct parameter:
		path_args - arguments to the URL after a $ */
	theErr = cgiAEGetParamString ( theAppleEvent, '----', &((*theCGIHdl)->path_args),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	if ( theErr == noErr )
	{
		CGIDecodeSpecialChars ( (*theCGIHdl)->path_args );
	}
	
	/* 'kfor' - search arguments:
		http_search_args - arguments to the URL after a ? */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIhttp_search_args, &((*theCGIHdl)->http_search_args),
		(char *)tempBuffer, kCGIParamMaxSize );
	/* leave decoding to after parsing of form fields */
	
	/* 'user' - user name:
		username - authenticated user name */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIusername, &((*theCGIHdl)->username),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'pass' - password:
		password - authenticated password */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIpassword, &((*theCGIHdl)->password),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'frmu' - from user:
		from_user - non-standard. e-mail address of remote user */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIfrom_user, &((*theCGIHdl)->from_user),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'addr' - client address:
		client_address - IP address or domain name of remote client's host */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIclient_address, &((*theCGIHdl)->client_address),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'post' - post arguments:
		post_args -  */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIpost_args, &((*theCGIHdl)->post_args),
		(char *)tempBuffer, kCGIParamMaxSize );
	/* leave decoding to after parsing of form fields */
	
	/* 'meth' - HTTP method:
		method - GET, POST, etc. Used to tell if post_args are valid */
	theErr = cgiAEGetParamHTTPMethod ( theAppleEvent, kCGImethod, &((*theCGIHdl)->method),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'svnm' - server name:
		server_name - name or IP address of this server */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIserver_name, &((*theCGIHdl)->server_name),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'svpt' - server port:
		server_port - TCP/IP port number being used by this server */
	theErr = cgiAEGetParamShort ( theAppleEvent, kCGIserver_port, &((*theCGIHdl)->server_port),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'scnm' - script name:
		script_name - URL name of this script */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIscript_name, &((*theCGIHdl)->script_name),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'ctyp' - content type:
		content_type - MIME content type of post_args */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIcontent_type, &((*theCGIHdl)->content_type),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'refr' - referer:
		referer - the URL of the page referencing this document */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIreferer, &((*theCGIHdl)->referer),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'Agnt' - user agent:
		user_agent - the name and version of the WWW client software being used */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIuser_agent, &((*theCGIHdl)->user_agent),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'Kact' - action name:
		action - the name of the action (CGI or ACGI if not a user defined action) */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIaction, &((*theCGIHdl)->action),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'Kapt' - action path:
		action_path - path to the action application */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIaction_path, &((*theCGIHdl)->action_path),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'Kcip' - client IP address:
		client_ip - the IP address of the client */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIclient_ip, &((*theCGIHdl)->client_ip),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* 'Kfrq' - full request:
		full_request - the full text of the request */
	theErr = cgiAEGetParamString ( theAppleEvent, kCGIfull_request, &((*theCGIHdl)->full_request),
		(char *)tempBuffer, kCGIParamMaxSize );
	
	/* separate the form fields into an array */
	switch ( (*theCGIHdl)->method )
	{
		case HTTP_get :
			if ( (*theCGIHdl)->http_search_args != nil )
			{
				(*theCGIHdl)->formFields = CGIFormFieldsFromArgs ( (*theCGIHdl)->http_search_args, &((*theCGIHdl)->totalFields), &fieldError );
			}
			break;
			
		case HTTP_post :
		default :
			if ( (*theCGIHdl)->post_args != nil )
			{
				(*theCGIHdl)->formFields = CGIFormFieldsFromArgs ( (*theCGIHdl)->post_args, &((*theCGIHdl)->totalFields), &fieldError );
			}
			break;
	}
	
	/* now that the possible need to use them for form fields is over, we can
		decode the search args */
	CGIDecodeSpecialChars ( (*theCGIHdl)->http_search_args );
	
	/* this is where the application specific cgi handling comes into play
		the function "MyCGIProcess" must be provided by the user of this source code */
	
	HUnlock			( (Handle)theCGIHdl );
	MyCGIProcess	( theCGIHdl );
	HLock			( (Handle)theCGIHdl );
	
	if ( (*theCGIHdl)->responseData != nil )
	{
		/* if the user's "MyCGIProcess" function set the responseData properly,
			return it */
		theErr = AEPutParamPtr ( TheReply, keyDirectObject, typeChar,
			(Ptr)((*theCGIHdl)->responseData), (*theCGIHdl)->responseSize );
	}
	else
	{
		/* if the user's "MyCGIProcess" failed to set the responseData properly,
			return an error header */
		theErr = AEPutParamPtr ( TheReply, keyDirectObject, typeChar, (Ptr)gHTTPHeaderErr, gHTTPHeaderErrSize );
	}
	
	HUnlock ( (Handle)theCGIHdl );
	
	/* deallocate memory */
	cgiDisposeHandle ( theCGIHdl );
	
	return theErr;
} /* CGIAESearchDoc */


#pragma segment AppleEvent
/* private function to get a string from a parameter */
OSErr
cgiAEGetParamString ( AppleEvent *theAppleEvent, AEKeyword theAEKeyword, char **theString, char *tempBuffer, long bufferSize )
{
	OSErr		theErr;
	DescType	actualType;
	Size		actualSize;
	
	my_assert ( *theString == nil, "\pcgiAEGetParamString: non-nil string received" );
	
	theErr = AEGetParamPtr ( theAppleEvent, theAEKeyword, typeChar,
		&actualType, (Ptr)tempBuffer, bufferSize, &actualSize );
	
	if ( theErr == noErr )
	{
		/* if the parameter was present, allocate space to copy it into the CGI */
		*theString = (char *) MyNewPtr ( actualSize + 1, &theErr );
		
		if ( *theString != nil )
		{
			/* copy the tempBuffer into the CGI handle */
			BlockMove ( tempBuffer, *theString, actualSize );
			
			/* put a null terminator at the end of the string */
			(*theString)[actualSize] = nil;
		}
	}
	
	return theErr;
} /* cgiAEGetParamString */


#pragma segment AppleEvent
/* private function to get a short from a parameter */
OSErr
cgiAEGetParamShort ( AppleEvent *theAppleEvent, AEKeyword theAEKeyword, short *theShort, char *tempBuffer, long bufferSize )
{
	OSErr		theErr;
	DescType	actualType;
	Size		actualSize;
	
	theErr = AEGetParamPtr
		( theAppleEvent, theAEKeyword, typeChar, &actualType, (Ptr)tempBuffer, bufferSize, &actualSize );
	
	if ( theErr == noErr )
	{
		/* terminate the buffer with a null byte */
		tempBuffer[actualSize] = nil;
		
		*theShort = atoi ( tempBuffer );
	}
	
	return theErr;
} /* cgiAEGetParamShort */


#pragma segment AppleEvent
/* private function to get an HTTPMethod from a parameter */
OSErr
cgiAEGetParamHTTPMethod ( AppleEvent *theAppleEvent, AEKeyword theAEKeyword, HTTPMethod *theMethod, char *tempBuffer, long bufferSize )
{
	OSErr		theErr;
	DescType	actualType;
	Size		actualSize;
	int			stringDiff;
	
	theErr = AEGetParamPtr
		( theAppleEvent, theAEKeyword, typeChar, &actualType, (Ptr)tempBuffer, bufferSize, &actualSize );
	
	if ( theErr == noErr )
	{
		/* terminate the buffer with a null byte */
		tempBuffer[actualSize] = nil;
		
		/* compare the buffer with constants to determine the http method used */
		
		stringDiff = strcmp ( tempBuffer, kCGIHTTPMethodPost );
		
		if ( stringDiff == nil )
		{
			*theMethod = HTTP_post;
		}
		else
		{
			stringDiff = strcmp ( tempBuffer, kCGIHTTPMethodGet );
			
			if ( stringDiff == nil )
			{
				*theMethod = HTTP_get;
			}
			else
			{
				*theMethod = HTTP_UNDEFINED;
			}
		}
	}
	
	return theErr;
} /* cgiAEGetParamHTTPMethod */


/***  EOF  ***/
