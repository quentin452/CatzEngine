/******************************************************************************/
enum DIST_TYPE // Distance Type
{
   DIST_NONE  , // none
   DIST_POINT0, // 0th point
   DIST_POINT1, // 1st point
   DIST_POINT2, // 2nd point
   DIST_POINT3, // 3rd point
   DIST_EDGE0 , // 0th edge
   DIST_EDGE1 , // 1st edge
   DIST_EDGE2 , // 2nd edge
   DIST_EDGE3 , // 3rd edge
   DIST_PLANE , // plane
   DIST_TRI   , // triangle
   DIST_QUAD  , // quad

   DIST_POINT=DIST_POINT0, // point
   DIST_EDGE =DIST_EDGE0 , // edge
};
/******************************************************************************/
#if EE_PRIVATE
struct Int24
{
   Byte b[3];

   Bool negative()C {return (b[2]>>7)!=0;}
   Int     asInt()C {Int out; CopyFast(&out, b, 3); ((Byte*)&out)[3]=(negative() ? 0xFF : 0); return out;}

   Int24& operator=(Int i) {CopyFast(b, &i, 3); return T;}
};
#endif
/******************************************************************************/
Bool Special(C Flt &r); // if 'r' is a special type float  (NaN, -Inf, +Inf)
Bool Special(C Dbl &r); // if 'r' is a special type double (NaN, -Inf, +Inf)
Bool NaN    (C Flt &r); // if 'r' is Not a Number
Bool NaN    (C Dbl &r); // if 'r' is Not a Number
Bool Inf    (C Flt &r); // if 'r' is Infinite
Bool Inf    (C Dbl &r); // if 'r' is Infinite

// get minimum and maximum
inline void MinMax (  Byte   a, Byte b  , Byte  &min, Byte  &max) {if(a<b){min=a; max=b;}else{min=b; max=a;}} // get min max value: min=Min(a, b); max=Max(a, b);
inline void MinMax (  Int    a, Int  b  , Int   &min, Int   &max) {if(a<b){min=a; max=b;}else{min=b; max=a;}} // get min max value: min=Min(a, b); max=Max(a, b);
inline void MinMax (  Flt    a, Flt  b  , Flt   &min, Flt   &max) {if(a<b){min=a; max=b;}else{min=b; max=a;}} // get min max value: min=Min(a, b); max=Max(a, b);
inline void MinMax (  Dbl    a, Dbl  b  , Dbl   &min, Dbl   &max) {if(a<b){min=a; max=b;}else{min=b; max=a;}} // get min max value: min=Min(a, b); max=Max(a, b);
       void MinMax (C Flt   *f, Int  num, Flt   &min, Flt   &max); // get min max value:   min =Min(f[]);   max =Max(f[]);
       void MinMax (C Dbl   *f, Int  num, Dbl   &min, Dbl   &max); // get min max value:   min =Min(f[]);   max =Max(f[]);
       void MinMaxI(C Flt   *f, Int  num, Int   &min, Int   &max); // get min max index: f[min]=Min(f[]); f[max]=Max(f[]);
       void MinMaxI(C Dbl   *f, Int  num, Int   &min, Int   &max); // get min max index: f[min]=Min(f[]); f[max]=Max(f[]);
       Bool MinMax (C Vec2  *v, Int  num, Vec2  &min, Vec2  &max);
       Bool MinMax (C VecD2 *v, Int  num, VecD2 &min, VecD2 &max);
       Bool MinMax (C Vec   *v, Int  num, Vec   &min, Vec   &max);
       Bool MinMax (C VecD  *v, Int  num, VecD  &min, VecD  &max);
       Bool MinMax (C Vec4  *v, Int  num, Vec4  &min, Vec4  &max);
       Bool MinMax (C VecD4 *v, Int  num, VecD4 &min, VecD4 &max);

// get absolute value
constexpr Int   Abs(  Int     x) {return (x>=0) ? x : -x;}
constexpr Long  Abs(  Long    x) {return (x>=0) ? x : -x;}
constexpr Flt   Abs(  Flt     x) {return (x>=0) ? x : -x;}
constexpr Dbl   Abs(  Dbl     x) {return (x>=0) ? x : -x;}
inline    Vec2  Abs(C Vec2   &v) {return Vec2 (Abs(v.x), Abs(v.y)                    );}
inline    VecD2 Abs(C VecD2  &v) {return VecD2(Abs(v.x), Abs(v.y)                    );}
inline    VecI2 Abs(C VecI2  &v) {return VecI2(Abs(v.x), Abs(v.y)                    );}
inline    VecB2 Abs(C VecSB2 &v) {return VecB2(Abs(v.x), Abs(v.y)                    );}
inline    Vec   Abs(C Vec    &v) {return Vec  (Abs(v.x), Abs(v.y), Abs(v.z)          );}
inline    VecD  Abs(C VecD   &v) {return VecD (Abs(v.x), Abs(v.y), Abs(v.z)          );}
inline    VecI  Abs(C VecI   &v) {return VecI (Abs(v.x), Abs(v.y), Abs(v.z)          );}
inline    VecB  Abs(C VecSB  &v) {return VecB (Abs(v.x), Abs(v.y), Abs(v.z)          );}
inline    Vec4  Abs(C Vec4   &v) {return Vec4 (Abs(v.x), Abs(v.y), Abs(v.z), Abs(v.w));}
inline    VecD4 Abs(C VecD4  &v) {return VecD4(Abs(v.x), Abs(v.y), Abs(v.z), Abs(v.w));}
inline    VecI4 Abs(C VecI4  &v) {return VecI4(Abs(v.x), Abs(v.y), Abs(v.z), Abs(v.w));}
inline    VecB4 Abs(C VecSB4 &v) {return VecB4(Abs(v.x), Abs(v.y), Abs(v.z), Abs(v.w));}

// make absolute value
inline void ABS(Int  &x) {if(x<0)x=-x;}              // works as "x=Abs(x);"
inline void ABS(Long &x) {if(x<0)x=-x;}              // works as "x=Abs(x);"
inline void ABS(Flt  &x) {((U32&) x)   &=~SIGN_BIT;} // works as "x=Abs(x);"
inline void ABS(Dbl  &x) {((U32*)&x)[1]&=~SIGN_BIT;} // works as "x=Abs(x);"

#if EE_PRIVATE
// get absolute scale
inline Flt AbsScale(Flt x, Flt base) {return (x>base) ? x/base : base/x;}
inline Dbl AbsScale(Dbl x, Dbl base) {return (x>base) ? x/base : base/x;}
#endif

// get middle value
constexpr Int  Mid(  Int   x,   Int   min,   Int   max) {return (x>=max) ? max : (x<=min) ? min : x;}
constexpr Long Mid(  Long  x,   Long  min,   Long  max) {return (x>=max) ? max : (x<=min) ? min : x;}
constexpr Flt  Mid(  Flt   x,   Int   min,   Int   max) {return (x>=max) ? max : (x<=min) ? min : x;}
constexpr Flt  Mid(  Flt   x,   Int   min,   Flt   max) {return (x>=max) ? max : (x<=min) ? min : x;}
constexpr Flt  Mid(  Flt   x,   Flt   min,   Int   max) {return (x>=max) ? max : (x<=min) ? min : x;}
constexpr Flt  Mid(  Flt   x,   Flt   min,   Flt   max) {return (x>=max) ? max : (x<=min) ? min : x;}
constexpr Dbl  Mid(  Dbl   x,   Dbl   min,   Dbl   max) {return (x>=max) ? max : (x<=min) ? min : x;}
inline    Vec2 Mid(C Vec2 &v, C Vec2 &min, C Vec2 &max) {return Vec2(Mid(v.x, min.x, max.x), Mid(v.y, min.y, max.y)                                                );}
inline    Vec  Mid(C Vec  &v, C Vec  &min, C Vec  &max) {return Vec (Mid(v.x, min.x, max.x), Mid(v.y, min.y, max.y), Mid(v.z, min.z, max.z)                        );}
inline    Vec4 Mid(C Vec4 &v, C Vec4 &min, C Vec4 &max) {return Vec4(Mid(v.x, min.x, max.x), Mid(v.y, min.y, max.y), Mid(v.z, min.z, max.z), Mid(v.w, min.w, max.w));}

