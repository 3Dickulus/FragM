
#define providesInside
#define providesInit
#include "MathUtils.frag"
#include "DE-Raytracer.frag"
//#include "Fast-Raytracer.frag"
//#include "DE-PK-1.1.9.frag"
//#include "DE-Raytracer-v0.9.10.frag"
//#include "DE-Kn2exp.frag"
//#include "DE-Kn9.frag"  doesn't work
const float pi=3.1415926535897932384626;
const float pi2=6.283185307179586476925;
//#define providesColor
const	float sr32=sqrt(3./2.);
const	float sr23=sqrt(2./3.);
const	float sr13=sqrt(1./3.);
const	float sr12=sqrt(.5);


void init() {
	// rot = rotationMatrix3(normalize(RotVector), RotAngle);
}

#group Fractal
// Number of fractal iterations.
uniform int Iterations;  slider[0,5,100]
//if color iteration is 0, takes the normal minimum  of all iterations.  If it is set to a specific number, takes the orbit of that iteration only.
uniform int ColorIteration;  slider[0,0,100]
uniform int trapmode; slider[1,1,2]
// Mandelbulb exponent (2 is standard)
uniform float manPower; slider[-10,2,10]

uniform float T1offset;slider[-5,2.,5]
uniform float T1Scale;slider[-5,2.,5]
uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-5,-5,-5),(0,0,0),(5,5,5)]
//  Multiply pixel parts by this
uniform vec3 PixelMult; slider[(-2,-2,-2),(1,0,0),(2,2,2)]

// Bailout
uniform float Bailout; slider[0,2,24]
#group Transforms
// magseed is calculated
uniform vec3 RotVector1; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float RotAngle1; slider[0.00,0,360]
uniform int RotIter1; slider[1,1,22]
mat3 rot1= rotationMatrix3(normalize(RotVector1), RotAngle1);



uniform int tIteration1;slider[0,0,20]
uniform int tCount1;slider[1,1,5]
uniform int tType1;slider[1,1,5]
uniform float tOffset1;slider[-3,2.,3]
uniform float tScale1;slider[-3,1.,3]
uniform int tIteration2;slider[0,0,20]
uniform int tCount2;slider[1,1,5]
uniform int tType2;slider[1,1,5]
uniform float tOffset2;slider[-3,2.,3]
uniform float tScale2;slider[-3,1.,3]
float tOffset=tOffset1;
float tScale=tScale1;

uniform int PolyIter1;slider[0,0,20]

uniform float P1sides;slider[2.0,4.0,15.0]
float sides=P1sides;
uniform float P1angle;slider[-3.1416,0.0,3.1416]
float angle=P1angle;
//Polyhedral transform instead of polygonal if checked
uniform bool Hedron1;checkbox[false]
bool polyhedral=Hedron1;
uniform float P1sides2;slider[2.0,4.0,15.0]
float sides2=P1sides2;
uniform float P1angle2;slider[-3.1416,0.0,3.1416]
float angle2=P1angle2;

uniform int PolyIter2;slider[0,0,20]
//Polyhedral transform instead of polygonal if checked
uniform float P2sides;slider[2.0,4.0,15.0]
uniform float P2angle;slider[-3.1416,0.0,3.1416]
uniform bool Hedron2;checkbox[false]
//set both sides to 3 for a tetra hedron, 4 for a cube, and that's it for the Platonics from this set

uniform float P2sides2;slider[2.0,4.0,15.0]
uniform float P2angle2;slider[-3.1416,0.0,3.1416]


void Polytype (inout vec3 z) {

		float rCyz=abs(atan(z.z,z.y));
	 	float i=1.;
		while (rCyz>pi/sides && i<sides)
			 {rCyz-=pi2/sides; i++;}
		rCyz=abs(rCyz);
		z.yz*=(cos(pi/sides*.5)*cos(rCyz-angle))/cos(pi/(sides)*.5-angle);

		if (polyhedral) {
	  		rCyz= abs(atan(sqrt(z.y*z.y+z.z*z.z),z.x));
			i=1.;
			while (rCyz>pi/sides2 && i<sides2) {rCyz-=pi2/sides2; i++;}
			rCyz=abs(rCyz);
		 	z*=(cos(pi/sides2*.5)*cos(rCyz-angle2))/cos(pi/(sides2)*.5-angle2);
		}

}


