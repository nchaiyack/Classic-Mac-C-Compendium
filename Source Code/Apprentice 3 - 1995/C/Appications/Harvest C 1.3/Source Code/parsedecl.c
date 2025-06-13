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
 * This file contains part of the Harvest C parser.  The parser is hand written,
 * recursive descent.  This file contains parser routines for declarations.
 * 
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "CHarvestDoc.h"
#include "CHarvestOptions.h"
#include "CErrorLog.h"

extern CErrorLog *gErrs;
extern CHarvestDoc *gProject;

#pragma segment ParseDecl


ParamRecVia_t
FindParam(char *name)
{
    ParamRecVia_t                   tmp;
    tmp = ParamList;
    while (tmp) {
	if (!strcmp(name, Via(tmp)->name))
	    return tmp;
	tmp = Via(tmp)->next;
    }
    return 0;
}

void
FreeSymImage(SymImageVia_t head)
{
    if (head) {
	FreeSymImage(Via(head)->next);
	Efree(head);
    }
}

int
Do_declaration(SymListVia_t table)
{
    /*
     * declaration : declaration_specifiers ';' | declaration_specifiers
     * init_declarator_list ';' ;
     */

    TypeRecordVia_t                 typerec;
    SymImageVia_t                   declared;
    SymImageVia_t                   curdeclared;
    int                             tmp;
    int                             regf, staticf, extf;
    enum StorageClassCode           stor_class;
    int junk;
    regf = staticf = extf = 0;
    tmp = 0;
    typerec = Do_declaration_specifiers(1, &stor_class, &junk);
    if (typerec) {
	if (stor_class == SCC_static) {
	    staticf = 1;
	}
	if (stor_class == SCC_extern) {
	    extf = 1;
	}
	if (stor_class == SCC_register) {
	    regf = 1;
	}
	if (NextIs(';')) {
	    /*
	     * This should have been a struct, union, enum or something like
	     * that. TODO Check this
	     */
	} else {
	    if (stor_class == SCC_typedef) {
		/* We should probably have local scope for typedef names. */
		declared = Do_init_declarator_list(TP_defnames, typerec, 0);
	    } else {
		declared = Do_init_declarator_list(table, typerec, stor_class);
	    }
	    if (declared) {
		/* The declaration has already been inserted into table */
		curdeclared = declared;
		if (regf) {
		    while (declared) {
			Via(Via(declared)->Symbol)->storage_class = SCC_register;
			declared = Via(declared)->next;
		    }
		} else if (staticf) {
		    while (declared) {
			Via(Via(declared)->Symbol)->storage_class = SCC_static;
			declared = Via(declared)->next;
		    }
		} else if (extf) {
		    while (declared) {
			Via(Via(declared)->Symbol)->storage_class = SCC_extern;
			declared = Via(declared)->next;
		    }
		} else {
		/* We need to check the symbols here, and mark them as extern
			if they are functions.  */
		    while (declared) {
		    	if (isFunctionType(Via(Via(declared)->Symbol)->TP)) {
				Via(Via(declared)->Symbol)->storage_class = SCC_extern;
			}
			declared = Via(declared)->next;
		    }
		}
		tmp = 1;
		FreeSymImage(curdeclared);
	    } else {
		SyntaxError("Expecting declarator");
	    }
	    if (!NextIs(';')) {
		SyntaxError("Missing semicolon");
	    }
	}
    }
    return tmp;
}

