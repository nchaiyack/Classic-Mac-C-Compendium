/*
 * format of a file control block
 */

struct FCBrec_R {
  long fcbFlNm;
  char fcbFlags;
  char fcbTypByt;
  short fcbSBlk;
  long fcbEOF;
  long fcbPLen;
  long fcbCrPs;
  void *fcbVPtr;
  void *fcbBfAdr;
  short fcbFlPos;
  long fcbClmpSize;
  void *fcbBTCBPtr;
  long fcbExtRec;
  long fcbExtRec2;
  long fcbExtRec3;
  char fcbFType[4];
  long fcbCatPos;
  long fcbDirID;
  char fcbCName[32];
};
typedef struct FCBrec_R FCBrec,*FCBrec_pt;
