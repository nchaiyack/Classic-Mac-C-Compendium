//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This is the implementation file for the Matrix3D class
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

#include "Matrix3D.hpp"

#include <iostream.h>
#include "Util3D.hpp"
#include "Vector3D.hpp"



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::Matrix3D
//|
//| Purpose: This method creates an identity Matrix3D
//|
//| Parameters: none
//|
//| Created by Jeff Palmer
//|
//| Modified:
//|   26-04-1995 by Maarten Gribnau
//|     Made it identity matrix
//|___________________________________________________________________

Matrix3D::Matrix3D(void)
{

  Zero();			// Clear all entries to 0
  Identity();

}  //==== Matrix3D::Matrix3D() =====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::Matrix3D
//|
//| Purpose: This method creates a Matrix3D by copying another
//|
//| Parameters: copy: the matrix to copy
//|
//| Created by Jeff Palmer
//|
//| Modified:
//|___________________________________________________________________

Matrix3D::Matrix3D(const Matrix3D& copy)
{

  *this = copy;			// Copy the matrix

}  //==== Matrix3D::Matrix3D() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::Matrix3D
//|
//| Purpose: This method creates a Matrix3D from four row vectors
//|
//| Parameters: row..: column vectors
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Matrix3D::Matrix3D(const Vector3D &row0, const Vector3D &row1,
                   const Vector3D &row2, const Vector3D &row3)
{

  m[0][0] = row0.x;
  m[0][1] = row0.y;
  m[0][2] = row0.z;
  m[1][0] = row1.x;
  m[1][1] = row1.y;
  m[1][2] = row1.z;
  m[2][0] = row2.x;
  m[2][1] = row2.y;
  m[2][2] = row2.z;
  m[3][0] = row3.x;
  m[3][1] = row3.y;
  m[3][2] = row3.z;

}  //==== Matrix3D::Matrix3D() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::~Matrix3D
//|
//| Purpose: destructor
//|
//| Parameters: none
//|
//| Created by Jeff Palmer
//|
//| Modified:
//|___________________________________________________________________

Matrix3D::~Matrix3D(void)
{

  
}  //==== Matrix3D::~Matrix3D() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::Rotate
//|
//| Purpose: Creates a rotation matrix that rotates the x-axis to the
//|          heading and a point in the plane x=0 to the up vector.          vectors.
//|          For an explanation on the algorithm take Computer
//|          Graphics, principles and practice, 2nd edition, pages
//|          217, 222 (inverted, changed to x-axis).
//|
//| Parameters: heading: where the x-axis lands:
//|             up:      defines angle of rotation around x-axis
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

void Matrix3D::Rotate(const Vector3D &heading, const Vector3D &up)
{

  Zero();
    
  Vector3D Rx(heading);
  Rx.Normalize();
  m[0][0] = Rx.x;
  m[0][1] = Rx.y;
  m[0][2] = Rx.z;

  Vector3D Rz(heading);
  Rz.Cross(up);
  Rz.Normalize();
  m[2][0] = Rz.x;
  m[2][1] = Rz.y;
  m[2][2] = Rz.z;
  
  Vector3D Ry(Rz);
  Ry.Cross(Rx);  
  m[1][0] = Ry.x;
  m[1][1] = Ry.y;
  m[1][2] = Ry.z;

}  //==== Matrix3D::Rotate ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::Rotate
//|
//| Purpose: Creates a rotation matrix that rotates twist radians 
//|          around the x-axisand than rotates the x-axis to the 
//|          heading.
//|          Modified version of the previous method.
//|
//| Parameters: heading: where the x-axis lands:
//|             twist:   amount of rotation around x-axis
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

