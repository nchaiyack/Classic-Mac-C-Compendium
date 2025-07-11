#include "comment.header"
/* */
#include <stdio.h>
extern int hcap;
extern int pass;
extern int MAXX, MAXY;
extern int blackTerritory, whiteCaptured, whiteTerritory, blackCaptured;

#define WHITE 1
#define BLACK 2
extern int lastMove;

int saveSmartGoFile(const char *fileName)
{
  FILE *NGoFile;
  int i,j;
  int x,y,color;

  if ((NGoFile = fopen(fileName, "w")) == NULL) return 1;

  fprintf(NGoFile, "(\n;\nGaMe[1]\nVieW[]\n");
  fprintf(NGoFile, "SiZe[%d]\nKoMi[%3.1f]\nHAndicap[%d]",
	    MAXX, (float)0.0, hcap);
  fprintf(NGoFile,"\nComment[ A game between ");
  fprintf(NGoFile,"the computer and a human player.\n\n");

  if (pass >= 2) {
    i = (blackTerritory + whiteCaptured - (whiteTerritory + blackCaptured));
    fprintf(NGoFile,"        Result:  %s wins by %8.1f.]\n",
	      (i > 0) ? "Black":"White", (float)((i > 0) ? i : -i));
  } else fprintf(NGoFile, "]\n");

#ifdef hcapSaveModifications
  if (hcap > 1) {
      int j, x, y;
      fprintf(NGoFile, "AddBlack");
      for (j=0;j<handicapNumber;j++) {
        x = handicapMoves[j][0];
        y = handicapMoves[j][1];
        fprintf(NGoFile,"[%c%c]", 'a'+x, 'a'+y); 
      }
      fprintf(NGoFile, "\n");
  }
#endif
    
  for (i = 0; i < lastMove; i++) {
    getHistory(i, &color, &x, &y);
    if (color == WHITE)      fprintf(NGoFile, ";\nWhite");
    else if (color == BLACK) fprintf(NGoFile, ";\nBlack");
    else continue;
    if (x < 0) fprintf(NGoFile, "[tt]\n");
    else fprintf(NGoFile,"[%c%c]\n",
		 y + 'a', x + 'a');
  }

  fprintf(NGoFile,")\n\n");
  fclose(NGoFile);

  return 0;
}
