#info Knighty's Pseudo Kleinian (Scale 1 JuliaBox + Something)
#info Modified by Crist-JRoger 4D Quaternion Julia
#info http://www.fractalforums.com/fragmentarium/fragmentarium-an-ide-for-exploring-3d-fractals-and-other-systems-on-the-gpu/msg81393/#msg81393
#include "MathUtils.frag"
#include "DE-Raytracer.frag"

#group 4D Quaternion Julia

uniform int jIterations;  slider[0,16,100]
uniform float jThreshold; slider[0,10,100]
uniform vec4 jC; slider[(-1,-1,-1,-1),(0.18,0.88,0.24,0.16),(1,1,1,1)]
float DE1(vec3 pos) {
	vec4 p = vec4(pos, 0.0);
	vec4 dp = vec4(1.0, 0.0,0.0,0.0);
	for (int i = 0; i < jIterations; i++) {
		dp = 2.0* vec4(p.x*dp.x-dot(p.yzw, dp.yzw), p.x*dp.yzw+dp.x*p.yzw+cross(p.yzw, dp.yzw));
		p = vec4(p.x*p.x-dot(p.yzw, p.yzw), vec3(2.0*p.x*p.yzw)) + jC;
		float p2 = dot(p,p);
		orbitTrap = min(orbitTrap, abs(vec4(p.xyz,p2)));
		if (p2 > jThreshold) break;
	}
	float r = length(p);
	return  0.5 * r * log(r) / length(dp);
}


#group PseudoKleinian

#define USE_INF_NORM

uniform int MI; slider[0,5,20]

// Bailout
//uniform float Bailout; slider[0,20,1000]

uniform float Size; slider[0,1,4]
uniform vec3 CSize; slider[(0,0,0),(1,1,1),(4,4,4)]
uniform vec3 C; slider[(-4,-4,-4),(0,0,0),(4,4,4)]
uniform float TThickness; slider[0,0.01,2]
uniform float DEoffset; slider[0,0,0.01]
uniform vec3 Offset; slider[(-1,-1,-1),(0,0,0),(1,1,1)]

float RoundBox(vec3 p, vec3 csize, float offset)
{
	vec3 di = abs(p) - csize;
	float k=max(di.x,max(di.y,di.z));
	return abs(k*float(k<0.)+ length(max(di,0.0))-offset);
}


float maxcomp(vec3 a) {
	return 	 max(a.x,max(a.y,a.z));
}

float sdToBox( vec3 p, vec3 b )
{
  vec3  di = abs(p) - b;
  float mc = maxcomp(di);
  return min(mc,length(max(di,0.0)));
}

float Thingy(vec3 p, float e){
	p-=Offset;
	return (abs(length(p.xy)*p.z)-e) / sqrt(dot(p,p)+abs(e));
}

float Thing2(vec3 p){
//Just scale=1 Julia box
	float DEfactor=1.;
   	vec3 ap=p+1.;
	for(int i=0;i<MI && ap!=p;i++){
		ap=p;
		p=2.*clamp(p, -CSize, CSize)-p;

		float r2=dot(p,p);
		orbitTrap = min(orbitTrap, abs(vec4(p,r2)));
		float k=max(Size/r2,1.);

		p*=k;DEfactor*=k;

		p+=C;
		orbitTrap = min(orbitTrap, abs(vec4(p,dot(p,p))));
	}
	//Call basic shape and scale its DE
	//return abs(0.5*Thingy(p,TThickness)/DEfactor-DEoffset);

	//Alternative shape
	//return abs(0.5*RoundBox(p, vec3(1.,1.,1.), 1.0)/DEfactor-DEoffset);
	//Just a plane
	return abs(0.5*abs((p.z-Offset.z)*DE1(p))/DEfactor-DEoffset);
}

float DE(vec3 p){
	return  Thing2(p);//RoundBox(p, CSize, Offset);
}