TypeRecordVia_t
Do_declaration_specifiers(int sc_legal, enum StorageClassCode * stor_class,
	 int *pascalFlag)
{
    /*
     * declaration_specifiers : storage_class_specifier |
     * storage_class_specifier declaration_specifiers | type_specifier |
     * type_specifier declaration_specifiers | type_qualifier |
     * type_qualifier declaration_specifiers ;
     */

    TypeRecordVia_t                 result;
    int                             done;
    int                             sc_count;
    int                             qu_count;
    SYMVia_t                        name;
    enum StorageClassCode           typequal;
    short                           isinline, ispascal;
    *stor_class = 0;
    done = 0;
    typequal = 0;
    isinline = 0;
    ispascal = 0;
    sc_count = 0;
    qu_count = 0;

    result = 0;
    while (!done) {
	FetchToken();
	switch (LastTokenKind) {
	case AUTO:
	    /* This is valid only within in a function. */
	    if (!sc_legal) {
		SyntaxError("Storage class specifier not allowed here");
	    }
	    sc_count++;
	    if (sc_count >= 2) {
		SyntaxError("Too many storage class specifiers");
	    } else {
		if (!FunctionBeingDefined) {
		    SyntaxError("auto may not appear outside of a function");
		}
		*stor_class = SCC_auto;
	    }

	    break;
	case VOID:
	    /* Done.  Nothing else is legal after this. */
	    result = BuildTypeRecord(0, TRC_void, SGN_unknown);
	    done = 1;
	    break;
	case CHAR:
	    /* Done.  Nothing else is legal after this. */
	    if (gProject->itsOptions->signedChars) {
		result = BuildTypeRecord(0, TRC_char, SGN_signed);
	    } else {
		result = BuildTypeRecord(0, TRC_char, SGN_unsigned);
	    }
	    done = 1;
	    break;
	case INT:
	    /* Done.  Nothing else is legal after this. */
	    result = BuildTypeRecord(0, TRC_int, SGN_signed);
	    done = 1;
	    break;
	case STRUCT:
	    result = Do_struct_or_union_specifier(STRUCT);
	    done = 1;
	    break;
	case TYPEDEF_NAME:
	    name = TableSearch(TP_defnames, (LastToken));
	    result = GetSymTP(name);
	    /*
	     * This means that the type returned is actually some other type,
	     * through a typedef.  In reality, this makes little difference,
	     * but was done to have more complete information.
	     */
	    SetTPFlags(result, GetTPFlags(result) | SIZEDONE);
#ifdef Undefined
	    SetTypeSize(result);
#endif
	    break;
	case EXTERN:
	    if (!sc_legal) {
		SyntaxError("Storage class specifier not allowed here");
	    }
	    sc_count++;
	    if (sc_count >= 2) {
		SyntaxError("Too many storage class specifiers");
	    } else {
		*stor_class = SCC_extern;
	    } break;
	case REGISTER:
	    if (!sc_legal) {
		SyntaxError("Storage class specifier not allowed here");
	    }
	    /* This is valid only within in a function. */
	    sc_count++;
	    if (sc_count >= 2) {
		SyntaxError("Too many storage class specifiers");
	    } else {
		if (!FunctionBeingDefined) {
		    SyntaxError("register may not appear outside of a function");
		}
		*stor_class = SCC_register;
	    } break;
	case STATIC:
	    if (!sc_legal) {
		SyntaxError("Storage class specifier not allowed here");
	    }
	    sc_count++;
	    if (sc_count >= 2) {
		SyntaxError("Too many storage class specifiers");
	    } else {
		*stor_class = SCC_static;
	    } break;
	case PASCAL:
	    UserWarning(WARN_pascal);
	    ispascal = 1;
	    break;
	case TYPEDEF:
	    if (!sc_legal) {
		SyntaxError("typedef not allowed here");
	    }
	    sc_count++;
	    if (sc_count >= 2) {
		SyntaxError("Too many storage class specifiers");
	    } else {
		/* Special case... */
		*stor_class = SCC_typedef;
	    } break;
	case SHORT:
	    /*
	     * After this, only signed, unsigned, or int are legal, but none
	     * are required.
	     */
	    if (NextIs(SIGNED)) {
		NextIs(INT);
		result = BuildTypeRecord(0, TRC_short, SGN_signed);
		done = 1;
	    } else if (NextIs(UNSIGNED)) {
		NextIs(INT);
		result = BuildTypeRecord(0, TRC_short, SGN_unsigned);
		done = 1;
	    } else if (NextIs(INT)) {
		result = BuildTypeRecord(0, TRC_short, SGN_signed);
		done = 1;
	    } else {
		result = BuildTypeRecord(0, TRC_short, SGN_signed);
		done = 1;
	    } break;
	case COMP:
	    /* Warning - non-ANSI keyword */
	    result = BuildTypeRecord(0, TRC_comp, SGN_signed);
	    done = 1;
	    break;
	case EXTENDED:
	    /* Warning - non-ANSI keyword */
	    result = BuildTypeRecord(0, TRC_longdouble, SGN_unknown);
	    done = 1;
	    break;
	case LONG:
	    /*
	     * After this, only signed, unsigned, double, or int are legal,
	     * but none are required.  Of these four, double may only appear
	     * if it is alone.
	     */
	    if (NextIs(DOUBLE)) {
		result = BuildTypeRecord(0, TRC_longdouble, SGN_unknown);
		done = 1;
	    } else if (NextIs(SIGNED)) {
		NextIs(INT);
		result = BuildTypeRecord(0, TRC_long, SGN_signed);
		done = 1;
	    } else if (NextIs(UNSIGNED)) {
		NextIs(INT);
		result = BuildTypeRecord(0, TRC_long, SGN_unsigned);
		done = 1;
	    } else if (NextIs(INT)) {
		result = BuildTypeRecord(0, TRC_long, SGN_signed);
		done = 1;
	    } else {
		result = BuildTypeRecord(0, TRC_long, SGN_signed);
		done = 1;
	    } break;
	case FLOAT:
	    /* Done.  Nothing else is legal after this. */
	    result = BuildTypeRecord(0, TRC_float, SGN_unknown);
	    done = 1;
	    break;
	case DOUBLE:
	    /* Done.  Nothing else is legal after this. */
	    result = BuildTypeRecord(0, TRC_double, SGN_unknown);
	    done = 1;
	    break;
	case SIGNED:
	    /*
	     * After this, only char, short, long, or int are legal, but none
	     * are required.
	     */
	    if (NextIs(SHORT)) {
		NextIs(INT);
		result = BuildTypeRecord(0, TRC_short, SGN_signed);
		done = 1;
	    } else if (NextIs(CHAR)) {
		result = BuildTypeRecord(0, TRC_char, SGN_signed);
		done = 1;
	    } else if (NextIs(LONG)) {
		NextIs(INT);
		result = BuildTypeRecord(0, TRC_long, SGN_signed);
		done = 1;
	    } else if (NextIs(INT)) {
		result = BuildTypeRecord(0, TRC_int, SGN_signed);
		done = 1;
	    } else {
		result = BuildTypeRecord(0, TRC_int, SGN_signed);
		done = 1;
	    } break;
	case UNSIGNED:
	    /*
	     * After this, only short, long, char, or int are legal, but none
	     * are required.
	     */
	    if (NextIs(SHORT)) {
		NextIs(INT);
		result = BuildTypeRecord(0, TRC_short, SGN_unsigned);
		done = 1;
	    } else if (NextIs(CHAR)) {
		result = BuildTypeRecord(0, TRC_char, SGN_unsigned);
		done = 1;
	    } else if (NextIs(LONG)) {
		NextIs(INT);
		result = BuildTypeRecord(0, TRC_long, SGN_unsigned);
		done = 1;
	    } else if (NextIs(INT)) {
		result = BuildTypeRecord(0, TRC_int, SGN_unsigned);
		done = 1;
	    } else {
		result = BuildTypeRecord(0, TRC_int, SGN_unsigned);
		done = 1;
	    } break;
	case CONST:
	    qu_count++;
	    if (qu_count >= 2) {
		SyntaxError("Too many type qualifiers");
	    } else {
		typequal = SCC_const;
	    } break;
	case VOLATILE:
	    qu_count++;
	    if (qu_count >= 2) {
		SyntaxError("Too many type qualifiers");
	    } else {
		UserWarning(WARN_novolatile);
		typequal = SCC_volatile;
	    } break;
	case UNION:
	    result = Do_struct_or_union_specifier(UNION);
	    done = 1;
	    break;
	case ENUM:
	    result = Do_enum_specifier();
	    done = 1;
	    break;
	default:
	    UnFetchToken();
	    done = 1;
	}
    }
    if (typequal) {
	TypeRecordVia_t                 other;
	other = CopyTypeRecord(result);
	SetTPQual(other, typequal);
	result = other;
    }
    if (isinline) {
	TypeRecordVia_t                 other;
	other = CopyTypeRecord(result);
	SetTPFlags(other, GetTPFlags(other) | ISINLINEMASK);
	result = other;
    }
    if (ispascal) {
	TypeRecordVia_t                 other;
	other = CopyTypeRecord(result);
	SetTPFlags(other, GetTPFlags(other) | ISPASCALMASK);
	result = other;
    }
    *pascalFlag = ispascal;
    return result;
}

SymImageVia_t
Do_init_declarator_list(SymListVia_t table, TypeRecordVia_t typerec,
			enum StorageClassCode stor_class)
{
    /*
     * init_declarator_list : init_declarator | init_declarator_list ','
     * init_declarator ;
     */

    SYMVia_t                        result;
    SYMVia_t                        others;
    int                             donelisting;
    SymImageVia_t                   list;
    list = NULL;
    result = Do_init_declarator(table, typerec, stor_class);
    if (result) {
	list = ListAdd(list, result);
	donelisting = 0;
	while (!donelisting) {
	    if (NextIs(',')) {
		others = Do_init_declarator(table, typerec, stor_class);
		/* The symbol has been inserted into table. */
		if (others) {
		    list = ListAdd(list, others);
		} else {
		    SyntaxError("Expecting declarator");
		}
	    } else {
		donelisting = 1;
	    }
	}
    }
    return list;
}

SYMVia_t
Do_init_declarator(SymListVia_t table, TypeRecordVia_t typerec,
		   enum StorageClassCode stor_class)
{
    /*
     * init_declarator : declarator | declarator '=' initializer ;
     */

    SYMVia_t                        SymbolRec;
    TypeRecordVia_t                 newtype;
    SymbolRec = Do_declarator(table, typerec, &newtype);
    if (SymbolRec) {
	Via(SymbolRec)->storage_class = stor_class;
	if (NextIs('=')) {
	    Via(SymbolRec)->Definition.Initializer = Do_initializer();
	    if (!Via(SymbolRec)->Definition.Initializer) {
		SyntaxError("Expecting initializer");
	    }
	} else {
	    if (isArrayType(GetSymTP(SymbolRec))) {
		if (isIncompleteType(GetTPBase(GetSymTP(SymbolRec)))) {
		    DeclError("Array of incomplete type");
		}
	    }
	}
    }
    return SymbolRec;
}

