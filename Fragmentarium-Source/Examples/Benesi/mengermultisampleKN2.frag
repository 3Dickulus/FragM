#version 120
#info Menger Distance Estimator.  Benesi.
#define providesInit
const float pi=2.*1.570796326794897;
const float pi2=2.*pi;
#include "MathUtils.frag"
//#include "Fast-Raytracerexp.frag"
#include "DE-Kn2ColorandHeightmap.frag"
#group Menger
// Based on Knighty's Kaleidoscopic IFS 3D Fractals, described here:
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/
float sr32=1.2247448713915890491;
uniform float time;

const float sr13=sqrt(1./3.);
const float sr23=sqrt (2./3.);
// Scale parameter. A perfect Menger is 3.0
uniform float Scale; slider[0.00,3.0,4.00]
uniform float s; slider[0.000,0.005,0.0200]
uniform bool HoleSphere; checkbox[false]
// Number of fractal iterations.
uniform int Iterations;  slider[0,8,100]
uniform int ColorIterations;  slider[0,8,100]
uniform bool trapmode; checkbox[false]
#group Transforms
//uniform bool Polyhedronate;checkbox[false]
//uniform bool Stellate; checkbox[false]
//uniform bool StellateHedron; checkbox[false]
//uniform float starangle;slider[-3.1416,1.0,3.1416]
//uniform float starangle2;slider[-3.1416,1.10,3.1416]
//uniform bool test1; checkbox[false]
//uniform bool test2; checkbox[false]

//uniform bool bothWithSphericAndUnspheric;checkbox[true]

//set both sides to 3 for a tetra hedron, 4 for a cube, and that's it for the Platonics from this set
//uniform float sides;slider[2.0,6.0,15.0]
//uniform float sides2;slider[2.0,8.0,15.0]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float RotAngle; slider[0.00,0,180]
uniform int RotIter1;slider[0,3,20]
uniform int RotEverAfter;slider[0,9,20]


mat3 rot;

void init() {
	rot = rotationMatrix3(normalize(RotVector), RotAngle);
}


//uniform bool Stellate; checkbox[false]
//uniform bool StellateHedron; checkbox[false]
//set both sides to 3 for a tetra hedron, 4 for a cube, and that's it for the Platonics from this set
//uniform float sides;slider[2.0,6.0,15.0]
//uniform float sides2;slider[2.0,8.0,15.0]
//uniform float starangle;slider[-3.1416,1.0,3.1416]
//uniform float starangle2;slider[-3.1416,1.10,3.1416]




/*
void Stellation (inout vec3 z) {

	float rCyz=abs(atan(z.z,z.y));
	 float i=1.;
	while (rCyz>pi/sides && i<sides) {rCyz-=pi2/sides; i++;}
	rCyz=abs(rCyz);
		z.yz*=(cos(pi/sides*.5)*cos(rCyz-starangle))/cos(pi/(sides)*.5-starangle);
		float rCxyz;
		if (StellateHedron) {
	  rCxyz= abs(atan(sqrt(z.y*z.y+z.z*z.z),z.x));
		i=1.;
		while (rCxyz>pi/sides2 && i<sides2) {rCxyz-=pi2/sides2; i++;}
		rCxyz=abs(rCxyz);
		 z*=(cos(pi/sides2*.5)*cos(rCxyz-starangle2))/cos(pi/(sides2)*.5-starangle2);
		}
 rCyz= (z.y*z.y)/(z.z*z.z);

	if (rCyz<1.) {rCyz=sqrt(rCyz+1.);} else {rCyz=sqrt(1./rCyz+1.);}
 if (StellateHedron) {
		 rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
		if (rCxyz<1.) {rCxyz=sqrt(rCxyz+1.);} else {rCxyz=sqrt(1./rCxyz+1.);}
		z*=rCxyz;
	}
z.yz*=rCyz;
}
*/

uniform int orbititer1; slider[0,3,20]
uniform int orbititer1app; slider[0,3,20]
uniform int zlapp; slider[0,10,20]
uniform vec3 zorbit; slider[(-3.0,-3.0,-3.0),(0,0,0),(3.0,3.0,3.00)];
uniform vec3 zorbitApp; slider[(-3.0,-3.0,-3.0),(0,0,0),(3.0,3.0,3.00)];
uniform vec4 orbit1; slider[(-3.00,-3.00,-3.00,-3.00),(0.00,0.00,0.00,0.00),(3.00,3.00,3.00,3.00)]
uniform vec4 orbitApp1;slider[(-3.00,-3.00,-3.00,-3.00),(0,0,0,0),(3.00,3.00,3.00,3.00)]
uniform bool orbitCut;checkbox[true]
uniform vec4 orbitGap1; slider[(0.00,0.00,0.00,0.00),(0,0,0,0),(2.00,2.00,2.00,2.00)]
uniform vec4 orbitAdd1; slider[(-2.00,-2.00,-2.00,-2.00),(0,0,0,0),(2.00,2.00,2.00,2.00)]
uniform vec3 CutXYZmult; slider[(-1,-1,-1),(0,0,0),(1,1,1)];


