#ifndef bsioh
#define bsioh

// Alternativ: #pragma once

#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#ifdef ibm
#include <stddef.h>
#endif
#ifdef apple
#include <size_t.h>
#include <stat.h>
#endif
#include <stdarg.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <math.h>

#ifdef modus_inter
#ifdef apple
#include <console.h>
#include <pascal.h>
#endif
#endif


/***************************************************************************/
/*****   Include-Datei f�r SETI: Allgemein I/O und INTER-MODE          *****/
/*****   Autor:    Andreas Amoroso                                     *****/
/*****   Datum:    11.8.1993                                           *****/
/***************************************************************************/

                            
                            /* Konstanten f�r Betriebsart                            */
                            /* ==========================                           */
                            
#define MCPMODE         1       /* MCP-Modus                                        */
#define INTERACTIVEMODE 0       /* Interaktiver Modus                               */


                            /* Allgemeines f�r I/O...                                */
                            /* ======================                               */

#define MaxFullPathName FILENAME_MAX       
                            /* Maximaler Pfadname inkl. Extension  (Mac!)           */
#define MaxLineD        132 /* Maximale L�nge einer Ausgabezeile in eine Datei       */
#define MaxLineS        80  /* Maximale L�nge einer Ausgabezeile auf der Konsole */

#ifdef modus_inter
#define MaxLine1        MaxLineS    
                            /* Maximale L�nge erste Ausgabezeile auf der Konsole */
#define MaxLine2        MaxLineS
                            /* Maximale L�nge zweite Ausgabezeile auf der Konsole   */
#define MaxLineCntS 24      /* Anzahl der Zeilen die am Stueck ausgegeben werden    */
#endif

#define MaxMessLen      74  /* Maximale L�nge der Ausgabemessage                 */
#define MaxAuxMessLen   20  /* Maximale L�nge der zus�tzlichen Ausgabemessage     */
#define MaxErrMessLen   50  /* Maximale L�nge der Systemfehlermeldung                */
#define MaxModulNamLen  20  /* Maximale L�nge des Modulnamen                     */
#define MaxTimeDateLen  21  /* Maximale L�nge Date/Time-String                       */
#define MaxDefStrLen    5   /* Maximale L�nge eines Defaultstring                    */
#define MaxHeadOutLen   44  /* Maximale L�nge der Header informationen               */
#define MaxMenuPointLen 72  /* Maximale L�nge der Men�punkte                      */
#define MaxMenuTitleLen 40  /* Maximale L�nge des Men�titels                      */
#define MaxMenuPointCnt 14  /* Maximale Anzahl von Men�punkten                       */
                            /* Achtung MaxMenuPointCnt <= 16 vorausgesetzt!!!       */
#define MenuOffset      5   /* Kopfzeilen und Space                                 */
#define MaxMenuString   MaxLineS * (MaxMenuPointCnt + MenuOffset)
                            /* L�nge Gesamtmen� als String (Ausgabe)              */
#define MenuTitle       " MENU: \0"
                            /* Men�einleitung                                        */
                            
                            
                            /* Konstanten f�r I/O: Tasten                            */
                            /* ==========================                           */
                            
#define ENTER       3       /* Taste Enter                                          */
#define HELP        5       /* Taste Help                                           */
#define DELETE      8       /* .                                                    */
#define TAB         9       /*                                                      */
#define NEWLINE     10      /* Return (getchar in buffered console-mode)            */
#define RETURN      13      /*                                                      */
#define ESCAPE      27      /*                                                      */
#define CLEFT       28      /*                                                      */
#define CRIGHT      29      /*                                                      */
#define CUP         30      /*                                                      */
#define CDOWN       31      /*                                                      */
#define PLUS        43      /*                                                      */
#define COLON       44      /*                                                      */
#define MINUS       45      /*                                                      */
#define KEY0        48      /* 0                                                    */
#define KEYa        97      /* Buchstabentasten                                     */
#define KEYA        65      /* .                                                    */
#define KEYe        101     /*                                                      */
#define KEYE        69      /*                                                      */
#define KEYh        104     /*                                                      */
#define KEYH        72      /*                                                      */
#define KEYj        106     /*                                                      */
#define KEYJ        74      /*                                                      */
#define KEYn        110     /*                                                      */
#define KEYN        78      /*                                                      */
#define KEYq        113     /*                                                      */
#define KEYQ        81      /*                                                      */
#define KEYx        120     /*                                                      */
#define KEYX        88      /*                                                      */
#define KEYy        121     /*                                                      */
#define KEYY        89      /*                                                      */
#define KEYS	    83	    /*													    */
#define KEYs	    115	    /*													    */
#define KEYL	    76	    /*							   							*/
#define KEYl	    108	    /*													    */
																	


                            /* Konstanten f�r Input-Return-Codes         	        */
                            /* =================================                    */

