#version 330 compatibility
#info MandalayBox Distance Estimator.
#define providesInit
#define WANG_HASH
#define KN_VOLUMETRIC
//#define USE_IQ_CLOUDS
#include "MathUtils.frag"
#include "DE-Kn2cr11.frag"
uniform float time;
#group MandalayBox
// Number of iterations.
uniform int Iterations;  slider[0,10,20]

// Scale parameter. A perfect Menger is 3.0
uniform float Scale; slider[-6.0,2,6.00]
uniform bool DoBoxFold; checkbox[false]
uniform vec3 RotVector; slider[(0,0,0),(1,0,0),(1,1,1)]

// Scale parameter. A perfect Menger is 3.0
uniform float RotAngle; slider[0.00,00,360]

uniform vec3 fo; slider[(0.,0,0),(0.5,0.5,0.5),(2.,2.,2.)]
uniform vec3 g; slider[(0.,0,0),(0.1,0.1,0.1),(2.,2.,2.)]
uniform bool Serial; checkbox[false]
//Mandelbox's radial fold
uniform float MinRad2;  slider[0,0.25,2.0]
// Julia
uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-4,-4,-4),(1,1,1),(4,4,4)]

mat3 rot;
float sr;
void init() {
	rot = rotationMatrix3(normalize(RotVector), RotAngle);
	sr = 30.;
}
//DarkBeam's "fold"... reinterpreted... it's more than a fold, much more! Just awesome!
float DBFold(vec3 p, float fo, float g){
	if(p.z>p.y) p.yz=p.zy;//Diagonal fold
	//Tis is slightly different from the original fold in order to make it continuous in this context
	float vx=p.x-2.*fo;
	float vy=p.y-4.*fo;
 	float v=max(abs(vx+fo)-fo,vy);
	float v1=max(vx-g,p.y);
	v=min(v,v1);
	return min(v,p.x);
}
//the coordinates are pushed/pulled in parallel
vec3 DBFoldParallel(vec3 p, vec3 fo, vec3 g){
	vec3 p1=p;
	p.x=DBFold(p1,fo.x,g.x);
	p.y=DBFold(p1.yzx,fo.y,g.y);
	p.z=DBFold(p1.zxy,fo.z,g.z);
	return p;
}
//serial version
vec3 DBFoldSerial(vec3 p, vec3 fo, vec3 g){
	p.x=DBFold(p,fo.x,g.x);
	p.y=DBFold(p.yzx,fo.y,g.y);
	p.z=DBFold(p.zxy,fo.z,g.z);
	return p;
}
float DE(vec3 p)
{
	vec4 JC=Julia? vec4(JuliaC,0.) : vec4(p,1.);
	float r2=dot(p,p);
	float dd = 1.;
	for(int i = 0; i<Iterations && r2<10000.; i++){

		if(DoBoxFold) p = p - clamp(p.xyz, -1.0, 1.0) * 2.0;  // mandelbox's box fold

		//Apply pull transformation
		vec3 signs=sign(p);//Save 	the original signs
		p=abs(p);
		if(Serial) p=DBFoldSerial(p,fo,g);
		else p=DBFoldParallel(p,fo,g);
		p*=signs;//resore signs: this way the mandelbrot set won't extend in negative directions

		//Sphere fold
		r2=dot(p,p);
		float  t = clamp(1./r2, 1., 1./MinRad2);
		p*=t; dd*=t;

		//Scale and shift
		p=p*Scale+JC.xyz; dd=dd*Scale+JC.w;
		p=rot*p;

		//For coloring and bailout
		r2=dot(p,p);
		orbitTrap = min(orbitTrap, abs(vec4(p.x,p.y,p.z,r2)));
	}
	dd=abs(dd);
#if 1
	return (sqrt(r2)-sr)/dd;//bounding volume is a sphere
#else
	p=abs(p); return (max(p.x,max(p.y,p.z))-sr)/dd;//bounding volume is a cube
#endif
}


