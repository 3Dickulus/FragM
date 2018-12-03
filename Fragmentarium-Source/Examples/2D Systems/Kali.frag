#group Kaliset
#include "MathUtils.frag"
#include "Progressive2DJulia.frag"
#info Kaliset

//  Fractal by Kali (Kaliset? Kaliduck?)
// (Implementation by Syntopia)
// http://www.fractalforums.com/new-theories-and-research/very-simple-formula-for-fractal-patterns/msg31800/#msg31800

uniform float MinRadius; slider[0,0,10]
uniform float Scaling; slider[-5,0,5]

vec2 formula(vec2 z,vec2 c) {

	float m =dot(z,z);

	if (m<MinRadius) {
		z = abs(z)/(MinRadius*MinRadius);
	}else {
		z = abs(z)/m*Scaling;
	}
	return z+c;
}

#preset Default
Gamma = 2.08335
Brightness = 1
Contrast = 1
Saturation = 1
Center = -0.0042381,-0.0028523
Zoom = 0.163192
ToneMapping = 3
Exposure = 0.6522
AARange = 2
AAExp = 1
GaussianAA = true
Iterations = 40
PreIterations = 1
R = 0
G = 0.4
B = 0.7
C = 1
Julia = true
JuliaX = 0.5663564
JuliaY = 0.0732411
ShowMap = false
MapZoom = 2.1
EscapeSize = 5
ColoringType = 0
ColorFactor = 0.5
MinRadius = 0
Scaling = -1.9231
#endpreset

#preset P1
Center = -0.245621,-0.0387084
Zoom = 0.209867
Iterations = 46
PreIterations = 0
R = 0.08696
G = 0.4
B = 0.7
C = 0.57972
Julia = true
JuliaX = 0.13193
JuliaY = 1.18487
ShowMap = false
MapZoom = 2.1
EscapeSize = 5.62793
ColoringType = 0
ColorFactor = 1
MinRadius = 0
Scaling = -1.9231
#endpreset
