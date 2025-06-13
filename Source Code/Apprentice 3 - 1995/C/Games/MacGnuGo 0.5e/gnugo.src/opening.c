#include "comment.header"

extern long int rd;
extern int MAXX, MAXY;
extern void uRandom(long int*);

int opening(int *i, int *j, int *cnd, int type)
     /* get move for opening from game tree */
{
  struct tnode {
    int i, j, ndct, next[8];
  };
  
  static struct tnode tree[] = {
    {-1, -1, 8, { 1, 2, 3, 4, 5, 6, 7, 20}},	/* 0 */
    {2, 3, 2, { 8, 9}},
    {2, 4, 1, {10}},
    {3, 2, 2, {11, 12}},
    {3, 3, 6, {14, 15, 16, 17, 18, 19}},
    {3, 4, 1, {10}},  /* 5 */
    {4, 2, 1, {13}},
    {4, 3, 1, {13}},
    {4, 2, 0},
    {4, 3, 0},
    {3, 2, 0},  /* 10 */
    {2, 4, 0},
    {3, 4, 0},
    {2, 3, 0},
    {2, 5, 1, {10}},
    {2, 6, 1, {10}},  /* 15 */
    {3, 5, 1, {10}},
    {5, 2, 1, {13}},
    {5, 3, 1, {13}},
    {6, 2, 1, {13}},
    {2, 2, 0}  /* 20 */
  };
  int m;
  
  /* get i, j */
  if ((type == 1) || (type == 3))
    *i = ((MAXX-1) - tree[*cnd].i)  ;   /* inverted */
  else
    *i = tree[*cnd].i  ;
  if ((type == 2) || (type == 3))
    *j = ((MAXY-1) - tree[*cnd].j)  ;   /* reflected */
  else
    *j = tree[*cnd].j  ;
  if (tree[*cnd].ndct)  /* more move */
    {
      uRandom(&rd);
      m = rd % tree[*cnd].ndct;  /* select move */
      *cnd = tree[*cnd].next[m];	/* new	current node */
      return 1;
    }
  else
    return 0;
}  /* end opening */