void SquareCube (inout vec3 z) {
	float rCyz= (z.y*z.y)/(z.z*z.z);
	if (rCyz<1.) {rCyz=1./sqrt(rCyz+1.);} else {rCyz=1./sqrt(1./rCyz+1.);}
	z.yz*=rCyz;
	if (polyhedral) {
		rCyz= (z.y*z.y+z.z*z.z)/(z.x*z.x);
		if (rCyz<1.) {rCyz=1./sqrt(rCyz+1.);} else {rCyz=1./sqrt(1./rCyz+1.);}
		z.xyz*=rCyz;
	}
}

void T1(inout vec3 z,inout float dr,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;
	z.x=tx*sr12-z.y*sr12;
	z.y=tx*sr12+z.y*sr12;

	z=abs(z)*tScale;
	dr*=tScale;
	if (i>=RotIter1) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12;
	z.x=tx*sr23+z.z*sr13-tOffset;
	z.z=-tx*sr13+z.z*sr23;
}
void T2(inout vec3 z,inout float dr,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;
	z.x=tx*sr12-z.y*sr12;
	z.y=tx*sr12+z.y*sr12;

	z=vec3(abs(sqrt(z.y*z.y+z.z*z.z)-tOffset),abs(sqrt(z.x*z.x+z.z*z.z)-tOffset),abs(sqrt(z.x*z.x+z.y*z.y)-tOffset));
	z*=tScale;
	dr*=tScale;

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12;
	z.x=tx*sr23+z.z*sr13;
	z.z=-tx*sr13+z.z*sr23;
}

void T3(inout vec3 z, inout float dr,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;
	z.x=tx*sr12-z.y*sr12;
	z.y=tx*sr12+z.y*sr12;
	z=abs(z);
	z=vec3(abs(z.y+z.z-tOffset),abs(z.x+z.z-tOffset),abs(z.x+z.y-tOffset));
	z*=tScale;
	dr*=tScale;
	//if (i>=Rot1Iteration) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12;
	z.x=tx*sr23+z.z*sr13;
	z.z=-tx*sr13+z.z*sr23;
}

void T4(inout vec3 z, inout float dr ,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;
	z.x=tx*sr12-z.y*sr12;
	z.y=tx*sr12+z.y*sr12;

	z=vec3(abs(z.y*z.y+z.z*z.z-tOffset),abs(z.x*z.x+z.z*z.z-tOffset),abs(z.x*z.x+z.y*z.y-tOffset));
	z*=tScale;
	dr*=tScale;
	//if (i>=Rot1Iteration) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12;
	z.x=tx*sr23+z.z*sr13;
	z.z=-tx*sr13+z.z*sr23;
}
void T5(inout vec3 z,inout float dr,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;
	z.x=tx*sr12-z.y*sr12;
	z.y=tx*sr12+z.y*sr12;

	z=vec3(abs(pow(pow(z.y,8.)+pow(z.z,8.),.25)-tOffset),abs(pow(pow(z.x,8.)+pow(z.z,8.),.25)-tOffset),abs(pow(pow(z.x,8.)+pow(z.y,8.),.25)-tOffset));
	z*=tScale;
	dr*=tScale;
	//if (i>=Rot1Iteration) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12;
	z.x=tx*sr23+z.z*sr13;
	z.z=-tx*sr13+z.z*sr23;
}

