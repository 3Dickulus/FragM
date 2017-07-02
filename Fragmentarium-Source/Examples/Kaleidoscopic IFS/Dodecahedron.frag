#info Dodecahedron Distance Estimator (Syntopia 2010)
#define providesInit
#include "DE-Raytracer.frag"
#include "MathUtils.frag"
#group Dodecahedron

// Based on Knighty's Kaleidoscopic IFS 3D Fractals, described here:
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/

uniform float Scale; slider[0.0,2.617924,4.00]

uniform float Phi; slider[-5,1.618,5] Locked

uniform float Bailout; slider[4,9,12] Locked
float bailout2 = pow(10.0,Bailout);

vec3 n1 = normalize(vec3(-1.0,Phi-1.0,1.0/(Phi-1.0)));
vec3 n2 = normalize(vec3(Phi-1.0,1.0/(Phi-1.0),-1.0));
vec3 n3 = normalize(vec3(1.0/(Phi-1.0),-1.0,Phi-1.0));

vec3 offset = vec3(1.0,1.0,1.0);

uniform float Angle1; slider[-180,0,180]
uniform vec3 Rot1; slider[(-1,-1,-1),(1,1,1),(1,1,1)]
uniform float Angle2; slider[-180,0,180]
uniform vec3 Rot2; slider[(-1,-1,-1),(1,1,1),(1,1,1)]

mat3 fracRotation2;
mat3 fracRotation1;

void init() {
	fracRotation2 = rotationMatrix3(normalize(Rot2), Angle2);
	fracRotation1 = rotationMatrix3(normalize(Rot1), Angle1);
}

// Number of fractal iterations.
uniform int Iterations;  slider[0,13,100]

float DE(vec3 z)
{
	float r;
	
	// Prefolds.
	float t;
	// Iterate to compute the distance estimator.
	int n = 0;
	while (n < Iterations) {
		z *= fracRotation1;
	
		// t =dot(z,n1); if (t<0.0) { z-=2.0*t*n1; } <- this form is a bit slower	
		z-=2.0 * min(0.0, dot(z, n1)) * n1;
		z-=	2.0 * min(0.0, dot(z, n2)) * n2;
		z-=	2.0 * min(0.0, dot(z, n3)) * n3;
		z-=2.0 * min(0.0, dot(z, n1)) * n1;
		z-=	2.0 * min(0.0, dot(z, n2)) * n2;
		z-=	2.0 * min(0.0, dot(z, n3)) * n3;
		z-=2.0 * min(0.0, dot(z, n1)) * n1;
		z-=	2.0 * min(0.0, dot(z, n2)) * n2;
		z-=	2.0 * min(0.0, dot(z, n3)) * n3;

		z = z*Scale - offset*(Scale-1.0);
		z *= fracRotation2;
		r = dot(z, z);
		orbitTrap = min(orbitTrap, abs(vec4(0.0,0.0,0.0,r)));
		if (r > bailout2) break;
		n++;
	}
	
	// Works better when subtracting -1
	return (length(z) ) * pow(Scale,  float(-n-1));
}

#preset Default
FOV = 0.360217
Eye = 1.81932,-3.83045,-3.01399
Target = 0.435564,-0.91705,-0.721581
Up = 0.0282583,0.619615,-0.784397
AntiAlias = 1
Detail = -2.53981
DetailAO = -1.33
FudgeFactor = 1
MaxRaySteps = 56
BoundingSphere = 5.66
Dither = 0.56521
AO = 0,0,0,0.77869
Specular = 0.8333
SpecularExp = 16
SpotLight = 1,1,1,0.32609
SpotLightDir = -0.39048,0.1
CamLight = 1,1,1,1.26882
CamLightMin = 0.15151
Glow = 1,1,1,0.06947
Fog = 0.08054
HardShadow = 0
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.49505
X = 0.5,0.6,0.6,-0.9843
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.79528
R = 0.4,0.7,1,0.14286
BackgroundColor = 0.666667,0.333333,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Scale = 2.61792
Phi = 1.618
Bailout = 9
Angle1 = 0
Rot1 = 1,1,1
Angle2 = 0
Rot2 = 1,1,1
Iterations = 13
#endpreset
