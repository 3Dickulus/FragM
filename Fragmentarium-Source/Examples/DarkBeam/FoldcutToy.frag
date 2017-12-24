#info Foldcut toy (DarkBeam 2015/knighty 2011)
#define providesInit
#include "MathUtils.frag"
#include "DE-Raytracer.frag"
#group Icosahedral

uniform float Scale; slider[0.00,2,4.00]
uniform int Iterations;  slider[0,2,10]

uniform vec3 Offset; slider[(-1,-1,-1),(1,1,1),(1,1,1)]
uniform vec3 Offset2; slider[(-1,-1,-1),(1,0,0),(1,1,1)]

uniform float Angle1; slider[-180,0,180]
uniform vec3 Rot1; slider[(-1,-1,-1),(1,1,1),(1,1,1)]
//uniform float Angle2; slider[-180,0,180]
//uniform vec3 Rot2; slider[(-1,-1,-1),(1,1,1),(1,1,1)]
mat3 fracRotation2;
mat3 fracRotation1;

void init() {
	//fracRotation2 = rotationMatrix3(normalize(Rot2), Angle2);
	fracRotation1 = Scale* rotationMatrix3(normalize(Rot1), Angle1);
}

float DE(vec3 z)
{
	float t; int n = 0;
      float scalep = 1;
      float DE1 = 1000;
      vec3 z0=z;
	// Folds.
	//Dodecahedral
	while (n < Iterations) {
	z *= fracRotation1;
	z = abs(z);
       z -= Offset;
	if (z.y>z.x) z.xy =z.yx;
	if (z.z>z.x) z.xz = z.zx;
	if (z.y>z.x) z.xy =z.yx;
       z -= Offset2;
	if (z.y>z.x) z.xy =z.yx;
	if (z.z>z.x) z.xz = z.zx;
	if (z.y>z.x) z.xy =z.yx;

	n++;  scalep *= Scale;
       DE1 = min(DE1,z.x/scalep);
      }
	//Distance to the plane going through vec3(Size,0.,0.) and which normal is plnormal
	return DE1;
}

#preset default
FOV = 0.62536
Eye = 3.916786,-3.025636,0.5673097
Target = -2.984674,2.462124,-0.3175513
Up = 0.401286,0.369883,-0.83588
EquiRectangular = false
AutoFocus = false
FocalPlane = 1
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -2.880143
DetailAO = -1.35716
FudgeFactor = 1
MaxDistance = 1000
MaxRaySteps = 164
Dither = 0.51754
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.63626,0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
GlowMax = 52
Fog = 0
HardShadow = 0.35385
ShadowSoft = 12.5806
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.14286
X = 1,1,1,1
Y = 0.345098,0.666667,0,0.02912
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,-0.0194
BackgroundColor = 0.607843,0.866667,0.560784
GradientBackground = 0.3261
CycleColors = false
Cycles = 4.04901
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Scale = 1.5
Offset = 0,1,1
Offset2 = 1,-0.3,-0.3
Angle1 = 0
Rot1 = 1,1,1
Iterations = 10
#endpreset
