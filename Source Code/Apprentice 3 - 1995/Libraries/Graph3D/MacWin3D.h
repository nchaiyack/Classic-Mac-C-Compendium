//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| MacWindow3D.h
//|
//| Maarten Gribnau 10-05-1995
//|
//| Interface for Mac version of Window3D, see file MacWin3D.cpp
//| for details.
//|________________________________________________________________


#ifndef _MacWindow3D_h
#define _MacWindow3D_h

#include "Window3D.h"

#include <Windows.h>


class MacWindow3D : public Window3D
{

public:

  MacWindow3D(int w, int h);
  MacWindow3D(CGrafPtr theGrafPtr, Rect *bounds);
  virtual ~MacWindow3D(void);

  virtual void InitBitmap(void);
  //virtual void InitZBuffer(void);
  virtual void UpdateWindowFromBitmap(void);

private:

  short    posX, posY;
  Rect     destRect;
  CGrafPtr grafPtr;
  PixMap   pixmap;
  Flag     ownsWindow;

};



#endif
