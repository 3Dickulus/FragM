#donotrun
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

for GLSL >= 400, double precision with trig functions:
  sin, cos, exp, tan, atan, log, log2 log10

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
#if __VERSION__ >= 400 && defined(USE_DOUBLE)
//--------------------------------------------------------------------
// 11/12/17
// double cos() sin() remez exp() by FractalForums.org user clacker
//--------------------------------------------------------------------
// 11/13/17
// remez double atan() by FractalForums.org user 3dickulus
// remez double log() gist.github.com/dhermes modified for FragM
//--------------------------------------------------------------------
//
// The rest? by Claude?
//
//--------------------------------------------------------------------
//
// Fixme pow(n) pow(n,n)
//
//--------------------------------------------------------------------

// Trig functions
const uint TrigIterMax = 20;
#group Trig
uniform int TrigIter;slider[0,5,20]
uniform double TrigLimit;slider[0.001,1.1,1.5]

double M_PI = 	3.14159265358979323846LF;
double M_2PI = M_PI*2.0LF;
double M_PI2 = M_PI/2.0LF;
double M_E =   2.71828182845904523536LF;
double M_EHALF = 1.6487212707001281469LF;

// sine
double sin( double x ){
	int i;
	int counter = 0;
	double sum = x, t = x;
	double s = x;

	if(isnan(x) || isinf(x))
		return 0.0LF;

  while(abs(s) > TrigLimit){
		s = s/3.0;
		counter += 1;
	}

	sum = s;
	t = s;

	for(i=1;i<=TrigIter;i++)
	{
		t=(t*(-1.0)*s*s)/(2.0*double(i)*(2.0*double(i)+1.0));
		sum=sum+t;
	}

	for(i=0;i<counter;i++)
		sum = 3.0*sum - 4.0*sum*sum*sum;
     
	return sum;
}

// cosine
double cos( double x ){
	int i;
	int counter = 0;
	double sum = 1, t = 1;
	double s = x;

	if(isnan(x) || isinf(x))
		return 0.0LF;

  while(abs(s) > TrigLimit){
		s = s/3.0;
		counter += 1;
	}

	for(i=1;i<=TrigIter;i++)
	{
        t=t*(-1.0)*s*s/(2.0*double(i)*(2.0*double(i)-1.0));
        sum=sum+t;
	}

	for(i=0;i<counter;i++)
		sum = -3.0*sum + 4.0*sum*sum*sum;
     
	return sum;
}

/* Approximation of f(x) = exp(x)
 * on interval [ 0, 1.0 ]
 * with a polynomial of degree 10.
 */
 double exp_approx( double x ) {
    double u = 4.5714785424007307e-7LF;
    u = u * x + 2.2861717525121477e-6LF;
    u = u * x + 2.5459354562599535e-5LF;
    u = u * x + 1.9784992840356075e-4LF;
    u = u * x + 1.389195677460962e-3LF;
    u = u * x + 8.3332264219304372e-3LF;
    u = u * x + 4.1666689828374069e-2LF;
    u = u * x + 1.6666666374456794e-1LF;
    u = u * x + 5.0000000018885691e-1LF;
    u = u * x + 9.9999999999524239e-1LF;
    u = u * x + 1.0000000000000198LF;
	if(isnan(u) || isinf(u))
		return 0.0LF;
    return u;
}

double exp(double x){

	int i;
	int n;
   double f;
   double e_accum = M_E;
   double answer = 1.0LF;
	bool invert_answer = false;

	// if x is negative, convert to positive and take inverse at end
   if(x < 0.0){
		x = -x;
		invert_answer = true;
	}

	// break i into integer andfractional parts
   n = int(x);
   f = x - double(n);

	// put f in the range 0-0.5 and adjust answer
	// subtract 0.5 from fractional exponent and
	// add 0.5 to integer exponent by multiplying answer by exp(0.5)
	if(f > 0.5){
		f -= 0.5;
		answer = M_EHALF;
	}

   for(i=0;i<8;i++){
		if(((n >> i) & 1) == 1)
			answer *= e_accum;
		e_accum *= e_accum;
	}
	
	answer *= exp_approx(f);

   if(invert_answer)
		answer = 1.0/answer;

	return answer;
}

double tan(double x) {
    return sin(x)/cos(x);
}

/* Approximation of f(x) = log(x)
 * on interval [ 0.5, 1.0 ]
 * with a polynomial of degree 7. */
double log_approx(double x)
{
    double u = 1.3419648079694775LF;
    u = u * x + -8.159823646011416LF;
    u = u * x + 2.1694837976736115e+1LF;
    u = u * x + -3.3104943376189169e+1LF;
    u = u * x + 3.2059105806949116e+1LF;
    u = u * x + -2.0778140811001331e+1LF;
    u = u * x + 9.8897820531599449LF;
    return u * x + -2.9427826194103015LF;
}

