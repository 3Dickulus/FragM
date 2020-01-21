#donotrun
// this file is intended to be included by Complex.frag

/*----------------------------------------------------------------

REAL functions not present in early GLSL versions:
  cosh, sinh, tanh

complex number functions:
  add, sub, conj, norm, abs, arg, sqr, mul, inverse, div, sqrt, exp, log, pow,
  sin, cos, tan, sinh, cosh, tanh, asin, acos, atan, asinh, acosh, atanh

dual complex number functions for automatic differentiation:
  const, var, deriv,
  add, sub, norm, abs, arg, sqr, mul, inverse, div, sqrt, exp, log, pow,
  sin, cos, tan, sinh, cosh, tanh, asin, acos, atan, asinh, acosh, atanh

some implementations taken from release 4.10 of the Linux man-pages project.

NOTE:
  some implementations have changed since previous versions, you can...
  #define USE_COMPLEX_1_0_31
  ...before...
  #include "Complex.frag"
  ...to use the earlier version's code (but try to fix the frags that
  depend on the older behaviour before resorting to this).

TODO:
  check extensions as well as version for double support
  test suite
  examples
  optimisations

  add proper credits ;)

  The new functions have been added by Claude @ FF

----------------------------------------------------------------*/

//----------------------------------------------------------------
// complex number functions

VEC2 cAdd( VEC2 a, REAL s ) {
  return VEC2( a.x+s, a.y );
}

VEC2 cAdd( REAL s, VEC2 a ) {
  return VEC2( s+a.x, a.y );
}

VEC2 cAdd( VEC2 a, VEC2 s ) {
  return a + s;
}

VEC2 cSub( VEC2 a, REAL s ) {
  return VEC2( a.x-s, a.y );
}

VEC2 cSub( REAL s, VEC2 a ) {
  return VEC2( s-a.x, -a.y );
}

VEC2 cSub( VEC2 a, VEC2 s ) {
  return a - s;
}

VEC2 cConj( VEC2 z ) {
  return VEC2(z.x,-z.y);
}

REAL cNorm(VEC2 z) {
  return dot(z, z);
}

REAL cAbs(VEC2 z) {
  return length(z);
}

REAL cArg(VEC2 a) {
  return atan(a.y,a.x);
}

VEC2 cSqr(VEC2 z) {
  return VEC2(z.x*z.x-z.y*z.y,2.*z.x*z.y);
}

VEC2 cMul(VEC2 a, VEC2 b) {
  return VEC2( a.x*b.x -  a.y*b.y,a.x*b.y + a.y * b.x);
}

VEC2 cInverse(VEC2 a) {
  return  VEC2(a.x,-a.y)/dot(a,a);
}

VEC2 cDiv( VEC2 a, VEC2 b ) {
  REAL d = dot(b,b);
  return VEC2( dot(a,b), a.y*b.x - a.x*b.y ) / d;
}

VEC2 cSqrt( VEC2 z ) {
  REAL m = length(z);
  return sqrt( max(VEC2(0.0), 0.5*VEC2(m+z.x, m-z.x)) ) *
    VEC2( 1.0, sign(z.y) );
}

VEC2 cExp(VEC2 z) {
  return exp(z.x) * VEC2(cos(z.y), sin(z.y));
}

// TODO fix those frags that need this to be set...
#ifdef USE_COMPLEX_1_0_31

// non-standard branch cut, bad PI value too...
VEC2 cLog(VEC2 a) {
  REAL b =  atan(a.y,a.x);
  if (b>0.0) b-=2.0*REAL(M_PI);
  return VEC2(log(length(a)),b);
}

// same as cPow
VEC2 cPower(VEC2 z, REAL n) {
  REAL r2 = dot(z,z);
  return pow(r2,n/2.0)*VEC2(cos(n*atan(z.y,z.x)),sin(n*atan(z.y,z.x)));
}

// same as cPower2 specialized to REAL
VEC2 cPow( VEC2 z, REAL n ) {
  REAL r = length( z );
  REAL a = atan( z.y, z.x );
  return pow( r, n )*VEC2( cos(a*n), sin(a*n) );
}

// this version is sensible but has a stupid name
VEC2 cPower2(VEC2 z, VEC2 a) {
  return cExp(cMul(cLog(z), a));
}

// distance evaluators
// deprecated: use cNorm()
REAL lengthSquared( in VEC2 v ) { return dot(v,v); }