void Transform1(inout vec3 z,inout float dr,in int i) {
	float tx=z.x*sr23-z.z*sr13;
	z.z=z.x*sr13 + z.z*sr23;
	z.x=tx*sr12-z.y*sr12;
	z.y=tx*sr12+z.y*sr12;

	z=abs(z)*T1Scale;
	dr*=T1Scale;
	if (i>=RotIter1) {z*=rot1;}     //rotation is here!!!!

	tx=z.x*sr12+z.y*sr12;
	z.y=-z.x*sr12+z.y*sr12;
	z.x=tx*sr23+z.z*sr13-T1offset;
	z.z=-tx*sr13+z.z*sr23;
}
void PinePart (inout vec3 z) {

	if ((manPower==2.)) {
		float sx2=z.x*z.x;
		float sy2=z.y*z.y;
		float sz2=z.z*z.z;
		float r1=2.*(z.x)/sqrt(sy2+sz2);
		z.x=sx2-sy2-sz2;
 		z.y=r1*2.*z.y*z.z;
		z.z=r1*(sy2-sz2);
	} else {
		float rxyz=pow((z.x*z.x+z.y*z.y+z.z*z.z),abs(manPower)/2.);
		float r1=sqrt(z.y*z.y+z.z*z.z);
		float victor=atan(r1,z.x)*manPower;
		float phi=atan(z.z,z.y)*manPower;
		z.x=rxyz*cos(victor);
		r1=rxyz*sin(victor);
		z.z=r1*cos(phi);
		z.y=r1*sin(phi);

	}
}
#group Warps
uniform int orbititer1; slider[0,0,20]
uniform int orbititer1app; slider[0,0,20]
uniform int zlapp; slider[0,0,20]
uniform vec3 zorbit; slider[(-3.0,-3.0,-3.0),(0,0,0),(3.0,3.0,3.00)];
uniform vec3 zorbitApp; slider[(-3.0,-3.0,-3.0),(0,0,0),(3.0,3.0,3.00)];
uniform vec4 orbit1; slider[(-3.00,-3.00,-3.00,-3.00),(0.00,0.00,0.00,0.00),(3.00,3.00,3.00,3.00)]
uniform vec4 orbitApp1;slider[(-3.00,-3.00,-3.00,-3.00),(0,0,0,0),(3.00,3.00,3.00,3.00)]
uniform int Warp1; slider[0,0,20]
uniform vec2 xparts; slider[(-5,-5),(0,0),(5,5)]
uniform float xScale; slider[-6,0,6]
uniform float yzScale; slider[-6,0,6]
uniform vec2 yparts; slider[(-5,-5),(0,0),(5,5)]
uniform float yScale; slider[-6,0,6]
uniform vec2 zparts; slider[(-5,-5),(0,0),(5,5)]
uniform float zScale; slider[-6,0,6]
void Warp (inout vec3 z) {
	z.x-=
	(xparts.x-abs(z.x))*(abs(z.x)-xparts.y)*xScale+
	length(z.yz)*yzScale+
	(yparts.x-abs(z.y))*(abs(z.y)-yparts.y)*yScale+
	(zparts.x-abs(z.z))*(abs(z.z)-zparts.y)*zScale;
}
float DE(vec3 pos) {


		orbitTrap=min(vec4(pos,length(pos)),
									vec4(10.,10.,10.,10.));

	vec3 z=pos;
	vec3 pixelparts=vec3(pos.x*PixelMult.x,pos.y*PixelMult.y,pos.z*PixelMult.z);
	int i=1;
	int i2=1;
	float r=length(z);
	float dr=1.0;
	float r2=0.;
	vec4 orbitstore1=min(vec4(100,100,100,100),vec4(abs(z),length(z)));

	while(r<Bailout && (i<Iterations)) {
			if (i==Warp1) {
				Warp(z);
			}
			if (i==tIteration1) {
				tOffset=tOffset1;
				tScale=tScale1;
				while (i2<=tCount1) {
					if (tType1==1) {
						T1(z,dr,i);
					} else if (tType1==2) {
						T2(z,dr,i);
					} else if (tType1==3) {
						T3(z,dr,i);
					} else if (tType1==4) {
						T4(z,dr,i);
					} else {
						T5(z,dr,i);
					}
					i2++;
				}
				i2=1;
			}
			if (i==tIteration2) {
				tOffset=tOffset2;
				tScale=tScale2;
				while (i2<=tCount2) {
					if (tType1==1) {
						T1(z,dr,i);
					} else if (tType1==2) {
						T2(z,dr,i);
					} else if (tType1==3) {
						T3(z,dr,i);
					} else if (tType1==4) {
						T4(z,dr,i);
					} else {
						T5(z,dr,i);
					}
					i2++;
				}
				i2=1;
			}
				//we do transform1 every iteration...  ;)
			Transform1(z,dr,i);
			if (i==PolyIter1) {
					sides=P1sides;
					angle=P1angle;
					sides2=P1sides2;
					angle2=P1angle2;
					polyhedral=Hedron1;
					Polytype(z);
				}
			if (i==PolyIter2) {
					sides=P2sides;
					angle=P2angle;
					sides2=P2sides2;
					angle2=P2angle2;
					polyhedral=Hedron2;
					Polytype(z);
				}

			PinePart(z);
			if (ColorIteration==0  || ColorIteration>=Iterations) {
					orbitTrap=min(orbitTrap,vec4(z.x*z.x,z.y*z.y,z.z*z.z,dot(z,z)));
				} else if (i==ColorIteration && trapmode==1) {
					orbitTrap=vec4(z,length(z));
				}  else if (i==ColorIteration && trapmode==2) {
					orbitTrap=min(orbitTrap,vec4(z,length(z)));
				}
		if (i==zlapp) {
			r=length (vec3(
				z.x*zorbit.x,
				z.y*zorbit.y,
				z.z*zorbit.z
				));
			z=vec3(z.x+zorbitApp.x*r,
					z.y+zorbitApp.y*r,
					z.z+zorbitApp.z*r);
		}
		if (i==orbititer1app) {
			orbitstore1=vec4(
				orbitstore1.x*orbit1.x,
				orbitstore1.y*orbit1.y,
				orbitstore1.z*orbit1.z,
				length(orbitstore1)*orbit1.w);
				r=length(orbitstore1);

				z=vec3(z.x+(orbitApp1.x+orbitApp1.w)*r,
				z.y+(orbitApp1.y+orbitApp1.w)*r,
				z.z+(orbitApp1.z+orbitApp1.w)*r);

		}

		if (i<=orbititer1) {
			orbitstore1=min(orbitstore1, (vec4(abs(z),length(z))));
			//orbitstore1=vec4(abs(z),length(z));
		}
		if (Julia) {
			z+=JuliaC;
		} else {
			z+=pixelparts;
		}
		r=length(z);
		dr = r*dr*2.0+ 1.0; //pow(i,-i);
		i++;
	}
	return 0.5*log(r)*r/dr;
}



