#include "::h:gsupport.h"
 
#if EBCDIC
 /*
  * These tables are defined to provide translations between
  *  ISO Latin-1 and IBM code page 1047 for EBCDIC.  ASCII LF
  *  is translated to EBCDIC NL due to requirements of ANSI C
  *  text I/O.
  */
char ToEBCDIC[256] = {          /* ASCII->EBCDIC translation */
        0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f,
        0x16, 0x05, 0x15, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26,
        0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f,
        ' ',  '!',  '"',  '#',  '$',  '%',  '&',  '\'',
        '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
        '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
        '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
        '@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',
        'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
        'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
        'X',  'Y',  'Z',  '[',  '\\', ']',  '^',  '_',
        '`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',
        'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
        'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
        'x',  'y',  'z',  '{',  '|',  '}',  '~',  0x07,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x06, 0x17,
        0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x09, 0x0a, 0x1b,
        0x30, 0x31, 0x1a, 0x33, 0x34, 0x35, 0x36, 0x08,
        0x38, 0x39, 0x3a, 0x3b, 0x04, 0x14, 0x3e, 0xff,
        0x41, 0xaa, 0x4a, 0xb1, 0x9f, 0xb2, 0x6a, 0xb5,
        0xbb, 0xb4, 0x9a, 0x8a, 0xb0, 0xca, 0xaf, 0xbc,
        0x90, 0x8f, 0xea, 0xfa, 0xbe, 0xa0, 0xb6, 0xb3,
        0x9d, 0xda, 0x9b, 0x8b, 0xb7, 0xb8, 0xb9, 0xab,
        0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9e, 0x68,
        0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,
        0xac, 0x69, 0xed, 0xee, 0xeb, 0xef, 0xec, 0xbf,
        0x80, 0xfd, 0xfe, 0xfb, 0xfc, 0xba, 0xae, 0x59,
        0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9c, 0x48,
        0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,
        0x8c, 0x49, 0xcd, 0xce, 0xcb, 0xcf, 0xcc, 0xe1,
        0x70, 0xdd, 0xde, 0xdb, 0xdc, 0x8d, 0x8e, 0xdf
};
char FromEBCDIC[256] = {        /* EBCDIC->ASCII translation */
        0x00, 0x01, 0x02, 0x03, 0x9c, 0x09, 0x86, 0x7f,
        0x97, 0x8d, 0x8e, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x9d, 0x0a, 0x08, 0x87,
        0x18, 0x19, 0x92, 0x8f, 0x1c, 0x1d, 0x1e, 0x1f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x17, 0x1b,
        0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x05, 0x06, 0x07,
        0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04,
        0x98, 0x99, 0x9a, 0x9b, 0x14, 0x15, 0x9e, 0x1a,
        0x20, 0xa0, 0xe2, 0xe4, 0xe0, 0xe1, 0xe3, 0xe5,
        0xe7, 0xf1, 0xa2, 0x2e, 0x3c, 0x28, 0x2b, 0x7c,
        0x26, 0xe9, 0xea, 0xeb, 0xe8, 0xed, 0xee, 0xef,
        0xec, 0xdf, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0x5e,
        0x2d, 0x2f, 0xc2, 0xc4, 0xc0, 0xc1, 0xc3, 0xc5,
        0xc7, 0xd1, 0xa6, 0x2c, 0x25, 0x5f, 0x3e, 0x3f,
        0xf8, 0xc9, 0xca, 0xcb, 0xc8, 0xcd, 0xce, 0xcf,
        0xcc, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22,
        0xd8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0xab, 0xbb, 0xf0, 0xfd, 0xfe, 0xb1,
        0xb0, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
        0x71, 0x72, 0xaa, 0xba, 0xe6, 0xb8, 0xc6, 0xa4,
        0xb5, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
        0x79, 0x7a, 0xa1, 0xbf, 0xd0, 0x5b, 0xde, 0xae,
        0xac, 0xa3, 0xa5, 0xb7, 0xa9, 0xa7, 0xb6, 0xbc,
        0xbd, 0xbe, 0xdd, 0xa8, 0xaf, 0x5d, 0xb4, 0xd7,
        0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0xad, 0xf4, 0xf6, 0xf2, 0xf3, 0xf5,
        0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
        0x51, 0x52, 0xb9, 0xfb, 0xfc, 0xf9, 0xfa, 0xff,
        0x5c, 0xf7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0xb2, 0xd4, 0xd6, 0xd2, 0xd3, 0xd5,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0xb3, 0xdb, 0xdc, 0xd9, 0xda, 0x9f
 };
#endif                                  /* EBCDIC */
 
#if EBCDIC
#include <ctype.h>
 
int tonum(c)
int c;
{
#if SASC 
   const static char *alphanum = "0123456789abcdefghijklmnopqrstuvwxyz" ;
   char *where;
 
   where = memchr(alphanum, tolower(c), 36);
   if (where == 0) return -1;
   return where - alphanum;
#else                                   /* SASC */
   if(isdigit(c)) return (c - '0');
   if( (c | ' ') >= 'A' & (c | ' ') <= 'I') return(((c | ' ') - 'A') + 10);
   if( (c | ' ') >= 'J' & (c | ' ') <= 'R') return(((c | ' ') - 'J') + 19);
   if( (c | ' ') >= 'S' & (c | ' ') <= 'Z') return(((c | ' ') - 'S') + 28);
   return 0;
#endif                                  /* SASC */
}
#else                                   /* EBCDIC */
static char pjunk;                      /* avoid empty module */
#endif                                  /* EBCDIC */
