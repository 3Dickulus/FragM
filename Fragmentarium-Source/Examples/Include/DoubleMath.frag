#donotrun
/*

for GLSL >= 400, double precision functions:
  sqrt, length, distance, normalize,
  sin, cos, tan, exp, pow, log, log2 log10, atan

functions not present in early GLSL versions
  isnan, sign, inversesqrt, sinh, cosh, tanh, modf, smoothstep

for GLSL >= 400, double precision functions
  sinh, cosh, tanh

functions not present in GLSL
  log10

*/

//--------------------------------------------------------------------
// 2019-08-13 {{{ improved sqrt by claude

// save built in square root
float _builtin_sqrt(float x) { return sqrt(x); }
vec2 _builtin_sqrt(vec2 x) { return sqrt(x); }
vec3 _builtin_sqrt(vec3 x) { return sqrt(x); }
vec4 _builtin_sqrt(vec4 x) { return sqrt(x); }
#if __VERSION__ >= 400
double _builtin_sqrt(double x) { return sqrt(x); }
dvec2 _builtin_sqrt(dvec2 x) { return sqrt(x); }
dvec3 _builtin_sqrt(dvec3 x) { return sqrt(x); }
dvec4 _builtin_sqrt(dvec4 x) { return sqrt(x); }
#endif
// redefine new square root for overloading
float sqrt(float x) { return _builtin_sqrt(x); }
vec2 sqrt(vec2 x) { return _builtin_sqrt(x); }
vec3 sqrt(vec3 x) { return _builtin_sqrt(x); }
vec4 sqrt(vec4 x) { return _builtin_sqrt(x); }

// implement improved square root
#if __VERSION >= 400
double sqrt(double x); // defined below
dvec2 sqrt(dvec2 x) { return dvec2(sqrt(x.x), sqrt(x.y)); }
dvec3 sqrt(dvec3 x) { return dvec3(sqrt(x.x), sqrt(x.y), sqrt(x.z)); }
dvec4 sqrt(dvec4 x) { return dvec4(sqrt(x.x), sqrt(x.y), sqrt(x.z), sqrt(x.w)); }

// Hardware sqrt improved by the Babylonian algorithm (Newton Raphson)
double sqrt(double m)
{
  if (! (m > 0.0LF)) { return 0.0LF; } // FIXME return NaN for < 0?
  double z = _builtin_sqrt(m);
  z = (z + m / z) / 2.0LF;
  z = (z + m / z) / 2.0LF;
  z = (z + m / z) / 2.0LF;
  z = (z + m / z) / 2.0LF;
  return z;
}
#endif

// save built in length
float _builtin_length(float x) { return length(x); }
float _builtin_length(vec2 x) { return length(x); }
float _builtin_length(vec3 x) { return length(x); }
float _builtin_length(vec4 x) { return length(x); }
#if __VERSION__ >= 400
double _builtin_length(double x) { return length(x); }
double _builtin_length(dvec2 x) { return length(x); }
double _builtin_length(dvec3 x) { return length(x); }
double _builtin_length(dvec4 x) { return length(x); }
#endif

// redefine new length for overloading
float length(float x) { return _builtin_length(x); }
float length(vec2 x) { return _builtin_length(x); }
float length(vec3 x) { return _builtin_length(x); }
float length(vec4 x) { return _builtin_length(x); }

// implement improved length
#if __VERSION__ >= 400
double length(double x) { return abs(x); }
double length(dvec2 x) { return sqrt(dot(x, x)); } // FIXME overflow / underflow
double length(dvec3 x) { return sqrt(dot(x, x)); } // FIXME overflow / underflow
double length(dvec4 x) { return sqrt(dot(x, x)); } // FIXME overflow / underflow
#endif

// save built in distance
float _builtin_distance(float x, float y) { return distance(x, y); }
float _builtin_distance(vec2 x, vec2 y) { return distance(x, y); }
float _builtin_distance(vec3 x, vec3 y) { return distance(x, y); }
float _builtin_distance(vec4 x, vec4 y) { return distance(x, y); }
#if __VERSION__ >= 400
double _builtin_distance(double x, double y) { return distance(x, y); }
double _builtin_distance(dvec2 x, dvec2 y) { return distance(x, y); }
double _builtin_distance(dvec3 x, dvec3 y) { return distance(x, y); }
double _builtin_distance(dvec4 x, dvec4 y) { return distance(x, y); }
#endif

// redefine new distance for overloading
float distance(float x, float y) { return _builtin_distance(x, y); }
float distance(vec2 x, vec2 y) { return _builtin_distance(x, y); }
float distance(vec3 x, vec3 y) { return _builtin_distance(x, y); }
float distance(vec4 x, vec4 y) { return _builtin_distance(x, y); }