#preset Default
FOV = 0.4
Eye = -3.93747,6.81834,-9.853
Target = -0.488534,1.38543,-2.19871
Up = 0.177166,0.852394,0.491972
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3
DetailAO = -1.77779
FudgeFactor = 1
MaxRaySteps = 56
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,1
Specular = 0.01987
SpecularExp = 69.118
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.7936,-0.77936
CamLight = 1,1,1,0.6171
CamLightMin = 1
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 1
ShadowSoft = 20
Reflection = 0
DebugSun = false
BaseColor = 0.65098,0.65098,0.65098
OrbitStrength = 0.34014
X = 0.5,0.6,0.6,0.4625
Y = 1,0.6,0,0.4875
Z = 0.8,0.78,1,0.63126
R = 0.4,0.7,1,-0.2978
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 0.1
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 10
Scale = 2
RotVector = 1,0,0
RotAngle = 0
fo = 1.17118,0.2943,1.6096
g = 1.12994,0.69436,0
Serial = false
MinRad2 = 0.07524
Julia = false
JuliaC = 1,1,1
DoBoxFold = false
#endpreset


#preset The_town
FOV = 0.56284
Eye = -3.45179,6.37631,2.98642
Target = -0.652397,-2.09871,-1.06227
Up = -0.999959,-0.00821777,-0.0039258
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3
DetailAO = -1.10789
FudgeFactor = 1
MaxRaySteps = 244
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.89655
Specular = 0.08679
SpecularExp = 16
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.36066,-0.60656
CamLight = 1,1,1,0.69828
CamLightMin = 0.81301
Glow = 1,1,1,0.17808
GlowMax = 20
Fog = 0
HardShadow = 1
ShadowSoft = 20
Reflection = 0
DebugSun = false
BaseColor = 0.623529,0.623529,0.623529
OrbitStrength = 0.37743
X = 0.5,0.6,0.6,-0.22968
Y = 1,0.6,0,0.44876
Z = 0.8,0.78,1,0.15902
R = 0.4,0.7,1,0.08464
BackgroundColor = 0.596078,0.6,0.513725
GradientBackground = 0.3
CycleColors = false
Cycles = 7.03846
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 12
Scale = 3.10832
DoBoxFold = true
RotVector = 1,0,0
RotAngle = 173.102
fo = 0.12,0.42286,0.74286
g = 0.4273,0.62314,0.5638
Serial = true
MinRad2 = 0.13794
Julia = true
JuliaC = -1.80488,1.0488,-0.12196
#endpreset

#preset Julia_without_boxfold
FOV = 0.4
Eye = -4.90824,2.53261,-8.51092
Target = 0.0511652,-1.16735,-0.655056
Up = 0.17362,0.915496,0.362937
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3
DetailAO = -0.5
FudgeFactor = 1
MaxRaySteps = 169
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,1
Specular = 0.01987
SpecularExp = 69.118
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.7936,-0.77936
CamLight = 1,1,1,1.07806
CamLightMin = 0.78445
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 1
ShadowSoft = 20
Reflection = 0
DebugSun = false
BaseColor = 0.882353,0.882353,0.882353
OrbitStrength = 0.38776
X = 0.5,0.6,0.6,0.4625
Y = 1,0.6,0,0.4875
Z = 0.8,0.78,1,0.63126
R = 0.4,0.7,1,-0.08464
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 0.1
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 10
Scale = 2
DoBoxFold = false
RotVector = 1,0,0
RotAngle = 0
fo = 0.93714,0.40572,1.49714
g = 1.11864,0.92656,0.77966
Serial = false
MinRad2 = 0.05298
Julia = true
JuliaC = 0.23172,1,1
#endpreset


