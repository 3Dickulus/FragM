#version 120
#info Menger Distance Estimator.  Benesi.
#define providesInit

const float pi=2.*1.570796326794897;
const float pi2=2.*pi;
//#include "Fast-Raytracerexp.frag"
#include "MathUtils.frag"
//#include "Fast-Raytracer-with-Textures.frag"
#include "Fast-Raytracer-with-Textures_and_MultiSampling_beta.frag"
//#include "Fast-Raytracer.frag"
#group Menger
// Based on Knighty's Kaleidoscopic IFS 3D Fractals, described here:
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/
float sr32=1.2247448713915890491;
uniform float time;
 float dummy(vec3 p) {
p*=time;
return time;
}
const float sr13=sqrt(1./3.);
const float sr23=sqrt (2./3.);
// Scale parameter. A perfect Menger is 3.0
uniform float Scale; slider[0.00,3.0,4.00]
uniform float s; slider[0.000,0.005,0.0200]

// Number of fractal iterations.
uniform int Iterations;  slider[0,8,100]
uniform int ColorIterations;  slider[0,8,100]


#group Transforms
//uniform bool HoleSphere; checkbox[false]
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


float sides;
float sides2;
float starangle;
float starangle2;
bool StellateHedron=false;




vec3 Stellation (in vec3 z) {

	float rCyz=abs(atan(z.z,z.y));
	 float i=1.;
	while (rCyz>pi/sides && i<sides) {rCyz-=pi2/sides; i++;}
	rCyz=abs(rCyz);
		z.yz*=(cos(pi/sides*.5)*cos(rCyz-starangle))/cos(pi/(sides)*.5-starangle);
		float rCxyz;

	  rCxyz= abs(atan(sqrt(z.y*z.y+z.z*z.z),z.x));
		i=1.;
		while (rCxyz>pi/sides2 && i<sides2) {rCxyz-=pi2/sides2; i++;}
		rCxyz=abs(rCxyz);
		 z*=(cos(pi/sides2*.5)*cos(rCxyz-starangle2))/cos(pi/(sides2)*.5-starangle2);

 	rCyz= (z.y*z.y)/(z.z*z.z);

	if (rCyz<1.) {rCyz=sqrt(rCyz+1.);} else {rCyz=sqrt(1./rCyz+1.);}

		 rCxyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
		if (rCxyz<1.) {rCxyz=sqrt(rCxyz+1.);} else {rCxyz=sqrt(1./rCxyz+1.);}
		z*=rCxyz;

	z.yz*=rCyz;
	return(z);

}


uniform int zlapp; slider[0,10,20]
uniform vec3 zorbit; slider[(-3.0,-3.0,-3.0),(0,0,0),(3.0,3.0,3.00)]
uniform vec3 zorbitApp; slider[(-3.0,-3.0,-3.0),(0,0,0),(3.0,3.0,3.00)]
uniform int orbititer1; slider[0,10,20]
uniform int orbititer1app;slider[0,10,20]
uniform vec4 orbit1; slider[(-3.00,-3.00,-3.00,-3.00),(0.00,0.00,0.00,0.00),(3.00,3.00,3.00,3.00)]
uniform vec4 orbitApp1;slider[(-3.00,-3.00,-3.00,-3.00),(0,0,0,0),(3.00,3.00,3.00,3.00)]
uniform bool orbitCut;checkbox[true]
uniform vec4 orbitGap1; slider[(0.00,0.00,0.00,0.00),(0,0,0,0),(2.00,2.00,2.00,2.00)]
uniform vec4 orbitAdd1; slider[(-2.00,-2.00,-2.00,-2.00),(0,0,0,0),(2.00,2.00,2.00,2.00)]
uniform vec3 CutXYZmult; slider[(-1,-1,-1),(0,0,0),(1,1,1)]


// math 1 = mandy, 2= menger, 3 = pine tree, 4= T1, 5= T4,
// 6= skews (like in Menger), 7=offset addin, 8= zappa!$! zappa powah!)
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

//#group ExtraVariables
//feedbackcontrol1.w[]
uniform float FeedbackVariable1;slider[-10,1.,10]
//feedbackcontrol2.z[]
uniform float FeedbackVariable2;slider[-10,1.,10]
//feedbackcontrol2.w[]
uniform float FeedbackVariable3;slider[-10,1.,10]
//and put a few happy little arrays over here

