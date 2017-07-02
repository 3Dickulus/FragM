#version 120
#info Mandelbulb Distance Estimator  Benesi Mod

//quick note:  changed the rotation to inside the feedback loop for fun...
//also, use with a modified Fast-Raytracer.frag for better coloring.

#define providesInit
#include "MathUtils.frag"
#include "Fast-Raytracer.frag"
//#include "Fast-Raytracer-with-Textures.frag"


#group Mandelbulb

// Number of fractal iterations.
uniform int Iterations;  slider[0,9,100]

//if color iteration is 0, takes the normal minimum orbit of all iterations.  If it is set to a specific number, takes the orbit of that iteration only.
uniform int ColorIteration;  slider[0,0,100]

// Mandelbulb exponent (8 is standard)
uniform float Power; slider[0,8,16]

// Bailout radius
uniform float Bailout; slider[0,5,30]

// Alternate is slightly different, but looks more like a Mandelbrot for Power=2
uniform bool AlternateVersion; checkbox[false]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]

uniform float RotAngle; slider[0.00,0,180]

uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-2,-2,-2),(0,0,0),(2,2,2)]

mat3 rot;
uniform float time;

void init() {
	 rot = rotationMatrix3(normalize(RotVector), RotAngle);

       float dummy = time*10.0;
}

// This is my power function, based on the standard spherical coordinates as defined here:
// http://en.wikipedia.org/wiki/Spherical_coordinate_system
//
// It seems to be similar to the one Quilez uses:
// http://www.iquilezles.org/www/articles/mandelbulb/mandelbulb.htm
//
// Notice the north and south poles are different here.
void powN1(inout vec3 z, float r, inout float dr) {
	// extract polar coordinates
	float theta = acos(z.z/r);
	float phi = atan(z.y,z.x);
	dr =  pow( r, Power-1.0)*Power*dr + 1.0;

	// scale and rotate the point
	float zr = pow( r,Power);
	theta = theta*Power;
	phi = phi*Power;

	// convert back to cartesian coordinates
	z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
}

// This is a power function taken from the implementation by Enforcer:
// http://www.fractalforums.com/mandelbulb-implementation/realtime-renderingoptimisations/
//
// I cannot follow its derivation from spherical coordinates,
// but it does give a nice mandelbrot like object for Power=2
void powN2(inout vec3 z, float zr0, inout float dr) {
	float zo0 = asin( z.z/zr0 );
	float zi0 = atan( z.y,z.x );
	float zr = pow( zr0, Power-1.0 );
	float zo = zo0 * Power;
	float zi = zi0 * Power;
	dr = zr*dr*Power + 1.0;
	zr *= zr0;
	z  = zr*vec3( cos(zo)*cos(zi), cos(zo)*sin(zi), sin(zo) );
}


#group Feedback
// math 1 = mandy, 2= menger, 3 = pine tree, 4= T1, 5= T4,
// 6= skews (like in Menger), 7=offset addin, 8= zappa!$! zappa powah!)

uniform vec3 feedbackcrds[102];
uniform vec4 feedbackcontrol1[102];
uniform vec4 feedbackcontrol2[102];
uniform vec4 feedbackrotation[102];

//feedbackcontrol1.x[]
uniform int ApplyOnIteration;slider[0,0,30]
//feedbackcontrol1.y[]
uniform int FormulaType;slider[1,1,30]
//feedbackcontrol1.z[]
uniform int ApplicationType;slider[1,1,30]


//feedbackcontrol2.x[]
uniform float FeedbackRadius;slider[0,.25,5]
//feedbackcontrol2.y[]
uniform float FeedbackStrength;slider[-10.,.2,10.]




//feedbackrotation.xyz
uniform vec3 FBRotVector; slider[(0,0,0),(1,1,1),(1,1,1)]
//feedbackrotation.w
uniform float FBRotAngle; slider[-180.00,0,180]


uniform int feedbackcount;
uniform int FeedBackCutOff;slider[8,99,102]

bool feedbackbool=true;