#group HeightMap
uniform int HeightIter; slider[0,0,20]

uniform int HeightIter2; slider[0,0,20]

//uniform int HeightIter3; slider[0,0,20]
uniform sampler2D tex2;file[texture.jpg]
//sampler2d htex=tex2;
//uniform sampler2D htex2; file[../]
uniform float hTextSpeedMult; slider[.01,1.,20.]
uniform float hTextureSpeed; slider[-5.,1.,5.]
float htexturespeed=hTextureSpeed*hTextSpeedMult;
uniform float HighStrength; slider[-10,1,10]
float hintensity=HighStrength*.02;
uniform vec4 horbitTexX;slider[(-1,-1,-1,-1),(0,0,0,0),(1,1,1,1)]
uniform vec4 horbitTexY;slider[(-1,-1,-1,-1),(1,0,0,0),(1,1,1,1)]
uniform vec4 hscale;slider[(-3,-3,-3,-3),(0,0,0,0),(3,3,3,3)]


uniform vec2 hTextureOffset; slider[(-100,-100),(0,0),(100,100)]
vec2 htextoff=hTextureOffset/100.0;
uniform int hMapType; slider[0,0,13]

 vec4 hcolor = texture2D(tex2,htextoff)*hintensity;


uniform float htsides; slider[2.,4.,30.]
uniform float htsides2; slider[2.,4.,30.]
void polymaptexture (inout vec3 z, out vec2 hangles) {
	float rCyz=abs(atan(z.z,z.y));
	 float i=1.;
	while (rCyz>pi/htsides && i<htsides) {rCyz-=pi2/htsides; i++;}

		rCyz=.5*(rCyz/pi*htsides+1.);
	i=1.;
	 float rCxyz= abs(atan(sqrt(z.y*z.y+z.z*z.z),z.x));
	while (rCxyz>3.14159265359/htsides2 && i<htsides2)
			 {rCxyz-=6.28318530718/htsides2; i++;}
		i=1.;
	rCxyz=.5*(rCxyz/pi*htsides2+1.);
		hangles=vec2(rCyz,rCxyz);

}

void hTextureIT (inout vec3 z) {
		vec2 horbittotal;
 	// vec4 hcolor=vec4(0.,0.,0.,0.);
		vec2 hangles;

// idea for maptype 0 from:
// https://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Textured_Spheres
	if (hMapType==0) {
		hangles= vec2((atan(z.z, z.x) / 3.1415926 + 1.0) * 0.5,
                                  (asin(z.y) / 3.1415926 + 0.5));
		hcolor=texture2D(tex2,htextoff+(hangles)*htexturespeed)*hintensity;
	}    else if (hMapType==1) {
		polymaptexture(z,hangles);
		hcolor = texture2D(tex2,htextoff+hangles*htexturespeed)*hintensity;
	}   	else if (hMapType==2) {
			horbittotal=vec2(horbitTexX.x*orbitTrap.x+
				horbitTexX.y*orbitTrap.y+
				horbitTexX.z*orbitTrap.z+
				horbitTexX.w*orbitTrap.w,
				horbitTexY.x*orbitTrap.x+
				horbitTexY.y*orbitTrap.y+
				horbitTexY.z*orbitTrap.z+
				horbitTexY.w*orbitTrap.w
				);
		horbittotal=((htextoff+(horbittotal)*htexturespeed));


		 hcolor = texture2D(tex2,horbittotal)*hintensity;
	}	else if (hMapType==3) {
			horbittotal=vec2(horbitTexX.x*z.x+
				horbitTexX.y*z.y+
				horbitTexX.z*z.z+
				horbitTexX.w*length(z),horbitTexY.x*z.x+
				horbitTexY.y*z.y+
				horbitTexY.z*z.z+
				horbitTexY.w*length(z)
				);
		horbittotal=((htextoff+(horbittotal)*htexturespeed));


		 hcolor = texture2D(tex2,horbittotal)*hintensity;
	} else if (hMapType==4) {
  		hangles= vec2((atan(z.y, z.z) / 3.1415926 + 1.0) * 0.5,
                                  (atan(z.x,length(z.yz)) / 3.1415926 + 0.5));
		hcolor=texture2D(tex2,htextoff+(hangles)*htexturespeed)*hintensity;
	} else if (hMapType==5) {
   	hcolor = texture2D(tex2,htextoff+ (z.xz)*htexturespeed)*hintensity;
	} else if (hMapType==6) {
   	hcolor = texture2D(tex2,htextoff+ (z.xy)*htexturespeed)*hintensity;
	} else if (hMapType==7) {
   	hcolor = texture2D(tex2, htextoff+(z.yz)*htexturespeed)*hintensity;
	} else  if (hMapType==8) {
 		hcolor = texture2DProj (tex2, z*htexturespeed)*hintensity;
	} else if (hMapType==9) {
		 hcolor = texture2D(tex2,Dir.xy*htexturespeed)*hintensity;
	} else if (hMapType==10) {
 		hcolor = texture2D(tex2, coord*htexturespeed)*hintensity;
	}
	hcolor.w=length(hcolor.xyz);
	float hh=hcolor.x*hscale.x +
			hcolor.y*hscale.y +
			hcolor.z*hscale.z +
			 hscale.w*hcolor.w;
		z+=vec3(hh,hh,hh);

}