// get saturated (clamped to 0..1)
constexpr Int   Sat(  Int    x) {return Mid(x, 0   , 1   );}
constexpr Long  Sat(  Long   x) {return Mid(x, 0LL , 1LL );}
constexpr Flt   Sat(  Flt    x) {return Mid(x, 0.0f, 1.0f);}
constexpr Dbl   Sat(  Dbl    x) {return Mid(x, 0.0 , 1.0 );}
inline    Vec2  Sat(C Vec2  &v) {return Vec2 (Sat(v.x), Sat(v.y)                    );}
inline    VecD2 Sat(C VecD2 &v) {return VecD2(Sat(v.x), Sat(v.y)                    );}
inline    Vec   Sat(C Vec   &v) {return Vec  (Sat(v.x), Sat(v.y), Sat(v.z)          );}
inline    VecD  Sat(C VecD  &v) {return VecD (Sat(v.x), Sat(v.y), Sat(v.z)          );}
inline    Vec4  Sat(C Vec4  &v) {return Vec4 (Sat(v.x), Sat(v.y), Sat(v.z), Sat(v.w));}
inline    VecD4 Sat(C VecD4 &v) {return VecD4(Sat(v.x), Sat(v.y), Sat(v.z), Sat(v.w));}

// make saturated (clamp to 0..1)
inline Int & SAT(Int  &x) {return x=Sat(x);}
inline Long& SAT(Long &x) {return x=Sat(x);}
inline Flt & SAT(Flt  &x) {return x=Sat(x);}
inline Dbl & SAT(Dbl  &x) {return x=Sat(x);}

// sign
constexpr Int SignBool(Bool x) {return x ? 1 : -1;} // sign from Bool

constexpr Int   Sign(  Int    x) {return (x>0) ? 1 : (x<0) ? -1 : 0;} // sign
constexpr Int   Sign(  Long   x) {return (x>0) ? 1 : (x<0) ? -1 : 0;} // sign
constexpr Int   Sign(  Flt    x) {return (x>0) ? 1 : (x<0) ? -1 : 0;} // sign
constexpr Int   Sign(  Dbl    x) {return (x>0) ? 1 : (x<0) ? -1 : 0;} // sign
inline    VecI2 Sign(C VecI2 &v) {return VecI2(Sign(v.x), Sign(v.y)                      );}
inline    VecI  Sign(C VecI  &v) {return VecI (Sign(v.x), Sign(v.y), Sign(v.z)           );}
inline    VecI4 Sign(C VecI4 &v) {return VecI4(Sign(v.x), Sign(v.y), Sign(v.z), Sign(v.w));}
inline    VecI2 Sign(C Vec2  &v) {return VecI2(Sign(v.x), Sign(v.y)                      );}
inline    VecI  Sign(C Vec   &v) {return VecI (Sign(v.x), Sign(v.y), Sign(v.z)           );}
inline    VecI4 Sign(C Vec4  &v) {return VecI4(Sign(v.x), Sign(v.y), Sign(v.z), Sign(v.w));}

inline Int    SignEps (  Flt   x, Flt eps=EPS ) {return (x>eps) ? 1 : (x<-eps) ? -1 : 0;} // sign using epsilon
inline Int    SignEps (  Dbl   x, Dbl eps=EPSD) {return (x>eps) ? 1 : (x<-eps) ? -1 : 0;} // sign using epsilon
inline VecI2  SignEps (C Vec2 &v, Flt eps=EPS ) {return VecI2 (SignEps(v.x, eps), SignEps(v.y, eps)                                      );}
inline VecI   SignEps (C Vec  &v, Flt eps=EPS ) {return VecI  (SignEps(v.x, eps), SignEps(v.y, eps), SignEps(v.z, eps)                   );}
inline VecI4  SignEps (C Vec4 &v, Flt eps=EPS ) {return VecI4 (SignEps(v.x, eps), SignEps(v.y, eps), SignEps(v.z, eps), SignEps(v.w, eps));}
inline VecSB2 SignEpsB(C Vec2 &v, Flt eps=EPS ) {return VecSB2(SignEps(v.x, eps), SignEps(v.y, eps)                                      );}
inline VecSB  SignEpsB(C Vec  &v, Flt eps=EPS ) {return VecSB (SignEps(v.x, eps), SignEps(v.y, eps), SignEps(v.z, eps)                   );}
inline VecSB4 SignEpsB(C Vec4 &v, Flt eps=EPS ) {return VecSB4(SignEps(v.x, eps), SignEps(v.y, eps), SignEps(v.z, eps), SignEps(v.w, eps));}

inline Int CompareEps(C Flt &a, C Flt &b) {return SignEps(a-b);}
inline Int CompareEps(C Dbl &a, C Dbl &b) {return SignEps(a-b);}

inline Flt   PackInf(Flt x) {return 1-1/(x+1);} //   pack value from 0..Inf to 0..1
inline Flt UnpackInf(Flt x) {return 1/(1-x)-1;} // unpack value from 0..1   to 0..Inf

inline Flt ScaleFactor (Flt x) {return (x>=0) ? (1+x) : (1/(1-x));} // get scaling factor from linear  value
inline Dbl ScaleFactor (Dbl x) {return (x>=0) ? (1+x) : (1/(1-x));} // get scaling factor from linear  value
inline Flt ScaleFactorR(Flt s) {return (s>=1) ? (s-1) : (1-(1/s));} // get linear  value  from scaling factor
inline Dbl ScaleFactorR(Dbl s) {return (s>=1) ? (s-1) : (1-(1/s));} // get linear  value  from scaling factor

Vec2  ScaleFactor(C Vec2  &vec);   Vec2  ScaleFactorR(C Vec2  &vec);
VecD2 ScaleFactor(C VecD2 &vec);   VecD2 ScaleFactorR(C VecD2 &vec);
Vec   ScaleFactor(C Vec   &vec);   Vec   ScaleFactorR(C Vec   &vec);
VecD  ScaleFactor(C VecD  &vec);   VecD  ScaleFactorR(C VecD  &vec);
Vec4  ScaleFactor(C Vec4  &vec);   Vec4  ScaleFactorR(C Vec4  &vec);
VecD4 ScaleFactor(C VecD4 &vec);   VecD4 ScaleFactorR(C VecD4 &vec);

constexpr Int   Sqr(  Int   x) {return x*x;} // square = x**2
constexpr UInt  Sqr(  UInt  x) {return x*x;} // square = x**2
constexpr Long  Sqr(  Long  x) {return x*x;} // square = x**2
constexpr ULong Sqr(  ULong x) {return x*x;} // square = x**2
constexpr Flt   Sqr(  Flt   x) {return x*x;} // square = x**2
constexpr Dbl   Sqr(  Dbl   x) {return x*x;} // square = x**2
inline    Vec2  Sqr(C Vec2 &x) {return x*x;} // square = x**2
inline    Vec   Sqr(C Vec  &x) {return x*x;} // square = x**2
inline    Vec4  Sqr(C Vec4 &x) {return x*x;} // square = x**2

constexpr Int   Cube(Int   x) {return x*x*x;} // cube = x**3
constexpr UInt  Cube(UInt  x) {return x*x*x;} // cube = x**3
constexpr Long  Cube(Long  x) {return x*x*x;} // cube = x**3
constexpr ULong Cube(ULong x) {return x*x*x;} // cube = x**3
constexpr Flt   Cube(Flt   x) {return x*x*x;} // cube = x**3
constexpr Dbl   Cube(Dbl   x) {return x*x*x;} // cube = x**3

constexpr Int   Quart(Int   x) {return Sqr(x*x);} // quartic = x**4
constexpr UInt  Quart(UInt  x) {return Sqr(x*x);} // quartic = x**4
constexpr Long  Quart(Long  x) {return Sqr(x*x);} // quartic = x**4
constexpr ULong Quart(ULong x) {return Sqr(x*x);} // quartic = x**4
constexpr Flt   Quart(Flt   x) {return Sqr(x*x);} // quartic = x**4
constexpr Dbl   Quart(Dbl   x) {return Sqr(x*x);} // quartic = x**4

constexpr Int   Quint(Int   x) {return Quart(x)*x;} // quintic = x**5
constexpr UInt  Quint(UInt  x) {return Quart(x)*x;} // quintic = x**5
constexpr Long  Quint(Long  x) {return Quart(x)*x;} // quintic = x**5
constexpr ULong Quint(ULong x) {return Quart(x)*x;} // quintic = x**5
constexpr Flt   Quint(Flt   x) {return Quart(x)*x;} // quintic = x**5
constexpr Dbl   Quint(Dbl   x) {return Quart(x)*x;} // quintic = x**5

constexpr Int  SqrS(Int  x) {return (x>=0) ? Sqr(x) : -Sqr(x);} // sign preserving square
constexpr Long SqrS(Long x) {return (x>=0) ? Sqr(x) : -Sqr(x);} // sign preserving square
constexpr Flt  SqrS(Flt  x) {return (x>=0) ? Sqr(x) : -Sqr(x);} // sign preserving square
constexpr Dbl  SqrS(Dbl  x) {return (x>=0) ? Sqr(x) : -Sqr(x);} // sign preserving square