bool feedbackbool=true;

float DE(vec3 z)
{
	int feedbackstop=int(feedbackcount);
	if (feedbackstop>FeedBackCutOff) {
			feedbackstop=FeedBackCutOff;
	}
	float t=9999.0;
	 float sc=Scale;
	 float sc1=sc-1.0;
	float sc2=sc1/sc;
	vec3 C=vec3(1.0,1.0,.5);
	float w=1.;
	int n = 1;
	vec3 z2=z;
	vec3 pos=z;
	int m=0;
		float zappaz;
		int fi=0;


	float r=length(z);
	float r2=0.;
	vec4 orbitstore1=vec4(abs(z),length(z));

	while (n <= Iterations) {

		//if (n==RotIter1 || n>=RotEverAfter) {
			//		z*=rot; }
			if (n==HeightIter && !orbitBoolSet) {
				MapType=HeightMapType;
				textoff=hTextureOffset1;
				texturespeed=hTextureSpeed1;
				hintensity=HighStrength1*.02;
				ColorTex=HeightMap1Tex;    //set colortex to height map 1 texture
				z2=TextureIT(vec4(z,length(z)));
				z+=z2*hintensity;
			}
			m=n-1;
			fi=0;

		while (fi<=feedbackstop) {
			if (feedbackcontrol1[fi].x==m) {   //for applying by iteration
				zappaz=length (pos-feedbackcrds[fi].xyz);
				if (zappaz<feedbackcontrol2[fi].x) {
							zappaz= (feedbackcontrol2[fi].x-zappaz)*feedbackcontrol2[fi].y;

					if (feedbackcontrol1[fi].y==1.) {
						if (feedbackcontrol1[fi].z==1.) {
							//zappaz=length(feedbackcrds[fi].xyz)*zappaz;
							z2=vec3(zappaz,0.,0.);
							z2*=rotationMatrix3(normalize(feedbackrotation[fi].xyz),feedbackrotation[fi].w);
							z+=z2;
						} else if (feedbackcontrol1[fi].z==2.) {
							z+=feedbackcrds[fi].xyz*zappaz;
						} else if (feedbackcontrol1[fi].z==3.) {
							zappaz=length(feedbackcrds[fi].xyz)*zappaz;
							z2=vec3(zappaz,0.,0.);
							z2*=rotationMatrix3(normalize(feedbackrotation[fi].xyz),feedbackrotation[fi].w);
							z+=z2;
						}
					} else if (feedbackcontrol1[fi].y==2.) {
						z2= (pos-feedbackcrds[fi].xyz)*zappaz;
						z2*=rotationMatrix3(normalize(feedbackrotation[fi].xyz),feedbackrotation[fi].w);

						if (feedbackcontrol1[fi].z==1.) {
							z=z2;
						} else if (feedbackcontrol1[fi].z==2.) {
							z+=z2;
						}

					} else if (feedbackcontrol1[fi].y==3.) {
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

//heightmap function here
// feedcontrol1.w = texture speed
// feedcontrol2.z=  heightmap texture selection
// zappa uses feedcontrol2.y as the heightmap strength variable...
						MapType=HeightMapType;
						textoff=vec2(feedbackcontrol2[fi].z,feedbackcontrol2[fi].w)*.1;
						texturespeed=feedbackcontrol1[fi].w;
						hintensity=zappaz;
						ColorTex=int(feedbackcontrol1[fi].z);    //set colortex to height map 1 texture
							z2= z*rotationMatrix3(normalize(feedbackrotation[fi].xyz),feedbackrotation[fi].w);
							z2=TextureIT(vec4(z2,length(z2)));

							z+=z2*hintensity;

						/* } else if (feedbackcontrol1[fi].z==2.) {
							z2=TextureIT(vec4(z,length(z)));
							z.x+=length(z2)*hintensity;

						} else if (feedbackcontrol1[fi].z==3.) {
							z2=TextureIT(vec4(z,length(z)));
							z.y+=length(z2)*hintensity;

						} else if (feedbackcontrol1[fi].z==4.) {
							z2=TextureIT(vec4(z,length(z)));
							z.z+=length(z2)*hintensity;

						}else if (feedbackcontrol1[fi].z==5.) {
							z2=TextureIT(vec4(z,length(z)));
							z.xy+=length(z2)*hintensity;

						}  else if (feedbackcontrol1[fi].z==6.) {
							z2=TextureIT(vec4(z,length(z)));
							z.yz+=length(z2)*hintensity;

						}  else if (feedbackcontrol1[fi].z==7.) {
							z2=TextureIT(vec4(z,length(z)));
							z.xz+=length(z2)*hintensity;

						} */


					}	 else if (feedbackcontrol1[fi].y==5.) {
						r=zappaz * length (vec3(
							z.x*feedbackrotation[fi].x,
							z.y*feedbackrotation[fi].y,
							z.z*feedbackrotation[fi].z
							));
						z=vec3(z.x+feedbackcontrol1[fi].w*r,
								z.y+feedbackcontrol2[fi].z*r,
								z.z+feedbackcontrol2[fi].w*r);

					}
				}  //zapparadius check
			}   //feedbackcontrol iteration check
			fi++;
		} // end of while (fi<FeedBacks)

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
		}


		if (n<=ColorIterations) {
				orbitTrap = min(orbitTrap, vec4(abs(z),length(z)));
		}
		if (n==ColorIterations && orbitBoolSet)
			{n=Iterations;orbitBoolSet=false;}
		n++;
	}
	return abs(length(z)-0.0 ) /w;
}