#preset lolcolors
FOV = 0.4
Eye = 2.562139,-0.0670966,0.4123994
Target = -4.418752,0.34414,0.0992925
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.7
DetailAO = -0.5
FudgeFactor = 0.8
MaxRaySteps = 181
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 0.7529412,1,0.8980392,1
SpotLightDir = -0.4693878,0.4285714
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
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0
cSpeed = 6
pOffset = 0
color0 = 1,1,1
Dist0to1 = 1
color1 = 1,0,0.06666667
Dist1to2 = 1
color2 = 0,0.04705882,0.9960784
Dist2to3 = 1
color3 = 0.2313725,1,0
Dist3to0 = 1
orbitStrengthXYZR = 0,0,0,1
Iterations = 6
ColorIteration = 3
manPower = 2
Julia = false
JuliaC = 0,0,0
PixelMult = 1,0,0
Bailout = 2
T1offset = 2
T1Scale = 2
RotVector1 = 1,1,1
RotAngle1 = 0
Rot1Iteration = 1
SphereIteration = 10
T2offset = 2
T2Scale = 1
T2Iteration1 = 5
T4offset = 2
T4Scale = 1
T4Iteration1 = 5
T3offset = 2
T3Scale = 2
T3Iteration1 = 10
T5boffset = 2
T5bScale = 1
T5bIteration1 = 5
Sharp0to1 = false
Sharp1to2 = true
Sharp2to3 = false
Sharp3to0 = false
Up = 0.0589955,0.9982494,-0.0042291
#endpreset