float DE(vec3 z)
{
	float t=9999.0;
	 float sc=Scale;
	 float sc1=sc-1.0;
	float sc2=sc1/sc;
	vec3 C=vec3(1.0,1.0,.5);
	float w=1.;
	int n = 1;

	float r=length(z);
	float r2=0.;

	if (trapmode) {orbitTrap=vec4((z),length(z));}
	vec4 orbitstore1=min(vec4(100,100,100,100),vec4(abs(z),length(z)));


	// if (Stellate) Stellation(z);

	while (n <= Iterations) {


		if (n==RotIter1 || n>=RotEverAfter) {
					z*=rot; }
			if (n==HeightIter  || n==HeightIter2) {
				hTextureIT(z);
			}

			z = vec3(sqrt(z.x*z.x+s),sqrt(z.y*z.y+s),sqrt(z.z*z.z+s));

			t=z.x-z.y;  t= .5*(t-sqrt(t*t+s));
			z.x=z.x-t;	z.y=z.y+t;

			t=z.x-z.z; t= 0.5*(t-sqrt(t*t+s));
  			z.x=z.x-t;	 z.z= z.z+t;

			t=z.y-z.z;  t= 0.5*(t-sqrt(t*t+s));
  			z.y=z.y-t;  z.z= z.z+t;

		z.z = z.z-C.z*sc2;
		z.z=-sqrt(z.z*z.z+s);
		z.z=z.z+C.z*sc2;

			z.x=sc*z.x-C.x*sc1;
			z.y=sc*z.y-C.y*sc1;
			z.z=sc*z.z;

		w=w*sc;

		if (n==zlapp) {
			r=length (vec3(
				z.x*zorbit.x,
				z.y*zorbit.y,
				z.z*zorbit.z
				));
			z=vec3(z.x+zorbitApp.x*r,
					z.y+zorbitApp.y*r,
					z.z+zorbitApp.z*r);
		}
		if (n==orbititer1app) {
			orbitstore1=vec4(
				orbitstore1.x*orbit1.x,
				orbitstore1.y*orbit1.y,
				orbitstore1.z*orbit1.z,
				length(orbitstore1)*orbit1.w);
				r=length(orbitstore1);
			if (orbitCut) {
				r2=orbitGap1.x+orbitGap1.y+orbitGap1.z+orbitGap1.w;
				while (r>r2  && r2!=0.) {
					r-=r2; }
				if (r<orbitGap1.x) {
					z=vec3(z.x+CutXYZmult.x*orbitAdd1.x,
							z.y+CutXYZmult.y*orbitAdd1.x,
							z.z+CutXYZmult.z*orbitAdd1.x);
				} else if (r<orbitGap1.x+orbitGap1.y) {
							z=vec3(z.x+CutXYZmult.x*orbitAdd1.y,
							z.y+CutXYZmult.y*orbitAdd1.y,
							z.z+CutXYZmult.z*orbitAdd1.y);
				} else if (r<orbitGap1.x+orbitGap1.y+orbitGap1.z) {
							z=vec3(z.x+CutXYZmult.x*orbitAdd1.z,
							z.y+CutXYZmult.y*orbitAdd1.z,
							z.z+CutXYZmult.z*orbitAdd1.z);
				} else {
							z=vec3(z.x+CutXYZmult.x*orbitAdd1.w,
							z.y+CutXYZmult.y*orbitAdd1.w,
							z.z+CutXYZmult.z*orbitAdd1.w);
				}
			} else {
				z=vec3(z.x+(orbitApp1.x+orbitApp1.w)*r,
				z.y+(orbitApp1.y+orbitApp1.w)*r,
				z.z+(orbitApp1.z+orbitApp1.w)*r);
			}
		}

		if (n<=orbititer1) {
			orbitstore1=min(orbitstore1, (vec4(abs(z),length(z))));
			//orbitstore1=vec4(abs(z),length(z));
		}
		if (n<=ColorIterations) orbitTrap = min(orbitTrap, (vec4(abs(z),length(z))));
		if (n==ColorIterations && orbitBoolSet) {n=Iterations;}
		n++;
	}
	return abs(length(z)-0.0 ) /w;
}