/*		if (n==orbititer1app) {
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
		}
*/


#preset Default
FOV = 0.4
Eye = 2.399813,0.3752105,-1.648577
Target = -3.01216,-0.6568988,2.669441
DepthToAlpha = true
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
SampleColorTex = 1
BackgroundColor = 0.2235294,0.3333333,0.3843137
GradientBackground = 0.5
tex1 = texture.jpg
tex2 = texture2.jpg
tex3 = vigga.jpg
tex4 = Ditch-River_Env.hdr
SpeedMult = 1
TextureSpeed = 1
intensity = 2.6
orbitTexX = -0.081,0,0.02,0
orbitTexY = 0.2,-0.3,-0.1,0
TextureOffset = 48.3871,0
BaseMapType = 2
tsides = 4
tsides2 = 4
HeightMapType = 0
HeightMap1Tex = 3
HeightIter = 3
HighStrength1 = 1
hTextureOffset1 = 30,0
hTextureSpeed1 = 1
SampleMapType = 0
SampleSpeed = 1
SampleOffset = 28.57143,78.57143
ThetaSteps = 5
PhiSteps = 7
ConeWidth = 6.283185
InteriorIntensity = 1
InteriorWeight = 0.5
PostWeight = 0.6
PostIntensity = 1
allsample = false
normalColor = true
colorDepth = 0
SampleStepsShort = 3
SampleDepthShort = 0.0002
LongFirst = false
SampleStepsLong = 2
SampleDepthLong = 0.002
Scale = 3
s = 0
Iterations = 8
ColorIterations = 3
RotVector = 1,0,0
RotAngle = 36
RotIter1 = 9
RotEverAfter = 9
zlapp = 1
zorbit = -0.975,-0.45,0
zorbitApp = 2,1,-0.2295082
ApplyOnIteration = 0
FormulaType = 1
ApplicationType = 1
FeedbackRadius = 0.25
FeedbackStrength = 0.2
FBRotVector = 1,1,1
FBRotAngle = 0
FeedBackCutOff = 99
FeedbackVariable1 = 1
FeedbackVariable2 = 1
FeedbackVariable3 = 1
orbit1 = 1.936709,-0.1898734,0.2658228,0
orbitApp1 = -0.5,1.4,-0.6,0
orbitCut = false
orbitGap1 = 0.5,0.5,0.5,0.5
orbitAdd1 = 2,2,2,2
CutXYZmult = 0,0,-0.1923077
orbititer1 = 1
orbititer1app = 2
Up = -0.0826471,0.9877304,0.1325056
#endpreset

#preset Lightning type
FOV = 0.4
Eye = 1.3846,0.3134653,-3.31864
Target = -1.697171,-0.6331271,2.894785
DepthToAlpha = true
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
tex = glitter_glow_texture_ix_by_hauntingmewithstock.jpg
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
DepthToAlpha = true
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
tex = glitter_glow_texture_ix_by_hauntingmewithstock.jpg
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
tex2 = 1037.jpg
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
DepthToAlpha = true
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
tex = glitter_glow_texture_ix_by_hauntingmewithstock.jpg
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
DepthToAlpha = true
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
tex = glitter_glow_texture_ix_by_hauntingmewithstock.jpg
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
tex2 = 1037.jpg
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
DepthToAlpha = true
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
tex = glitter_glow_texture_ix_by_hauntingmewithstock.jpg
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
DepthToAlpha = true
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
tex = glitter_glow_texture_ix_by_hauntingmewithstock.jpg
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