TypeRecordVia_t
Do_struct_or_union_specifier(Codigo_t keyword_code)
{
    /*
     * struct_or_union_specifier : struct_or_union IDENTIFIER '{'
     * struct_declaration_list '}' | struct_or_union '{'
     * struct_declaration_list '}' | struct_or_union IDENTIFIER ;
     */

    char                            TagName[200];
    int                             tmp;
    TypeRecordVia_t                 result;
    SYMVia_t                        tagsym;
    int                             prevNextStructOffset;
    int                             prevCurFieldOffset;
    int                             prevCurStructSize;
    int                             prevCurFieldBit;
    result = 0;
    prevNextStructOffset = NextStructOffset;
    prevCurFieldOffset = CurFieldOffset;
    prevCurStructSize = CurStructSize;
    prevCurFieldBit = CurFieldBit;
    NextStructOffset = 0;
    CurStructSize = 0;
    CurFieldOffset = -1;
    CurFieldBit = 0;

    if (keyword_code) {
	if (GetIDENTIFIER()) {
	    strcpy(TagName, LastToken);
	    if (NextIs('{')) {
		tagsym = LookUpTag(TagName);
		if (tagsym) {
		    result = GetSymTP(tagsym);
		} else {
		    if (keyword_code == STRUCT) {
			result = BuildTagTypeRecord(0, TRC_struct, TagName);
		    } else {
			result = BuildTagTypeRecord(0, TRC_union, TagName);
		    }
		    tagsym = TableAdd(CurrentTags(), TagName);
		    SetSymTP(tagsym, result);
		}
		if (GetTPMembers(result)) {
		    DeclError2("Redeclaration of struct tag ", TagName);
		} else {
		    SetTPMembers(result, RawTable(11));
		}
		tmp = Do_struct_declaration_list(GetTPMembers(result));
		if (!tmp) {
		    SyntaxError("Expecting struct declaration list");
		}
		if (CurStructSize % 2) {
		    CurStructSize++;
		}
		if (CurStructSize) {
		    SetTPSize(result, CurStructSize);
		} else {
		    DeclError("No zero sized structs allowed");
		}
		if (keyword_code != STRUCT) {
		    SetTPSize(result, MaxSizes(GetTPMembers(result)));
		}
		if (!NextIs('}')) {
		    SyntaxError("Missing right brace");
		}
	    } else {
		if (NextIs(';')) {
		    PutBackToken(";", ';');
		    if (keyword_code == STRUCT) {
			result = BuildTagTypeRecord(0, TRC_struct, TagName);
		    } else {
			result = BuildTagTypeRecord(0, TRC_union, TagName);
		    }
		    tagsym = TableAdd(CurrentTags(), TagName);
		    SetSymTP(tagsym, result);
		} else {
		    tagsym = LookUpTag(TagName);
		    if (tagsym) {
			result = GetSymTP(tagsym);
		    } else {
			if (keyword_code == STRUCT) {
			    result = BuildTagTypeRecord(0, TRC_struct, TagName);
			} else {
			    result = BuildTagTypeRecord(0, TRC_union, TagName);
			}
			tagsym = TableAdd(CurrentTags(), TagName);
			SetSymTP(tagsym, result);
		    }
		}
	    }
	} else if (NextIs('{')) {
	    if (keyword_code == STRUCT) {
		result = BuildTagTypeRecord(0, TRC_struct, NULL);
	    } else {
		result = BuildTagTypeRecord(0, TRC_union, NULL);
	    }
	    SetTPMembers(result, RawTable(11));
	    tmp = Do_struct_declaration_list(GetTPMembers(result));
	    if (CurStructSize % 2) {
		CurStructSize++;
	    }
	    if (CurStructSize) {
		SetTPSize(result, CurStructSize);
	    } else {
		DeclError("No zero sized structs allowed");
	    }
	    if (!tmp) {
		/*
		 * This is an error because empty structs are not allowed,
		 * are they ?
		 */
		SyntaxError("Expecting struct declaration list");
	    }
	    if (keyword_code != STRUCT) {
		SetTPSize(result, MaxSizes(GetTPMembers(result)));
	    }
	    if (!NextIs('}')) {
		SyntaxError("Missing right brace");
	    }
	} else {
	    SyntaxError("Expecting either tag or left brace");
	}
    } else {
	result = 0;
    }
    NextStructOffset = prevNextStructOffset;
    CurFieldOffset = prevCurFieldOffset;
    CurStructSize = prevCurStructSize;
    CurFieldBit = prevCurFieldBit;
    return result;
}

int
Do_struct_declaration_list(SymListVia_t table)
{
    /*
     * struct_declaration_list : struct_declaration | struct_declaration_list
     * struct_declaration ;
     */

    int                             notdone;
    int                             one;
    one = 0;
    notdone = 1;
    while (notdone) {
	notdone = Do_struct_declaration(table);
	if (notdone)
	    one++;
	else {
	    notdone = NextIs(atPUBLIC);
	    if (notdone) {
		/* Handle this TODO */
	    }
	}
    }
    return one;
}

int
Do_struct_declaration(SymListVia_t table)
{
    /*
     * struct_declaration : specifier_qualifier_list struct_declarator_list
     * ';' ;
     */

    int                             tmp;
    enum StorageClassCode           nope;
    TypeRecordVia_t                 typerec;
    int junk;
    tmp = 0;
    typerec = Do_declaration_specifiers(0, &nope,&junk);	/* The 0 indicates that
							 * storage class
							 * specifiers are not
							 * legal, thus making
							 * this a specifier
							 * qualifer list. */
    if (typerec) {
	tmp = Do_struct_declarator_list(table, typerec);
	if (!tmp) {
	    SyntaxError("Expecting declarator");
	}
	if (!NextIs(';')) {
	    SyntaxError("Missing semicolon");
	}
    }
    return tmp;
}

int
Do_struct_declarator_list(SymListVia_t table, TypeRecordVia_t typerec)
{
    /*
     * struct_declarator_list : struct_declarator | struct_declarator_list
     * ',' struct_declarator ;
     */

    int                             result;
    int                             donedecl;
    result = Do_struct_declarator(table, typerec);
    if (result) {
	donedecl = 0;
	while (!donedecl) {
	    if (NextIs(',')) {
		result = Do_struct_declarator(table, typerec);
		if (!result) {
		    SyntaxError("Expecting declarator");
		}
	    } else {
		donedecl = 1;
	    }
	}
    }
    return result;
}

int
Do_struct_declarator(SymListVia_t table, TypeRecordVia_t typerec)
{
    /*
     * struct_declarator : declarator | ':' constant_expr | declarator ':'
     * constant_expr ;
     */
    /*
     * The colons above are used to specify bit fields.
     */

    SYMVia_t                        result;
    ParseTreeVia_t                  tmp;
    int                             funcres;
    funcres = 0;
    if (NextIs(':')) {
	tmp = Do_constant_expr();
	if (tmp) {
	    int                             constvalue;
	    constvalue = GetIntValue(tmp);
	    FreeTree(tmp);
	    funcres = 1;
	    if (constvalue) {
		CurFieldBit += constvalue;
		while (CurFieldBit >= BITFIELDUNIT) {
		    CurFieldBit -= BITFIELDUNIT;
		    NextStructOffset += BITFIELDUNIT / 8;
		}
	    } else {
		CurFieldBit = 0;
		NextStructOffset += BITFIELDUNIT / 8;
	    }
	} else {
	    SyntaxError("Expecting constant expression for bit field width");
	}
    } else {
	TypeRecordVia_t                 newtype;
	result = Do_declarator(table, typerec, &newtype);
	if (result) {
	    funcres = 1;
	    if (NextIs(':')) {
		tmp = Do_constant_expr();
		if (tmp) {
		    int                             constvalue;
		    constvalue = GetIntValue(tmp);
		    FreeTree(tmp);
		    funcres = 1;
		    if (constvalue) {
			/* Modify result to represent a bit field. */
			int                             bitstart, bitend;
			if (constvalue > BITFIELDUNIT) {
			    DeclError("A field cannot be that wide.");
			}
			if (CurFieldOffset != -1) {
			    /*
			     * Here, we are already in the middle of a field.
			     * We may not need to increase the struct size.
			     */
			    if (CurFieldBit + constvalue >= BITFIELDUNIT) {
				CurFieldOffset = NextStructOffset;
				NextStructOffset += BITFIELDUNIT / 8;
				bitstart = 0;
				bitend = constvalue - 1;
				CurStructSize += BITFIELDUNIT / 8;
				CurFieldBit = constvalue;
			    } else {
				bitstart = CurFieldBit;
				CurFieldBit += constvalue;
				bitend = CurFieldBit - 1;
			    }
			} else {
			    CurFieldOffset = NextStructOffset;
			    NextStructOffset += BITFIELDUNIT / 8;
			    bitstart = 0;
			    bitend = constvalue - 1;
			    CurStructSize += BITFIELDUNIT / 8;
			    CurFieldBit = constvalue;
			}
			Via(result)->Definition.StartEndBits.StartBit = bitstart;
			Via(result)->Definition.StartEndBits.EndBit = bitend;
			SetSymTP(result, BuildTypeRecord(Via(result)->TP, TRC_bitfield, GetTPSign(Via(result)->TP)));
			Via(result)->numbers.structoffset = CurFieldOffset;
		    } else {
			/* Should we issue a warning ? */
			CurFieldBit = 0;
			NextStructOffset += BITFIELDUNIT / 8;
			Via(result)->TP = BuildTypeRecord(Via(result)->TP, TRC_bitfield, GetTPSign(Via(result)->TP));
			Via(result)->numbers.structoffset = CurFieldOffset;
			Via(result)->Definition.StartEndBits.StartBit = 0;
			Via(result)->Definition.StartEndBits.EndBit = 0;
		    }
		} else {
		    SyntaxError("Expecting constant expression for bit field width");
		}
	    } else {
		/*
		 * Here we calculate the offset of the member, and update the
		 * struct size.
		 */
		CurFieldBit = 0;
		CurFieldOffset = -1;
		if (isIncompleteType(Via(result)->TP)) {
		    DeclError("Struct member with incomplete type");
		}
		if (isCharType(Via(result)->TP)) {
		    Via(result)->numbers.structoffset = NextStructOffset++;
		    CurStructSize++;
		} else {
		    if (NextStructOffset % 2) {
			NextStructOffset++;
			CurStructSize++;
		    }
		    Via(result)->numbers.structoffset = NextStructOffset;
		    NextStructOffset += GetTPSize(Via(result)->TP);
		    CurStructSize += GetTPSize(Via(result)->TP);
		}
	    }
	}
    }
    return funcres;
}