// implement improved length
#if __VERSION__ >= 400
double distance(double x, double y) { return length(x - y); }
double distance(dvec2 x, dvec2 y) { return length(x - y); }
double distance(dvec3 x, dvec3 y) { return length(x - y); }
double distance(dvec4 x, dvec4 y) { return length(x - y); }
#endif

// save built in normalize
float _builtin_normalize(float x) { return normalize(x); }
vec2 _builtin_normalize(vec2 x) { return normalize(x); }
vec3 _builtin_normalize(vec3 x) { return normalize(x); }
vec4 _builtin_normalize(vec4 x) { return normalize(x); }
#if __VERSION__ >= 400
double _builtin_normalize(double x) { return normalize(x); }
dvec2 _builtin_normalize(dvec2 x) { return normalize(x); }
dvec3 _builtin_normalize(dvec3 x) { return normalize(x); }
dvec4 _builtin_normalize(dvec4 x) { return normalize(x); }
#endif

// redefine new normalize for overloading
float normalize(float x) { return _builtin_normalize(x); }
vec2 normalize(vec2 x) { return _builtin_normalize(x); }
vec3 normalize(vec3 x) { return _builtin_normalize(x); }
vec4 normalize(vec4 x) { return _builtin_normalize(x); }

// implement improved normalize
#if __VERSION__ >= 400
double normalize(double x) { double l = length(x); return l > 0.0LF ? x / l : 0.0LF; }
dvec2 normalize(dvec2 x) { double l = length(x); return l > 0.0LF ? x / l : dvec2(0.0LF); }
dvec3 normalize(dvec3 x) { double l = length(x); return l > 0.0LF ? x / l : dvec3(0.0LF); }
dvec4 normalize(dvec4 x) { double l = length(x); return l > 0.0LF ? x / l : dvec4(0.0LF); }
#endif

// 2018-08-13 }}} improved sqrt by claude
//--------------------------------------------------------------------


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
#if __VERSION__ >= 400
const uint TrigIterMax = 20;
#group Trig
uniform int TrigIter;slider[0,5,20]
uniform double TrigLimit;slider[0.001,1.1,1.5]
#endif

// sine

float _builtin_sin(float x) { return sin(x); }
vec2 _builtin_sin(vec2 x) { return sin(x); }
vec3 _builtin_sin(vec3 x) { return sin(x); }
vec4 _builtin_sin(vec4 x) { return sin(x); }
float sin(float x) { return _builtin_sin(x); }
vec2 sin(vec2 x) { return _builtin_sin(x); }
vec3 sin(vec3 x) { return _builtin_sin(x); }
vec4 sin(vec4 x) { return _builtin_sin(x); }

#if __VERSION__ >= 400
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
#endif

// cosine

float _builtin_cos(float x) { return cos(x); }
vec2 _builtin_cos(vec2 x) { return cos(x); }
vec3 _builtin_cos(vec3 x) { return cos(x); }
vec4 _builtin_cos(vec4 x) { return cos(x); }
float cos(float x) { return _builtin_cos(x); }
vec2 cos(vec2 x) { return _builtin_cos(x); }
vec3 cos(vec3 x) { return _builtin_cos(x); }
vec4 cos(vec4 x) { return _builtin_cos(x); }

#if __VERSION__ >= 400
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
#endif

float _builtin_exp(float x) { return exp(x); }
vec2 _builtin_exp(vec2 x) { return exp(x); }
vec3 _builtin_exp(vec3 x) { return exp(x); }
vec4 _builtin_exp(vec4 x) { return exp(x); }
float exp(float x) { return _builtin_exp(x); }
vec2 exp(vec2 x) { return _builtin_exp(x); }
vec3 exp(vec3 x) { return _builtin_exp(x); }
vec4 exp(vec4 x) { return _builtin_exp(x); }

#if __VERSION__ >= 400
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
#endif

float _builtin_tan(float x) { return tan(x); }
vec2 _builtin_tan(vec2 x) { return tan(x); }
vec3 _builtin_tan(vec3 x) { return tan(x); }
vec4 _builtin_tan(vec4 x) { return tan(x); }
float tan(float x) { return _builtin_tan(x); }
vec2 tan(vec2 x) { return _builtin_tan(x); }
vec3 tan(vec3 x) { return _builtin_tan(x); }
vec4 tan(vec4 x) { return _builtin_tan(x); }

#if __VERSION__ >= 400
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
#endif