float dummy(vec3 p){
p*=time;
return time;
}

#preset Default
AutoFocus = false
FOV = 0.4
Eye = 0.547457993,1.27550578,-0.44279784
Target = -3.90192628,-2.3278079,3.51672554
Up = -0.345633477,0.85402447,0.388799638
EquiRectangular = false
FocalPlane = 1
Aperture = 0
InFocusAWidth = 0
ApertureNbrSides = 5
ApertureRot = 0
ApStarShaped = false
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
Bloom = false
BloomIntensity = 0.25
BloomPow = 2
BloomTaps = 4
LensFlare = false
FlareIntensity = 0.25
FlareSamples = 8
FlareDispersal = 0.25
FlareHaloWidth = 0.5
FlareDistortion = 1
DepthToAlpha = false
Detail = -2.3
DetailAO = -0.5
FudgeFactor = 1
MaxRaySteps = 56
MaxDistance = 20
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
AoCorrect = 0
Specular = 0.4
SpecularExp = 16
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Reflection = 1,1,1
ReflectionsNumber = 0
SpotGlow = true
SpotLight = 1,1,1,1
LightPos = 0,0,0
LightSize = 0.1
LightFallOff = 0
LightGlowRad = 0
LightGlowExp = 1
HardShadow = 0
ShadowSoft = 10
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.600000024,0.600000024,0.7
Y = 1,0.600000024,0,0.4
Z = 0.800000012,0.779999971,1,0.5
R = 0.400000006,0.699999988,1,0.12
BackgroundColor = 0.600000024,0.600000024,0.449999988
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,1
FloorHeight = 0
FloorColor = 1,1,1
HF_Fallof = 0.1
HF_Const = 0
HF_Intensity = 0
HF_Dir = 0,0,1
HF_Offset = 0
HF_Color = 1,1,1,1
HF_Scatter = 0
HF_Anisotropy = 0,0,0
HF_FogIter = 1
HF_CastShadow = false
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 10
pOffset = 0
color0 = 0.949999988,0.829999983,0.419999987
Sharp0to1 = false
Dist0to1 = 1
color1 = 1,0,0.0700000003
Sharp1to2 = false
Dist1to2 = 1
color2 = 0.699999988,0.699999988,0.419999987
Sharp2to3 = false
Dist2to3 = 1
color3 = 1,0.370000005,0
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 1,1,1,1
ifTexture = false
tex = /home/toonfish/Fragmentarium/Examples/Include/texture.jpg
TextSpeedMult = 1
TextureSpeed = 1
intensity = 2.5
orbitTexX = 0,0,0,0
orbitTexY = 1,0,0,0
TextureOffset = 0,0
MapType = 1
testA = false
tsides = 4
tsides2 = 4
ThetaSteps = 7
PhiSteps = 7
ConeWidth = 6.2831853
InteriorIntensity = 1
InteriorWeight = 0.5
PostWeight = 0
PostIntensity = 1
allsample = false
normalColor = true
colorDepth = 0
SampleStepsShort = 1
SampleDepthShort = 0
LongFirst = false
SampleStepsLong = 0
SampleDepthLong = 0
CloudScale = 1
CloudFlatness = 0
CloudTops = 1
CloudBase = -1
CloudDensity = 1
CloudRoughness = 1
CloudContrast = 1
CloudColor = 0.649999976,0.680000007,0.699999988
SunLightColor = 0.699999988,0.5,0.300000012
WindDir = 0,0,1
WindSpeed = 1
Scale = 3
s = 0.005
HoleSphere = false
Iterations = 8
ColorIterations = 8
trapmode = false
RotVector = 1,1,1
RotAngle = 0
RotIter1 = 3
RotEverAfter = 9
orbititer1 = 3
orbititer1app = 3
zlapp = 10
zorbit = 0,0,0
zorbitApp = 0,0,0
orbit1 = 0,0,0,0
orbitApp1 = 0,0,0,0
orbitCut = true
orbitGap1 = 0,0,0,0
orbitAdd1 = 0,0,0,0
CutXYZmult = 0,0,0
HeightIter = 4
HeightIter2 = 4
tex2 = /home/toonfish/Fragmentarium/Examples/Include/texture.jpg
hTextSpeedMult = 8.92697736
hTextureSpeed = -0.28871391
HighStrength = 6.37075719
horbitTexX = 0,0,0,0
horbitTexY = 1,0,0,0
hscale = 0,0,0,0
hTextureOffset = 0,0
hMapType = 0
htsides = 4
htsides2 = 4
#endpreset