#preset colorstest
FOV = 0.4
Eye = -0.2074053,0.0261624,-0.1871428
Target = 0.6009195,-0.5656118,6.740802
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -5
DetailAO = -0.5
FudgeFactor = 0.95
MaxRaySteps = 411
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 2
SpecularExp = 16
SpotLight = 1,0.6470588,0.07843137,1
SpotLightDir = 0.0204082,0.877551
CamLight = 1,1,1,1.405405
CamLightMin = 0.2352941
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
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 5
ColorIterationsMin = 0
manPower = 2
Julia = false
JuliaC = 0,0,0
PixelMult = 1,0,0
Bailout = 2
T1offset = 2
T1Scale = 2
RotVector1 = 1,1,1
RotAngle1 = 0
Rot1Iteration = 1
SphereIteration = 10
T2offset = 2
T2Scale = 1
T2Iteration1 = 5
T4offset = 2
T4Scale = 1
T4Iteration1 = 5
T3offset = 2
T3Scale = 2
T3Iteration1 = 10
T5boffset = 2
T5bScale = 1
T5bIteration1 = 5
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0.3
cSpeed = 4
color0 = 0.9490196,0,0.9019608
Dist0to1 = 1
color1 = 0.03137255,0,1
Dist1to2 = 1
color2 = 0,0.6039216,0.6980392
Dist2to3 = 1
color3 = 0.01568627,1,0
Dist3to0 = 1
orbitStrengthXYZR = 0,0,0,1
ColorBackFromMaxIter = 2
Up = -0.0428541,0.99502,0.0899931
OrbitTrapMode = 1
#endpreset

#preset Christmas
FOV = 0.4
Eye = 3.359955,-0.114095,0.4481835
Target = -3.620936,0.2971416,0.1350767
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.7
DetailAO = -0.5
FudgeFactor = 0.8
MaxRaySteps = 181
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 0.7529412,1,0.8980392,1
SpotLightDir = -0.4693878,0.4285714
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
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0
cSpeed = 4
pOffset = 3.225806
color0 = 1,1,1
Sharp0to1 = false
Dist0to1 = 1
color1 = 1,0,0.06666667
Sharp1to2 = true
Dist1to2 = 1
color2 = 0,0.04705882,0.9960784
Sharp2to3 = false
Dist2to3 = 1
color3 = 0.2313725,1,0
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 0,0,0,1
ifTexture = false
MapType = 3
tex = texture.jpg
TextureSpeed = 1.5
TextSpeedMult = 0.1
intensity = 2
ColorIterations = 7
trapmode = false
Iterations = 6
ColorIteration = 3
manPower = 2
Julia = false
JuliaC = 0,0,0
PixelMult = 1,0,0
Bailout = 2
T1offset = 2
T1Scale = 2
RotVector1 = 1,1,1
RotAngle1 = 0
Rot1Iteration = 1
SphereIteration = 10
T2offset = 2
T2Scale = 1
T2Iteration1 = 5
T4offset = 2
T4Scale = 1
T4Iteration1 = 5
T3offset = 2
T3Scale = 2
T3Iteration1 = 10
T5boffset = 2
T5bScale = 1
T5bIteration1 = 5
Up = 0.0589955,0.9982494,-0.0042291
#endpreset

