#info Icosahedral polyhedra Distance Estimator (knighty 2011)
#define providesInit
#include "MathUtils.frag"
#include "DE-Raytracer.frag"
#group Icosahedral

//Cut and fold (Houdini?) technique with icosahedral folding
//Well... in 2d, it's proved that any (simple?) polygon can be obtained with Cut and fold
//Seems it's the same for non auto intersecting polyhedra. Right?

//Size of the polyhedra
uniform float Size; slider[-2.0,.5,2.00]

//normal of the cutting plane
uniform vec3 plnormal; slider[(-1,-1,-1),(1,0,0),(1,1,1)]

#define Phi (.5*(1.+sqrt(5.)))

vec3 n1 = normalize(vec3(-Phi,Phi-1.0,1.0));
vec3 n2 = normalize(vec3(1.0,-Phi,Phi+1.0));
vec3 n3 = normalize(vec3(0.0,0.0,-1.0));

void init() {
}

float DE(vec3 z)
{
	float t;
	// Folds.
	//Dodecahedral
	z = abs(z);
	if (z.y>z.x) z.xy =z.yx;
	if (z.z>z.x) z.xz = z.zx;
	if (z.y>z.x) z.xy =z.yx;

	//Distance to the plane going through vec3(Size,0.,0.) and which normal is plnormal
	return abs(dot(z-vec3(Size,0.,0.),normalize(plnormal)));
}

#preset Default
FOV = 0.4
Eye = -2.502036,-2.180617,-3.467663
Target = 2.71054,2.362336,3.756637
Up = 0,0.8465328,-0.5323365
EquiRectangular = false
AutoFocus = false
FocalPlane = 1
Aperture = 0
Gamma = 2
ToneMapping = 4
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -2.3
DetailAO = -0.5
FudgeFactor = 1
MaxDistance = 1000
MaxRaySteps = 56
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 0.4
SpecularExp = 16
SpecularMax = 10
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
Size = 0.9565217
plnormal = 0.03125,0.03125,0.25
#endpreset
