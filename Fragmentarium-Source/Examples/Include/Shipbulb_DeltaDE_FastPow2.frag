#info Mandelbulb Distance Estimator
#define providesInit
#include "MathUtils.frag"
#include "DE-Raytracer.frag"
#group Mandelbulb


// Number of fractal iterations.
uniform int Iterations;  slider[0,9,100]

// Number of color iterations.
uniform int ColorIterations;  slider[0,9,50]

uniform int SkipColorIterations; slider[0,0,20]

// Mandelbulb exponent (8 is standard)
uniform float Power; slider[0,8,16]

// Bailout radius
uniform float Bailout; slider[0,16,64]

// Alternate is slightly different, but looks more like a Mandelbrot for Power=2
uniform bool AlternateVersion; checkbox[false]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]

uniform float RotAngle; slider[0.00,0,180]

uniform bool CutZ; checkbox[false]

mat3 rot;
uniform float time;
void init() {
	 rot = rotationMatrix3(normalize(RotVector), RotAngle+time*10.0);
}




vec2 complexSqr(vec2 a){
	return  vec2( a.x*a.x -  a.y*a.y,2.0*a.x*a.y);
}


vec3 triplexSqr(vec3 a) {
	a = abs(a);
	float r = length(a.xy);
	vec2 newXY =  complexSqr(a.xy/r);
	vec2 newRZ = complexSqr(vec2(r,a.z));
	
	newXY = newXY * newRZ.x;
	
	return vec3(newXY.x, newXY.y, newRZ.y);
}


vec3 triplexPow(inout vec3 z) {
	z = abs(z);
	float zr0 = length(z);
	float zo0 = asin( z.z/zr0 );
	float zi0 = atan( z.y,z.x );
	float zr = pow( zr0, Power-1.0 );
	float zo = zo0 * Power;
	float zi = zi0 * Power;
	zr *= zr0;
	z  = zr*vec3( cos(zo)*cos(zi), cos(zo)*sin(zi), sin(zo) );
	return z;
}


uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-2,-2,-2),(0,0,0),(2,2,2)]

// Compute the distance from `pos` to the Mandelbox.
float DE(vec3 pos) {
	float delta = exp2(-18. + max( floor(log2(length(pos))),0.0) );

	vec3 pos2 = pos + vec3(delta,0.,0.);
	vec3 pos3 = pos + vec3(0.,delta,0.);
	vec3 pos4 = pos + vec3(0.,0.,delta);
	
	
	vec3 z = pos;
	vec3 z2 = pos2;
	vec3 z3 = pos3;
	vec3 z4 = pos4;


	float r;
	//float dr=1.0;
	int i=0;
	r=length(z);
	while(r<Bailout && (i<Iterations)) {
		/*
		if (AlternateVersion) {
			powN2(z,r,dr);
		} else {
			powN1(z,r,dr);
		}
		*/
		z = triplexSqr(z);
		z+=(Julia ? JuliaC : pos);

		z2 = triplexSqr(z2);
		z2 +=(Julia ? JuliaC : pos2);

		z3 = triplexSqr(z3);
		z3 +=(Julia ? JuliaC : pos3);

		z4 = triplexSqr(z4);
		z4 +=(Julia ? JuliaC : pos4);

		r=length(z);
		if (i >= SkipColorIterations &&  i<ColorIterations) orbitTrap = min(orbitTrap, abs(vec4(z.x,z.y,z.z,r*r)));
		i++;
	}
	float dx = length(z2-z)/delta;
	float dy = length(z3-z)/delta;
	float dz = length(z4-z)/delta;

	float dr = length(vec3(dx,dy,dz));
	
	float de = 0.5*log(r)*r/dr;

	if (CutZ) de = max(de,pos.z);
	return de;
	/*
	Use this code for some nice intersections (Power=2)
	float a =  max(0.5*log(r)*r/dr, abs(pos.y));
	float b = 1000;
	if (pos.y>0)  b = 0.5*log(r)*r/dr;
	return min(min(a, b),
		max(0.5*log(r)*r/dr, abs(pos.z)));
	*/
}

