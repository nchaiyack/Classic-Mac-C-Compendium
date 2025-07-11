/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains the lexical analyzer and preprocessor for Harvest C. The
 * preprocessor is not implemented as a separate stage, but as layers within
 * the lexer.
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "structs.h"

#pragma segment LexToken

/*
 * The ParseBuffer facilitates the parser's ability to put a token back if it
 * decides it has read too many.
 */
int                             ParseBufferNdx = 0;
struct PBnodeS                  ParseBuffer[16];

Codigo_t
GetaToken()
{				/* QQQQ routine too long ? */
    /*
     * This routine is a relatively high level routine.  It calls
     * GetCharacter to obtain the input from the source file. It reads one
     * token from the source input and return the kind of that token.  It
     * leaves the token string in a global string ThisToken.
     */
    int                             ndx;
    char                            c;
    char                            c2;
    int                             stop;
    Codigo_t                        result;
    ndx = 0;
    stop = 0;
    result = 0;
    /*
     * Note that the use of GetCharacter here for obtaining the first
     * character of the next token implies that preprocessor directives have
     * already been handled and that this character will not be a space.
     */
    c = GetCharacter();
    if (gAllDone || PartDone)	/* This occurs if the SRC stack empties */
	return -1;
    /*
     * First, we check if the character found is a keyword of its own.
     */
    ThisToken[0] = c;
    ThisToken[1] = 0;
    ndx = 1;
    if (isKeyword(ThisToken)) {
	/*
	 * If this single character is a keyword, then we keep expanding it
	 * by one character, to obtain the longest keyword possible. This is
	 * necessary so that != is not parsed as two tokens (for example).
	 * Note also, there are NO single character keywords which might also
	 * be identifiers.  Also, all operators which are composed of
	 * multiple characters have as their first character, a character
	 * which is also a keyword in itself. This is important, because this
	 * fragment here is the only opportunity for parsing of operator
	 * keywords. This list of valid keywords is in isKeyword().  This
	 * routine has a special check for the case of a + or - followed by a
	 * digit, in which case the prefix sign should NOT be parsed as a
	 * keyword, and the routine needs to fall down into the numeric
	 * constant part of this function.
	 */
	/*
	 * This routine handles a unary plus or minus depending on the
	 * context, or the last token read.  QQQQ Perhaps, this should be
	 * handled by the parser itself, instead of here in the lexer. There
	 * IS an entry for unary plus and minus under unary_expr...
	 */
	while (!stop) {
	    c = TokenCharacter();
	    if (!PartDone) {
		ThisToken[ndx++] = c;
		ThisToken[ndx] = 0;
		if (!strcmp(ThisToken, "..")) {
		    ThisToken[ndx++] = c = TokenCharacter();
		    ThisToken[ndx] = 0;
		}
		stop = !isKeyword(ThisToken);
	    } else {
		stop = 1;
		ndx = 2;
		c = 0;
	    }
	}
	ThisToken[ndx - 1] = 0;
	PutBackChar(c);
	c = 0;
	result = isKeyword(ThisToken);
    }
    if ((!result) && c) {
	if (isFirstIDChar(c)) {
	    ndx = 0;
	    ThisToken[ndx++] = c;
	    while (isAnyIDChar(c = TokenCharacter())) {
		ThisToken[ndx++] = c;
	    }
	    ThisToken[ndx] = 0;
	    PutBackChar(c);
	    result = IDENTIFIER;
	    /*
	     * Now, we need to handle numeric constants (including floats,
	     * octal, hex, exponential, negative, etc...). The lexical
	     * parsing of numeric constants is very tricky now because of the
	     * expanded rules which ANSI specifies.  Constants may have a
	     * prefix sign, a suffix indicating its type, an exponent field
	     * (for float and double), and integers may be hex, octal or
	     * decimal.
	     */
#ifdef OOPECC
	} else if (c == '@') {
	    ndx = 0;
	    ThisToken[ndx++] = c;
	    while (isAnyIDChar(c = TokenCharacter())) {
		ThisToken[ndx++] = c;
	    }
	    ThisToken[ndx] = 0;
	    PutBackChar(c);
	    result = isKeyword(ThisToken);	/* We assume that this is an
						 * ObjC keyword, because it
						 * cannot be a typedef name
						 * of identifier (because it
						 * begins with a @) */
#endif
	} else if (isdigit(c)) {
	    int                             numdone;
	    int                             nonnum;
	    int                             exponya;
	    int                             isoctal;
	    int                             ishex;
	    int                             base;
	    long                            intresult;
	    long double                     floatresult;
	    int                             expon;
	    int                             exponsign;
	    int                             isintresult;
	    isintresult = 1;
	    exponsign = 0;
	    expon = 0;
	    intresult = 0;
	    floatresult = 0;
	    base = 10;
	    isoctal = 0;
	    ishex = 0;
	    exponya = 0;
	    numdone = 0;
	    nonnum = 0;
	    ndx = 0;

	    ThisToken[ndx++] = c;
	    if (isdigit(c)) {
		intresult = (c - '0');
	    }
	    /* We first check for octal. */
	    if (c == '0') {
		c2 = TokenCharacter();
		if (c2 == 'x') {
		    ishex = 1;
		    base = 16;
		    ThisToken[ndx++] = 'x';
		    while (ishexdigit(c = TokenCharacter())) {
			ThisToken[ndx++] = c;
			intresult = intresult * base + hexvalue(c);
		    }
		    PutBackChar(c);
		    nonnum = 1;
		} else if (c2 == '.') {
		    PutBackChar(c2);
		} else {
		    isoctal = 1;
		    base = 8;
		    PutBackChar(c2);
		    while (isoctaldigit(c = TokenCharacter())) {
			ThisToken[ndx++] = c;
			intresult = intresult * base + (c - '0');
		    }
		    PutBackChar(c);
		    nonnum = 1;
		}
	    }
	    /* Below, deals with floats and type suffices. */
	    while (!numdone) {
		c = TokenCharacter();
		if ((!nonnum) && isdigit(c)) {
		    ThisToken[ndx++] = c;
		    intresult = intresult * base + (c - '0');
		} else {
		    nonnum = 1;
		    switch (c) {
		    case '.':
		    case 'e':
		    case 'E':
			/* This is now a floating point constant. */
			if (ishex || isoctal) {
			    LexError("No floating hex or octal constants");
			    break;
			}
			floatresult = intresult;
			isintresult = 0;
			if (c != '.') {
			    ThisToken[ndx++] = c;
			} else {
			    double                          digitval;
			    ThisToken[ndx++] = c;
			    digitval = 1;
			    while (isdigit(c = TokenCharacter())) {
				ThisToken[ndx++] = c;
				digitval = digitval / base;
				floatresult = floatresult + (c - '0') * digitval;
			    }
			}
			switch (c) {
			    /* Exponential fields must be handled here. */
			case 'e':
			case 'E':
			    if (exponya) {
				LexError("FP constants may not have two exp fields");
			    } else {
				exponya = 1;
				ThisToken[ndx++] = 'E';
				c = TokenCharacter();
				if ((c == '+') || (c == '-')) {
				    ThisToken[ndx++] = c;
				    exponsign = -1;
				}
				while (isdigit(c = TokenCharacter())) {
				    ThisToken[ndx++] = c;
				    expon = expon * base + (c - '0');
				}
				PutBackChar(c);
				if (exponsign == -1) {
				    expon = -expon;
				}
				floatresult = floatresult * Eintpower(base, expon);
			    }
			    break;
			case 'f':
			case 'F':
			    ThisToken[ndx++] = 'F';
			    numdone = 1;
			    break;
			case 'l':
			case 'L':
			    ThisToken[ndx++] = 'L';
			    numdone = 1;
			    break;
			case '.':
			    if (exponya) {
				LexError("FP constants may not have decimal pts in exp fields");
			    } else {
				LexError("FP constants may not have 2 decimal points");
			    }
			    numdone = 1;
			    break;
			default:
			    numdone = 1;
			    PutBackChar(c);
			}
			break;
		    case 'u':
		    case 'U':
			ThisToken[ndx++] = 'U';
			c = TokenCharacter();
			if ((c == 'l') || (c == 'L')) {
			    ThisToken[ndx++] = 'L';
			} else {
			    PutBackChar(c);
			}
			numdone = 1;
			break;
		    case 'l':
		    case 'L':
			ThisToken[ndx++] = 'L';
			c = TokenCharacter();
			if ((c == 'u') || (c == 'U')) {
			    ThisToken[ndx++] = 'U';
			} else {
			    PutBackChar(c);
			}
			numdone = 1;
			break;
		    default:
			PutBackChar(c);
			numdone = 1;
		    }
		}
	    }
	    ThisToken[ndx] = 0;
	    if (isintresult) {
		LastIntegerConstant = intresult;
		result = INTCONSTANT;
	    } else {
		LastFloatingConstant = floatresult;
		result = FLOATCONSTANT;
	    }
	} else if (c == '\"') {
	    int                             ispascalstring = 0;
	    int                             skipchar = 0;
	    /*
	     * This will be a string literal.  Note that the handling of
	     * string literals is very tricky because of the special rules
	     * for what may appear inside. We used PreProcCharacter() to
	     * fetch characters within the string literal because spaces are
	     * significant. Also, this fragment handles escape constants.
	     */
	    ndx = 0;
	    while ((c = PreProcCharacter()) != '\"') {
		if (c == '\\') {
		    c = PreProcCharacter();
		    if ((ndx == 0) && (c == 'p')) {
			ispascalstring = 1;
			skipchar = 1;
		    } else
			c = EscapeConvert(c);
		}
		if (!skipchar)
		    ThisToken[ndx++] = c;
		skipchar = 0;
		if (ndx >= MAXIDLENGTH) {
		    LexError("String Literal too long");
		    break;
		}
	    }
	    ThisToken[ndx] = 0;
	    if (ispascalstring)
		result = PASCSTRING_LITERAL;
	    else
		result = STRING_LITERAL;
	} else if (c == '\'') {
	    /*
	     * This is a character constant.  We will allow multi-character
	     * constants, for the Macintosh.  The limit will probably be
	     * four.  Much of this fragment is identical to that above for
	     * string literals.
	     */
	    long                            intresult;
	    int                             countconstant;
	    countconstant = 0;
	    intresult = 0;
	    ndx = 0;
	    while ((c = PreProcCharacter()) != '\'') {
		if (c == '\\') {
		    c = PreProcCharacter();
		    c = EscapeConvert(c);
		}
		intresult = intresult * MAXUNSIGNEDCHAR + c;
		countconstant++;
		ThisToken[ndx++] = c;
	    }
	    ThisToken[ndx] = 0;
	    if (countconstant) {
		if (countconstant != 1) {
		    UserWarning(WARN_multicharconstant);
		}
	    } else {
		LexError("Empty character constant");
	    }
	    LastIntegerConstant = intresult;
	    result = CHARCONSTANT;
	} else {
	    /*
	     * Whatever character arrived here should not have done so.
	     */
	    FatalError("Illegal character for start of token");
	}
    }
    if (PartDone && !result)
	return -1;
    LastTokenKind = result;
    return result;
}

