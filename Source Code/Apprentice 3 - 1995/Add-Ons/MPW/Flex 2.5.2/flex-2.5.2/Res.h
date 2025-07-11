/***
 *
 * Res.h - by Christopher E. Hyde, 95-06-29
 *
 ***/

#define	ByteTable	'BTbl'
#define	ShortTable	'STbl'
#define	LongTable	'LTbl'

// scan/lex tables
#define	ryy_accept	130
#define	ryy_ec		131
#define	ryy_meta	132
#define	ryy_base	133
#define	ryy_def		134
#define	ryy_nxt		135
#define	ryy_chk		136

// parse/yacc tables
#define	ryylhs		150
#define	ryylen		151
#define	ryydefred	152
#define	ryydgoto	153
#define	ryysindex	154
#define	ryyrindex	155
#define	ryygindex	156
#define	ryytable	157
#define	ryycheck	158


#ifndef rez

#include <Resources.h>

#define	BLoad(n)	n = (char*)  *Get1Resource(ByteTable,  r##n)
#define	SLoad(n)	n = (short*) *Get1Resource(ShortTable, r##n)
#define	LLoad(n)	n = (int*)   *Get1Resource(LongTable,  r##n)

#else

#define	yyResFlags	preload, locked /*, protected*/

type ByteTable {		// Table of bytes
	array Data {
		byte;
	};
};

type ShortTable {		// Table of short integers
	array Data {
		integer;
	};
};

type LongTable {		// Table of long integers
	array Data {
		longint;
	};
};

#endif
