/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   'ObjMatrix' is combined with 'CamMatrix' as 'ViewMatrix' to:
      -support large distances (because matrix calculations can be done in Dbl precision on the CPU side, and stored as Flt precision in view space)
      -reduce number of matrix operations in shaders

/******************************************************************************

   Matrix calculation rules:

   ~A in Esenthel means inverse (1/A)

      (A*B)*C = A*(B*C)

      A*B=C
      A*B/B=C/B
      A=C/B

      A=B, can multiply both sides on the right by same value
      A*C=B*C

      A=B, can multiply both sides on the left by same value
      C*A=C*B

      A=B, can divide both sides on the right by same value (which is multiply by inverse: *(1/C))
      A/C=B/C

      A=B, can inverse both sides
      1/A = 1/B

      1/(A*B) = (1/B)*(1/A)
       ~(A*B) =   ~B *  ~A

      A*B=C, multiply both sides on the left by ~A:
      ~A * (A*B) = ~A * (C)
      (~A * A) * B = ~A * C
      B = ~A * C

      A=C/B, inverse both sides:
      ~A=~(C /  B)
      ~A=~(C * ~B)
      ~A=~~B * ~C
      ~A=B * ~C
      ~A*C=B
      B=~A*C

/******************************************************************************/
      MatrixM ObjMatrix(1),
              CamMatrix,
              CamMatrixInv,
              CamMatrixInvPrev,
              EyeMatrix[2],
              EyeMatrixPrev[2];
      Matrix4 ProjMatrix;
      Flt     ProjMatrixEyeOffset[2];

const Matrix  MatrixIdentity (1);
const MatrixM MatrixMIdentity(1);