void Matrix3D::Rotate(const Vector3D &heading, Real twist)
{

  // Create the twist rotation matrix
  Matrix3D rotTwist;
  rotTwist.RotateAroundX(twist);

  Real dxy = REAL_SQRT(REAL_MULT(heading.x, heading.x) +
                       REAL_MULT(heading.y, heading.y));
  Real dxyz = heading.Magnitude();
  Real sinTemp = 0;
  Real cosTemp = REAL_ONE;

  // Create the z-axis rotation matrix
  Matrix3D rotZ;
  if (dxy)
    {
    sinTemp = REAL_DIV(heading.y, dxy);
    cosTemp = REAL_DIV(heading.x, dxy);
    rotZ.m[0][0] = cosTemp;
    rotZ.m[0][1] = sinTemp;
    rotZ.m[1][0] = -sinTemp;
    rotZ.m[1][1] = cosTemp;
    }
  
  // Create the y-axis rotation matrix
  Matrix3D rotY;
  Real tempX = REAL_MULT(cosTemp, heading.x) + 
               REAL_MULT(sinTemp, heading.y);
  if (dxyz)
    {
    sinTemp = REAL_DIV(-heading.z, dxyz);
    cosTemp = REAL_DIV(tempX, dxyz);
    rotY.m[0][0] = cosTemp;
    rotY.m[0][2] = -sinTemp;
    rotY.m[2][0] = sinTemp;
    rotY.m[2][2] = cosTemp;
    }
  
  *this = rotTwist * rotY * rotZ;

}  //==== Matrix3D::Rotate ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::RotateAroundVector
//|
//| Purpose: This sets this matrix to a rotation transformation
//|          matrix, which rotates around a vector.
//|          Algorithm explained in CAD/CAM, Theory and Practice pages
//|          495-497.
//|
//| Parameters: v:     vector to rotate about
//|             theta: angle to rotate around vector, counterclockwise.
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

void Matrix3D::RotateAroundVector(const Vector3D &v, Real theta)
{

	Vector3D direction(v);

	// Normalize the rotation axis
	direction.Normalize();
	Real nx = direction.x;
	Real ny = direction.y;
	Real nz = direction.z;
	
	// Save some calculations (costs some memory though)
	Real st  = REAL_SIN(theta);
	Real ct  = REAL_COS(theta);
	Real vt  = REAL_ONE - ct;
	
	// Construct the matrix
	m[0][0] = REAL_MULT(REAL_MULT(nx, nx), vt) + ct;
	m[0][1] = REAL_MULT(REAL_MULT(nx, ny), vt) - REAL_MULT(nz, st);                                                                                                                                                                                                              
	m[0][2] = REAL_MULT(REAL_MULT(nx,nz), vt) + REAL_MULT(ny, st);
	
	m[1][0] = REAL_MULT(REAL_MULT(nx, ny), vt) + REAL_MULT(nz, st);
	m[1][1] = REAL_MULT(REAL_MULT(ny, ny), vt) + ct;
	m[1][2] = REAL_MULT(REAL_MULT(ny, nz), vt) - REAL_MULT(nx, st);

	m[2][0] = REAL_MULT(REAL_MULT(nx, nz), vt) - REAL_MULT(ny, st);
	m[2][1] = REAL_MULT(REAL_MULT(ny, nz), vt) + REAL_MULT(nx, st);
	m[2][2] = REAL_MULT(REAL_MULT(nz, nz), vt) + ct;

	m[3][0] = 0.0;
	m[3][1] = 0.0;
	m[3][2] = 0.0;  

}  //==== Matrix3D::RotateAroundVector() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::Determinant
//|
//| Purpose: Calculates determinant.
//|
//| Parameters: none
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Real Matrix3D::Determinant()
{

  Real result;

  result = Det3D(m[0][0], m[0][1], m[0][2],
                 m[1][0], m[1][1], m[1][2],
                 m[2][0], m[2][1], m[2][2]);
  
  return result;
  
}  //==== Matrix3D::Determinant ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::Invert
//|
//| Purpose: Inverts this matrix.
//|
//| Parameters: returns FALSE if singular
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Flag Matrix3D::Invert()
{

  Matrix3D temp;
  if (InvertTo(temp) == FALSE)
    return FALSE;
  *this = temp;
  return TRUE;

}  //==== Matrix3D::Invert ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::InvertTo
//|
//| Purpose: Inverts this matrix puts result in matrix handed.
//|          Standard matrix inversion.
//|
//| Parameters: result: where to put resulting matrix
//|             returns FALSE if singular
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Flag Matrix3D::InvertTo(Matrix3D &result)
{

  // Calculate the determinant
  Real determinant = Determinant();
  if (determinant == 0)
    return FALSE;
  // Multiply is faster than divide
  determinant = REAL_DIV(REAL_ONE, determinant);

 	// 1st row entries
  result.m[0][0] = REAL_MULT(
                       Det2D(m[1][1], m[1][2],
                             m[2][1], m[2][2]), determinant);
  result.m[0][1] = REAL_MULT(
                       Det2D(m[2][1], m[2][2],
                             m[0][1], m[0][2]), determinant);
  result.m[0][2] = REAL_MULT(
                       Det2D(m[0][1], m[0][2],
                             m[1][1], m[1][2]), determinant);
 
  // 2nd row entries
  result.m[1][0] = REAL_MULT(
 	                   Det2D(m[1][2], m[1][0],
	                         m[2][2], m[2][0]), determinant);
  result.m[1][1] = REAL_MULT(
                     Det2D(m[2][2], m[2][0],
                           m[0][2], m[0][0]), determinant);
  result.m[1][2] = REAL_MULT(
                     Det2D(m[0][2], m[0][0],
                           m[1][2], m[1][0]), determinant);
 
  // 3rd row entries
  result.m[2][0] = REAL_MULT(
                     Det2D(m[1][0], m[1][1],
                           m[2][0], m[2][1]), determinant);
  result.m[2][1] = REAL_MULT(
                     Det2D(m[2][0], m[2][1],
                           m[0][0], m[0][1]), determinant);
  result.m[2][2] = REAL_MULT(
                     Det2D(m[0][0], m[0][1],
                           m[1][0], m[1][1]), determinant);
                           
  // Invert the translation vector
  result.m[3][0] = -REAL_MULT(m[3][0], result.m[0][0])
                   -REAL_MULT(m[3][1], result.m[1][0])
                   -REAL_MULT(m[3][2], result.m[2][0]);
  result.m[3][1] = -REAL_MULT(m[3][0], result.m[0][1])
                   -REAL_MULT(m[3][1], result.m[1][1])
                   -REAL_MULT(m[3][2], result.m[2][1]);
  result.m[3][2] = -REAL_MULT(m[3][0], result.m[0][2])
                   -REAL_MULT(m[3][1], result.m[1][2])
                   -REAL_MULT(m[3][2], result.m[2][2]);

  return TRUE;

}  //==== Matrix3D::InvertTo ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::Translation
//|
//| Purpose: Extracts translation from this.
//|
//| Parameters: translation: where the values are stored
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