TypeRecordVia_t
Do_enum_specifier(void)
{
    /*
     * enum_specifier : ENUM '{' enumerator_list '}' | ENUM IDENTIFIER '{'
     * enumerator_list '}' | ENUM IDENTIFIER ;
     */

    TypeRecordVia_t                 result;
    SYMVia_t                        tagsym;
    int                             tmp;
    char                            holdid[64];
    int                             wasid = 0;
    wasid = 0;
    result = 0;
    tagsym = 0;
    if (NextIs(IDENTIFIER)) {
	strcpy(holdid, LastToken);
	wasid = 1;
	tagsym = LookUpTag(holdid);
	if (tagsym) {
	    result = Via(tagsym)->TP;
	} else {
	    result = BuildTagTypeRecord(0, TRC_enum, holdid);
	    SetTPMembers(result, RawTable(53));
	    EnumBeingDeclared = result;
	    if (gProject->itsOptions->int2byte)
		SetTPSize(EnumBeingDeclared, 2);
	    else
		SetTPSize(EnumBeingDeclared, 4);
	    CurrentEnumK = 0;
	    tagsym = TableAdd(CurrentTags(), holdid);
	    Via(tagsym)->TP = result;
	}
    } else {
	result = BuildTagTypeRecord(0, TRC_enum, holdid);
	SetTPMembers(result, RawTable(53));
	EnumBeingDeclared = result;
	if (gProject->itsOptions->int2byte)
	    SetTPSize(EnumBeingDeclared, 2);
	else
	    SetTPSize(EnumBeingDeclared, 4);
	CurrentEnumK = 0;
    }
    if (NextIs('{')) {
	PushEnumTable(GetTPMembers(result));
	tmp = Do_enumerator_list();
	if (!tmp) {
	    SyntaxError("Expecting enumerator list");
	}
	if (!NextIs('}')) {
	    SyntaxError("Missing right brace");
	}
    } else {
	if (!wasid) {
	    SyntaxError("Expected either tag or left brace");
	}
    }
    return result;
}

int
Do_enumerator_list(void)
{
    /*
     * enumerator_list : enumerator | enumerator_list ',' enumerator ;
     */

    int                             result;
    int                             doneenumerating;
    result = Do_enumerator();
    if (result) {
	doneenumerating = 0;
	while (!doneenumerating) {
	    if (NextIs(',')) {
		result = Do_enumerator();
		if (!result) {
		    /*
		     * Note that this fragment allows an enumerator list to
		     * end in a comma.  I don't know if this should be this
		     * way or not.
		     */
		    doneenumerating = 1;
		    result = 1;
		}
	    } else {
		doneenumerating = 1;
	    }
	}
    }
    return result;
}

int
Do_enumerator(void)
{
    /*
     * enumerator : IDENTIFIER | IDENTIFIER '=' constant_expr ;
     */

    int                             result;
    ParseTreeVia_t                  exprtree;
    SYMVia_t                        enumerator;
    enumerator = 0;
    if (NextIs(IDENTIFIER)) {
	if ((enumerator = LookUpSymbol(LastToken)) != 0) {
	    DeclErrorSYM("enum constant has previous definition", enumerator);
	}
	enumerator = TableAdd(CurrentEnums(), (LastToken));
	Via(enumerator)->TP = EnumBeingDeclared;
	Via(enumerator)->storage_class = SCC_enum;
	Via(enumerator)->numbers.EnumVal = CurrentEnumK++;
	if (!EnumBeingDeclared) {
	    VeryBadParseError("EnumBeingDeclared == NULL in Do_enumerator");
	}
	result = 1;
	if (NextIs('=')) {
	    int                             constvalue;
	    constvalue = 1;
	    exprtree = Do_constant_expr();
	    if (!exprtree) {
		SyntaxError("Expected constant expression for enum initializer");
		result = 0;
	    } else {
		constvalue = GetIntValue(exprtree);
		FreeTree(exprtree);
		Via(enumerator)->numbers.EnumVal = CurrentEnumK = constvalue;
		CurrentEnumK++;
	    }
	}
    } else {
	result = 0;
    }
    return result;
}

SYMVia_t
Do_declarator(SymListVia_t table, TypeRecordVia_t typerec, TypeRecordVia_t * newtype)
{
    /*
     * declarator : direct_declarator | pointer direct_declarator ;
     */

    TypeRecordVia_t                 result;
    SYMVia_t                        tmp;
    result = Do_pointer(typerec);
    if (newtype)
	*newtype = result;
    tmp = Do_direct_declarator(table, result);
    return tmp;
}

void
RecurseSetSize(TypeRecordVia_t TP)
{
    if (TP) {
	if (GetTPFlags(TP) & SIZEDONE)
	    return;
	if (GetTPKind(TP) == TRC_array) {
	    if (isArrayType(GetTPBase(TP))) {
		RecurseSetSize(GetTPBase(TP));
	    }
	    SetTPSize(TP, GetTPSize(TP) * GetTPSize(GetTPBase(TP)));
	    SetTPFlags(TP, GetTPFlags(TP) | SIZEDONE);
	} else if (GetTPKind(TP) == TRC_typedef) {
	    RecurseSetSize(GetTPBase(TP));
	    SetTPSize(TP, GetTPSize(GetTPBase(TP)));
	    SetTPFlags(TP, GetTPFlags(TP) | SIZEDONE);
	}
    }
}

