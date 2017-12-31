#version 130
#info 3D hyperbolic tessellation. Coxeter group bifurcating 5-3-3. Poincar� ball model. Distance Estimator (knighty 2012)
#define providesInit
#define providesColor
#include "MathUtils.frag"
#include "DE-Raytracer.frag"

#group Hyperbolic-tesselation
// Iteration number.
uniform int Iterations;  slider[0,10,20]

// U 'barycentric' coordinate for the 'principal' node
uniform float U; slider[0,1,1]

// V
uniform float V; slider[0,0,1]

// W
uniform float W; slider[0,0,1]

// T
uniform float T; slider[0,0,1]

//vertex radius 
uniform float VRadius; slider[0,0.05,0.5]

//segments radius 
uniform float SRadius; slider[0,0.01,0.1]

//If you want to have thickness of vetices and segments not affected by the stereographic projection
uniform bool useUniformRadius; checkbox[false]

//If you want to dive inside the hyperbolic space. You will need to set the position of the camera at 0,0,0
uniform bool useCameraAsRotVector; checkbox[false]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]

uniform float RotAngle; slider[-1,0,1]

//cutting sphere radius
uniform float CSphRad; slider[0,0.75,1]

#group HTess-Color
uniform vec3 segAColor; color[0.0,0.0,0.0]
uniform vec3 segBColor; color[0.0,0.0,0.0]
uniform vec3 segCColor; color[0.0,0.0,0.0]
uniform vec3 segDColor; color[0.0,0.0,0.0]
uniform vec3 verticesColor; color[0.0,0.0,0.0]

//#define PI 3.14159
vec4 nd,p;
float cVR,sVR,cSR,sSR,cRA,sRA;
float hdot(vec4 a, vec4 b){//dot product for Minkowski space.
	return dot(a.xyz,b.xyz)-a.w*b.w;
}
vec4 hnormalizew(vec4 v){//normalization of (timelike) vectors in Minkowski space.
	float l=1./sqrt(v.w*v.w-dot(v.xyz,v.xyz));
	return v*l;
}
void init() {
	float cospin=cos(PI/5.);
	float scospin=sqrt(cospin*cospin-0.5);

	//na, nb and nc are simply vec4(1.,0.,0.,0.), vec4(0.,1.,0.,0.) and vec4(0.,0.,1.,0.) respectively
	nd=vec4(-cospin,-0.5,-0.5,-scospin);

	vec4 pabc,pbdc,pcda,pdba;
	pabc=vec4(0.,0.,0.,1.);
	pbdc=vec4(scospin,0.,0.,cospin);
	pcda=vec4(0.,scospin,0.,0.5);
	pdba=vec4(0.,0.,scospin,0.5);
	
	p=hnormalizew(U*pabc+V*pbdc+W*pcda+T*pdba);

	cVR=cosh(VRadius);sVR=sinh(VRadius);
	cSR=cosh(SRadius);sSR=sinh(SRadius);
	cRA=cosh(RotAngle);sRA=-sinh(RotAngle);
}
uniform vec3 Eye; //slider[(-50,-50,-50),(0,0,-10),(50,50,50)] NotLockable
uniform vec3 Target; //slider[(-50,-50,-50),(0,0,0),(50,50,50)] NotLockable
vec4 Rotate(vec4 p){
	//this is a (hyperbolic) rotation (that is, a boost) on the plane defined by RotVector and w axis
	//We do not need more because the remaining 3 rotation are in our 3D space
	//That would be redundant.
	//This rotation is equivalent to a translation inside the hyperbolic space when the camera is at 0,0,0
	vec4 p1=p;
	vec3 rv;
	if (useCameraAsRotVector) rv=normalize(Eye-Target); else rv=normalize(RotVector);
	float vp=dot(rv,p.xyz);
	p1.xyz+=rv*(vp*(cRA-1.)+p.w*sRA);
	p1.w+=vp*sRA+p.w*(cRA-1.);
	return p1;
}

vec4 fold(vec4 pos) {//beside using minkowski dot product, its exactly the same as for euclidean space
	for(int i=0;i<Iterations;i++){
		pos.xyz=abs(pos.xyz);
		float t=-2.*min(0.,hdot(pos,nd)); pos+=t*nd;
	}
	return pos;
}

