#donotrun
/*
refactored by Claude 2019-10-29 as per fragm github ticket #67
*/

#include "Math.frag"

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