// ln_ieee754(double x)

float _builtin_log(float x) { return log(x); }
vec2 _builtin_log(vec2 x) { return log(x); }
vec3 _builtin_log(vec3 x) { return log(x); }
vec4 _builtin_log(vec4 x) { return log(x); }
float log(float x) { return _builtin_log(x); }
vec2 log(vec2 x) { return _builtin_log(x); }
vec3 log(vec3 x) { return _builtin_log(x); }
vec4 log(vec4 x) { return _builtin_log(x); }

#if __VERSION__ >= 400
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
#endif

float _builtin_log2(float x) { return log2(x); }
vec2 _builtin_log2(vec2 x) { return log2(x); }
vec3 _builtin_log2(vec3 x) { return log2(x); }
vec4 _builtin_log2(vec4 x) { return log2(x); }
float log2(float x) { return _builtin_log2(x); }
vec2 log2(vec2 x) { return _builtin_log2(x); }
vec3 log2(vec3 x) { return _builtin_log2(x); }
vec4 log2(vec4 x) { return _builtin_log2(x); }

#if __VERSION__ >= 400
double log2(double N)
{
    return (log(N) / 0.69314718055995LF);
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
#endif

// double pow(double a, double b) {
//
// // return exp(log(a) * b);
//
//   bool ltz = b<0;
//   if(ltz) b = abs(b);
//
//   // put unpacked double bits into long int
//   uvec2 unpacked = unpackDouble2x32(a);
//
//   double r = 1.0;
//   int exp = int(b);
//
//   // use the IEEE 754 trick for the fraction of the exponent
//   unpacked.y = int((b - exp) * (unpacked.y - 1072632447) + 1072632447);
//   unpacked.x = 0;
//
//   // exponentiation by squaring
//   while (exp != 0) {
//     if ((exp & 1) != 0) r *= a;
//     a *= a;
//     exp >>= 1;
//   }
//
//   r *= packDouble2x32(unpacked);
//   return ltz ? 1.0/r : r;
//
// }

// requires #extension GL_ARB_gpu_shader_int64 : enable

float _builtin_pow(float x, float y) { return pow(x, y); }
vec2 _builtin_pow(vec2 x, vec2 y) { return pow(x, y); }
vec3 _builtin_pow(vec3 x, vec3 y) { return pow(x, y); }
vec4 _builtin_pow(vec4 x, vec4 y) { return pow(x, y); }
float pow(float x, float y) { return _builtin_pow(x, y); }
vec2 pow(vec2 x, vec2 y) { return _builtin_pow(x, y); }
vec3 pow(vec3 x, vec3 y) { return _builtin_pow(x, y); }
vec4 pow(vec4 x, vec4 y) { return _builtin_pow(x, y); }

#if __VERSION__ >= 400
double pow(double a, double b) {

return exp(log(a) * b);

//         bool ltz = b<0;
//   if(ltz) b = abs(b);
//
//   // put unpacked double bits into long int
//   uvec2 unpacked = unpackDouble2x32(a);
//   int64_t tmp = int64_t(unpacked.y) << 32 + unpacked.x;
//
//   double r = 1.0;
//   int ex = int(b);
//
//   // use the IEEE 754 trick for the fraction of the exponent
//   int64_t tmp2 = int64_t((b - ex) * (tmp - 4606921280493453312L)) + 4606921280493453312L;
//   unpacked.y = uint(tmp2 >> 32);
//   unpacked.x = uint(tmp2 - (int64_t(unpacked.y) << 32));
//
//   // exponentiation by squaring
//   while (ex != 0) {
//     if ( (ex & 1) != 0) {
//       r *= a;
//     }
//     a *= a;
//     ex >>= 1;
//   }
//
//   r *= packDouble2x32(unpacked);
//   return ltz ? 1.0/r : r;

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
#endif

float _builtin_atan(float yx) { return atan(yx); }
vec2 _builtin_atan(vec2 yx) { return atan(yx); }
vec3 _builtin_atan(vec3 yx) { return atan(yx); }
vec4 _builtin_atan(vec4 yx) { return atan(yx); }
float _builtin_atan(float y, float x) { return atan(y, x); }
vec2 _builtin_atan(vec2 y, vec2 x) { return atan(y, x); }
vec3 _builtin_atan(vec3 y, vec3 x) { return atan(y, x); }
vec4 _builtin_atan(vec4 y, vec4 x) { return atan(y, x); }
float atan(float yx) { return _builtin_atan(yx); }
vec2 atan(vec2 yx) { return _builtin_atan(yx); }
vec3 atan(vec3 yx) { return _builtin_atan(yx); }
vec4 atan(vec4 yx) { return _builtin_atan(yx); }
float atan(float y, float x) { return _builtin_atan(y, x); }
vec2 atan(vec2 y, vec2 x) { return _builtin_atan(y, x); }
vec3 atan(vec3 y, vec3 x) { return _builtin_atan(y, x); }
vec4 atan(vec4 y, vec4 x) { return _builtin_atan(y, x); }

#if __VERSION__ >= 400
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


//----------------------------------------------------------------
// functions not present in early GLSL versions

// isnan, sign, inversesqrt, sinh, cosh, tanh, modf, smoothstep

#if __VERSION__ < 130

float isnan(float val)
{
  return ! (val == val);
}

bvec2 isnan(vec2 val) { return bvec2(isnan(val.x), isnan(val.y)); }
bvec3 isnan(vec3 val) { return bvec3(isnan(val.x), isnan(val.y), isnan(val.z)); }
bvec4 isnan(vec4 val) { return bvec4(isnan(val.x), isnan(val.y), isnan(val.z), isnan(val.w)); }

float sign(float val)
{
  if (val <  0.0) return -1.0;
  if (val == 0.0) return  0.0;
  if (val >= 0.0) return  1.0;
  return val; // NaN
}

vec2 sign(vec2 val) { return vec2(sign(val.x), sign(val.y)); }
vec3 sign(vec3 val) { return vec3(sign(val.x), sign(val.y), sign(val.z)); }
vec4 sign(vec4 val) { return vec4(sign(val.x), sign(val.y), sign(val.z), sign(val.w)); }

float inversesqrt(float val)
{
  return 1.0 / sqrt(val);
}

vec2 inversesqrt(vec2 val) { return vec2(inversesqrt(val.x), inversesqrt(val.y)); }
vec3 inversesqrt(vec3 val) { return vec3(inversesqrt(val.x), inversesqrt(val.y), inversesqrt(val.z)); }
vec4 inversesqrt(vec4 val) { return vec4(inversesqrt(val.x), inversesqrt(val.y), inversesqrt(val.z), inversesqrt(val.w)); }

float sinh(float val) {
  float tmp = exp(val); // FIXME expm1 may be better near 0
  return (tmp - 1.0 / tmp) / 2.0;
}

vec2 sinh(vec2 val) { return vec2(sinh(val.x), sinh(val.y)); }
vec3 sinh(vec3 val) { return vec3(sinh(val.x), sinh(val.y), sinh(val.z)); }
vec4 sinh(vec4 val) { return vec4(sinh(val.x), sinh(val.y), sinh(val.z), sinh(val.w)); }

float cosh(float val) {
  float tmp = exp(val);
  return (tmp + 1.0 / tmp) / 2.0;
}

vec2 cosh(vec2 val) { return vec2(cosh(val.x), cosh(val.y)); }
vec3 cosh(vec3 val) { return vec3(cosh(val.x), cosh(val.y), cosh(val.z)); }
vec4 cosh(vec4 val) { return vec4(cosh(val.x), cosh(val.y), cosh(val.z), cosh(val.w)); }

float tanh(float val) {
  float tmp = exp(val);
  return (tmp - 1.0 / tmp) / (tmp + 1.0 / tmp);
}

vec2 tanh(vec2 val) { return vec2(tanh(val.x), tanh(val.y)); }
vec3 tanh(vec3 val) { return vec3(tanh(val.x), tanh(val.y), tanh(val.z)); }
vec4 tanh(vec4 val) { return vec4(tanh(val.x), tanh(val.y), tanh(val.z), tanh(val.w)); }

float modf(float x, out float i)
{
  i = floor(x);
  return x - i;
}

vec2 modf(vec2 x, out vec2 i)
{
  i = floor(x);
  return x - i;
}

vec3 modf(vec3 x, out vec3 i)
{
  i = floor(x);
  return x - i;
}

vec4 modf(vec4 x, out vec4 i)
{
  i = floor(x);
  return x - i;
}

float smoothstep(float edge0, float edge1, float x)
{
  float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  return t * t * (3.0 - 2.0 * t);
}

vec2 smoothstep(vec2 edge0, vec2 edge1, vec2 x)
{
  vec2 t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  return t * t * (3.0 - 2.0 * t);
}

vec2 smoothstep(float edge0, float edge1, vec2 x)
{
  return smoothstep(vec2(edge0), vec2(edge1), x);
}

vec3 smoothstep(vec3 edge0, vec3 edge1, vec3 x)
{
  vec3 t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  return t * t * (3.0 - 2.0 * t);
}

vec3 smoothstep(float edge0, float edge1, vec3 x)
{
  return smoothstep(vec3(edge0), vec3(edge1), x);
}

vec4 smoothstep(vec4 edge0, vec4 edge1, vec4 x)
{
  vec4 t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  return t * t * (3.0 - 2.0 * t);
}

vec4 smoothstep(float edge0, float edge1, vec4 x)
{
  return smoothstep(vec4(edge0), vec4(edge1), x);
}

#else

// capture builtins for overloading

float _builtin_sinh(float x) { return sinh(x); }
vec2 _builtin_sinh(vec2 x) { return sinh(x); }
vec3 _builtin_sinh(vec3 x) { return sinh(x); }
vec4 _builtin_sinh(vec4 x) { return sinh(x); }
float sinh(float x) { return _builtin_sinh(x); }
vec2 sinh(vec2 x) { return _builtin_sinh(x); }
vec3 sinh(vec3 x) { return _builtin_sinh(x); }
vec4 sinh(vec4 x) { return _builtin_sinh(x); }

float _builtin_cosh(float x) { return cosh(x); }
vec2 _builtin_cosh(vec2 x) { return cosh(x); }
vec3 _builtin_cosh(vec3 x) { return cosh(x); }
vec4 _builtin_cosh(vec4 x) { return cosh(x); }
float cosh(float x) { return _builtin_cosh(x); }
vec2 cosh(vec2 x) { return _builtin_cosh(x); }
vec3 cosh(vec3 x) { return _builtin_cosh(x); }
vec4 cosh(vec4 x) { return _builtin_cosh(x); }

float _builtin_tanh(float x) { return tanh(x); }
vec2 _builtin_tanh(vec2 x) { return tanh(x); }
vec3 _builtin_tanh(vec3 x) { return tanh(x); }
vec4 _builtin_tanh(vec4 x) { return tanh(x); }
float tanh(float x) { return _builtin_tanh(x); }
vec2 tanh(vec2 x) { return _builtin_tanh(x); }
vec3 tanh(vec3 x) { return _builtin_tanh(x); }
vec4 tanh(vec4 x) { return _builtin_tanh(x); }

// double versions of sinh, cosh, tanh

#if __VERSION__ >= 400

double sinh(double val) {
  double tmp = exp(val); // FIXME expm1 may be better near 0
  return (tmp - 1.0LF / tmp) / 2.0LF;
}

dvec2 sinh(dvec2 val) { return dvec2(sinh(val.x), sinh(val.y)); }
dvec3 sinh(dvec3 val) { return dvec3(sinh(val.x), sinh(val.y), sinh(val.z)); }
dvec4 sinh(dvec4 val) { return dvec4(sinh(val.x), sinh(val.y), sinh(val.z), sinh(val.w)); }

double cosh(double val) {
  double tmp = exp(val);
  return (tmp + 1.0LF / tmp) / 2.0LF;
}

dvec2 cosh(dvec2 val) { return dvec2(cosh(val.x), cosh(val.y)); }
dvec3 cosh(dvec3 val) { return dvec3(cosh(val.x), cosh(val.y), cosh(val.z)); }
dvec4 cosh(dvec4 val) { return dvec4(cosh(val.x), cosh(val.y), cosh(val.z), cosh(val.w)); }

double tanh(double val) {
  double tmp = exp(val); // FIXME expm1 may be better near 0
  return (tmp - 1.0LF / tmp) / (tmp + 1.0LF / tmp);
}

dvec2 tanh(dvec2 val) { return dvec2(tanh(val.x), tanh(val.y)); }
dvec3 tanh(dvec3 val) { return dvec3(tanh(val.x), tanh(val.y), tanh(val.z)); }
dvec4 tanh(dvec4 val) { return dvec4(tanh(val.x), tanh(val.y), tanh(val.z), tanh(val.w)); }

#endif

#endif

// extra functions not in GLSL
// log10

float log10(float x) { return log(x) / log(float(10.0)); }
vec2 log10(vec2 x) { return log(x) / log(float(10.0)); }
vec3 log10(vec3 x) { return log(x) / log(float(10.0)); }
vec4 log10(vec4 x) { return log(x) / log(float(10.0)); }
#if __VERSION__ >= 400
double log10(double x) { return log(x) / log(double(10.0)); }
dvec2 log10(dvec2 x) { return log(x) / log(double(10.0)); }
dvec3 log10(dvec3 x) { return log(x) / log(double(10.0)); }
dvec4 log10(dvec4 x) { return log(x) / log(double(10.0)); }
#endif