float DD(float ca, float sa, float r){//converts hyperbolic distance to distance in projection flat space. ca and sa are the hyperbolic cosine and sine of the hyperbolic distance which is an "angle".
	return (2.*r*ca+(1.+r*r)*sa)/((1.+r*r)*ca+2.*r*sa+1.-r*r)-r;
}

float dist2Vertex(vec4 z, float r){
	float ca=-hdot(z,p), sa=0.5*sqrt(-hdot(p-z,p-z)*hdot(p+z,p+z));
	
	if(useUniformRadius) return DD(ca,sa,r)-VRadius;
	else return DD(ca*cVR-sa*sVR,sa*cVR-ca*sVR,r);
}

float dist2Segment(vec4 z, vec4 n, float r){
	//pmin is the orthogonal projection of z onto the plane defined by p and n
	//then pmin is projected onto the unit sphere
	float zn=hdot(z,n),zp=hdot(z,p),np=hdot(n,p);
	float det=-1./(1.+np*np);
	float alpha=det*(zp-zn*np), beta=det*(-np*zp-zn);
	vec4 pmin=hnormalizew(alpha*p+min(0.,beta)*n);
	//ca and sa are the hyperbolic cosine and sine of the "angle" between z and pmin. This is the distance in hyperbolic space.
	float ca=-hdot(z,pmin), sa=0.5*sqrt(-hdot(pmin-z,pmin-z)*hdot(pmin+z,pmin+z));
	if(useUniformRadius) return DD(ca,sa,r)-SRadius;
	else return DD(ca*cSR-sa*sSR,sa*cSR-ca*sSR,r);//we subtract the width of the sgment before conversion
}
//it is possible to compute the distance to a face just as for segments: pmin will be the orthogonal projection
// of z onto the 3-plane defined by p and two n's (na and nb, na and nc, na and and, nb and nd... and so on).
//that involves solving a system of 3 linear equations.
//it's not implemented here because it is better with transparency

float dist2Segments(vec4 z, float r){
	float da=dist2Segment(z, vec4(1.,0.,0.,0.), r);
	float db=dist2Segment(z, vec4(0.,1.,0.,0.), r);
	float dc=dist2Segment(z, vec4(0.,0.,1.,0.), r);
	float dd=dist2Segment(z, nd, r);
	
	return min(min(da,db),min(dc,dd));
}

float DE(vec3 pos) {
	float r=length(pos);
	vec4 z4=vec4(2.*pos,1.+r*r)*1./(1.-r*r);//Inverse stereographic projection of pos: z4 lies onto the unit 3-parabolid of revolution around w axis centered at 0.
	z4=Rotate(z4);
	z4=fold(z4);
	orbitTrap=z4;
	return max(r-CSphRad,min(dist2Vertex(z4,r),dist2Segments(z4, r)));
}