#preset CitadelGoodHeightColor
FOV = 0.4
Eye = -2.291127,-0.1807599,-2.277811
Target = 2.622306,0.5272921,2.657452
DepthToAlpha = true
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
LegacyColor = false
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
TexturePercent = 76
tex = glitter_glow_texture_ix_by_hauntingmewithstock.jpg
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
ConeWidth = 6.283185
InteriorIntensity = 1
InteriorWeight = 0.5
PostWeight = 1
PostIntensity = 1.5
allsample = true
normalColor = false
colorDepth = -0.01
SampleStepsShort = 4
SampleDepthShort = 0.007
LongFirst = false
SampleStepsLong = 3
SampleDepthLong = 0.05
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
HeightIter = 3
HeightIter2 = 4
tex2 = 1037.jpg
hTextSpeedMult = 1
hTextureSpeed = 1
HighStrength = 2
horbitTexX = 0,0,-0.1,0.2
horbitTexY = 0,0,0,-0.4
hscale = 0.1,0,0,-0.3
hTextureOffset = 40,30
hMapType = 0
htsides = 4
htsides2 = 4
Up = -0.0493508,0.9943924,-0.0935309
#endpreset

#preset ExpHeight
FOV = 0.4
Eye = -2.541803,-0.5598441,-1.38403
Target = 3.193846,1.186106,2.229
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
LegacyColor = false
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
PalettePercent = 0
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
BaseColorTex = 2
SampleColorTex = 1
tex1 = texture.jpg
tex2 = texture2.jpg
tex3 = vigga.jpg
tex4 = Ditch-River_Env.hdr
SpeedMult = 1
TextureSpeed = 1
intensity = 2.6
orbitTexX = -0.081,0,0.02,0
orbitTexY = 0.2,-0.3,-0.1,0
TextureOffset = 44.3871,1
BaseMapType = 2
testA = false
tsides = 4
tsides2 = 4
HeightMapType = 0
HeightMap1Tex = 3
HeightIter = 0
HighStrength1 = 0.3
hTextureOffset1 = 20,10
hTextureSpeed1 = 1
HeightMap2Tex = 4
HeightIter2 = 3
HighStrength2 = 0
hTextureOffset2 = 10,10
hTextureSpeed2 = 1
HeightMap3Tex = 3
HeightIter3 = 3
HighStrength3 = 0
hTextureOffset3 = 0,0
hTextureSpeed3 = 0
SampleMapType = 2
SampleSpeed = 1
SampleOffset = 40,10
ThetaSteps = 5
PhiSteps = 5
ConeWidth = 6.283185
InteriorIntensity = 1
InteriorWeight = 0.5
PostWeight = 1
PostIntensity = 1.5
allsample = true
normalColor = false
colorDepth = -0.01
SampleStepsShort = 4
SampleDepthShort = 0.002
LongFirst = false
SampleStepsLong = 3
SampleDepthLong = 0.07
Scale = 3
s = 0
HoleSphere = false
Iterations = 8
ColorIterations = 3
trapmode = false
polytrap = false
Trapangle = 0
Trapsides = 4
Trapangle2 = 0
Trapsides2 = 4
TrapHedron = false
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
HeightAngle3 = 0
HeightVector3 = 0,0,1
HeightStart3 = 0
HeightEnd3 = 0
HeightTextSpeed3 = 1
HeightTextIntensity3 = 1
DropOff = 1
Up = -0.184223,0.9671923,-0.1749312
zorbit2:Linear:0:-0.451172:0.0351562:1:120:0.3:1:1.7:1:1
#endpreset





#preset KeyFrame.001
FOV = 0.4
Eye = -2.541803,-0.5598441,-1.38403
Target = 3.193846,1.186106,2.229
Up = -0.184223,0.9671923,-0.1749312
#endpreset

#preset KeyFrame.002
FOV = 0.4
Eye = -2.070433,-0.1533558,0.1076616
Target = 4.476472,1.692714,-1.544623
Up = -0.3142836,0.9255353,-0.2112114
#endpreset
