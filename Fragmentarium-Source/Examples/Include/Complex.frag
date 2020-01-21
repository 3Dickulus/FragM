#donotrun
/*
refactored by Claude 2019-10-29 as per fragm github ticket #67
*/

#if __VERSION__ >= 400

double M_PI = 	3.14159265358979323846LF;
double M_2PI = M_PI*2.0LF;
double M_PI2 = M_PI/2.0LF;
double M_E =   2.71828182845904523536LF;
double M_EHALF = 1.6487212707001281469LF;

#else

float M_PI = 	3.14159265358979323846;
float M_2PI = M_PI*2.0;
float M_PI2 = M_PI/2.0;
float M_E =   2.71828182845904523536;
float M_EHALF = 1.6487212707001281469;

#endif

#include "DoubleMath.frag"

#define REAL float
#define VEC2 vec2
#define VEC3 vec3
#define VEC4 vec4

#include "ComplexBase.frag"

#if __VERSION__ >= 400

#undef REAL
#undef VEC2
#undef VEC3
#undef VEC4

#define REAL double
#define VEC2 dvec2
#define VEC3 dvec3
#define VEC4 dvec4

#include "ComplexBase.frag"

#endif
