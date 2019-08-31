#info BioCube
#define providesInit
#include "MathUtils.frag"
#include "DE-Raytracer.frag"
#group Icosahedral

//Cut and fold (Houdini?) technique with icosahedral folding
//Well... in 2d, it's proved that any (simple?) polygon can be obtained with Cut and fold
//Seems it's the same for non auto intersecting polyhedra. Right?

//Size of the polyhedra
uniform float Scale; slider[0.00,1.5,4.00]
uniform int Iterations;  slider[0,2,50]

uniform vec3 Offset; slider[(-1,-1,-1),(0,1,1),(1,1,1)]
uniform vec3 Offset2; slider[(-1,-1,-1),(1,-0.3,-0.3),(1,1,1)]
uniform  float Qube; slider[-1,0.1,1]

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
      float scalep = 1.0;

      vec3 z0=z;
	z = abs(z);
       //z -= (1,1,1);
	if (z.y>z.x) z.xy =z.yx;
	if (z.z>z.x) z.xz = z.zx;
	if (z.y>z.x) z.xy =z.yx;
       float DE1 =1.0-z.x;
       z = z0;
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
       DE1 = abs(min(Qube/float(n)-DE1,(+z.x)/scalep));
      //DE1 = z.x/scalep;
      }
       //DE1 = z.x/scalep;
	//Distance to the plane going through vec3(Size,0.,0.) and which normal is plnormal
	return DE1;
}

#preset default
FOV = 0.62536
Eye = 1.65826,-1.22975,0.277736
Target = -5.2432,4.25801,-0.607125
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
Detail = -2.84956
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
Iterations = 12
Qube = 0.1
#endpreset