#preset test
FOV = 0.5
Eye = 2.7293,6.2764,-2.28296
Target = -3.06278,13.1388,-6.68298
Up = -0.115178,0.105882,0.316753
Gamma = 0.8654
ToneMapping = 2
Exposure = 1.75533
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3.7
FudgeFactor = 0.7
Dither = 0.86364
NormalBackStep = 1
AO = 0,0,0,0.91489
CamLight = 0.760784,0.870588,1,0.95384
CamLightMin = 1
Glow = 1,1,1,0
GlowMax = 20
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.59596
Z = 0.8,0.78,1,0.41414
R = 0.4,0.7,1,0.06122
BackgroundColor = 0.313725,0.6,0.541176
GradientBackground = 0.3
CycleColors = false
Cycles = 3.25689
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
jIterations = 7
jThreshold = 10
jC = 0.856,1,-1,0.456
MI = 10
Size = 1.03334
CSize = 0.54868,2,2
C = 0,0,0
TThickness = 0
DEoffset = 0
Offset = 0,0,0.26924
Samples = 0
Intensity = 5
Decay = 0.9
ScreenX = 0.5
ScreenY = 0.5
SubframeMax = 10
EquiRectangular = false
FocalPlane = 1.76135
Aperture = 0
DetailAO = -0.67018
MaxRaySteps = 4310
Specular = 0.07143
SpecularExp = 16.176
SpecularMax = 100
SpotLight = 1,0.803922,0.631373,5.4
SpotLightDir = 0.14286,-0.71428
Fog = 0.281
HardShadow = 0.37179 NotLocked
ShadowSoft = 0
Reflection = 0 NotLocked
DebugSun = false
#endpreset

#preset test1
FOV = 0.5
Eye = 4.74822,7.73998,-3.86969
Target = -4.51354,11.4635,-3.27358
Up = 0.0140523,-0.0214554,0.352351
Gamma = 0.8654
ToneMapping = 1
Exposure = 1.75533
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3.7
FudgeFactor = 0.88608
Dither = 0.86364
NormalBackStep = 1
AO = 0,0,0,0.73404
CamLight = 0.760784,0.870588,1,1.6
CamLightMin = 1
Glow = 1,1,1,0.39535
GlowMax = 20
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.59596
Z = 0.8,0.78,1,0.41414
R = 0.4,0.7,1,0.06122
BackgroundColor = 0.313725,0.6,0.541176
GradientBackground = 0.3
CycleColors = false
Cycles = 3.25689
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
jIterations = 7
jThreshold = 9.091
jC = 0.008,-0.104,0.168,1
MI = 10
Size = 1.03334
CSize = 0.54868,2,2
C = 0,0,0
TThickness = 0
DEoffset = 0
Offset = 0,0,0.26924
Samples = 0
Intensity = 5
Decay = 0.9
ScreenX = 0.5
ScreenY = 0.5
SubframeMax = 10
EquiRectangular = false
FocalPlane = 0.328
Aperture = 0.003
DetailAO = -0.67018
MaxRaySteps = 4310
Specular = 0.07143
SpecularExp = 16.176
SpecularMax = 100
SpotLight = 1,0.803922,0.631373,5.4
SpotLightDir = 0.14286,-0.71428
Fog = 0
HardShadow = 0.37179 NotLocked
ShadowSoft = 0
Reflection = 0.2 NotLocked
DebugSun = false
#endpreset

#preset default
FOV = 0.5
Eye = -6.28661,0.100048,1.08467
Target = 3.45371,0.318971,-1.15726
Up = 0,0,1
EquiRectangular = false
FocalPlane = 2.3936
Aperture = 0
Gamma = 0.8654
ToneMapping = 2
Exposure = 1.75533
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -4.38984
DetailAO = -0.48041
FudgeFactor = 0.18391
Dither = 0
NormalBackStep = 1
AO = 0,0,0,0.91489
Specular = 0.07143
SpecularExp = 16.176
SpecularMax = 100
SpotLight = 1,0.803922,0.631373,5.4
SpotLightDir = 0.14286,-0.71428
CamLight = 0.760784,0.870588,1,0.95384
CamLightMin = 1
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.281
HardShadow = 0.37179 NotLocked
ShadowSoft = 0
QualityShadows = false
Reflection = 0 NotLocked
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,0.719
Y = 1,0.6,0,0.5372
Z = 0.8,0.78,1,0.32774
R = 0.4,0.7,1,0.1405
BackgroundColor = 0,0.501961,0.501961
GradientBackground = 0.86955
CycleColors = false
Cycles = 3.25689
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
jIterations = 10
jThreshold = 5
jC = 1,-1,0.3669,-0.32374
MI = 10
TThickness = 0
DEoffset = 0
Offset = -1,-1,-1
C = -2,-0.000377293,-0.50792
CSize = 0.22428,0.97196,0.07476
Size = 0.93912
MaxRaySteps = 3211
#endpreset