void Matrix3D::Translation(Vector3D &translation)
{

	translation.x = m[3][0];
	translation.y = m[3][1];
	translation.z = m[3][2];

}  //==== Translation() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::ToExplicit
//|
//| Purpose: Makes the transformation values of this matrix explicit.
//|          From Graphic Gems II, page 600, without perspective.
//|
//| Parameters: scale, rotation, shear, translation:
//|             Vectors where the explicit values are stored.
//|             shear is defined as a Vector3D (x=Sx/y, y=Sx/z, z=Sz/y)
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

void Matrix3D::ToExplicit(Vector3D &scale, Vector3D &rotation,
                          Vector3D &shear, Vector3D &translation)
{

  int      i;
  Vector3D row[3];
 
  // Extract translation, simple
  Translation(translation);
 
	// Copy matrix into row vectors
  for (i = 0; i < 3; i++)
    {
    row[i].x = m[i][0];
    row[i].y = m[i][1];
    row[i].z = m[i][2];
    }

  // Compute X scale factor and normalize first row
  scale.x = row[0].Magnitude();
  row[0].Normalize();
	
  // Compute XY shear factor and make 2nd row orthogonal to 1st
  shear.x = row[0].Dot(row[1]);
  row[1].Combine(row[1], row[0], REAL_ONE, -shear.x);
 
  // Now, compute Y scale and normalize 2nd row
  scale.y = row[1].Magnitude();
  row[1].Normalize();
  shear.x = REAL_DIV(shear.x, scale.y);

  // Compute XZ and YZ shears, orthogonalize 3rd row
  shear.y = row[0].Dot(row[2]);
  row[2].Combine(row[2], row[0], REAL_ONE, -shear.y);
  shear.z = row[1].Dot(row[2]);
  row[2].Combine(row[2], row[1], REAL_ONE, -shear.z);

  // Next, get Z scale and normalize 3rd row
  scale.z = row[2].Magnitude();
  row[2].Normalize();
  shear.y = REAL_DIV(shear.y, scale.z);
  shear.z = REAL_DIV(shear.z, scale.z);
 
  // At this point, the matrix in row[] is orthonormal. Check for coor-
  // dinate system flip. If the determinant is -1, then negate the matrix
  // and the scaling factors.
  if (row[0].Dot(row[1].CrossTo(row[2])) < 0)
    {
    scale.Negate();
    for (i = 0; i < 3; i++)
      {
      row[i].Negate();
      }
    }
 
  // Now get the rotations out as described in the gem. 
  rotation.y = asin(-row[0].z);
  Real temp = REAL_COS(rotation.y);
  if (!REAL_IS_ZERO(temp))
    {
    rotation.x = atan2(row[1].z, row[2].z);
    rotation.z = atan2(row[0].y, row[0].x);
    }
	else
    {
    rotation.x = atan2(row[1].x, row[1].y);
    rotation.z = 0;
    }

}  //==== Matrix3D::ToExplicit ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::FromExplicit
//|
//| Purpose: Creates a matrix from explicit values
//|
//| Parameters: scale, rotation, translation:
//|             Vectors where the explicit values are.
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