// what is this for? seems unused throughout the entire code base
// seems rather specific, and not really complex-related, so hide it behind
// the backwards-compatibility #define too...
REAL sdSegmentSquared( VEC2 p, VEC2 a, VEC2 b )
{
  VEC2 pa = p-a, ba = b-a;
  REAL h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return lengthSquared( pa - ba*h );
}

#else

VEC2 cLog(VEC2 a) {
  return VEC2(log(cAbs(a)),cArg(a));
}

VEC2 cPow(VEC2 z, VEC2 a) {
  return cExp(cMul(cLog(z), a));
}

VEC2 cPow(VEC2 z, REAL a) {
  return cExp(cLog(z) * a);
}

#endif

VEC2 cSin(VEC2 z) {
  return VEC2(sin(z.x)*cosh(z.y), cos(z.x)*sinh(z.y));
}

VEC2 cCos(VEC2 z) {
  return VEC2(cos(z.x)*cosh(z.y), -sin(z.x)*sinh(z.y));
}

VEC2 cTan(VEC2 z) {
  return cDiv(cSin(z), cCos(z));
}

VEC2 cSinh(VEC2 z) {
  return 0.5 * (cExp(z) - cExp(-z));
}

VEC2 cCosh(VEC2 z) {
  return 0.5 * (cExp(z) + cExp(-z));
}

VEC2 cTanh(VEC2 z) {
  return cDiv(cSinh(z), cCosh(z));
}

VEC2 cAsin(VEC2 z) {
  const VEC2 I = VEC2(0.0, 1.0);
  return cMul(-I, cLog(cMul(I, z) + cSqrt(cSub(1.0, cSqr(z)))));
}

VEC2 cAcos(VEC2 z) {
  const VEC2 I = VEC2(0.0, 1.0);
  return cMul(-I, cLog(z + cMul(I, cSqrt(cSub(1.0, cSqr(z))))));
}

VEC2 cAtan(VEC2 z) {
  const VEC2 I = VEC2(0.0, 1.0);
  return cDiv
    ( cLog(cAdd(1.0, cMul(I, z))) - cLog(cSub(1.0, cMul(I, z)))
    , 2.0 * I
    );
}

VEC2 cAsinh(VEC2 z) {
  return cLog(z + cSqrt(cAdd(cSqr(z), 1.0)));
}

VEC2 cAcosh(VEC2 z) {
  return 2.0 *
    cLog(cSqrt(0.5 * cAdd(z, 1.0)) + cSqrt(0.5 * cSub(z, 1.0)));
}

VEC2 cAtanh(VEC2 z) {
  return 0.5 * (cLog(cAdd(1.0, z)) - cLog(cSub(1.0, z)));
}

//----------------------------------------------------------------
// dual complex number functions for automatic differentiation

VEC4 cConst( REAL z ) {
  return VEC4( z, 0.0, 0.0, 0.0 );
}

VEC4 cConst( VEC2 z ) {
  return VEC4( z, 0.0, 0.0 );
}

VEC4 cVar( REAL z ) {
  return VEC4( z, 0.0, 1.0, 0.0 );
}

VEC4 cVar( VEC2 z ) {
  return VEC4( z, 1.0, 0.0 );
}

VEC2 cVar( VEC4 z ) {
  return z.xy;
}

VEC2 cDeriv( VEC4 z ) {
  return z.zw;
}

REAL cNorm(VEC4 z) {
  return cNorm(z.xy);
}

REAL cAbs(VEC4 z) {
  return cAbs(z.xy);
}

REAL cArg(VEC4 z) {
  return cArg(z.xy);
}

VEC4 cAdd( VEC4 z, REAL a ) {
  return VEC4( z.x + a, z.yzw );
}

VEC4 cAdd( REAL a, VEC4 z ) {
  return VEC4( a + z.x, z.yzw );
}

VEC4 cAdd( VEC4 z, VEC2 a ) {
  return VEC4( z.xy + a, z.zw );
}

VEC4 cAdd( VEC2 a, VEC4 z ) {
  return VEC4( a + z.xy, z.zw );
}

VEC4 cAdd( VEC4 a, VEC4 z ) {
  return a + z;
}

VEC4 cSub( VEC4 z, REAL a ) {
  return VEC4( z.x - a, z.yzw );
}

