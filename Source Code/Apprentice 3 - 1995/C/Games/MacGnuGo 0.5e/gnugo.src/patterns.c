#include "comment.header"
/* revised 92Nov01 rhn */

#define EMPTY	0
#define MOVE	3
#define EDGE	4
#define PATNO	27	/* 30+1 - 4 (14, 18, 21, 29) */

/* pattern x, y coor and attribute */
/* for each pattern coordinate 0,0 must have my piece */
/* att = 0 - empty, 1 - your piece, 2 - my piece, 3 - my next move */
/* 4 - empty on edge, 5 - your piece on edge, 6 - my piece on edge */
/* 7 - not my piece, 8 - not your piece */
/*
  struct patval {int x, y, att;};
  */

/* patn - patern */
/* patlen - no. of pieces in pattern */
/* trfno - no. of transformation to match pattern */
/*	   8 for normal pattern, 4 for symmetrical pattern */
/* patwt - pattern value */
/*
  struct pattern {
  struct patval patn[MAXPC];
  int patlen, trfno, patwt;
  };
  */
static struct pattern pat[PATNO+8] = {
  /*
    pattern 0:
    232   connect if invaded
    010
    */
  {{{0, 0, 2},
    {2, 0, 2},
    {0, 1, EMPTY},
    {1, 1, 1},
    {2, 1, EMPTY},
    {1, 0, MOVE}}, 6, 4, 82},
  /*
    pattern 1:
    230   connect if invaded
    012
    */
  {{{0, 0, 2},
    {2, 0, EMPTY},
    {0, 1, EMPTY},
    {1, 1, 1},
    {2, 1, 2},
    {1, 0, MOVE}}, 6, 8, 84},
  /*
    pattern 2:
    212   connect if invaded
    838
    */
  {{{0, 0, 2},
    {2, 0, 2},
    {0, 1, 8},
    {1, 0, 1},
    {2, 1, 8},
    {1, 1, MOVE}}, 6, 4, 82},
  /*
    pattern 3:
    2302   connect if invaded
    7100
    */
  {{{0, 0, 2},
    {3, 0, 2},
    {0, 1, EMPTY},
    {1, 1, 1},
    {2, 0, 7},
    {2, 1, EMPTY},
    {1, 0, MOVE},
    {3, 1, EMPTY}},8, 8, 83},
  /*
    pattern 4:
    20302   connect if invaded
    80108
    */
  {{{0, 0, 2},
    {4, 0, 2},
    {0, 1, 8},
    {1, 0, EMPTY},
    {1, 1, EMPTY},
    {2, 1, 1},
    {3, 0, EMPTY},
    {3, 1, EMPTY},
    {2, 0, MOVE},
    {4, 1, 8}}, 10, 4, 84},
  /*
    pattern 5:
    203   form eye to protect
    021
    */
  {{{0, 0, 2},
    {0, 1, EMPTY},
    {1, 0, EMPTY},
    {1, 1, 2},
    {2, 1, 1},
    {2, 0, MOVE}}, 6, 8, 82},
  /*
    pattern 6:
    202    form eye to protect
    031
    */
  {{{0, 0, 2},
    {2, 0, 2},
    {0, 1, EMPTY},
    {1, 0, EMPTY},
    {2, 1, 1},
    {1, 1, MOVE}}, 6, 8, 82},
  /*
    pattern 7:
    230   connect if invaded
    102
    */
  {{{0, 0, 2},
    {2, 0, EMPTY},
    {0, 1, 1},
    {1, 1, EMPTY},
    {2, 1, 2},
    {1, 0, MOVE}}, 6, 8, 82},
  /*
    pattern 8:
    200000
     00030  extend
     00007
    */
  {{{0, 0, 2},
    {4, 2, EMPTY},
    {5, 0, EMPTY},
    {1, 0, EMPTY},
    {2, 0, EMPTY},
    {3, 0, EMPTY},
    {4, 0, EMPTY},
    {1, 1, EMPTY},
    {2, 1, EMPTY},
    {3, 1, EMPTY},
    {5, 1, EMPTY},
    {1, 2, EMPTY},
    {2, 2, EMPTY},
    {3, 2, EMPTY},
    {4, 1, MOVE},
    {5, 2, 7}}, 16, 8, 80},
  /*
    pattern 9:
    2
   000
   000  extend
   000
   030
   000
    */
  {{{ 0, 0, 2},
    {-1, 5, EMPTY},
    {-1, 1, EMPTY},
    { 0, 1, EMPTY},
    { 1, 1, EMPTY},
    {-1, 2, EMPTY},
    { 0, 2, EMPTY},
    { 1, 2, EMPTY},
    {-1, 3, EMPTY},
    { 0, 3, EMPTY},
    { 1, 3, EMPTY},
    {-1, 4, EMPTY},
    { 1, 4, EMPTY},
    { 0, 5, EMPTY},
    { 0, 4, MOVE},
    { 1, 5, EMPTY}}, 16, 4, 80},
  /*
    pattern 10:
    20000
     0030  extend
     0007
    */
  {{{0, 0, 2},
    {3, 2, EMPTY},
    {1, 0, EMPTY},
    {2, 0, EMPTY},
    {3, 0, EMPTY},
    {4, 0, EMPTY},
    {1, 1, EMPTY},
    {2, 1, EMPTY},
    {4, 1, EMPTY},
    {1, 2, EMPTY},
    {2, 2, EMPTY},
    {3, 1, MOVE},
    {4, 2, 7}}, 13, 8, 79},
  /*
    pattern 11:
     2
    000
    000  extend
    030
    000
    */
  {{{ 0, 0, 2},
    { 0, 4, EMPTY},
    {-1, 1, EMPTY},
    { 0, 1, EMPTY},
    { 1, 1, EMPTY},
    {-1, 2, EMPTY},
    { 0, 2, EMPTY},
    { 1, 2, EMPTY},
    {-1, 3, EMPTY},
    { 1, 3, EMPTY},
    {-1, 4, EMPTY},
    { 0, 3, MOVE},
    { 1, 4, EMPTY}}, 13, 4, 79},
  /*
    pattern 12:
    2000
     037  extend
     007
    */
  {{{0, 0, 2},
    {3, 1, 7},
    {1, 2, EMPTY},
    {1, 0, EMPTY},
    {2, 0, EMPTY},
    {1, 1, EMPTY},
    {2, 2, EMPTY},
    {3, 0, EMPTY},
    {2, 1, MOVE},
    {3, 2, 7}}, 10, 8, 76},
  /*
    pattern 13:
    2
   000  extend
   030
   707
    */
  {{{ 0, 0, 2},
    { 0, 3, EMPTY},
    {-1, 1, EMPTY},
    { 0, 1, EMPTY},
    { 1, 1, EMPTY},
    {-1, 2, EMPTY},
    { 1, 2, EMPTY},
    {-1, 3, 7},
    { 0, 2, MOVE},
    { 1, 3, 7}}, 10, 4, 76},
  /*
    pattern 15:
     202 	eye shape
    803
     2
    */
  {{{0, 0, 2},
    {1, 0, EMPTY},
    {2, 0, 2},
    {-1, 1, 8},
    {0, 1, EMPTY},
    {1, 1, MOVE},
    {0, 2, 2}}, 7, 8, 74},
  /*
    pattern 16:
    646    solidify eye on the edge
     237
     7
    */
  {{{0, 0, 6},
    {1, 0, EDGE},
    {1, 1, 2},
    {2, 0, 6},
    {2, 2, 1},
    {2, 1, MOVE},
    {3, 1, 7}}, 7, 8, 75},
  /*
    pattern 17:
     2	   form eye shape ???
    803
     2
    */
  {{{0, 0, 2},
    {-1, 1, 8},
    {0, 1, EMPTY},
    {0, 2, 2},
    {1, 1, MOVE}}, 5, 4, 80},
  /*
    pattern 19:
     2	   solidify eye on center
    202
    837
    */
  {{{0, 0, 2},
    {-1, 1, 2},
    {0, 1, EMPTY},
    {1, 1, 2},
    {0, 2, 8},
    {1, 2, MOVE},
    {2, 2, 7}}, 7, 8, 80},
  /*
    pattern 20:
    2	   solidify eye on center
   202
    230
    7
    */
  {{{0, 0, 2},
    {-1, 1, 2},
    {0, 1, EMPTY},
    {1, 1, 2},
    {0, 2, 2},
    {2, 2, EMPTY},
    {1, 2, MOVE},
    {1, 3, 7}}, 8, 8, 75},
  /*
    pattern 22:
     21		make wall ???
    031
    */
  {{{0, 0, 2},
    {1, 0, 1},
    {-1, 1, 0},
    {2, 1, 1},
    {0, 1, MOVE}}, 5, 8, 74},
  /*
    pattern 23:
    20108	 connect if invaded
    80302
    */
  {{{0, 0, 2},
    {4, 0, 8},
    {0, 1, 8},
    {1, 0, EMPTY},
    {1, 1, EMPTY},
    {2, 0, 1},
    {3, 0, EMPTY},
    {3, 1, EMPTY},
    {2, 1, MOVE},
    {4, 1, 2}}, 10, 8, 84},
  /*
    pattern 24:
    2100	connect if invaded
    0302
    */
  {{{0, 0, 2},
    {3, 0, EMPTY},
    {0, 1, EMPTY},
    {1, 0, 1},
    {2, 0, EMPTY},
    {2, 1, EMPTY},
    {1, 1, MOVE},
    {3, 1, 2}}, 8, 8, 83},
  /*
    pattern 25:
    2	block near edge ???
   031
   444
    */
  {{{0, 0, 2},
    {-1, 2, EDGE},
    {-1, 1, EMPTY},
    {1, 1, 1},
    {0, 2, EDGE},
    {0, 1, MOVE},
    {1, 2, EDGE}}, 7, 8, 76},
  /*
    pattern 26:
    21	block on edge	??? ???
    434
    */
  {{{0, 0, 2},
    {1, 0, 1},
    {0, 1, EDGE},
    {2, 1, EDGE},
    {1, 1, MOVE}}, 5, 8, 88},
  /*
    pattern 27:
    2	block on edge ??? ???
   435
    */
  {{{0, 0, 2},
    {-1, 1, EDGE},
    {1, 1, 5},
    {0, 1, MOVE}}, 4, 8, 88},
  /*
    pattern 28:
    214	block near corner
    374
    444
    */
  {{{0, 0, 2},
    {2, 1, EDGE},
    {1, 0, 1},
    {2, 0, EDGE},
    {1, 1, 7},
    {0, 2, EDGE},
    {1, 2, EDGE},
    {0, 1, MOVE},
    {2, 2, EDGE}}, 9, 8, 82},
  /*
    pattern 30:
    231		prevent a cut from isolating a stone
    120
     07
    */
  {{{0, 0, 2},
    {2, 2, 7},
    {2, 0, 1},
    {0, 1, 1},
    {1, 1, 2},
    {1, 2, 0},
    {2, 1, 0},
    {1, 0, MOVE}}, 8, 8, 82}
};

/* att = 0 - empty, 1 - your piece, 2 - my piece, 3 - my next move */
/* 4 - empty on edge, 5 - your piece on edge, 6 - my piece on edge */
/* 7 - not my piece (0 || 1), 8 - not your piece (0 || 2) */
/* end patterns */
