//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This is the interface definition for the Color3D class
//|
//| This code was created by Greg Ferrar, based on original code
//| created by Jeff Palmer.  There are no restrictions on the use
//| or distribution of this library, except that you may not
//| restrict its use or distribution.  Any project which is
//| created from this code, or any significant modification of
//| this code, may be distributed in any way you choose; you may
//| even sell it or its source code.  However, any limitations
//| placed on the distribution or use of such a project or
//| modification may not in any way limit the distribution or use
//| of this original code.
//|________________________________________________________________


#ifndef _Color3D_h
#define _Color3D_h

#include "Types3D.h"

class ostream;
class istream;

class Color3D {

  friend class Vertex3D;		// Vertex3D needs to access the Real color components directly
  friend class Light3D;			// Light3D needs to access the Real color components directly
  friend class View3D;			// View3D needs to access the Real color components directly
  friend class Polygon3D;		// Polygon3D needs to access the Real color components directly
  friend class Object3D;		// Object3D needs to access the Real color components directly
  friend class Window3D;

public:

  Color3D(void);
  Color3D(double newR, double newG, double newB);
  Color3D(const Color3D &c);
 ~Color3D(void);

  inline Real R(void) const;
  inline Real G(void) const;
  inline Real B(void) const;
  inline double RDouble(void) const;
  inline double GDouble(void) const;
  inline double BDouble(void) const;

//  inline void SetR(Real newR);
//  inline void SetG(Real newG);
//  inline void SetB(Real newB);
  inline void SetRDouble(double newR);
  inline void SetGDouble(double newG);
  inline void SetBDouble(double newB);

  inline Color3D &operator=(const Color3D &c);

  friend ostream& operator<<(ostream &o, const Color3D &color);
  friend istream& operator>>(istream &i, Color3D &color);

protected:

  inline ostream &Print(ostream &o);

private:

  Real r;		// The color components
  Real g;
  Real b;

};


#endif
