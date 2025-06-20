//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This is the interface definition of the Matrix3D class
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

#ifndef _Matrix_h
#define _Matrix_h

#include "Types3D.h"

class Vector3D;
class ostream;


class Matrix3D {

  friend class Vector3D;
  friend class View3D;  // TEMPORARY
  
public:

  Matrix3D(void);
  Matrix3D(const Matrix3D &M);
  Matrix3D(const Vector3D &row0, const Vector3D &row1,
           const Vector3D &row2, const Vector3D &row3);
  ~Matrix3D(void);

  inline void Zero(void);
  inline void Identity(void);

  inline void Translate(const Vector3D &translation);
  inline void AddTranslate(const Vector3D &translation);
  inline void Scale(const Vector3D &scaling);
  inline void AddScale(const Vector3D &scaling);
  inline void Rotate(const Vector3D &rotation);
  inline void RotateAroundX(Real theta);
  inline void RotateAroundY(Real theta);
  inline void RotateAroundZ(Real theta);
  inline void AddRotateAroundX(Real theta);
  inline void AddRotateAroundY(Real theta);
  inline void AddRotateAroundZ(Real theta);

  void Rotate(const Vector3D &heading, const Vector3D &up);
  void Rotate(const Vector3D &heading, Real twist);
  void RotateAroundVector(const Vector3D &v, Real theta);

  Real Determinant();
  Flag Invert();
  Flag InvertTo(Matrix3D &result);
  
  void Translation(Vector3D &translation);
  void ToExplicit(Vector3D &scale, Vector3D &rotation,
                  Vector3D &shear, Vector3D &translation);
  void FromExplicit(const Vector3D &scale, const Vector3D &rotation,
                    const Vector3D &translation);
  void FromExplicit(const Vector3D &scale,
                    const Vector3D &heading, const Vector3D &up,
                    const Vector3D &translation);
  void FromExplicit(const Vector3D &scale,
                    const Vector3D &heading, Real twist,
                    const Vector3D &translation);

  inline Matrix3D &operator=(const Matrix3D &M);

  Matrix3D operator*(const Matrix3D &M) const;
  Matrix3D operator*=(const Matrix3D &M);


  friend ostream &operator<<(ostream &o, const Matrix3D &M);

private:

  Real m[4][3];

};


#endif


