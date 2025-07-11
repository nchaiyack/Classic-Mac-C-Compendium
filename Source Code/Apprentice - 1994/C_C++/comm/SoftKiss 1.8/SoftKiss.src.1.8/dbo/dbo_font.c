/*
 * dbo_font.c - automaticly generated .c file
 * by Aaron Wohl
 * Public domain
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15208
 * home: 412-731-6159
 * work: 412-268-5032
 *
 * genfont.c generated at Wed May  6 00:17:47 1992
 *
 */

#ifdef DBO_ENABLED
#include "dbo_stdio.h"
#include "dbo_font.h"
#include <string.h>

unsigned char dbo_draw_font[DBO_CHARSET_SIZE][DBO_LINE_HEIGHT]={
 /* '^@' 00 */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '^A' 01 */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '^B' 02 */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '^C' 03 */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '^D' 04 */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '^E' 05 */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '^F' 06 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^G' 07 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^H' 08 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^I' 09 */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '^J' 0a */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^K' 0b */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^L' 0c */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^M' 0d */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '^N' 0e */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^O' 0f */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^P' 10 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^Q' 11 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^R' 12 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^S' 13 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^T' 14 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^U' 15 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^V' 16 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^W' 17 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^X' 18 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^Y' 19 */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^Z' 1a */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^[' 1b */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^\' 1c */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^]' 1d */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^^' 1e */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* '^_' 1f */  {0x00,0x1e,0x12,0x12,0x12,0x12,0x12,0x1e,0x00,0x00},
 /* space 20 */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '!' 21 */  {0x00,0x08,0x08,0x08,0x08,0x08,0x00,0x08,0x00,0x00},
 /* '"' 22 */  {0x00,0x14,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '#' 23 */  {0x00,0x14,0x3e,0x14,0x3e,0x14,0x00,0x00,0x00,0x00},
 /* '$' 24 */  {0x08,0x1c,0x2a,0x28,0x1c,0x0a,0x2a,0x1c,0x08,0x00},
 /* '%' 25 */  {0x00,0x1e,0x2a,0x2c,0x14,0x1a,0x2a,0x24,0x00,0x00},
 /* '&' 26 */  {0x00,0x18,0x24,0x28,0x10,0x2a,0x24,0x1a,0x00,0x00},
 /* ''' 27 */  {0x00,0x08,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '(' 28 */  {0x00,0x04,0x08,0x10,0x10,0x10,0x08,0x04,0x00,0x00},
 /* ')' 29 */  {0x00,0x08,0x04,0x02,0x02,0x02,0x04,0x08,0x00,0x00},
 /* '*' 2a */  {0x00,0x08,0x2a,0x1c,0x2a,0x08,0x00,0x00,0x00,0x00},
 /* '+' 2b */  {0x00,0x00,0x08,0x08,0x3e,0x08,0x08,0x00,0x00,0x00},
 /* ',' 2c */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x10},
 /* '-' 2d */  {0x00,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x00,0x00},
 /* '.' 2e */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00},
 /* '/' 2f */  {0x00,0x02,0x02,0x04,0x04,0x08,0x08,0x10,0x10,0x00},
 /* '0' 30 */  {0x00,0x1c,0x22,0x22,0x22,0x22,0x22,0x1c,0x00,0x00},
 /* '1' 31 */  {0x00,0x04,0x0c,0x04,0x04,0x04,0x04,0x04,0x00,0x00},
 /* '2' 32 */  {0x00,0x1c,0x22,0x02,0x04,0x08,0x10,0x3e,0x00,0x00},
 /* '3' 33 */  {0x00,0x1c,0x22,0x02,0x0c,0x02,0x22,0x1c,0x00,0x00},
 /* '4' 34 */  {0x00,0x04,0x0c,0x14,0x24,0x3e,0x04,0x04,0x00,0x00},
 /* '5' 35 */  {0x00,0x3e,0x20,0x3c,0x02,0x02,0x22,0x1c,0x00,0x00},
 /* '6' 36 */  {0x00,0x1c,0x20,0x3c,0x22,0x22,0x22,0x1c,0x00,0x00},
 /* '7' 37 */  {0x00,0x3e,0x02,0x02,0x04,0x08,0x08,0x08,0x00,0x00},
 /* '8' 38 */  {0x00,0x1c,0x22,0x22,0x1c,0x22,0x22,0x1c,0x00,0x00},
 /* '9' 39 */  {0x00,0x1c,0x22,0x22,0x22,0x1e,0x02,0x1c,0x00,0x00},
 /* ':' 3a */  {0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x08,0x00,0x00},
 /* ';' 3b */  {0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x08,0x08,0x10},
 /* '<' 3c */  {0x00,0x00,0x04,0x08,0x10,0x08,0x04,0x00,0x00,0x00},
 /* '=' 3d */  {0x00,0x00,0x00,0x3e,0x00,0x3e,0x00,0x00,0x00,0x00},
 /* '>' 3e */  {0x00,0x00,0x10,0x08,0x04,0x08,0x10,0x00,0x00,0x00},
 /* '?' 3f */  {0x00,0x1c,0x22,0x02,0x04,0x08,0x00,0x08,0x00,0x00},
 /* '@' 40 */  {0x00,0x1c,0x22,0x3a,0x2a,0x3c,0x20,0x1c,0x00,0x00},
 /* 'A' 41 */  {0x00,0x1c,0x22,0x22,0x3e,0x22,0x22,0x22,0x00,0x00},
 /* 'B' 42 */  {0x00,0x3c,0x22,0x22,0x3c,0x22,0x22,0x3c,0x00,0x00},
 /* 'C' 43 */  {0x00,0x1c,0x22,0x20,0x20,0x20,0x22,0x1c,0x00,0x00},
 /* 'D' 44 */  {0x00,0x3c,0x22,0x22,0x22,0x22,0x22,0x3c,0x00,0x00},
 /* 'E' 45 */  {0x00,0x3e,0x20,0x20,0x3c,0x20,0x20,0x3e,0x00,0x00},
 /* 'F' 46 */  {0x00,0x3e,0x20,0x20,0x3c,0x20,0x20,0x20,0x00,0x00},
 /* 'G' 47 */  {0x00,0x1c,0x22,0x20,0x26,0x22,0x22,0x1c,0x00,0x00},
 /* 'H' 48 */  {0x00,0x22,0x22,0x22,0x3e,0x22,0x22,0x22,0x00,0x00},
 /* 'I' 49 */  {0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x00,0x00},
 /* 'J' 4a */  {0x00,0x02,0x02,0x02,0x02,0x22,0x22,0x1c,0x00,0x00},
 /* 'K' 4b */  {0x00,0x22,0x24,0x28,0x30,0x28,0x24,0x22,0x00,0x00},
 /* 'L' 4c */  {0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x3e,0x00,0x00},
 /* 'M' 4d */  {0x00,0x22,0x36,0x2a,0x22,0x22,0x22,0x22,0x00,0x00},
 /* 'N' 4e */  {0x00,0x22,0x32,0x2a,0x26,0x22,0x22,0x22,0x00,0x00},
 /* 'O' 4f */  {0x00,0x1c,0x22,0x22,0x22,0x22,0x22,0x1c,0x00,0x00},
 /* 'P' 50 */  {0x00,0x3c,0x22,0x22,0x3c,0x20,0x20,0x20,0x00,0x00},
 /* 'Q' 51 */  {0x00,0x1c,0x22,0x22,0x22,0x22,0x22,0x1c,0x02,0x00},
 /* 'R' 52 */  {0x00,0x3c,0x22,0x22,0x3c,0x22,0x22,0x22,0x00,0x00},
 /* 'S' 53 */  {0x00,0x1c,0x22,0x20,0x1c,0x02,0x22,0x1c,0x00,0x00},
 /* 'T' 54 */  {0x00,0x3e,0x08,0x08,0x08,0x08,0x08,0x08,0x00,0x00},
 /* 'U' 55 */  {0x00,0x22,0x22,0x22,0x22,0x22,0x22,0x1c,0x00,0x00},
 /* 'V' 56 */  {0x00,0x22,0x22,0x22,0x22,0x22,0x14,0x08,0x00,0x00},
 /* 'W' 57 */  {0x00,0x22,0x22,0x22,0x22,0x2a,0x36,0x22,0x00,0x00},
 /* 'X' 58 */  {0x00,0x22,0x14,0x08,0x08,0x08,0x14,0x22,0x00,0x00},
 /* 'Y' 59 */  {0x00,0x22,0x22,0x22,0x14,0x08,0x08,0x08,0x00,0x00},
 /* 'Z' 5a */  {0x00,0x3e,0x02,0x04,0x08,0x10,0x20,0x3e,0x00,0x00},
 /* '[' 5b */  {0x00,0x0c,0x08,0x08,0x08,0x08,0x08,0x0c,0x00,0x00},
 /* '\' 5c */  {0x00,0x10,0x10,0x08,0x08,0x04,0x04,0x02,0x02,0x00},
 /* ']' 5d */  {0x00,0x0c,0x04,0x04,0x04,0x04,0x04,0x0c,0x00,0x00},
 /* '^' 5e */  {0x00,0x08,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* '_' 5f */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00},
 /* '`' 60 */  {0x00,0x08,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* 'a' 61 */  {0x00,0x00,0x00,0x1e,0x22,0x22,0x26,0x1a,0x00,0x00},
 /* 'b' 62 */  {0x00,0x20,0x20,0x3c,0x22,0x22,0x22,0x3c,0x00,0x00},
 /* 'c' 63 */  {0x00,0x00,0x00,0x1c,0x22,0x20,0x20,0x1e,0x00,0x00},
 /* 'd' 64 */  {0x00,0x02,0x02,0x1e,0x22,0x22,0x22,0x1e,0x00,0x00},
 /* 'e' 65 */  {0x00,0x00,0x00,0x1c,0x22,0x3e,0x20,0x1e,0x00,0x00},
 /* 'f' 66 */  {0x00,0x06,0x08,0x1c,0x08,0x08,0x08,0x08,0x00,0x00},
 /* 'g' 67 */  {0x00,0x00,0x00,0x1e,0x22,0x22,0x22,0x1e,0x02,0x1c},
 /* 'h' 68 */  {0x00,0x20,0x20,0x3c,0x22,0x22,0x22,0x22,0x00,0x00},
 /* 'i' 69 */  {0x00,0x08,0x00,0x08,0x08,0x08,0x08,0x08,0x00,0x00},
 /* 'j' 6a */  {0x00,0x08,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x30},
 /* 'k' 6b */  {0x00,0x20,0x20,0x24,0x28,0x38,0x24,0x22,0x00,0x00},
 /* 'l' 6c */  {0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x00,0x00},
 /* 'm' 6d */  {0x00,0x00,0x00,0x3c,0x2a,0x2a,0x2a,0x2a,0x00,0x00},
 /* 'n' 6e */  {0x00,0x00,0x00,0x2c,0x32,0x22,0x22,0x22,0x00,0x00},
 /* 'o' 6f */  {0x00,0x00,0x00,0x1c,0x22,0x22,0x22,0x1c,0x00,0x00},
 /* 'p' 70 */  {0x00,0x00,0x00,0x3c,0x22,0x22,0x22,0x3c,0x20,0x20},
 /* 'q' 71 */  {0x00,0x00,0x00,0x1e,0x22,0x22,0x22,0x1e,0x02,0x02},
 /* 'r' 72 */  {0x00,0x00,0x00,0x2c,0x32,0x20,0x20,0x20,0x00,0x00},
 /* 's' 73 */  {0x00,0x00,0x00,0x1e,0x20,0x1c,0x02,0x3c,0x00,0x00},
 /* 't' 74 */  {0x00,0x08,0x08,0x1c,0x08,0x08,0x08,0x06,0x00,0x00},
 /* 'u' 75 */  {0x00,0x00,0x00,0x22,0x22,0x22,0x26,0x1a,0x00,0x00},
 /* 'v' 76 */  {0x00,0x00,0x00,0x22,0x22,0x22,0x14,0x08,0x00,0x00},
 /* 'w' 77 */  {0x00,0x00,0x00,0x2a,0x2a,0x2a,0x2a,0x14,0x00,0x00},
 /* 'x' 78 */  {0x00,0x00,0x00,0x22,0x14,0x08,0x14,0x22,0x00,0x00},
 /* 'y' 79 */  {0x00,0x00,0x00,0x22,0x22,0x22,0x22,0x1e,0x02,0x1c},
 /* 'z' 7a */  {0x00,0x00,0x00,0x3e,0x04,0x08,0x10,0x3e,0x00,0x00},
 /* '{' 7b */  {0x02,0x04,0x04,0x04,0x08,0x04,0x04,0x04,0x02,0x00},
 /* '|' 7c */  {0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x00},
 /* '}' 7d */  {0x10,0x08,0x08,0x08,0x04,0x08,0x08,0x08,0x10,0x00},
 /* '~' 7e */  {0x00,0x1a,0x2c,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 /* delete 0x7f*/  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

#endif
