/* Param.h */
#ifndef PARAM_H
#define PARAM_H

struct paramrec {
    char                            name[64];
    short                           returnreg;
    short                           args[4];
    short                           argcount;
    struct paramrec P__H           *next;
};
typedef struct paramrec         ParamRec_t;
typedef ParamRec_t P__H        *ParamRecVia_t;

#define param__NULL 0
#define param__A0 1
#define param__A1 2
#define param__D0 3
#define param__D1 4
#define param__D2 5

#endif