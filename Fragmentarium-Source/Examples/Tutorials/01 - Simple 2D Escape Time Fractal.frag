#include "2DJulia.frag"
#include "Complex.frag"

// Escape time fractals iterate a functions for each point
// in the plane, and check if the sequence generated converges.
// 
// The "2DJulia.frag" helper file makes it easy to generate
// these kind of systems.
//
// Just implement the 'formula' function below.
// It is possible to draw Mandelbrots and Julias
// and customize the coloring.
//
// Here is an example of a Mandelbrot:

vec2 formula(vec2 z, vec2 offset) {
	z = cMul(z,z)+offset;
	return z;
}

#preset Default
Center = -0.399375,0.012916666
Zoom = 0.756143667
AntiAliasScale = 1
EnableTransform = true
RotateAngle = 0
StretchAngle = 0
StretchAmount = 0
AntiAlias = 1
Iterations = 200
PreIterations = 1
R = 0
G = 0.4
B = 0.7
C = 1
Julia = false
JuliaX = 0.23528
JuliaY = 5.5384
ShowMap = true
MapZoom = 2.1
EscapeSize = 5
ColoringType = 0
ColorFactor = 0.5
TrigIter = 5
TrigLimit = 1.10000000000000009
#endpreset