#preset Default
FOV = 0.82926
Eye = -0.600794,-0.290776,2.02198
Target = -0.587564,-1.1107,-6.80163
Up = 0.0607786,-0.993842,0.0924425
EquiRectangular = false
FocalPlane = 0.06375
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3.71679
DetailAO = -2.28571
FudgeFactor = 0.6
MaxRaySteps = 923
Dither = 0.64035
NormalBackStep = 1.5
AO = 0,0,0,0.46939
Specular = 0
SpecularExp = 11.111
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.28396,-0.4321
CamLight = 1,1,1,1.30434
CamLightMin = 0.15663
Glow = 1,1,1,0.83333
GlowMax = 491
HardShadow = 0.39024
ShadowSoft = 11.3924
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.80519
X = 1,0.258824,0.129412,1
Y = 0,0.74902,1,1
Z = 0.913725,1,0.12549,0
R = 0.85098,1,0.803922,0.11764
BackgroundColor = 0.258824,0.623529,0.866667
GradientBackground = 0.3261
CycleColors = false
Cycles = 32.3
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 11
Power = 8
Bailout = 2
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
CutZ = false
Julia = false
JuliaC = -1.81984,-0.006,-0.00684
ColorIterations = 25
SkipColorIterations = 0
Fog = 0
#endpreset

#preset Octobulb
FOV = 0.62536
Eye = -0.184126,0.843469,1.32991
Target = 1.48674,-5.55709,-4.56665
Up = -0.240056,-0.718624,0.652651
AntiAlias = 1
Detail = -2.47786
DetailAO = -0.21074
FudgeFactor = 1
MaxRaySteps = 164
BoundingSphere = 2
Dither = 0.5
AO = 0,0,0,0.7
Specular = 1
SpecularExp = 27.082
SpotLight = 1,1,1,0.94565
SpotLightDir = 0.5619,0.18096
CamLight = 1,1,1,0.23656
CamLightMin = 0.15151
Glow = 0.415686,1,0.101961,0.18421
Fog = 0.60402
HardShadow = 0.72308
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.62376
X = 0.411765,0.6,0.560784,-0.37008
Y = 0.666667,0.666667,0.498039,0.86886
Z = 0.666667,0.333333,1,-0.25984
R = 0.4,0.7,1,0.36508
BackgroundColor = 0.666667,0.666667,0.498039
GradientBackground = 0.5
CycleColors = true
Cycles = 7.03524
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 14
ColorIterations = 6
Power = 8.18304
Bailout = 6.279
AlternateVersion = true
RotVector = 1,0,0
RotAngle = 77.8374
CutZ = false
#endpreset


#preset Rabbit
FOV = 0.62536
Eye = 1.65826,-1.22975,0.277736
Target = -5.2432,4.25801,-0.607125
Up = 0.401286,0.369883,-0.83588
EquiRectangular = false
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
Detail = -3.5
DetailAO = -1.35716
FudgeFactor = 1
MaxRaySteps = 600
BoundingSphere = 2
Dither = 0.51754
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1.6456
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
Iterations = 24
ColorIterations = 8
Power = 2
Bailout = 6.279
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = true
JuliaC = -0.12500,-0.64952,0
CutZ = true
#endpreset

#preset spires
FOV = 0.82926
Eye = 0.0651154,0.312317,0.0478826
Target = -8.43045,0.0513401,-2.45927
Up = -0.00704173,0.996777,-0.0798962
EquiRectangular = false
FocalPlane = 0.06375
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -4.76994
DetailAO = -2.28571
FudgeFactor = 0.6
MaxRaySteps = 923
Dither = 0.64035
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.63626,0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.83333
GlowMax = 491
HardShadow = 0.35385
ShadowSoft = 12.5806
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.74026
X = 0.694118,0.184314,0.0941176,1
Y = 0.345098,0.666667,0,1
Z = 0.913725,1,0.12549,1
R = 0.631373,0.764706,0.427451,0.39216
BackgroundColor = 0.258824,0.623529,0.866667
GradientBackground = 0.3261
CycleColors = false
Cycles = 32.3
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 95
Power = 8
Bailout = 2
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
CutZ = false
Julia = true
JuliaC = -1.81984,-0.006,-0.00684
ColorIterations = 25
SkipColorIterations = 0
Fog = 1.16
#endpreset

#preset Jellyfish
FOV = 0.82926
Eye = -2.38201,-0.734606,0.03702
Target = 6.04027,2.02095,0.0121062
Up = -0.0103291,0.0225321,-0.999675
EquiRectangular = false
FocalPlane = 0.06375
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3.53094
DetailAO = -1.85717
FudgeFactor = 0.6
MaxRaySteps = 857
Dither = 0.64035
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.30864,0.65432
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.83333
GlowMax = 491
HardShadow = 0.57317
ShadowSoft = 11.8988
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.64935
X = 0.694118,0.184314,0.0941176,0
Y = 1,0.615686,0,1
Z = 0.0901961,0.333333,1,1
R = 0.768627,0.862745,0.282353,0
BackgroundColor = 0.258824,0.623529,0.866667
GradientBackground = 0.3261
CycleColors = false
Cycles = 15.2887
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 95
Power = 8
Bailout = 2
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
CutZ = false
Julia = true
JuliaC = -0.23424,0.52252,-0.8108
ColorIterations = 8
SkipColorIterations = 0
Fog = 0.24
#endpreset



