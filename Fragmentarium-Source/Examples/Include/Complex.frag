#donotrun
/*----------------------------------------------------------------

float functions not present in early GLSL versions:
  cosh, sinh, tanh

complex number functions:
  add, sub, conj, norm, abs, arg, sqr, mul, inverse, div, sqrt, exp, log, pow,
  sin, cos, tan, sinh, cosh, tanh, asin, acos, atan, asinh, acosh, atanh

dual complex number functions for automatic differentiation:
  const, var, deriv,
  add, sub, norm, abs, arg, sqr, mul, inverse, div, sqrt, exp, log, pow,
  sin, cos, tan, sinh, cosh, tanh, asin, acos, atan, asinh, acosh, atanh

for GLSL >= 400, double precision without trig functions:

double precision complex number functions:
  add, sub, conj, norm, abs, sqr, mul, inverse, div, sqrt

double precision dual complex number functions for automatic differentiation:
  const, var, deriv, add, sub, norm, abs, sqr, mul, inverse, div, sqrt

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
// float functions not present in early GLSL versions

#if __VERSION__ < 130
float cosh(float val)
{
	float tmp = exp(val);
	float cosH = (tmp + 1.0 / tmp) / 2.0;
	return cosH;
}

float tanh(float val)
{
	float tmp = exp(val);
	float tanH = (tmp - 1.0 / tmp) / (tmp + 1.0 / tmp);
	return tanH;
}

float sinh(float val)
{
	float tmp = exp(val);
	float sinH = (tmp - 1.0 / tmp) / 2.0;
	return sinH;
}
#endif

//----------------------------------------------------------------
// complex number functions

vec2 cAdd( vec2 a, float s ) {
  return vec2( a.x+s, a.y );
}

vec2 cAdd( float s, vec2 a ) {
  return vec2( s+a.x, a.y );
}

vec2 cAdd( vec2 a, vec2 s ) {
  return a + s;
}

vec2 cSub( vec2 a, float s ) {
  return vec2( a.x-s, a.y );
}

vec2 cSub( float s, vec2 a ) {
  return vec2( s-a.x, -a.y );
}

vec2 cSub( vec2 a, vec2 s ) {
  return a - s;
}

vec2 cConj( vec2 z ) {
  return vec2(z.x,-z.y);
}

float cNorm(vec2 z) {
	return dot(z, z);
}

float cAbs(vec2 z) {
	return length(z);
}

float cArg(vec2 a) {
	return atan(a.y,a.x);
}

vec2 cSqr(vec2 z) {
	return vec2(z.x*z.x-z.y*z.y,2.*z.x*z.y);
}

vec2 cMul(vec2 a, vec2 b) {
	return vec2( a.x*b.x -  a.y*b.y,a.x*b.y + a.y * b.x);
}

vec2 cInverse(vec2 a) {
	return	vec2(a.x,-a.y)/dot(a,a);
}

vec2 cDiv( vec2 a, vec2 b ) {
  float d = dot(b,b);
  return vec2( dot(a,b), a.y*b.x - a.x*b.y ) / d;
}

vec2 cSqrt( vec2 z ) {
  float m = length(z);
  return sqrt( max(vec2(0.0), 0.5*vec2(m+z.x, m-z.x)) ) *
    vec2( 1.0, sign(z.y) );
}

vec2 cExp(vec2 z) {
	return exp(z.x) * vec2(cos(z.y), sin(z.y));
}

// TODO fix those frags that need this to be set...
#ifdef USE_COMPLEX_1_0_31

// non-standard branch cut, bad PI value too...
vec2 cLog(vec2 a) {
	float b =  atan(a.y,a.x);
	if (b>0.0) b-=2.0*3.1415;
	return vec2(log(length(a)),b);
}

// same as cPow
vec2 cPower(vec2 z, float n) {
	float r2 = dot(z,z);
	return pow(r2,n/2.0)*vec2(cos(n*atan(z.y,z.x)),sin(n*atan(z.y,z.x)));
}

// same as cPower2 specialized to float
vec2 cPow( vec2 z, float n ) {
  float r = length( z );
  float a = atan( z.y, z.x );
  return pow( r, n )*vec2( cos(a*n), sin(a*n) );
}

// this version is sensible but has a stupid name
vec2 cPower2(vec2 z, vec2 a) {
	return cExp(cMul(cLog(z), a));
}

// distance evaluators
// deprecated: use cNorm()
float lengthSquared( in vec2 v ) { return dot(v,v); }

// what is this for? seems unused throughout the entire code base
// seems rather specific, and not really complex-related, so hide it behind
// the backwards-compatibility #define too...
float sdSegmentSquared( vec2 p, vec2 a, vec2 b )
{
  vec2 pa = p-a, ba = b-a;
  float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return lengthSquared( pa - ba*h );
}

#else

vec2 cLog(vec2 a) {
	return vec2(log(cAbs(a)),cArg(a));
}

vec2 cPow(vec2 z, vec2 a) {
	return cExp(cMul(cLog(z), a));
}

vec2 cPow(vec2 z, float a) {
	return cExp(cLog(z) * a);
}

#endif

vec2 cSin(vec2 z) {
	return vec2(sin(z.x)*cosh(z.y), cos(z.x)*sinh(z.y));
}

vec2 cCos(vec2 z) {
	return vec2(cos(z.x)*cosh(z.y), -sin(z.x)*sinh(z.y));
}

vec2 cTan(vec2 z) {
	return cDiv(cSin(z), cCos(z));
}

vec2 cSinh(vec2 z) {
  return 0.5 * (cExp(z) - cExp(-z));
}

vec2 cCosh(vec2 z) {
  return 0.5 * (cExp(z) + cExp(-z));
}

vec2 cTanh(vec2 z) {
	return cDiv(cSinh(z), cCosh(z));
}

vec2 cAsin(vec2 z) {
  const vec2 I = vec2(0.0, 1.0);
  return cMul(-I, cLog(cMul(I, z) + cSqrt(cSub(1.0, cSqr(z)))));
}

vec2 cAcos(vec2 z) {
  const vec2 I = vec2(0.0, 1.0);
  return cMul(-I, cLog(z + cMul(I, cSqrt(cSub(1.0, cSqr(z))))));
}

vec2 cAtan(vec2 z) {
  const vec2 I = vec2(0.0, 1.0);
  return cDiv
    ( cLog(cAdd(1.0, cMul(I, z))) - cLog(cSub(1.0, cMul(I, z)))
    , 2.0 * I
    );
}

vec2 cAsinh(vec2 z) {
  return cLog(z + cSqrt(cAdd(cSqr(z), 1.0)));
}

vec2 cAcosh(vec2 z) {
  return 2.0 *
    cLog(cSqrt(0.5 * cAdd(z, 1.0)) + cSqrt(0.5 * cSub(z, 1.0)));
}

vec2 cAtanh(vec2 z) {
  return 0.5 * (cLog(cAdd(1.0, z)) - cLog(cSub(1.0, z)));
}

//----------------------------------------------------------------
// dual complex number functions for automatic differentiation

vec4 cConst( float z ) {
  return vec4( z, 0.0, 0.0, 0.0 );
}

vec4 cConst( vec2 z ) {
  return vec4( z, 0.0, 0.0 );
}

vec4 cVar( float z ) {
  return vec4( z, 0.0, 1.0, 0.0 );
}

vec4 cVar( vec2 z ) {
  return vec4( z, 1.0, 0.0 );
}

vec2 cVar( vec4 z ) {
  return z.xy;
}

vec2 cDeriv( vec4 z ) {
  return z.zw;
}

float cNorm(vec4 z) {
	return cNorm(z.xy);
}

float cAbs(vec4 z) {
	return cAbs(z.xy);
}

float cArg(vec4 z) {
	return cArg(z.xy);
}

vec4 cAdd( vec4 z, float a ) {
  return vec4( z.x + a, z.yzw );
}

vec4 cAdd( float a, vec4 z ) {
  return vec4( a + z.x, z.yzw );
}

vec4 cAdd( vec4 z, vec2 a ) {
  return vec4( z.xy + a, z.zw );
}

vec4 cAdd( vec2 a, vec4 z ) {
  return vec4( a + z.xy, z.zw );
}

vec4 cAdd( vec4 a, vec4 z ) {
  return a + z;
}

vec4 cSub( vec4 z, float a ) {
  return vec4( z.x - a, z.yzw );
}

vec4 cSub( float a, vec4 z ) {
  return vec4( a - z.x, -z.yzw );
}

vec4 cSub( vec4 z, vec2 a ) {
  return vec4( z.xy - a, z.zw );
}

vec4 cSub( vec2 a, vec4 z ) {
  return vec4( a - z.xy, -z.zw );
}

vec4 cSub( vec4 a, vec4 z ) {
  return a - z;
}

vec4 cSqr( vec4 z ) {
  return vec4( cSqr(z.xy), 2.0 * cMul(z.xy, z.zw) );
}

vec4 cMul( vec4 a, vec4 b ) {
  return vec4
    ( cMul(a.xy, b.xy)
    , cMul(a.xy, b.zw) + cMul(a.zw, b.xy)
    );
}

vec4 cMul( vec2 a, vec4 b ) {
  return cMul( cConst(a), b );
}

vec4 cMul( vec4 a, vec2 b ) {
  return cMul( a, cConst(b) );
}

vec4 cDiv( vec4 a, vec4 b ) {
  return vec4
    ( cDiv(a.xy, b.xy)
    , cDiv(cMul(a.zw, b.xy) - cMul(a.xy, b.zw), cSqr(b.xy))
    );
}

vec4 cDiv( float a, vec4 b ) {
  return cDiv( cConst(a), b );
}

vec4 cDiv( vec2 a, vec4 b ) {
  return cDiv( cConst(a), b );
}

vec4 cDiv( vec4 a, vec2 b ) {
  return cDiv( a, cConst(b) );
}

vec4 cInverse( vec4 a ) {
  return cDiv(1.0, a);
}

vec4 cSqrt( vec4 a ) {
  vec2 s = cSqrt(a.xy);
  return vec4( s, cDiv(a.zw, 2.0 * s) );
}

vec4 cExp( vec4 a ) {
  vec2 s = cExp(a.xy);
  return vec4( s, cMul(s, a.zw) );
}

vec4 cLog( vec4 a ) {
  return vec4( cLog(a.xy), cDiv(a.zw, a.xy) );
}

vec4 cSin( vec4 z ) {
  const vec2 I = vec2(0.0, 1.0);
  return cDiv(cExp(cMul(I, z)) - cExp(cMul(-I, z)), 2.0 * I);
}

vec4 cCos( vec4 z ) {
  const vec2 I = vec2(0.0, 1.0);
  return cExp(cMul(I, z)) + cExp(cMul(-I, z)) / 2.0;
}

vec4 cTan( vec4 a ) {
  return cDiv(cSin(a), cCos(a));
}

vec4 cSinh(vec4 z) {
  return 0.5 * (cExp(z) - cExp(-z));
}

vec4 cCosh(vec4 z) {
  return 0.5 * (cExp(z) + cExp(-z));
}

vec4 cTanh(vec4 z) {
	return cDiv(cSinh(z), cCosh(z));
}

vec4 cAsin(vec4 z) {
  const vec2 I = vec2(0.0, 1.0);
  return cMul(-I, cLog(cMul(I, z) + cSqrt(cSub(1.0, cSqr(z)))));
}

vec4 cAcos(vec4 z) {
  const vec2 I = vec2(0.0, 1.0);
  return cMul(-I, cLog(z + cMul(I, cSqrt(cSub(1.0, cSqr(z))))));
}

vec4 cAtan(vec4 z) {
  const vec2 I = vec2(0.0, 1.0);
  return cDiv
    ( cLog(cAdd(1.0, cMul(I, z))) - cLog(cSub(1.0, cMul(I, z)))
    , 2.0 * I
    );
}

vec4 cAsinh(vec4 z) {
  return cLog(z + cSqrt(cAdd(cSqr(z), 1.0)));
}

vec4 cAcosh(vec4 z) {
  return 2.0 *
    cLog(cSqrt(0.5 * cAdd(z, 1.0)) + cSqrt(0.5 * cSub(z, 1.0)));
}

vec4 cAtanh(vec4 z) {
  return 0.5 * (cLog(cAdd(1.0, z)) - cLog(cSub(1.0, z)));
}

//----------------------------------------------------------------
// double precision versions of everything (but no trig available)

// TODO check extensions for earlier versions
#if __VERSION__ >= 400

//----------------------------------------------------------------
// double precision complex number functions

dvec2 cAdd( dvec2 a, double s ) {
  return dvec2( a.x+s, a.y );
}

dvec2 cDiv( dvec2 a, dvec2 b ) {
  double d = dot(b,b);
  return dvec2( dot(a,b), a.y*b.x - a.x*b.y ) / d;
}

dvec2 cSqrt( dvec2 z ) {
  double m = length(z);
  return sqrt( max(dvec2(0.0LF), 0.5LF*dvec2(m+z.x, m-z.x)) ) *
    dvec2( 1.0LF, sign(z.y) );
}

dvec2 cConj( dvec2 z ) {
        return dvec2(z.x,-z.y);
}

dvec2 cMul(dvec2 a, dvec2 b) {
	return dvec2( a.x*b.x -  a.y*b.y,a.x*b.y + a.y * b.x);
}

dvec2 cInverse(dvec2 a) {
	return	dvec2(a.x,-a.y)/dot(a,a);
}

double cNorm(dvec2 z) {
	return dot(z, z);
}

double cAbs(dvec2 z) {
	return length(z);
}

dvec2 cSqr(dvec2 z) {
	return dvec2(z.x*z.x-z.y*z.y,2.0LF*z.x*z.y);
}

//----------------------------------------------------------------
// double precision dual complex number functions for automatic differentiation

dvec4 cConst( double z ) {
  return dvec4( z, 0.0LF, 0.0LF, 0.0LF );
}

dvec4 cConst( dvec2 z ) {
  return dvec4( z, 0.0LF, 0.0LF );
}

dvec4 cVar( double z ) {
  return dvec4( z, 0.0LF, 1.0LF, 0.0LF );
}

dvec4 cVar( dvec2 z ) {
  return dvec4( z, 1.0LF, 0.0LF );
}

dvec2 cVar( dvec4 z ) {
  return z.xy;
}

dvec2 cDeriv( dvec4 z ) {
  return z.zw;
}

double cNorm(dvec4 z) {
	return cNorm(z.xy);
}

double cAbs(dvec4 z) {
	return length(z.xy);
}

dvec4 cAdd( dvec4 z, double a ) {
  return dvec4( z.x + a, z.yzw );
}

dvec4 cAdd( double a, dvec4 z ) {
  return dvec4( a + z.x, z.yzw );
}

dvec4 cAdd( dvec4 z, dvec2 a ) {
  return dvec4( z.xy + a, z.zw );
}

dvec4 cAdd( dvec2 a, dvec4 z ) {
  return dvec4( a + z.xy, z.zw );
}

dvec4 cSqr( dvec4 z ) {
  return dvec4( cSqr(z.xy), 2.0LF * cMul(z.xy, z.zw) );
}

dvec4 cMul( dvec4 a, dvec4 b ) {
  return dvec4
    ( cMul(a.xy, b.xy)
    , cMul(a.xy, b.zw) + cMul(a.zw, b.xy)
    );
}

dvec4 cMul( dvec2 a, dvec4 b ) {
  return cMul( cConst(a), b );
}

dvec4 cMul( dvec4 a, dvec2 b ) {
  return cMul( a, cConst(b) );
}

dvec4 cDiv( dvec4 a, dvec4 b ) {
  return dvec4
    ( cDiv(a.xy, b.xy)
    , cDiv(cMul(a.zw, b.xy) - cMul(a.xy, b.zw), cSqr(b.xy))
    );
}

dvec4 cDiv( double a, dvec4 b ) {
  return cDiv( cConst(a), b );
}

dvec4 cDiv( dvec2 a, dvec4 b ) {
  return cDiv( cConst(a), b );
}

dvec4 cDiv( dvec4 a, dvec2 b ) {
  return cDiv( a, cConst(b) );
}

dvec4 cInverse( dvec4 a ) {
  return cDiv(1.0LF, a);
}

dvec4 cSqrt( dvec4 a ) {
  dvec2 s = cSqrt(a.xy);
  return dvec4( s, cDiv(a.zw, 2.0LF * s) );
}

#endif