float DE(vec3 pos) {
	int feedbackstop=int(feedbackcount);
	if (feedbackstop>FeedBackCutOff) {
			feedbackstop=FeedBackCutOff;
	}
	//if (!spheremode) {}
		orbitTrap=min(vec4(pos,length(pos)),
									vec4(10.,10.,10.,10.));



vec3 z=pos;


	vec3 z2=z;
	float dr=1.0;
	float dr2=1.0;

		float zappaz;
		int i=0;
		int fi=0;

//fi=72;
//if (feedbackcrds[fi].x!=333.0) {
//	z+=feedbackcrds[fi].xyz*2;   //boom  hahahahah@!$!$@  Don't do too many feedbacks
//}

	float r;
	r=length(z);
	while(r<Bailout && (i<Iterations)) {
		fi=0;

		while (fi<=feedbackstop) {
			if (feedbackcontrol1[fi].x==i) {   //for applying by iteration
				zappaz=length (pos-feedbackcrds[fi].xyz);
				if (zappaz<feedbackcontrol2[fi].x) {

					if (feedbackcontrol1[fi].y==1.) {
						zappaz= (feedbackcontrol2[fi].x-zappaz)*feedbackcontrol2[fi].y;
						if (feedbackcontrol1[fi].z==1.) {
							z+=feedbackcrds[fi].xyz*zappaz;
						} else if (feedbackcontrol1[fi].z==2.) {
							zappaz=length(feedbackcrds[fi].xyz)*zappaz;
							z+=vec3(zappaz,zappaz,zappaz);
						} else if (feedbackcontrol1[fi].z==3.) {
							zappaz=length(feedbackcrds[fi].xyz)*zappaz;
							z.x+=zappaz;
						}  else if (feedbackcontrol1[fi].z==4.) {
							zappaz=length(feedbackcrds[fi].xyz)*zappaz;
							z.y+=zappaz;
						}  else if (feedbackcontrol1[fi].z==5.) {
							zappaz=length(feedbackcrds[fi].xyz)*zappaz;
							z.z+=zappaz;
						}  else if (feedbackcontrol1[fi].z==6.) {
							zappaz=length(feedbackcrds[fi].xyz)*zappaz;
							z.xy+=vec2(zappaz,zappaz);
						}  else if (feedbackcontrol1[fi].z==7.) {
							zappaz=length(feedbackcrds[fi].xyz)*zappaz;
							z.yz+=vec2(zappaz,zappaz);
						}  else if (feedbackcontrol1[fi].z==8.) {
							zappaz=length(feedbackcrds[fi].xyz)*zappaz;
							z.xz+=vec2(zappaz,zappaz);
						} else if (feedbackcontrol1[fi].z==9.) {
							z.x+=feedbackcrds[fi].x*zappaz;
						} else if (feedbackcontrol1[fi].z==10.) {
							z.y+=feedbackcrds[fi].y*zappaz;
						} else if (feedbackcontrol1[fi].z==11.) {
							z.z+=feedbackcrds[fi].z*zappaz;
						}
					} else if (feedbackcontrol1[fi].y==2.) {
						zappaz= (feedbackcontrol2[fi].x-zappaz)*feedbackcontrol2[fi].y;

						if (feedbackcontrol1[fi].z==1.) {
							z=(pos-feedbackcrds[fi].xyz)*zappaz;
						} else if (feedbackcontrol1[fi].z==2.) {
							z+=(pos-feedbackcrds[fi].xyz)*zappaz;
						} else if (feedbackcontrol1[fi].z==3.) {
							z*=(pos-feedbackcrds[fi].xyz)*zappaz;
						}
						z*=rot;
					} else if (feedbackcontrol1[fi].y==3.) {
						zappaz= (feedbackcontrol2[fi].x-zappaz)*feedbackcontrol2[fi].y;
						if (feedbackcontrol1[fi].z==1.) {
							z*= rotationMatrix3(normalize(feedbackrotation[fi].xyz),feedbackrotation[fi].w*zappaz);
						} else if (feedbackcontrol1[fi].z==2.) {
							if (zappaz<0) {zappaz=-(feedbackrotation[fi].w-zappaz);} else {zappaz+=feedbackrotation[fi].w;}
							z*= rotationMatrix3(normalize(feedbackrotation[fi].xyz),zappaz);
						} else if (feedbackcontrol1[fi].z==3.) {
							if (zappaz<0) {zappaz=zappaz-.1;} else {zappaz=zappaz+.1;}
							z*= rotationMatrix3(normalize(feedbackrotation[fi].xyz),feedbackrotation[fi].w/zappaz);
						}

					}	 else if (feedbackcontrol1[fi].y==4.) {
						zappaz= (feedbackcontrol2[fi].x-zappaz)*feedbackcontrol2[fi].y;
						if (feedbackcontrol1[fi].z==1.) {
							z*= rotationMatrix3(normalize(feedbackrotation[fi].xyz),feedbackrotation[fi].w*zappaz);
						} else if (feedbackcontrol1[fi].z==2.) {
							if (zappaz<0) {zappaz=-(feedbackrotation[fi].w-zappaz);} else {zappaz+=feedbackrotation[fi].w;}
							z*= rotationMatrix3(normalize(feedbackrotation[fi].xyz),zappaz);
						} else if (feedbackcontrol1[fi].z==3.) {
							if (zappaz<0) {zappaz=zappaz-.1;} else {zappaz=zappaz+.1;}
							z*= rotationMatrix3(normalize(feedbackrotation[fi].xyz),feedbackrotation[fi].w/zappaz);
						}

					}
				}  //zapparadius check
			}   //feedbackcontrol iteration check
			fi++;
		} // end of while (fi<FeedBacks)

		if (AlternateVersion) {
			powN2(z,r,dr);
		} else {
			powN1(z,r,dr);
		}
		z+=(Julia ? JuliaC : pos);
		r=length(z);
	//	z*=rot;
		if (ColorIteration==0  || ColorIteration>=Iterations) {
					orbitTrap=min(orbitTrap,vec4(z.x*z.x,z.y*z.y,z.z*z.z,dot(z,z)));
				} else if (i==ColorIteration) {
					orbitTrap=vec4(z,length(z));
				}

	i++;
	}
	return 0.5*log(r)*r/dr;

}