#define     INPUTOK 0       /* Korrekte Eingabe                                     */
#define     INPUTVO 1       /* Leere Eingabe                                        */
#define     INPUTQI 2       /* Eingabe Q                                            */
#define     INPUTHE 4       /* Eingabe H                                            */
#define     INPUTIR 8       /* Unzul�ssige Eingabe                                  */
#define     INPUTER 16      /* Fehler bei der Eingabe                               */

                            
                            /* Konstanten f�r I/O: Meldungen                 	    */
                            /* =============================                        */
                            
#define ShortScr        0   /* Ausgabeformate                                       */
#define LongScr         1
// #define ShortFile        /* Future use                                           */
#define LongFile        3   
#define ModusReturn     0   /* Ausgabemodus f�r GenOutAcknowledge                   */
#define ModusNoRet      1   /* .                                                    */
#define ReqStr          " ----> \0"
#define InfStr          " INFO: \0"
#define SnfStr          " SINF: \0"
#define WrnStr          " WARN: \0"
#define ErrStr          " ERR : \0"
#define SrrStr          " SERR: \0"
#define ReqTStr1        ": \0"
#define ReqTStr2        ": [\0"
#define ReqTStr3        "]\0"
#define HelStr          " Help:\0"
#define LotStr          "+++++ \0"

#define AReq            100 /* Acknowledge Request                                  */
#define BReq            101 /* Binary Request                                       */
#define MReq            102 /* Menu Request                                         */
                            /* Achtung, Da es maximal 16 Men�punkte gibt => 
                            /* EIN CHARACTER ALS HEX bzw x = letzter Men�punkt + 1  */
                            /* oder h = letzter Men�punkt + 2                       */
#define IReq            103 /* Integer Request                                      */
#define RReq            104 /* Real Request                                         */
#define SReq            105 /* String Request                                       */

#define AReqStr         "Acknowledge Request:\0"
#define BReqStr         "Binary Input Request:\0"
#define MReqStr         "Menu Input Request:\0" 
#define IReqStr         "Integer Input Request:\0"  
#define RReqStr         "Real Input Request:\0" 
#define SReqStr         "String Input Request:\0"   


#define IInf            200 /* Informational Message                                */
#define AInf            201 /* Information containing user's answer to above request*/
#define EInf            202 /* Explanation                                          */

#define IInfStr         "Informational message: \0"
#define AInfStr         "User's answer: \0"
#define EInfStr         "Explanation: \0"                                           


#define CSnf            300 /* Execution successfully completed                     */
#define WSnf            301 /* Execution completed with warnings                    */
#define ASnf            302 /* Execution abnormally terminated                      */
#define ESnf            303 /* Execution aborted due to error                       */
#define SSnf            304 /* Execution aborted due to severe error                */
#define PSnf            305 /* Computation of "AuxStr" is in progress               */

#define CSnfStr         "Execution successfully completed.\0"
#define WSnfStr         "Execution completed with warnings.\0"
#define ASnfStr         "Execution abnormally terminated.\0"
#define ESnfStr         "Execution aborted due to error.\0"
#define SSnfStr         "Execution aborted due to severe error.\0"
#define PSnfStr         "Computation is in progress...\0"
                
                            /* USE WARNINGS IN CASE OF INPUT ERRORS IN "INTER_MODE! */
                            /* USE ShortScr FORMAT IN INTER MODE UNLESS ERRORS OR   */
                            /* SEVERE ERRORS OCCUR. IN THESE CASES ONLY USE         */
                            /* LongScr FORMAT!!!                                    */
                            /* ==================================================== */
                            
#define IWrn            400 /* Inconsistency detected                               */
#define DWrn            401 /* Incomplete definition detected                       */
#define SWrn            402 /* Unknown syntax detected                              */
#define AWrn            403 /* Data not found                                       */
#define JWrn            404 /* Instruction not found                                */
#define BWrn            405 /* Data invalid                                         */
#define KWrn            406 /* Instruction invalid                                  */

#define IWrnStr         "Inconsistency detected.\0"
#define DWrnStr         "Incomplete definition detected.\0"
#define SWrnStr         "Unknown syntax detected.\0"
#define AWrnStr         "Data not found.\0"
#define JWrnStr         "Instruction not found.\0"
#define BWrnStr         "Data invalid.\0"
#define KWrnStr         "Instruction invalid.\0"


