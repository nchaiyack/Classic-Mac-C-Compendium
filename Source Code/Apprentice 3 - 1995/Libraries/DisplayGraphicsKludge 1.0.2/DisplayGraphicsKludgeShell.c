	/************************************************************************/
	/*																		*/
	/*							DisplayGraphicsKludge						*/
	/*																		*/
	/*		This is a sample program that calls DisplayGraphicsKludge.		*/
	/*																		*/
	/************************************************************************/

#include	<stdio.h>

	/************************************************************************/
	/*																		*/
	/*		If you're using Think C and have the prefix set to 				*/
	/*			#include <Macheaders.c>										*/
	/*	you can delete the following #include								*/
	/*																		*/
	/************************************************************************/
#include	"Memory.h"


main()
{
#define				PixelsPerRow	256
#define				NumRows			256

char				*myDataPtr;
long				NumBytes;
long				column;
long				row;
char				theChar;

NumBytes = (long)PixelsPerRow * (long)NumRows;
myDataPtr = NewPtr(NumBytes);

if (myDataPtr == nil)
	printf("Not Enough Memory!\n\n");
else
  {
    printf("... normally this is printf & scanf output/input ...\n");
    printf("... normally this is printf & scanf output/input ...\n\n");
    printf("Press return to display an image where each row gets darker... ");
    theChar = getchar();

	for (row = 0; row < NumRows; row++)
		for (column = 0; column < PixelsPerRow; column++)
			*(myDataPtr + column + (row * PixelsPerRow)) = 255 - row;					/* each row is a darker shade				*/

    DisplayGraphicsKludge("\p256x256 Darker Rows", (Ptr)myDataPtr, PixelsPerRow, NumRows);

    printf("\n...Now you're back in your code...\n\n");
    printf("Press return to display an image where each column gets darker... ");
    theChar = getchar();
    
	for (row = 0; row < NumRows; row++)
		for (column = 0; column < PixelsPerRow; column++)
			*(myDataPtr + column + (row * PixelsPerRow)) = 255 - column;				/* each column is a darker shade			*/

    DisplayGraphicsKludge("\p256x256 Darker Cols", (Ptr)myDataPtr, PixelsPerRow, NumRows);

    printf("\n...Now you're back in your code...\n\n");
  }
printf("\nPress return to exit this demo...");

}