// ln_ieee754(double x)
// https://gist.github.com/dhermes/105da2a3c9861c90ea39
// Accuracy: the error is always less than 1 ulp
// modified for FragM by 3Dickulus @ FractalForums.org
double log(double x)  {

	x += 4.94065645841247E-308LF;
	
	double
		Ln2Hi = 6.93147180369123816490e-01LF, /* 3fe62e42 fee00000 */
		Ln2Lo = 1.90821492927058770002e-10LF, /* 3dea39ef 35793c76 */
		L0    = 7.0710678118654752440e-01LF,  /* 1/sqrt(2) */
		L1    = 6.666666666666735130e-01LF,   /* 3FE55555 55555593 */
		L2    = 3.999999999940941908e-01LF,   /* 3FD99999 9997FA04 */
		L3    = 2.857142874366239149e-01LF,   /* 3FD24924 94229359 */
		L4    = 2.222219843214978396e-01LF,   /* 3FCC71C5 1D8E78AF */
		L5    = 1.818357216161805012e-01LF,   /* 3FC74664 96CB03DE */
		L6    = 1.531383769920937332e-01LF,   /* 3FC39A09 D078C69F */
		L7    = 1.479819860511658591e-01LF;   /* 3FC2F112 DF3E5244 */

	// special cases
	if( isinf(x) )
        return double(1.0/0.0); /* return +inf */
	if( isnan(x) || x < 0 )
        return double(-0.0); /* nan */
	if( x == 0 )
        return double(-1.0/0.0); /* return -inf */

    // Argument Reduction
    int ki;
    double f1 = frexp(x, ki);
    
    if (f1 < L0) {
		f1 += f1;
		ki--;
	}
	
	double f = f1 - 1.0;
	double k = double(ki);

	// Approximation
	double s = f / (2.0 + f);
	double s2 = s * s;
	double s4 = s2 * s2;
    // Terms with odd powers of s^2.
	double t1 = s2 * (L1 + s4 * (L3 + s4 * (L5 + s4 * L7)));
    // Terms with even powers of s^2.
	double t2 = s4 * (L2 + s4 * (L4 + s4 * L6));
	double R = t1 + t2;
	double hfsq = 0.5 * f * f;
    
    return k*Ln2Hi - ((hfsq - (s*(hfsq+R) + k*Ln2Lo)) - f);

}

double log2(double N)
{
    return (log(N) / 0.69314718055995LF);
}

double log10(double N)
{
    return (log(N) / 2.30258509299405LF);
}

double log(double N, double B)
{
    return (log(N) / log(B));
}

dvec2 log( dvec2 n ) {
    return dvec2(log(n.x), log(n.y));
}

dvec3 log( dvec3 n ) {
    return dvec3(log(n.x), log(n.y), log(n.z));
}

dvec4 log( dvec4 n ) {
    return dvec4(log(n.x), log(n.y), log(n.z), log(n.w));
}

// double pow(double a, double b) {
// 
// // return exp(log(a) * b);
// 
// 	bool ltz = b<0;
// 	if(ltz) b = abs(b);
// 
// 	// put unpacked double bits into long int
// 	uvec2 unpacked = unpackDouble2x32(a);
// 
// 	double r = 1.0;
// 	int exp = int(b);
// 
// 	// use the IEEE 754 trick for the fraction of the exponent
// 	unpacked.y = int((b - exp) * (unpacked.y - 1072632447) + 1072632447);
// 	unpacked.x = 0;
// 
// 	// exponentiation by squaring
// 	while (exp != 0) {
// 		if ((exp & 1) != 0) r *= a;
// 		a *= a;
// 		exp >>= 1;
// 	}
// 
// 	r *= packDouble2x32(unpacked);
// 	return ltz ? 1.0/r : r;
// 
// }

// requires #extension GL_ARB_gpu_shader_int64 : enable
double pow(double a, double b) {
 
return exp(log(a) * b);
 
//         bool ltz = b<0;
// 	if(ltz) b = abs(b);
// 	
// 	// put unpacked double bits into long int
// 	uvec2 unpacked = unpackDouble2x32(a);
// 	int64_t tmp = int64_t(unpacked.y) << 32 + unpacked.x;
// 
// 	double r = 1.0;
// 	int ex = int(b);
// 	
// 	// use the IEEE 754 trick for the fraction of the exponent
// 	int64_t tmp2 = int64_t((b - ex) * (tmp - 4606921280493453312L)) + 4606921280493453312L;
// 	unpacked.y = uint(tmp2 >> 32);
// 	unpacked.x = uint(tmp2 - (int64_t(unpacked.y) << 32));
// 	
// 	// exponentiation by squaring
// 	while (ex != 0) {
// 		if ( (ex & 1) != 0) {
// 			r *= a;
// 		}
// 		a *= a;
// 		ex >>= 1;
// 	}
// 
// 	r *= packDouble2x32(unpacked);
// 	return ltz ? 1.0/r : r;

}

