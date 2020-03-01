#info Animation Playback Mode test example
#info activate animation playback with 10 second duration
#info toggle #if or make other changes and trigger rebuild
#info observe behaviour of time playback cursor
#info compare with animation playback mode checkbox in preferences toggled

#include "MathUtils.frag"
#include "DE-Raytracer.frag" 

uniform float time;

float t = 2.0 * 3.141592653 * time / 10.0;
float c = cos(t);
float s = sin(t);
mat2 transform = mat2(c, s, -s, c);

#if 1
float ballSize = 1.2;
#else
float ballSize = 2.0;
#endif

float DE(vec3 pos) {
	pos.xy *= transform;
	return abs(length(abs(pos)+vec3(-1.0))-ballSize);
}

#preset Default
FOV = 0.4
Eye = 5.582327,4.881191,-6.709066
Target = 0,0,0
Up = -0.1207781,0.8478234,0.5163409
#endpreset