#preset Lightning type
FOV = 0.4
Eye = 1.3846,0.3134653,-3.31864
Target = -1.697171,-0.6331271,2.894785
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.3
DetailAO = -0.4
FudgeFactor = 0.3
MaxRaySteps = 221
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.2235294,0.3333333,0.3843137
GradientBackground = 0.5
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 10
pOffset = 0
color0 = 0.95,0.83,0.42
Sharp0to1 = false
Dist0to1 = 1
color1 = 1,0,0.07
Sharp1to2 = false
Dist1to2 = 1
color2 = 0.7,0.7,0.42
Sharp2to3 = false
Dist2to3 = 1
color3 = 1,0.37,0
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 1,1,1,1
ifTexture = true
tex = MB-FS-tex/glitter_glow_texture_ix_by_hauntingmewithstock.jpg
TextSpeedMult = 1
TextureSpeed = 1
intensity = 2.6
orbitTexX = -0.081,0,0.02,0
orbitTexY = 0.2,-0.3,-0.1,0
TextureOffset = 48.3871,0
MapType = 1
Scale = 3
HoleSphere = false
Iterations = 8
ColorIterations = 3
Polyhedronate = false
Stellate = false
StellateHedron = false
starangle = -0.2026839
starangle2 = -0.2244
test1 = false
test2 = false
bothWithSphericAndUnspheric = false
sides = 10
sides2 = 4
RotVector = 1,0,0
RotAngle = 0
RotIter1 = 4
RotEverAfter = 4
orbititer1 = 0
orbititer1app = 2
s = 0
orbit1 = 0,0,0.5,0
orbitApp1 = -1,-1,0,0
orbitCut = true
orbitGap1 = 0,0,0,1
orbitAdd1 = 0,0,1,0
Up = 0.023573,0.9865118,0.1619834
#endpreset


//multisample has palette setup too... not the greatest.. but what the hey
// so can check the texture textbox if you want the texture... but on palette now

#preset Multisample
FOV = 0.4
Eye = -2.010359,-0.1402999,-1.995796
Target = 2.903074,0.5677521,2.939467
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.3
DetailAO = -0.4
FudgeFactor = 0.2
MaxRaySteps = 221
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.2235294,0.3333333,0.3843137
GradientBackground = 0.5
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 10
pOffset = 0
pIntensity = 1
color0 = 0.5137255,0.4705882,0.4156863
Sharp0to1 = false
Dist0to1 = 0.0422535
color1 = 0.3372549,0.5803922,0.345098
Sharp1to2 = false
Dist1to2 = 0.2112676
color2 = 0.7333333,0.745098,0.6392157
Sharp2to3 = false
Dist2to3 = 1
color3 = 0.6039216,0.345098,0.02352941
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 1.666667,1,1.333333,0
ifTexture = false
tex = texture.jpg
TextSpeedMult = 1
TextureSpeed = 1
intensity = 2.6
orbitTexX = -0.081,0,0.02,0
orbitTexY = 0.2,-0.3,-0.1,0
TextureOffset = 44.3871,1
MapType = 2
testA = false
tsides = 4
tsides2 = 4
ThetaSteps = 5
PhiSteps = 5
ConeWidth = 3
InteriorIntensity = 1.3
InteriorWeight = 0.5
PostWeight = 0.86
PostIntensity = 1.6
allsample = false
normalColor = false
colorDepth = 0.1
Scale = 3
s = 0
HoleSphere = false
Iterations = 8
ColorIterations = 3
trapmode = false
RotVector = 1,0,0
RotAngle = 36
RotIter1 = 9
RotEverAfter = 9
orbititer1 = 4
orbititer1app = 1
zlapp = 1
zorbit = -0.975,-0.45,0
zorbitApp = 2,1,0
orbit1 = 0,3,-3,0
orbitApp1 = 0,0,0,0
orbitCut = true
orbitGap1 = 0.5,0.5,0.5,0.5
orbitAdd1 = 2,2,2,0
CutXYZmult = 0,0,-0.2307692
HeightIter = 0
HeightIter2 = 0
tex2 = texture2.jpg
hTextSpeedMult = 1
hTextureSpeed = 1
HighStrength = 1
horbitTexX = 0,0,0,0
horbitTexY = 1,0,0,0
hscale = 0,0,0,0
hTextureOffset = 0,0
hMapType = 0
htsides = 4
htsides2 = 4
SampleStepsShort = 3
SampleDepthShort = 0.008
LongFirst = true
SampleStepsLong = 1
SampleDepthLong = 0.085
Up = -0.0493508,0.9943924,-0.0935309
#endpreset