Codigo_t
GetToken()
{
    /*
     * This routine is the preprocessor interface to GetaToken. It simply has
     * the function of returning a valid C token to its caller, handling
     * macro expansion at the same time. It is this routine that is called by
     * the Parser.
     */
    register Codigo_t               tok;
    register Codigo_t               result;
    register PPSYMVia_t             def;
    /*
     * First, check the Parse Buffer for any old tokens lying around.
     */
    if (ParseBufferNdx) {
	LastToken = ParseBuffer[--ParseBufferNdx].token;
	LastTokenKind = ParseBuffer[ParseBufferNdx].tokval;
	LastFloatingConstant = ParseBuffer[ParseBufferNdx].floating;
	LastIntegerConstant = ParseBuffer[ParseBufferNdx].integer;
	result = LastTokenKind;
	return result;
    }
    /* QQQQ Is there a valid reason for not checking the PB here ? */
    result = 0;
    while (!result) {
	tok = GetaToken();
	if (gAllDone || (tok == -1))
	    return -1;
	/*
	 * PPStatus is true when tokens are to be ignored due to conditional
	 * compilation directives.
	 */
	while (PPStatus && (tok != -1)) {
	    tok = GetaToken();
	}
	if (gAllDone || (tok == -1))
	    return -1;
	if (tok == IDENTIFIER) {
	    /*
	     * Check the defined macro list to see if something needs to be
	     * expanded or substituted here
	     */
	    /*
	     * The following special macros are defined, according to ANSI:
	     * __LINE__ __FILE__ __DATE__ __TIME__ defined
	     */
	    if ((def = isDefined(ThisToken)) != 0) {
		register SymListVia_t           parmnames;
		char                            TempName[128];	/* MAXIDLENGTH */
		CurrentSRC.PreprocSubsts++;
		if (GetPPSymArgCount(def)) {
		    /*
		     * First we parse the arguments and place them into
		     * parmnames symbol list
		     */

		    int                             PrevCharCount;
		    PrevCharCount = CurrentSRC.CharCount;
		    tok = GetaToken();
		    if (tok != '(') {
			PreprocError2("Macro call without parenthesis ", ThisToken);
		    } else {
			int                             doneargs;
			int                             currentarg;
			register int                    charndx;
			char                            c;
			int                             parens;
			doneargs = 0;
			currentarg = 0;
			charndx = 0;
			parmnames = RawTable(11);
			parens = 0;
			while (!doneargs) {
			    c = TokenCharacter();
			    if (c == ',' && !parens) {
				TempName[charndx] = 0;
				TableAdd(parmnames, TempName);
				charndx = 0;
				currentarg++;
			    } else if (c == ')') {
				parens--;
				if (parens == -1) {
				    TempName[charndx] = 0;
				    TableAdd(parmnames, TempName);
				    doneargs = 1;
				} else {
				    TempName[charndx] = c;
				    charndx++;
				}
			    } else if (c == '(') {
				parens++;
				TempName[charndx] = c;
				charndx++;
			    } else {
				TempName[charndx] = c;
				charndx++;
			    }
			}
			/*
			 * Now we have all the arguments inserted into the
			 * parmnames symbol table.
			 */
			CurrentSRC.PreprocBefore += (CurrentSRC.CharCount - PrevCharCount);
			if (currentarg != (GetPPSymArgCount(def) - 1)) {
			    char                            nm[64];
			    GetPPSymName(def, nm);
			    PreprocError2("Wrong number of arguments to macro func ", nm);
			} else {
			    /*
			     * Now, we must substitute macro parameters in
			     * for the names of the parameters of the macro
			     * function in def. In the previous attmpt, we
			     * simply #defined all the values, in order to
			     * avoid having to parse the macro value field
			     * separately, but that turned out to be a real
			     * mess (NeedtoKill,KillArgs,isParam,infinite
			     * loops,duplicate entries in the macro list,
			     * addmacroparam, etc...) What we will do here is
			     * to create a string to hold this mess, and make
			     * that string the current SRC.  A single new
			     * field in CurrentSRC will allow the memory for
			     * the string to be freed upon completion.
			     */
			    EString_t                       Val;
			    int                             ndx;
			    EString_t                       theval;
			    int                             Valndx;
			    char                            ArgName[128];	/* MAXIDLENGTH */
			    int                             quotenext;
			    int                             eatspace;
			    int                             argndx;
			    int                             paramfound;
			    ndx = 0;
			    Val = Ealloc(MAXMACROLENGTH);
			    /* Build the string into Val here */
			    /*
			     * Begin searching
			     * Via(def)->Definition.PreProcValue for
			     * identifiers which are parameters of the macro
			     * function.  Whenever an identifier is found, if
			     * it is a parameter, replace it with the correct
			     * argument from parmnames.
			     */
			    doneargs = 0;
			    ndx = argndx = Valndx = quotenext = eatspace = 0;
			    theval = GetPPSymValue(def);
			    while (!doneargs) {
				c = Via(theval)[ndx++];
				/*
				 * According to ANSI, there are two special
				 * operators used in macro expansion.  The #
				 * operator, when it precedes a parameter,
				 * causes the argument for that parameter to
				 * be surrounded by quotes after expansion.
				 * The ## operator, consumes whitespace
				 * surrounding it, thus allowing the
				 * preprocessor to construct tokens by
				 * concatenation.
				 */
				if (c == '#') {
				    if (Via(theval)[ndx] != '#')
					quotenext = 1;
				    else {
					/*
					 * Here, we must eat white space to
					 * concatenate tokens...
					 */
					while (isspace(Via(Val)[Valndx - 1])) {
					    Valndx--;
					}
					eatspace = 1;
					ndx++;
				    }
				} else if (isFirstIDChar(c)) {
				    /*
				     * We have found the beginning of an
				     * identifier.
				     */
				    ArgName[argndx = 0] = c;
				    argndx++;
				    while (isFirstIDChar(Via(theval)[ndx++])) {
					ArgName[argndx++] = Via(theval)[ndx - 1];
				    }
				    ArgName[argndx] = 0;
				    ndx--;
				    /*
				     * Now, we have an identifier in ArgName,
				     * which may or may not be a parameter of
				     * the macro function.
				     */
				    paramfound = -1;
				    paramfound = PPSymSearchArgNum(def, ArgName);
				    if (!paramfound) {
					paramfound = -1;
				    }
				    Via(Val)[Valndx] = 0;
				    /*
				     * The parameter list for the macro
				     * function has been searched. paramfound
				     * holds the ndx of the parameter found
				     * or -1 if the search failed.
				     */
				    if (paramfound > -1) {
					int                             eatspot;
					/*
					 * Now, we substitute the argument
					 * for the parameter.
					 */
					char                           *tempptr = NULL;
					SYMVia_t                        searchres;
					searchres = TableGetNum(parmnames, paramfound);
					assert(searchres);
					tempptr = Via(searchres)->name;
					if (quotenext) {
					    Via(Val)[Valndx] = '\"';
					    Via(Val)[++Valndx] = 0;
					}
					eatspot = 0;
					if (eatspace) {
					    while (isspace((tempptr)[eatspot]))
						eatspot++;
					}
					strcat(Via(Val), &((tempptr)[eatspot]));
					Valndx += strlen((tempptr));
					if (quotenext) {
					    Via(Val)[Valndx] = '\"';
					    Via(Val)[++Valndx] = 0;
					    quotenext = 0;
					}
					eatspace = 0;
				    } else {
					/*
					 * In this case, the identifier we
					 * previously found was NOT a
					 * parameter.  We therefore pass it
					 * into the Val string unchanged.
					 */
					strcat(Via(Val), ArgName);
					Valndx += strlen(ArgName);
				    }
				} else {
				    /*
				     * This means that the character
				     * (obtained from the macro value tmplate
				     * Via(def)->Definition.PreProcValue) was
				     * not a FirstIDChar, nor was it a '#'
				     * (indicating a preprocessor expansion
				     * operator).
				     */
				    if (c) {
					if (!(eatspace && isspace(c)))
					    Via(Val)[Valndx++] = c;
				    } else {
					doneargs = 1;
					Via(Val)[Valndx] = 0;
				    }
				}
			    }

			    /*
			     * Now, the expanded string has been constructed
			     * in Val, and it will become the current SRC.
			     */
			    CurrentSRC.PreprocAfter += strlen(Via(Val));
			    PushSRC();
			    CurrentSRC.NeedtoKill = Val;
			    CurrentSRC.Macro = def;
			    CurrentSRC.isIO = 0;
			    CurrentSRC.where.mem = Val;

			    result = GetToken();
			}
		    }
		} else {
		    /*
		     * Substitution for defined symbols is accomplished by
		     * pointing the current SRC to a string in memory.
		     */
		    char                            nm[64];
		    GetPPSymName(def, nm);
		    CurrentSRC.PreprocBefore += strlen(nm);
		    if (GetPPSymValue(def)) {
			CurrentSRC.PreprocAfter += GetPPSymValueLength(def);
		    }
		    PushSRC();
		    CurrentSRC.isIO = 0;
		    CurrentSRC.NeedtoKill = NULL;
		    CurrentSRC.Macro = def;
		    CurrentSRC.where.mem = GetPPSymValue(def);
		    CurrentSRC.memindex = 0;
		    CurrentSRC.alreadyincluded = NULL;
		    CurrentSRC.NumExtras = 0;

		    result = GetToken();
		}
	    } else if (!strcmp(ThisToken, "__LINE__")) {
		sprintf(ThisToken, "%d", CurrentSRC.LineCount);
		result = INTCONSTANT;
		LastIntegerConstant = CurrentSRC.LineCount;
	    } else if (!strcmp(ThisToken, "__FILE__")) {
		strcpy(ThisToken, CurrentSRC.fname);
		result = STRING_LITERAL;
	    } else if (!strcmp(ThisToken, "__DATE__")) {
		time_t                          thetime;
		thetime = time(NULL);
		strftime(ThisToken,
			 MAXIDLENGTH, "%b %d %Y", localtime(&thetime));
		result = STRING_LITERAL;
	    } else if (!strcmp(ThisToken, "__TIME__")) {
		time_t                          thetime;
		thetime = time(NULL);
		strftime(ThisToken,
			 MAXIDLENGTH, "%H:%M:%S", localtime(&thetime));
		result = STRING_LITERAL;
	    } else {
		/*
		 * Now, determine if ThisToken contains a keyword,
		 * typedefname, or identifier
		 */
		result = isKeyword(ThisToken);
		if (result) {
		} else {
		    result = (int) isTypedefName(ThisToken) ? TYPEDEF_NAME : 0;
		    if (!result) {
			result = IDENTIFIER;
		    }
		}
	    }
	} else {
	    result = tok;
	}
    }
    /*
     * The fragment below handles the concatenation of adjacent string
     * literals by the preprocessor. QQQQ This routine fails to work
     * correctly if the string literal is the very last token in the file. Of
     * course, with legal C code being parsed, this should NEVER happen.
     */
    if ((result == STRING_LITERAL) || (result == PASCSTRING_LITERAL)) {
	int                             wasresult;
	int                             donestrings;
	char                            holdtoken[MAXIDLENGTH];
	wasresult = result;
	donestrings = 0;
	strcpy(holdtoken, ThisToken);
	while (!donestrings) {
	    result = GetToken();
	    if ((result == STRING_LITERAL) || (result ==
					       PASCSTRING_LITERAL)) {
		if ((strlen(holdtoken) + strlen(LastToken)) >= MAXIDLENGTH) {
		    LexError("String literal too long after concatenation");
		}
		strcat(holdtoken, LastToken);
	    } else {
		PutBackToken(LastToken, result);
		strcpy(ThisToken, holdtoken);
		result = wasresult;
		donestrings = 1;
	    }
	}
    }
    LastTokenKind = result;
    LastToken = ThisToken;
    return result;
}

