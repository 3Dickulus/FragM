// Described in http://www.fractalforums.com/general-discussion-b77/solids-many-many-solids/

#info fold and cut regular polychora (stereographic projection) Distance Estimator (knighty 2012)
#define providesInit
//#define providesColor
#include "MathUtils.frag"
#include "DE-Raytracer.frag"

#group polychora

//24-cell's specific symmetry group :).

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

//If you want to dive inside the hypersphere. You will need to set the position of the camera at 0,0,0
uniform bool useCameraAsRotVector; checkbox[false]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]

uniform float RotAngle; slider[0.00,0,180]

vec4 nd,p;
float cVR,sVR,cSR,sSR,cRA,sRA;
void init() {
	nd=vec4(-0.5,-0.5,-0.5,0.5);

	vec4 pabc,pbdc,pcda,pdba;
	pabc=vec4(0.,0.,0.,1.);
	pbdc=1./sqrt(2.)*vec4(1.,0.,0.,1.);
	pcda=1./sqrt(2.)*vec4(0.,1.,0.,1.);
	pdba=1./sqrt(2.)*vec4(0.,0.,1.,1.);
	
	p=normalize(U*pabc+V*pbdc+W*pcda+T*pdba);

	cVR=cos(VRadius);sVR=sin(VRadius);
	cSR=cos(SRadius);sSR=sin(SRadius);
	cRA=cos(RotAngle*PI/180.);sRA=sin(RotAngle*PI/180.);
}
uniform vec3 Eye; //slider[(-50,-50,-50),(0,0,-10),(50,50,50)] NotLockable
uniform vec3 Target; //slider[(-50,-50,-50),(0,0,0),(50,50,50)] NotLockable
vec4 Rotate(vec4 p){
	//this is a rotation on the plane defined by RotVector and w axis
	//We do not need more because the remaining 3 rotation are in our 3D space
	//That would be redundant.
	//This rotation is equivalent to translation inside the hypersphere when the camera is at 0,0,0
	vec4 p1=p;
	vec3 rv;
	if (useCameraAsRotVector) rv=normalize(Eye-Target); else rv=normalize(RotVector);
	float vp=dot(rv,p.xyz);
	p1.xyz+=rv*(vp*(cRA-1.)-p.w*sRA);
	p1.w+=vp*sRA+p.w*(cRA-1.);
	return p1;
}

vec4 fold(vec4 pos) {
	for(int i=0;i<3;i++){
		pos.xyz=abs(pos.xyz);
		float t=-2.*min(0.,dot(pos,nd)); pos+=t*nd;
	}
	return pos;
}

float DD(float ca, float sa, float r){
	//magic formula to convert from spherical distance to planar distance.
	//involves transforming from 3-plane to 3-sphere, getting the distance
	//on the sphere (which is an angle -read: sa==sin(a) and ca==cos(a))
	//then going back to the 3-plane.
	//return r-(2.*r*ca-(1.-r*r)*sa)/((1.-r*r)*ca+2.*r*sa+1.+r*r);
	return r-(2.*r*ca-(1.-r*r)*sa)/((1.-r*r)*ca+2.*r*sa+1.+r*r);
}

float dist2Vertex(vec4 z, float r){
	float ca=dot(z,p), sa=0.5*length(p-z)*length(p+z);//sqrt(1.-ca*ca);//
	return DD(ca,sa,r)-VRadius;
}

float dist2Segment(vec4 z, vec4 n, float r){
	//pmin is the orthogonal projection of z onto the plane defined by p and n
	//then pmin is projected onto the unit sphere
	float zn=dot(z,n),zp=dot(z,p),np=dot(n,p);
	float alpha=zp-zn*np, beta=zn-zp*np;
	vec4 pmin=normalize(alpha*p+min(0.,beta)*n);
	//ca and sa are the cosine and sine of the angle between z and pmin. This is the spherical distance.
	float ca=dot(z,pmin), sa=0.5*length(pmin-z)*length(pmin+z);//sqrt(1.-ca*ca);//
	return DD(ca,sa,r)-SRadius;
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
	vec4 z4=vec4(2.*pos,1.-r*r)*1./(1.+r*r);//Inverse stereographic projection of pos: z4 lies onto the unit 3-sphere centered at 0.
	z4=Rotate(z4);//z4.xyw=rot*z4.xyw;
	z4=fold(z4);//fold it
	orbitTrap=z4;
	return min(dist2Vertex(z4,r),dist2Segments(z4, r));
}

#preset 24-cell
FOV = 0.62536
Eye = -7.73293,-2.18087,-0.0495736
Target = 0.733436,0.421329,0.228751
Up = 0.209193,-0.746573,0.616648
AntiAlias = 1
Detail = -3
DetailAO = -1.57143
FudgeFactor = 1 Locked
MaxRaySteps = 100 Locked
BoundingSphere = 4 Locked
Dither = 0.5 Locked
NormalBackStep = 1
AO = 0,0,0,0.90123
Specular = 4.4304
SpecularExp = 16
SpotLight = 1,1,1,0.75
SpotLightDir = 0.6923,0.78462
CamLight = 1,0.827451,0.768627,0.6415
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
HardShadow = 0
ShadowSoft = 8.254
Reflection = 0
BaseColor = 0.721569,0.721569,0.721569
OrbitStrength = 0 Locked
X = 0.411765,0.6,0.560784,0.41748
Y = 0.666667,0.666667,0.498039,-0.16504
Z = 1,0.258824,0.207843,1
R = 0.0823529,0.278431,1,0.82352
BackgroundColor = 0.501961,0.737255,0.956863
GradientBackground = 0.86955
CycleColors = true
Cycles = 4.04901
EnableFloor = false
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
U = 0
V = 0
W = 0
T = 1
VRadius = 0.10476
SRadius = 0.02095
useCameraAsRotVector = false
RotVector = 0,1,1
RotAngle = 90
#endpreset

#preset Default
FOV = 0.62536
Eye = 4.091351,-0.472971,0.7249154
Target = -4.540205,0.8712733,-0.7643374
Up = 0.2143544,0.3831323,-0.8965496
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
U = 1
V = 0
W = 0
T = 0
VRadius = 0.05
SRadius = 0.01
useCameraAsRotVector = false
RotVector = 1,1,1
RotAngle = 0
#endpreset
