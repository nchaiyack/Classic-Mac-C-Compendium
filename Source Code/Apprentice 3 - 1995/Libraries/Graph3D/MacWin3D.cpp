//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| MacWin3D.cpp
//|
//| This implements the code for the Macintosh port of this
//| library.
//|
//| Created: Maarten Gribnau 10-05-1995
//|
//| Still under construction.
//| Problems with memory allocation, seems like memory is not
//| correctly released.
//| Windows still must have a width that is a multiple of 16.
//|________________________________________________________________

#include "MacWin3D.h"

#include <stdlib.h>
#include "Macros3D.h"




//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| MacWindow3D::MacWindow3D
//|
//| Purpose: This method creates a Macintosh window.
//|
//| Parameters: w, h: the size of the window
//|
//| Created by Greg Ferrar, February 13, 1995
//|
//| Modified: Maarten Gribnau: Rewrote the whole thing
//|________________________________________________________________

MacWindow3D::MacWindow3D(int w, int h)
           : Window3D(w, h)
{

  // Make width a multiple of four (See documentation on CopyBits)
  long rowBytes = (((PIXEL_BYTES*8) * w + 31) / 32) * 4;
  width  = rowBytes / PIXEL_BYTES;
  
  //  Create the rectangle for the new window.
  posX = 0;
  posY = 40;
  Rect windowRect;
  SetRect(&windowRect, posX, posY, posX+width, posY+height);

  //  Create the window
  grafPtr = (CGrafPtr)NewCWindow(nil,              // Create new CWindowRecord
                                 &windowRect,      // With bounds calculated
                                 "\pGraph3D",      // Give it a name
                                 TRUE,             // Make it visible
                                 0,                // Looks like a document (0=documentProc)
                                 (WindowPtr) -1L,  // Put it in front
                                 FALSE,            // Has close box
                                 0);               // No special references
  SetRect(&destRect, 0, 0, width, height);
  // Set local coordinates for the bitmap
  posX = 0;
  posY = 0;
  //  Initialize the bitmap
  InitBitmap();
  
  //  Initialize the Z buffer
  InitZBuffer();
  
  ownsWindow = TRUE;
  
}  //==== MacWindow3D::MacWindow3D() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| MacWindow3D::MacWindow3D
//|
//| Purpose: This method creates sets up a MacWindow3D record for
//|          display in a toolbox CGrafPort.
//|          Sets up the offscreen bitmap with dimensions as close as
//|          possible to the destination rectangle.
//|
//| Parameters: theGrafPtr: Pointer to the Mac CGrafPort
//|             bounds:     CopyBits destination rectangle in the CGrafPort
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|________________________________________________________________

MacWindow3D::MacWindow3D(CGrafPtr theGrafPtr, Rect *bounds)
           : Window3D(bounds->right-bounds->left, bounds->bottom-bounds->top)
{

  grafPtr = theGrafPtr;
  destRect = *bounds;
  
  posX   = bounds->left;
  posY   = bounds->top;

  // Make width a multiple of four (See documentation on CopyBits)
  long rowBytes = (((PIXEL_BYTES*8) * width + 31) / 32) * 4;
  width = rowBytes / PIXEL_BYTES;
  
  //  Initialize the bitmap
  InitBitmap();
  
  //  Initialize the Z buffer
  InitZBuffer();
  
  ownsWindow = FALSE;

}  //==== MacWindow3D::MacWindow3D() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| MacWindow3D::~MacWindow3D
//|
//| Purpose: This method disposes of a Macintosh window.
//|
//| Parameters: none
//|
//| Created by Greg Ferrar, February 13, 1995
//|
//| Modified: Maarten Gribnau: Since allocation is local, made
//|           deallocation local too.
//|________________________________________________________________