void Matrix3D::FromExplicit(const Vector3D &scale,
                            const Vector3D &rotation,
                            const Vector3D &translation)
{

  Zero();
  Identity();
  AddScale(scale);
  AddRotateAroundX(rotation.X());
  AddRotateAroundY(rotation.Y());
  AddRotateAroundZ(rotation.Z());
  AddTranslate(translation);

}  //==== Matrix3D::FromExplicit ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::FromExplicit
//|
//| Purpose: Creates a matrix from scale, heading, up and translation
//|          vectors.
//|
//| Parameters: scale, heading, up, translation:
//|             Vectors where the explicit values are.
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

void Matrix3D::FromExplicit(const Vector3D &scale,
                            const Vector3D &heading, const Vector3D &up,
                            const Vector3D &translation)
{

  Zero();
  Identity();
  AddScale(scale);
  Matrix3D rotation;
  rotation.Rotate(heading, up);
  *this *= rotation;
  AddTranslate(translation);

}  //==== Matrix3D::FromExplicit ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D::FromExplicit
//|
//| Purpose: Creates a matrix from scale, heading, twist and
//|          translation.
//|
//| Parameters: scale, heading, twist, translation:
//|             Where the explicit values are.
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

void Matrix3D::FromExplicit(const Vector3D &scale,
                            const Vector3D &heading, Real twist,
                            const Vector3D &translation)
{

  Zero();
  Identity();
  AddScale(scale);
  Matrix3D rotation;
  rotation.Rotate(heading, twist);
  *this *= rotation;
  AddTranslate(translation);

}  //==== Matrix3D::FromExplicit ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D operator *
//|
//| Purpose: This performs matrix multiplication.
//|
//| Parameters: M: the matrix to multiply this matrix by
//|             returns the matrix product
//|
//| Created by Jeff Palmer
//|
//| Modified:
//|   December 15, 1994 by Greg Ferrar
//|     changed matrix elements to Real
//|   April 26, 1995 by Maarten Gribnau
//|     updated for 4x3 matrix
//|___________________________________________________________________