#preset fire
FOV = 0.4
Eye = 3.359955,-0.114095,0.4481835
Target = -3.620936,0.2971416,0.1350767
DepthToAlpha = false
depthMag = 1
ShowDepth = false
AntiAlias = 1
Detail = -3.7
DetailAO = -0.5
FudgeFactor = 0.8
MaxRaySteps = 181
BoundingSphere = 2
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
Specular = 4
SpecularExp = 16
SpotLight = 0.7529412,1,0.8980392,1
SpotLightDir = -0.4693878,0.4285714
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
BackgroundColor = 0.0627451,0.05490196,0.04313725
GradientBackground = 1.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
paletteColoring = true
pBaseColor = 0,0,0
BaseStrength = 0
cSpeed = 4
pOffset = 3.225806
color0 = 1,1,1
Sharp0to1 = false
Dist0to1 = 1
color1 = 1,0,0.06666667
Sharp1to2 = true
Dist1to2 = 1
color2 = 0,0.04705882,0.9960784
Sharp2to3 = false
Dist2to3 = 1
color3 = 0.2313725,1,0
Sharp3to0 = false
Dist3to0 = 1
orbitStrengthXYZR = 0,0,0,1
ifTexture = true
tex = texture.jpg
TextSpeedMult = 1
TextureSpeed = 1
intensity = 2
orbitTexX = 0.1612903,0,0,0
orbitTexY = 0.2258065,0,0,0
TextureOffset = 25.80645,77.41935
MapType = 1
Iterations = 6
ColorIteration = 3
manPower = 2
Julia = false
JuliaC = 0,0,0
PixelMult = 1,0,0
Bailout = 2
T1offset = 2
T1Scale = 2
RotVector1 = 1,1,1
RotAngle1 = 0
Rot1Iteration = 1
SphereIteration = 10
T2offset = 2
T2Scale = 1
T2Iteration1 = 5
T4offset = 2
T4Scale = 1
T4Iteration1 = 5
T3offset = 2
T3Scale = 2
T3Iteration1 = 10
T5boffset = 2
T5bScale = 1
T5bIteration1 = 5
Up = 0.0589955,0.9982494,-0.0042291
trapmode = 1
#endpreset

#preset Default
FOV = 0.4
Eye = 0.0406691,-5.057769,0.5512041
Target = -0.1162459,1.894107,-0.2530369
Up = -0.0689974,0.1131096,0.9911843
DepthToAlpha = false
ShowDepth = false
DepthMagnitude = 1
Detail = -3.7
DetailAO = -0.5
FudgeFactor = 0.8
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.7
SpecularExp = 16
SpotLight = 0.7529412,1,0.8980392,1
SpotLightDir = -0.4693878,0.4285714
CamLight = 1,1,1,1
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 0.3414634
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
CycleColors = false
Cycles = 0.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 3
ColorIteration = 3
trapmode = 1
manPower = 2
T1offset = 2
T1Scale = 2
Julia = true
JuliaC = -0.8823529,0.0588235,0
PixelMult = 0.7692308,0,0
Bailout = 2
RotVector1 = 1,1,1
RotAngle1 = 0
RotIter1 = 1
tIteration1 = 0
tCount1 = 1
tType1 = 1
tOffset1 = 2
tScale1 = 1
tIteration2 = 0
tCount2 = 1
tType2 = 1
tOffset2 = 2
tScale2 = 1
PolyIter1 = 0
P1sides = 4
P1angle = 0
Hedron1 = false
P1sides2 = 4
P1angle2 = 0
PolyIter2 = 0
P2sides = 4
P2angle = 0
Hedron2 = false
P2sides2 = 4
P2angle2 = 0
orbititer1 = 0
orbititer1app = 0
zlapp = 0
zorbit = 0,0,0
zorbitApp = 0,0,0
orbit1 = 0,0,0,0
orbitApp1 = 0,0,0,0
Warp1 = 0
xparts = 0,0
xScale = 0
yzScale = 0
yparts = 0,0
yScale = 0
zparts = 0,0
zScale = 0
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
MaxDistance = 1000
MaxRaySteps = 181
Specular = 1
SpecularMax = 10
QualityShadows = false
DebugSun = false
#endpreset