// make square
inline Int  & SQR(Int   &x) {return x*=x;}
inline UInt & SQR(UInt  &x) {return x*=x;}
inline Long & SQR(Long  &x) {return x*=x;}
inline ULong& SQR(ULong &x) {return x*=x;}
inline Flt  & SQR(Flt   &x) {return x*=x;}
inline Dbl  & SQR(Dbl   &x) {return x*=x;}

// fast inverse sqrt approximation
INLINE Flt RSqrtSimd(Flt x)
{
#if X86
   __m128 vx=_mm_set_ss   (x);
   __m128 vy=_mm_rsqrt_ss (vx);
   return    _mm_cvtss_f32(vy);
#elif ARM
   float32x2_t vx=vdup_n_f32 (x);
   float32x2_t vy=vrsqrte_f32(vx);
   return vy[0];
#else
   return 1/SqrtFast(x);
#endif
}
inline Flt RSqrt0(Flt x) // ~1/Sqrt(x) inverse square root, high speed, low  precision
{
#if X86 || ARM // faster/similar speed, but more precision
   return RSqrtSimd(x);
#else
   Int i=0x5F3759DF-((Int&)x>>1); // initial guess
   Flt y=(Flt&)i;
   return y;
#endif
}
inline Flt RSqrt1(Flt x) // ~1/Sqrt(x) inverse square root, med  speed, med  precision
{
#if X86 || ARM // faster/similar speed, but more precision
   return RSqrtSimd(x);
#else
   Flt x_2=x/2;
   Int i=0x5F3759DF-((Int&)x>>1); // initial guess
   Flt y=(Flt&)i;
   y*=1.5f-(x_2*y*y); // 1st Newton iteration
   return y;
#endif
}
Flt RSqrt2(Flt x); // ~1/Sqrt(x) inverse square root, low  speed, high precision

#if EE_PRIVATE
      UInt  SqrtI(UInt  x               ); // integer square root (binary    method, fastest)
       Int  SqrtI( Int  x               ); // integer square root (binary    method, fastest)
      UInt  SqrtI(ULong x               ); // integer square root (binary    method, fastest)
       Int  SqrtI( Long x               ); // integer square root (binary    method, fastest)
      UInt  SqrtI(UInt  x, Int max_steps); // integer square root (iterative method, slower but can be faster if "max_steps<=2")
#endif
inline Flt  SqrtFast(  Int   x) {return sqrtf((Flt)x);}                                    //                 square root, returns      NaN  for negative values
inline Flt  SqrtFast(  Flt   x) {return sqrtf(     x);}                                    //                 square root, returns      NaN  for negative values
inline Dbl  SqrtFast(  Dbl   x) {return sqrt (     x);}                                    //                 square root, returns      NaN  for negative values
       Flt  Sqrt    (  Int   x);                                                           //                 square root, returns        0  for negative values
       Flt  Sqrt    (  Flt   x);                                                           //                 square root, returns        0  for negative values
       Dbl  Sqrt    (  Dbl   x);                                                           //                 square root, returns        0  for negative values
inline Vec2 Sqrt    (C Vec2 &x) {return Vec2(Sqrt(x.x), Sqrt(x.y));}                       //                 square root, returns        0  for negative values
inline Vec  Sqrt    (C Vec  &x) {return Vec (Sqrt(x.x), Sqrt(x.y), Sqrt(x.z));}            //                 square root, returns        0  for negative values
inline Vec4 Sqrt    (C Vec4 &x) {return Vec4(Sqrt(x.x), Sqrt(x.y), Sqrt(x.z), Sqrt(x.w));} //                 square root, returns        0  for negative values
       Flt  SqrtS   (  Int   x);                                                           // sign preserving square root, returns -Sqrt(-x) for negative values
       Flt  SqrtS   (  Flt   x);                                                           // sign preserving square root, returns -Sqrt(-x) for negative values
       Dbl  SqrtS   (  Dbl   x);                                                           // sign preserving square root, returns -Sqrt(-x) for negative values
inline Flt  Cbrt    (  Int   x) {return cbrtf((Flt)x);}                                    //                 cube   root, works         ok  for negative values
inline Flt  Cbrt    (  Flt   x) {return cbrtf(     x);}                                    //                 cube   root, works         ok  for negative values
inline Dbl  Cbrt    (  Dbl   x) {return cbrt (     x);}                                    //                 cube   root, works         ok  for negative values

inline Flt Pow(Int x, Flt y) {return powf((Flt)x,      y);} // raise 'x' to the power 'y' = Exp2(Log2(x)*y)
inline Flt Pow(Flt x, Int y) {return powf(     x, (Flt)y);} // raise 'x' to the power 'y' = Exp2(Log2(x)*y)
inline Flt Pow(Flt x, Flt y) {return powf(     x,      y);} // raise 'x' to the power 'y' = Exp2(Log2(x)*y)
inline Dbl Pow(Int x, Dbl y) {return pow (     x,      y);} // raise 'x' to the power 'y' = Exp2(Log2(x)*y)
inline Dbl Pow(Dbl x, Int y) {return pow (     x,      y);} // raise 'x' to the power 'y' = Exp2(Log2(x)*y)
inline Dbl Pow(Dbl x, Dbl y) {return pow (     x,      y);} // raise 'x' to the power 'y' = Exp2(Log2(x)*y)

inline Flt  Exp(  Flt   x) {return expf(x);} // raise 'e' to the power 'x' = Exp2(x/Log(2)) = Exp2(x*1.4426950408889634)
inline Dbl  Exp(  Dbl   x) {return exp (x);} // raise 'e' to the power 'x' = Exp2(x/Log(2)) = Exp2(x*1.4426950408889634)
inline Vec2 Exp(C Vec2 &v) {return Vec2(Exp(v.x), Exp(v.y));}
inline Vec  Exp(C Vec  &v) {return Vec (Exp(v.x), Exp(v.y), Exp(v.z));}
inline Vec4 Exp(C Vec4 &v) {return Vec4(Exp(v.x), Exp(v.y), Exp(v.z), Exp(v.w));}

inline Flt  Exp2(  Flt   x) {return exp2f(x);} // raise '2' to the power 'x'
inline Dbl  Exp2(  Dbl   x) {return exp2 (x);} // raise '2' to the power 'x'
inline Vec2 Exp2(C Vec2 &v) {return Vec2(Exp2(v.x), Exp2(v.y));}
inline Vec  Exp2(C Vec  &v) {return Vec (Exp2(v.x), Exp2(v.y), Exp2(v.z));}
inline Vec4 Exp2(C Vec4 &v) {return Vec4(Exp2(v.x), Exp2(v.y), Exp2(v.z), Exp2(v.w));}

inline Flt Ln  (Flt x) {return logf (x);} // e-base logarithm
inline Dbl Ln  (Dbl x) {return log  (x);} // e-base logarithm
inline Flt Log2(Flt x) {return log2f(x);} // 2-base logarithm
inline Dbl Log2(Dbl x) {return log2 (x);} // 2-base logarithm
       Flt Log (Flt x, Flt base);         //        logarithm
       Dbl Log (Dbl x, Dbl base);         //        logarithm

Flt Pinch      (Flt x, Flt pinch       ); // pinch, 'x'=0..1, 'pinch'       =   0..Inf (<1 makes the curve start slow and accelerate, 1 makes the curve linear, >1 makes the curve start fast and deccelerate)
Flt PinchFactor(Flt x, Flt pinch_factor); // pinch, 'x'=0..1, 'pinch_factor'=-Inf..Inf (<0 makes the curve start slow and accelerate, 0 makes the curve linear, >0 makes the curve start fast and deccelerate)

// calculate squared distance of coordinates from point zero
inline Int Dist2(Int x, Int y              ) {return x*x + y*y            ;}
inline Flt Dist2(Flt x, Flt y              ) {return x*x + y*y            ;}
inline Dbl Dist2(Dbl x, Dbl y              ) {return x*x + y*y            ;}
inline Int Dist2(Int x, Int y, Int z       ) {return x*x + y*y + z*z      ;}
inline Flt Dist2(Flt x, Flt y, Flt z       ) {return x*x + y*y + z*z      ;}
inline Dbl Dist2(Dbl x, Dbl y, Dbl z       ) {return x*x + y*y + z*z      ;}
inline Flt Dist2(Flt x, Flt y, Flt z, Flt w) {return x*x + y*y + z*z + w*w;}