Matrix3D Matrix3D::operator * (const Matrix3D &M) const
{

  Matrix3D product;

  product.m[0][0] =
    REAL_MULT(m[0][0], M.m[0][0]) + REAL_MULT(m[0][1], M.m[1][0]) +
      REAL_MULT(m[0][2], M.m[2][0]);

  product.m[0][1] =
    REAL_MULT(m[0][0], M.m[0][1]) + REAL_MULT(m[0][1], M.m[1][1]) +
      REAL_MULT(m[0][2], M.m[2][1]);

  product.m[0][2] =
    REAL_MULT(m[0][0], M.m[0][2]) + REAL_MULT(m[0][1], M.m[1][2]) +
      REAL_MULT(m[0][2], M.m[2][2]);

  product.m[1][0] =
    REAL_MULT(m[1][0], M.m[0][0]) + REAL_MULT(m[1][1], M.m[1][0]) +
      REAL_MULT(m[1][2], M.m[2][0]);

  product.m[1][1] =
    REAL_MULT(m[1][0], M.m[0][1]) + REAL_MULT(m[1][1], M.m[1][1]) +
      REAL_MULT(m[1][2], M.m[2][1]);

  product.m[1][2] =
    REAL_MULT(m[1][0], M.m[0][2]) + REAL_MULT(m[1][1], M.m[1][2]) +
      REAL_MULT(m[1][2], M.m[2][2]);

  product.m[2][0] =
    REAL_MULT(m[2][0], M.m[0][0]) + REAL_MULT(m[2][1], M.m[1][0]) +
      REAL_MULT(m[2][2], M.m[2][0]);

  product.m[2][1] =
    REAL_MULT(m[2][0], M.m[0][1]) + REAL_MULT(m[2][1], M.m[1][1]) +
      REAL_MULT(m[2][2], M.m[2][1]);

  product.m[2][2] =
    REAL_MULT(m[2][0], M.m[0][2]) + REAL_MULT(m[2][1], M.m[1][2]) +
      REAL_MULT(m[2][2], M.m[2][2]);

  product.m[3][0] =
    REAL_MULT(m[3][0], M.m[0][0]) + REAL_MULT(m[3][1], M.m[1][0]) +
      REAL_MULT(m[3][2], M.m[2][0]) + M.m[3][0];

  product.m[3][1] =
    REAL_MULT(m[3][0], M.m[0][1]) + REAL_MULT(m[3][1], M.m[1][1]) +
      REAL_MULT(m[3][2], M.m[2][1]) + M.m[3][1];

  product.m[3][2] =
    REAL_MULT(m[3][0], M.m[0][2]) + REAL_MULT(m[3][1], M.m[1][2]) +
      REAL_MULT(m[3][2], M.m[2][2]) + M.m[3][2];

  return product;

}  //==== Matrix3D::operator * ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Matrix3D operator *=
//|
//| Purpose: This performs matrix multiplication, stores it in this.
//|
//| Parameters: M: the matrix to multiply this matrix by
//|             returns the matrix product
//|
//| Created byMaarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Matrix3D Matrix3D::operator *= (const Matrix3D &M)
{

  Matrix3D t = *this;
  Matrix3D u = M;     // M could be this

  m[0][0] =
    REAL_MULT(t.m[0][0], u.m[0][0]) + REAL_MULT(t.m[0][1], u.m[1][0]) +
      REAL_MULT(t.m[0][2], u.m[2][0]);

  m[0][1] =
    REAL_MULT(t.m[0][0], u.m[0][1]) + REAL_MULT(t.m[0][1], u.m[1][1]) +
      REAL_MULT(t.m[0][2], u.m[2][1]);

  m[0][2] =
    REAL_MULT(t.m[0][0], u.m[0][2]) + REAL_MULT(t.m[0][1], u.m[1][2]) +
      REAL_MULT(t.m[0][2], u.m[2][2]);

  m[1][0] =
    REAL_MULT(t.m[1][0], u.m[0][0]) + REAL_MULT(t.m[1][1], u.m[1][0]) +
      REAL_MULT(t.m[1][2], u.m[2][0]);

  m[1][1] =
    REAL_MULT(t.m[1][0], u.m[0][1]) + REAL_MULT(t.m[1][1], u.m[1][1]) +
      REAL_MULT(t.m[1][2], u.m[2][1]);

  m[1][2] =
    REAL_MULT(t.m[1][0], u.m[0][2]) + REAL_MULT(t.m[1][1], u.m[1][2]) +
      REAL_MULT(t.m[1][2], u.m[2][2]);

  m[2][0] =
    REAL_MULT(t.m[2][0], u.m[0][0]) + REAL_MULT(t.m[2][1], u.m[1][0]) +
      REAL_MULT(t.m[2][2], u.m[2][0]);

  m[2][1] =
    REAL_MULT(t.m[2][0], u.m[0][1]) + REAL_MULT(t.m[2][1], u.m[1][1]) +
      REAL_MULT(t.m[2][2], u.m[2][1]);

  m[2][2] =
    REAL_MULT(t.m[2][0], u.m[0][2]) + REAL_MULT(t.m[2][1], u.m[1][2]) +
      REAL_MULT(t.m[2][2], u.m[2][2]);

  m[3][0] =
    REAL_MULT(t.m[3][0], u.m[0][0]) + REAL_MULT(t.m[3][1], u.m[1][0]) +
      REAL_MULT(t.m[3][2], u.m[2][0]) + u.m[3][0];

  m[3][1] =
    REAL_MULT(t.m[3][0], u.m[0][1]) + REAL_MULT(t.m[3][1], u.m[1][1]) +
      REAL_MULT(t.m[3][2], u.m[2][1]) + u.m[3][1];

  m[3][2] =
    REAL_MULT(t.m[3][0], u.m[0][2]) + REAL_MULT(t.m[3][1], u.m[1][2]) +
      REAL_MULT(t.m[3][2], u.m[2][2]) + u.m[3][2];

  return *this;

}  //==== Matrix3D::operator *= ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| operator << 
//|
//| Purpose: This outputs a Matrix3D to an output stream
//|
//| Parameters: o: the output stream
//|             M: the Matrix3D
//|
//| Created by Jeff Palmer
//|
//| Modified:
//|   April 26, 1995 by Maarten Gribnau
//|     updated for 4x3 matrix
//|___________________________________________________________________

ostream &operator << (ostream &o, const Matrix3D &M)
{

  o << '\n'; 

  register int i;
  register int j;
  for (i = 0; i < 4; i++)		   // Loop through all rows
    {
    o << '[';
    for (j = 0; j < 3; j++)	           // Loop through all columns
      o << REAL_TO_DOUBLE(M.m[i][j]) << " " << "\t"; // Print one element
    o << ']' << '\n';
    }

  return o;

}  //==== operator <<  ====