MacWindow3D::~MacWindow3D(void)
{
  if (pixelsBase != NULL) {
    //DisposPtr((Ptr) bitmapBase);
    delete [] pixelsBase;
    //free((void *) pixmap.baseAddr);
    pixelsBase      = NULL;
    pixmap.baseAddr = NULL;
  }

  if (zBuffer != NULL) {
    delete [] zBuffer;
    //free((void *) zBuffer);
    //DisposPtr((Ptr) zBuffer);
    zBuffer = NULL;
  }

  if (ownsWindow) {
  	DisposeWindow((WindowPtr) grafPtr);
  }

}  //==== MacWindow3D::MacWindow3D() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| MacWindow3D::InitBitmap
//|
//| Purpose: This method initializes the bitmap.
//|          Create a QD PixMap from scratch, same size as CGrafPort.
//|          members bitmapBase, bytesPerLine are updated.
//|
//| Parameters: none
//|
//| Created by Maarten Gribnau
//|________________________________________________________________

void MacWindow3D::InitBitmap(void)
{
  Rect  bounds;
  short rowBytes;
  
  Window3D::InitBitmap();
  
  pixmap.baseAddr = (Ptr) pixelsBase;	
  
  // Set the rowBytes high bytes
  rowBytes         = pixelsInc * PIXEL_BYTES;
  rowBytes        &= 0x1F00;    // Mask off 3 highest bits
  pixmap.rowBytes  = 0x8000;    // Set high bit
  pixmap.rowBytes |= rowBytes;  // Combine result
  
  SetRect(&bounds, 0, 0, width, height);  
  pixmap.bounds     = bounds;
  
  pixmap.pmVersion  = 0;             // Not used
  pixmap.packType   = 0;             // No packing
  pixmap.packSize   = 0;             // So no pack size
  pixmap.hRes       = 4718592;       // Default horizontal screen resolution
  pixmap.vRes       = 4718592;       // For vertical too
  pixmap.pixelType  = RGBDirect;     // No indexed colors
  pixmap.pixelSize  = PIXEL_BYTES*8; // 16, 24 or 32
  pixmap.cmpCount   = 3;             // Color components per pixel
  pixmap.cmpSize    = 5;             // Logical bits per component
  pixmap.planeBytes = 0;             // Not used
  pixmap.pmTable    = nil;           // No color table
  pixmap.pmReserved = 0;             // Not used

}


#if 0
//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| MacWindow3D::InitZBuffer
//|
//| Purpose: This method initializes the z buffer.
//|
//| Parameters: w, h: width and height of the bitmap
//|
//| Created by Maarten Gribnau
//|________________________________________________________________

void MacWindow3D::InitZBuffer(void)
{
  //size_t bytes;
  //Size bytes;

  // Allocate a chunk of memory for the z buffer
  zBuffer = new Real[width * height];
  //bytes   = width * height;
  //bytes  *= sizeof(Real);
  //zBuffer = (Real *) malloc(bytes);
  //zBuffer = (Real *) NewPtr(bytes);
  
}  //==== Window3D::InitZBuffer() ====
#endif


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| MacWindow3D::UpdateWindowFromBitmap
//|
//| Purpose: This method updates the contents of the window from
//|          the bitmap.
//|
//| Parameters: none
//|
//| Created by Greg Ferrar, February 13, 1995
//|
//| Modified: Maarten Gribnau: modified is an understatement
//|             Uses copybits now to support all screens.
//|             Screen depth equal too pixel size is optimal
//|             offcause.
//|________________________________________________________________