// calculate distance of coordinates from point zero
Flt Dist(Int x, Int y       );
Flt Dist(Flt x, Flt y       );
Dbl Dist(Dbl x, Dbl y       );
Flt Dist(Int x, Int y, Int z);
Flt Dist(Flt x, Flt y, Flt z);
Dbl Dist(Dbl x, Dbl y, Dbl z);

// density
Flt       SplitAlpha  (Flt alpha  , Int steps); // calculate alpha to be used for drawing graphics using 'steps' amount of steps to generate the same results as when drawing with 'alpha' using only 1 step
Flt     VisibleOpacity(Flt density, Flt range); // calculate visible     opacity (0..1) having 'density' environment density (0..1), and 'range' (0..Inf)
Flt AccumulatedDensity(Flt density, Flt range); // calculate accumulated density (0..1) having 'density' environment density (0..1), and 'range' (0..Inf)

// sigmoid
Flt SigmoidExp    (Flt x);
Flt SigmoidExp2   (Flt x);
Flt SigmoidDiv    (Flt x);
Flt SigmoidAtan   (Flt x);
Flt SigmoidSqrt   (Flt x);
Flt SigmoidSqrtInv(Flt y); // inverse function of 'SigmoidSqrt', SigmoidSqrtInv(SigmoidSqrt(x))=x
Flt SigmoidPow    (Flt x, Flt y);
Flt SigmoidGd     (Flt x);
Flt SigmoidTanh   (Flt x);
Flt SigmoidErf    (Flt x);

#if EE_PRIVATE
       Flt DistDot(Flt dist2, Flt dist_plane); // calculate distance scaled by angle, 'dist2'=squared distance, 'dist_plane'=distance along the plane of interest
inline Flt LengthMul(C Vec &a, C Vec &b) {return SqrtFast(a.length2()*b.length2());} // return multiplication of vector lengths, faster version of "a.length()*b.length()"
#endif
/******************************************************************************/
// ANGLES
/******************************************************************************/
inline Flt  Sin   (  Flt   angle) {return sinf(angle);}       // get   sine of angle, -1..1
inline Dbl  Sin   (  Dbl   angle) {return sin (angle);}       // get   sine of angle, -1..1
inline Flt  Cos   (  Flt   angle) {return cosf(angle);}       // get cosine of angle, -1..1
inline Dbl  Cos   (  Dbl   angle) {return cos (angle);}       // get cosine of angle, -1..1
       void CosSin(  Flt  &cos, Flt &sin, Flt angle);         // get cosine and sine of angle, cos=Cos(angle), sin=Sin(angle)
       void CosSin(  Dbl  &cos, Dbl &sin, Dbl angle);         // get cosine and sine of angle, cos=Cos(angle), sin=Sin(angle)
inline Flt  Tan   (  Flt   angle) {return tanf(angle);}       // get   tangent "sin/cos" of angle, -Inf..Inf
inline Dbl  Tan   (  Dbl   angle) {return tan (angle);}       // get   tangent "sin/cos" of angle, -Inf..Inf
       Vec2 Tan   (C Vec2 &angle);                            // get   tangent "sin/cos" of angle, -Inf..Inf
inline Flt  Ctg   (  Flt   angle) {return tanf(PI_2 -angle);} // get cotangent "cos/sin" of angle, -Inf..Inf, alternative: {Flt t=tanf(angle); return t ? 1.0f/t : 0;}
inline Dbl  Ctg   (  Dbl   angle) {return tan (PID_2-angle);} // get cotangent "cos/sin" of angle, -Inf..Inf, alternative: {Dbl t=tan (angle); return t ? 1.0 /t : 0;}
       Flt  Acos  (  Flt   cos  );                            // get arc  cosine (angle which has specified  cosine),     0..PI
       Dbl  Acos  (  Dbl   cos  );                            // get arc  cosine (angle which has specified  cosine),     0..PI
       Flt  Asin  (  Flt   sin  );                            // get arc    sine (angle which has specified    sine), -PI_2..PI_2
       Dbl  Asin  (  Dbl   sin  );                            // get arc    sine (angle which has specified    sine), -PI_2..PI_2
inline Flt  Atan  (  Flt   tan  ) {return atanf(tan);}        // get arc tangent (angle which has specified tangent), -PI_2..PI_2
inline Dbl  Atan  (  Dbl   tan  ) {return atan (tan);}        // get arc tangent (angle which has specified tangent), -PI_2..PI_2
       Vec2 Atan  (C Vec2 &tan  );                            // get arc tangent (angle which has specified tangent), -PI_2..PI_2

inline Flt AcosFast(Flt cos) {return acosf(cos);} // get arc cosine (angle which has specified cosine),     0..PI  , returns NaN for values out of -1..1 range
inline Dbl AcosFast(Dbl cos) {return acos (cos);} // get arc cosine (angle which has specified cosine),     0..PI  , returns NaN for values out of -1..1 range
inline Flt AsinFast(Flt sin) {return asinf(sin);} // get arc   sine (angle which has specified   sine), -PI_2..PI_2, returns NaN for values out of -1..1 range
inline Dbl AsinFast(Dbl sin) {return asin (sin);} // get arc   sine (angle which has specified   sine), -PI_2..PI_2, returns NaN for values out of -1..1 range
#if EE_PRIVATE
Flt ACosSin(Flt cos, Flt sin); // get angle which has specified cosine and sine, 0..PI (this assumes "sin>=0"), this is faster than 'Angle', however point needs to be normalized (X=cos, Y=sin), and remember that this function ignores "sin<0"
Dbl ACosSin(Dbl cos, Dbl sin); // get angle which has specified cosine and sine, 0..PI (this assumes "sin>=0"), this is faster than 'Angle', however point needs to be normalized (X=cos, Y=sin), and remember that this function ignores "sin<0"
#endif

inline Flt Angle  (Flt x, Flt y) {return atan2f(  y,   x);} // get point angle (-PI..PI)
inline Dbl Angle  (Dbl x, Dbl y) {return atan2 (  y,   x);} // get point angle (-PI..PI)
inline Flt Angle  (C Vec2  &v  ) {return atan2f(v.y, v.x);} // get point angle (-PI..PI)
inline Dbl Angle  (C VecD2 &v  ) {return atan2 (v.y, v.x);} // get point angle (-PI..PI)
       Flt Angle  (C Vec   &v,                 C Vec &x, C Vec &y);                                                   // get point angle according to given axes            (-PI..PI)
inline Flt Angle  (C Vec   &v, C Vec     &pos, C Vec &x, C Vec &y) {return Angle(v-pos,                x,        y);} // get point angle according to given axes and center (-PI..PI)
inline Flt Angle  (C Vec   &v, C Matrix3 &matrix                 ) {return Angle(v            , matrix.x, matrix.y);} // get point angle according to given matrix          (-PI..PI)
inline Flt AngleXZ(C Vec   &v, C Matrix3 &matrix                 ) {return Angle(v            , matrix.x, matrix.z);} // get point angle according to given matrix          (-PI..PI)
inline Flt Angle  (C Vec   &v, C Matrix  &matrix                 ) {return Angle(v, matrix.pos, matrix.x, matrix.y);} // get point angle according to given matrix          (-PI..PI)
inline Flt AngleXZ(C Vec   &v, C Matrix  &matrix                 ) {return Angle(v, matrix.pos, matrix.x, matrix.z);} // get point angle according to given matrix          (-PI..PI)

Flt AngleFast(Flt x, Flt y); // get point angle approximation (-PI..PI)
Flt AngleFast(C Vec2 &v   ); // get point angle approximation (-PI..PI)
#if EE_PRIVATE
Flt Angle1Fast(Flt x, Flt y); // get point angle approximation (-1..1)

Vec2 AngleFastToPos (Flt angle_fast); // convert angle approximation (-PI..PI) into a point position
Vec2 Angle1FastToPos(Flt angle_fast); // convert angle approximation ( -1..1 ) into a point position

       Flt AngleFast  (C Vec  &v,                 C Vec &x, C Vec &y);                                                       // get point angle approximation according to given axes            (-PI..PI)
inline Flt AngleFast  (C Vec  &v, C Vec     &pos, C Vec &x, C Vec &y) {return AngleFast(v-pos,                x,        y);} // get point angle approximation according to given axes and center (-PI..PI)
inline Flt AngleFast  (C Vec  &v, C Matrix3 &matrix                 ) {return AngleFast(v            , matrix.x, matrix.y);} // get point angle approximation according to given matrix          (-PI..PI)
inline Flt AngleFastXZ(C Vec  &v, C Matrix3 &matrix                 ) {return AngleFast(v            , matrix.x, matrix.z);} // get point angle approximation according to given matrix          (-PI..PI)
inline Flt AngleFast  (C Vec  &v, C Matrix  &matrix                 ) {return AngleFast(v, matrix.pos, matrix.x, matrix.y);} // get point angle approximation according to given matrix          (-PI..PI)
inline Flt AngleFastXZ(C Vec  &v, C Matrix  &matrix                 ) {return AngleFast(v, matrix.pos, matrix.x, matrix.z);} // get point angle approximation according to given matrix          (-PI..PI)
#endif