void
SetArraySizes(TypeRecordVia_t TP)
{
    if (GetTPKind(TP) == TRC_typedef) {
	SetArraySizes(GetTPBase(TP));
	SetTPSize(TP, GetTPSize(GetTPBase(TP)));
    } else {
	assert(GetTPKind(TP) == TRC_array);
	if (isArrayType(GetTPBase(TP))) {
	    int                             stack[MAXARRAYDIMS];
	    int                             sp;
	    TypeRecordVia_t                 tp2;
	    sp = 0;
	    tp2 = TP;

	    while (isArrayType(tp2)) {
		if (GetTPFlags(tp2) & SIZEDONE)
		    break;
		if (GetTPKind(tp2) == TRC_array) {
		    stack[sp++] = GetTPSize(tp2);
		}
		tp2 = GetTPBase(tp2);
	    }
	    tp2 = TP;

	    while (isArrayType(tp2)) {
		if (GetTPFlags(tp2) & SIZEDONE)
		    break;
		if (GetTPKind(tp2) == TRC_array) {
		    SetTPSize(tp2, stack[--sp]);
		}
		tp2 = GetTPBase(tp2);
	    }
	    RecurseSetSize(TP);
	} else {
	    if (GetTPFlags(TP) & SIZEDONE)
		return;
	    SetTPSize(TP, GetTPSize(TP) * GetTPSize(GetTPBase(TP)));
	    SetTPFlags(TP, GetTPFlags(TP) | SIZEDONE);
	}
    }
}

SYMVia_t                        FirstSym(SymListVia_t tbl);

SYMVia_t
Do_direct_declarator(SymListVia_t table, TypeRecordVia_t typerec)
{
    /*
     * direct_declarator : IDENTIFIER | '(' declarator ')' |
     * direct_declarator '[' ']' | direct_declarator '[' constant_expr ']' |
     * direct_declarator '(' ')' | direct_declarator '(' parameter_type_list
     * ')' | direct_declarator '(' identifier_list ')' ;
     */

    SYMVia_t                        result;
    int                             donedirecting;
    TypeRecordVia_t                 already;
    int                             OldNoArgs;
    int                             redefTypedef;
    TypeRecordVia_t                 newtype = 0;
    TypeRecordVia_t                 oldtype = 0;
    OldNoArgs = 0;
    result = 0;
    already = 0;
    if ((redefTypedef = GetIDENTIFIER()) != 0) {
	/*
	 * We add id to table now, keeping the result for later changes.
	 */
	/*
	 * Check here to see if the token already exists.  If so, check that
	 * the types are the same.
	 */
	CurrentSRC.CountIdentifiers++;
	CurrentSRC.TotalIdentifierLength += strlen(LastToken);
	if (redefTypedef == 2) {
	    if (!FunctionBeingDefined) {
		DeclError("redefinition of typedef names at global scope not allowed");
	    }
	}
	result = TableSearch(table, (LastToken));
	if (result) {
	    already = Via(result)->TP;
	    Via(result)->TP = typerec;
	} else {
	    result = TableAdd(table, (LastToken));
	    Via(result)->storage_class = SCC_auto;	/* QQQQ WHy ? */
	    /*
	     * QQQQ Here, is table == Via(Via(CurrentBlock)->block)->Symbols
	     * ?
	     */
	    Via(result)->TP = typerec;
	}
    } else if (NextIs('(')) {
	result = Do_declarator(table, 0, &newtype);
	newtype = Via(result)->TP;
	if (result) {
	    newtype = Via(result)->TP;
	    Via(result)->TP = oldtype = typerec;
	    if (!NextIs(')')) {
		SyntaxError("Missing right parenthesis");
	    }
	} else {
	    SyntaxError("Expected declarator");
	}
    } else {
	result = 0;
    }

    if (result) {
	donedirecting = 0;
	while (!donedirecting) {
	    if (NextIs('[')) {
		if (NextIs(']')) {
		    Via(result)->TP = BuildTypeRecord(Via(result)->TP, TRC_array, SGN_unknown);
		    SetTPFlags(Via(result)->TP, GetTPFlags(Via(result)->TP) | INCOMPLETEMASK);
		    SetTPFlags(Via(result)->TP, GetTPFlags(Via(result)->TP) & (!SIZEDONE));
		    SetTPSize(Via(result)->TP, 0);
		    /*
		     * Generally, when a typerecord is an array, the typesize
		     * field indicates the size of the array in bytes.
		     */
		    /*
		     * The type of an object of incomplete type is completed
		     * by another, complete, declaration for the object
		     * (A10.2), or by initializing it (A8.7). - K&R ][,
		     * A8.6.2
		     */
		} else {
		    ParseTreeVia_t                  tmp;
		    int                             constantvalue;
		    tmp = Do_constant_expr();
		    if (tmp) {
			constantvalue = GetIntValue(tmp);
			FreeTree(tmp);
			Via(result)->TP = BuildTypeRecord(Via(result)->TP, TRC_array, SGN_unknown);
			SetTPFlags(Via(result)->TP, GetTPFlags(Via(result)->TP) & (!SIZEDONE));
			SetTPSize(Via(result)->TP, constantvalue);
			/* This will be fixed later... */
			if (!NextIs(']')) {
			    SyntaxError("Expected right bracket");
			}
		    } else {
			SyntaxError("Expected constant expr for array size");
		    }
		}
	    } else if (NextIs('(')) {
		if (NextIs(')')) {
		    if (gProject->itsOptions->requireProtos) {
			TypeError("Prototypes required");
		    }
		    Via(result)->TP = BuildTypeRecord(Via(result)->TP, TRC_OLDfunction, SGN_unknown);
		    SetTPMembers(Via(result)->TP, RawTable(11));
		    OldNoArgs = 1;
		    /*
		     * ALL functions are either ANSI or not.  If ANSI, then
		     * args are type checked.  If not, args are not type
		     * checked, but promoted according to specific rules.
		     * NOARGS is simply a special case of ANSI, and
		     * INCOMPLETE is treated as old style.
		     */
		} else {
		    int                             listresult;
		    SymListVia_t                    templist, tagslist,
		                                    lablist;
		    templist = RawTable(11);
		    tagslist = RawTable(11);
		    lablist = RawTable(11);
		    PushSpaces(templist, tagslist, lablist);
		    listresult = Do_parameter_type_list(templist);	/* A proto */
		    PopBlock();
		    if (Via(tagslist)->count) {
			UserMesgCR("Tag(s) in proto being lost");
		    }
		    FreeSymbolList(tagslist, 0);
		    FreeSymbolList(lablist, 0);
		    /*
		     * Perhaps we should check
		     * Via(Via(CurrentBlock)->block)->Tags to see if some
		     * struct/enum declaration is getting lost.
		     */
		    if (listresult) {
			/*
			 * If the parameter type list is simply void, then
			 * this should really be a noargs function.
			 */
			enum TypeRecordCode             FuncTRC;
			FuncTRC = TRC_ANSIfunction;
			if (listresult == ELLIPSIS) {
			    FuncTRC = TRC_ANSIELLIPSISfunction;
			} else if (Via(templist)->count == 1) {
			    if (isVoidType(Via(FirstSym(templist))->TP)) {
				FuncTRC = TRC_NOARGSfunction;
			    }
			}
			/*
			 * Also, this parameter type list must have valid
			 * declarators within it, if this is a function
			 * definition and not merely a prototype.  Also, if
			 * this is a function definition, the parameter type
			 * list symbol table should be entered into current
			 * scope.
			 * 
			 */
			Via(result)->TP = BuildTypeRecord(Via(result)->TP, FuncTRC, SGN_unknown);
			SetTPMembers(Via(result)->TP, templist);
			if (!NextIs(')')) {
			    SyntaxError("Missing right parenthesis");
			}
		    } else {
			templist = RawTable(11);
			listresult = Do_identifier_list(templist);
			/*
			 * QQQQ Shouldn't this be identifier list with
			 * ellipsis ?
			 */
			if (listresult) {
			    if (gProject->itsOptions->requireProtos) {
				TypeError("Prototypes required");
			    }
			    Via(result)->TP = BuildTypeRecord(Via(result)->TP, TRC_OLDfunction, SGN_unknown);
			    SetTPMembers(Via(result)->TP, templist);
			    if (!NextIs(')')) {
				SyntaxError("Missing right parenthesis");
			    }
			} else {
			    SyntaxError("Expecting identifier list or prototype");
			}
		    }
		}
	    } else {
		donedirecting = 1;
	    }
	}
	if (isArrayType(Via(result)->TP)) {
	    SetArraySizes(Via(result)->TP);
	}
    }
    if (oldtype && newtype && result) {
	TypeRecordVia_t                 curtype;
	curtype = newtype;
	while (GetTPBase(curtype)) {
	    curtype = GetTPBase(curtype);
	}
	SetTPBase(curtype, Via(result)->TP);
	Via(result)->TP = newtype;
    }
    if (already) {
	int                             preapproved;
	preapproved = 0;
	if (!SameType(already, Via(result)->TP)) {
	    if (isFunctionType(already)) {
		if (isFunctionType(Via(result)->TP)) {
		    if (GetTPKind(already) == TRC_OLDfunction) {
			if (!CountFuncArgs(already)) {
			    preapproved = 1;
			}
		    }
		    if (GetTPKind(Via(result)->TP) == TRC_OLDfunction) {
			if (!CountFuncArgs(Via(result)->TP)) {
			    preapproved = 1;
			}
		    }
		    if (!preapproved) {
			switch (GetTPKind(already)) {
			case TRC_OLDfunction:
			    switch (GetTPKind(Via(result)->TP)) {
			    case TRC_ANSIfunction:
				if (!SameType(isFunctionType(already), isFunctionType(Via(result)->TP))) {
				    DeclErrorSYM("Conflicting function redeclaration of ", result);
				}
				break;
			    case TRC_NOARGSfunction:
				if (CountFuncArgs(already)) {
				    DeclErrorSYM("Conflicting function redeclaration of ", result);
				}
				break;
			    default:
				DeclErrorSYM("Conflicting function redeclaration of ", result);
				break;
			    }
			    break;
			case TRC_ANSIfunction:
			    switch (GetTPKind(Via(result)->TP)) {
			    case TRC_OLDfunction:
				if (!SameType(isFunctionType(already), isFunctionType(Via(result)->TP))) {
				    DeclErrorSYM("Conflicting function redeclaration of ", result);
				}
				if (Via(GetTPMembers(already))->count !=
				Via(GetTPMembers(Via(result)->TP))->count) {
				    DeclErrorSYM("Conflicting function redeclaration of ", result);
				}
				/*
				 * QQQQ Here, should we set Via(result)->TP
				 * to already ?
				 */
				/*
				 * We can't check further because the old
				 * style declaration list has not been read.
				 * This level of checking will occur in
				 * Do_function_body(void)
				 */
				break;
			    default:
				DeclErrorSYM("Conflicting function redeclaration of ", result);
				break;
			    }
			    break;
			case TRC_NOARGSfunction:
			    if (!((GetTPKind(Via(result)->TP) == TRC_OLDfunction) && (OldNoArgs))) {
				DeclErrorSYM("Conflicting function redeclaration of ", result);
			    }
			    break;
			default:
			    DeclErrorSYM("Conflicting function redeclaration of ", result);
			    break;
			}
		    }
		} else {
		    DeclErrorSYM("Conflicting function redeclaration of ", result);
		}
	    } else if (isIncompleteType(already)) {
		/*
		 * TODO Do something here.
		 */
	    } else {
		DeclErrorSYM("Conflicting redeclaration of ", result);
	    }
	}
    }
    return result;
}