#preset newcolor
FOV = 0.4
Eye = -1.626183,-2.446988,-1.576882
Target = -0.9232691,-1.534037,-1.003499
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3
DetailAO = -0.5
FudgeFactor = 0.4
MaxRaySteps = 111
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.6
Specular = 2.2
SpecularExp = 31.37255
SpotLight = 0.8941176,1,0.8235294,0.5319149
SpotLightDir = -0.2,1
CamLight = 0.8980392,0.8980392,0.8980392,1.4
CamLightMin = 0.0645161
Glow = 1,1,1,0.0144928
GlowMax = 17
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.3571429
X = 0.3098039,0.6,0.6784314,0.7
Y = 0.05490196,0.7803922,1,0.4
Z = 0.7019608,1,0.8392157,0.5
R = 0.827451,0.654902,1,0.0617284
BackgroundColor = 0.01960784,0.01176471,0.05882353
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0.9568627,0.9568627,0.9568627
BaseStrength = 0
cSpeed = 15.97222
pOffset = 82.8125
color0 = 0.1411765,0,0.9490196
Sharp0to1 = false
Dist0to1 = 1
color1 = 0.2313725,1,0
Sharp1to2 = false
Dist1to2 = 1
color2 = 0.02352941,0.9098039,0.7764706
Sharp2to3 = false
Dist2to3 = 1
color3 = 0.145098,1,0.2
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 0.6666667,1.666667,1.333333,1
ifTexture = true
tex = texture.jpg
TextSpeedMult = 1
TextureSpeed = 2
intensity = 2.5
orbitTexX = 0.1290323,0.4516129,0.3225806,0.5483871
orbitTexY = 0.3225806,0.2580645,-0.0322581,0.1290323
TextureOffset = 41.93548,35.48387
MapType = 6
Scale = 3
s = 0
HoleSphere = false
Iterations = 8
ColorIterations = 3
Polyhedronate = false
Stellate = false
StellateHedron = false
starangle = 1
starangle2 = 1
test1 = false
test2 = false
bothWithSphericAndUnspheric = true
sides = 3
sides2 = 3
RotVector = 1,0,0
RotAngle = 0
RotIter1 = 3
RotEverAfter = 9
Up = 0.2296935,0.2537297,-0.6855762
#endpreset

#preset CitadelMenger
FOV = 0.4
Eye = -2.220935,-0.1706449,-2.207307
Target = 2.692498,0.5374071,2.727956
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.3
DetailAO = -0.4
FudgeFactor = 0.2
MaxRaySteps = 221
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.2235294,0.3333333,0.3843137
GradientBackground = 0.5
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 10
pOffset = 0
pIntensity = 1
color0 = 0.5137255,0.4705882,0.4156863
Sharp0to1 = false
Dist0to1 = 0.0422535
color1 = 0.3372549,0.5803922,0.345098
Sharp1to2 = false
Dist1to2 = 0.2112676
color2 = 0.7333333,0.745098,0.6392157
Sharp2to3 = false
Dist2to3 = 1
color3 = 0.6039216,0.345098,0.02352941
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 1.666667,1,1.333333,0
ifTexture = true
tex = texture.jpg
TextSpeedMult = 1
TextureSpeed = 1
intensity = 2.6
orbitTexX = -0.081,0,0.02,0
orbitTexY = 0.2,-0.3,-0.1,0
TextureOffset = 44.3871,1
MapType = 2
testA = false
tsides = 4
tsides2 = 4
ThetaSteps = 5
PhiSteps = 5
ConeWidth = 3
InteriorIntensity = 1.3
InteriorWeight = 0.5
PostWeight = 0.86
PostIntensity = 1.6
allsample = false
normalColor = false
colorDepth = 0.1
Scale = 3
s = 0
HoleSphere = false
Iterations = 8
ColorIterations = 3
trapmode = false
RotVector = 1,0,0
RotAngle = 16
RotIter1 = 10
RotEverAfter = 10
orbititer1 = 1
orbititer1app = 3
zlapp = 1
zorbit = -0.975,-0.2,0
zorbitApp = 2,1,-1
orbit1 = 1,1,1,0
orbitApp1 = -0.3,-0.2,-0.3,0
orbitCut = false
orbitGap1 = 0.5,0.5,0.5,0.5
orbitAdd1 = 2,2,2,0
CutXYZmult = 0,0,-0.2307692
HeightIter = 0
HeightIter2 = 0
tex2 = texture2.jpg
hTextSpeedMult = 1
hTextureSpeed = 1
HighStrength = 1
horbitTexX = 0,0,0,0
horbitTexY = 1,0,0,0
hscale = 0,0,0,0
hTextureOffset = 0,0
hMapType = 0
htsides = 4
htsides2 = 4
SampleStepsShort = 3
SampleDepthShort = 0.008
LongFirst = true
SampleStepsLong = 1
SampleDepthLong = 0.085
Up = -0.0493508,0.9943924,-0.0935309
#endpreset