GpuMatrix *ViewMatrix, *ViewMatrixPrev;
/******************************************************************************/
Matrix2& Matrix2::operator*=(Flt f)
{
   x*=f;
   y*=f;
   return T;
}
Matrix3& Matrix3::operator*=(Flt f)
{
   x*=f;
   y*=f;
   z*=f;
   return T;
}
MatrixD3& MatrixD3::operator*=(Dbl f)
{
   x*=f;
   y*=f;
   z*=f;
   return T;
}
Matrix2& Matrix2::operator/=(Flt f)
{
   x/=f;
   y/=f;
   return T;
}
Matrix3& Matrix3::operator/=(Flt f)
{
   x/=f;
   y/=f;
   z/=f;
   return T;
}
MatrixD3& MatrixD3::operator/=(Dbl f)
{
   x/=f;
   y/=f;
   z/=f;
   return T;
}
Matrix2P& Matrix2P::operator*=(Flt f)
{
   x  *=f;
   y  *=f;
   pos*=f;
   return T;
}
Matrix& Matrix::operator*=(Flt f)
{
   x  *=f;
   y  *=f;
   z  *=f;
   pos*=f;
   return T;
}
MatrixM& MatrixM::operator*=(Flt f)
{
   x  *=f;
   y  *=f;
   z  *=f;
   pos*=f;
   return T;
}
MatrixD& MatrixD::operator*=(Dbl f)
{
   x  *=f;
   y  *=f;
   z  *=f;
   pos*=f;
   return T;
}
Matrix2P& Matrix2P::operator/=(Flt f)
{
   x  /=f;
   y  /=f;
   pos/=f;
   return T;
}
Matrix& Matrix::operator/=(Flt f)
{
   x  /=f;
   y  /=f;
   z  /=f;
   pos/=f;
   return T;
}
MatrixM& MatrixM::operator/=(Flt f)
{
   x  /=f;
   y  /=f;
   z  /=f;
   pos/=f;
   return T;
}
MatrixD& MatrixD::operator/=(Dbl f)
{
   x  /=f;
   y  /=f;
   z  /=f;
   pos/=f;
   return T;
}
Matrix2& Matrix2::operator*=(C Vec2 &v)
{
   x*=v;
   y*=v;
   return T;
}
Matrix3& Matrix3::operator*=(C Vec &v)
{
   x*=v;
   y*=v;
   z*=v;
   return T;
}
MatrixD3& MatrixD3::operator*=(C VecD &v)
{
   x*=v;
   y*=v;
   z*=v;
   return T;
}
Matrix2& Matrix2::operator/=(C Vec2 &v)
{
   x/=v;
   y/=v;
   return T;
}
Matrix3& Matrix3::operator/=(C Vec &v)
{
   x/=v;
   y/=v;
   z/=v;
   return T;
}
MatrixD3& MatrixD3::operator/=(C VecD &v)
{
   x/=v;
   y/=v;
   z/=v;
   return T;
}
Matrix2P& Matrix2P::operator*=(C Vec2 &v)
{
   x  *=v;
   y  *=v;
   pos*=v;
   return T;
}
Matrix& Matrix::operator*=(C Vec &v)
{
   x  *=v;
   y  *=v;
   z  *=v;
   pos*=v;
   return T;
}
MatrixM& MatrixM::operator*=(C Vec &v)
{
   x  *=v;
   y  *=v;
   z  *=v;
   pos*=v;
   return T;
}
MatrixD& MatrixD::operator*=(C VecD &v)
{
   x  *=v;
   y  *=v;
   z  *=v;
   pos*=v;
   return T;
}
Matrix2P& Matrix2P::operator/=(C Vec2 &v)
{
   x  /=v;
   y  /=v;
   pos/=v;
   return T;
}
Matrix& Matrix::operator/=(C Vec &v)
{
   x  /=v;
   y  /=v;
   z  /=v;
   pos/=v;
   return T;
}
MatrixM& MatrixM::operator/=(C Vec &v)
{
   x  /=v;
   y  /=v;
   z  /=v;
   pos/=v;
   return T;
}
MatrixD& MatrixD::operator/=(C VecD &v)
{
   x  /=v;
   y  /=v;
   z  /=v;
   pos/=v;
   return T;
}
Matrix3& Matrix3::operator+=(C Matrix3 &m)
{
   x+=m.x;
   y+=m.y;
   z+=m.z;
   return T;
}
MatrixD3& MatrixD3::operator+=(C MatrixD3 &m)
{
   x+=m.x;
   y+=m.y;
   z+=m.z;
   return T;
}
Matrix3& Matrix3::operator-=(C Matrix3 &m)
{
   x-=m.x;
   y-=m.y;
   z-=m.z;
   return T;
}
MatrixD3& MatrixD3::operator-=(C MatrixD3 &m)
{
   x-=m.x;
   y-=m.y;
   z-=m.z;
   return T;
}
Matrix& Matrix::operator+=(C Matrix &m)
{
   x  +=m.x  ;
   y  +=m.y  ;
   z  +=m.z  ;
   pos+=m.pos;
   return T;
}
MatrixM& MatrixM::operator+=(C MatrixM &m)
{
   x  +=m.x  ;
   y  +=m.y  ;
   z  +=m.z  ;
   pos+=m.pos;
   return T;
}
MatrixD& MatrixD::operator+=(C MatrixD &m)
{
   x  +=m.x  ;
   y  +=m.y  ;
   z  +=m.z  ;
   pos+=m.pos;
   return T;
}
Matrix& Matrix::operator-=(C Matrix &m)
{
   x  -=m.x  ;
   y  -=m.y  ;
   z  -=m.z  ;
   pos-=m.pos;
   return T;
}
MatrixM& MatrixM::operator-=(C MatrixM &m)
{
   x  -=m.x  ;
   y  -=m.y  ;
   z  -=m.z  ;
   pos-=m.pos;
   return T;
}
MatrixD& MatrixD::operator-=(C MatrixD &m)
{
   x  -=m.x  ;
   y  -=m.y  ;
   z  -=m.z  ;
   pos-=m.pos;
   return T;
}
/******************************************************************************/
Bool Matrix2 ::operator==(C Matrix2  &m)C {return x==m.x && y==m.y;}
Bool Matrix2 ::operator!=(C Matrix2  &m)C {return x!=m.x || y!=m.y;}
Bool Matrix3 ::operator==(C Matrix3  &m)C {return x==m.x && y==m.y && z==m.z;}
Bool Matrix3 ::operator!=(C Matrix3  &m)C {return x!=m.x || y!=m.y || z!=m.z;}
Bool MatrixD3::operator==(C MatrixD3 &m)C {return x==m.x && y==m.y && z==m.z;}
Bool MatrixD3::operator!=(C MatrixD3 &m)C {return x!=m.x || y!=m.y || z!=m.z;}
Bool Matrix2P::operator==(C Matrix2P &m)C {return x==m.x && y==m.y &&           pos==m.pos;}
Bool Matrix2P::operator!=(C Matrix2P &m)C {return x!=m.x || y!=m.y ||           pos!=m.pos;}
Bool Matrix  ::operator==(C Matrix   &m)C {return x==m.x && y==m.y && z==m.z && pos==m.pos;}
Bool Matrix  ::operator!=(C Matrix   &m)C {return x!=m.x || y!=m.y || z!=m.z || pos!=m.pos;}
Bool MatrixM ::operator==(C MatrixM  &m)C {return x==m.x && y==m.y && z==m.z && pos==m.pos;}
Bool MatrixM ::operator!=(C MatrixM  &m)C {return x!=m.x || y!=m.y || z!=m.z || pos!=m.pos;}
Bool MatrixD ::operator==(C MatrixD  &m)C {return x==m.x && y==m.y && z==m.z && pos==m.pos;}
Bool MatrixD ::operator!=(C MatrixD  &m)C {return x!=m.x || y!=m.y || z!=m.z || pos!=m.pos;}
/******************************************************************************/
void Matrix2::mul(C Matrix2 &m, Matrix2 &dest)C
{
   Flt x, y;
   if(&dest!=&m) // here it's OK if "&dest==this"
   {
               x=T.x.x;  y=T.x.y;
      dest.x.x=x*m.x.x + y*m.y.x;
      dest.x.y=x*m.x.y + y*m.y.y;

               x=T.y.x;  y=T.y.y;
      dest.y.x=x*m.x.x + y*m.y.x;
      dest.y.y=x*m.x.y + y*m.y.y;
   }else
   if(&dest!=this) // here it's OK if "&dest==&m"
   {
               x=m.x.x;  y=m.y.x;
      dest.x.x=x*T.x.x + y*T.x.y;
      dest.y.x=x*T.y.x + y*T.y.y;

               x=m.x.y;  y=m.y.y;
      dest.x.y=x*T.x.x + y*T.x.y;
      dest.y.y=x*T.y.x + y*T.y.y;
   }else // &dest==&m && &dest==this
   {
      Matrix2 temp=T; temp.mul(temp, dest); // have to operate on a temporary because all matrixes are the same
   }
}
void Matrix3::mul(C Matrix3 &m, Matrix3 &dest)C
{
   Flt x, y, z;
   if(&dest!=&m) // here it's OK if "&dest==this"
   {
               x=T.x.x;  y=T.x.y;  z=T.x.z;
      dest.x.x=x*m.x.x + y*m.y.x + z*m.z.x;
      dest.x.y=x*m.x.y + y*m.y.y + z*m.z.y;
      dest.x.z=x*m.x.z + y*m.y.z + z*m.z.z;

               x=T.y.x;  y=T.y.y;  z=T.y.z;
      dest.y.x=x*m.x.x + y*m.y.x + z*m.z.x;
      dest.y.y=x*m.x.y + y*m.y.y + z*m.z.y;
      dest.y.z=x*m.x.z + y*m.y.z + z*m.z.z;

               x=T.z.x;  y=T.z.y;  z=T.z.z;
      dest.z.x=x*m.x.x + y*m.y.x + z*m.z.x;
      dest.z.y=x*m.x.y + y*m.y.y + z*m.z.y;
      dest.z.z=x*m.x.z + y*m.y.z + z*m.z.z;
   }else
   if(&dest!=this) // here it's OK if "&dest==&m"
   {
               x=m.x.x;  y=m.y.x;  z=m.z.x;
      dest.x.x=x*T.x.x + y*T.x.y + z*T.x.z;
      dest.y.x=x*T.y.x + y*T.y.y + z*T.y.z;
      dest.z.x=x*T.z.x + y*T.z.y + z*T.z.z;

               x=m.x.y;  y=m.y.y;  z=m.z.y;
      dest.x.y=x*T.x.x + y*T.x.y + z*T.x.z;
      dest.y.y=x*T.y.x + y*T.y.y + z*T.y.z;
      dest.z.y=x*T.z.x + y*T.z.y + z*T.z.z;

               x=m.x.z;  y=m.y.z;  z=m.z.z;
      dest.x.z=x*T.x.x + y*T.x.y + z*T.x.z;
      dest.y.z=x*T.y.x + y*T.y.y + z*T.y.z;
      dest.z.z=x*T.z.x + y*T.z.y + z*T.z.z;
   }else // &dest==&m && &dest==this
   {
      Matrix3 temp=T; temp.mul(temp, dest); // have to operate on a temporary because all matrixes are the same
   }
}
void MatrixD3::mul(C Matrix3 &m, MatrixD3 &dest)C
{
   Dbl x, y, z;
            x=T.x.x;  y=T.x.y;  z=T.x.z;
   dest.x.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.x.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.x.z=x*m.x.z + y*m.y.z + z*m.z.z;

            x=T.y.x;  y=T.y.y;  z=T.y.z;
   dest.y.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.y.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.y.z=x*m.x.z + y*m.y.z + z*m.z.z;

            x=T.z.x;  y=T.z.y;  z=T.z.z;
   dest.z.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.z.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.z.z=x*m.x.z + y*m.y.z + z*m.z.z;
}
void MatrixD3::mul(C MatrixD3 &m, MatrixD3 &dest)C
{
   Dbl x, y, z;
   if(&dest!=&m) // here it's OK if "&dest==this"
   {
               x=T.x.x;  y=T.x.y;  z=T.x.z;
      dest.x.x=x*m.x.x + y*m.y.x + z*m.z.x;
      dest.x.y=x*m.x.y + y*m.y.y + z*m.z.y;
      dest.x.z=x*m.x.z + y*m.y.z + z*m.z.z;

               x=T.y.x;  y=T.y.y;  z=T.y.z;
      dest.y.x=x*m.x.x + y*m.y.x + z*m.z.x;
      dest.y.y=x*m.x.y + y*m.y.y + z*m.z.y;
      dest.y.z=x*m.x.z + y*m.y.z + z*m.z.z;

               x=T.z.x;  y=T.z.y;  z=T.z.z;
      dest.z.x=x*m.x.x + y*m.y.x + z*m.z.x;
      dest.z.y=x*m.x.y + y*m.y.y + z*m.z.y;
      dest.z.z=x*m.x.z + y*m.y.z + z*m.z.z;
   }else
   if(&dest!=this) // here it's OK if "&dest==&m"
   {
               x=m.x.x;  y=m.y.x;  z=m.z.x;
      dest.x.x=x*T.x.x + y*T.x.y + z*T.x.z;
      dest.y.x=x*T.y.x + y*T.y.y + z*T.y.z;
      dest.z.x=x*T.z.x + y*T.z.y + z*T.z.z;

               x=m.x.y;  y=m.y.y;  z=m.z.y;
      dest.x.y=x*T.x.x + y*T.x.y + z*T.x.z;
      dest.y.y=x*T.y.x + y*T.y.y + z*T.y.z;
      dest.z.y=x*T.z.x + y*T.z.y + z*T.z.z;

               x=m.x.z;  y=m.y.z;  z=m.z.z;
      dest.x.z=x*T.x.x + y*T.x.y + z*T.x.z;
      dest.y.z=x*T.y.x + y*T.y.y + z*T.y.z;
      dest.z.z=x*T.z.x + y*T.z.y + z*T.z.z;
   }else // &dest==&m && &dest==this
   {
      MatrixD3 temp=T; temp.mul(temp, dest); // have to operate on a temporary because all matrixes are the same
   }
}
/******************************************************************************/
#if 0 // performance is nearly the same so don't use
void mulNormalized(C Matrix3 &matrix, Matrix3 &dest)C; // multiply self by 'matrix' and store result in 'dest', this method assumes that both self and 'matrix' are normalized, this is faster than 'mul'
void Matrix3::mulNormalized(C Matrix3 &m, Matrix3 &dest)C
{
   Flt x, y, z;
   if(&dest!=&m) // here it's OK if "&dest==this"
   {
             /*x=T.x.x;  y=T.x.y;  z=T.x.z;
      dest.x.x=x*m.x.x + y*m.y.x + z*m.z.x;
      dest.x.y=x*m.x.y + y*m.y.y + z*m.z.y;
      dest.x.z=x*m.x.z + y*m.y.z + z*m.z.z;*/

               x=T.y.x;  y=T.y.y;  z=T.y.z;
      dest.y.x=x*m.x.x + y*m.y.x + z*m.z.x;
      dest.y.y=x*m.x.y + y*m.y.y + z*m.z.y;
      dest.y.z=x*m.x.z + y*m.y.z + z*m.z.z;

               x=T.z.x;  y=T.z.y;  z=T.z.z;
      dest.z.x=x*m.x.x + y*m.y.x + z*m.z.x;
      dest.z.y=x*m.x.y + y*m.y.y + z*m.z.y;
      dest.z.z=x*m.x.z + y*m.y.z + z*m.z.z;
   }else
   if(&dest!=this) // here it's OK if "&dest==&m"
   {
               x=m.x.x;  y=m.y.x;  z=m.z.x;
    //dest.x.x=x*T.x.x + y*T.x.y + z*T.x.z;
      dest.y.x=x*T.y.x + y*T.y.y + z*T.y.z;
      dest.z.x=x*T.z.x + y*T.z.y + z*T.z.z;

               x=m.x.y;  y=m.y.y;  z=m.z.y;
    //dest.x.y=x*T.x.x + y*T.x.y + z*T.x.z;
      dest.y.y=x*T.y.x + y*T.y.y + z*T.y.z;
      dest.z.y=x*T.z.x + y*T.z.y + z*T.z.z;

               x=m.x.z;  y=m.y.z;  z=m.z.z;
    //dest.x.z=x*T.x.x + y*T.x.y + z*T.x.z;
      dest.y.z=x*T.y.x + y*T.y.y + z*T.y.z;
      dest.z.z=x*T.z.x + y*T.z.y + z*T.z.z;
   }else // &dest==&m && &dest==this
   {
      fixme
   }
 //dest.x=Cross(dest.y, dest.z);
   dest.x.x=dest.y.y*dest.z.z - dest.y.z*dest.z.y;
   dest.x.y=dest.y.z*dest.z.x - dest.y.x*dest.z.z;
   dest.x.z=dest.y.x*dest.z.y - dest.y.y*dest.z.x;
}
#endif
/******************************************************************************/
void Matrix2::mul(C Matrix2P &m, Matrix2P &dest)C
{
   dest.pos=m.pos;
   mul(m.orn(), dest.orn());
}
void Matrix3::mul(C Matrix &m, Matrix &dest)C
{
   dest.pos=m.pos;
   mul(m.orn(), dest.orn());
}
void Matrix3::mul(C MatrixM &m, MatrixM &dest)C
{
   dest.pos=m.pos;
   mul(m.orn(), dest.orn());
}
void MatrixD3::mul(C MatrixD &m, MatrixD &dest)C
{
   dest.pos=m.pos;
   mul(m.orn(), dest.orn());
}
/******************************************************************************/
void Matrix2P::mul(C Matrix2 &m, Matrix2P &dest)C
{
   Flt        x=pos.x , y=pos.y;
   dest.pos.x=x*m.x.x + y*m.y.x;
   dest.pos.y=x*m.x.y + y*m.y.y;
   super::mul(m, dest.orn());
}
void Matrix::mul(C Matrix3 &m, Matrix &dest)C
{
   Flt        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z;
   super::mul(m, dest.orn());
}
void MatrixM::mul(C Matrix3 &m, MatrixM &dest)C
{
   Dbl        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z;
   super::mul(m, dest.orn());
}
void MatrixD::mul(C MatrixD3 &m, MatrixD &dest)C
{
   Dbl        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z;
   super::mul(m, dest.orn());
}
/******************************************************************************
void mulNormalized(C Matrix3 &matrix, Matrix &dest)C; // multiply self by 'matrix' and store result in 'dest', this method assumes that both self and 'matrix' are normalized, this is faster than 'mul'
void Matrix::mulNormalized(C Matrix3 &m, Matrix &dest)C
{
   Flt        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z;
   super::mulNormalized(m, dest.orn());
}
/******************************************************************************/
void Matrix2P::mul(C Matrix2P &m, Matrix2P &dest)C
{
   Flt        x=pos.x , y=pos.y;
   dest.pos.x=x*m.x.x + y*m.y.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + m.pos.y;
   super::mul(m.orn(), dest.orn());
}
void Matrix::mul(C Matrix &m, Matrix &dest)C
{
   Flt        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   super::mul(m.orn(), dest.orn());
}
void Matrix::mul(C MatrixM &m, Matrix &dest)C
{
   Flt        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   super::mul(m.orn(), dest.orn());
}
void Matrix::mul(C MatrixM &m, MatrixM &dest)C
{
   Flt        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   super::mul(m.orn(), dest.orn());
}
void MatrixM::mul(C MatrixM &m, Matrix &dest)C
{
   Dbl        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   super::mul(m.orn(), dest.orn());
}
void MatrixM::mul(C Matrix &m, MatrixM &dest)C
{
   Dbl        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   super::mul(m.orn(), dest.orn());
}
void MatrixM::mul(C MatrixM &m, MatrixM &dest)C
{
   Dbl        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   super::mul(m.orn(), dest.orn());
}
void MatrixD::mul(C MatrixD &m, MatrixD &dest)C
{
   Dbl        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   super::mul(m.orn(), dest.orn());
}
void MatrixD::mul(C MatrixM &m, MatrixD &dest)C
{
   Dbl        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   super::mul(m.orn(), dest.orn());
}
/******************************************************************************
void Matrix::mulNormalized(C Matrix &m, Matrix &dest)C
{
   Flt        x=pos.x , y=pos.y , z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;
   super::mulNormalized(m.orn(), dest.orn());
}
/******************************************************************************/
void Matrix::mul(C Matrix &m, Matrix4 &dest)C
{
   Flt x, y, z;

            x=T.x.x;  y=T.x.y;  z=T.x.z;
   dest.x.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.x.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.x.z=x*m.x.z + y*m.y.z + z*m.z.z;

            x=T.y.x;  y=T.y.y;  z=T.y.z;
   dest.y.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.y.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.y.z=x*m.x.z + y*m.y.z + z*m.z.z;

            x=T.z.x;  y=T.z.y;  z=T.z.z;
   dest.z.x=x*m.x.x + y*m.y.x + z*m.z.x;
   dest.z.y=x*m.x.y + y*m.y.y + z*m.z.y;
   dest.z.z=x*m.x.z + y*m.y.z + z*m.z.z;

              x=pos.x;  y=pos.y;  z=pos.z;
   dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + m.pos.x;
   dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + m.pos.y;
   dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + m.pos.z;

   dest.x.w=dest.y.w=dest.z.w=0;
   dest.pos.w=1;
}
void Matrix::mul(C Matrix4 &m, Matrix4 &dest)C
{
   Flt x, y, z;

   x=m.x.x; y=m.y.x; z=m.z.x;
   dest.x  .x=T.x.x*x + T.x.y*y + T.x.z*z;
   dest.y  .x=T.y.x*x + T.y.y*y + T.y.z*z;
   dest.z  .x=T.z.x*x + T.z.y*y + T.z.z*z;
   dest.pos.x=pos.x*x + pos.y*y + pos.z*z + m.pos.x;

   x=m.x.y; y=m.y.y; z=m.z.y;
   dest.x  .y=T.x.x*x + T.x.y*y + T.x.z*z;
   dest.y  .y=T.y.x*x + T.y.y*y + T.y.z*z;
   dest.z  .y=T.z.x*x + T.z.y*y + T.z.z*z;
   dest.pos.y=pos.x*x + pos.y*y + pos.z*z + m.pos.y;

   x=m.x.z; y=m.y.z; z=m.z.z;
   dest.x  .z=T.x.x*x + T.x.y*y + T.x.z*z;
   dest.y  .z=T.y.x*x + T.y.y*y + T.y.z*z;
   dest.z  .z=T.z.x*x + T.z.y*y + T.z.z*z;
   dest.pos.z=pos.x*x + pos.y*y + pos.z*z + m.pos.z;

   x=m.x.w; y=m.y.w; z=m.z.w;
   dest.x  .w=T.x.x*x + T.x.y*y + T.x.z*z;
   dest.y  .w=T.y.x*x + T.y.y*y + T.y.z*z;
   dest.z  .w=T.z.x*x + T.z.y*y + T.z.z*z;
   dest.pos.w=pos.x*x + pos.y*y + pos.z*z + m.pos.w;
}
void Matrix4::mul(C Matrix4 &m, Matrix4 &dest)C
{
   Flt x, y, z, w;
   if(&dest!=&m) // here it's OK if "&dest==this"
   {
               x=T.x.x;  y=T.x.y;  z=T.x.z;  w=T.  x.w;
      dest.x.x=x*m.x.x + y*m.y.x + z*m.z.x + w*m.pos.x;
      dest.x.y=x*m.x.y + y*m.y.y + z*m.z.y + w*m.pos.y;
      dest.x.z=x*m.x.z + y*m.y.z + z*m.z.z + w*m.pos.z;
      dest.x.w=x*m.x.w + y*m.y.w + z*m.z.w + w*m.pos.w;

               x=T.y.x;  y=T.y.y;  z=T.y.z;  w=T.  y.w;
      dest.y.x=x*m.x.x + y*m.y.x + z*m.z.x + w*m.pos.x;
      dest.y.y=x*m.x.y + y*m.y.y + z*m.z.y + w*m.pos.y;
      dest.y.z=x*m.x.z + y*m.y.z + z*m.z.z + w*m.pos.z;
      dest.y.w=x*m.x.w + y*m.y.w + z*m.z.w + w*m.pos.w;

               x=T.z.x;  y=T.z.y;  z=T.z.z;  w=T.  z.w;
      dest.z.x=x*m.x.x + y*m.y.x + z*m.z.x + w*m.pos.x;
      dest.z.y=x*m.x.y + y*m.y.y + z*m.z.y + w*m.pos.y;
      dest.z.z=x*m.x.z + y*m.y.z + z*m.z.z + w*m.pos.z;
      dest.z.w=x*m.x.w + y*m.y.w + z*m.z.w + w*m.pos.w;

               x=  pos.x;  y=pos.y;  z=pos.z;    w=pos.w;
      dest.pos.x=x*m.x.x + y*m.y.x + z*m.z.x + w*m.pos.x;
      dest.pos.y=x*m.x.y + y*m.y.y + z*m.z.y + w*m.pos.y;
      dest.pos.z=x*m.x.z + y*m.y.z + z*m.z.z + w*m.pos.z;
      dest.pos.w=x*m.x.w + y*m.y.w + z*m.z.w + w*m.pos.w;
   }else
   if(&dest!=this) // here it's OK if "&dest==&m"
   {
      x=m.x.x; y=m.y.x; z=m.z.x; w=m.pos.x;
      dest.x  .x=T.x.x*x + T.x.y*y + T.x.z*z + T.x.w*w;
      dest.y  .x=T.y.x*x + T.y.y*y + T.y.z*z + T.y.w*w;
      dest.z  .x=T.z.x*x + T.z.y*y + T.z.z*z + T.z.w*w;
      dest.pos.x=pos.x*x + pos.y*y + pos.z*z + pos.w*w;

      x=m.x.y; y=m.y.y; z=m.z.y; w=m.pos.y;
      dest.x  .y=T.x.x*x + T.x.y*y + T.x.z*z + T.x.w*w;
      dest.y  .y=T.y.x*x + T.y.y*y + T.y.z*z + T.y.w*w;
      dest.z  .y=T.z.x*x + T.z.y*y + T.z.z*z + T.z.w*w;
      dest.pos.y=pos.x*x + pos.y*y + pos.z*z + pos.w*w;

      x=m.x.z; y=m.y.z; z=m.z.z; w=m.pos.z;
      dest.x  .z=T.x.x*x + T.x.y*y + T.x.z*z + T.x.w*w;
      dest.y  .z=T.y.x*x + T.y.y*y + T.y.z*z + T.y.w*w;
      dest.z  .z=T.z.x*x + T.z.y*y + T.z.z*z + T.z.w*w;
      dest.pos.z=pos.x*x + pos.y*y + pos.z*z + pos.w*w;

      x=m.x.w; y=m.y.w; z=m.z.w; w=m.pos.w;
      dest.x  .w=T.x.x*x + T.x.y*y + T.x.z*z + T.x.w*w;
      dest.y  .w=T.y.x*x + T.y.y*y + T.y.z*z + T.y.w*w;
      dest.z  .w=T.z.x*x + T.z.y*y + T.z.z*z + T.z.w*w;
      dest.pos.w=pos.x*x + pos.y*y + pos.z*z + pos.w*w;
   }else
   {
      Matrix4 temp=T; temp.mul(temp, dest); // have to operate on a temporary because all matrixes are the same
   }
}
/******************************************************************************/
void Matrix::mulTimes(Int n, C Matrix &matrix, Matrix &dest)C
{
   switch(n)
   {
      case  0:             dest=T; break;
      case  1: mul(matrix, dest ); break;
      default:
      {
         if(Equal(matrix.orn(), MatrixIdentity.orn())) // no rotation and no scale - we can just move by "n*pos"
         {
            dest.orn()=T.orn();
            dest.pos  =T.pos+matrix.pos*n;
         }else
         if(matrix.pos.allZero() && Equal(matrix.scale2(), VecOne)) // no position and no scale - we can just rotate by "n*angle"
         {
            Vec axis; Flt angle=matrix.axisAngle(axis);
            Matrix matrix_n; matrix_n.setRotate(axis, angle*n);
            mul(matrix_n, dest);
         }else
         {
            Matrix temp=matrix; if(n<0){CHS(n); temp.inverse();}
            FREPA(PrimeNumbers) // optimize using prime numbers
            {
               Int pm=PrimeNumbers[i]; if(n<pm)break;
               for(; n%pm==0; n/=pm){Matrix single=temp; REP(pm-1)temp*=single;}
            }
            mul(temp, dest); REP(n-1)dest*=temp; // make the first mul from this->dest, and remaining mul's from dest->dest
         }
      }break;
   }
}
void Matrix::mulTimes(Int n, C RevMatrix &matrix, Matrix &dest)C
{
   switch(n)
   {
      case  0:             dest=T; break;
      case  1: mul(matrix, dest ); break;
      default:
      {
         if(Equal(matrix.orn(), MatrixIdentity.orn())) // no rotation and no scale - we can just move by "n*pos"
         {
            dest.orn()=T.orn();
            dest.pos  =T.pos+(matrix.pos*n)*T.orn();
         }else
         if(matrix.pos.allZero() && Equal(matrix.scale2(), VecOne)) // no position and no scale - we can just rotate by "n*angle"
         {
            Vec axis; Flt angle=matrix.axisAngle(axis);
            RevMatrix matrix_n; matrix_n.setRotate(axis, angle*n);
            mul(matrix_n, dest);
         }else
         {
            RevMatrix temp=matrix; if(n<0){CHS(n); temp.inverse();}
            FREPA(PrimeNumbers) // optimize using prime numbers
            {
               Int pm=PrimeNumbers[i]; if(n<pm)break;
               for(; n%pm==0; n/=pm){Matrix single=temp; REP(pm-1)temp*=single;} // in this case it doesn't matter if 'single' is 'Matrix' or 'RevMatrix'
            }
            mul(temp, dest); REP(n-1)dest*=temp; // make the first mul from this->dest, and remaining mul's from dest->dest
         }
      }break;
   }
}
/******************************************************************************/
void Matrix2::divNormalized(C Matrix2 &m, Matrix2 &dest)C
{
   if(&dest!=&m) // here it's OK if "&dest==this"
   {
      Flt x, y;
               x=T.x.x;  y=T.x.y;
      dest.x.x=x*m.x.x + y*m.x.y;
      dest.x.y=x*m.y.x + y*m.y.y;

               x=T.y.x;  y=T.y.y;
      dest.y.x=x*m.x.x + y*m.x.y;
      dest.y.y=x*m.y.x + y*m.y.y;
   }else
   if(&dest!=this) // here it's OK if "&dest==&m"
   {
      Flt x_x=m.x.x, x_y=m.x.y;
                     dest.x.x=x_x*T.x.x + x_y*T.x.y;
      Flt y_x=m.y.x; dest.y.x=x_x*T.y.x + x_y*T.y.y;

      Flt y_y=m.y.y;
      dest.x.y=y_x*T.x.x + y_y*T.x.y;
      dest.y.y=y_x*T.y.x + y_y*T.y.y;
   }else
   {
      Matrix2 temp=T; temp.divNormalized(temp, dest); // have to operate on a temporary because all matrixes are the same
   }
}
void Matrix3::divNormalized(C Matrix3 &m, Matrix3 &dest)C
{
   if(&dest!=&m) // here it's OK if "&dest==this"
   {
      Flt x, y, z;
               x=T.x.x;  y=T.x.y;  z=T.x.z;
      dest.x.x=x*m.x.x + y*m.x.y + z*m.x.z;
      dest.x.y=x*m.y.x + y*m.y.y + z*m.y.z;
      dest.x.z=x*m.z.x + y*m.z.y + z*m.z.z;

               x=T.y.x;  y=T.y.y;  z=T.y.z;
      dest.y.x=x*m.x.x + y*m.x.y + z*m.x.z;
      dest.y.y=x*m.y.x + y*m.y.y + z*m.y.z;
      dest.y.z=x*m.z.x + y*m.z.y + z*m.z.z;

               x=T.z.x;  y=T.z.y;  z=T.z.z;
      dest.z.x=x*m.x.x + y*m.x.y + z*m.x.z;
      dest.z.y=x*m.y.x + y*m.y.y + z*m.y.z;
      dest.z.z=x*m.z.x + y*m.z.y + z*m.z.z;
   }else
   if(&dest!=this) // here it's OK if "&dest==&m"
   {
      Flt x_x=m.x.x, x_y=m.x.y, x_z=m.x.z;
                     dest.x.x=x_x*T.x.x + x_y*T.x.y + x_z*T.x.z;
      Flt y_x=m.y.x; dest.y.x=x_x*T.y.x + x_y*T.y.y + x_z*T.y.z;
      Flt z_x=m.z.x; dest.z.x=x_x*T.z.x + x_y*T.z.y + x_z*T.z.z;

      Flt y_y=m.y.y, y_z=m.y.z;
                     dest.x.y=y_x*T.x.x + y_y*T.x.y + y_z*T.x.z;
                     dest.y.y=y_x*T.y.x + y_y*T.y.y + y_z*T.y.z;
      Flt z_y=m.z.y; dest.z.y=y_x*T.z.x + y_y*T.z.y + y_z*T.z.z;

      Flt z_z=m.z.z;
      dest.x.z=z_x*T.x.x + z_y*T.x.y + z_z*T.x.z;
      dest.y.z=z_x*T.y.x + z_y*T.y.y + z_z*T.y.z;
      dest.z.z=z_x*T.z.x + z_y*T.z.y + z_z*T.z.z;
   }else
   {
      Matrix3 temp=T; temp.divNormalized(temp, dest); // have to operate on a temporary because all matrixes are the same
   }
}
void MatrixD3::divNormalized(C Matrix3 &m, MatrixD3 &dest)C
{
   Dbl x, y, z;
            x=T.x.x;  y=T.x.y;  z=T.x.z;
   dest.x.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.x.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.x.z=x*m.z.x + y*m.z.y + z*m.z.z;

            x=T.y.x;  y=T.y.y;  z=T.y.z;
   dest.y.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.y.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.y.z=x*m.z.x + y*m.z.y + z*m.z.z;

            x=T.z.x;  y=T.z.y;  z=T.z.z;
   dest.z.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.z.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.z.z=x*m.z.x + y*m.z.y + z*m.z.z;
}
void MatrixD3::divNormalized(C MatrixD3 &m, MatrixD3 &dest)C
{
   if(&dest!=&m) // here it's OK if "&dest==this"
   {
      Dbl x, y, z;
               x=T.x.x;  y=T.x.y;  z=T.x.z;
      dest.x.x=x*m.x.x + y*m.x.y + z*m.x.z;
      dest.x.y=x*m.y.x + y*m.y.y + z*m.y.z;
      dest.x.z=x*m.z.x + y*m.z.y + z*m.z.z;

               x=T.y.x;  y=T.y.y;  z=T.y.z;
      dest.y.x=x*m.x.x + y*m.x.y + z*m.x.z;
      dest.y.y=x*m.y.x + y*m.y.y + z*m.y.z;
      dest.y.z=x*m.z.x + y*m.z.y + z*m.z.z;

               x=T.z.x;  y=T.z.y;  z=T.z.z;
      dest.z.x=x*m.x.x + y*m.x.y + z*m.x.z;
      dest.z.y=x*m.y.x + y*m.y.y + z*m.y.z;
      dest.z.z=x*m.z.x + y*m.z.y + z*m.z.z;
   }else
   if(&dest!=this) // here it's OK if "&dest==&m"
   {
      Dbl x_x=m.x.x, x_y=m.x.y, x_z=m.x.z;
                     dest.x.x=x_x*T.x.x + x_y*T.x.y + x_z*T.x.z;
      Dbl y_x=m.y.x; dest.y.x=x_x*T.y.x + x_y*T.y.y + x_z*T.y.z;
      Dbl z_x=m.z.x; dest.z.x=x_x*T.z.x + x_y*T.z.y + x_z*T.z.z;

      Dbl y_y=m.y.y, y_z=m.y.z;
                     dest.x.y=y_x*T.x.x + y_y*T.x.y + y_z*T.x.z;
                     dest.y.y=y_x*T.y.x + y_y*T.y.y + y_z*T.y.z;
      Dbl z_y=m.z.y; dest.z.y=y_x*T.z.x + y_y*T.z.y + y_z*T.z.z;

      Dbl z_z=m.z.z;
      dest.x.z=z_x*T.x.x + z_y*T.x.y + z_z*T.x.z;
      dest.y.z=z_x*T.y.x + z_y*T.y.y + z_z*T.y.z;
      dest.z.z=z_x*T.z.x + z_y*T.z.y + z_z*T.z.z;
   }else
   {
      MatrixD3 temp=T; temp.divNormalized(temp, dest); // have to operate on a temporary because all matrixes are the same
   }
}
/******************************************************************************/
void Matrix2P::divNormalized(C Matrix2 &m, Matrix2P &dest)C
{
   Flt x=pos.x, y=pos.y;
   dest.pos.x=x*m.x.x + y*m.x.y;
   dest.pos.y=x*m.y.x + y*m.y.y;

   super::divNormalized(m, dest);
}
void Matrix::divNormalized(C Matrix3 &m, Matrix &dest)C
{
   Flt x=pos.x, y=pos.y, z=pos.z;
   dest.pos.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.pos.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.pos.z=x*m.z.x + y*m.z.y + z*m.z.z;

   super::divNormalized(m, dest);
}
void MatrixM::divNormalized(C Matrix3 &m, MatrixM &dest)C
{
   Dbl x=pos.x, y=pos.y, z=pos.z;
   dest.pos.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.pos.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.pos.z=x*m.z.x + y*m.z.y + z*m.z.z;

   super::divNormalized(m, dest);
}
void MatrixD::divNormalized(C MatrixD3 &m, MatrixD &dest)C
{
   Dbl x=pos.x, y=pos.y, z=pos.z;
   dest.pos.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.pos.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.pos.z=x*m.z.x + y*m.z.y + z*m.z.z;

   super::divNormalized(m, dest);
}
void Matrix2P::divNormalized(C Matrix2P &m, Matrix2P &dest)C
{
   Flt x=pos.x-m.pos.x, y=pos.y-m.pos.y;
   dest.pos.x=x*m.x.x + y*m.x.y;
   dest.pos.y=x*m.y.x + y*m.y.y;

   super::divNormalized(m, dest);
}
void Matrix::divNormalized(C Matrix &m, Matrix &dest)C
{
   Flt x=pos.x-m.pos.x, y=pos.y-m.pos.y, z=pos.z-m.pos.z;
   dest.pos.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.pos.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.pos.z=x*m.z.x + y*m.z.y + z*m.z.z;

   super::divNormalized(m, dest);
}
void Matrix::divNormalized(C MatrixM &m, Matrix &dest)C
{
   Dbl x=pos.x-m.pos.x, y=pos.y-m.pos.y, z=pos.z-m.pos.z;
   dest.pos.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.pos.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.pos.z=x*m.z.x + y*m.z.y + z*m.z.z;

   super::divNormalized(m, dest);
}
void MatrixM::divNormalized(C MatrixM &m, Matrix &dest)C
{
   Dbl x=pos.x-m.pos.x, y=pos.y-m.pos.y, z=pos.z-m.pos.z;
   dest.pos.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.pos.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.pos.z=x*m.z.x + y*m.z.y + z*m.z.z;

   super::divNormalized(m, dest);
}
void MatrixM::divNormalized(C Matrix &m, MatrixM &dest)C
{
   Dbl x=pos.x-m.pos.x, y=pos.y-m.pos.y, z=pos.z-m.pos.z;
   dest.pos.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.pos.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.pos.z=x*m.z.x + y*m.z.y + z*m.z.z;

   super::divNormalized(m, dest);
}
void MatrixM::divNormalized(C MatrixM &m, MatrixM &dest)C
{
   Dbl x=pos.x-m.pos.x, y=pos.y-m.pos.y, z=pos.z-m.pos.z;
   dest.pos.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.pos.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.pos.z=x*m.z.x + y*m.z.y + z*m.z.z;

   super::divNormalized(m, dest);
}
void MatrixD::divNormalized(C MatrixD &m, MatrixD &dest)C
{
   Dbl x=pos.x-m.pos.x, y=pos.y-m.pos.y, z=pos.z-m.pos.z;
   dest.pos.x=x*m.x.x + y*m.x.y + z*m.x.z;
   dest.pos.y=x*m.y.x + y*m.y.y + z*m.y.z;
   dest.pos.z=x*m.z.x + y*m.z.y + z*m.z.z;

   super::divNormalized(m, dest);
}
/******************************************************************************/
void Matrix2 ::div(C Matrix2  &m, Matrix2  &dest)C {Matrix2  temp; m.inverse(temp); mul(temp, dest);}
void Matrix2 ::div(C Matrix2P &m, Matrix2P &dest)C {Matrix2P temp; m.inverse(temp); mul(temp, dest);}
void Matrix3 ::div(C Matrix3  &m, Matrix3  &dest)C {Matrix3  temp; m.inverse(temp); mul(temp, dest);}
void Matrix3 ::div(C Matrix   &m, Matrix   &dest)C {Matrix   temp; m.inverse(temp); mul(temp, dest);}
void Matrix3 ::div(C MatrixM  &m, MatrixM  &dest)C {MatrixM  temp; m.inverse(temp); mul(temp, dest);}
void MatrixD3::div(C Matrix3  &m, MatrixD3 &dest)C {Matrix3  temp; m.inverse(temp); mul(temp, dest);}
void MatrixD3::div(C MatrixD3 &m, MatrixD3 &dest)C {MatrixD3 temp; m.inverse(temp); mul(temp, dest);}
void MatrixD3::div(C MatrixD  &m, MatrixD  &dest)C {MatrixD  temp; m.inverse(temp); mul(temp, dest);}
void Matrix2P::div(C Matrix2  &m, Matrix2P &dest)C {Matrix2  temp; m.inverse(temp); mul(temp, dest);}
void Matrix  ::div(C Matrix3  &m, Matrix   &dest)C {Matrix3  temp; m.inverse(temp); mul(temp, dest);}
void MatrixM ::div(C Matrix3  &m, MatrixM  &dest)C {Matrix3  temp; m.inverse(temp); mul(temp, dest);}
void MatrixD ::div(C MatrixD3 &m, MatrixD  &dest)C {MatrixD3 temp; m.inverse(temp); mul(temp, dest);}
void Matrix2P::div(C Matrix2P &m, Matrix2P &dest)C {Matrix2P temp; m.inverse(temp); mul(temp, dest);}
void Matrix  ::div(C Matrix   &m, Matrix   &dest)C {Matrix   temp; m.inverse(temp); mul(temp, dest);}
void Matrix  ::div(C MatrixM  &m, Matrix   &dest)C {MatrixM  temp; m.inverse(temp); mul(temp, dest);}
void Matrix  ::div(C MatrixM  &m, MatrixM  &dest)C {MatrixM  temp; m.inverse(temp); mul(temp, dest);}
void MatrixM ::div(C MatrixM  &m, Matrix   &dest)C {MatrixM  temp; m.inverse(temp); mul(temp, dest);}
void MatrixM ::div(C Matrix   &m, MatrixM  &dest)C {Matrix   temp; m.inverse(temp); mul(temp, dest);}
void MatrixM ::div(C MatrixM  &m, MatrixM  &dest)C {MatrixM  temp; m.inverse(temp); mul(temp, dest);}
void MatrixD ::div(C MatrixD  &m, MatrixD  &dest)C {MatrixD  temp; m.inverse(temp); mul(temp, dest);}
/******************************************************************************/
Matrix2& Matrix2::inverseScale()
{
   if(Flt l=x.length2())x/=l;
   if(Flt l=y.length2())y/=l;
   return T;
}
Matrix3& Matrix3::inverseScale()
{
   if(Flt l=x.length2())x/=l;
   if(Flt l=y.length2())y/=l;
   if(Flt l=z.length2())z/=l;
   return T;
}
MatrixD3& MatrixD3::inverseScale()
{
   if(Dbl l=x.length2())x/=l;
   if(Dbl l=y.length2())y/=l;
   if(Dbl l=z.length2())z/=l;
   return T;
}
void Matrix2::inverse(Matrix2 &dest, Bool normalized)C
{
 //dest=transpose(T);
 //dest.inverseScale();
   if(&dest!=this)
   {
      dest.x.x=x.x;
      dest.y.y=y.y;
   }
   Flt temp;
   temp=x.y; dest.x.y=y.x; dest.y.x=temp;
   if(!normalized)dest.inverseScale();
}
void Matrix3::inverse(Matrix3 &dest, Bool normalized)C
{
 //dest=transpose(T);
 //dest.inverseScale();
   if(&dest!=this)
   {
      dest.x.x=x.x;
      dest.y.y=y.y;
      dest.z.z=z.z;
   }
   Flt temp;
   temp=x.y; dest.x.y=y.x; dest.y.x=temp;
   temp=x.z; dest.x.z=z.x; dest.z.x=temp;
   temp=y.z; dest.y.z=z.y; dest.z.y=temp;
   if(!normalized)dest.inverseScale();
}
void MatrixD3::inverse(MatrixD3 &dest, Bool normalized)C
{
   if(&dest!=this)
   {
      dest.x.x=x.x;
      dest.y.y=y.y;
      dest.z.z=z.z;
   }
   Dbl temp;
   temp=x.y; dest.x.y=y.x; dest.y.x=temp;
   temp=x.z; dest.x.z=z.x; dest.z.x=temp;
   temp=y.z; dest.y.z=z.y; dest.z.y=temp;
   if(!normalized)dest.inverseScale();
}
void Matrix3::inverseNonOrthogonal(Matrix3 &dest)C
{
   Flt f1=(y.y*z.z - z.y*y.z),
       f2=(z.y*x.z - x.y*z.z),
       f3=(x.y*y.z - y.y*x.z),
      det=x.x*f1 + y.x*f2 + z.x*f3;
   if(det)
   {
      det=1/det;
      if(&dest==this)
      {
         Matrix3 temp;
         temp.x.x=det*f1;
         temp.x.y=det*f2;
         temp.x.z=det*f3;
         temp.y.x=det*(z.x*y.z - y.x*z.z);
         temp.y.y=det*(x.x*z.z - z.x*x.z);
         temp.y.z=det*(y.x*x.z - x.x*y.z);
         temp.z.x=det*(y.x*z.y - z.x*y.y);
         temp.z.y=det*(z.x*x.y - x.x*z.y);
         temp.z.z=det*(x.x*y.y - y.x*x.y);
         dest=temp;
      }else
      {
         dest.x.x=det*f1;
         dest.x.y=det*f2;
         dest.x.z=det*f3;
         dest.y.x=det*(z.x*y.z - y.x*z.z);
         dest.y.y=det*(x.x*z.z - z.x*x.z);
         dest.y.z=det*(y.x*x.z - x.x*y.z);
         dest.z.x=det*(y.x*z.y - z.x*y.y);
         dest.z.y=det*(z.x*x.y - x.x*z.y);
         dest.z.z=det*(x.x*y.y - y.x*x.y);
      }
   }
}
void MatrixD3::inverseNonOrthogonal(MatrixD3 &dest)C
{
   Dbl f1=(y.y*z.z - z.y*y.z),
       f2=(z.y*x.z - x.y*z.z),
       f3=(x.y*y.z - y.y*x.z),
      det=x.x*f1 + y.x*f2 + z.x*f3;
   if(det)
   {
      det=1/det;
      if(&dest==this)
      {
         MatrixD3 temp;
         temp.x.x=det*f1;
         temp.x.y=det*f2;
         temp.x.z=det*f3;
         temp.y.x=det*(z.x*y.z - y.x*z.z);
         temp.y.y=det*(x.x*z.z - z.x*x.z);
         temp.y.z=det*(y.x*x.z - x.x*y.z);
         temp.z.x=det*(y.x*z.y - z.x*y.y);
         temp.z.y=det*(z.x*x.y - x.x*z.y);
         temp.z.z=det*(x.x*y.y - y.x*x.y);
         dest=temp;
      }else
      {
         dest.x.x=det*f1;
         dest.x.y=det*f2;
         dest.x.z=det*f3;
         dest.y.x=det*(z.x*y.z - y.x*z.z);
         dest.y.y=det*(x.x*z.z - z.x*x.z);
         dest.y.z=det*(y.x*x.z - x.x*y.z);
         dest.z.x=det*(y.x*z.y - z.x*y.y);
         dest.z.y=det*(z.x*x.y - x.x*z.y);
         dest.z.z=det*(x.x*y.y - y.x*x.y);
      }
   }
}
void Matrix2P::inverse(Matrix2P &dest, Bool normalized)C
{
   super::inverse(dest.orn(), normalized);
   Flt x=pos.x, y=pos.y;
   dest.pos.x=-(x*dest.x.x + y*dest.y.x);
   dest.pos.y=-(x*dest.x.y + y*dest.y.y);
}
void Matrix::inverse(Matrix &dest, Bool normalized)C
{
   super::inverse(dest.orn(), normalized);
   Flt x=pos.x, y=pos.y, z=pos.z;
   dest.pos.x=-(x*dest.x.x + y*dest.y.x + z*dest.z.x);
   dest.pos.y=-(x*dest.x.y + y*dest.y.y + z*dest.z.y);
   dest.pos.z=-(x*dest.x.z + y*dest.y.z + z*dest.z.z);
}
void MatrixM::inverse(MatrixM &dest, Bool normalized)C
{
   super::inverse(dest.orn(), normalized);
   Dbl x=pos.x, y=pos.y, z=pos.z;
   dest.pos.x=-(x*dest.x.x + y*dest.y.x + z*dest.z.x);
   dest.pos.y=-(x*dest.x.y + y*dest.y.y + z*dest.z.y);
   dest.pos.z=-(x*dest.x.z + y*dest.y.z + z*dest.z.z);
}
void MatrixD::inverse(MatrixD &dest, Bool normalized)C
{
   super::inverse(dest.orn(), normalized);
   Dbl x=pos.x, y=pos.y, z=pos.z;
   dest.pos.x=-(x*dest.x.x + y*dest.y.x + z*dest.z.x);
   dest.pos.y=-(x*dest.x.y + y*dest.y.y + z*dest.z.y);
   dest.pos.z=-(x*dest.x.z + y*dest.y.z + z*dest.z.z);
}
void Matrix::inverseNonOrthogonal(Matrix &dest)C
{
   super::inverseNonOrthogonal(dest.orn());
   Flt x=pos.x, y=pos.y, z=pos.z;
   dest.pos.x=-(x*dest.x.x + y*dest.y.x + z*dest.z.x);
   dest.pos.y=-(x*dest.x.y + y*dest.y.y + z*dest.z.y);
   dest.pos.z=-(x*dest.x.z + y*dest.y.z + z*dest.z.z);
}
void MatrixM::inverseNonOrthogonal(MatrixM &dest)C
{
   super::inverseNonOrthogonal(dest.orn());
   Dbl x=pos.x, y=pos.y, z=pos.z;
   dest.pos.x=-(x*dest.x.x + y*dest.y.x + z*dest.z.x);
   dest.pos.y=-(x*dest.x.y + y*dest.y.y + z*dest.z.y);
   dest.pos.z=-(x*dest.x.z + y*dest.y.z + z*dest.z.z);
}
void MatrixD::inverseNonOrthogonal(MatrixD &dest)C
{
   super::inverseNonOrthogonal(dest.orn());
   Dbl x=pos.x, y=pos.y, z=pos.z;
   dest.pos.x=-(x*dest.x.x + y*dest.y.x + z*dest.z.x);
   dest.pos.y=-(x*dest.x.y + y*dest.y.y + z*dest.z.y);
   dest.pos.z=-(x*dest.x.z + y*dest.y.z + z*dest.z.z);
}
Matrix4& Matrix4::inverse()
{
   if(Flt det=determinant())
   {
      Matrix4 t;
      t.x  .x=y.z*z.w*pos.y - y.w*z.z*pos.y + y.w*z.y*pos.z - y.y*z.w*pos.z - y.z*z.y*pos.w + y.y*z.z*pos.w;
      t.x  .y=x.w*z.z*pos.y - x.z*z.w*pos.y - x.w*z.y*pos.z + x.y*z.w*pos.z + x.z*z.y*pos.w - x.y*z.z*pos.w;
      t.x  .z=x.z*y.w*pos.y - x.w*y.z*pos.y + x.w*y.y*pos.z - x.y*y.w*pos.z - x.z*y.y*pos.w + x.y*y.z*pos.w;
      t.x  .w=x.w*y.z*  z.y - x.z*y.w*  z.y - x.w*y.y*  z.z + x.y*y.w*  z.z + x.z*y.y*  z.w - x.y*y.z*  z.w;
      t.y  .x=y.w*z.z*pos.x - y.z*z.w*pos.x - y.w*z.x*pos.z + y.x*z.w*pos.z + y.z*z.x*pos.w - y.x*z.z*pos.w;
      t.y  .y=x.z*z.w*pos.x - x.w*z.z*pos.x + x.w*z.x*pos.z - x.x*z.w*pos.z - x.z*z.x*pos.w + x.x*z.z*pos.w;
      t.y  .z=x.w*y.z*pos.x - x.z*y.w*pos.x - x.w*y.x*pos.z + x.x*y.w*pos.z + x.z*y.x*pos.w - x.x*y.z*pos.w;
      t.y  .w=x.z*y.w*  z.x - x.w*y.z*  z.x + x.w*y.x*  z.z - x.x*y.w*  z.z - x.z*y.x*  z.w + x.x*y.z*  z.w;
      t.z  .x=y.y*z.w*pos.x - y.w*z.y*pos.x + y.w*z.x*pos.y - y.x*z.w*pos.y - y.y*z.x*pos.w + y.x*z.y*pos.w;
      t.z  .y=x.w*z.y*pos.x - x.y*z.w*pos.x - x.w*z.x*pos.y + x.x*z.w*pos.y + x.y*z.x*pos.w - x.x*z.y*pos.w;
      t.z  .z=x.y*y.w*pos.x - x.w*y.y*pos.x + x.w*y.x*pos.y - x.x*y.w*pos.y - x.y*y.x*pos.w + x.x*y.y*pos.w;
      t.z  .w=x.w*y.y*  z.x - x.y*y.w*  z.x - x.w*y.x*  z.y + x.x*y.w*  z.y + x.y*y.x*  z.w - x.x*y.y*  z.w;
      t.pos.x=y.z*z.y*pos.x - y.y*z.z*pos.x - y.z*z.x*pos.y + y.x*z.z*pos.y + y.y*z.x*pos.z - y.x*z.y*pos.z;
      t.pos.y=x.y*z.z*pos.x - x.z*z.y*pos.x + x.z*z.x*pos.y - x.x*z.z*pos.y - x.y*z.x*pos.z + x.x*z.y*pos.z;
      t.pos.z=x.z*y.y*pos.x - x.y*y.z*pos.x - x.z*y.x*pos.y + x.x*y.z*pos.y + x.y*y.x*pos.z - x.x*y.y*pos.z;
      t.pos.w=x.y*y.z*  z.x - x.z*y.y*  z.x + x.z*y.x*  z.y - x.x*y.z*  z.y - x.y*y.x*  z.z + x.x*y.y*  z.z;

      det=1/det;
      x  =t.x  *det;
      y  =t.y  *det;
      z  =t.z  *det;
      pos=t.pos*det;
   }
   return T;
}
Matrix4& Matrix4::transpose()
{
   Swap(x.y, y.x);
   Swap(x.z, z.x);
   Swap(x.w, pos.x);
   Swap(y.z, z.y);
   Swap(y.w, pos.y);
   Swap(z.w, pos.z);
   return T;
}
/******************************************************************************/
Matrix2& Matrix2::normalize()
{
   if(!x.normalize()
   || !y.normalize())identity();
   return T;
}
Matrix3& Matrix3::normalize()
{
   if(!x.normalize()
   || !y.normalize()
   || !z.normalize())identity();
   return T;
}
MatrixD3& MatrixD3::normalize()
{
   if(!x.normalize()
   || !y.normalize()
   || !z.normalize())identity();
   return T;
}
Matrix2& Matrix2::normalize(Flt scale)
{
   if(!x.setLength(scale)
   || !y.setLength(scale))setScale(scale);
   return T;
}
Matrix3& Matrix3::normalize(Flt scale)
{
   if(!x.setLength(scale)
   || !y.setLength(scale)
   || !z.setLength(scale))setScale(scale);
   return T;
}
MatrixD3& MatrixD3::normalize(Dbl scale)
{
   if(!x.setLength(scale)
   || !y.setLength(scale)
   || !z.setLength(scale))setScale(scale);
   return T;
}
Matrix2& Matrix2::normalize(C Vec2 &scale)
{
   if(!x.setLength(scale.x)
   || !y.setLength(scale.y))setScale(scale);
   return T;
}
Matrix3& Matrix3::normalize(C Vec &scale)
{
   if(!x.setLength(scale.x)
   || !y.setLength(scale.y)
   || !z.setLength(scale.z))setScale(scale);
   return T;
}
MatrixD3& MatrixD3::normalize(C VecD &scale)
{
   if(!x.setLength(scale.x)
   || !y.setLength(scale.y)
   || !z.setLength(scale.z))setScale(scale);
   return T;
}
/******************************************************************************/
Matrix2& Matrix2::scaleL(C Vec2 &scale)
{
   x*=scale.x;
   y*=scale.y;
   return T;
}
Matrix3& Matrix3::scaleL(C Vec &scale)
{
   x*=scale.x;
   y*=scale.y;
   z*=scale.z;
   return T;
}
Matrix2P& Matrix2P::scaleL(C Vec2 &scale)
{
   // adjust position before scaling axes, in case 'scale' is zero and would destroy them
   Vec2 dir; Flt d;
   if(d=scale.x-1){dir=x; dir.normalize(); pos+=dir*(DistPointPlane(pos, dir)*d);}
   if(d=scale.y-1){dir=y; dir.normalize(); pos+=dir*(DistPointPlane(pos, dir)*d);}
   super::scaleL(scale);
   return T;
}
Matrix& Matrix::scaleL(C Vec &scale)
{
   // adjust position before scaling axes, in case 'scale' is zero and would destroy them
   Vec dir; Flt d;
   if(d=scale.x-1){dir=x; dir.normalize(); pos+=dir*(DistPointPlane(pos, dir)*d);}
   if(d=scale.y-1){dir=y; dir.normalize(); pos+=dir*(DistPointPlane(pos, dir)*d);}
   if(d=scale.z-1){dir=z; dir.normalize(); pos+=dir*(DistPointPlane(pos, dir)*d);}
   super::scaleL(scale);
   return T;
}
/******************************************************************************/
Matrix3& Matrix3::scale(C Vec &dir, Flt scale)
{
 //Flt plane_dist=DistPointPlane(axis, dir); Flt wanted_plane_dist=plane_dist*scale; axis+=dir*(wanted_plane_dist-plane_dist);
 //axis+=dir*(DistPointPlane(axis, dir)*(scale-1));

   scale--;
   x+=dir*(DistPointPlane(x, dir)*scale);
   y+=dir*(DistPointPlane(y, dir)*scale);
   z+=dir*(DistPointPlane(z, dir)*scale);
   return T;
}
MatrixD3& MatrixD3::scale(C VecD &dir, Dbl scale)
{
   scale--;
   x+=dir*(DistPointPlane(x, dir)*scale);
   y+=dir*(DistPointPlane(y, dir)*scale);
   z+=dir*(DistPointPlane(z, dir)*scale);
   return T;
}
Matrix& Matrix::scale(C Vec &dir, Flt scale)
{
   scale--;
   x  +=dir*(DistPointPlane(x  , dir)*scale);
   y  +=dir*(DistPointPlane(y  , dir)*scale);
   z  +=dir*(DistPointPlane(z  , dir)*scale);
   pos+=dir*(DistPointPlane(pos, dir)*scale);
   return T;
}
MatrixD& MatrixD::scale(C VecD &dir, Dbl scale)
{
   scale--;
   x  +=dir*(DistPointPlane(x  , dir)*scale);
   y  +=dir*(DistPointPlane(y  , dir)*scale);
   z  +=dir*(DistPointPlane(z  , dir)*scale);
   pos+=dir*(DistPointPlane(pos, dir)*scale);
   return T;
}
Matrix3& Matrix3::scalePlane(C Vec &nrm, Flt scale)
{
            T.scale(scale       ); // first             scale globally
   if(scale)T.scale(nrm, 1/scale); // then perform axis-scale by reverse of 'scale'
   return T;
}
MatrixD3& MatrixD3::scalePlane(C VecD &nrm, Dbl scale)
{
            T.scale(scale       ); // first             scale globally
   if(scale)T.scale(nrm, 1/scale); // then perform axis-scale by reverse of 'scale'
   return T;
}
Matrix& Matrix::scalePlane(C Vec &nrm, Flt scale)
{
            T.scale(scale       ); // first             scale globally
   if(scale)T.scale(nrm, 1/scale); // then perform axis-scale by reverse of 'scale'
   return T;
}
MatrixD& MatrixD::scalePlane(C VecD &nrm, Dbl scale)
{
            T.scale(scale       ); // first             scale globally
   if(scale)T.scale(nrm, 1/scale); // then perform axis-scale by reverse of 'scale'
   return T;
}
/******************************************************************************/
Matrix2& Matrix2::rotate(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=T.x.x, y=T.x.y;
      T.x.x=x*cos - y*sin;
      T.x.y=y*cos + x*sin;

      x=T.y.x; y=T.y.y;
      T.y.x=x*cos - y*sin;
      T.y.y=y*cos + x*sin;
   }
   return T;
}
/******************************************************************************/
Matrix3& Matrix3::rotateX(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt y=T.x.y, z=T.x.z;
      T.x.y=y*cos - z*sin;
      T.x.z=y*sin + z*cos;

      y=T.y.y; z=T.y.z;
      T.y.y=y*cos - z*sin;
      T.y.z=y*sin + z*cos;

      y=T.z.y; z=T.z.z;
      T.z.y=y*cos - z*sin;
      T.z.z=y*sin + z*cos;
   }
   return T;
}
Matrix3& Matrix3::rotateY(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=T.x.x, z=T.x.z;
      T.x.x=x*cos + z*sin;
      T.x.z=z*cos - x*sin;

      x=T.y.x; z=T.y.z;
      T.y.x=x*cos + z*sin;
      T.y.z=z*cos - x*sin;

      x=T.z.x; z=T.z.z;
      T.z.x=x*cos + z*sin;
      T.z.z=z*cos - x*sin;
   }
   return T;
}
Matrix3& Matrix3::rotateZ(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=T.x.x, y=T.x.y;
      T.x.x=x*cos - y*sin;
      T.x.y=y*cos + x*sin;

      x=T.y.x; y=T.y.y;
      T.y.x=x*cos - y*sin;
      T.y.y=y*cos + x*sin;

      x=T.z.x; y=T.z.y;
      T.z.x=x*cos - y*sin;
      T.z.y=y*cos + x*sin;
   }
   return T;
}
/******************************************************************************/
MatrixD3& MatrixD3::rotateX(Dbl angle)
{
   if(Any(angle))
   {
      Dbl cos, sin; CosSin(cos, sin, angle);

      Dbl y=T.x.y, z=T.x.z;
      T.x.y=y*cos - z*sin;
      T.x.z=y*sin + z*cos;

      y=T.y.y; z=T.y.z;
      T.y.y=y*cos - z*sin;
      T.y.z=y*sin + z*cos;

      y=T.z.y; z=T.z.z;
      T.z.y=y*cos - z*sin;
      T.z.z=y*sin + z*cos;
   }
   return T;
}
MatrixD3& MatrixD3::rotateY(Dbl angle)
{
   if(Any(angle))
   {
      Dbl cos, sin; CosSin(cos, sin, angle);

      Dbl x=T.x.x, z=T.x.z;
      T.x.x=x*cos + z*sin;
      T.x.z=z*cos - x*sin;

      x=T.y.x; z=T.y.z;
      T.y.x=x*cos + z*sin;
      T.y.z=z*cos - x*sin;

      x=T.z.x; z=T.z.z;
      T.z.x=x*cos + z*sin;
      T.z.z=z*cos - x*sin;
   }
   return T;
}
MatrixD3& MatrixD3::rotateZ(Dbl angle)
{
   if(Any(angle))
   {
      Dbl cos, sin; CosSin(cos, sin, angle);

      Dbl x=T.x.x, y=T.x.y;
      T.x.x=x*cos - y*sin;
      T.x.y=y*cos + x*sin;

      x=T.y.x; y=T.y.y;
      T.y.x=x*cos - y*sin;
      T.y.y=y*cos + x*sin;

      x=T.z.x; y=T.z.y;
      T.z.x=x*cos - y*sin;
      T.z.y=y*cos + x*sin;
   }
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::rotate(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=T.x.x, y=T.x.y;
      T.x.x=x*cos - y*sin;
      T.x.y=y*cos + x*sin;

      x=T.y.x; y=T.y.y;
      T.y.x=x*cos - y*sin;
      T.y.y=y*cos + x*sin;

      x=pos.x; y=pos.y;
      pos.x=x*cos - y*sin;
      pos.y=y*cos + x*sin;
   }
   return T;
}
/******************************************************************************/
Matrix& Matrix::rotateX(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt y=T.x.y, z=T.x.z;
      T.x.y=y*cos - z*sin;
      T.x.z=y*sin + z*cos;

      y=T.y.y; z=T.y.z;
      T.y.y=y*cos - z*sin;
      T.y.z=y*sin + z*cos;

      y=T.z.y; z=T.z.z;
      T.z.y=y*cos - z*sin;
      T.z.z=y*sin + z*cos;

      y=pos.y; z=pos.z;
      pos.y=y*cos - z*sin;
      pos.z=y*sin + z*cos;
   }
   return T;
}
Matrix& Matrix::rotateY(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=T.x.x, z=T.x.z;
      T.x.x=x*cos + z*sin;
      T.x.z=z*cos - x*sin;

      x=T.y.x; z=T.y.z;
      T.y.x=x*cos + z*sin;
      T.y.z=z*cos - x*sin;

      x=T.z.x; z=T.z.z;
      T.z.x=x*cos + z*sin;
      T.z.z=z*cos - x*sin;

      x=pos.x; z=pos.z;
      pos.x=x*cos + z*sin;
      pos.z=z*cos - x*sin;
   }
   return T;
}
Matrix& Matrix::rotateZ(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=T.x.x, y=T.x.y;
      T.x.x=x*cos - y*sin;
      T.x.y=y*cos + x*sin;

      x=T.y.x; y=T.y.y;
      T.y.x=x*cos - y*sin;
      T.y.y=y*cos + x*sin;

      x=T.z.x; y=T.z.y;
      T.z.x=x*cos - y*sin;
      T.z.y=y*cos + x*sin;

      x=pos.x; y=pos.y;
      pos.x=x*cos - y*sin;
      pos.y=y*cos + x*sin;
   }
   return T;
}
/******************************************************************************/
MatrixM& MatrixM::rotateX(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt y=T.x.y, z=T.x.z;
      T.x.y=y*cos - z*sin;
      T.x.z=y*sin + z*cos;

      y=T.y.y; z=T.y.z;
      T.y.y=y*cos - z*sin;
      T.y.z=y*sin + z*cos;

      y=T.z.y; z=T.z.z;
      T.z.y=y*cos - z*sin;
      T.z.z=y*sin + z*cos;
      {
         Dbl y=pos.y, z=pos.z;
         pos.y=y*cos - z*sin;
         pos.z=y*sin + z*cos;
      }
   }
   return T;
}
MatrixM& MatrixM::rotateY(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=T.x.x, z=T.x.z;
      T.x.x=x*cos + z*sin;
      T.x.z=z*cos - x*sin;

      x=T.y.x; z=T.y.z;
      T.y.x=x*cos + z*sin;
      T.y.z=z*cos - x*sin;

      x=T.z.x; z=T.z.z;
      T.z.x=x*cos + z*sin;
      T.z.z=z*cos - x*sin;
      {
         Dbl x=pos.x, z=pos.z;
         pos.x=x*cos + z*sin;
         pos.z=z*cos - x*sin;
      }
   }
   return T;
}
MatrixM& MatrixM::rotateZ(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=T.x.x, y=T.x.y;
      T.x.x=x*cos - y*sin;
      T.x.y=y*cos + x*sin;

      x=T.y.x; y=T.y.y;
      T.y.x=x*cos - y*sin;
      T.y.y=y*cos + x*sin;

      x=T.z.x; y=T.z.y;
      T.z.x=x*cos - y*sin;
      T.z.y=y*cos + x*sin;
      {
         Dbl x=pos.x, y=pos.y;
         pos.x=x*cos - y*sin;
         pos.y=y*cos + x*sin;
      }
   }
   return T;
}
/******************************************************************************/
MatrixD& MatrixD::rotateX(Dbl angle)
{
   if(Any(angle))
   {
      Dbl cos, sin; CosSin(cos, sin, angle);

      Dbl y=T.x.y, z=T.x.z;
      T.x.y=y*cos - z*sin;
      T.x.z=y*sin + z*cos;

      y=T.y.y; z=T.y.z;
      T.y.y=y*cos - z*sin;
      T.y.z=y*sin + z*cos;

      y=T.z.y; z=T.z.z;
      T.z.y=y*cos - z*sin;
      T.z.z=y*sin + z*cos;

      y=pos.y; z=pos.z;
      pos.y=y*cos - z*sin;
      pos.z=y*sin + z*cos;
   }
   return T;
}
MatrixD& MatrixD::rotateY(Dbl angle)
{
   if(Any(angle))
   {
      Dbl cos, sin; CosSin(cos, sin, angle);

      Dbl x=T.x.x, z=T.x.z;
      T.x.x=x*cos + z*sin;
      T.x.z=z*cos - x*sin;

      x=T.y.x; z=T.y.z;
      T.y.x=x*cos + z*sin;
      T.y.z=z*cos - x*sin;

      x=T.z.x; z=T.z.z;
      T.z.x=x*cos + z*sin;
      T.z.z=z*cos - x*sin;

      x=pos.x; z=pos.z;
      pos.x=x*cos + z*sin;
      pos.z=z*cos - x*sin;
   }
   return T;
}
MatrixD& MatrixD::rotateZ(Dbl angle)
{
   if(Any(angle))
   {
      Dbl cos, sin; CosSin(cos, sin, angle);

      Dbl x=T.x.x, y=T.x.y;
      T.x.x=x*cos - y*sin;
      T.x.y=y*cos + x*sin;

      x=T.y.x; y=T.y.y;
      T.y.x=x*cos - y*sin;
      T.y.y=y*cos + x*sin;

      x=T.z.x; y=T.z.y;
      T.z.x=x*cos - y*sin;
      T.z.y=y*cos + x*sin;

      x=pos.x; y=pos.y;
      pos.x=x*cos - y*sin;
      pos.y=y*cos + x*sin;
   }
   return T;
}
/******************************************************************************/
Matrix3 & Matrix3 ::rotateXY(Flt x, Flt y) {if(Any(x, y ))T*=Matrix3 ().setRotateXY(x, y); return T;}
MatrixD3& MatrixD3::rotateXY(Dbl x, Dbl y) {if(Any(x, y ))T*=MatrixD3().setRotateXY(x, y); return T;}
Matrix  & Matrix  ::rotateXY(Flt x, Flt y) {if(Any(x, y ))T*=Matrix3 ().setRotateXY(x, y); return T;}
MatrixM & MatrixM ::rotateXY(Flt x, Flt y) {if(Any(x, y ))T*=Matrix3 ().setRotateXY(x, y); return T;}
MatrixD & MatrixD ::rotateXY(Dbl x, Dbl y) {if(Any(x, y ))T*=MatrixD3().setRotateXY(x, y); return T;}

