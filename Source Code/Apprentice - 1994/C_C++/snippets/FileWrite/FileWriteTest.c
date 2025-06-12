// File Write Test, Mac Version
//  requires 1.1 MB RAM
//  does not make use of Asynch Write w/ completion
//  06may94  bhamlin@netcom.com (Brian Hamlin)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <Files.h>

#define DEFAULT_DIR 0

#define MB         (1)
#define TEST_BLK     (0x100000)
#define MIN_WRITE_SIZE  (0x100)

//char memory[1024*1024];

main()
{
  register char i;
  register long j,n;
  //FILE *handle;
  double mytime, transfer;
  clock_t start, end;
  
  HParamBlockRec  PBHRec;
  OSErr      err;
  short      refNum;
  Ptr        mBuff;
  
  printf("Disk Test, Mac Version\n");  fflush( stdout);

  mBuff = NewPtr( TEST_BLK);
  if ( mBuff == 0 || MemError() )
  {
    printf("can't allocate buffer\n");
    exit(-1);
  }
  
  n = MIN_WRITE_SIZE;
  for(i=0; i<21-8; i++)
  {
    PBHRec.fileParam.ioCompletion = 0;
    PBHRec.fileParam.ioNamePtr = "\ptest.tmp";
    PBHRec.fileParam.ioVRefNum = DEFAULT_DIR;
    PBHRec.fileParam.ioDirID = DEFAULT_DIR;
    err = PBHCreate (&PBHRec, false);
    if ( err != 0  && err != dupFNErr)
    {  // ok if tmp file already exists
      printf("can't create file\n");
      exit(-1);
    }

        // if((handle = fopen("test.tmp","wb")) == NULL)

    // open file
    PBHRec.ioParam.ioCompletion = 0;
    PBHRec.ioParam.ioNamePtr = "\ptest.tmp";
    PBHRec.ioParam.ioVRefNum = DEFAULT_DIR;
    PBHRec.ioParam.ioPermssn = fsWrPerm;
    PBHRec.ioParam.ioMisc = 0;
    PBHRec.fileParam.ioDirID = DEFAULT_DIR;    // <- note union use
    err = PBHOpen( &PBHRec, false);
    if ( err != 0)
    {
      printf("can't open file\n");
      exit(-1);
    }
    refNum = PBHRec.ioParam.ioRefNum;
        
        // set-up for Write
    PBHRec.ioParam.ioCompletion = 0;
    PBHRec.ioParam.ioRefNum = refNum;
    PBHRec.ioParam.ioBuffer = mBuff;
    PBHRec.ioParam.ioReqCount = n;
    PBHRec.ioParam.ioPosMode = fsFromStart;
    PBHRec.ioParam.ioPosOffset = 0;

        //start = clock();
        start = TickCount();
    for ( j=0; j < (MB*TEST_BLK)/n; j++)
      err = PBWriteSync( (ParmBlkPtr)&PBHRec);
        
        //for(j=0; j < ((MB*1024*1024)/n); j++)
        //{
        //  fwrite(memory,sizeof(char),n,handle);
        //}
 
        end = TickCount();
        //end = clock();
        mytime = ((double)(end - start)) / ((double) 60);
        //mytime = ((double)(end - start)) / ((double) CLOCKS_PER_SEC);
        transfer = (double)(MB*1024)/mytime;
        printf("transferrate to harddisk = %f KByte/sec with blocksize %ld 
Bytes\n",transfer,n);

        //fclose(handle);
    PBHRec.ioParam.ioCompletion = 0;
    PBHRec.ioParam.ioRefNum = refNum;
    err = PBCloseSync( (ParmBlkPtr)&PBHRec);

        n = n * 2;
  }
}
