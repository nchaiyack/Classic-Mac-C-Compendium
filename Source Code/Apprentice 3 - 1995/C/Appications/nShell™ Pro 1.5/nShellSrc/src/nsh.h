/* ==========================================

	nsh.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#define  HACKING		0			// set to 1 to run hack.c in place of real commands
#define  PROFESSIONAL	1			// set to 1 to allow flow commands
#define  FULL_APP		1			// set to 0 to build drag 'n drop app

#define APPLICATION_SIGNATURE	'NSHA'
#define DROP_SIGNATURE			'NSHS'
#define TEACHTEXT_SIGNATURE		'ttxt'
#define COMMAND_FILE			'NSHC'
#define TRANSCRIPT_FILE			'NSHS'
#define SCRIPT_FILE				'TEXT'

#define C_CMD					'NSHC'

#define RETURN_KEY	'\r'
#define NEWLINE_KEY	'\n'
#define CONTROL_C	'\03'
#define CONTROL_D	'\04'
