#version 130

#camera 2D

#buffer RGBA32F

#vertex

out vec2 coord;
out vec2 viewCoord;

#group Camera

uniform float Zoom; slider[1e-4,1,1e16] NotLockable
uniform vec2 pixelSize;

void main(void)
{
  float ar = float(pixelSize.y / pixelSize.x);
  coord = ((gl_ProjectionMatrix * gl_Vertex).xy * vec2(ar, 1.0)) / Zoom;
  viewCoord = gl_Vertex.xy;
  gl_Position =  gl_Vertex;
}

#endvertex

in vec2 coord;
in vec2 viewCoord;

out vec4 fragColor;

uniform vec2 Center; slider[(-100,-100),(0,0),(100,100)] NotLockable

uniform sampler2D backbuffer;

vec3 color(vec2 p)
{
  int x = int(floor(16777216.0 * p.x)) & 0xFFFFFF;
  int y = int(floor(16777216.0 * p.y)) & 0xFFFFFF;
  int u = x ^ y;
  float g = float(u) / 16777216.0;
  return vec3(g);
}

void main()
{
  vec2 p = coord;
  vec4 next = vec4(color(p + vec2(Center)), 1.0);
  vec4 prev = texture(backbuffer, vec2(viewCoord + vec2(1.0)) * 0.5);
  if (! (next == next)) // NaN check
  {
    next = vec4(0.0);
  }
  fragColor = vec4(prev + vec4(next.xyz * next.w, next.w));
}