TypeRecordVia_t
Do_pointer(TypeRecordVia_t base)
{
    /*
     * pointer : '*' | '*' type_qualifier_list | '*' pointer | '*'
     * type_qualifier_list pointer ;
     */

    /*
     * This routine is only called from within do_declarator and
     * do_abstract_declarator.  The information in it is only used to
     * modify/construct type records.
     */

    TypeRecordVia_t                 result;
    int                             done;
    result = base;
    if (NextIs('*')) {
	result = BuildTypeRecord(result, TRC_pointer, SGN_unknown);
	done = 0;
	while (!done) {
	    if (NextIs(CONST)) {
		TypeRecordVia_t                 other;
		other = CopyTypeRecord(result);
		SetTPQual(other, SCC_const);
		result = other;
	    } else if (NextIs(VOLATILE)) {
		TypeRecordVia_t                 other;
		other = CopyTypeRecord(result);
		SetTPQual(other, SCC_volatile);
		result = other;
		UserWarning(WARN_novolatile);
	    } else if (NextIs('*')) {
		result = BuildTypeRecord(result, TRC_pointer, SGN_unknown);
	    } else {
		done = 1;
	    }
	}
    }
    return result;
}

int
Do_parameter_identifier_list(SymListVia_t table)
/* What happened to this function ?!  It's not used anywhere ? */
{
    /*
     * parameter_identifier_list : identifier_list | identifier_list ','
     * ELLIPSIS ;
     */

    int                             result;
    result = Do_identifier_list(table);
    if (result) {
	if (result == ELLIPSIS) {
	    /*
	     * TODO What here ?  Modify table to indicate a vararg function ?
	     */
	}
    }
    return result;
}

int
Do_identifier_list(SymListVia_t table)
{
    /*
     * identifier_list : IDENTIFIER | identifier_list ',' IDENTIFIER ;
     */

    int                             result = 0;
    int                             donelisting;
    if (NextIs(IDENTIFIER)) {
	donelisting = 0;
	result = 1;
	TableAdd(table, (LastToken));
	while (!donelisting) {
	    if (NextIs(',')) {
		if (NextIs(IDENTIFIER)) {
		    TableAdd(table, (LastToken));
		} else {
		    if (NextIs(ELLIPSIS)) {
			result = ELLIPSIS;
		    } else {
			SyntaxError("Extra comma");
		    }
		    donelisting = 1;
		}
	    } else {
		donelisting = 1;
	    }
	}
    } else {
	result = 0;
    }
    return result;
}

int
Do_parameter_type_list(SymListVia_t table)
{
    /*
     * parameter_type_list : parameter_list | parameter_list ',' ELLIPSIS ;
     */

    /* Note that the ELLIPSIS is now handled in Do_parameter_list() */
    int                             result;
    result = Do_parameter_list(table);
    if (result) {
	if (result == ELLIPSIS) {
	    /*
	     * TODO What here ?  Modify table to indiciate proto with vararg
	     * ?
	     */
	}
    }
    return result;
}

int
Do_parameter_list(SymListVia_t table)
{
    /*
     * parameter_list : parameter_declaration | parameter_list ','
     * parameter_declaration ;
     */

    int                             result;
    int                             donelisting;
    result = Do_parameter_declaration(table);
    if (result) {
	result = 1;
	donelisting = 0;
	while (!donelisting) {
	    if (NextIs(',')) {
		int                             tmp;
		tmp = Do_parameter_declaration(table);
		if (!tmp) {
		    if (NextIs(ELLIPSIS)) {
			result = ELLIPSIS;
		    } else {
			SyntaxError("Extra comma");
		    }
		    donelisting = 1;
		}
	    } else {
		donelisting = 1;
	    }
	}
    }
    return result;
}

