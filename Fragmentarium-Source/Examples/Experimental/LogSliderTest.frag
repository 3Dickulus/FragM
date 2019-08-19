#include "MathUtils.frag"
#include "Progressive2D.frag"

uniform float Good; slider[1e-3,1,1e3] Logarithmic
uniform float Bad; slider[-1,0,1] Logarithmic
uniform float Ugly; slider[-1e3,-1,-1e-3] Logarithmic

vec3 color(vec2 p)
{
  return vec3(Good / 1000.0, Bad / 2.0 + 0.5, Ugly / -1000.0);
}