#preset smoothmenger
FOV = 0.4
Eye = 2.976287,-0.5031761,-1.760986
Target = -3.047907,0.7115838,1.590645
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.3
DetailAO = -0.4
FudgeFactor = 0.3
MaxRaySteps = 221
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.2235294,0.3333333,0.3843137
GradientBackground = 0.5
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 10
pOffset = 0
color0 = 0.95,0.83,0.42
Sharp0to1 = false
Dist0to1 = 1
color1 = 1,0,0.07
Sharp1to2 = false
Dist1to2 = 1
color2 = 0.7,0.7,0.42
Sharp2to3 = false
Dist2to3 = 1
color3 = 1,0.37,0
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 1,1,1,1
ifTexture = true
tex = texture.jpg
TextSpeedMult = 1
TextureSpeed = 1
intensity = 2.6
orbitTexX = -0.081,0,0.02,0
orbitTexY = 0.2,-0.3,-0.1,0
TextureOffset = 48.3871,0
MapType = 1
Scale = 3
HoleSphere = false
Iterations = 8
ColorIterations = 3
Polyhedronate = false
Stellate = false
StellateHedron = false
starangle = 1
starangle2 = 0
test1 = false
test2 = false
bothWithSphericAndUnspheric = false
sides = 7
sides2 = 4
RotVector = 0,0,1
RotAngle = 80
RotIter1 = 9
RotEverAfter = 9
orbititer1 = 0
orbititer1app = 1
s = 0
orbit1 = 0.2658228,0.5063291,-0.1139241,-0.1139241
orbitApp1 = 0,-1,-1,1
orbitCut = false
orbitGap1 = 0,0,0,1
orbitAdd1 = 0,0,1,0
Up = 0.0896531,0.9770952,-0.192995
#endpreset

#preset organometalloid
FOV = 0.4
Eye = 2.399813,0.3752105,-1.648577
Target = -3.01216,-0.6568988,2.669441
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.3
DetailAO = -0.4
FudgeFactor = 0.2
MaxRaySteps = 221
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.2235294,0.3333333,0.3843137
GradientBackground = 0.5
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 10
pOffset = 0
color0 = 0.95,0.83,0.42
Sharp0to1 = false
Dist0to1 = 1
color1 = 1,0,0.07
Sharp1to2 = false
Dist1to2 = 1
color2 = 0.7,0.7,0.42
Sharp2to3 = false
Dist2to3 = 1
color3 = 1,0.37,0
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 1,1,1,1
ifTexture = true
tex = texture.jpg
TextSpeedMult = 1
TextureSpeed = 1
intensity = 2.6
orbitTexX = -0.081,0,0.02,0
orbitTexY = 0.2,-0.3,-0.1,0
TextureOffset = 48.3871,0
MapType = 1
testA = false
Scale = 3
s = 0
HoleSphere = false
Iterations = 8
ColorIterations = 3
RotVector = 1,0,0
RotAngle = 36
RotIter1 = 9
RotEverAfter = 9
orbititer1 = 1
orbititer1app = 2
zlapp = 1
zorbit = -0.975,-0.45,0
zorbitApp = 2,1,-0.2295082
orbit1 = 1.936709,-0.1898734,0.2658228,0
orbitApp1 = -0.5,1.4,-0.6,0
orbitCut = false
orbitGap1 = 0.5,0.5,0.5,0.5
orbitAdd1 = 2,2,2,2
CutXYZmult = 0,0,-0.1923077
Up = -0.0826471,0.9877304,0.1325056
#endpreset