inline Flt AngleFull     (Flt angle       ) {return Frac(angle, PI2 );}       // normalize angle to   0..PI2 (wrap angle to   0..PI2 range)
inline Dbl AngleFull     (Dbl angle       ) {return Frac(angle, PID2);}       // normalize angle to   0..PI2 (wrap angle to   0..PI2 range)
       Flt AngleNormalize(Flt angle       );                                  // normalize angle to -PI..PI  (wrap angle to -PI..PI  range)
       Dbl AngleNormalize(Dbl angle       );                                  // normalize angle to -PI..PI  (wrap angle to -PI..PI  range)
inline Flt AngleDelta    (Flt from, Flt to) {return AngleNormalize(to-from);} // get angle delta    -PI..PI  (difference between angles wrapped to -PI..PI range)
inline Dbl AngleDelta    (Dbl from, Dbl to) {return AngleNormalize(to-from);} // get angle delta    -PI..PI  (difference between angles wrapped to -PI..PI range)

inline Flt CosSin(Flt cos_sin) {return Sqrt(1-cos_sin*cos_sin);} // convert cos<->sin, this function converts sine value to cosine value (of the same angle), and cosine value to sine value (of the same angle) at the same time
inline Dbl CosSin(Dbl cos_sin) {return Sqrt(1-cos_sin*cos_sin);} // convert cos<->sin, this function converts sine value to cosine value (of the same angle), and cosine value to sine value (of the same angle) at the same time

#if EE_PRIVATE
inline Flt CosSin2(Flt cos_sin_2) {return Sqrt(1-cos_sin_2);} // convert cos2->sin, sin2->cos, this function converts sine value to cosine value (of the same angle), and cosine value to sine value (of the same angle) at the same time
inline Dbl CosSin2(Dbl cos_sin_2) {return Sqrt(1-cos_sin_2);} // convert cos2->sin, sin2->cos, this function converts sine value to cosine value (of the same angle), and cosine value to sine value (of the same angle) at the same time

inline Flt CosSinPrecise(Flt cos_sin) {return (Flt)CosSin(Dbl (cos_sin));} // this is more precise than regular 'CosSin' and still fast
inline Dbl CosSinPrecise(Dbl cos_sin) {return      Sin   (Acos(cos_sin));} // this is more precise than regular 'CosSin' but slow
#endif

inline Flt CalcZ(C Vec2 &v) {return Sqrt(1 - v.x*v.x - v.y*v.y);}

// angle between vectors
       Flt    AngleBetween (C Vec2  &a, C Vec2  &b);                                  // -PI..PI, 'a b'=direction vectors (they DON'T NEED to be normalized)
       Dbl    AngleBetween (C VecD2 &a, C VecD2 &b);                                  // -PI..PI, 'a b'=direction vectors (they DON'T NEED to be normalized)
inline Flt AbsAngleBetween (C Vec2  &a, C Vec2  &b) {return Abs(AngleBetween(a, b));} //   0..PI, 'a b'=direction vectors (they DON'T NEED to be normalized)
inline Dbl AbsAngleBetween (C VecD2 &a, C VecD2 &b) {return Abs(AngleBetween(a, b));} //   0..PI, 'a b'=direction vectors (they DON'T NEED to be normalized)
       Flt AbsAngleBetween (C Vec   &a, C Vec   &b);                                  //   0..PI, 'a b'=direction vectors (they DON'T NEED to be normalized)
       Dbl AbsAngleBetween (C VecD  &a, C VecD  &b);                                  //   0..PI, 'a b'=direction vectors (they DON'T NEED to be normalized)
       Flt AbsAngleBetweenN(C Vec   &a, C Vec   &b);                                  //   0..PI, 'a b'=direction vectors (they       NEED to be normalized)
       Dbl AbsAngleBetweenN(C VecD  &a, C VecD  &b);                                  //   0..PI, 'a b'=direction vectors (they       NEED to be normalized)
       Flt    AngleBetween (C Vec   &a, C Vec   &b, C Vec  &z);                       // -PI..PI, 'a b'=direction vectors (they DON'T NEED to be normalized), 'z'=forward axis orientation (it doesn't need to be normalized)
       Dbl    AngleBetween (C VecD  &a, C VecD  &b, C VecD &z);                       // -PI..PI, 'a b'=direction vectors (they DON'T NEED to be normalized), 'z'=forward axis orientation (it doesn't need to be normalized)

// cos of angle between vectors (they DON'T NEED to be normalized)
Flt CosBetween(C Vec2  &a, C Vec2  &b); // -1..1
Dbl CosBetween(C VecD2 &a, C VecD2 &b); // -1..1
Flt CosBetween(C Vec   &a, C Vec   &b); // -1..1
Dbl CosBetween(C VecD  &a, C VecD  &b); // -1..1

// cos of angle between vectors (they NEED to be normalized)
inline Flt CosBetweenN(C Vec2  &a, C Vec2  &b) {return Dot(a, b);} // -1..1
inline Dbl CosBetweenN(C VecD2 &a, C VecD2 &b) {return Dot(a, b);} // -1..1
inline Flt CosBetweenN(C Vec   &a, C Vec   &b) {return Dot(a, b);} // -1..1
inline Dbl CosBetweenN(C VecD  &a, C VecD  &b) {return Dot(a, b);} // -1..1

// sin of angle between vectors (they DON'T NEED to be normalized)
Flt    SinBetween(C Vec2  &a, C Vec2  &b); // -1..1
Dbl    SinBetween(C VecD2 &a, C VecD2 &b); // -1..1
Flt AbsSinBetween(C Vec   &a, C Vec   &b); //  0..1
Dbl AbsSinBetween(C VecD  &a, C VecD  &b); //  0..1

// sin of angle between vectors (they NEED to be normalized)
inline Flt    SinBetweenN(C Vec2  &a, C Vec2  &b) {return Cross(a, b)         ;} // -1..1
inline Dbl    SinBetweenN(C VecD2 &a, C VecD2 &b) {return Cross(a, b)         ;} // -1..1
inline Flt AbsSinBetweenN(C Vec   &a, C Vec   &b) {return Cross(a, b).length();} //  0..1
inline Dbl AbsSinBetweenN(C VecD  &a, C VecD  &b) {return Cross(a, b).length();} //  0..1

#if EE_PRIVATE
inline Flt    AngleBetween(C Vec2  &prev, C Vec2  &cur, C Vec2  &next) {return    AngleBetween(prev-cur, next-cur);} // -PI..PI
inline Dbl    AngleBetween(C VecD2 &prev, C VecD2 &cur, C VecD2 &next) {return    AngleBetween(prev-cur, next-cur);} // -PI..PI
inline Flt AbsAngleBetween(C Vec2  &prev, C Vec2  &cur, C Vec2  &next) {return AbsAngleBetween(prev-cur, next-cur);} //   0..PI
inline Dbl AbsAngleBetween(C VecD2 &prev, C VecD2 &cur, C VecD2 &next) {return AbsAngleBetween(prev-cur, next-cur);} //   0..PI
inline Flt AbsAngleBetween(C Vec   &prev, C Vec   &cur, C Vec   &next) {return AbsAngleBetween(prev-cur, next-cur);} //   0..PI
inline Dbl AbsAngleBetween(C VecD  &prev, C VecD  &cur, C VecD  &next) {return AbsAngleBetween(prev-cur, next-cur);} //   0..PI

inline Flt CosBetween(C Vec2  &prev, C Vec2  &cur, C Vec2  &next) {return CosBetween(prev-cur, next-cur);} // -1..1
inline Dbl CosBetween(C VecD2 &prev, C VecD2 &cur, C VecD2 &next) {return CosBetween(prev-cur, next-cur);} // -1..1
inline Flt CosBetween(C Vec   &prev, C Vec   &cur, C Vec   &next) {return CosBetween(prev-cur, next-cur);} // -1..1
inline Dbl CosBetween(C VecD  &prev, C VecD  &cur, C VecD  &next) {return CosBetween(prev-cur, next-cur);} // -1..1
#endif