int
Do_parameter_declaration(SymListVia_t table)
{
    /*
     * parameter_declaration : declaration_specifiers declarator |
     * declaration_specifiers abstract_declarator | declaration_specifiers ;
     */

    int                             result;
    enum StorageClassCode           stor_class;
    TypeRecordVia_t                 typerec;
    SYMVia_t                        tmp;
    int junk;
    tmp = 0;
    result = 0;
    typerec = Do_declaration_specifiers(1, &stor_class, &junk);
    /*
     * QQQQ Is a storage class really legal here, or just const/volatile ?
     */
    if (typerec) {
	TypeRecordVia_t                 newtype;
	result = 1;
	newtype = 0;
	tmp = Do_declarator(table, typerec, &newtype);
	if (!tmp) {
	    /*
	     * In this case, we need to insert a fake name (which we can
	     * recognize as fake later !) because the type STILL must go into
	     * the symbol table.  This is a proto we are parsing.
	     */
	    SYMVia_t                        fake;
	    fake = TableAdd(table, ("-fake-"));
	    newtype = Do_abstract_declarator(newtype);
	    Via(fake)->TP = newtype;
	    tmp = fake;
	}
    }
    if (tmp) {
	if (isArrayType(Via(tmp)->TP)) {
	    Via(tmp)->TP =
		BuildTypeRecord(isArrayType(Via(tmp)->TP), TRC_pointer, SGN_unknown);
	}
	if (isFunctionType(Via(tmp)->TP)) {
	    Via(tmp)->TP = BuildTypeRecord(Via(tmp)->TP, TRC_pointer, SGN_unknown);
	}
    }
    return result;
}

TypeRecordVia_t
Do_type_name(void)
{
    /*
     * type_name : specifier_qualifier_list | specifier_qualifier_list
     * abstract_declarator ;
     */
    /*
     * This routine is called for type casts.  It should return a type
     * record.  Abstract declarators are declarators which do not go into the
     * symbol table, therefore no table is needed here.  The
     * abstract_declarator routines only modify the type record they receive.
     */
    TypeRecordVia_t                 result;
    enum StorageClassCode           nope;
    int junk;
    result = Do_declaration_specifiers(0, &nope, &junk);
    if (result) {
	result = Do_abstract_declarator(result);
    }
    return result;
}

TypeRecordVia_t
Do_abstract_declarator(TypeRecordVia_t typerec)
{
    /*
     * abstract_declarator : pointer | direct_abstract_declarator | pointer
     * direct_abstract_declarator ;
     */
    /*
     * This routine is only called for parameter type lists (for protos) and
     * type casts...
     */

    typerec = Do_pointer(typerec);
    typerec = Do_direct_abstract_declarator(typerec);
    return typerec;
}

TypeRecordVia_t
Do_direct_abstract_declarator(TypeRecordVia_t typerec)
{
    /*
     * direct_abstract_declarator : '(' abstract_declarator ')' | '[' ']' |
     * '[' constant_expr ']' | direct_abstract_declarator '[' ']' |
     * direct_abstract_declarator '[' constant_expr ']' | '(' ')' | '('
     * parameter_type_list ')' | direct_abstract_declarator '(' ')' |
     * direct_abstract_declarator '(' parameter_type_list ')' ;
     */

    int                             result;
    ParseTreeVia_t                  tmp;
    int                             donedecl;
    result = 0;
    if (NextIs('(')) {
	result = 1;
	if (NextIs(')')) {
	    typerec = BuildTypeRecord(typerec, TRC_OLDfunction, SGN_unknown);
	} else {
	    typerec = Do_abstract_declarator(typerec);
	    if (typerec) {
		if (!NextIs(')')) {
		    SyntaxError("Missing right parenthesis");
		}
	    } else {
		SyntaxError("Expected abstract declarator");
	    }
	}
    } else if (NextIs('[')) {
	result = 1;
	if (NextIs(']')) {
	    typerec = BuildTypeRecord(typerec, TRC_array, SGN_unknown);
	} else {
	    tmp = Do_constant_expr();
	    if (tmp) {
		int                             constvalue;
		constvalue = GetIntValue(tmp);
		FreeTree(tmp);
		typerec = BuildTypeRecord(typerec, TRC_array, SGN_unknown);
		SetTPSize(typerec, constvalue);
		if (!NextIs(']'))
		    SyntaxError("Missing right bracket");
	    } else {
		SyntaxError("Expected constant expression for array size");
	    }
	}
    }
    if (result) {
	donedecl = 0;
	while (!donedecl) {
	    if (NextIs('[')) {
		if (NextIs(']')) {
		    typerec = BuildTypeRecord(typerec, TRC_array, SGN_unknown);
		    SetTPFlags(typerec, GetTPFlags(typerec) | INCOMPLETEMASK);
		    SetTPSize(typerec, 0);
		} else {
		    tmp = Do_constant_expr();
		    if (tmp) {
			int                             constvalue;
			constvalue = GetIntValue(tmp);
			FreeTree(tmp);
			typerec = BuildTypeRecord(typerec, TRC_array, SGN_unknown);
			SetTPSize(typerec, constvalue);
			if (!NextIs(']'))
			    SyntaxError("Missing right bracket");
		    } else {
			SyntaxError("Expected constant expression for array size");
		    }
		}
	    } else if (NextIs('(')) {
		SymListVia_t                    prot = NULL;
		if (NextIs(')')) {
		    if (gProject->itsOptions->requireProtos) {
			TypeError("Prototypes required");
		    }
		    typerec = BuildTypeRecord(typerec, TRC_OLDfunction, SGN_unknown);
		} else {
		    prot = RawTable(11);
		    result = Do_parameter_type_list(prot);
		    if (result) {
			typerec = BuildTypeRecord(typerec, TRC_ANSIfunction, SGN_unknown);
			SetTPMembers(typerec, prot);
			if (!NextIs(')'))
			    SyntaxError("Missing right parenthesis");
		    } else {
			SyntaxError("Expected parameter type list");
			FreeSymbolList(prot, 0);
		    }
		}
	    } else {
		donedecl = 1;
	    }
	}
	if (isArrayType(typerec)) {
	    SetArraySizes(typerec);
	}
    }
    return typerec;
}

ParseTreeVia_t
Do_initializer(void)
{
    /*
     * initializer : assignment_expr | '{' initializer_list '}' | '{'
     * initializer_list ',' '}' ;
     */

    ParseTreeVia_t                  result;
    int                             tmp;
    result = Do_assignment_expr();
    tmp = 0;
    if (result) {
	PtrGenerate(&result);
    }
    if (!result) {
	if (NextIs('{')) {
	    result = Do_initializer_list();
	    if (result) {
		NextIs(',');
		result = BuildTreeNode(PTF_multi_initializer, result, NULL, NULL);
		if (!NextIs('}')) {
		    SyntaxError("Missing right bracket");
		}
	    } else {
			SyntaxError("Expected initializer list");
	    }
	} else {
	    result = 0;
	}
    }
    return result;
}

ParseTreeVia_t
Do_initializer_list(void)
{
    /*
     * initializer_list : initializer | initializer_list ',' initializer ;
     */

    ParseTreeVia_t                  result;
    int                             donelisting;
    result = Do_initializer();
    if (result) {
		donelisting = 0;
		while (!donelisting) {
		    if (NextIs(',')) {
				ParseTreeVia_t                  tmp;
				tmp = Do_initializer();
				if (tmp) {
				    result = BuildTreeNode(PTF_initializer_list, tmp, result, NULL);
				} else {
				    donelisting = 1;
				}
		    } else {
				donelisting = 1;
		    }
		}
    }
    return result;
}

int
Do_declaration_list(SymListVia_t table)
{
    /*
     * declaration_list : declaration | declaration_list declaration ;
     */

    int                             result;
    int                             donelisting;
    int                             count;
    count = 0;
    result = Do_declaration(table);
    if (result) {
	count = 1;
	donelisting = 0;
	while (!donelisting) {
	    result = Do_declaration(table);
	    if (result) {
		count++;
	    } else {
		donelisting = 1;
	    }
	}
    }
    return count;
}