#preset heightmap
FOV = 0.4
Eye = -0.3282062,-0.417313,-3.598835
Target = 0.7690911,0.6648888,3.229397
Detail = -3.3
FudgeFactor = 0.2
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.2235294,0.3333333,0.3843137
GradientBackground = 0.5
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 10
pOffset = 0
color0 = 0.95,0.83,0.42
Sharp0to1 = false
Dist0to1 = 1
color1 = 1,0,0.07
Sharp1to2 = false
Dist1to2 = 1
color2 = 0.7,0.7,0.42
Sharp2to3 = false
Dist2to3 = 1
color3 = 1,0.37,0
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 1,1,1,1
ifTexture = true
tex = texture.jpg
TextSpeedMult = 1
TextureSpeed = 1
intensity = 2.6
orbitTexX = -0.081,0,0.02,0
orbitTexY = 0.2,-0.3,-0.1,0
TextureOffset = 48.3871,0
MapType = 1
testA = false
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
DetailAO = -0.4
MaxRaySteps = 221
Scale = 3
s = 0
HoleSphere = false
Iterations = 8
ColorIterations = 3
RotVector = 1,0,0
RotAngle = 36
RotIter1 = 9
RotEverAfter = 9
orbititer1 = 4
orbititer1app = 0
zlapp = 0
zorbit = -0.975,-0.45,0
zorbitApp = 2,1,0
orbit1 = 0,3,-3,0
orbitApp1 = 0,0,0,0
orbitCut = true
orbitGap1 = 0.5,0.5,0.5,0.5
orbitAdd1 = 2,2,2,0
CutXYZmult = 0,0,-0.2307692
HeightIter = 4
HeightIter2 = 0
hTextSpeedMult = 1
hTextureSpeed = 1
hintensity = 2.5
horbitTexX = -0.081,0,0,0
horbitTexY = 0.2,-0.3,-0.1,0
hscale = 0.6623377,0.2727273,-0.1168831,-0.5064935
hTextureOffset = 55,0
hMapType = 1
htestA = false
Up = -0.1098654,0.9842718,-0.1383412
#endpreset

#preset Wow
FOV = 0.4
Eye = 0.099698,-0.0500547,-0.4952366
Target = -3.813296,0.0761789,5.307569
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.3
DetailAO = -0.4
FudgeFactor = 0.3
MaxRaySteps = 221
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.6060606
Specular = 4
SpecularExp = 16
SpotLight = 1,1,1,0.4
SpotLightDir = 0.1,0.1
CamLight = 1,1,1,1.837838
CamLightMin = 0.1568627
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.2235294,0.3333333,0.3843137
GradientBackground = 0.5
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 10
pOffset = 0
color0 = 0.95,0.83,0.42
Sharp0to1 = false
Dist0to1 = 1
color1 = 1,0,0.07
Sharp1to2 = false
Dist1to2 = 1
color2 = 0.7,0.7,0.42
Sharp2to3 = false
Dist2to3 = 1
color3 = 1,0.37,0
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 1,1,1,1
ifTexture = true
tex = texture.jpg
TextSpeedMult = 1
TextureSpeed = 1
intensity = 2.6
orbitTexX = -0.081,0,0.02,0
orbitTexY = 0.2,-0.3,-0.1,-0.1935484
TextureOffset = 48.3871,62.96296
MapType = 2
testA = false
tsides = 4
tsides2 = 4
Giter = 3
Samples = 3
ThetaPhiSteps = 8
backStep = 0.01
mindistmod = 0
oldColor = false
correctreflect = false
FS2Weight = 16.27907
Scale = 3
s = 0
HoleSphere = false
Iterations = 9
ColorIterations = 3
trapmode = false
RotVector = 1,0,0
RotAngle = 0
RotIter1 = 4
RotEverAfter = 4
orbititer1 = 0
orbititer1app = 2
zlapp = 10
zorbit = 0,0,0
zorbitApp = 0,0,0
orbit1 = 0,0,0.5,0
orbitApp1 = -1,-1,0,0
orbitCut = true
orbitGap1 = 0,0,0,1
orbitAdd1 = 0,0,1,0
CutXYZmult = 0,0,0
HeightIter = 0
HeightIter2 = 0
tex2 = texture2.jpg
hTextSpeedMult = 1
hTextureSpeed = 1
HighStrength = 1
horbitTexX = 0,0,0,0
horbitTexY = 1,0,0,0
hscale = 0,0,0,0
hTextureOffset = 0,0
hMapType = 0
htsides = 4
htsides2 = 4
test = true
Distadd = 0.1257067
Up = 0.0798876,0.9962836,0.0321974
ConeWidth = 6.283185
#endpreset


/// zorbit2:Linear:0:-0.451172:0.0351562:1:120:0.3:1:1.7:1:1


#preset KeyFrame.001
FOV = 0.4
Eye = -0.0619155,-0.0341772,-0.8464459
Target = -0.5066848,2.054669,5.817978
Up = -0.1637892,0.9381678,-0.304983
#endpreset

#preset KeyFrame.002
FOV = 0.4
Eye = 0.1315935,0.0661878,-0.3964669
Target = 2.520877,1.802637,5.947957
Up = -0.2893614,0.9454279,-0.1497885
#endpreset