#preset Mandelbox
FOV = 0.4
Eye = -5.48673,5.57229,-8.29521
Target = -0.691605,-0.0782387,-1.58117
Up = 0.243612,0.817671,0.521601
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3
DetailAO = -0.5
FudgeFactor = 1
MaxRaySteps = 56
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,1
Specular = 0.01987
SpecularExp = 69.118
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.7936,-0.77936
CamLight = 1,1,1,0.6171
CamLightMin = 1
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 1
ShadowSoft = 20
Reflection = 0
DebugSun = false
BaseColor = 0.65098,0.65098,0.65098
OrbitStrength = 0.14966
X = 0.5,0.6,0.6,0.4625
Y = 1,0.6,0,0.4875
Z = 0.8,0.78,1,0.63126
R = 0.4,0.7,1,-0.2978
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 0.1
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 10
Scale = 2
RotVector = 1,0,0
RotAngle = 0
fo = 0,0,0
g = 0,0,0
Serial = false
MinRad2 = 0.07524
Julia = false
JuliaC = 0.23172,1,1
DoBoxFold = true
#endpreset


#preset Extrusions
FOV = 0.4
Eye = -7.09247,3.6575,-7.85706
Target = -0.960155,-0.410378,-1.08601
Up = 0.407833,0.818881,0.403865
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3
DetailAO = -0.88886
FudgeFactor = 1
MaxRaySteps = 56
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,1
Specular = 0.01987
SpecularExp = 34.926
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.7936,-0.77936
CamLight = 1,1,1,0.6171
CamLightMin = 1
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 1
ShadowSoft = 20
Reflection = 0
DebugSun = false
BaseColor = 0.65098,0.65098,0.65098
OrbitStrength = 0.14966
X = 0.5,0.6,0.6,0.4625
Y = 1,0.6,0,0.4875
Z = 0.8,0.78,1,0.63126
R = 0.4,0.7,1,-0.2978
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 0.1
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 10
Scale = 2
DoBoxFold = false
RotVector = 1,0,0
RotAngle = 0
fo = 0.6967,0,0
g = 1.11864,0.92656,0.77966
Serial = false
MinRad2 = 0.07524
Julia = true
JuliaC = 0.23172,1,1
#endpreset


#preset Spaceship
FOV = 0.56284
Eye = -3.61238,0.483741,1.84241
Target = 1.45174,1.93239,-6.42262
Up = -0.915273,-0.286927,-0.282753
EquiRectangular = false
FocalPlane = 1
Aperture = 0
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3
DetailAO = -1.10789
FudgeFactor = 1
MaxRaySteps = 244
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.89655
Specular = 0.08679
SpecularExp = 16
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.36066,-0.60656
CamLight = 1,1,1,0.69828
CamLightMin = 0.81301
Glow = 1,1,1,0.17808
GlowMax = 20
Fog = 0
HardShadow = 1
ShadowSoft = 20
Reflection = 0
DebugSun = false
BaseColor = 0.623529,0.623529,0.623529
OrbitStrength = 0.37743
X = 0.5,0.6,0.6,-0.22968
Y = 1,0.6,0,0.44876
Z = 0.8,0.78,1,0.15902
R = 0.4,0.7,1,0.08464
BackgroundColor = 0.596078,0.6,0.513725
GradientBackground = 0.3
CycleColors = false
Cycles = 7.03846
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 12
DoBoxFold = true
RotVector = 1,0,0
RotAngle = 0
fo = 0.0961,1.28528,0.74286
g = 0.4273,0.62314,0.5638
Serial = true
MinRad2 = 0.11258
Julia = true
Scale = -4.92996
JuliaC = -1.6592,1.8392,1.09328
#endpreset

