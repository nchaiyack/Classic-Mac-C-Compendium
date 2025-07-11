//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//|This is the interface definition for the Light3D class
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


#ifndef _Light3D_h
#define _Light3D_h

#include "Color3D.h"
#include "Vector3D.h"

class Scene3D;

class Light3D
{

  friend class Object3D;

public:

  Light3D(const Vector3D& newPos);
  Light3D(Vector3D *newPos);
  Light3D(const Vector3D& newPos, const Color3D& newColor);
  Light3D(Vector3D *newPos, Color3D *newColor);
  Light3D(const Vector3D& newPos, const Color3D& newColor, double newDiffuse);
  Light3D(Vector3D *newPos, Color3D *newColor, double newDiffuse);
  Light3D(const Light3D &l);
  Light3D(Light3D *l);
  void Initialize(const Vector3D& newPos, const Color3D& newColor, double newDiffuse);
 ~Light3D(void);

  double Diffuse(void);
  void SetPosition(const Vector3D& v);
  Vector3D &CGetPosition(void);
  inline Vector3D *GetPosition(void);
  void TransformPos(Matrix3D& transform);
  Vector3D &GetTransformedPos(void);

  void SetColor(const Color3D& color);
  inline Color3D &CGetColor(void);
  inline Color3D *GetColor(void);
  
  inline Real CalculateFlatRedIntensity(Real red, Real dot);
  inline Real CalculateFlatGreenIntensity(Real green, Real dot);
  inline Real CalculateFlatBlueIntensity(Real blue, Real dot);

  friend ostream &operator<<(ostream &o, Light3D &l);
  friend istream &operator >> (istream &in, Light3D &light);

private:

  void UpdateColorMultipliers(void);

  // The scene this light belongs to
  Scene3D *scene;

  Vector3D pos;
  Vector3D transformedPos;

  Color3D color;

  Real kDiffuse;

  Real RedColorMult;
  Real GreenColorMult;
  Real BlueColorMult;

};


#endif