#preset Jungle
FOV = 0.82926
Eye = -0.847473,-0.523592,0.190509
Target = 5.96199,3.95752,-3.28504
Up = 0.271204,0.291978,0.917168
EquiRectangular = false
FocalPlane = 0.06375
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 0.365
Detail = -3.555
DetailAO = -2.88813
FudgeFactor = 0.6
MaxRaySteps = 923
Dither = 0
NormalBackStep = 1
AO = 0,0,0,0.56349
Specular = 0.25397
SpecularExp = 12.121
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.17432,0.26606
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.83333
GlowMax = 491
Fog = 0
HardShadow = 0.35385 NotLocked
ShadowSoft = 12.5806
QualityShadows = true
Reflection = 0 NotLocked
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.74026
X = 0.694118,0.184314,0.0941176,1
Y = 1,0.25098,0,-1
Z = 0.415686,1,0.0235294,1
R = 0.631373,0.764706,0.427451,0.28572
BackgroundColor = 0.258824,0.623529,0.866667
GradientBackground = 0.3261
CycleColors = false
Cycles = 32.3
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 95
ColorIterations = 25
SkipColorIterations = 0
Power = 8
Bailout = 2
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
CutZ = false
Julia = true
JuliaC = -0.02152,-0.92472,0
#endpreset

#preset Jungle2
FOV = 0.82926
Eye = 0.528178,-1.53261,0.269996
Target = -2.46019,6.62827,-1.46154
Up = -0.231643,0.12,0.965353
EquiRectangular = false
FocalPlane = 0.06375
Aperture = 0
Gamma = 2.08335
ToneMapping = 3
Exposure = 0.6522
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Detail = -3.53094
DetailAO = -1.85717
FudgeFactor = 0.6
MaxRaySteps = 857
Dither = 0.64035
NormalBackStep = 1
AO = 0,0,0,0.85185
Specular = 1.6456
SpecularExp = 16.364
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = -0.30864,0.65432
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.83333
GlowMax = 491
HardShadow = 0.57317
ShadowSoft = 11.8988
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.7013
X = 0.101961,0.694118,0,0.6699
Y = 1,0,0,0.45632
Z = 0,0.768627,1,1
R = 0.768627,0.862745,0.282353,0
BackgroundColor = 0.258824,0.623529,0.866667
GradientBackground = 0.3261
CycleColors = false
Cycles = 15.2887
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 95
Power = 8
Bailout = 2
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
CutZ = false
Julia = true
JuliaC = 0.88288,-1.27928,-0.05404
ColorIterations = 8
SkipColorIterations = 0
Fog = 0.24
#endpreset

#preset Cacti
FOV = 0.78048
Eye = -1.23955,0.700567,0.207592
Target = 5.94216,-4.04142,-1.90545
Up = 0.21928,-0.1,0.969697
EquiRectangular = false
FocalPlane = 0.06375
Aperture = 0
Gamma = 2.2
ToneMapping = 3
Exposure = 0.79593
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 1.7
Detail = -3.7
DetailAO = -3.35713
FudgeFactor = 0.6
MaxRaySteps = 923
Dither = 0
NormalBackStep = 1.5
AO = 0,0,0,0.5679
Specular = 0.25397
SpecularExp = 9.722
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.03126,-0.34374
CamLight = 1,1,1,0.72464
CamLightMin = 0.3253
Glow = 1,1,1,0.45556
GlowMax = 456
Fog = 0
HardShadow = 0.87692 NotLocked
ShadowSoft = 10
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.74
X = 0.780392,0.2,0.0980392,1
Y = 0.223529,0.988235,1,-0.24272
Z = 0.415686,1,0.0235294,0.68932
R = 0.694118,0.764706,0.564706,0.33334
BackgroundColor = 0.258824,0.623529,0.866667
GradientBackground = 0.3261
CycleColors = false
Cycles = 32.3
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 23
ColorIterations = 17
SkipColorIterations = 0
Power = 8
Bailout = 2
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
CutZ = false
Julia = true
JuliaC = -0.37836,-0.88288,-0.00084
#endpreset