vec3 baseColor(vec3 pos, vec3 normal){
	float r=length(pos);
	vec4 z4=vec4(2.*pos,1.+r*r)*1./(1.-r*r);
	z4=Rotate(z4);
	z4=fold(z4);
	float da=dist2Segment(z4, vec4(1.,0.,0.,0.), r);
	float db=dist2Segment(z4, vec4(0.,1.,0.,0.), r);
	float dc=dist2Segment(z4, vec4(0.,0.,1.,0.), r);
	float dd=dist2Segment(z4, nd, r);
	float dv=dist2Vertex(z4,r);
	float d=min(min(min(da,db),min(dc,dd)),dv);
	vec3 color=segAColor;
	if(d==db) color=segBColor;
	if(d==dc) color=segCColor;
	if(d==dd) color=segDColor;
	if(d==dv) color=verticesColor;
	return color;
}
#preset Default
FOV = 0.4
Eye = 0,0,0
Target = -0.662946,0.0228356,0.748319
Up = -0.00329765,0.999759,-0.0216964
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 0
V = 1
W = 0
T = 0
VRadius = 0.17143
SRadius = 0.06191
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.47368
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset alternated order-5 cubic
FOV = 0.4
Eye = 0,0,0
Target = -0.132165,0.00632025,0.991208
Up = -0.0153689,0.999881,-0.00148792
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 0
V = 0
W = 1
T = 0
VRadius = 0.36517
SRadius = 0.06517
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.2405
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset truncated alternated order-5 cubic
FOV = 0.4
Eye = 0,0,0
Target = 0.00306226,0.193628,0.98107
Up = 0.00960794,0.982332,-0.186898
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 1
V = 0
W = 1
T = 0
VRadius = 0.15169
SRadius = 0.05393
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.2405
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset rectified order-4 dodecahedral
FOV = 0.4
Eye = 0,0,0
Target = 0.00306226,0.193628,0.98107
Up = 0.00960794,0.982332,-0.186898
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 1
V = 0
W = 0
T = 0
VRadius = 0.15169
SRadius = 0.05393
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.2405
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset rectified order-5 cubic
FOV = 0.4
Eye = 0,0,0
Target = -0.126432,-0.000575171,0.991975
Up = 0.0100673,0.999911,0.0087944
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 0
V = 0
W = 1
T = 1
VRadius = 0.12922
SRadius = 0.0382
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.56962
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset bitruncated order-5 cubic
FOV = 0.4
Eye = 0,0,0
Target = -0.126432,-0.000575171,0.991975
Up = 0.0100673,0.999911,0.0087944
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 1
V = 0
W = 1
T = 1
VRadius = 0.12922
SRadius = 0.0382
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.56962
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset truncated order-4 dodecahedral
FOV = 0.4
Eye = 0,0,0
Target = -0.134155,0.00188327,0.990959
Up = 0.0129237,0.999893,0.00678797
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 1
V = 1
W = 0
T = 0
VRadius = 0.14045
SRadius = 0.05843
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.2405
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset Order-4 dodecahedral
FOV = 0.4
Eye = 0,0,0
Target = -0.00105277,0.20056,0.979681
Up = 0.0397883,0.98027,-0.19362
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 0
V = 1
W = 0
T = 0
VRadius = 0.19101
SRadius = 0.08989
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.2405
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset runcinated alternated order-5 cubic
FOV = 0.4
Eye = 0,0,0
Target = -0.00105277,0.20056,0.979681
Up = 0.0397883,0.98027,-0.19362
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 0
V = 1
W = 1
T = 0
VRadius = 0.15169
SRadius = 0.06405
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.2405
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset cantellated order-4 dodecahedral
FOV = 0.4
Eye = 0,0,0
Target = -0.00105277,0.20056,0.979681
Up = 0.0397883,0.98027,-0.19362
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 0
V = 1
W = 1
T = 1
VRadius = 0.10113
SRadius = 0.0382
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.2405
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset runcitruncated alternated order-5 cubic
FOV = 0.4
Eye = 0,0,0
Target = -0.00105277,0.20056,0.979681
Up = 0.0397883,0.98027,-0.19362
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 1
V = 1
W = 1
T = 0
VRadius = 0.11236
SRadius = 0.05843
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.2405
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset

#preset cantitruncated order-4 dodecahedral
FOV = 0.4
Eye = 0,0,0
Target = -0.00105277,0.20056,0.979681
Up = 0.0397883,0.98027,-0.19362
AntiAlias = 1
Detail = -3.53094
DetailAO = -1.57143
FudgeFactor = 1
MaxRaySteps = 100
BoundingSphere = 4
Dither = 0.50877
NormalBackStep = 1
AO = 0,0,0,1
Specular = 3.25
SpecularExp = 44.643
SpotLight = 1,1,1,0.42308
SpotLightDir = 0.1,-0.9077
CamLight = 1,1,1,2
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0.88
HardShadow = 0
ShadowSoft = 2
Reflection = 0
BaseColor = 0.694118,0.694118,0.694118
OrbitStrength = 0.42857
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
BackgroundColor = 0,0,0
GradientBackground = 0.3
CycleColors = false
Cycles = 1.1
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
Iterations = 13
U = 1
V = 1
W = 1
T = 1
VRadius = 0.08427
SRadius = 0.04607
useUniformRadius = false
useCameraAsRotVector = true
RotVector = 0,0,1
RotAngle = 0.2405
CSphRad = 0.999
segAColor = 0.815686,0.129412,0.129412
segBColor = 0.152941,0.741176,0.0862745
segCColor = 0.145098,0.372549,0.866667
segDColor = 0.847059,0.109804,0.74902
verticesColor = 0.792157,0.607843,0.145098
#endpreset