#pragma once
/*****
 *
 *	Grant's CGI Framework (Common Grant Interface :-)
 *		http://arpp1.carleton.ca/grant/mac/cgi.html
 *
 *	CGI.h
 *
 *	standard types and function prototypes for cgi applications
 *	See the Read Me or CGI.c for instructions on using the CGI Utilities
 *
 *	#include this file in your source files that need to access the cgi module
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright �1995 by Grant Neufeld
 *
 *	http://arpp1.carleton.ca/grant/
 *	gneufeld@ccs.carleton.ca
 *	grant@acm.org
 *
 *****/


/***  CONSTANT DECLARATIONS  ***/

#define kCGIParamMaxSize		32767

#define kCGIHTTPMethodGet		"GET"
#define kCGIHTTPMethodPost		"POST"

#define kCGIFormFieldDelimiter	'='
#define kCGIFormFieldSeparator	'&'

#define kCGIpath_args			'----'
#define kCGIhttp_search_args	'kfor'
#define kCGIusername			'user'
#define kCGIpassword			'pass'
#define kCGIfrom_user			'frmu'
#define kCGIclient_address		'addr'
#define kCGIpost_args			'post'
#define kCGImethod				'meth'
#define kCGIserver_name			'svnm'
#define kCGIserver_port			'svpt'
#define kCGIscript_name			'scnm'
#define kCGIcontent_type		'ctyp'
#define kCGIreferer				'refr'
#define kCGIuser_agent			'Agnt'
#define kCGIaction				'Kact'
#define kCGIaction_path			'Kapt'
#define kCGIclient_ip			'Kcip'
#define kCGIfull_request		'Kfrq'


/***  TYPE DECLARATIONS  ***/

typedef struct
{
	char *	name;
	char *	value;
} CGIFormField;

typedef enum
{
	HTTP_UNDEFINED,
	HTTP_get,
	HTTP_post
} HTTPMethod;

typedef struct
{
	char *		path_args;			/* '----' path_args			*/
	char *		http_search_args;	/* 'kfor' http_search_args	*/
	char *		username;			/* 'user' username			*/
	char *		password;			/* 'pass' password			*/
	char *		from_user;			/* 'frmu' from_user			*/
	char *		client_address;		/* 'addr' client_address	*/
	char *		post_args;			/* 'post' post_args			*/
	HTTPMethod	method;				/* 'meth' method			*/
	char *		server_name;		/* 'svnm' server_name		*/
	short		server_port;		/* 'svpt' server_port		*/
	char *		script_name;		/* 'scnm' script_name		*/
	char *		content_type;		/* 'ctyp' content_type		*/
	char *		referer;			/* 'refr' referer			*/
	char *		user_agent;			/* 'Agnt' user_agent		*/
	char *		action;				/* 'Kact' action			*/
	char *		action_path;		/* 'Kapt' action_path		*/
	char *		client_ip;			/* 'Kcip' client_ip			*/
	char *		full_request;		/* 'Kfrq' full_request		*/
	
	CGIFormField *	formFields;		/* the fields from form submission */
	long			totalFields;	/* total number of fields	*/
	
	char *		responseData;		/* the data to be returned	*/
	long		responseSize;		/* the size of the response	*/
	
	AppleEvent *	appleEvent;		/* originating appleEvent	*/
	AppleEvent *	replyEvent;		/* apple event reply record	*/
} CGIrecord;

typedef CGIrecord ** CGIHdl;


/***  GLOBAL DECLARATIONS  ***/

#ifdef __CGISegment__
#define _GLOBAL_	
#else
#define _GLOBAL_	extern
#endif

/* these are globals for holding the standard http headers.
	One of the headers must be prepended to the data returned in the Apple Event */

_GLOBAL_	Str255	gHTTPHeaderOK;				/* use data returned after header	*/
_GLOBAL_	Str255	gHTTPHeaderRedirect;		/* redirect client to different url	*/
_GLOBAL_	Str255	gHTTPHeaderErr;				/* an application level error		*/
_GLOBAL_	long	gHTTPHeaderOKSize;
_GLOBAL_	long	gHTTPHeaderRedirectSize;
_GLOBAL_	long	gHTTPHeaderErrSize;

#undef _GLOBAL_


/***  FUNCTION PROTOTYPES  ***/

		void	InitCGIUtil	( void );
		
		CGIFormField *	CGIFormFieldsFromArgs	( char *, long *, short * );
		CGIFormField *	CGIFormFieldsFindRecord	( CGIFormField *, char * );
		void			CGIFormFieldsDispose	( CGIFormField * );
		
		void			CGIDecodeSpecialChars	( char * );
		char *			CGIEncodeSpecialChars	( char * );
		void			CGICharToHex			( unsigned char, char * );

pascal	OSErr	CGIAESearchDoc	( AppleEvent *, AppleEvent *, long );


/***  EXTERNAL FUNCTION PROTOTYPES  ***/

	/* this is the function which you must define for your particular application */
	void	MyCGIProcess		( CGIHdl );


/***  EOF  ***/
