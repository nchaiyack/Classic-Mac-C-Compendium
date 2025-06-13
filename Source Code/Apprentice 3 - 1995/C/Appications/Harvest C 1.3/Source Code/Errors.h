/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains interfaces for error handling
 * 
 * 
 * 
 */

#ifndef Errors_INTERFACE
#define Errors_INTERFACE

#define MAXWARNINGS 100
/* ALL warnings should be optional. */

/*
 * Do NOT delete any warnings from this enum.  Numbers must remain constant.
 * The correct way to remove a warning, is to remove all calls to it, and
 * change its warning message in warnings.c.
 */
enum Ecc_Warnings {
    WARN_emptystatement = 1,
    WARN_multicharconstant,
    WARN_redundantcast,
    WARN_floateqcompare,
    WARN_discardfuncresult,
    WARN_assignif,
    WARN_nonvoidreturn,
    WARN_constantif,
    WARN_comparepointerint,
    WARN_pointernonequivassign,
    WARN_pointernonequivreturn,
    WARN_pointernonequivarg,
    WARN_constantswitch,
    WARN_constantwhile,
    WARN_constantdowhile,
    WARN_constantfor,
    WARN_nonintegralswitch,
    WARN_novolatile,
    WARN_AVAILABLE1,
    WARN_unusedvariable,
    WARN_deadcode,
    WARN_implicitdecl,
    WARN_preprocredef,
    WARN_nestedcomment,
    WARN_AVAILABLE2,
    WARN_goto,
    WARN_multireturn,
    WARN_emptycompound,
    WARN_missingreturntype,
    WARN_trigraphs,
    WARN_AVAILABLE3,
    WARN_pascal,
    WARN_AVAILABLE4,
    WARN_semiafterfunction
};

void
                                Output(char *mesg, GenericFileVia_t thefile);
void
                                OutputCR(char *mesg, GenericFileVia_t thefile);
void
                                UserMesg(char *mesg);
void
                                UserMesgCR(char *mesg);
void
                                File_Line(char *);

void
                                EccErrorNOFILE(char *mesg);

void
                                EccError(char *mesg);

int
                                AssertError(char *file, int line, char *expr);

void
                                TypeError(char *mesg);
void
                                SemanticError(char *mesg);

void
                                DeclError2(char *mesg, char *info);

void
                                DeclErrorSYM(char *mesg, SYMVia_t thesym);

void
                                DeclError(char *mesg);

void
                                FatalError2(char *mesg, char *info);

void
                                FatalError(char *mesg);

void
                                PreprocError2(char *mesg, char *info);

void
                                UserError(char *mesg);

void
                                PreprocError(char *mesg);

void
                                LexError(char *mesg);

void
                                SemanticError2(char *mesg, char *info);

void
                                InlineAsmError(char *mesg);

void
                                SyntaxError(char *mesg);

void
                                Gen68Error(char *mesg);

void
                                VeryBadParseError(char *mesg);
void
                                UserWarning(int num);

void
                                UserWarning2(int num, char *mesg);
void
                                EccWarning(char *mesg);

#endif