VEC4 cSub( REAL a, VEC4 z ) {
  return VEC4( a - z.x, -z.yzw );
}

VEC4 cSub( VEC4 z, VEC2 a ) {
  return VEC4( z.xy - a, z.zw );
}

VEC4 cSub( VEC2 a, VEC4 z ) {
  return VEC4( a - z.xy, -z.zw );
}

VEC4 cSub( VEC4 a, VEC4 z ) {
  return a - z;
}

VEC4 cSqr( VEC4 z ) {
  return VEC4( cSqr(z.xy), 2.0 * cMul(z.xy, z.zw) );
}

VEC4 cMul( VEC4 a, VEC4 b ) {
  return VEC4
    ( cMul(a.xy, b.xy)
    , cMul(a.xy, b.zw) + cMul(a.zw, b.xy)
    );
}

VEC4 cMul( VEC2 a, VEC4 b ) {
  return cMul( cConst(a), b );
}

VEC4 cMul( VEC4 a, VEC2 b ) {
  return cMul( a, cConst(b) );
}

VEC4 cDiv( VEC4 a, VEC4 b ) {
  return VEC4
    ( cDiv(a.xy, b.xy)
    , cDiv(cMul(a.zw, b.xy) - cMul(a.xy, b.zw), cSqr(b.xy))
    );
}

VEC4 cDiv( REAL a, VEC4 b ) {
  return cDiv( cConst(a), b );
}

VEC4 cDiv( VEC2 a, VEC4 b ) {
  return cDiv( cConst(a), b );
}

VEC4 cDiv( VEC4 a, VEC2 b ) {
  return cDiv( a, cConst(b) );
}

VEC4 cInverse( VEC4 a ) {
  return cDiv(1.0, a);
}

VEC4 cSqrt( VEC4 a ) {
  VEC2 s = cSqrt(a.xy);
  return VEC4( s, cDiv(a.zw, 2.0 * s) );
}

VEC4 cExp( VEC4 a ) {
  VEC2 s = cExp(a.xy);
  return VEC4( s, cMul(s, a.zw) );
}

VEC4 cLog( VEC4 a ) {
  return VEC4( cLog(a.xy), cDiv(a.zw, a.xy) );
}

VEC4 cSin( VEC4 z ) {
  const VEC2 I = VEC2(0.0, 1.0);
  return cDiv(cExp(cMul(I, z)) - cExp(cMul(-I, z)), 2.0 * I);
}

VEC4 cCos( VEC4 z ) {
  const VEC2 I = VEC2(0.0, 1.0);
  return cExp(cMul(I, z)) + cExp(cMul(-I, z)) / 2.0;
}

VEC4 cTan( VEC4 a ) {
  return cDiv(cSin(a), cCos(a));
}

VEC4 cSinh(VEC4 z) {
  return 0.5 * (cExp(z) - cExp(-z));
}

VEC4 cCosh(VEC4 z) {
  return 0.5 * (cExp(z) + cExp(-z));
}

VEC4 cTanh(VEC4 z) {
  return cDiv(cSinh(z), cCosh(z));
}

VEC4 cAsin(VEC4 z) {
  const VEC2 I = VEC2(0.0, 1.0);
  return cMul(-I, cLog(cMul(I, z) + cSqrt(cSub(1.0, cSqr(z)))));
}

VEC4 cAcos(VEC4 z) {
  const VEC2 I = VEC2(0.0, 1.0);
  return cMul(-I, cLog(z + cMul(I, cSqrt(cSub(1.0, cSqr(z))))));
}

VEC4 cAtan(VEC4 z) {
  const VEC2 I = VEC2(0.0, 1.0);
  return cDiv
    ( cLog(cAdd(1.0, cMul(I, z))) - cLog(cSub(1.0, cMul(I, z)))
    , 2.0 * I
    );
}

VEC4 cAsinh(VEC4 z) {
  return cLog(z + cSqrt(cAdd(cSqr(z), 1.0)));
}

VEC4 cAcosh(VEC4 z) {
  return 2.0 *
    cLog(cSqrt(0.5 * cAdd(z, 1.0)) + cSqrt(0.5 * cSub(z, 1.0)));
}

VEC4 cAtanh(VEC4 z) {
  return 0.5 * (cLog(cAdd(1.0, z)) - cLog(cSub(1.0, z)));
}
