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

#ifndef Lexer_INTERFACE
#define Lexer_INTERFACE


/* Parser token codes... */
/*
 * Note : At this time, the parser token codes are being left as integer
 * defines, not an enum.  The reason for this is that the tokens must be able
 * to include the values for the single character C tokens, which ARE handled
 * as single characters, for readability.
 */
#define IDENTIFIER 257
#define FLOATCONSTANT 329
#define INTCONSTANT 258
#define CHARCONSTANT 330
#define STRING_LITERAL 259
#define SIZEOF 260
#define PTR_OP 261
#define INC_OP 262
#define DEC_OP 263
#define LEFT_OP 264
#define RIGHT_OP 265
#define LE_OP 266
#define GE_OP 267
#define EQ_OP 268
#define NE_OP 269
#define AND_OP 270
#define OR_OP 271
#define MUL_ASSIGN 272
#define DIV_ASSIGN 273
#define MOD_ASSIGN 274
#define ADD_ASSIGN 275
#define SUB_ASSIGN 276
#define LEFT_ASSIGN 277
#define RIGHT_ASSIGN 278
#define AND_ASSIGN 279
#define XOR_ASSIGN 280
#define OR_ASSIGN 281
#define TYPEDEF_NAME 282
#define TYPEDEF 283
#define EXTERN 284
#define STATIC 285
#define AUTO 286
#define REGISTER 287
#define CHAR 288
#define SHORT 289
#define INT 290
#define LONG 291
#define SIGNED 292
#define UNSIGNED 293
#define FLOAT 294
#define DOUBLE 295
#define CONST 296
#define VOLATILE 297
#define VOID 298
#define STRUCT 299
#define UNION 300
#define ENUM 301
#define ELIPSIS 302
#define RANGE 303
#define CASE 304
#define DEFAULT 305
#define IF 306
#define ELSE 307
#define SWITCH 308
#define WHILE 309
#define DO 310
#define FOR 311
#define GOTO 312
#define CONTINUE 313
#define BREAK 314
#define RETURN 315
#define PASCAL 316
#define INLINE 317
#define ELLIPSIS 328
#define ASM 318
#define PASCSTRING_LITERAL 319
#define DEFINED 400
/* DEFINED is a strange token - see ecc.c comments for details. */
/* Key words for Object oriented extensions */
#define atINTERFACE 500
#define atIMPLEMENTATION 501
#define atEND 502
#define atSELECTOR 503
#define atDEFS 504
#define atENCODE 505
#define atPUBLIC 506
#define EXTENDED 507
#define COMP 508

#endif