#preset CoolThing
FOV = 0.78048
Eye = 1.15285,1.37228,0.852599
Target = -2.9811,-6.01607,-1.76491
Up = -0.0701926,-0.297721,0.951226
EquiRectangular = false
FocalPlane = 0.06375
Aperture = 0
Gamma = 2.2
ToneMapping = 3
Exposure = 0.79593
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 1.7
Detail = -3.71679
DetailAO = -3.35713
FudgeFactor = 0.62651
MaxRaySteps = 1055
Dither = 0
NormalBackStep = 1
AO = 0,0,0,0.5679
Specular = 0.25397
SpecularExp = 9.722
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.15626,0.65626
CamLight = 1,1,1,0.72464
CamLightMin = 0.3253
Glow = 1,1,1,0.45556
GlowMax = 456
Fog = 0
HardShadow = 0.87692 NotLocked
ShadowSoft = 10
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.74
X = 0.243137,0.592157,0.780392,0.45632
Y = 0.482353,1,0.262745,1
Z = 1,0.968627,0.6,-1
R = 0.494118,0.282353,0.133333,0.66666
BackgroundColor = 0.258824,0.623529,0.866667
GradientBackground = 0.3261
CycleColors = false
Cycles = 27.7434
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 30
ColorIterations = 20
SkipColorIterations = 2
Power = 8
Bailout = 2
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
CutZ = false
Julia = true
JuliaC = -1.35136,-0.16216,0.018
#endpreset

#preset Roots
FOV = 0.78048
Eye = 0.00659785,0.264316,0.616031
Target = -1.22874,-0.842479,9.32105
Up = -0.972616,0.199288,-0.112686
EquiRectangular = false
FocalPlane = 0.06375
Aperture = 0
Gamma = 2.2
ToneMapping = 3
Exposure = 0.79593
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 1.7
Detail = -3.09736
DetailAO = -3.35713
FudgeFactor = 0.62651
MaxRaySteps = 1055
Dither = 0
NormalBackStep = 1
AO = 0,0,0,0.5679
Specular = 0.25397
SpecularExp = 9.722
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.1875,-0.0625
CamLight = 1,1,1,0.72464
CamLightMin = 0.3253
Glow = 1,1,1,0.45556
GlowMax = 456
Fog = 0
HardShadow = 0.50769 NotLocked
ShadowSoft = 13.871
QualityShadows = false
Reflection = 0
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.74
X = 0.243137,0.592157,0.780392,0.45632
Y = 0.482353,1,0.262745,-0.18446
Z = 1,0.968627,0.6,0.68932
R = 0.494118,0.282353,0.133333,0.31372
BackgroundColor = 0.258824,0.623529,0.866667
GradientBackground = 0.3261
CycleColors = false
Cycles = 27.7434
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 26
ColorIterations = 19
SkipColorIterations = 2
Power = 8
Bailout = 2
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
CutZ = false
Julia = true
JuliaC = 0.4144,-0.37836,-0.48648
#endpreset

#preset GlassOrnament
FOV = 0.78048
Eye = 2.73774,-1.32644,0.0194423
Target = -5.4435,2.07599,-0.119293
Up = -0.00484898,-0.0523786,-0.998615
EquiRectangular = false
FocalPlane = 0.06375
Aperture = 0
Gamma = 2.2
ToneMapping = 3
Exposure = 0.79593
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 1.7
Detail = -2.74687
DetailAO = -3.35713
FudgeFactor = 0.62651
MaxRaySteps = 1055
Dither = 0
NormalBackStep = 1
AO = 0,0,0,0.46032
Specular = 0.33846
SpecularExp = 61
SpecularMax = 10
SpotLight = 1,1,1,1
SpotLightDir = 0.52294,-0.02752
CamLight = 1,1,1,0.43298
CamLightMin = 0
Glow = 1,1,1,0.11864
GlowMax = 211
Fog = 0
HardShadow = 0.53636 NotLocked
ShadowSoft = 13.8318
QualityShadows = false
Reflection = 0.78049 NotLocked
DebugSun = false
BaseColor = 1,1,1
OrbitStrength = 0.74
X = 0.243137,0.592157,0.780392,-0.14864
Y = 0.482353,1,0.262745,-0.17568
Z = 1,0.968627,0.6,0.45946
R = 0.819608,0.0823529,0.0823529,1
BackgroundColor = 0.109804,0.415686,0.596078
GradientBackground = 0.65935
CycleColors = false
Cycles = 8.41662
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 10
ColorIterations = 4
SkipColorIterations = 1
Power = 8
Bailout = 2
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
CutZ = false
Julia = true
JuliaC = -0.20512,0.76924,-1.10256
#endpreset