Matrix3 & Matrix3 ::rotate(C Vec  &axis, Flt angle) {if(Any(angle))T*=Matrix3 ().setRotate(axis, angle); return T;}
MatrixD3& MatrixD3::rotate(C VecD &axis, Dbl angle) {if(Any(angle))T*=MatrixD3().setRotate(axis, angle); return T;}
Matrix  & Matrix  ::rotate(C Vec  &axis, Flt angle) {if(Any(angle))T*=Matrix3 ().setRotate(axis, angle); return T;}
MatrixM & MatrixM ::rotate(C Vec  &axis, Flt angle) {if(Any(angle))T*=Matrix3 ().setRotate(axis, angle); return T;}
MatrixD & MatrixD ::rotate(C VecD &axis, Dbl angle) {if(Any(angle))T*=MatrixD3().setRotate(axis, angle); return T;}

Matrix3& Matrix3::rotateXL(Flt angle) {if(Any(angle))rotate(!x, angle); return T;}
Matrix3& Matrix3::rotateYL(Flt angle) {if(Any(angle))rotate(!y, angle); return T;}
Matrix3& Matrix3::rotateZL(Flt angle) {if(Any(angle))rotate(!z, angle); return T;}

MatrixD3& MatrixD3::rotateXL(Dbl angle) {if(Any(angle))rotate(!x, angle); return T;}
MatrixD3& MatrixD3::rotateYL(Dbl angle) {if(Any(angle))rotate(!y, angle); return T;}
MatrixD3& MatrixD3::rotateZL(Dbl angle) {if(Any(angle))rotate(!z, angle); return T;}
/******************************************************************************/
Matrix3& Matrix3::rotateXLOrthoNormalized(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);
      Vec z=T.z;

      T.z=z*cos - y*sin;
      T.y=z*sin + y*cos;
   }
   return T;
}
Matrix3& Matrix3::rotateXLOrthoNormalized(Flt cos, Flt sin)
{
   Vec z=T.z;
   T.z=z*cos - y*sin;
   T.y=z*sin + y*cos;
   return T;
}
Matrix3& Matrix3::rotateYLOrthoNormalized(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);
      Vec x=T.x;

      T.x=x*cos - z*sin;
      T.z=x*sin + z*cos;
   }
   return T;
}
Matrix3& Matrix3::rotateZLOrthoNormalized(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);
      Vec y=T.y;

      T.y=y*cos - x*sin;
      T.x=y*sin + x*cos;
   }
   return T;
}
/******************************************************************************/
MatrixD3& MatrixD3::rotateXLOrthoNormalized(Dbl angle)
{
   if(Any(angle))
   {
      Dbl cos, sin; CosSin(cos, sin, angle);
      VecD z=T.z;

      T.z=z*cos - y*sin;
      T.y=z*sin + y*cos;
   }
   return T;
}
MatrixD3& MatrixD3::rotateYLOrthoNormalized(Dbl angle)
{
   if(Any(angle))
   {
      Dbl cos, sin; CosSin(cos, sin, angle);
      VecD x=T.x;

      T.x=x*cos - z*sin;
      T.z=x*sin + z*cos;
   }
   return T;
}
MatrixD3& MatrixD3::rotateZLOrthoNormalized(Dbl angle)
{
   if(Any(angle))
   {
      Dbl cos, sin; CosSin(cos, sin, angle);
      VecD y=T.y;

      T.y=y*cos - x*sin;
      T.x=y*sin + x*cos;
   }
   return T;
}
/******************************************************************************/
Matrix3& Matrix3::rotateToYKeepX(C Vec &y)
{
   T.y=y;
   x=PointOnPlane(x, y); if(x.normalize()<=EPS)x=PerpN(y); // try to preserve x
   z=Cross(x, y);
   return T;
}
Matrix3& Matrix3::rotateToYKeepZ(C Vec &y)
{
   T.y=y;
   z=PointOnPlane(z, y); if(z.normalize()<=EPS)z=PerpN(y); // try to preserve z
   x=Cross(y, z);
   return T;
}
Matrix3& Matrix3::rotateToZKeepX(C Vec &z)
{
   T.z=z;
   x=PointOnPlane(x, z); if(x.normalize()<=EPS)x=PerpN(z); // try to preserve x
   y=Cross(z, x);
   return T;
}
Matrix3& Matrix3::rotateToZKeepY(C Vec &z)
{
   T.z=z;
   y=PointOnPlane(y, z); if(y.normalize()<=EPS)y=PerpN(z); // try to preserve y
   x=Cross(y, z);
   return T;
}
/******************************************************************************/
Matrix2& Matrix2::zero()
{
   x.zero();
   y.zero();
   return T;
}
Matrix3& Matrix3::zero()
{
   x.zero();
   y.zero();
   z.zero();
   return T;
}
MatrixD3& MatrixD3::zero()
{
   x.zero();
   y.zero();
   z.zero();
   return T;
}
Matrix2P& Matrix2P::zero()
{
   x  .zero();
   y  .zero();
   pos.zero();
   return T;
}
Matrix& Matrix::zero()
{
   x  .zero();
   y  .zero();
   z  .zero();
   pos.zero();
   return T;
}
MatrixM& MatrixM::zero()
{
   x  .zero();
   y  .zero();
   z  .zero();
   pos.zero();
   return T;
}
MatrixD& MatrixD::zero()
{
   x  .zero();
   y  .zero();
   z  .zero();
   pos.zero();
   return T;
}
Matrix4& Matrix4::zero()
{
   x  .zero();
   y  .zero();
   z  .zero();
   pos.zero();
   return T;
}
/******************************************************************************/
Matrix2& Matrix2::identity()
{
   x.set(1, 0);
   y.set(0, 1);
   return T;
}
Matrix3& Matrix3::identity()
{
   x.set(1, 0, 0);
   y.set(0, 1, 0);
   z.set(0, 0, 1);
   return T;
}
MatrixD3& MatrixD3::identity()
{
   x.set(1, 0, 0);
   y.set(0, 1, 0);
   z.set(0, 0, 1);
   return T;
}
Matrix2P& Matrix2P::identity()
{
   x  .set (1, 0);
   y  .set (0, 1);
   pos.zero(    );
   return T;
}
Matrix& Matrix::identity()
{
   x  .set (1, 0, 0);
   y  .set (0, 1, 0);
   z  .set (0, 0, 1);
   pos.zero(       );
   return T;
}
MatrixM& MatrixM::identity()
{
   x  .set (1, 0, 0);
   y  .set (0, 1, 0);
   z  .set (0, 0, 1);
   pos.zero(       );
   return T;
}
MatrixD& MatrixD::identity()
{
   x  .set (1, 0, 0);
   y  .set (0, 1, 0);
   z  .set (0, 0, 1);
   pos.zero(       );
   return T;
}
Matrix4& Matrix4::identity()
{
   x  .set(1, 0, 0, 0);
   y  .set(0, 1, 0, 0);
   z  .set(0, 0, 1, 0);
   pos.set(0, 0, 0, 1);
   return T;
}
/******************************************************************************/
Matrix3& Matrix3::keep01(Flt blend)
{
   // orientation
   Vec axis; Flt angle=axisAngle(axis);

   // scale
   Vec  scale=T.scale();
   REPA(scale)scale.c[i]=ScaleFactor(ScaleFactorR(scale.c[i])*blend);

   return setRotate(axis, angle*blend).scaleL(scale);
}
Matrix& Matrix::keep01(Flt blend)
{
   super::keep01(blend);
   pos*=blend;
   return T;
}
MatrixM& MatrixM::keep01(Flt blend)
{
   super::keep01(blend);
   pos*=blend;
   return T;
}
/******************************************************************************/
Matrix3& Matrix3::identity(Flt blend)
{
   if(blend>0)
   {
      if(blend>=1)identity();
      else        keep01  (1-blend);
   }
   return T;
}
Matrix& Matrix::identity(Flt blend)
{
   if(blend>0)
   {
      if(blend>=1)identity();
      else        keep01  (1-blend);
   }
   return T;
}
MatrixM& MatrixM::identity(Flt blend)
{
   if(blend>0)
   {
      if(blend>=1)identity();
      else        keep01  (1-blend);
   }
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::setPos(Flt x, Flt y)
{
   T.pos.set(x, y);
   T.x  .set(1, 0);
   T.y  .set(0, 1);
   return T;
}
Matrix& Matrix::setPos(Flt x, Flt y, Flt z)
{
   T.pos.set(x, y, z);
   T.x  .set(1, 0, 0);
   T.y  .set(0, 1, 0);
   T.z  .set(0, 0, 1);
   return T;
}
MatrixM& MatrixM::setPos(Dbl x, Dbl y, Dbl z)
{
   T.pos.set(x, y, z);
   T.x  .set(1, 0, 0);
   T.y  .set(0, 1, 0);
   T.z  .set(0, 0, 1);
   return T;
}
MatrixD& MatrixD::setPos(Dbl x, Dbl y, Dbl z)
{
   T.pos.set(x, y, z);
   T.x  .set(1, 0, 0);
   T.y  .set(0, 1, 0);
   T.z  .set(0, 0, 1);
   return T;
}
/******************************************************************************/
Matrix& Matrix::setPos(C Vec2 &pos)
{
   T.pos.set(pos , 0);
     x  .set(1, 0, 0);
     y  .set(0, 1, 0);
     z  .set(0, 0, 1);
   return T;
}
MatrixM& MatrixM::setPos(C VecD2 &pos)
{
   T.pos.set(pos , 0);
     x  .set(1, 0, 0);
     y  .set(0, 1, 0);
     z  .set(0, 0, 1);
   return T;
}
MatrixD& MatrixD::setPos(C VecD2 &pos)
{
   T.pos.set(pos , 0);
     x  .set(1, 0, 0);
     y  .set(0, 1, 0);
     z  .set(0, 0, 1);
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::setPos(C Vec2 &pos)
{
   T.pos=pos;
     x  .set(1, 0);
     y  .set(0, 1);
   return T;
}
Matrix& Matrix::setPos(C Vec &pos)
{
   T.pos=pos;
     x  .set(1, 0, 0);
     y  .set(0, 1, 0);
     z  .set(0, 0, 1);
   return T;
}
MatrixM& MatrixM::setPos(C VecD &pos)
{
   T.pos=pos;
     x  .set(1, 0, 0);
     y  .set(0, 1, 0);
     z  .set(0, 0, 1);
   return T;
}
MatrixD& MatrixD::setPos(C VecD &pos)
{
   T.pos=pos;
     x  .set(1, 0, 0);
     y  .set(0, 1, 0);
     z  .set(0, 0, 1);
   return T;
}
/******************************************************************************/
Matrix2& Matrix2::setScale(Flt scale)
{
   x.set(scale, 0);
   y.set(0, scale);
   return T;
}
Matrix3& Matrix3::setScale(Flt scale)
{
   x.set(scale, 0, 0);
   y.set(0, scale, 0);
   z.set(0, 0, scale);
   return T;
}
MatrixD3& MatrixD3::setScale(Dbl scale)
{
   x.set(scale, 0, 0);
   y.set(0, scale, 0);
   z.set(0, 0, scale);
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::setScale(Flt scale)
{
   x  .set (scale, 0);
   y  .set (0, scale);
   pos.zero(        );
   return T;
}
Matrix& Matrix::setScale(Flt scale)
{
   x  .set (scale, 0, 0);
   y  .set (0, scale, 0);
   z  .set (0, 0, scale);
   pos.zero(           );
   return T;
}
MatrixM& MatrixM::setScale(Flt scale)
{
   x  .set (scale, 0, 0);
   y  .set (0, scale, 0);
   z  .set (0, 0, scale);
   pos.zero(           );
   return T;
}
MatrixD& MatrixD::setScale(Dbl scale)
{
   x  .set (scale, 0, 0);
   y  .set (0, scale, 0);
   z  .set (0, 0, scale);
   pos.zero(           );
   return T;
}
/******************************************************************************/
Matrix2& Matrix2::setScale(C Vec2 &scale)
{
   x.set(scale.x, 0);
   y.set(0, scale.y);
   return T;
}
Matrix3& Matrix3::setScale(C Vec &scale)
{
   x.set(scale.x, 0, 0);
   y.set(0, scale.y, 0);
   z.set(0, 0, scale.z);
   return T;
}
MatrixD3& MatrixD3::setScale(C VecD &scale)
{
   x.set(scale.x, 0, 0);
   y.set(0, scale.y, 0);
   z.set(0, 0, scale.z);
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::setScale(C Vec2 &scale)
{
   x  .set (scale.x, 0);
   y  .set (0, scale.y);
   pos.zero(          );
   return T;
}
Matrix& Matrix::setScale(C Vec &scale)
{
   x  .set (scale.x, 0, 0);
   y  .set (0, scale.y, 0);
   z  .set (0, 0, scale.z);
   pos.zero(             );
   return T;
}
MatrixM& MatrixM::setScale(C Vec &scale)
{
   x  .set (scale.x, 0, 0);
   y  .set (0, scale.y, 0);
   z  .set (0, 0, scale.z);
   pos.zero(             );
   return T;
}
MatrixD& MatrixD::setScale(C VecD &scale)
{
   x  .set (scale.x, 0, 0);
   y  .set (0, scale.y, 0);
   z  .set (0, 0, scale.z);
   pos.zero(             );
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::setPosScale(C Vec2 &pos, Flt scale)
{
   T.pos.set(pos.x*scale, pos.y*scale);
     x  .set(scale, 0);
     y  .set(0, scale);
   return T;
}
Matrix& Matrix::setPosScale(C Vec &pos, Flt scale)
{
   T.pos.set(pos.x*scale, pos.y*scale, pos.z*scale);
     x  .set(scale, 0, 0);
     y  .set(0, scale, 0);
     z  .set(0, 0, scale);
   return T;
}
MatrixM& MatrixM::setPosScale(C VecD &pos, Flt scale)
{
   T.pos.set(pos.x*scale, pos.y*scale, pos.z*scale);
     x  .set(scale, 0, 0);
     y  .set(0, scale, 0);
     z  .set(0, 0, scale);
   return T;
}
MatrixD& MatrixD::setPosScale(C VecD &pos, Dbl scale)
{
   T.pos.set(pos.x*scale, pos.y*scale, pos.z*scale);
     x  .set(scale, 0, 0);
     y  .set(0, scale, 0);
     z  .set(0, 0, scale);
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::setPosScale(C Vec2 &pos, C Vec2 &scale)
{
   T.pos.set(pos.x*scale.x, pos.y*scale.y);
     x  .set(scale.x, 0);
     y  .set(0, scale.y);
   return T;
}
Matrix& Matrix::setPosScale(C Vec &pos, C Vec &scale)
{
   T.pos.set(pos.x*scale.x, pos.y*scale.y, pos.z*scale.z);
     x  .set(scale.x, 0, 0);
     y  .set(0, scale.y, 0);
     z  .set(0, 0, scale.z);
   return T;
}
MatrixM& MatrixM::setPosScale(C VecD &pos, C Vec &scale)
{
   T.pos.set(pos.x*scale.x, pos.y*scale.y, pos.z*scale.z);
     x  .set(scale.x, 0, 0);
     y  .set(0, scale.y, 0);
     z  .set(0, 0, scale.z);
   return T;
}
MatrixD& MatrixD::setPosScale(C VecD &pos, C VecD &scale)
{
   T.pos.set(pos.x*scale.x, pos.y*scale.y, pos.z*scale.z);
     x  .set(scale.x, 0, 0);
     y  .set(0, scale.y, 0);
     z  .set(0, 0, scale.z);
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::setScalePos(Flt scale, C Vec2 &pos)
{
   T.pos=pos;
     x  .set(scale, 0);
     y  .set(0, scale);
   return T;
}
Matrix& Matrix::setScalePos(Flt scale, C Vec &pos)
{
   T.pos=pos;
     x  .set(scale, 0, 0);
     y  .set(0, scale, 0);
     z  .set(0, 0, scale);
   return T;
}
MatrixM& MatrixM::setScalePos(Flt scale, C VecD &pos)
{
   T.pos=pos;
     x  .set(scale, 0, 0);
     y  .set(0, scale, 0);
     z  .set(0, 0, scale);
   return T;
}
MatrixD& MatrixD::setScalePos(Dbl scale, C VecD &pos)
{
   T.pos=pos;
     x  .set(scale, 0, 0);
     y  .set(0, scale, 0);
     z  .set(0, 0, scale);
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::setScalePos(C Vec2 &scale, C Vec2 &pos)
{
   T.pos=pos;
     x  .set(scale.x, 0);
     y  .set(0, scale.y);
   return T;
}
Matrix& Matrix::setScalePos(C Vec &scale, C Vec &pos)
{
   T.pos=pos;
     x  .set(scale.x, 0, 0);
     y  .set(0, scale.y, 0);
     z  .set(0, 0, scale.z);
   return T;
}
MatrixM& MatrixM::setScalePos(C Vec &scale, C VecD &pos)
{
   T.pos=pos;
     x  .set(scale.x, 0, 0);
     y  .set(0, scale.y, 0);
     z  .set(0, 0, scale.z);
   return T;
}
MatrixD& MatrixD::setScalePos(C VecD &scale, C VecD &pos)
{
   T.pos=pos;
     x  .set(scale.x, 0, 0);
     y  .set(0, scale.y, 0);
     z  .set(0, 0, scale.z);
   return T;
}
/******************************************************************************/
Matrix2& Matrix2::setRotate(Flt angle)
{
   Flt c, s; CosSin(c, s, angle);
   x.set( c, s);
   y.set(-s, c);
   return T;
}
/******************************************************************************/
Matrix3& Matrix3::setRotateX(Flt angle)
{
   Flt c, s; CosSin(c, s, angle);
   x.set( 1, 0, 0);
   y.set( 0, c, s);
   z.set( 0,-s, c);
   return T;
}
MatrixD3& MatrixD3::setRotateX(Dbl angle)
{
   Dbl c, s; CosSin(c, s, angle);
   x.set( 1, 0, 0);
   y.set( 0, c, s);
   z.set( 0,-s, c);
   return T;
}
Matrix3& Matrix3::setRotateY(Flt angle)
{
   Flt c, s; CosSin(c, s, angle);
   x.set( c, 0,-s);
   y.set( 0, 1, 0);
   z.set( s, 0, c);
   return T;
}
MatrixD3& MatrixD3::setRotateY(Dbl angle)
{
   Dbl c, s; CosSin(c, s, angle);
   x.set( c, 0,-s);
   y.set( 0, 1, 0);
   z.set( s, 0, c);
   return T;
}
Matrix3& Matrix3::setRotateZ(Flt angle)
{
   Flt c, s; CosSin(c, s, angle);
   x.set( c, s, 0);
   y.set(-s, c, 0);
   z.set( 0, 0, 1);
   return T;
}
MatrixD3& MatrixD3::setRotateZ(Dbl angle)
{
   Dbl c, s; CosSin(c, s, angle);
   x.set( c, s, 0);
   y.set(-s, c, 0);
   z.set( 0, 0, 1);
   return T;
}
Matrix3& Matrix3::setRotateXY(Flt pitch, Flt yaw)
{
   Flt cp, sp; CosSin(cp, sp, pitch);
   Flt cy, sy; CosSin(cy, sy, yaw  );
   y.x= sp*sy;
   y.z= sp*cy;
   z.x= cp*sy;
   z.z= cp*cy;
   y.y= cp;
   x.x= cy;
   z.y=-sp;
   x.z=-sy;
   x.y=  0;
   return T;
}
MatrixD3& MatrixD3::setRotateXY(Dbl pitch, Dbl yaw)
{
   Dbl cp, sp; CosSin(cp, sp, pitch);
   Dbl cy, sy; CosSin(cy, sy, yaw  );
   y.x= sp*sy;
   y.z= sp*cy;
   z.x= cp*sy;
   z.z= cp*cy;
   y.y= cp;
   x.x= cy;
   z.y=-sp;
   x.z=-sy;
   x.y=  0;
   return T;
}
Matrix3& Matrix3::setRotate(C Vec &axis, Flt angle) // !! this must match 'setRotateCosSin' !!
{
   Flt cos, sin; CosSin(cos, sin, angle);
   Flt x  =axis.x,
       y  =axis.y,
       z  =axis.z,
       cc = 1-cos,
       ccx=cc*x,
       ccy=cc*y,
       ccz=cc*z,
       p, q;

                     T.x.x=ccx*x+cos; T.y.y=ccy*y+cos; T.z.z=ccz*z+cos;
   p=x*sin;          T.z.y=ccz*y-p  ; T.y.z=ccy*z+p  ;
   p=y*sin; q=ccx*z; T.x.z=    q-p  ; T.z.x=    q+p  ;
   p=z*sin; q=ccx*y; T.y.x=    q-p  ; T.x.y=    q+p  ;
   return T;
}
MatrixD3& MatrixD3::setRotate(C VecD &axis, Dbl angle) // !! this must match 'setRotateCosSin' !!
{
   Dbl cos, sin; CosSin(cos, sin, angle);
   Dbl x  =axis.x,
       y  =axis.y,
       z  =axis.z,
       cc = 1-cos,
       ccx=cc*x,
       ccy=cc*y,
       ccz=cc*z,
       p, q;

                     T.x.x=ccx*x+cos; T.y.y=ccy*y+cos; T.z.z=ccz*z+cos;
   p=x*sin;          T.z.y=ccz*y-p  ; T.y.z=ccy*z+p  ;
   p=y*sin; q=ccx*z; T.x.z=    q-p  ; T.z.x=    q+p  ;
   p=z*sin; q=ccx*y; T.y.x=    q-p  ; T.x.y=    q+p  ;
   return T;
}
Matrix3& Matrix3::setRotateCosSin(C Vec &axis, Flt cos, Flt sin) // !! this must match 'setRotate' !!
{
   Flt x  =axis.x,
       y  =axis.y,
       z  =axis.z,
       cc = 1-cos,
       ccx=cc*x,
       ccy=cc*y,
       ccz=cc*z,
       p, q;

                     T.x.x=ccx*x+cos; T.y.y=ccy*y+cos; T.z.z=ccz*z+cos;
   p=x*sin;          T.z.y=ccz*y-p  ; T.y.z=ccy*z+p  ;
   p=y*sin; q=ccx*z; T.x.z=    q-p  ; T.z.x=    q+p  ;
   p=z*sin; q=ccx*y; T.y.x=    q-p  ; T.x.y=    q+p  ;
   return T;
}
MatrixD3& MatrixD3::setRotateCosSin(C VecD &axis, Dbl cos, Dbl sin) // !! this must match 'setRotate' !!
{
   Dbl x  =axis.x,
       y  =axis.y,
       z  =axis.z,
       cc = 1-cos,
       ccx=cc*x,
       ccy=cc*y,
       ccz=cc*z,
       p, q;

                     T.x.x=ccx*x+cos; T.y.y=ccy*y+cos; T.z.z=ccz*z+cos;
   p=x*sin;          T.z.y=ccz*y-p  ; T.y.z=ccy*z+p  ;
   p=y*sin; q=ccx*z; T.x.z=    q-p  ; T.z.x=    q+p  ;
   p=z*sin; q=ccx*y; T.y.x=    q-p  ; T.x.y=    q+p  ;
   return T;
}
/******************************************************************************/
Matrix2P& Matrix2P::setRotate(Flt angle) {super::setRotate(angle); pos.zero(); return T;}

Matrix& Matrix::setRotateX (              Flt angle) {super::setRotateX (      angle); pos.zero(); return T;}
Matrix& Matrix::setRotateY (              Flt angle) {super::setRotateY (      angle); pos.zero(); return T;}
Matrix& Matrix::setRotateZ (              Flt angle) {super::setRotateZ (      angle); pos.zero(); return T;}
Matrix& Matrix::setRotateXY(  Flt   x   , Flt y    ) {super::setRotateXY(x   , y    ); pos.zero(); return T;}
Matrix& Matrix::setRotate  (C Vec  &axis, Flt angle) {super::setRotate  (axis, angle); pos.zero(); return T;}

MatrixM& MatrixM::setRotateX (              Flt angle) {super::setRotateX (      angle); pos.zero(); return T;}
MatrixM& MatrixM::setRotateY (              Flt angle) {super::setRotateY (      angle); pos.zero(); return T;}
MatrixM& MatrixM::setRotateZ (              Flt angle) {super::setRotateZ (      angle); pos.zero(); return T;}
MatrixM& MatrixM::setRotateXY(  Flt   x   , Flt y    ) {super::setRotateXY(x   , y    ); pos.zero(); return T;}
MatrixM& MatrixM::setRotate  (C Vec  &axis, Flt angle) {super::setRotate  (axis, angle); pos.zero(); return T;}

MatrixD& MatrixD::setRotateX (              Dbl angle) {super::setRotateX (      angle); pos.zero(); return T;}
MatrixD& MatrixD::setRotateY (              Dbl angle) {super::setRotateY (      angle); pos.zero(); return T;}
MatrixD& MatrixD::setRotateZ (              Dbl angle) {super::setRotateZ (      angle); pos.zero(); return T;}
MatrixD& MatrixD::setRotateXY(  Dbl   x   , Dbl y    ) {super::setRotateXY(x   , y    ); pos.zero(); return T;}
MatrixD& MatrixD::setRotate  (C VecD &axis, Dbl angle) {super::setRotate  (axis, angle); pos.zero(); return T;}
/******************************************************************************/
Matrix3& Matrix3::setOrient(DIR_ENUM dir)
{
   Zero(T); switch(dir)
   {
      default       : x.x= 1; y.y= 1; z.z= 1; break; // DIR_FORWARD identity
      case DIR_BACK : x.x=-1; y.y= 1; z.z=-1; break;
      case DIR_UP   : x.x= 1; y.z=-1; z.y= 1; break;
      case DIR_DOWN : x.x= 1; y.z= 1; z.y=-1; break;
      case DIR_RIGHT: x.z=-1; y.y= 1; z.x= 1; break;
      case DIR_LEFT : x.z= 1; y.y= 1; z.x=-1; break;
   }
   return T;
}
MatrixD3& MatrixD3::setOrient(DIR_ENUM dir)
{
   Zero(T); switch(dir)
   {
      default       : x.x= 1; y.y= 1; z.z= 1; break; // DIR_FORWARD identity
      case DIR_BACK : x.x=-1; y.y= 1; z.z=-1; break;
      case DIR_UP   : x.x= 1; y.z=-1; z.y= 1; break;
      case DIR_DOWN : x.x= 1; y.z= 1; z.y=-1; break;
      case DIR_RIGHT: x.z=-1; y.y= 1; z.x= 1; break;
      case DIR_LEFT : x.z= 1; y.y= 1; z.x=-1; break;
   }
   return T;
}
Matrix3& Matrix3::setTerrainOrient(DIR_ENUM dir) // this assumes that terrain mesh is located on the XZ plane, and Y=up height #TerrainOrient
{
   Zero(T); switch(dir)
   {
      case DIR_FORWARD: x.x=-1; y.z= 1; z.y= 1; break;
      case DIR_BACK   : x.x= 1; y.z=-1; z.y= 1; break;
      default         : x.x= 1; y.y= 1; z.z= 1; break; // DIR_UP identity
      case DIR_DOWN   : x.x= 1; y.y=-1; z.z=-1; break;
      case DIR_RIGHT  : x.z= 1; y.x= 1; z.y= 1; break;
      case DIR_LEFT   : x.z=-1; y.x=-1; z.y= 1; break;
   }
   return T;
}
Matrix3 & Matrix3 ::setRight(C Vec  &right                         ) {x=right; y=PerpN(x); z=Cross(x, y); return T;}
MatrixD3& MatrixD3::setRight(C VecD &right                         ) {x=right; y=PerpN(x); z=Cross(x, y); return T;}
Matrix3 & Matrix3 ::setRight(C Vec  &right, C Vec  &up             ) {x=right; y=up      ; z=Cross(x, y); return T;}
MatrixD3& MatrixD3::setRight(C VecD &right, C VecD &up             ) {x=right; y=up      ; z=Cross(x, y); return T;}
Matrix3 & Matrix3 ::setUp   (C Vec  &up                            ) {y=up   ; z=PerpN(y); x=Cross(y, z); return T;}
MatrixD3& MatrixD3::setUp   (C VecD &up                            ) {y=up   ; z=PerpN(y); x=Cross(y, z); return T;}
Matrix3 & Matrix3 ::setDir  (C Vec  &dir                           ) {z=dir  ; y=PerpN(z); x=Cross(y, z); return T;}
MatrixD3& MatrixD3::setDir  (C VecD &dir                           ) {z=dir  ; y=PerpN(z); x=Cross(y, z); return T;}
Matrix3 & Matrix3 ::setDir  (C Vec  &dir, C Vec  &up               ) {z=dir  ; y=up;       x=Cross(y, z); return T;}
MatrixD3& MatrixD3::setDir  (C VecD &dir, C VecD &up               ) {z=dir  ; y=up;       x=Cross(y, z); return T;}
Matrix3 & Matrix3 ::setDir  (C Vec  &dir, C Vec  &up, C Vec  &right) {z=dir  ; y=up;       x=right      ; return T;}
MatrixD3& MatrixD3::setDir  (C VecD &dir, C VecD &up, C VecD &right) {z=dir  ; y=up;       x=right      ; return T;}

Matrix & Matrix ::setPosOrient(C Vec  &pos, DIR_ENUM dir                          ) {super::setOrient(dir    ); T.pos=pos; return T;}
MatrixM& MatrixM::setPosOrient(C VecD &pos, DIR_ENUM dir                          ) {super::setOrient(dir    ); T.pos=pos; return T;}
MatrixD& MatrixD::setPosOrient(C VecD &pos, DIR_ENUM dir                          ) {super::setOrient(dir    ); T.pos=pos; return T;}
Matrix & Matrix ::setPosRight (C Vec  &pos, C Vec  &right                         ) {super::setRight (right  ); T.pos=pos; return T;}
MatrixM& MatrixM::setPosRight (C VecD &pos, C Vec  &right                         ) {super::setRight (right  ); T.pos=pos; return T;}
MatrixD& MatrixD::setPosRight (C VecD &pos, C VecD &right                         ) {super::setRight (right  ); T.pos=pos; return T;}
Matrix & Matrix ::setPosUp    (C Vec  &pos, C Vec  &up                            ) {super::setUp    (up     ); T.pos=pos; return T;}
MatrixM& MatrixM::setPosUp    (C VecD &pos, C Vec  &up                            ) {super::setUp    (up     ); T.pos=pos; return T;}
MatrixD& MatrixD::setPosUp    (C VecD &pos, C VecD &up                            ) {super::setUp    (up     ); T.pos=pos; return T;}
Matrix & Matrix ::setPosDir   (C Vec  &pos, C Vec  &dir                           ) {super::setDir   (dir    ); T.pos=pos; return T;}
MatrixM& MatrixM::setPosDir   (C VecD &pos, C Vec  &dir                           ) {super::setDir   (dir    ); T.pos=pos; return T;}
MatrixD& MatrixD::setPosDir   (C VecD &pos, C VecD &dir                           ) {super::setDir   (dir    ); T.pos=pos; return T;}
Matrix & Matrix ::setPosDir   (C Vec  &pos, C Vec  &dir, C Vec  &up               ) {super::setDir   (dir, up); T.pos=pos; return T;}
MatrixM& MatrixM::setPosDir   (C VecD &pos, C Vec  &dir, C Vec  &up               ) {super::setDir   (dir, up); T.pos=pos; return T;}
MatrixD& MatrixD::setPosDir   (C VecD &pos, C VecD &dir, C VecD &up               ) {super::setDir   (dir, up); T.pos=pos; return T;}
Matrix & Matrix ::setPosDir   (C Vec  &pos, C Vec  &dir, C Vec  &up, C Vec  &right) {     z=dir; y=up; x=right; T.pos=pos; return T;}
MatrixM& MatrixM::setPosDir   (C VecD &pos, C Vec  &dir, C Vec  &up, C Vec  &right) {     z=dir; y=up; x=right; T.pos=pos; return T;}
MatrixD& MatrixD::setPosDir   (C VecD &pos, C VecD &dir, C VecD &up, C VecD &right) {     z=dir; y=up; x=right; T.pos=pos; return T;}

Matrix & Matrix ::setPosTerrainOrient(C Vec  &pos, DIR_ENUM dir) {super::setTerrainOrient(dir); T.pos=pos; return T;}
MatrixM& MatrixM::setPosTerrainOrient(C VecD &pos, DIR_ENUM dir) {super::setTerrainOrient(dir); T.pos=pos; return T;}
/******************************************************************************/
Matrix3& Matrix3::setRotation(C Vec &dir_from, C Vec &dir_to) // !! Warning: this will give identity if directions are opposite "dir_from==-dir_to" !!
{
   Vec cross=Cross(dir_from, dir_to); if(Flt sin=cross.normalize())setRotateCosSin(cross, Dot(dir_from, dir_to), sin);else identity();
   return T;
}
MatrixD3& MatrixD3::setRotation(C VecD &dir_from, C VecD &dir_to) // !! Warning: this will give identity if directions are opposite "dir_from==-dir_to" !!
{
   VecD cross=Cross(dir_from, dir_to); if(Dbl sin=cross.normalize())setRotateCosSin(cross, Dot(dir_from, dir_to), sin);else identity();
   return T;
}

Matrix3& Matrix3::setRotationUp(C Vec &dir_to)
{
   Vec cross=CrossUp(dir_to); if(Flt sin=cross.normalize())setRotateCosSin(cross, dir_to.y, sin);else identity();
   return T;
}

Matrix3& Matrix3::setRotation(C Vec &dir_from, C Vec &dir_to, Flt blend) // !! Warning: this will give identity if directions are opposite "dir_from==-dir_to" !!
{
   Vec cross=Cross(dir_from, dir_to); if(Flt sin=cross.normalize())setRotate(cross, ACosSin(Dot(dir_from, dir_to), sin)*blend);else identity();
   return T;
}
MatrixD3& MatrixD3::setRotation(C VecD &dir_from, C VecD &dir_to, Dbl blend) // !! Warning: this will give identity if directions are opposite "dir_from==-dir_to" !!
{
   VecD cross=Cross(dir_from, dir_to); if(Dbl sin=cross.normalize())setRotate(cross, ACosSin(Dot(dir_from, dir_to), sin)*blend);else identity();
   return T;
}

Matrix3& Matrix3::setRotation(C Vec &dir_from, C Vec &dir_to, Flt blend, Flt roll) // !! Warning: this will give identity if directions are opposite "dir_from==-dir_to" !!
{
   Vec cross=Cross(dir_from, dir_to); Flt sin=cross.normalize();
   if(roll)
   {
          setRotate(dir_from, roll);
      if(sin)rotate(cross, ACosSin(Dot(dir_from, dir_to), sin)*blend);
   }else
   {
      if(sin)setRotate(cross, ACosSin(Dot(dir_from, dir_to), sin)*blend);else identity();
   }
   return T;
}
MatrixD3& MatrixD3::setRotation(C VecD &dir_from, C VecD &dir_to, Dbl blend, Dbl roll) // !! Warning: this will give identity if directions are opposite "dir_from==-dir_to" !!
{
   VecD cross=Cross(dir_from, dir_to); Dbl sin=cross.normalize();
   if(roll)
   {
          setRotate(dir_from, roll);
      if(sin)rotate(cross, ACosSin(Dot(dir_from, dir_to), sin)*blend);
   }else
   {
      if(sin)setRotate(cross, ACosSin(Dot(dir_from, dir_to), sin)*blend);else identity();
   }
   return T;
}

Matrix& Matrix::setRotation(C Vec &pos, C Vec &dir_from, C Vec &dir_to, Flt blend) // !! Warning: this will give identity if directions are opposite "dir_from==-dir_to" !!
{
   Vec cross=Cross(dir_from, dir_to); if(Flt sin=cross.normalize()){orn().setRotate(cross, ACosSin(Dot(dir_from, dir_to), sin)*blend); anchor(pos);}else identity();
   return T;
}
/******************************************************************************/
Matrix& Matrix::set(C Box &src, C Box &dest)
{
   Vec scale(dest.w()/src.w(),
             dest.h()/src.h(),
             dest.d()/src.d());
   super::setScale(scale);
   pos=dest.min-src.min*scale;
   return T;
}
Matrix& Matrix::setNormalizeX(C Box &box) {Flt d=box.w(); setPosScale(-box.center(), (d>EPS) ? 1/d : 1); return T;}
Matrix& Matrix::setNormalizeY(C Box &box) {Flt d=box.h(); setPosScale(-box.center(), (d>EPS) ? 1/d : 1); return T;}
Matrix& Matrix::setNormalizeZ(C Box &box) {Flt d=box.d(); setPosScale(-box.center(), (d>EPS) ? 1/d : 1); return T;}
Matrix& Matrix::setNormalize (C Box &box)
{
   Vec size=box.size();
   if( size.x>=size.y && size.x>=size.z)return setNormalizeX(box); // x is the biggest
   if(                   size.y>=size.z)return setNormalizeY(box); // y is the biggest
                                        return setNormalizeZ(box); // z is the biggest
}
/******************************************************************************/
Matrix3& Matrix3::mirrorX()
{
   CHS(x.x);
   CHS(y.x);
   CHS(z.x);
   return T;
}
Matrix3& Matrix3::mirrorY()
{
   CHS(x.y);
   CHS(y.y);
   CHS(z.y);
   return T;
}
Matrix3& Matrix3::mirrorZ()
{
   CHS(x.z);
   CHS(y.z);
   CHS(z.z);
   return T;
}

MatrixD3& MatrixD3::mirrorX()
{
   CHS(x.x);
   CHS(y.x);
   CHS(z.x);
   return T;
}
MatrixD3& MatrixD3::mirrorY()
{
   CHS(x.y);
   CHS(y.y);
   CHS(z.y);
   return T;
}
MatrixD3& MatrixD3::mirrorZ()
{
   CHS(x.z);
   CHS(y.z);
   CHS(z.z);
   return T;
}

Matrix3& Matrix3::mirror(C Vec &normal)
{
   x=Mirror(x, normal);
   y=Mirror(y, normal);
   z=Mirror(z, normal);
   return T;
}
MatrixD3& MatrixD3::mirror(C VecD &normal)
{
   x=Mirror(x, normal);
   y=Mirror(y, normal);
   z=Mirror(z, normal);
   return T;
}

Matrix & Matrix ::mirrorX() {super::mirrorX(); CHS(pos.x); return T;}
Matrix & Matrix ::mirrorY() {super::mirrorY(); CHS(pos.y); return T;}
Matrix & Matrix ::mirrorZ() {super::mirrorZ(); CHS(pos.z); return T;}
MatrixD& MatrixD::mirrorX() {super::mirrorX(); CHS(pos.x); return T;}
MatrixD& MatrixD::mirrorY() {super::mirrorY(); CHS(pos.y); return T;}
MatrixD& MatrixD::mirrorZ() {super::mirrorZ(); CHS(pos.z); return T;}

Matrix & Matrix ::mirror(C Plane  &plane) {super::mirror(plane.normal); pos=Mirror(pos, plane.pos, plane.normal); return T;}
MatrixM& MatrixM::mirror(C PlaneM &plane) {super::mirror(plane.normal); pos=Mirror(pos, plane.pos, plane.normal); return T;}
MatrixD& MatrixD::mirror(C PlaneD &plane) {super::mirror(plane.normal); pos=Mirror(pos, plane.pos, plane.normal); return T;}
/******************************************************************************/
Matrix3& Matrix3::swapYZ()
{
   x.swapYZ();
   y.swapYZ();
   z.swapYZ();
   return T;
}
Matrix& Matrix::swapYZ()
{
   super::swapYZ();
   pos   .swapYZ();
   return T;
}
/******************************************************************************/
Vec2 Matrix2 ::   scale ()C {return          Vec2(x.length (), y.length ()             ) ;}
Vec  Matrix3 ::   scale ()C {return          Vec (x.length (), y.length (), z.length ()) ;}
VecD MatrixD3::   scale ()C {return          VecD(x.length (), y.length (), z.length ()) ;}
Vec2 Matrix2 ::   scale2()C {return          Vec2(x.length2(), y.length2()             ) ;}
Vec  Matrix3 ::   scale2()C {return          Vec (x.length2(), y.length2(), z.length2()) ;}
VecD MatrixD3::   scale2()C {return          VecD(x.length2(), y.length2(), z.length2()) ;}
Flt  Matrix2 ::avgScale ()C {return          Avg (x.length (), y.length ()             ) ;}
Flt  Matrix3 ::avgScale ()C {return          Avg (x.length (), y.length (), z.length ()) ;}
Dbl  MatrixD3::avgScale ()C {return          Avg (x.length (), y.length (), z.length ()) ;}
Flt  Matrix2 ::maxScale ()C {return SqrtFast(Max (x.length2(), y.length2()             ));}
Flt  Matrix3 ::maxScale ()C {return SqrtFast(Max (x.length2(), y.length2(), z.length2()));}
Dbl  MatrixD3::maxScale ()C {return SqrtFast(Max (x.length2(), y.length2(), z.length2()));}
/******************************************************************************/
Vec Matrix3::angles()C
{
   Vec     O;
   Matrix3 temp=T;
   O.y= Angle(temp.z.zx()); temp.rotateY(-O.y);
   O.x=-Angle(temp.z.zy()); temp.rotateX(-O.x);
   O.z= Angle(temp.x.xy  );
 //if(){O.x=PI-O.x; O.y=O.y-PI; O.z=O.z-PI;} generates alternative version
   return O;
}
VecD MatrixD3::angles()C
{
   VecD     O;
   MatrixD3 temp=T;
   O.y= Angle(temp.z.zx()); temp.rotateY(-O.y);
   O.x=-Angle(temp.z.zy()); temp.rotateX(-O.x);
   O.z= Angle(temp.x.xy  );
   return O;
}
/******************************************************************************/
Vec Matrix3::axis(Bool normalized)C
{
   Matrix3 temp;
 C Matrix3 *m=this; if(!normalized){temp=T; temp.normalize(); m=&temp;}

   Vec axis(m->y.z - m->z.y,
            m->z.x - m->x.z,
            m->x.y - m->y.x);

   Flt len2=axis.length2(); if(len2>Sqr(EPS))axis/=SqrtFast(len2);else // normalize
   {
      // singularity
      if(m->x.x>=EPS_COS)axis.set(1, 0, 0);else
      {
         Flt xx=(m->x.x+1)*0.5f,
             yy=(m->y.y+1)*0.5f,
             zz=(m->z.z+1)*0.5f,
             xy=(m->x.y+m->y.x)*0.25f,
             xz=(m->x.z+m->z.x)*0.25f,
             yz=(m->y.z+m->z.y)*0.25f;

         if(xx>=yy && xx>=zz)
         {
            axis.x=SqrtFast(xx);
            axis.y=xy/axis.x;
            axis.z=xz/axis.x;
         }else
         if(yy>=zz)
         {
            axis.y=SqrtFast(yy);
            axis.x=xy/axis.y;
            axis.z=yz/axis.y;
         }else
         {
            axis.z=SqrtFast(zz);
            axis.x=xz/axis.z;
            axis.y=yz/axis.z;
         }
      }
   }
   return axis;
}
VecD MatrixD3::axis(Bool normalized)C
{
   MatrixD3 temp;
 C MatrixD3 *m=this; if(!normalized){temp=T; temp.normalize(); m=&temp;}

   VecD axis(m->y.z - m->z.y,
             m->z.x - m->x.z,
             m->x.y - m->y.x);

   Dbl len2=axis.length2(); if(len2>Sqr(EPSD))axis/=SqrtFast(len2);else // normalize
   {
      // singularity
      if(m->x.x>=EPSD_COS)axis.set(1, 0, 0);else
      {
         Dbl xx=(m->x.x+1)*0.5,
             yy=(m->y.y+1)*0.5,
             zz=(m->z.z+1)*0.5,
             xy=(m->x.y+m->y.x)*0.25,
             xz=(m->x.z+m->z.x)*0.25,
             yz=(m->y.z+m->z.y)*0.25;

         if(xx>=yy && xx>=zz)
         {
            axis.x=SqrtFast(xx);
            axis.y=xy/axis.x;
            axis.z=xz/axis.x;
         }else
         if(yy>=zz)
         {
            axis.y=SqrtFast(yy);
            axis.x=xy/axis.y;
            axis.z=yz/axis.y;
         }else
         {
            axis.z=SqrtFast(zz);
            axis.x=xz/axis.z;
            axis.y=yz/axis.z;
         }
      }
   }
   return axis;
}
/******************************************************************************/
Flt Matrix3::angle(Bool normalized)C
{
   Matrix3 temp;
 C Matrix3 *m=this; if(!normalized){temp=T; temp.normalize(); m=&temp;}

   Vec axis(m->y.z - m->z.y,
            m->z.x - m->x.z,
            m->x.y - m->y.x);

   Flt len2=axis.length2(); if(len2>Sqr(EPS))
   {
      Flt sin=                SqrtFast(len2)/2,
          cos=(m->x.x + m->y.y + m->z.z - 1)/2;
      return ACosSin(cos, sin); // using 'ACosSin' is much more precise than just "Acos(cos)"
   }
   if(m->x.x>=EPS_COS && m->y.y>=0)return 0;
                                   return PI;
}
Dbl MatrixD3::angle(Bool normalized)C
{
   MatrixD3 temp;
 C MatrixD3 *m=this; if(!normalized){temp=T; temp.normalize(); m=&temp;}

   VecD axis(m->y.z - m->z.y,
             m->z.x - m->x.z,
             m->x.y - m->y.x);

   Dbl len2=axis.length2(); if(len2>Sqr(EPSD))
   {
      Dbl sin=                SqrtFast(len2)/2,
          cos=(m->x.x + m->y.y + m->z.z - 1)/2;
      return ACosSin(cos, sin); // using 'ACosSin' is much more precise than just "Acos(cos)"
   }
   if(m->x.x>=EPSD_COS && m->y.y>=0)return 0;
                                    return PID;
}
/******************************************************************************/
Flt Matrix3::angleY(Bool normalized)C
{
   Matrix3 temp;
 C Matrix3 *m=this; if(!normalized){temp=T; temp.normalize(); m=&temp;}

   Vec axis(m->y.z - m->z.y,
            m->z.x - m->x.z,
            m->x.y - m->y.x);

   Flt len2=axis.length2(); if(len2>Sqr(EPS))
   {
      Flt len=                  SqrtFast(len2),
          sin=                           len/2,
          cos=(m->x.x + m->y.y + m->z.z - 1)/2;
      return axis.y/len*ACosSin(cos, sin); // axis.y/len = normalize, using 'ACosSin' is much more precise than just "Acos(cos)"
   }
   if(m->x.x>=EPS_COS)return 0; // axis=(1,0,0) so axis.y==0

   Flt xx=m->x.x+1,
       yy=m->y.y+1,
       zz=m->z.z+1;

   if(xx>=yy && xx>=zz)return (m->x.y+m->y.x)/SqrtFast(xx)*(PI_4*SQRT2);
   if(yy>=zz          )return                 SqrtFast(yy)*(PI  /SQRT2);
                       return (m->y.z+m->z.y)/SqrtFast(zz)*(PI_4*SQRT2);
}
Dbl MatrixD3::angleY(Bool normalized)C
{
   MatrixD3 temp;
 C MatrixD3 *m=this; if(!normalized){temp=T; temp.normalize(); m=&temp;}

   VecD axis(m->y.z - m->z.y,
             m->z.x - m->x.z,
             m->x.y - m->y.x);

   Dbl len2=axis.length2(); if(len2>Sqr(EPSD))
   {
      Dbl len=                  SqrtFast(len2),
          sin=                           len/2,
          cos=(m->x.x + m->y.y + m->z.z - 1)/2;
      return axis.y/len*ACosSin(cos, sin); // axis.y/len = normalize, using 'ACosSin' is much more precise than just "Acos(cos)"
   }
   if(m->x.x>=EPSD_COS)return 0; // axis=(1,0,0) so axis.y==0

   Dbl xx=m->x.x+1,
       yy=m->y.y+1,
       zz=m->z.z+1;

   if(xx>=yy && xx>=zz)return (m->x.y+m->y.x)/SqrtFast(xx)*(PID_4*SQRT2);
   if(yy>=zz          )return                 SqrtFast(yy)*(PID  /SQRT2);
                       return (m->y.z+m->z.y)/SqrtFast(zz)*(PID_4*SQRT2);
}
/******************************************************************************/
Flt Matrix3::axisAngle(Vec &axis, Bool normalized)C
{
   Matrix3 temp;
 C Matrix3 *m=this; if(!normalized){temp=T; temp.normalize(); m=&temp;}

   axis.set(m->y.z - m->z.y,
            m->z.x - m->x.z,
            m->x.y - m->y.x);

   Flt len2=axis.length2(); if(len2>Sqr(EPS))
   {
      Flt len=                  SqrtFast(len2),
          sin=                           len/2,
          cos=(m->x.x + m->y.y + m->z.z - 1)/2;
      axis/=len; // normalize
      return ACosSin(cos, sin); // using 'ACosSin' is much more precise than just "Acos(cos)"
   }

   // singularity
   if(m->x.x>=EPS_COS)
   {
      axis.set(1, 0, 0);
      return (m->y.y>=0) ? 0 : PI;
   }

   Flt xx=(m->x.x+1)*0.5f,
       yy=(m->y.y+1)*0.5f,
       zz=(m->z.z+1)*0.5f,
       xy=(m->x.y+m->y.x)*0.25f,
       xz=(m->x.z+m->z.x)*0.25f,
       yz=(m->y.z+m->z.y)*0.25f;

   if(xx>=yy && xx>=zz)
   {
      axis.x=SqrtFast(xx);
      axis.y=xy/axis.x;
      axis.z=xz/axis.x;
   }else
   if(yy>=zz)
   {
      axis.y=SqrtFast(yy);
      axis.x=xy/axis.y;
      axis.z=yz/axis.y;
   }else
   {
      axis.z=SqrtFast(zz);
      axis.x=xz/axis.z;
      axis.y=yz/axis.z;
   }
   return PI;
}
Dbl MatrixD3::axisAngle(VecD &axis, Bool normalized)C
{
   MatrixD3 temp;
 C MatrixD3 *m=this; if(!normalized){temp=T; temp.normalize(); m=&temp;}

   axis.set(m->y.z - m->z.y,
            m->z.x - m->x.z,
            m->x.y - m->y.x);

   Dbl len2=axis.length2(); if(len2>Sqr(EPSD))
   {
      Dbl len=                  SqrtFast(len2),
          sin=                           len/2,
          cos=(m->x.x + m->y.y + m->z.z - 1)/2;
      axis/=len; // normalize
      return ACosSin(cos, sin); // using 'ACosSin' is much more precise than just "Acos(cos)"
   }

   // singularity
   if(m->x.x>=EPSD_COS)
   {
      axis.set(1, 0, 0);
      return (m->y.y>=0) ? 0 : PID;
   }

   Dbl xx=(m->x.x+1)*0.5,
       yy=(m->y.y+1)*0.5,
       zz=(m->z.z+1)*0.5,
       xy=(m->x.y+m->y.x)*0.25,
       xz=(m->x.z+m->z.x)*0.25,
       yz=(m->y.z+m->z.y)*0.25;

   if(xx>=yy && xx>=zz)
   {
      axis.x=SqrtFast(xx);
      axis.y=xy/axis.x;
      axis.z=xz/axis.x;
   }else
   if(yy>=zz)
   {
      axis.y=SqrtFast(yy);
      axis.x=xy/axis.y;
      axis.z=yz/axis.y;
   }else
   {
      axis.z=SqrtFast(zz);
      axis.x=xz/axis.z;
      axis.y=yz/axis.z;
   }
   return PID;
}
Vec  Matrix3 ::axisAngle(Bool normalized)C {Vec  axis; Flt angle=axisAngle(axis, normalized); return axis*=angle;}
VecD MatrixD3::axisAngle(Bool normalized)C {VecD axis; Dbl angle=axisAngle(axis, normalized); return axis*=angle;}
/******************************************************************************/
Vec2 Matrix3::convert(C Vec &src, Bool normalized)C
{
   Vec2 ret(Dot(src, x),
            Dot(src, y));
   if(!normalized)
   {
      ret.x/=x.length2();
      ret.y/=y.length2();
   }
   return ret;
}
VecD2 MatrixD3::convert(C VecD &src, Bool normalized)C
{
   VecD2 ret(Dot(src, x),
             Dot(src, y));
   if(!normalized)
   {
      ret.x/=x.length2();
      ret.y/=y.length2();
   }
   return ret;
}
Vec   Matrix3 ::convert(C Vec2  &src                 )C {return src.x*x + src.y*y                   ;}
VecD  MatrixD3::convert(C VecD2 &src                 )C {return src.x*x + src.y*y                   ;}
Vec2  Matrix  ::convert(C Vec   &src, Bool normalized)C {return super::convert(src -pos, normalized);}
VecD2 MatrixD ::convert(C VecD  &src, Bool normalized)C {return super::convert(src -pos, normalized);}
Vec   Matrix  ::convert(C Vec2  &src                 )C {return super::convert(src)+pos             ;}
VecD  MatrixD ::convert(C VecD2 &src                 )C {return super::convert(src)+pos             ;}

Edge2  Matrix3 ::convert(C Edge   &src, Bool normalized)C {return Edge2 (convert(src.p[0], normalized), convert(src.p[1], normalized));}
EdgeD2 MatrixD3::convert(C EdgeD  &src, Bool normalized)C {return EdgeD2(convert(src.p[0], normalized), convert(src.p[1], normalized));}
Edge   Matrix3 ::convert(C Edge2  &src                 )C {return Edge  (convert(src.p[0]            ), convert(src.p[1]            ));}
EdgeD  MatrixD3::convert(C EdgeD2 &src                 )C {return EdgeD (convert(src.p[0]            ), convert(src.p[1]            ));}
Edge2  Matrix  ::convert(C Edge   &src, Bool normalized)C {return Edge2 (convert(src.p[0], normalized), convert(src.p[1], normalized));}
EdgeD2 MatrixD ::convert(C EdgeD  &src, Bool normalized)C {return EdgeD2(convert(src.p[0], normalized), convert(src.p[1], normalized));}
Edge   Matrix  ::convert(C Edge2  &src                 )C {return Edge  (convert(src.p[0]            ), convert(src.p[1]            ));}
EdgeD  MatrixD ::convert(C EdgeD2 &src                 )C {return EdgeD (convert(src.p[0]            ), convert(src.p[1]            ));}
/******************************************************************************/
Matrix& Matrix::anchor(C Vec &anchor)
{
   Flt x=anchor.x, y=anchor.y, z=anchor.z;
   pos.set(x - x*T.x.x - y*T.y.x - z*T.z.x,
           y - x*T.x.y - y*T.y.y - z*T.z.y,
           z - x*T.x.z - y*T.y.z - z*T.z.z);
   return T;
}

MatrixM& MatrixM::anchor(C Vec &anchor)
{
   Dbl x=anchor.x, y=anchor.y, z=anchor.z;
   pos.set(x - x*T.x.x - y*T.y.x - z*T.z.x,
           y - x*T.x.y - y*T.y.y - z*T.z.y,
           z - x*T.x.z - y*T.y.z - z*T.z.z);
   return T;
}
MatrixM& MatrixM::anchor(C VecD &anchor)
{
   Dbl x=anchor.x, y=anchor.y, z=anchor.z;
   pos.set(x - x*T.x.x - y*T.y.x - z*T.z.x,
           y - x*T.x.y - y*T.y.y - z*T.z.y,
           z - x*T.x.z - y*T.y.z - z*T.z.z);
   return T;
}

MatrixD& MatrixD::anchor(C VecD &anchor)
{
   Dbl x=anchor.x, y=anchor.y, z=anchor.z;
   pos.set(x - x*T.x.x - y*T.y.x - z*T.z.x,
           y - x*T.x.y - y*T.y.y - z*T.z.y,
           z - x*T.x.z - y*T.y.z - z*T.z.z);
   return T;
}

Matrix& Matrix::setTransformAtPos(C Vec &pos, C Matrix3 &matrix) {orn()=matrix; anchor(pos);                   return T;}
Matrix& Matrix::setTransformAtPos(C Vec &pos, C Matrix  &matrix) {orn()=matrix; anchor(pos); move(matrix.pos); return T;}
Matrix& Matrix::   transformAtPos(C Vec &pos, C Matrix3 &matrix) {return moveBack(pos).mul(matrix).move(pos);}
Matrix& Matrix::   transformAtPos(C Vec &pos, C Matrix  &matrix) {return moveBack(pos).mul(matrix).move(pos);}

MatrixM& MatrixM::setTransformAtPos(C VecD &pos, C Matrix3 &matrix) {orn()=matrix; anchor(pos);                   return T;}
MatrixM& MatrixM::setTransformAtPos(C VecD &pos, C MatrixM &matrix) {orn()=matrix; anchor(pos); move(matrix.pos); return T;}
MatrixM& MatrixM::   transformAtPos(C VecD &pos, C Matrix3 &matrix) {return moveBack(pos).mul(matrix).move(pos);}
MatrixM& MatrixM::   transformAtPos(C VecD &pos, C MatrixM &matrix) {return moveBack(pos).mul(matrix).move(pos);}

MatrixD& MatrixD::setTransformAtPos(C VecD &pos, C MatrixD3 &matrix) {orn()=matrix; anchor(pos);                   return T;}
MatrixD& MatrixD::setTransformAtPos(C VecD &pos, C MatrixD  &matrix) {orn()=matrix; anchor(pos); move(matrix.pos); return T;}
MatrixD& MatrixD::   transformAtPos(C VecD &pos, C MatrixD3 &matrix) {return moveBack(pos).mul(matrix).move(pos);}
MatrixD& MatrixD::   transformAtPos(C VecD &pos, C MatrixD  &matrix) {return moveBack(pos).mul(matrix).move(pos);}
/******************************************************************************/
// this adjusts projection matrix so that final 2D position on the screen is moved by delta
void Matrix4::offsetX(Flt dx)
{
   x  .x+=x  .w*dx;
   y  .x+=y  .w*dx;
   z  .x+=z  .w*dx;
   pos.x+=pos.w*dx;
}
void Matrix4::offsetY(Flt dy)
{
   x  .y+=x  .w*dy;
   y  .y+=y  .w*dy;
   z  .y+=z  .w*dy;
   pos.y+=pos.w*dy;
}
/******************************************************************************/
Matrix3::Matrix3(C Matrix &matrix)
{
   x=matrix.x;
   y=matrix.y;
   z=matrix.z;
}
Matrix3::Matrix3(C MatrixD3 &matrix)
{
   x=matrix.x;
   y=matrix.y;
   z=matrix.z;
}
Matrix3::Matrix3(C Matrix4 &matrix)
{
   x=matrix.x.xyz;
   y=matrix.y.xyz;
   z=matrix.z.xyz;
}
Matrix3::Matrix3(C Orient &orient)
{
   x=orient.cross();
   y=orient.perp;
   z=orient.dir;
}
Matrix3::Matrix3(C OrientD &orient)
{
   x=orient.cross();
   y=orient.perp;
   z=orient.dir;
}
/******************************************************************************/
MatrixD3::MatrixD3(C Matrix3 &matrix)
{
   x=matrix.x;
   y=matrix.y;
   z=matrix.z;
}
MatrixD3::MatrixD3(C MatrixD &matrix)
{
   x=matrix.x;
   y=matrix.y;
   z=matrix.z;
}
MatrixD3::MatrixD3(C Orient  &orient) : MatrixD3(OrientD(orient)) {}
MatrixD3::MatrixD3(C OrientD &orient)
{
   x=orient.cross();
   y=orient.perp;
   z=orient.dir;
}
/******************************************************************************/
Matrix::Matrix(C Matrix3 &matrix)
{
   x=matrix.x;
   y=matrix.y;
   z=matrix.z;
   pos.zero();
}
Matrix::Matrix(C MatrixD3 &matrix)
{
   x=matrix.x;
   y=matrix.y;
   z=matrix.z;
   pos.zero();
}
Matrix::Matrix(C MatrixM &matrix)
{
   x  =matrix.x  ;
   y  =matrix.y  ;
   z  =matrix.z  ;
   pos=matrix.pos;
}
Matrix::Matrix(C MatrixD &matrix)
{
   x  =matrix.x  ;
   y  =matrix.y  ;
   z  =matrix.z  ;
   pos=matrix.pos;
}
Matrix::Matrix(C Matrix4 &matrix)
{
   x  =matrix.x  .xyz;
   y  =matrix.y  .xyz;
   z  =matrix.z  .xyz;
   pos=matrix.pos.xyz;
}
Matrix::Matrix(C OrientP &orient)
{
   x  =orient.cross();
   y  =orient.perp;
   z  =orient.dir;
   pos=orient.pos;
}
Matrix::Matrix(C OrientM &orient)
{
   x  =orient.cross();
   y  =orient.perp;
   z  =orient.dir;
   pos=orient.pos;
}
Matrix::Matrix(C OrientPD &orient)
{
   x  =orient.cross();
   y  =orient.perp;
   z  =orient.dir;
   pos=orient.pos;
}
MatrixM::MatrixM(C Matrix3 &matrix)
{
   x=matrix.x;
   y=matrix.y;
   z=matrix.z;
   pos.zero();
}
MatrixM::MatrixM(C Matrix &matrix)
{
   x  =matrix.x  ;
   y  =matrix.y  ;
   z  =matrix.z  ;
   pos=matrix.pos;
}
MatrixM::MatrixM(C MatrixD &matrix)
{
   x  =matrix.x  ;
   y  =matrix.y  ;
   z  =matrix.z  ;
   pos=matrix.pos;
}
MatrixM::MatrixM(C OrientP &orient)
{
   x  =orient.cross();
   y  =orient.perp;
   z  =orient.dir;
   pos=orient.pos;
}
MatrixM::MatrixM(C OrientM &orient)
{
   x  =orient.cross();
   y  =orient.perp;
   z  =orient.dir;
   pos=orient.pos;
}
MatrixM::MatrixM(C OrientPD &orient)
{
   x  =orient.cross();
   y  =orient.perp;
   z  =orient.dir;
   pos=orient.pos;
}
/******************************************************************************/
MatrixD::MatrixD(C MatrixD3 &matrix)
{
   x=matrix.x;
   y=matrix.y;
   z=matrix.z;
   pos.zero();
}
MatrixD::MatrixD(C Matrix &matrix)
{
   x  =matrix.x  ;
   y  =matrix.y  ;
   z  =matrix.z  ;
   pos=matrix.pos;
}
MatrixD::MatrixD(C MatrixM &matrix)
{
   x  =matrix.x  ;
   y  =matrix.y  ;
   z  =matrix.z  ;
   pos=matrix.pos;
}
MatrixD::MatrixD(C OrientP &o) : MatrixD3(o)
{
   pos=o.pos;
}
MatrixD::MatrixD(C OrientM &o) : MatrixD3(o)
{
   pos=o.pos;
}
MatrixD::MatrixD(C OrientPD &o) : MatrixD3(o)
{
   pos=o.pos;
}
/******************************************************************************/
Matrix4::Matrix4(C Matrix3 &matrix)
{
   x  .set(matrix.x, 0);
   y  .set(matrix.y, 0);
   z  .set(matrix.z, 0);
   pos.set(0, 0, 0,  1);
}
Matrix4::Matrix4(C Matrix &matrix)
{
   x  .set(matrix.x  , 0);
   y  .set(matrix.y  , 0);
   z  .set(matrix.z  , 0);
   pos.set(matrix.pos, 1);
}
/******************************************************************************/
Flt Matrix3::determinant()C
{
// return Dot(x, Cross(y, z));

// return x.x*y.y*z.z + x.y*y.z*z.x + x.z*y.x*z.y
//      - x.x*y.z*z.y - x.y*y.x*z.z - x.z*y.y*z.x;

   return x.x*(y.y*z.z - y.z*z.y)
        + x.y*(y.z*z.x - y.x*z.z)
        + x.z*(y.x*z.y - y.y*z.x);
}
Dbl MatrixD3::determinant()C
{
// return Dot(x, Cross(y, z));

// return x.x*y.y*z.z + x.y*y.z*z.x + x.z*y.x*z.y
//      - x.x*y.z*z.y - x.y*y.x*z.z - x.z*y.y*z.x;

   return x.x*(y.y*z.z - y.z*z.y)
        + x.y*(y.z*z.x - y.x*z.z)
        + x.z*(y.x*z.y - y.y*z.x);
}
Flt Matrix4::determinant()C
{
   return
      x.w*y.z*z.y*pos.x - x.z*y.w*z.y*pos.x - x.w*y.y*z.z*pos.x + x.y*y.w*z.z*pos.x
     +x.z*y.y*z.w*pos.x - x.y*y.z*z.w*pos.x - x.w*y.z*z.x*pos.y + x.z*y.w*z.x*pos.y
     +x.w*y.x*z.z*pos.y - x.x*y.w*z.z*pos.y - x.z*y.x*z.w*pos.y + x.x*y.z*z.w*pos.y
     +x.w*y.y*z.x*pos.z - x.y*y.w*z.x*pos.z - x.w*y.x*z.y*pos.z + x.x*y.w*z.y*pos.z
     +x.y*y.x*z.w*pos.z - x.x*y.y*z.w*pos.z - x.z*y.y*z.x*pos.w + x.y*y.z*z.x*pos.w
     +x.z*y.x*z.y*pos.w - x.x*y.z*z.y*pos.w - x.y*y.x*z.z*pos.w + x.x*y.y*z.z*pos.w;
}
/******************************************************************************/
void Matrix3::draw(C Vec &pos, C Color &x_color, C Color &y_color, C Color &z_color, Bool arrow)C
{
   if(arrow)
   {
      DrawArrow(x_color, pos, pos+x);
      DrawArrow(y_color, pos, pos+y);
      DrawArrow(z_color, pos, pos+z);
   }else
   {
      VI.line(x_color, pos, pos+x);
      VI.line(y_color, pos, pos+y);
      VI.line(z_color, pos, pos+z);
      VI.end ();
   }
}
void MatrixD3::draw(C VecD &pos, C Color &x_color, C Color &y_color, C Color &z_color, Bool arrow)C
{
   if(arrow)
   {
      DrawArrow(x_color, pos, pos+x);
      DrawArrow(y_color, pos, pos+y);
      DrawArrow(z_color, pos, pos+z);
   }else
   {
      VI.line(x_color, pos, pos+x);
      VI.line(y_color, pos, pos+y);
      VI.line(z_color, pos, pos+z);
      VI.end ();
   }
}
/******************************************************************************/
// GPU MATRIX
/******************************************************************************/
GpuMatrix::GpuMatrix(C Matrix &m)
{
   T.xx=m.x.x; T.yx=m.y.x; T.zx=m.z.x; T._x=m.pos.x;
   T.xy=m.x.y; T.yy=m.y.y; T.zy=m.z.y; T._y=m.pos.y;
   T.xz=m.x.z; T.yz=m.y.z; T.zz=m.z.z; T._z=m.pos.z;
}
GpuMatrix::GpuMatrix(C MatrixM &m)
{
   T.xx=m.x.x; T.yx=m.y.x; T.zx=m.z.x; T._x=m.pos.x;
   T.xy=m.x.y; T.yy=m.y.y; T.zy=m.z.y; T._y=m.pos.y;
   T.xz=m.x.z; T.yz=m.y.z; T.zz=m.z.z; T._z=m.pos.z;
}
/******************************************************************************/
GpuMatrix& GpuMatrix::fromMul(C Matrix &a, C Matrix &b)
{
   Flt x, y, z;

   x=b.x.x; y=b.y.x; z=b.z.x;
   T.xx=x*a.x.x + y*a.x.y + z*a.x.z;
   T.yx=x*a.y.x + y*a.y.y + z*a.y.z;
   T.zx=x*a.z.x + y*a.z.y + z*a.z.z;

   x=b.x.y; y=b.y.y; z=b.z.y;
   T.xy=x*a.x.x + y*a.x.y + z*a.x.z;
   T.yy=x*a.y.x + y*a.y.y + z*a.y.z;
   T.zy=x*a.z.x + y*a.z.y + z*a.z.z;

   x=b.x.z; y=b.y.z; z=b.z.z;
   T.xz=x*a.x.x + y*a.x.y + z*a.x.z;
   T.yz=x*a.y.x + y*a.y.y + z*a.y.z;
   T.zz=x*a.z.x + y*a.z.y + z*a.z.z;

   x=a.pos.x; y=a.pos.y; z=a.pos.z;
   T._x=x*b.x.x + y*b.y.x + z*b.z.x + b.pos.x;
   T._y=x*b.x.y + y*b.y.y + z*b.z.y + b.pos.y;
   T._z=x*b.x.z + y*b.y.z + z*b.z.z + b.pos.z;

   return T;
}
GpuMatrix& GpuMatrix::fromMul(C Matrix &a, C MatrixM &b)
{
   Flt x, y, z;

   x=b.x.x; y=b.y.x; z=b.z.x;
   T.xx=x*a.x.x + y*a.x.y + z*a.x.z;
   T.yx=x*a.y.x + y*a.y.y + z*a.y.z;
   T.zx=x*a.z.x + y*a.z.y + z*a.z.z;

   x=b.x.y; y=b.y.y; z=b.z.y;
   T.xy=x*a.x.x + y*a.x.y + z*a.x.z;
   T.yy=x*a.y.x + y*a.y.y + z*a.y.z;
   T.zy=x*a.z.x + y*a.z.y + z*a.z.z;

   x=b.x.z; y=b.y.z; z=b.z.z;
   T.xz=x*a.x.x + y*a.x.y + z*a.x.z;
   T.yz=x*a.y.x + y*a.y.y + z*a.y.z;
   T.zz=x*a.z.x + y*a.z.y + z*a.z.z;

   x=a.pos.x; y=a.pos.y; z=a.pos.z;
   T._x=x*b.x.x + y*b.y.x + z*b.z.x + b.pos.x;
   T._y=x*b.x.y + y*b.y.y + z*b.z.y + b.pos.y;
   T._z=x*b.x.z + y*b.y.z + z*b.z.z + b.pos.z;

   return T;
}
GpuMatrix& GpuMatrix::fromMul(C MatrixM &a, C MatrixM &b)
{
   {
      Flt x, y, z;

      x=b.x.x; y=b.y.x; z=b.z.x;
      T.xx=x*a.x.x + y*a.x.y + z*a.x.z;
      T.yx=x*a.y.x + y*a.y.y + z*a.y.z;
      T.zx=x*a.z.x + y*a.z.y + z*a.z.z;

      x=b.x.y; y=b.y.y; z=b.z.y;
      T.xy=x*a.x.x + y*a.x.y + z*a.x.z;
      T.yy=x*a.y.x + y*a.y.y + z*a.y.z;
      T.zy=x*a.z.x + y*a.z.y + z*a.z.z;

      x=b.x.z; y=b.y.z; z=b.z.z;
      T.xz=x*a.x.x + y*a.x.y + z*a.x.z;
      T.yz=x*a.y.x + y*a.y.y + z*a.y.z;
      T.zz=x*a.z.x + y*a.z.y + z*a.z.z;
   }
   {
      Dbl x, y, z;

      x=a.pos.x; y=a.pos.y; z=a.pos.z;
      T._x=x*b.x.x + y*b.y.x + z*b.z.x + b.pos.x;
      T._y=x*b.x.y + y*b.y.y + z*b.z.y + b.pos.y;
      T._z=x*b.x.z + y*b.y.z + z*b.z.z + b.pos.z;
   }
   return T;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
Bool Equal(C Matrix3  &a, C Matrix3  &b, Flt eps             ) {return Equal(a.x, b.x, eps) && Equal(a.y, b.y, eps) && Equal(a.z, b.z, eps);}
Bool Equal(C MatrixD3 &a, C MatrixD3 &b, Dbl eps             ) {return Equal(a.x, b.x, eps) && Equal(a.y, b.y, eps) && Equal(a.z, b.z, eps);}
Bool Equal(C Matrix4  &a, C Matrix4  &b, Flt eps             ) {return Equal(a.x, b.x, eps) && Equal(a.y, b.y, eps) && Equal(a.z, b.z, eps) && Equal(a.pos, b.pos,     eps);}
Bool Equal(C Matrix   &a, C Matrix   &b, Flt eps, Flt pos_eps) {return                                         Equal(a.orn(), b.orn(), eps) && Equal(a.pos, b.pos, pos_eps);}
Bool Equal(C MatrixM  &a, C MatrixM  &b, Flt eps, Dbl pos_eps) {return                                         Equal(a.orn(), b.orn(), eps) && Equal(a.pos, b.pos, pos_eps);}
Bool Equal(C MatrixD  &a, C MatrixD  &b, Dbl eps, Dbl pos_eps) {return                                         Equal(a.orn(), b.orn(), eps) && Equal(a.pos, b.pos, pos_eps);}
/******************************************************************************/
void GetTransform(Matrix3 &transform, C Orient &start, C Orient &result)
{
#if 0 // unoptimized
   start.inverse(transform); transform.mul(Matrix3(result));
#else //   optimized
   Vec  start_cross= start.cross(),
       result_cross=result.cross();

   // this inverses 'start' into T
   #define T_x_x start_cross.x
   #define T_x_y start.perp .x
   #define T_x_z start.dir  .x

   #define T_y_x start_cross.y
   #define T_y_y start.perp .y
   #define T_y_z start.dir  .y

   #define T_z_x start_cross.z
   #define T_z_y start.perp .z
   #define T_z_z start.dir  .z

   #define m_x result_cross
   #define m_y result.perp
   #define m_z result.dir

   // following code is from 'Matrix3.mul'
   Flt x, y, z;
                 x=T_y_x;  y=T_y_y;  z=T_y_z;
   transform.y.x=x*m_x.x + y*m_y.x + z*m_z.x;
   transform.y.y=x*m_x.y + y*m_y.y + z*m_z.y;
   transform.y.z=x*m_x.z + y*m_y.z + z*m_z.z;

                 x=T_z_x;  y=T_z_y;  z=T_z_z;
   transform.z.x=x*m_x.x + y*m_y.x + z*m_z.x;
   transform.z.y=x*m_x.y + y*m_y.y + z*m_z.y;
   transform.z.z=x*m_x.z + y*m_y.z + z*m_z.z;

#if 1 // this has more calculations but they can be done independently, performance in tests was the same as below, yet this has better precision because original values are used, so choose this version
                 x=T_x_x;  y=T_x_y;  z=T_x_z;
   transform.x.x=x*m_x.x + y*m_y.x + z*m_z.x;
   transform.x.y=x*m_x.y + y*m_y.y + z*m_z.y;
   transform.x.z=x*m_x.z + y*m_y.z + z*m_z.z;
#else
 //transform.x=Cross(transform.y, transform.z);
   transform.x.x=transform.y.y*transform.z.z - transform.y.z*transform.z.y;
   transform.x.y=transform.y.z*transform.z.x - transform.y.x*transform.z.z;
   transform.x.z=transform.y.x*transform.z.y - transform.y.y*transform.z.x;
#endif

   #undef T_x_x
   #undef T_x_y
   #undef T_x_z

   #undef T_y_x
   #undef T_y_y
   #undef T_y_z

   #undef T_z_x
   #undef T_z_y
   #undef T_z_z

   #undef m_x
   #undef m_y
   #undef m_z
#endif
}
void     GetTransform(Matrix3  &transform, C Matrix3  &start, C Matrix3  &result) {Matrix3  temp     ; start.inverse(temp     ); temp.mul(result, transform);} // use 'temp' in case "&transform==&result"
void     GetTransform(MatrixD3 &transform, C MatrixD3 &start, C MatrixD3 &result) {MatrixD3 temp     ; start.inverse(temp     ); temp.mul(result, transform);} // use 'temp' in case "&transform==&result"
void     GetTransform(Matrix   &transform, C Matrix   &start, C Matrix   &result) {Matrix   temp     ; start.inverse(temp     ); temp.mul(result, transform);} // use 'temp' in case "&transform==&result"
void     GetTransform(MatrixM  &transform, C MatrixM  &start, C MatrixM  &result) {MatrixM  temp     ; start.inverse(temp     ); temp.mul(result, transform);} // use 'temp' in case "&transform==&result"
void     GetTransform(MatrixD  &transform, C MatrixD  &start, C MatrixD  &result) {MatrixD  temp     ; start.inverse(temp     ); temp.mul(result, transform);} // use 'temp' in case "&transform==&result"
Matrix3  GetTransform(                     C Matrix3  &start, C Matrix3  &result) {Matrix3  transform; start.inverse(transform); transform*=result; return transform;}
MatrixD3 GetTransform(                     C MatrixD3 &start, C MatrixD3 &result) {MatrixD3 transform; start.inverse(transform); transform*=result; return transform;}
Matrix   GetTransform(                     C Matrix   &start, C Matrix   &result) {Matrix   transform; start.inverse(transform); transform*=result; return transform;}
MatrixM  GetTransform(                     C MatrixM  &start, C MatrixM  &result) {MatrixM  transform; start.inverse(transform); transform*=result; return transform;}
MatrixD  GetTransform(                     C MatrixD  &start, C MatrixD  &result) {MatrixD  transform; start.inverse(transform); transform*=result; return transform;}
Matrix3  GetTransform(                     C Orient   &start, C Orient   &result) {Matrix3  transform;  GetTransform(transform, start, result);     return transform;}
/******************************************************************************/
void GetDelta(Vec &angle, C Matrix3 &prev, C Matrix3 &cur)
{
#if 1 // use quaternions since it will be slightly faster, conversions to quaternions automatically normalize scale
   Quaternion q=prev; q.inverseNormalized(); q*=Quaternion(cur); angle=q.axisAngle();
#else
   Matrix3 t; GetTransform(t, prev, cur); angle=t.axisAngle();
#endif
}
void GetDelta(Vec &pos, Vec &angle, C Matrix &prev, C Matrix &cur)
{
   // pos
   pos=cur.pos-prev.pos;

   // angle
#if 1 // use quaternions since it will be slightly faster, conversions to quaternions automatically normalize scale
   Quaternion q=prev; q.inverseNormalized(); q*=Quaternion(cur); angle=q.axisAngle();
#else
   Matrix3 t; GetTransform(t, prev, cur); angle=t.axisAngle();
#endif
}
void GetDelta(Vec &pos, Vec &angle, C MatrixM &prev, C MatrixM &cur)
{
   // pos
   pos=cur.pos-prev.pos;

   // angle
#if 1 // use quaternions since it will be slightly faster, conversions to quaternions automatically normalize scale
   Quaternion q=prev; q.inverseNormalized(); q*=Quaternion(cur); angle=q.axisAngle();
#else
   Matrix3 t; GetTransform(t, prev, cur); angle=t.axisAngle();
#endif
}
INLINE void CalcPos(Vec &pos_delta, C Vec &pos_delta0, C Vec &pos_delta1)
{
   pos_delta=pos_delta1;
   Vec cross=Cross(pos_delta0, pos_delta1);
   Flt sin  =cross.normalize(); if(sin>EPS) // here 'sin' is proportional to vector lengths
   {
      Flt cos  =Dot  (pos_delta0, pos_delta1), // here 'cos' is proportional to vector lengths
          angle=Angle(cos, sin); // can use 'Angle' because 'cos' and 'sin' have proportional lengths

      // if one delta length is 2x smaller, then make angle 2x smaller. This is to minimize rotations when one length is very big, and another very small
      Flt l0=pos_delta0.length2(), l1=pos_delta1.length2(); // use 'length2' and do Sqrt below just one time
      if(l0<l1)angle*=SqrtFast(l0/l1); // use Sqrt just one time instead of 2 times above if using 'length'
      else     angle*=SqrtFast(l1/l0);
    //if(angle>EPS)
      {
         pos_delta*=Matrix3().setRotate(cross, angle/2); // div angle by 2 because we want to have the average of normal and rotated

         if(1) // high precision adjustment, this is needed because we use discrete points with timesteps, however precise movement on circle travels "angle" distance, but we operate on "Dist(a,b)" which is distance between 2 points on a circle in a straight line. The following code will make the adjustment for precise "angle" movement based on "Dist". To better understand this, draw a circle, and mark 2 points on the circle line. Travelled distance should be the circular line between them, and not the straight line.
         {
         #if 0
            Vec2 p(1, 0), // CosSin(p.x, p.y, 0); point cos sin with angle=0
                 t; CosSin(t.x, t.y, angle); // point cos sin with angle='angle'
            Flt  dist_actual =Dist(p, t), // distance between the 2 points is 'd'
                 dist_precise=angle, // however using precise circular movement we should get 'angle'
                 scale=dist_precise/dist_actual; // so scale by "expected/actual"
            pos_delta*=scale;
         #else // optimized
            CosSin(cos, sin, angle); pos_delta*=angle/Dist(cos-1, sin);
         #endif
         }
      }
   }
}
void GetDelta(Vec &pos, Vec &angle, C Vec &prev2_pos, C Matrix &prev, C Matrix &cur)
{
   // pos
   CalcPos(pos, prev.pos-prev2_pos, cur.pos-prev.pos);

   // angle
#if 1 // use quaternions since it will be slightly faster, conversions to quaternions automatically normalize scale
   Quaternion q=prev; q.inverseNormalized(); q*=Quaternion(cur); angle=q.axisAngle();
#else
   Matrix3 t; GetTransform(t, prev, cur); angle=t.axisAngle();
#endif
}
void GetDelta(Vec &pos, Vec &angle, C VecD &prev2_pos, C MatrixM &prev, C MatrixM &cur)
{
   // pos
   CalcPos(pos, prev.pos-prev2_pos, cur.pos-prev.pos); // no need for VecD

   // angle
#if 1 // use quaternions since it will be slightly faster, conversions to quaternions automatically normalize scale
   Quaternion q=prev; q.inverseNormalized(); q*=Quaternion(cur); angle=q.axisAngle();
#else
   Matrix3 t; GetTransform(t, prev, cur); angle=t.axisAngle();
#endif
}
void GetDelta(VecD &pos, VecD &angle, C MatrixD &prev, C MatrixD &cur)
{
   // pos
   pos=cur.pos-prev.pos;

   // angle
#if 1 // use quaternions since it will be slightly faster, conversions to quaternions automatically normalize scale
   QuaternionD q=prev; q.inverseNormalized(); q*=QuaternionD(cur); angle=q.axisAngle();
#else
   MatrixD3 t; GetTransform(t, prev, cur); angle=t.axisAngle();
#endif
}
/******************************************************************************/
void GetVel(Vec &vel, Vec &ang_vel, C Matrix &prev, C Matrix &cur, Flt dt)
{
   if(dt)
   {
      GetDelta(vel, ang_vel, prev, cur);
          vel/=dt;
      ang_vel/=dt;
   }else
   {
          vel.zero();
      ang_vel.zero();
   }
}
void GetVel(Vec &vel, Vec &ang_vel, C MatrixM &prev, C MatrixM &cur, Flt dt)
{
   if(dt)
   {
      GetDelta(vel, ang_vel, prev, cur);
          vel/=dt;
      ang_vel/=dt;
   }else
   {
          vel.zero();
      ang_vel.zero();
   }
}
void GetVel(Vec &vel, Vec &ang_vel, C Vec &prev2_pos, C Matrix &prev, C Matrix &cur, Flt dt)
{
   if(dt)
   {
      GetDelta(vel, ang_vel, prev2_pos, prev, cur);
          vel/=dt;
      ang_vel/=dt;
   }else
   {
          vel.zero();
      ang_vel.zero();
   }
}
void GetVel(Vec &vel, Vec &ang_vel, C VecD &prev2_pos, C MatrixM &prev, C MatrixM &cur, Flt dt)
{
   if(dt)
   {
      GetDelta(vel, ang_vel, prev2_pos, prev, cur);
          vel/=dt;
      ang_vel/=dt;
   }else
   {
          vel.zero();
      ang_vel.zero();
   }
}
void GetVel(VecD &vel, VecD &ang_vel, C MatrixD &prev, C MatrixD &cur, Dbl dt)
{
   if(dt)
   {
      GetDelta(vel, ang_vel, prev, cur);
          vel/=dt;
      ang_vel/=dt;
   }else
   {
          vel.zero();
      ang_vel.zero();
   }
}
/******************************************************************************/
// SHADER
/******************************************************************************/
void AnimatedSkeleton::setMatrix()C
{
   Int matrixes=Min(bones.elms(), MAX_MATRIX-VIRTUAL_ROOT_BONE); // this is the amount of matrixes for bones (without the virtual), leave room for root bone
   SetMatrixCount(VIRTUAL_ROOT_BONE+matrixes); // root + bones
   ObjMatrix=matrix(); // 'Mesh.drawBlend' makes use of the 'ObjMatrix' so it must be set
   if(Renderer._mesh_shader_vel) // we need to process velocities (this is disabled in 'Renderer.del' to prevent using shader handles after deletion)
   {
      if(VIRTUAL_ROOT_BONE)
      {
         ViewMatrix    [0].fromMul(root._matrix     , CamMatrixInv    ); // Warning: this does not call 'setChanged'
         ViewMatrixPrev[0].fromMul(root._matrix_prev, CamMatrixInvPrev); // Warning: this does not call 'setChanged'
      }
      REP(matrixes)
      {
       C AnimSkelBone &bone=bones[i];
         ViewMatrix    [VIRTUAL_ROOT_BONE+i].fromMul(bone._matrix     , CamMatrixInv    ); // Warning: this does not call 'setChanged'
         ViewMatrixPrev[VIRTUAL_ROOT_BONE+i].fromMul(bone._matrix_prev, CamMatrixInvPrev); // Warning: this does not call 'setChanged'
      }
      Sh.ViewMatrixPrev->setChanged(); // call 'setChanged' only once, instead of for each bone
      /* Bone Splits old code
      if(VIRTUAL_ROOT_BONE)
      {
         Sh.ViewMatrix->set(GObjMatrix[0].fromMul(matrix(), CamMatrixInv));
         Sh.ObjVel    ->set(GObjVel   [0]=v, ang_delta_shader            );
      }
      REP(matrixes)
      {
         C AnimSkelBone &bone=bones[i];
         v=bone._pos_delta-ActiveCam.pos_delta; v*=CamMatrixInv.orn();
         Sh.ViewMatrix->set(GObjMatrix[VIRTUAL_ROOT_BONE+i].fromMul(bone._matrix, CamMatrixInv), VIRTUAL_ROOT_BONE+i);
         Sh.ObjVel    ->set(GObjVel   [VIRTUAL_ROOT_BONE+i]=v, ang_delta_shader                , VIRTUAL_ROOT_BONE+i);
      }*/
   }else
   {
      if(VIRTUAL_ROOT_BONE)ViewMatrix[                  0].fromMul(         matrix(), CamMatrixInv); // Warning: this does not call 'setChanged'
              REP(matrixes)ViewMatrix[VIRTUAL_ROOT_BONE+i].fromMul(bones[i].matrix(), CamMatrixInv); // Warning: this does not call 'setChanged'
      /* Bone Splits old code
      if(VIRTUAL_ROOT_BONE)Sh.ViewMatrix->set(GObjMatrix[0                  ].fromMul(          matrix(), CamMatrixInv));
              REP(matrixes)Sh.ViewMatrix->set(GObjMatrix[VIRTUAL_ROOT_BONE+i].fromMul(bones[i]._matrix  , CamMatrixInv), VIRTUAL_ROOT_BONE+i);*/
   }
   Sh.ViewMatrix->setChanged(); // call 'setChanged' only once, instead of for each bone
}
/******************************************************************************/
void SetVelFur(C Matrix3 &view_matrix, C Vec &vel)
{
   Vec v=vel*D.furStaticVelScale(); v.y+=D.furStaticGravity();
   v.clipLength(0.92f);
#if DEBUG && 0
   Matrix3 obj_matrix=view_matrix*CamMatrix.orn(); obj_matrix.normalize(); Vec v2=v/obj_matrix; // 'v2' should be the same as 'v' below, this can be used for testing
#endif
   v*=CamMatrixInv.orn(); // below we should divide by 'object_matrix', however we only have 'view_matrix', applying this transformation will provide the same result
   v.divNormalized(view_matrix)/=view_matrix.x.length(); // this is equal to dividing by normalized matrix, v/=matrix.normalize(), as a faster approximation because we use only 'x.length' ignoring y and z, yes in this case it should be 'length' and not 'length2'
   Sh.FurVel->set(v);
}
INLINE Vec FurVelShader(C Vec &vel, C Matrix3 &matrix) {return Vec().fromDivNormalized(vel, matrix)/=matrix.x.length();} // this is equal to dividing by normalized matrix, v/=matrix.normalize(), as a faster approximation because we use only 'x.length' ignoring y and z, yes in this case it should be 'length' and not 'length2'
void AnimatedSkeleton::setFurVel()C
{
   Int matrixes=Min(bones.elms(), MAX_MATRIX-VIRTUAL_ROOT_BONE); // this is the amount of matrixes for bones (without the virtual), leave room for root bone
   SetFurVelCount(VIRTUAL_ROOT_BONE+matrixes); // root + bones
   if(VIRTUAL_ROOT_BONE)Sh.FurVel->set(FurVelShader(VecZero/*    root._fur_vel*/,           matrix()));
           REP(matrixes)Sh.FurVel->set(FurVelShader(VecZero/*bones[i]._fur_vel*/, bones[i]._matrix  ), VIRTUAL_ROOT_BONE+i);
   /* Bone Splits
   if(VIRTUAL_ROOT_BONE)Sh.FurVel->set(GFurVel[0                  ]=FurVelShader(    root._fur_vel,           matrix()));
   REP(matrixes)Sh.FurVel->set(GFurVel[VIRTUAL_ROOT_BONE+i]=FurVelShader(bones[i]._fur_vel, bones[i]._matrix  ), VIRTUAL_ROOT_BONE+i);*/
}
/******************************************************************************/
void SetOneMatrix()
{
   SetMatrixCount();
   SetFastMatrix ();
}
void SetOneMatrix(C Matrix &matrix)
{
   SetMatrixCount();
   SetFastMatrix (matrix);
}
void SetOneMatrix(C MatrixM &matrix)
{
   SetMatrixCount();
   SetFastMatrix (matrix);
}
void SetOneMatrix(C Matrix &matrix, C Matrix &matrix_prev)
{
   SetMatrixCount   ();
   SetFastMatrix    (matrix);
   SetFastMatrixPrev(matrix_prev);
}
void SetOneMatrix(C MatrixM &matrix, C MatrixM &matrix_prev)
{
   SetMatrixCount   ();
   SetFastMatrix    (matrix);
   SetFastMatrixPrev(matrix_prev);
}
void SetOneMatrixAndPrev()
{
   SetMatrixCount   ();
   SetFastMatrix    ();
   SetFastMatrixPrev();
}
/******************************************************************************/
// To be used by the user, these must always set both 'cur' and 'prev' matrix
void SetMatrix()
{
   ObjMatrix.identity();
   SetMatrixCount   ();
   SetFastMatrix    ();
   SetFastMatrixPrev();
}
void SetMatrix(C Matrix &matrix)
{
   ObjMatrix=matrix;
   SetMatrixCount   ();
   SetFastMatrix    (matrix);
   SetFastMatrixPrev(matrix);
}
void SetMatrix(C MatrixM &matrix)
{
   ObjMatrix=matrix;
   SetMatrixCount   ();
   SetFastMatrix    (matrix);
   SetFastMatrixPrev(matrix);
}
void SetMatrix(C Matrix &matrix, C Matrix &matrix_prev)
{
   ObjMatrix=matrix;
   SetMatrixCount   ();
   SetFastMatrix    (matrix);
   SetFastMatrixPrev(matrix_prev);
}
void SetMatrix(C MatrixM &matrix, C MatrixM &matrix_prev)
{
   ObjMatrix=matrix;
   SetMatrixCount   ();
   SetFastMatrix    (matrix);
   SetFastMatrixPrev(matrix_prev);
}
/******************************************************************************/
INLINE void TestProjMatrix(Matrix4 &m)
{
#if DEBUG
   if(Any(         m.x  .y, m.x.z, m.x.w)
   || Any(m.y  .x,          m.y.z, m.y.w)
 //|| Any(m.z  .x, m.z  .y              ) // 'm.z.x' can be adjusted for VR Stereo or Temporal, 'm.z.y' can be adjusted for Temporal
   || Any(m.pos.x, m.pos.y              ))Exit("Shader 'Project' needs to be adjusted"); // 'm.pos.x' could be adjusted, however it's possible only for orthogonal mode, which is not used in VR (shadows are not stereo, and main view is always perspective based on VR FOV)
#endif
}
inline void FlipY(Matrix4 &m)
{
   CHS(m.x.y); CHS(m.y.y); CHS(m.z.y); CHS(m.pos.y);
}
//Flt cam_offset; m.pos.x+=m.x.x*cam_offset; // this matches "m=Matrix().setPos(cam_offset, 0, 0)*m"
void SetProjMatrix() // this needs to be additionally called when switching between '_main' and some other RT on OpenGL
{
   if(Renderer._temporal_use && Renderer()!=RM_SHADOW)
   {
      Matrix4 m=ProjMatrix                         ; m.offsetX(Renderer._temporal_offset.x); m.offsetY(Renderer._temporal_offset.y);
      Matrix4 p=Renderer._ctx_sub->proj_matrix_prev; p.offsetX(Renderer._temporal_offset.x); p.offsetY(Renderer._temporal_offset.y);

      if(GL && !D.mainFBO()){FlipY(m); FlipY(p);} // in OpenGL when drawing to a RenderTarget the Y must be flipped
      Sh.ProjMatrix    ->set(m); TestProjMatrix(m);
      Sh.ProjMatrixPrev->set(p); TestProjMatrix(p);
   }else
   if(GL && !D.mainFBO()) // in OpenGL when drawing to a RenderTarget the Y must be flipped
   {
      Matrix4 m=ProjMatrix                         ; FlipY(m); Sh.ProjMatrix    ->set(m); TestProjMatrix(m);
              m=Renderer._ctx_sub->proj_matrix_prev; FlipY(m); Sh.ProjMatrixPrev->set(m); TestProjMatrix(m);
   }else
   {
      Sh.ProjMatrix    ->set(ProjMatrix                         ); TestProjMatrix(ProjMatrix                         );
      Sh.ProjMatrixPrev->set(Renderer._ctx_sub->proj_matrix_prev); TestProjMatrix(Renderer._ctx_sub->proj_matrix_prev);
   }
}
void SetProjMatrix(Flt proj_offset)
{
   Matrix4 m=ProjMatrix;
   Matrix4 p=Renderer._ctx_sub->proj_matrix_prev;
   if(Renderer._temporal_use && Renderer()!=RM_SHADOW)
   {
      proj_offset+=Renderer._temporal_offset.x;
      m.offsetY(Renderer._temporal_offset.y);
      p.offsetY(Renderer._temporal_offset.y);
   }
   m.offsetX(proj_offset);
   p.offsetX(proj_offset);

   if(GL && !D.mainFBO()){FlipY(m); FlipY(p);} // in OpenGL when drawing to a RenderTarget the Y must be flipped
   Sh.ProjMatrix    ->set(m); TestProjMatrix(m);
   Sh.ProjMatrixPrev->set(p); TestProjMatrix(p);
}
/******************************************************************************/
// LOD
/******************************************************************************/
Flt GetLodDist2(C Vec &lod_center)
{
   Flt dist2=D._lod_current_factor;
   if(FovPerspective(D.viewFovMode()))dist2*=Dist2(lod_center, ActiveCam.matrix.pos); // has to be 'ActiveCam.matrix' and not 'CamMatrix' because we need the same LOD for shadows as for normal
   return dist2;
}
Flt GetLodDist2(C Vec &lod_center, C Matrix3 &matrix)
{
   Flt dist2=D._lod_current_factor/matrix.x.length2(); // Warning: only 'matrix.x' length is used (ignoring Y and Z)
   if(FovPerspective(D.viewFovMode()))dist2*=Dist2(lod_center*matrix, ActiveCam.matrix.pos); // has to be 'ActiveCam.matrix' and not 'CamMatrix' because we need the same LOD for shadows as for normal
   return dist2;
}
Flt GetLodDist2(C Vec &lod_center, C Matrix &matrix)
{
   Flt dist2=D._lod_current_factor/matrix.x.length2(); // Warning: only 'matrix.x' length is used (ignoring Y and Z)
   if(FovPerspective(D.viewFovMode()))dist2*=Dist2(lod_center*matrix, ActiveCam.matrix.pos); // has to be 'ActiveCam.matrix' and not 'CamMatrix' because we need the same LOD for shadows as for normal
   return dist2;
}
Flt GetLodDist2(C Vec &lod_center, C MatrixM &matrix)
{
   Flt dist2=D._lod_current_factor/matrix.x.length2(); // Warning: only 'matrix.x' length is used (ignoring Y and Z)
   if(FovPerspective(D.viewFovMode()))dist2*=Dist2(lod_center*matrix, ActiveCam.matrix.pos); // has to be 'ActiveCam.matrix' and not 'CamMatrix' because we need the same LOD for shadows as for normal
   return dist2;
}
/******************************************************************************/
}
/******************************************************************************/