#define IErr            500 /* Fatal inconsistency detected                         */
#define DErr            501 /* Fatal incompleteness in definition detected          */
#define SErr            502 /* Illegal syntax detected                              */
#define AErr            503 /* Required data not found                              */
#define JErr            504 /* Required instruction not found                       */
#define BErr            505 /* Required data invalid                                */
#define KErr            506 /* Required instruction invalid                         */
#define WErr            507 /* Wrong type of input                                  */
#define RErr            508 /* Input out of range                                   */
#define UErr            509 /* Unknown internal error occured                       */
#define EErr            510 /* Program abort requested                              */

#define IErrStr         "Fatal inconsistency detected.\0"
#define DErrStr         "Fatal incompleteness in def. detected.\0"
#define SErrStr         "Illegal syntax detected.\0"
#define AErrStr         "Required data not found.\0"
#define JErrStr         "Required instruction not found.\0"
#define BErrStr         "Required data invalid.\0"
#define KErrStr         "Required instruction invalid.\0"
#define WErrStr         "Wrong type of input.\0"
#define RErrStr         "Input out of range.\0"
#define UErrStr         "Unknown internal error occured.\0"
#define EErrStr         "Program abort requested.\0"


#define ASrr            600 /* Memory request failed.\0"							*/
#define FSrr            601 /* I/O Request failed: Unable to find file              */
#define NSrr            602 /* I/O Request failed: Unable to rename file            */
#define OSrr            603 /* I/O Request failed: Unable to open file              */
#define QSrr            604 /* I/O Request failed: Unable to close file             */
#define RSrr            605 /* I/O Request failed: Unable to read file              */
#define WSrr            606 /* I/O Request failed: Unable to write to file          */
#define CSrr            607 /* I/O Request failed: Unable to create file            */
#define DSrr            608 /* I/O Request failed: Unable to delete file            */
#define USrr            609 /* Unknown external error occured                       */
#define MSrr            610 /* Module not found                                     */
#define ISrr			611 /* Con file not available								*/

#define ASrrStr         "Memory request failed.\0"
#define FSrrStr         "I/O Req. failed: Unable to find file.\0"
#define NSrrStr         "I/O Req. failed: Unable to rename file.\0"
#define OSrrStr         "I/O Req. failed: Unable to open file.\0"
#define QSrrStr         "I/O Req. failed: Unable to close file.\0"
#define RSrrStr         "I/O Req. failed: Unable to read file.\0"
#define WSrrStr         "I/O Req. failed: Unable to write to file.\0"
#define CSrrStr         "I/O Req. failed: Unable to create file.\0"
#define DSrrStr         "I/O Req. failed: Unable to delete file.\0"
#define USrrStr         "Unknown external error occured.\0"
#define MSrrStr         "Module not found.\0"
#define ISrrStr			"Con file not available.\0"


//Types
//=====


//Machine-Specific
//================

#ifdef  ibm
#define MaxFileName     12  /* Maximaler Dateiname inkl. Extension (PC!)            */
#endif

#define MaxFileName     31  /* Maximaler Dateiname inkl. Extension (Mac!)           */


//REAL MACROS
//===========

#define bsintmin(a,b) ((((int)a)<=((int)b))?((int)a):((int)b))
#define bsintmax(a,b) ((((int)a)<=((int)b))?((int)b):((int)a))


//TYPES
//=====

enum bsbool {bsFalse = 0, bsTrue = 1};
typedef enum bsbool bsBoolean;


//PROTOTYPES: COPY THIS INTO YOUR FILE!!!
//            ===========================


void ConInit(char *);                                   //  Sets Console to 24 * 80 and gets params on macs 
int GenOutHeader(char *,char *,char *);                 //  prints SETI-Header
int GenOutTrailer(void);                                //  prints SETI-Trailor
int GenOutLine(int,int,int,char [],char [],char *,char *,char *,char *);
                                                        //  generates output string, long format
int GenOutLine0(int,int,char [],char *,char *);
int GenOutLine2(int,int,int,char [],char [],char *,char *,char *,char *);
int GenInAnswer(int,int,bsBoolean *,char *,char,int *,int,int,char *,char *,char *,char *,char *);
                                                        //  handles input requests
int GenOutHelp(char *);                                 //  generates Help-Info, NEEDS TO BE ADAPTED!!!
                                                        //  ===========================================
int GenOutMenu(char **, char *,char *, int);            //  generates menus 
void GenInName(char *str);								//  returns process name
void GenExit(unsigned );								//  sets error code and terminates program


//PROTOTYPES: used in this file
#ifdef modus_inter
int GenOutCentered(char *);                             //  centers Title and prints it
int GenOutAcknowledge(int);                             //  generates acknowledge request
int GenOutClrScr(int,char *);                           //  prints count empty lines, up to MaxLineCnt_S
#endif
int GenCorStr(int,char *);                              //  function(outttype)=outstring


#endif