#donotrun

#vertex

varying vec2 coord;

void main(void)
{
  gl_Position =  gl_Vertex;
  coord = ((gl_ProjectionMatrix * gl_Vertex).xy + vec2(1.0)) * 0.5;
}

#endvertex

varying vec2 coord;

uniform sampler2D frontbuffer;

#group Post

uniform float Exposure; slider[-10.0,0.0,10.0]

void main(void)
{
  vec4 tex = texture2D(frontbuffer, coord);
  vec3 c = pow(2.0, Exposure) * tex.xyz / tex.a;
  gl_FragColor = vec4(clamp(c, 0.0, 1.0), 1.0);
}