dvec2 pow(dvec2 A, dvec2 B)
{
    return dvec2( pow(A.x,B.x), pow(A.y,B.y) );
}

dvec3 pow(dvec3 A, dvec3 B)
{
    return dvec3( pow(A.x,B.x), pow(A.y,B.y), pow(A.z,B.z) );
}
/* Approximation of f(x) = atan(x)
 * on interval [ 0.0 , π/4 ]
 * with a polynomial of degree 10.
 */
double atan_approx(double x)
{
    double u = -2.9140257478972053e-3LF;
    u = u * x + 3.2005571699830107e-2LF;
    u = u * x + -1.627659300903442e-1LF;
    u = u * x + 5.0513223367120972e-1LF;
    u = u * x + -1.0595254685451083LF;
    u = u * x + 1.5689337521140527LF;
    u = u * x + -1.6640521237136246LF;
    u = u * x + 1.270853367426007LF;
    u = u * x + -7.356602708332424e-1LF;
    u = u * x + 4.0096549787833572e-1LF;
    u = u * x + -2.317084220916499e-1LF;
    u = u * x + 5.5673464120677798e-2LF;
    u = u * x + 6.1518997985636844e-2LF;
    u = u * x + 3.4871637890152628e-3LF;
    u = u * x + -9.1551371689992248e-2LF;
    u = u * x + 9.5405115942529782e-5LF;
    u = u * x + 1.1109982274527962e-1LF;
    u = u * x + 1.0462503881004859e-6LF;
    u = u * x + -1.4285721713962809e-1LF;
    u = u * x + 3.9206483284047854e-9LF;
    u = u * x + 1.9999999985236683e-1LF;
    u = u * x + 3.7405487051591751e-12LF;
    u = u * x + -3.3333333333339171e-1LF;
    u = u * x + 4.8455084038412012e-16LF;
    u = u * x + 1.0LF;
    u = u * x + 8.8145999826527008e-22LF;

    if(isnan(u) || isinf(u))
        return double(0.0);
    return u;
}


double atan(double y, double x){
 
    double sign_factor = 1.0;
    /*
      Account for signs.
    */
//     if ( x < 0.0 && y < 0.0 )
//     {
//         x = -x;
//         y = -y;
//     }

    if ( x < 0.0 )
    {
        x = -x;
        sign_factor = -1.0;
    }
    else
    if ( y < 0.0 )
    {
        y = -y;
        sign_factor = -1.0;
    }

    double ay = abs(y), ax = abs(x);
    bool inv = (ay > ax);
    
    double z;
    if(inv) z = ax/ay; else z = ay/ax; // [0,1]
    double th = atan_approx(z);        // [0,π/4]
    if(inv) th = M_PI2 - th;           // [0,π/2]
//     if(x < 0.0) th = M_PI - th;        // [0,π]
    if(y < 0.0) th = -th;              // [-π,π]
    return sign_factor * th;
}


#endif

#if defined(USE_DOUBLE)

#define REAL double
#define VEC2 dvec2
#define VEC3 dvec3
#define VEC4 dvec4

#else
float M_PI = 	3.14159265358979323846;
float M_2PI = M_PI*2.0;
float M_PI2 = M_PI/2.0;
float M_E =   2.71828182845904523536;
float M_EHALF = 1.6487212707001281469;

#define REAL float
#define VEC2 vec2
#define VEC3 vec3
#define VEC4 vec4
#endif

//----------------------------------------------------------------
// REAL functions not present in early GLSL versions

REAL cosh(REAL val)
{
	REAL tmp = exp(val);
	REAL cosH = (tmp + 1.0 / tmp) / 2.0;
	return cosH;
}

REAL tanh(REAL val)
{
	REAL tmp = exp(val);
	REAL tanH = (tmp - 1.0 / tmp) / (tmp + 1.0 / tmp);
	return tanH;
}

REAL sinh(REAL val)
{
	REAL tmp = exp(val);
	REAL sinH = (tmp - 1.0 / tmp) / 2.0;
	return sinH;
}

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
	return	VEC2(a.x,-a.y)/dot(a,a);
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
	if (b>0.0) b-=2.0*M_PI;
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