#group ExtraVariables
//feedbackcontrol1.w[]
uniform float FeedbackVariable1;slider[-10,0.,10]
//feedbackcontrol2.z[]
uniform float FeedbackVariable2;slider[-10,0.,10]
//feedbackcontrol2.w[]
uniform float FeedbackVariable3;slider[-10,0.,10]
//and put a few happy little arrays over here

#preset Default
FOV = 0.4
Eye = 2.253022,-0.0255647,-1.487696
Target = -3.588144,0.0407131,2.369295
DepthToAlpha = true
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3
DetailAO = -0.5
FudgeFactor = 0.9
MaxRaySteps = 111
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
LegacyColor = false
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
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
PalettePercent = 0
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 10
pOffset = 0
pIntensity = 1
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
BaseColorTex = 1
tex1 = texture.jpg
tex2 = texture2.jpg
tex3 = vigga.jpg
tex4 = Ditch-River_Env.hdr
SpeedMult = 1
TextureSpeed = 1
intensity = 2.5
orbitTexX = 0,0,0,0.1
orbitTexY = 0,0,0,1
TextureOffset = 76,28
BaseMapType = 2
testA = false
tsides = 4
tsides2 = 4
HeightMapType = 0
HeightMap1Tex = 1
HeightIter = 0
HighStrength1 = 1
hTextureOffset1 = 0,0
hTextureSpeed1 = 1
HeightMap2Tex = 1
HeightIter2 = 0
HighStrength2 = 1
hTextureOffset2 = 0,0
hTextureSpeed2 = 1
HeightMap3Tex = 1
HeightIter3 = 0
HighStrength3 = 1
hTextureOffset3 = 0,0
hTextureSpeed3 = 1
HeightAngle3 = 3.14
HeightVector3 = 1,0,0
HeightStart3 = -1
HeightEnd3 = 1
HeightTextSpeed3 = 1
HeightTextIntensity3 = 1
DropOff = 1
spheremode = false
Iterations = 9
ColorIteration = 0
Power = 8
Bailout = 5
AlternateVersion = false
RotVector = 1,1,1
RotAngle = 0
Julia = false
JuliaC = 0,0,0
ApplyOnIteration = 0
FormulaType = 1
ApplicationType = 1
FeedbackRadius = 0.25
FeedbackStrength = 0.2
FBRotVector = 1,1,1
FBRotAngle = 0
FeedBackCutOff = 99
FeedbackVariable1 = 0
FeedbackVariable2 = 0
FeedbackVariable3 = 0
Up = 0,0.9998524,-0.0171813
#endpreset