inline Flt AngleFlipX (Flt angle) {return PI -angle;} // flip angle so that (x, y)->(-x, y), where x=Cos(angle), y=Sin(angle)
inline Flt AngleFlipY (Flt angle) {return PI2-angle;} // flip angle so that (x, y)->( x,-y), where x=Cos(angle), y=Sin(angle)
inline Flt AngleFlipXY(Flt angle) {return PI +angle;} // flip angle so that (x, y)->(-x,-y), where x=Cos(angle), y=Sin(angle)

// average direction
void AvgDirU(Vec2 &dir, C CMemPtr<Vec2> &points); // get average direction from array of points, points should be located around center Vec2(0,0), direction is not normalized
/******************************************************************************/
// BLENDING
/******************************************************************************/
// get smooth curves that start slowly from 0, accelerate, and slow down to 1
#if EE_PRIVATE
       Flt _SmoothQuintic(Flt x           ); // fast version that doesn't check for 'x' being out of 0..1 range, this function is not an approximate of 'SmoothSin', it's sharper
#endif
       Flt  SmoothSqr    (Flt x           );
inline Flt _SmoothCube   (Flt x           ) {return (3-2*x)*x*x;} // fast version that doesn't check for 'x' being out of 0..1 range
       Flt  SmoothCube   (Flt x           );
       Flt  SmoothCubeInv(Flt y           ); // inverse function of 'SmoothCube', SmoothCubeInv(SmoothCube(x))=x
       Flt  SmoothCube2  (Flt x           ); // this function is sharper than 'SmoothCube'
       Flt  SmoothSextic (Flt x           );
       Flt  SmoothSin    (Flt x           );
       Flt  SmoothPow    (Flt x, Flt pow  );
       Flt  SmoothPinch  (Flt x, Flt pinch);

// get smooth curves that return blend value (0..1), basing on the 'x' distance (-1..1), highest intensity '1' is returned for "x==0" which smoothly drops to zero when 'x' becomes more away from the '0' position and reaches '-1' or '+1' or greater distances
Flt BlendSqr         (Flt x); // get        square blend value (0..1), basing on the 'x' distance (-1..1), highest intensity '1' is returned for "x==0" which smoothly drops to zero when 'x' becomes more away from the '0' position and reaches '-1' or '+1' or greater distances
Flt BlendSmoothSqr   (Flt x); // get smooth square blend value (0..1), basing on the 'x' distance (-1..1), highest intensity '1' is returned for "x==0" which smoothly drops to zero when 'x' becomes more away from the '0' position and reaches '-1' or '+1' or greater distances
Flt BlendSmoothCube  (Flt x); // get smooth cubic  blend value (0..1), basing on the 'x' distance (-1..1), highest intensity '1' is returned for "x==0" which smoothly drops to zero when 'x' becomes more away from the '0' position and reaches '-1' or '+1' or greater distances
Flt BlendSmoothSextic(Flt x); // get smooth sextic blend value (0..1), basing on the 'x' distance (-1..1), highest intensity '1' is returned for "x==0" which smoothly drops to zero when 'x' becomes more away from the '0' position and reaches '-1' or '+1' or greater distances
Flt BlendSmoothSin   (Flt x); // get smooth sine   blend value (0..1), basing on the 'x' distance (-1..1), highest intensity '1' is returned for "x==0" which smoothly drops to zero when 'x' becomes more away from the '0' position and reaches '-1' or '+1' or greater distances

constexpr Int BlendSmoothCubeSum    (Int range) {return range          ;} // get sum of all weights for all "-range..range" steps, calculated using "Flt weight=0; for(Int dist=-range; dist<=range; dist++)weight+=BlendSmoothCube(dist/Flt(range));"
constexpr Flt BlendSmoothCubeSumHalf(Int range) {return range*0.5f+0.5f;} // get sum of all weights for all "     0..range" steps, calculated using "Flt weight=0; for(Int dist=     0; dist<=range; dist++)weight+=BlendSmoothCube(dist/Flt(range));"

Flt Gaussian(Flt x); // get the Probability Density Function (PDF) of the Normal (Gaussian) distribution "Exp(-x*x)", this function returns highest value of 1 at "x==0" and drops towards zero when 'x' becomes more away from the '0' position, theoretically it never reaches zero value, however due to floating point numerical precision zero will be reached for 'x' values far from '0' position

Flt  SmoothOffset(Flt  &offset, Flt max_length);
Vec2 SmoothOffset(Vec2 &offset, Flt max_length);
Vec  SmoothOffset(Vec  &offset, Flt max_length);
/******************************************************************************/
// INTERPOLATION
/******************************************************************************/
// get tangent, based on previous and next value
Flt  GetTangent   (  Flt   prev,   Flt   next);
Vec2 GetTangent   (C Vec2 &prev, C Vec2 &next);
Vec  GetTangent   (C Vec  &prev, C Vec  &next);
Vec4 GetTangent   (C Vec4 &prev, C Vec4 &next);
Vec2 GetTangentDir(C Vec2 &prev, C Vec2 &next); // get tangent for direction vectors
Vec  GetTangentDir(C Vec  &prev, C Vec  &next); // get tangent for direction vectors
Flt  GetTangent   (  Flt   prev,   Flt   cur,   Flt   next); // get tangent from previous, current and next values
Vec2 GetTangent   (C Vec2 &prev, C Vec2 &cur, C Vec2 &next); // get tangent from previous, current and next values
Vec  GetTangent   (C Vec  &prev, C Vec  &cur, C Vec  &next); // get tangent from previous, current and next values
Vec4 GetTangent   (C Vec4 &prev, C Vec4 &cur, C Vec4 &next); // get tangent from previous, current and next values

