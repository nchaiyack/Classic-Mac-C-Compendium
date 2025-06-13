#include "comment.header"

extern unsigned char p[19][19];
extern int MAXX, MAXY;
extern int currentStone;

int fioe(int i, int j)
{
  /* check top edge */
  if (i == 0)
    {
      if ((j == 0) && ((p[1][0] == currentStone) && (p[0][1] == currentStone))) return 1;
      if ((j == MAXY - 1) && ((p[1][MAXY - 1] == currentStone) && (p[0][MAXY - 2] == currentStone))) return 1;
      if ((p[1][j] == currentStone) &&
	  ((p[0][j - 1] == currentStone) && (p[0][j + 1] == currentStone))) return 1;
      else
	return 0;
    }
  /* check bottom edge */
  if (i == MAXX - 1)
    {
      if ((j == 0) && ((p[MAXX - 2][0] == currentStone) && (p[MAXX - 1][1] == currentStone))) return 1;
      if ((j == MAXY - 1) && ((p[MAXX - 2][MAXY - 1] == currentStone) && (p[MAXX - 1][MAXY - 2] == currentStone))) return 1;
      if ((p[MAXX - 2][j] == currentStone) &&
	  ((p[MAXX - 1][j - 1] == currentStone) && (p[MAXX - 1][j + 1] == currentStone)))
	return 1;
      else
	return 0;
    }
  /* check left edge */
  if (j == 0)
    if ((p[i][1] == currentStone) &&
	((p[i - 1] [0] == currentStone) && (p[i + 1][0] == currentStone)))
      return 1;
    else
      return 0;
  /* check right edge */
  if (j == MAXY - 1)
    if ((p[i][MAXY - 2] == currentStone) &&
	((p[i - 1][MAXY - 1] == currentStone) && (p[i + 1][MAXY - 1] == currentStone)))
      return 1;
    else
      return 0;
  /* check center pieces */
  if (((p[i][j - 1] == currentStone) && (p[i][j + 1] == currentStone)) &&
      ((p[i - 1][j] == currentStone) && (p[i + 1][j] == currentStone)))
    return 1;
  else
    return 0;
}  /* fioe */