#preset AbstractBlue
FOV = 0.56284
Eye = -0.7515541,0.4852493,-3.535848
Target = -0.8981702,2.15349,6.120738
Up = -0.9607636,0.0051662,-0.0154797
EquiRectangular = false
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
OrbitStrength = 0.5157895
X = 0.5,0.6,0.6,-0.22968
Y = 1,0.6,0,0.44876
Z = 0.8,0.78,1,0.15902
R = 0.4,0.7,1,-0.030303
CycleColors = false
Cycles = 0.1
Aperture = 0
Iterations = 8
Scale = 3.111429
DoBoxFold = true
RotVector = 1,0,0
RotAngle = 174.9533
fo = 0.07,0.07,0.4444444
g = 1.168675,0,0
Serial = true
MinRad2 = 0.1290323
Julia = true
JuliaC = -1.870504,0.8800007,-2.158273
FocalPlane = 1
InFocusAWidth = 0
ApertureNbrSides = 7
ApertureRot = 0
ApStarShaped = false
BokehCircle = 2
LensFlare = false
FlareIntensity = 0
FlareSamples = 10
FlareDispersal = 0.5
FlareHaloWidth = 0.5
FlareDistortion = 0.5
Bloom = false
BloomIntensity = 0
BloomPow = 1
BloomTaps = 10
RayBounces = 2
RaySteps = 745
FudgeFactor = 1
HitDistance = 1e-05
MaxDepth = 15
Mat_Color = 0.65,0.65,0.65
Mat_Refl = 0.8503937
Mat_ReflExp = 0.1944525
UseTexture = true
DeTex = seamless-texture.jpg
DeTexScale = 0.5
DeTexExposure = 0.939759
SpotLight = 1,1,1,1
SpotLightDir = -0.2323232,-0.5353535
SpotSize = 3
SpotExp = 0.3100996
BgColor = 0.3,0.5,0.7,2
Ambience = 1
Fog = 0.3651316
GRIter = 5
GRPower = 1
#endpreset

#preset AbstractBlue2
FOV = 0.56284
Eye = -0.7483918,0.498884,-3.536915
Target = -0.8519837,0.6920967,-1.375187
Up = -0.9152372,-0.0049344,-0.0434179
EquiRectangular = false
Gamma = 2
ToneMapping = 5
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 6
AntiAliasScale = 1.5
OrbitStrength = 0.5
X = 0.5,0.6,0.6,-0.22968
Y = 1,0.6,0,0.44876
Z = 0.8,0.78,1,0.15902
R = 0.4,0.7,1,0.0731707
CycleColors = false
Cycles = 0.1
Iterations = 8
Scale = 3.111429
DoBoxFold = true
RotVector = 1,0,0
RotAngle = 174.9533
fo = 0.08,0.08,0.4444444
g = 1.172414,0,0
Serial = true
MinRad2 = 0.1290323
Julia = true
JuliaC = -1.880504,0.8800007,-2.188273
Bloom = false
BloomPow = 1
FocalPlane = 3
Aperture = 0.002
InFocusAWidth = 0
DofCorrect = true
ApertureNbrSides = 7
ApertureRot = 0
ApStarShaped = false
BloomIntensity = 0
BloomTaps = 10
BloomStrong = 1
DepthToAlpha = true
ShowDepth = false
DepthMagnitude = 1
Detail = -4.425287
RefineSteps = 4
FudgeFactor = 1
MaxRaySteps = 974
MaxDistance = 20
Dither = 3
NormalBackStep = 5
DetailAO = -3.716049
coneApertureAO = 0.5
maxIterAO = 20
FudgeAO = 1
AO_ambient = 1
AO_camlight = 1
AO_pointlight = 1
AoCorrect = 1
Specular = 0.2877193
SpecularExp = 50
CamLight = 1,1,1,1
AmbiantLight = 1,1,1,1
Reflection = 0.5215687,0.5215687,0.5215687
ReflectionsNumber = 2
SpotGlow = false
SpotLight = 1,1,1,1
LightPos = -2.564103,2.271062,-0.6593407
LightSize = 0.1
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 0
ShadowSoft = 0
ShadowBlur = 0
perf = false
SSS = true
sss1 = 0.1140065
sss2 = 0.4836601
BaseColor = 1,1,1
BackgroundColor = 0.3647059,0.4,0.6
GradientBackground = 0.3
EnableFloor = true
FloorNormal = 1,0,0
FloorHeight = -0.4512635
FloorColor = 0.2588235,0.3882353,0.6
HF_Fallof = 5
HF_Const = 0.024169
HF_Intensity = 0.022
HF_Dir = -1,0,0
HF_Offset = 1
HF_Color = 1,1,1,1
HF_Scatter = 10
HF_Anisotropy = 0.7176471,0.5921569,0.1294118
HF_FogIter = 4
HF_CastShadow = true
EnCloudsDir = false
CloudDir = 0,0,1
CloudScale = 1
CloudFlatness = 0
CloudTops = 1
CloudBase = -1
CloudDensity = 1
CloudRoughness = 1
CloudContrast = 1
CloudColor = 0.65,0.68,0.7
CloudColor2 = 0.07,0.17,0.24
SunLightColor = 0.7,0.5,0.3
Cloudvar1 = 0.99
Cloudvar2 = 1
CloudIter = 5
CloudBgMix = 1
WindDir = 0,0,1
WindSpeed = 1
#endpreset