SymImageVia_t
Do_external_declaration(short filenum)
{
    /*
     * external_declaration : declarator function_body |
     * declaration_specifiers declarator function_body |
     * declaration_specifiers ';' | declaration_specifiers
     * init_declarator_list ';' | @interface | @implementation ;
     */

    TypeRecordVia_t                 typerec;
    ParseTreeVia_t                  result = 0;
    SymImageVia_t                   declared = 0;
    SymImageVia_t                   cursym;
    SYMVia_t                        thesym;
    int                             funcresult = 0;
    int pascalFlag;
    enum StorageClassCode           stor_class;
	typerec = Do_declaration_specifiers(1, &stor_class, &pascalFlag);
	if (typerec) {
	    funcresult = 1;
	    if (NextIs(';')) {
		/*
		 * What is a declaration specifier without a declarator ? It
		 * is a struct, union, enum, or something like that. In this
		 * case, we check the type record to see if it really was
		 * something that made a table entry.  We want to disallow
		 * declarations such as int;
		 */
		if (!(isStructUnionType(typerec) || isEnumType(typerec) ||
		      isFunctionType(typerec))) {
		    SyntaxError("Empty declarator");
		}
	    } else {
		if (stor_class == SCC_typedef) {
		    char                            nm[64];
		    declared = Do_init_declarator_list(TP_defnames, typerec, stor_class);
		    GetSymName(Via(declared)->Symbol, nm);
		    Via(Via(declared)->Symbol)->TP = BuildTagTypeRecord(Via(Via(declared)->Symbol)->TP, TRC_typedef,
									nm);
		    SetTPFlags(Via(Via(declared)->Symbol)->TP, GetTPFlags(Via(Via(declared)->Symbol)->TP) | SIZEDONE);
		    SetTypeSize(Via(Via(declared)->Symbol)->TP);
		} else {
		    declared = Do_init_declarator_list(GlobalSymbolTable, typerec, stor_class);
		    if (declared) {
			char                            thename[64];
			cursym = declared;
			while (cursym) {
			    thesym = Via(cursym)->Symbol;
			    GetSymName(thesym, thename);
			    if (GetTPFlags(Via(thesym)->TP) & ISPASCALMASK) {
					int                             ndx = 0;
					while (thename[ndx]) {
					    if (islower(thename[ndx]))
						thename[ndx] = toupper(thename[ndx]);
					    ndx++;
					}
			    }
				if (gProject->itsOptions->bigGlobals) {
				    Via(thesym)->M68kDef.Loc = BuildLargeGlobal(MakeUserLabel(thename));
				    MakeLocGlobal(Via(thesym)->M68kDef.Loc);
				} else {
				    Via(thesym)->M68kDef.Loc = BuildARegLabelDisplace(5, MakeUserLabel(thename));
				    MakeLocGlobal(Via(thesym)->M68kDef.Loc);
				}
			    SetLocSZ(Via(thesym)->M68kDef.Loc, M68_TypeSize(Via(thesym)->TP));
			    if (isFloatingType(Via(thesym)->TP))
				SetLocIsFloat(Via(thesym)->M68kDef.Loc, 1);
			    cursym = Via(cursym)->next;
			}
		    }
		}
		if (declared) {
		    if (NextIs(';')) {
			/*
			 * Here, this could have been any number of things.
			 * One possibility is that it was a prototype. Of
			 * course, it could also have been a normal variable
			 * declaration or something like that.  Or a direct
			 * function.
			 */
			cursym = declared;
			while (cursym) {
			    if (isFunctionType(Via(Via(cursym)->Symbol)->TP)) {
				if (Via(Via(cursym)->Symbol)->Definition.Initializer) {
				    /* This is a direct function */
				    char                            nm[64];
				    SetTPTrap(Via(Via(cursym)->Symbol)->TP, Via(Via(cursym)->Symbol)->Definition.Initializer);
				    GetSymName(Via(cursym)->Symbol, nm);
				    SetTPParam(Via(Via(cursym)->Symbol)->TP, FindParam(nm));
				    if (GetTPParam(Via(Via(cursym)->Symbol)->TP))
					Via(Via(cursym)->Symbol)->SymbolFlags |= 512;
				    /* Danger UNNAMED FLAG */
				}
				Via(Via(cursym)->Symbol)->storage_class = SCC_extern;
			    }
			    cursym = Via(cursym)->next;
			}
		    } else {
			ParseTreeVia_t                  tmp2;
			if (Via(declared)->count > 1) {
			    SyntaxError("You may not define more than one function at once");
			}
			tmp2 = Do_function_body(Via(declared)->Symbol);
			if (!tmp2) {
			    SyntaxError("Expected function body");
			}
		    }
		} else {
		    SyntaxError("Expected declarator");
		}
	    }
	} else {		/* No typerec */
	    TypeRecordVia_t                 newtype;
	    typerec = BuildTypeRecord(0, TRC_int, SGN_signed);
	    /*
	     * This means the type of the function is missing.
	     */
	    if (pascalFlag) {
		TypeRecordVia_t                 other;
		other = CopyTypeRecord(typerec);
		SetTPFlags(other, GetTPFlags(other) | ISPASCALMASK);
		typerec = other;
	    }
	    thesym = Do_declarator(GlobalSymbolTable, typerec, &newtype);
	    if (thesym) {
		ParseTreeVia_t                  tmptree;
		char                            thename[64];
		Via(thesym)->storage_class = stor_class;
		UserWarning(WARN_missingreturntype);
		GetSymName(thesym, thename);
		if (GetTPFlags(Via(thesym)->TP) & ISPASCALMASK) {
		    int                             ndx = 0;
		    while (thename[ndx]) {
			if (islower(thename[ndx]))
			    thename[ndx] = toupper(thename[ndx]);
			ndx++;
		    }
		}
	    if (gProject->itsOptions->bigGlobals) {
			Via(thesym)->M68kDef.Loc = BuildLargeGlobal(MakeUserLabel(thename));
			MakeLocGlobal(Via(thesym)->M68kDef.Loc);
	    } else {
			Via(thesym)->M68kDef.Loc = BuildARegLabelDisplace(5, MakeUserLabel(thename));
			MakeLocGlobal(Via(thesym)->M68kDef.Loc);
	    }
		SetLocSZ(Via(thesym)->M68kDef.Loc, M68_TypeSize(Via(thesym)->TP));
		if (NextIs(';')) {
		    funcresult = 1;
		} else {
		    tmptree = Do_function_body(thesym);
		    if (tmptree) {
				funcresult = 1;
		    } else {
				SyntaxError("Expected function body");
		    }
		}
	    if (Via(thesym)->storage_class != SCC_extern) {
			if (Via(thesym)->storage_class != SCC_typedef) {
			    GenSymbol(thesym, GlobalCodes);
			}
	    }
	    } else {
			if (stor_class) {
			    DeclError("Just a storage class ?! What are you declaring ?");
			}
	    }
	}
	if (declared) {
	    cursym = declared;
	    while (cursym) {
		if (isVoidType(Via(Via(cursym)->Symbol)->TP)) {
		    DeclError("There are no void objects !");
		}
		    if (Via(Via(cursym)->Symbol)->storage_class != SCC_typedef) {
			if (Via(Via(cursym)->Symbol)->storage_class != SCC_extern) {
			    GenSymbol(Via(cursym)->Symbol, GlobalCodes);
			}
		    }
		cursym = Via(cursym)->next;
	    }
	} else {
	    declared = ListAdd(declared, thesym);
	}
    GenSegments(GlobalCodes);
    GenStringLits(GlobalCodes);
    GenFloatLits(GlobalCodes);
    GenStatics(GlobalCodes);
#ifdef nomore
    Optimize68(GlobalCodes);
    DumpCodeList(GlobalCodes, GlobalRecords);
    FlushOMF(filenum);
    KillCodesList(GlobalCodes);
    KillSomeLocations();
#endif
    return declared;
}