void MacWindow3D::UpdateWindowFromBitmap()
{
  Rect     srcRect;
  BitMap  *srcBmp;
  BitMap  *dstBmp;
  RGBColor col;
  GrafPtr  savePort;

  SetRect(&srcRect, posX, posY, width, height);
  srcBmp = (BitMap *) (&pixmap);
  dstBmp = (BitMap *) (*grafPtr->portPixMap);

  GetPort(&savePort);
  SetPort((GrafPtr) grafPtr);
  col.red = col.green = col.blue = 0x0000;
  RGBForeColor(&col);
  col.red = col.green = col.blue = 0xFFFF;
  RGBBackColor(&col);
  
  CopyBits(srcBmp,    // From the off-screen
           dstBmp,    // to the window
           &srcRect,  // Copy from this rect in the off-screen bitmap
           &destRect, // to this in the window's GrafPort
           srcCopy,   // I said copy
           nil);      // with no masks please

  SetPort(savePort);
  
}  //==== MacWindow3D::UpdateWindowFromBitmap() ====



#ifdef DIRECT_SCREEN

  // This works for 16 bit screens only! 
  
#ifdef PIXEL_IS_24BIT
  Byte   r, g, b;
  unsigned short color;
  PIXEL *srcPtr;
    
  unsigned short* dstPtr;
  unsigned short* dstLinePtr;  
  unsigned short  dstLineInc;
  
  register pixelCounter;
  register lineCounter = height;
  
  srcPtr     = bitmapBase;
  dstLinePtr = (unsigned short *) (*(grafPtr->portPixMap))->baseAddr;
  dstLineInc = (*(grafPtr->portPixMap))->rowBytes & 0x7FFF;
  dstLineInc /= sizeof(unsigned short);
  dstLinePtr += V_WINDOW_OFFSET * dstLineInc;
  dstLinePtr += H_WINDOW_OFFSET;
  
  double temp; // DEBUG
  while (lineCounter--) {

    dstPtr = dstLinePtr;  
    pixelCounter = width;
    //temp = ((float)lineCounter * 255.0) / (float)height;
    //r = (Byte) temp;
    //g = 0;
    //b = 0;
    //r >>= 3;
    //g >>= 3;
    //b >>= 3;
    
    while (pixelCounter--) {
      
      // Extract color components
      r = *(srcPtr++);
      g = *(srcPtr++);
      b = *(srcPtr++);
      
      // Truncate components
      r >>= 3;
      g >>= 3;
      b >>= 3;
      
      // Combine components into short
      color = r;
      color <<= 5;
      color |= g;
      color <<= 5;
      color |= b;
      
      *(dstPtr++) = color;
    }
    dstLinePtr += dstLineInc;
  }
  
#else
#ifdef PIXEL_IS_16BIT

//#define DEBUG
#ifdef DEBUG
  Byte   r, g, b;  // DEBUG
  short color;  // DEBUG
  double temp; // DEBUG
#endif

  PIXEL *srcPtr;    
  PIXEL *dstPtr;
  unsigned short* dstLinePtr;  
  unsigned short  dstLineInc;
  
  register pixelCounter;
  register lineCounter = height;
  
  srcPtr     = bitmapBase;
  dstLinePtr = (unsigned short *) (*(grafPtr->portPixMap))->baseAddr;
  dstLineInc = (*(grafPtr->portPixMap))->rowBytes & 0x7FFF;
  dstLineInc /= sizeof(unsigned short);
  dstLinePtr += V_WINDOW_OFFSET * dstLineInc;
  dstLinePtr += H_WINDOW_OFFSET;
  
  while (lineCounter--) {

#ifdef DEBUG
    temp = ((float)lineCounter * 255.0) / (float)height;
    r = (Byte) temp;
    g = 0;
    b = 0;
    r >>= 3;
    g >>= 3;
    b >>= 3;
    color = r;
    color <<= 5;
    color |= g;
    color <<= 5;
    color |= b;
#endif
   
    dstPtr = dstLinePtr;  
    pixelCounter = width;

    while (pixelCounter--) {
      
      // Combine components into short      
#ifdef DEBUG
      *(dstPtr++) = color;
#else
      *(dstPtr++) = *(srcPtr++);
#endif
    }
    dstLinePtr += dstLineInc;
  }

#endif
#endif
 
#endif // DIRECT_SCREEN