#preset CityScape
FOV = 0.4565916
Eye = -0.2481333,0.6908153,-0.1966778
Target = 9.256749,-2.397778,0.1462449
Up = 0.0325578,-0.0105796,-0.9976999
EquiRectangular = false
Gamma = 1
ToneMapping = 2
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 10
AntiAliasScale = 3
DepthToAlpha = true
ShowDepth = false
DepthMagnitude = 1
Detail = -3
FudgeFactor = 1
NormalBackStep = 1
CamLight = 1,1,1,1
BaseColor = 0.65098,0.65098,0.65098
OrbitStrength = 0.7265625
X = 0.5,0.6,0.6,0.4625
Y = 1,0.6,0,0.4875
Z = 0.8,0.78,1,0.63126
R = 0.4,0.7,1,-0.2978
BackgroundColor = 0.1490196,0.1607843,0.2470588
GradientBackground = 1.139535
CycleColors = true
Cycles = 2.1
EnableFloor = false
FloorNormal = 0,1,0
FloorHeight = -1.245487
FloorColor = 1,1,1
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
DofCorrect = true
ApertureNbrSides = 7
ApertureRot = 0
ApStarShaped = false
Bloom = false
BloomIntensity = 0
BloomPow = 1
BloomTaps = 10
BloomStrong = 1
RefineSteps = 4
MaxRaySteps = 469
MaxDistance = 20
Dither = 0.5
DetailAO = -0.88886
coneApertureAO = 0.5
maxIterAO = 20
FudgeAO = 1
AO_ambient = 1
AO_camlight = 1
AO_pointlight = 1
AoCorrect = 1
Specular = 0.01987
SpecularExp = 34.926
AmbiantLight = 1,1,1,1
Reflection = 0.5411765,0.5411765,0.5411765
ReflectionsNumber = 2
SpotGlow = false
SpotLight = 1,1,1,1
LightPos = -2.564103,2.271062,-0.6593407
LightSize = 0.1
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 1
ShadowSoft = 20
ShadowBlur = 0
perf = false
SSS = true
sss1 = 0.1140065
sss2 = 0.4836601
HF_Fallof = 4
HF_Const = 0.026
HF_Intensity = 0.02
HF_Dir = 0,0,-1
HF_Offset = 1
HF_Color = 0.2862745,0.2862745,0.2862745,1
HF_Scatter = 10
HF_Anisotropy = 0.7176471,0.5921569,0.1294118
HF_FogIter = 5
HF_CastShadow = true
EnCloudsDir = true
CloudDir = 0,0,-1
CloudScale = 1.1
CloudFlatness = 1
CloudTops = 1
CloudBase = 0
CloudDensity = 1
CloudRoughness = 1.61017
CloudContrast = 1
CloudColor = 0.65,0.68,0.7
CloudColor2 = 0.07,0.17,0.24
SunLightColor = 0.7,0.5,0.3
Cloudvar1 = 1
Cloudvar2 = 10
CloudIter = 5
CloudBgMix = 1
WindDir = 0,1,0
WindSpeed = 1
Iterations = 10
Scale = 2.292359
DoBoxFold = false
RotVector = 0,0,1
RotAngle = 45.67164
fo = 1,1.188854,0
g = 0.7828746,1.051988,1.027523
Serial = false
MinRad2 = 0.8280702
Julia = true
JuliaC = -0.0533333,0.64,-0.0533333
#endpreset