int
NextIs(Codigo_t val)
/*
 * This routine checks the next token to see if it is of kind val, and
 * returns true or false.  If false, the token is put back.  If true, the
 * token is consumed.  NextIs and FetchToken are essentially the only
 * routines used to read tokens by the parser.
 */
{
    register Codigo_t               tmp;
    if (TokenOnDeck) {
	if (LastTokenKind == val) {
	    TokenOnDeck = 0;
	    return 1;
	} else {
	    return 0;
	}
    }
    if ((tmp = GetToken()) == val) {
	return 1;
    } else {
	if (tmp != -1) {
	    TokenOnDeck = 1;
	}
	return 0;
    }
}

long
FetchToken(void)
/*
 * This routine reads the next token, regardless of what it is.  NextIs and
 * FetchToken are essentially the only routines used to read tokens by the
 * parser.
 */
{
    register Codigo_t               tmp;
    if (TokenOnDeck) {
	TokenOnDeck = 0;
	return LastTokenKind;
    }
    tmp = GetToken();
    if (tmp != -1)
	return LastTokenKind;
    else
	return 0;
}

void
PutBackToken(char *name, Codigo_t tokval)
/* Add a token back to the parse buffer. */
{
    /*
     * Note that the values for constants were NOT taken from the function
     * parameters, therefore the following two lines may not be safe.  For
     * now, I'll try them as they are...
     */
    if (TokenOnDeck) {
	ParseBuffer[ParseBufferNdx].tokval = LastTokenKind;
	ParseBuffer[ParseBufferNdx].floating = LastFloatingConstant;
	ParseBuffer[ParseBufferNdx].integer = LastIntegerConstant;
	strcpy(ParseBuffer[ParseBufferNdx++].token, LastToken);
	TokenOnDeck = 0;
    }
    ParseBuffer[ParseBufferNdx].tokval = tokval;
    ParseBuffer[ParseBufferNdx].floating = LastFloatingConstant;
    ParseBuffer[ParseBufferNdx].integer = LastIntegerConstant;
    strcpy(ParseBuffer[ParseBufferNdx++].token, name);
}

void
UnFetchToken(void)
{
    PutBackToken(LastToken, LastTokenKind);
}