// linear interpolation, 'step'=0..1
inline Flt   Lerp(  Int    from,   Int    to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Flt   Lerp(  Int    from,   Flt    to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Flt   Lerp(  Flt    from,   Int    to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Flt   Lerp(  Flt    from,   Flt    to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Dbl   Lerp(  Dbl    from,   Dbl    to,   Dbl  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Vec2  Lerp(C VecI2 &from, C VecI2 &to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Vec2  Lerp(C Vec2  &from, C Vec2  &to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline VecD2 Lerp(C VecD2 &from, C VecD2 &to,   Dbl  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Vec   Lerp(C VecI  &from, C VecI  &to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Vec   Lerp(C Vec   &from, C Vec   &to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Vec   Lerp(C Vec   &from, C Vec   &to, C Vec &step) {return from+step*(to-from);}
inline VecD  Lerp(C VecD  &from, C VecD  &to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline VecD  Lerp(C VecD  &from, C VecD  &to,   Dbl  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Vec4  Lerp(C VecI4 &from, C VecI4 &to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline Vec4  Lerp(C Vec4  &from, C Vec4  &to,   Flt  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
inline VecD4 Lerp(C VecD4 &from, C VecD4 &to,   Dbl  step) {return from+step*(to-from);} // faster than "from*(1-step) + to*step"
#if EE_PRIVATE
VecB4 Lerp(C VecB4 &from, C VecB4 &to, Flt step);
VecB4 Lerp(C VecB4 &a   , C VecB4 &b , C VecB4 &c, C Vec &blend); // linear interpolation between vectors, returns a*blend.x + b*blend.y + c*blend.z
#endif

// linear interpolation reverse, get interpolation step from value, these functions return 0 if "value==from" and 1 if "value==to", in other cases returned value is interpolated between 0..1
inline Flt LerpR(  Int    from,   Int    to,   Int    value) {return Flt(value-from)/(to-from);}
inline Flt LerpR(  Int    from,   Int    to,   Flt    value) {return    (value-from)/(to-from);}
inline Flt LerpR(  Int    from,   Flt    to,   Flt    value) {return    (value-from)/(to-from);}
inline Flt LerpR(  Flt    from,   Int    to,   Flt    value) {return    (value-from)/(to-from);}
inline Flt LerpR(  Flt    from,   Flt    to,   Flt    value) {return    (value-from)/(to-from);}
inline Dbl LerpR(  Dbl    from,   Dbl    to,   Dbl    value) {return    (value-from)/(to-from);}
       Flt LerpR(C Vec2  &from, C Vec2  &to, C Vec2  &value);
       Dbl LerpR(C VecD2 &from, C VecD2 &to, C VecD2 &value);
       Flt LerpR(C Vec   &from, C Vec   &to, C Vec   &value);
       Dbl LerpR(C VecD  &from, C VecD  &to, C VecD  &value);
       Flt LerpR(C Vec4  &from, C Vec4  &to, C Vec4  &value);
       Dbl LerpR(C VecD4 &from, C VecD4 &to, C VecD4 &value);

// linear interpolation reverse saturated, get saturated interpolation step from value, these functions return 0 if "value==from" and 1 if "value==to", in other cases returned value is interpolated between 0..1 and clamped to 0..1 range
inline Flt LerpRS(  Int    from,   Int    to,   Int    value) {return Sat(LerpR(from, to, value));}
inline Flt LerpRS(  Int    from,   Int    to,   Flt    value) {return Sat(LerpR(from, to, value));}
inline Flt LerpRS(  Int    from,   Flt    to,   Flt    value) {return Sat(LerpR(from, to, value));}
inline Flt LerpRS(  Flt    from,   Int    to,   Flt    value) {return Sat(LerpR(from, to, value));}
inline Flt LerpRS(  Flt    from,   Flt    to,   Flt    value) {return Sat(LerpR(from, to, value));}
inline Dbl LerpRS(  Dbl    from,   Dbl    to,   Dbl    value) {return Sat(LerpR(from, to, value));}
inline Flt LerpRS(C Vec2  &from, C Vec2  &to, C Vec2  &value) {return Sat(LerpR(from, to, value));}
inline Dbl LerpRS(C VecD2 &from, C VecD2 &to, C VecD2 &value) {return Sat(LerpR(from, to, value));}
inline Flt LerpRS(C Vec   &from, C Vec   &to, C Vec   &value) {return Sat(LerpR(from, to, value));}
inline Dbl LerpRS(C VecD  &from, C VecD  &to, C VecD  &value) {return Sat(LerpR(from, to, value));}
inline Flt LerpRS(C Vec4  &from, C Vec4  &to, C Vec4  &value) {return Sat(LerpR(from, to, value));}
inline Dbl LerpRS(C VecD4 &from, C VecD4 &to, C VecD4 &value) {return Sat(LerpR(from, to, value));}

Flt LerpAngle(Flt from, Flt to, Flt step); // angle based interpolation, 'step'=0..1

void Lerp4Weights(Vec4 &weights, Flt step); // get weights used for 'Lerp4' function, 'step'=0..1, sum of weights is always equal to 1

Flt  Lerp4(  Flt   v0,   Flt   v1,   Flt   v2,   Flt   v3, Flt step); // 4 values based interpolation (hermite spline), smoothly interpolate between v1..v2, 'step'=0..1
Vec2 Lerp4(C Vec2 &v0, C Vec2 &v1, C Vec2 &v2, C Vec2 &v3, Flt step); // 4 values based interpolation (hermite spline), smoothly interpolate between v1..v2, 'step'=0..1
Vec  Lerp4(C Vec  &v0, C Vec  &v1, C Vec  &v2, C Vec  &v3, Flt step); // 4 values based interpolation (hermite spline), smoothly interpolate between v1..v2, 'step'=0..1
Vec4 Lerp4(C Vec4 &v0, C Vec4 &v1, C Vec4 &v2, C Vec4 &v3, Flt step); // 4 values based interpolation (hermite spline), smoothly interpolate between v1..v2, 'step'=0..1

Flt  LerpTan(  Flt   from,   Flt   to, Flt step,   Flt   tan0,   Flt   tan1); // tangent based interpolation (hermite spline), 'step'=0..1
Vec2 LerpTan(C Vec2 &from, C Vec2 &to, Flt step, C Vec2 &tan0, C Vec2 &tan1); // tangent based interpolation (hermite spline), 'step'=0..1
Vec  LerpTan(C Vec  &from, C Vec  &to, Flt step, C Vec  &tan0, C Vec  &tan1); // tangent based interpolation (hermite spline), 'step'=0..1
Vec4 LerpTan(C Vec4 &from, C Vec4 &to, Flt step, C Vec4 &tan0, C Vec4 &tan1); // tangent based interpolation (hermite spline), 'step'=0..1

inline Flt LerpSmoothSqr   (Flt from, Flt to, Flt step           ) {return Lerp(from, to, SmoothSqr   (step       ));}
inline Flt LerpSmoothCube  (Flt from, Flt to, Flt step           ) {return Lerp(from, to, SmoothCube  (step       ));}
inline Flt LerpSmoothCube2 (Flt from, Flt to, Flt step           ) {return Lerp(from, to, SmoothCube2 (step       ));}
inline Flt LerpSmoothSextic(Flt from, Flt to, Flt step           ) {return Lerp(from, to, SmoothSextic(step       ));}
inline Flt LerpSmoothSin   (Flt from, Flt to, Flt step           ) {return Lerp(from, to, SmoothSin   (step       ));}
inline Flt LerpSmoothPow   (Flt from, Flt to, Flt step, Flt pow  ) {return Lerp(from, to, SmoothPow   (step, pow  ));}
inline Flt LerpSmoothPinch (Flt from, Flt to, Flt step, Flt pinch) {return Lerp(from, to, SmoothPinch (step, pinch));}
/******************************************************************************/
// ADJUST VALUE
/******************************************************************************/
void AdjustValDir    (Flt  &value,   Int      dir,           Flt dv                       ); // adjust value by direction, move 'value' according to 'dir' directon * 'dv'     and clamps it to -1..1, if 'dir'==0 then moves 'value' towards 0 by 'dv'
void AdjustValDir    (Flt  &value,   Int      dir,           Flt change  , Flt reset      ); // adjust value by direction, move 'value' according to 'dir' directon * 'change' and clamps it to -1..1, if 'dir'==0 then moves 'value' towards 0 by 'reset'
void AdjustValBool   (Flt  &value,   Bool      on,           Flt dv                       ); // adjust value by Bool     , increases/decreases 'value' and clamps it to 0..1, "value=Sat(on ? value+dv : value-dv)"
void AdjustValBool   (Flt  &value,   Bool      on,           Flt inc     , Flt dec        ); // adjust value by Bool     , increases/decreases 'value' and clamps it to 0..1, "value=Sat(on ? value+dv : value-dv)"
void AdjustValBoolSet(Flt  &value,   Bool      on, Bool set, Flt dv                       ); // adjust value by Bool or immediate set, "if(set)value=on;else AdjustValBool(value, on, dv      );"
void AdjustValBoolSet(Flt  &value,   Bool      on, Bool set, Flt inc     , Flt dec        ); // adjust value by Bool or immediate set, "if(set)value=on;else AdjustValBool(value, on, inc, dec);"
void AdjustValTime   (Flt  &value,   Flt   target,           Flt exponent, Flt dt=Time.d()); // adjust value to target by smooth time interpolation, 'exponent'=0..1
void AdjustValTime   (Dbl  &value,   Dbl   target,           Flt exponent, Flt dt=Time.d()); // adjust value to target by smooth time interpolation, 'exponent'=0..1
void AdjustValTime   (Vec2 &value, C Vec2 &target,           Flt exponent, Flt dt=Time.d()); // adjust value to target by smooth time interpolation, 'exponent'=0..1
void AdjustValTime   (Vec  &value, C Vec  &target,           Flt exponent, Flt dt=Time.d()); // adjust value to target by smooth time interpolation, 'exponent'=0..1
void AdjustValTime   (VecD &value, C VecD &target,           Flt exponent, Flt dt=Time.d()); // adjust value to target by smooth time interpolation, 'exponent'=0..1
void AdjustValTarget (Flt  &value,   Flt   target,           Flt dv                       ); // adjust value to target, move     'value' towards 'target' by 'dv'
void AdjustValTarget (Dbl  &value,   Dbl   target,           Dbl dv                       ); // adjust value to target, move     'value' towards 'target' by 'dv'
void AdjustValTarget (Vec2 &value, C Vec2 &target,           Flt dv                       ); // adjust value to target, move     'value' towards 'target' by 'dv'
void AdjustValTarget (Vec  &value, C Vec  &target,           Flt dv                       ); // adjust value to target, move     'value' towards 'target' by 'dv'
void AdjustValTarget (VecD &value, C VecD &target,           Dbl dv                       ); // adjust value to target, move     'value' towards 'target' by 'dv'
void AdjustValTarget (Flt  &value,   Flt   target,           Flt inc     , Flt dec        ); // adjust value to target, increase 'value' towards 'target' by 'inc' or decrease 'value' towards 'target' by 'dec'
/******************************************************************************/
// EQUATIONS
/******************************************************************************/
// solve equations of given formula
// return number of solutions (-1=infinite, 0=none, 1=one, 2=two, 3=three), set 'x' as solutions

// polynominals
inline Int Polynominal0(Flt a) {return a ? 0 : -1;}                             // a                       = 0
inline Int Polynominal0(Dbl a) {return a ? 0 : -1;}                             // a                       = 0
       Int Polynominal1(Flt a, Flt b,               Flt &x                   ); // a*x   + b               = 0
       Int Polynominal1(Dbl a, Dbl b,               Dbl &x                   ); // a*x   + b               = 0
       Int Polynominal2(Flt a, Flt b, Flt c,        Flt &x0, Flt &x1         ); // a*x^2 + b*x   + c       = 0
       Int Polynominal2(Dbl a, Dbl b, Dbl c,        Dbl &x0, Dbl &x1         ); // a*x^2 + b*x   + c       = 0
       Int Polynominal3(Flt a, Flt b, Flt c, Flt d, Flt &x0, Flt &x1, Flt &x2); // a*x^3 + b*x^2 + c*x + d = 0
       Int Polynominal3(Dbl a, Dbl b, Dbl c, Dbl d, Dbl &x0, Dbl &x1, Dbl &x2); // a*x^3 + b*x^2 + c*x + d = 0

// simultaneous equations
Int Solve(Flt a1, Flt a2, Flt b1, Flt b2, Flt c1, Flt c2, Flt &x, Flt &y);
Int Solve(Dbl a1, Dbl a2, Dbl b1, Dbl b2, Dbl c1, Dbl c2, Dbl &x, Dbl &y);
// x*a1 + y*b1 = c1
// x*a2 + y*b2 = c2
/******************************************************************************/
// CONVERT UNITS
/******************************************************************************/
inline Flt  DegToRad(  Int   deg) {return deg*(PI /180);} // convert Degrees to Radians
inline Flt  DegToRad(  Flt   deg) {return deg*(PI /180);} // convert Degrees to Radians
inline Dbl  DegToRad(  Dbl   deg) {return deg*(PID/180);} // convert Degrees to Radians
inline Vec2 DegToRad(C Vec2 &deg) {return deg*(PI /180);} // convert Degrees to Radians
inline Vec  DegToRad(C Vec  &deg) {return deg*(PI /180);} // convert Degrees to Radians
inline Vec4 DegToRad(C Vec4 &deg) {return deg*(PI /180);} // convert Degrees to Radians

inline Flt  RadToDeg(  Flt   rad) {return rad*(180/PI );} // convert Radians to Degrees
inline Dbl  RadToDeg(  Dbl   rad) {return rad*(180/PID);} // convert Radians to Degrees
inline Vec2 RadToDeg(C Vec2 &rad) {return rad*(180/PI );} // convert Radians to Degrees
inline Vec  RadToDeg(C Vec  &rad) {return rad*(180/PI );} // convert Radians to Degrees
inline Vec4 RadToDeg(C Vec4 &rad) {return rad*(180/PI );} // convert Radians to Degrees

inline Flt MpsToKmph   (Flt x) {return x*((60*60)/1000.0f);} // convert "Meter     per Second" To "Kilometer per Hour  "
inline Flt KmphToMps   (Flt x) {return x*(1000.0f/(60*60));} // convert "Kilometer per Hour  " To "Meter     per Second"
inline Flt RadpsToRotpm(Flt x) {return x*(60/PI2         );} // convert "Radian    per Second" To "Rotation  per Minute"
inline Flt RotpmToRadps(Flt x) {return x*(PI2/60         );} // convert "Rotation  per Minute" To "Radian    per Second"

#if EE_PRIVATE
inline Byte FltToByte(Flt  f) {return Mid(RoundPos(f*255), 0, 255);} // 0..1   -> 0..255, it's okay to clamp after converting to int for small values
inline Flt  ByteToFlt(Byte b) {return              b/255.0f       ;} // 0..255 -> 0..1  , faster than using 'ByteToFltArray'

inline SByte SFltToSByte(Flt f) {return Mid(Round(f*127          ), -127, 127);} // -1..1 -> -127..127, it's okay to clamp after converting to int for small values
inline  Byte SFltToUByte(Flt f) {return Mid(Round(f*127.5f+127.5f),    0, 255);} // -1..1 ->    0..255, it's okay to clamp after converting to int for small values

inline Flt SByteToSFlt(SByte b) {return (b<=-127) ? -1 : b/127.0f      ;} // -127..127 -> -1..1
inline Flt UByteToSFlt( Byte b) {return                  b*(2.0f/255)-1;} //    0..255 -> -1..1

inline Byte FltToU2(Flt  f) {return Mid(RoundPos(f*3), 0, 3);} // 0..1 -> 0..3, it's okay to clamp after converting to int for small values
inline Flt  U2ToFlt(Byte u) {return u/3.0f                  ;} // 0..3 -> 0..1

inline UInt FltToU10(Flt  f) {return Mid(RoundPos(f*1023), 0, 1023);} // 0..1    -> 0..1023, it's okay to clamp after converting to int for small values
inline Flt  U10ToFlt(UInt u) {return u/1023.0f                     ;} // 0..1023 -> 0..1

inline UInt FltToU12(Flt  f) {return RoundPos(Sat(f)*4095);} // 0..1    -> 0..4095
inline Flt  U12ToFlt(UInt u) {return u/4095.0f            ;} // 0..4095 -> 0..1

inline Flt  ShortToSFlt (Short s) {return         (s<=-32767) ? -1 : s/32767.0f;} // -32767..32767 -> -1..1
inline Short SFltToShort(Flt   f) {return RoundPos(Mid(f, -1.0f, 1.0f)*32767)  ;} // -1..1 -> -32767..32767

inline UInt FltToU16(Flt  f) {return RoundPos(Sat(f)*65535);} // 0..1     -> 0..65535
inline Flt  U16ToFlt(UInt u) {return u/65535.0f            ;} // 0..65535 -> 0..1

inline Byte U1ToByte(Bool x) {return  x*255            ;} // 0..1   -> 0..255
inline Byte U2ToByte(Byte x) {return (x*255/*+ 1*/)/  3;} // 0..3   -> 0..255 (this version exactly matches float with Round, +1 is not needed in this case, function will return the same value with or without it)
inline Byte U3ToByte(Byte x) {return (x*255+   3  )/  7;} // 0..7   -> 0..255 (this version exactly matches float with Round)
inline Byte U4ToByte(Byte x) {return (x<<4)|x          ;} // 0..15  -> 0..255 (this version exactly matches float with Round, same as   "(x*255/*+ 7*/)/ 15" but faster, +7 is not needed in this case, function will return the same value with or without it)
inline Byte U5ToByte(Byte x) {return (x*255  +15  )/ 31;} // 0..31  -> 0..255 (this version exactly matches float with Round, can't use "(x<<3)|(x>>2)"      because it has error of "1")
inline Byte U6ToByte(Byte x) {return (x*255  +31  )/ 63;} // 0..63  -> 0..255 (this version exactly matches float with Round, can't use "(x<<2)|(x>>4)"      because it has error of "1")
inline Byte U7ToByte(Byte x) {return (x<<1)|(x>>6)     ;} // 0..127 -> 0..255 (this version exactly matches float with Round, same as   "(x*255  +63  )/127" but faster)

inline Byte ByteToU2 (Byte x) {return (x>>6      )    ;} // 0..255 -> 0..3
inline UInt ByteToU10(Byte x) {return (x*1023+127)/255;} // 0..255 -> 0..1023 , can't use "(x<<2)|(x>>6)"     because it has error of "1"
inline UInt ByteToU16(Byte x) {return (x<<8)|x        ;} // 0..255 -> 0..65535, same as   "(x*65535+127)/255" but faster

inline Byte SByteToByte(SByte s) {return (s<=0) ? 0 : (s<<1)|(s>>6);} // -128..0 -> 0 , 0..127 -> 0..255, same as "(s*255+63)/127" but faster
#endif
/******************************************************************************/
// TIME, DISTANCE, VELOCITY, ACCELERATION
/******************************************************************************/
inline Flt ZeroVelTravelTime (Flt dist, Flt accel) {return 2*SqrtFast(accel*dist)/accel;} // get time needed to travel 'dist' distance using 'accel' acceleration, assuming that initial velocity is zero, first half is accelerated with 'accel', second half decelerated with 'accel' and final velocity is zero
inline Flt ZeroVelTravelDist (Flt time, Flt accel) {return            accel*Sqr(time)/4;} // get travelled distance in 'time'          using 'accel' acceleration, assuming that initial velocity is zero, first half is accelerated with 'accel', second half decelerated with 'accel' and final velocity is zero
inline Flt ZeroVelTravelAccel(Flt time, Flt dist ) {return            dist /Sqr(time)*4;} // get acceleration needed to travel 'dist' distance in 'time'         , assuming that initial velocity is zero, first half is accelerated with 'accel', second half decelerated with 'accel' and final velocity is zero
/******************************************************************************/
extern const Int PrimeNumbers[16]; // first 16 prime numbers = { 2, 3, 5, .. }
/******************************************************************************/
