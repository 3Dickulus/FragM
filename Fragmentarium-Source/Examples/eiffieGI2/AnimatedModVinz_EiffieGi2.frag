#define providesInit
#include "MathUtils.frag"
#include "3D.frag"
#define PI  3.14159265358979323846264


uniform float time ;
#group Raytracer
uniform int RayBounces;slider[1,6,128]
uniform int RaySteps;slider[64,128,512]
uniform float FudgeFactor;slider[0.1,0.9,1.0]
uniform float HitDistance;slider[0.00001,0.0001,0.001]
uniform float SoftShadows;slider[1.0,8.0,16.0]
uniform float HitDistanceMultiplier;slider[0.03,1,400]
//Non standard Tweaked param
uniform float Factor;slider[1,1,100]
uniform float Suspicious;slider[1.0,1.0,16.0]
uniform float GloubiGoulba;slider[0.0,0.0,8]
uniform float RayDistFactor;slider[0.01,1.0,5]
uniform float Flat;slider[0.0,0.0,0.5]

#group Emissive
uniform bool UseEmissive; checkbox[false]
uniform vec3 lightPosition;slider[(-2.0,-1.0,-10.0),(-1.3,0.9,0.0),(4.0,3.0,2.0)]
uniform vec3 lightColor2;color[1.0,0.8,0.75]
uniform vec3 lightColor;color[1.0,0.8,0.75]
uniform float lightSize;slider[0.001,0.1,2]
uniform float lightStrength;slider[1.0,10.0,32.0]
uniform float LightSoft;slider[0.0,0.0,0.5]
uniform float lightBloom;slider[1.0,10.0,16.0]
uniform bool UseEmissive2; checkbox[false]
uniform vec3 lightPosition2;slider[(-2.0,-1.0,-2.0),(-1.3,0.9,0.0),(2.0,3.0,2.0)]
uniform float lightSize2;slider[0.001,0.1,2]

#group Environment
uniform bool UseBackgroundPicture; checkbox[false]
uniform vec2 RotatePicXY;slider[(-1.0,-1.0),(0.0,0.0),(1.0,1.0)]
uniform float BackgroundExposure; slider[-1,1,6]
uniform sampler2D texture; file[Ditch-River_Env.hdr]
uniform float AmbientStrenght;slider[0.0,1.0,1.0]
uniform vec3 sunColor;color[1.0,1.0,0.5]
uniform vec3 sunDirection;slider[(-1.0,-1.0,-1.0),(0.55,1.0,-0.1),(1.0,1.0,1.0)]
uniform vec3 skyColor;color[0.3,0.6,1.0]
uniform bool UseOrbitSky; checkbox[false]
// Vignette background
uniform float SunPower;slider[0.0,1.0,10.0]
uniform bool GradientSkyVignette; checkbox[false]
uniform bool GradientSkyVertical; checkbox[false]
uniform bool GradientSkyHorizontal; checkbox[false]
uniform vec3 GradientSkyColor;color[0.0,0.0,0.0]
uniform float GradientSky; slider[0.0,0.3,10.0]
uniform float GradientSkyOffset;slider[-1.5,0.0,1.5]
uniform float fog;slider[0.0,5.0,10.0]
uniform float fogCorrector;slider[0.0,0.0,0.5]
uniform float ambience;slider[0.0,0.2,5.0]
uniform float SuperBloom;slider[1.0,1.0,1.5]
#group Material

uniform bool FX; checkbox[false]
uniform float FXTone;slider[-3.14,0.0,6]
uniform vec3 mengerColor;color[0.85,0.9,0.95]
uniform float mengerRefrRefl;slider[-1.0,0.0,1.0]
uniform float mengerGloss;slider[0.0,1.0,1.0]
uniform float mengerSpec;slider[0.0,1.5,5.0]
uniform float mengerSpecExp;slider[1.0,14.0,20.0]
uniform bool MakeEmit; checkbox[false]
uniform float EmitStrength;slider[0.1,1,10]
uniform float IOR;slider[0.0,1.55,10.0]
uniform vec3 mengerfalloffColor;color[0.45,0.64,0.53]
uniform float mengerfalloffDiffuse;slider[-2,0.0,1.4]
uniform float mengerfalloffRefl;slider[-2,0.0,1.4]

#group Coloring
uniform bool EnableColorMultiplier; checkbox[false]
uniform bool EnableStrenghMultiplier; checkbox[false]
uniform float OrbitMultiplier; slider[0,1,15]
uniform float TreshMultiplier; slider[0.0,0.1,1.0]
// This is the pure color of object (in white light)
uniform vec3 BaseColor; color[1.0,1.0,1.0];
// Determines the mix between pure light coloring and pure orbit trap coloring
uniform float OrbitStrength; slider[0,0,1]
uniform float OrbitStrength2; slider[0,0,1]
// Closest distance to YZ-plane during orbit
uniform vec4 X; color[-1,0.7,1,0.5,0.6,0.6];
// Closest distance to XZ-plane during orbit
uniform vec4 Y; color[-1,0.4,1,1.0,0.6,0.0];
// Closest distance to XY-plane during orbit
uniform vec4 Z; color[-1,0.5,1,0.8,0.78,1.0];
// Closest distance to  origin during orbit
uniform vec4 R; color[-1,0.12,1,0.4,0.7,1.0];
uniform bool CycleColors; checkbox[false]
uniform float Cycles; slider[0.1,1.1,32.3]

//ColoringMode - LineArt
uniform bool OrbitTrap; checkbox[True]
uniform bool SPecialZ; checkbox[False]
uniform bool LineArt2; checkbox[False]
uniform bool LineArt3; checkbox[False]
uniform bool LineArt4; checkbox[False]
uniform bool LineArt5; checkbox[False]
//LineArt Param
uniform float LineArtPower; slider[0,0.1,5]
uniform float LineArtShow; slider[0,0.3,1]

//---------------------------------------------------------------------------------------------------------------------------
#group Octahedron
/*
uniform float Scale; slider[0.00,2,4.00]
uniform vec3 Offset; slider[(0,0,0),(1,0,0),(1,1,1)]
uniform float Angle1; slider[-180,0,180]
uniform vec3 Rot1; slider[(-1,-1,-1),(1,1,1),(1,1,1)]
uniform float Angle2; slider[-180,0,180]
uniform vec3 Rot2; slider[(-1,-1,-1),(1,1,1),(1,1,1)]*/


uniform float MengerScale; slider[0.00,3.0,4.00]
uniform vec3 MengerRotVector; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float MengerRotAngle; slider[0.00,0,180]
uniform int MengerIterations;  slider[0,8,100]
uniform int MengerColorIterations;  slider[0,8,100]
uniform vec3 MengerOffset; slider[(0,0,0),(1,1,1),(1,1,1)]

uniform bool UseDupli;checkbox[false]
uniform vec2 OffsetDupX; slider[(0,0,0),(2,1,0),(10,10,10)]
uniform vec2 OffsetDupY; slider[(0,0,0),(2,1,0),(10,10,10)]
uniform vec2 OffsetDupZ; slider[(0,0,0),(4,2,0),(10,10,10)]

// Number of fractal iterations.
uniform int Iterations;  slider[0,13,100]
uniform int ColorIterations;  slider[1,13,20]

#group Floor

uniform float UseOrbit;slider[0.0,0.0,1.0]
uniform bool UseFloor; checkbox[True]
uniform bool UseWall; checkbox[True]
uniform bool UseCurvedFloor; checkbox[false]

uniform vec3 floorColor;color[0.45,0.64,0.53]
uniform float floorReflection;slider[0.0,0.1,1.0]
uniform float floorGloss;slider[0.0,0.9,1.0]
uniform float floorSpec;slider[0.0,0.1,2.0]
uniform float floorSpecExp;slider[1.0,4.0,20.0]
uniform float floorHeight;slider[0.0,1.0,2.0]
uniform float wallHeight;slider[-1.0,1.0,50.0]

uniform vec3 floorfalloffColor;color[0.45,0.64,0.53]
uniform float floorfalloffDiffuse;slider[-5.0,0.0,5.0]
uniform float floorfalloffRefl;slider[-5.0,0.0,5.0]

uniform float curvature; slider[1.,4.,10.];
//uniform float curvature; slider[.01,3.,10.];
uniform float width; slider[0.00,9.20,10.00];
uniform float depth; slider[0.,1.,10.];
uniform vec2 CurvedSol_Placement; slider[(-10.0,-10.0),(5.0,1.02),(10.0,10.0)]
uniform vec3 CurvedFloorRot; slider[(-180.0,-180.0,-180.0),(0.0,-90.0,0.0),(180.0,180.0,180.0)]
uniform vec3 Sol_Placement; slider[(-1.0,-1.0,-1.0),(0.0,1.0,0.0),(1.0,1.0,1.0)]
uniform vec3 Wall_Placement; slider[(-1.0,-1.0,-1.0),(0.04,0.0,-1.0),(1.0,1.0,1.0)]
#group test
uniform vec3 Test; slider[(-1.0,-1.0,-1.0),(0.0,0.0,1.0),(1.0,1.0,1.0)]
uniform vec3 Test2; slider[(-1.0,-1.0,-1.0),(0.04,0.0,-1.0),(1.0,1.0,1.0)]
uniform float Test3;slider[0.0,0.0,1.0]
//Variable-----------------------------------------------
mat3 fracRotation2;
mat3 fracRotation1;
mat3 rot;
mat3 floorrot;

void init() {
	//fracRotation2 = rotationMatrix3(normalize(Rot2), Angle2);
	//fracRotation1 = rotationMatrix3(normalize(Rot1), Angle1);
	rot = rotationMatrix3(normalize(MengerRotVector), MengerRotAngle);
	floorrot = rotationMatrixXYZ(CurvedFloorRot);
}

float rayDistfactor = RayDistFactor;
float varDist = 1.0;
float minDist =0.0;
float maxDist =1.0;
float coef;
vec4 orbitTrap = vec4(10000.0);
vec3 emColor;
float multiStrength = OrbitStrength ;
float tgd = 1.0; //gradientvalue

float maxDepth=80.0, OOMD=1.0/maxDepth;
//const vec3 ior=vec3(1.0,1.52,1.0/1.52);
vec3 ior=vec3(1.0,IOR,1.0/IOR);
int iRay=subframe;//taking the user input and making it ready to use
vec3 sunDir=normalize(sunDirection);
float lBloom=pow(2.,17.-lightBloom)-1.999,lStrength=pow(1.05,17.-lightStrength)-1.04999;
float ShadowExp=pow(2.,17.0-SoftShadows);

float minLiteDist,side=1.0; //1.0=outside, -1.0=inside of object
struct material {vec3 color; float refrRefl, gloss, spec, specExp; vec3 falloffColor; float falloffDiffuse,falloffRefl;};
material Mater1=material(floorColor,floorReflection,floorGloss,floorSpec,pow(2.,floorSpecExp),floorfalloffColor,floorfalloffDiffuse,floorfalloffRefl);
material Mater0=material(mengerColor,mengerRefrRefl,mengerGloss,mengerSpec,pow(2.,mengerSpecExp),mengerfalloffColor,mengerfalloffDiffuse,mengerfalloffRefl);

//-------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------------------



//some simple distance estimate functions
/*float DEBox(in vec3 z, float hlen){return max(abs(z.x),max(abs(z.y),abs(z.z)))-hlen;}
float DECylinder(in vec3 z, float hlen, float radius){return max(length(z.zy)-radius,abs(z.x)-hlen);}
float DERRect(in vec3 z, vec4 radii){return length(max(abs(z)-radii.xyz,0.0))-radii.w;}
float DEOcto(in vec3 z, float hlen){return max(abs(z.x+z.y+z.z),max(abs(-z.x-z.y+z.z),max(abs(-z.x+z.y-z.z),abs(z.x-z.y-z.z))))-hlen;}*/

/*float DEbox1(vec3 z) //Beveled cube distance function by syntopia
{

	float d =  length(max(abs(z)-Size,0.0))-Edge;
		//r = dot(s, s);
		//orbitTrap = min(orbitTrap, abs(vec4(s,r)));
	return d;
}*/
void Colorize ( vec3 z, int n, int maxiter  ){
	if (n < maxiter) {
			if (LineArt2) {orbitTrap =  vec4( cos(length(z)) );}
			if (LineArt3) {orbitTrap = vec4((cos(sin(length(z)))+abs(z.z))*LineArtPower+ LineArtShow);}
			if (LineArt4) {orbitTrap = vec4((cos(sin(length(z.z)))+tan(z.z)))*LineArtPower+ LineArtShow;}
			if (LineArt5) {orbitTrap = vec4((cos(length(z.z))*sin(length(z.z))))*LineArtPower+ LineArtShow;}
			if (SPecialZ) {orbitTrap = (min(length(z)-vec4(1.0) , dot(z,z)))*LineArtPower+ LineArtShow;}
			if (OrbitTrap) {orbitTrap = min(orbitTrap, abs(vec4(z,dot(z,z))));}
	}
}

float DEMenger(vec3 z)
{
	int n = 0;
	if(UseDupli) {
	if(OffsetDupX.x!=0)z.x = mod((z.x),OffsetDupX.x) - OffsetDupX.y; // instance on XZ
	if(OffsetDupY.x!=0)z.y = mod((z.y),OffsetDupY.x) - OffsetDupY.y; // instance on XZ
	if(OffsetDupZ.x!=0)z.z = mod((z.z),OffsetDupZ.x) - OffsetDupZ.y; // instance on XZ
	}
	while (n < MengerIterations) {
		z = rot *z;
		z = abs(z);
		if (z.x<z.y){ z.xy = z.yx;}
		if (z.x< z.z){ z.xz = z.zx;}
		if (z.y<z.z){ z.yz = z.zy;}
		z = MengerScale*z-MengerOffset*(MengerScale-1.0);
		if( z.z<-0.5*MengerOffset.z*(MengerScale-1.0))  z.z+=MengerOffset.z*(MengerScale-1.0);

		//coloring func.
		Colorize(z,n,MengerColorIterations);

		n++;
	}

	return abs(length(z)-1.0 ) * pow(MengerScale, float(-n));
}

/*float DEIco(vec3 z)
{
	float r;

	// Iterate to compute the distance estimator.
	int n = 0;
	while (n < Iterations) {
		z *= fracRotation1;

		if (z.x+z.y<0.0) z.xy = -z.yx;
		if (z.x+z.z<0.0) z.xz = -z.zx;
		if (z.x-z.y<0.0) z.xy = z.yx;
		if (z.x-z.z<0.0) z.xz = z.zx;

		z = z*Scale - Offset*(Scale-1.0);
		z *= fracRotation2;

		r = dot(z, z);
            if (n< ColorIterations)  orbitTrap = min(orbitTrap, abs(vec4(z,r)));

		n++;
	}

	return (length(z) ) * pow(Scale, -float(n));
}
float DEbox(vec3 z)
{
	float r;

	// Iterate to compute the distance estimator.
	int n = 0;
	while (n < Iterations) {
		z *= fracRotation1;

		if (z.x+z.y<0.0) z.xy = -z.yx;
		if (z.x+z.z<0.0) z.xz = -z.zx;
		if (z.x-z.y<0.0) z.xy = z.yx;
		if (z.x-z.z<0.0) z.xz = z.zx;

		z = z*Scale - Offset*(Scale-1.0);
		z *= fracRotation2;

		r = dot(z, z);
            if (n< ColorIterations)  orbitTrap = min(orbitTrap, abs(vec4(z,r)));

		n++;
	}

	return (length(z) ) * pow(Scale, -float(n));
}*/

float  DECurvedSol(vec3 p) {
	p = p *  floorrot + vec3(CurvedSol_Placement,.0) ;
	p.xy-=vec2(width);
	float l=length(pow(abs(p.xy),vec2(curvature))); l=pow(l,1./curvature);
	float d=max(abs(p.z)-depth*10,max(-l+(width),l-(width+.01)));
	d=max(d,p.x-width/2.); d=max(d,p.y-width/2.)-.01;
return d ;
}

float DESol(vec3 z)
{
	float d= dot(z,Sol_Placement.xyz) + 1.0;
	return d;
}
float DEWall(vec3 z)
{
	float d = dot(z,Wall_Placement) + 1.0;
	return d;
}
//---------------------------------------------------------------------------------------------------------------------x^y = e^(y*ln(x))

vec3 colorBase = vec3(0.0,0.0,0.0);


vec3 cycle(vec3 c, float s) {
	return vec3(0.5)+0.5*vec3(cos(s*Cycles+c.x),cos(s*Cycles+c.y),cos(s*Cycles+c.z));
}

vec3 getColorOrbit() {
	orbitTrap.w = sqrt(orbitTrap.w);

	vec3 orbitColor;
	if (CycleColors) {
		orbitColor = cycle(X.xyz,orbitTrap.x)*X.w*orbitTrap.x +
		cycle(Y.xyz,orbitTrap.y)*Y.w*orbitTrap.y +
		cycle(Z.xyz,orbitTrap.z)*Z.w*orbitTrap.z +
		cycle(R.xyz,orbitTrap.w)*R.w*orbitTrap.w;
	} else {
		orbitColor = X.xyz*X.w*orbitTrap.x +
		Y.xyz*Y.w*orbitTrap.y +
		Z.xyz*Z.w*orbitTrap.z +
		R.xyz*R.w*orbitTrap.w;
	}

	if(EnableColorMultiplier){if( orbitColor.x < TreshMultiplier) orbitColor *= OrbitMultiplier ;}
	if(EnableStrenghMultiplier){ multiStrength = ( length(orbitColor) < TreshMultiplier)? OrbitMultiplier*OrbitStrength:OrbitStrength; }
	vec3 color = mix(BaseColor, 3.0*orbitColor,  multiStrength);
	return color;
}

#ifdef  providesColor
vec3 baseColor(vec3 point, vec3 normal);
#endif
//--------------------------------------------------------------------------------------------------------------------

float mapL(in vec3 z){//this is the DE for emissive light (use folding etc. to create multiple lights)
	//z-=lightPosition;
	vec3 z2 = z;
	return (UseEmissive2)?min( length(z-=lightPosition)-lightSize , length(z2-=lightPosition2)-lightSize2 ):length(z-=lightPosition)-lightSize;
}

//---------------------------------
vec2 min2(vec2 d1, vec2 d2){return (d1.x<d2.x)?d1:d2;}//sorts vectors based on .x

vec2 map(in vec3 z)
{//return distance estimate and object id
	vec2 box1=vec2( DEMenger(z),(MakeEmit == false)?0.0:-2.0);
	vec2 wll=vec2( DEWall(z),2.0);
	vec2 flr=vec2( DESol(z),2.0);
	vec2 cflr=vec2( DECurvedSol(z),2.0);
	vec2 lit=vec2((UseEmissive)?mapL(z):1000.0,-2.0);//the id -2 is for emissive light

	minLiteDist=min(minLiteDist,lit.x);//save the closest distance to the light for bloom

	vec2 obj = min2(lit,box1);
	//vec2 obj =lit;
	if(UseCurvedFloor) obj = min2(obj,cflr);
	if(UseFloor) obj = min2(obj,flr);
	if(UseWall) obj = min2(obj,wll);

return obj;
}

material getMaterial( in vec3 z0, in vec3 nor, in float item )
{//get material properties (color,refr/refl,gloss,spec,specExp)
	//Mater0.color=(OrbitTrap==true)?getColorOrbit():BaseColor;
	// Orbit = vec3(0);
	Mater0.color= clamp(getColorOrbit(),0.0,1.0);
	if(UseOrbit!=0.0)Mater1.color = mix( Mater1.color, clamp(getColorOrbit(),0.0,1.0),UseOrbit);
	return (item==0.0)?Mater0:Mater1; //you can extend this with a texture lookup etc.
}
//-------------------------------------------------------------------
vec2 spherical(vec3 n) {
	return vec2 (acos(n.z)/PI, atan(n.y,n.x)/(2.0*PI) );
}
//--------------------------------------------------------------------------
vec3 getBackground( in vec3 rd ){
	vec3 backColor = skyColor;


	if(UseBackgroundPicture){
		float theta = 1.57079633;
		backColor = texture2D( texture, spherical(normalize(vec3(rd.x,rd.y*cos(theta)-rd.z*sin(theta), rd.y*sin(theta)+  rd.z*cos(theta) ))).yx+RotatePicXY).xyz*BackgroundExposure;
/*vec2 position =  (viewCoord+vec2(1.0))/2.0;
position.y = 1.0-position.y;
backColor = texture2D( texture, rd.zy).xyz*BackgroundExposure;*/
}
	if (GradientSky>0.0) {
		if(GradientSkyVignette)  tgd = length( coord );
		if(GradientSkyHorizontal) tgd = length(rd.y+ GradientSkyOffset + 0.3);
		if(GradientSkyVertical) tgd = length(rd.x + GradientSkyOffset) ;
		backColor = mix(backColor , GradientSkyColor, tgd*GradientSky);
	}
	return backColor + ( sunColor*(max(0.0,dot(rd,sunDir))*0.2+pow(max(0.0,dot(rd,sunDir)),256.0)*2.0)*SunPower);
}

//the code below can be left as is so if you don't understand it that makes two of us :)

//random seed and generator
vec2 randv2=fract(cos((gl_FragCoord.xy+gl_FragCoord.yx*vec2(1000.0,1000.0))+vec2(time)*10.0+vec2(iRay,iRay))*10000.0);

vec2 rand2(){// implementation derived from one found at: lumina.sourceforge.net/Tutorials/Noise.html
	randv2+=vec2(1.0);
	return vec2(fract(sin(dot(randv2.xy ,vec2(12.9898,78.233))) * 43758.5453),
		fract(cos(dot(randv2.xy ,vec2(4.898,7.23))) * 23421.631));
}

vec2 intersect(in vec3 ro, in vec3 rd )
{//march the ray until you hit something or go out of bounds
	float t=HitDistance*10.0*rayDistfactor,d=1000.0;
	side=sign(map(ro+t*rd).x);//keep track of which side you are on
	float mult=side*FudgeFactor;
	//for(int i=0;i<RaySteps && abs(d)>HitDistance;i++){ //old
	for(int i=0;i<RaySteps && abs(d)>t*HitDistance*rayDistfactor;i++){
		t+=d=map(ro+t*rd).x*mult;
		if(t>=maxDepth)return vec2(t,-1.0);//-1.0 is id for "hit nothing"
	}
	vec2 h=map(ro+t*rd);//move close to the hit point without fudging
	h.x=t+h.x*side;
	return h;//returns distance, object id
}
vec3 ve=vec3(HitDistance*0.1*HitDistanceMultiplier*rayDistfactor,0.0,0.0);
vec3 getNormal( in vec3 pos, in float item )
{// get the normal to the surface at the hit point
	return side*normalize(vec3(-map(pos-ve.xyy).x+map(pos+ve.xyy).x,
		-map(pos-ve.yxy).x+map(pos+ve.yxy).x,-map(pos-ve.yyx).x+map(pos+ve.yyx).x));
}

vec4 getEmissiveDir( in vec3 pos)
{//get the direction to a DE based light
	vec2 vt=vec2(mapL(pos),0.0);//find emissive light dir by triangulating its nearest point
	float d=length(pos-lightPosition)-lightSize;
	float d2=min(d,length(pos-lightPosition2)-lightSize2);
	if(d2==d) emColor=lightColor; else emColor=lightColor2;

	return vec4(-normalize(vec3(-mapL(pos-vt.xyy)+mapL(pos+vt.xyy),
			-mapL(pos-vt.yxy)+mapL(pos+vt.yxy),-mapL(pos-vt.yyx)+mapL(pos+vt.yyx))),vt.x);
}

vec3 powDir(vec3 nor, vec3  dir, float power)
{//creates a biased random sample without penetrating the surface
	float ddn=max(0.01,abs(dot(dir,nor)));//new
	vec2 r=rand2()*vec2(6.2831853,1.0);
	vec3 nr=(ddn<0.99)?nor:((abs(nor.x)<0.5)?vec3(1.0,0.0,0.0):vec3(0.0,1.0,0.0));
	vec3 sdir=normalize(cross(dir,nr));
	r.y=pow(r.y,1.0/ power + Flat);
	vec3 ro= normalize(sqrt(1.0-r.y*r.y)*(cos(r.x)*sdir + sin(r.x)*cross(dir,sdir)*ddn) + r.y*dir);//new
	return (dot(ro,nor)<0.0)?reflect(ro,nor):ro;
}

vec4 scene(vec3 ro, vec3 rd)
{// find color and depth of scene
	vec3 tcol = vec3(0.0),acol=vec3(0.05*ambience),fcol = vec3(1.0),bcol=getBackground(rd),fgcl=bcol;//total color, ambient, mask, background, fog
	float drl=1.0,spec=1.0,frl=0.0,erl=0.0,fgrl=0.0,smld=0.0;//direct and specular masks, important ray lengths
	minLiteDist=1000.0;//for bloom (sketchy)
	bool bLightRay=false; //is this ray used as a shadow check
	vec2 hit=vec2(0.0);int i=0; //vec2 hit=vec2(0.0); //old
	for(i=0; i <RayBounces && dot(fcol,fcol)>0.001 && drl>0.01; i++ )
	{// create light paths iteratively
		hit = intersect( ro, rd ); //find the first object along the ray march
		if(i==0)frl=hit.x;else erl+=hit.x;//frl=depth, emissive light decays so track of the distance------------------------------------------------------------------

		minDist= min(minDist,hit.x);
		maxDist= max(maxDist,hit.x);

        	if( hit.y >= 0.0 ){//hit something
			if(i>0)acol+=bcol*hit.x*OOMD*ambience; //calc some ambient lighting??? //new
        		ro+= rd * hit.x;// advance ray position
        		vec3 nor = getNormal( ro, hit.y );// get the surface normal
			material m=getMaterial( ro, nor, hit.y );//and material
			if( m.falloffDiffuse != 0.0){
				coef = dot(nor,rd)-m.falloffDiffuse;
				m.color = m.falloffColor * vec3(coef) +  getMaterial( ro, nor, hit.y ).color * vec3(1.0-coef);
			}
			if( m.falloffRefl != 0.0){
				coef = dot(nor,rd)-m.falloffRefl;
				m.refrRefl = mix( 0.01,m.refrRefl, coef);
			}

			if(Factor!=1.0){
			varDist = pow(((minDist - hit.x)/(maxDist)),2.0) ;
			 rayDistfactor = mix(1.0 , Factor ,varDist);
			}

			 if(FX)fcol*= pow(m.color,vec3(FXTone));
			if(bLightRay)drl*=abs(m.refrRefl)+(GloubiGoulba*GloubiGoulba);else fgrl+=hit.x;//if we are checking for shadows then decrease the light unless refl/refr
			vec3 refl=reflect(rd,nor),newRay=refl;//setting up for a new ray direction and defaulting to a reflection
			float se=m.specExp;//also defaulting to the sample bias for specular light
			vec2 rnd=rand2();//get 2 random numbers
			if(abs(m.refrRefl)>rnd.x){//do we reflect and/or refract
				if(m.refrRefl<0.0){//if the material refracts use the fresnel eq.

					vec3 refr=refract(rd,nor,(side>=0.0)?ior.z:ior.y);//calc the probabilty of reflecting instead

					vec2 ca=vec2(dot(nor,rd),dot(nor,refr)),n=(side>=0.0)?ior.xy:ior.yx,nn=vec2(n.x,-n.y);
					if(rnd.y>0.5*(pow(dot(nn,ca)/dot(n,ca),2.0)+pow(dot(nn,ca.yx)/dot(n,ca.yx),2.0))){newRay=refr;nor=-nor;}
				}
				else{

				}
			}else {//if we didn't reflect/refract then use gloss & light direction to determine how we bounce
				if(UseEmissive){//determine the best light to sample
					vec4 emld=getEmissiveDir(ro);//direction and distance to DE light
					float pe=max(dot(emld.xyz,nor),0.0)/max(lStrength*emld.w*emld.w,1.0),ps=max(dot(sunDir,nor),0.0);
					newRay=(pe/(pe+ps)>fract((rnd.x+rnd.y)*213.79))?emld.xyz:sunDir;
				}else newRay=sunDir;
				if(dot(newRay,nor)*m.gloss>rnd.y){bLightRay=true;se=ShadowExp;smld=minLiteDist;}//switch to a shadow check
				else {se=1.5;newRay=normalize(nor+refl);}//checking for ambient light in any direction since direct lighting was improbable
			}
			rd = powDir(nor,newRay,se);//finally redirect the ray
			spec*=se=pow(max(0.0,dot(rd,refl)),m.specExp)*m.spec;//how much does it contribute to specular lighting?
			fcol*=mix(m.color,vec3(1.0),se);// modulate the frequency mask dependant on diffuse contribution //new
		}else{//hit a light so light up the pixel and bail out
			if(i==0)spec=0.0;//lights themselves don't have specularity
			if(hit.y==-1.0)tcol=fcol*bcol;//-1 = background lighting
			else{ tcol=lightColor/max(lStrength*erl*erl,0.5);//we hit a DE light so make it bright!
				if(MakeEmit)tcol=getColorOrbit()*EmitStrength;}
			tcol*=(fcol+vec3(spec))*drl*Suspicious;//this adds light (seems suspicious;) light color * (color mask + specular lighting) * light power remaining after bounces
			break;
		}
		bcol=getBackground(rd);//get the background color in this new direction
	}

	tcol=mix(tcol,fgcl+ bcol*fogCorrector,clamp(log(fgrl*OOMD*fog),0.0,1.0));//fog, still a bit broken :(
	minLiteDist=max(minLiteDist,smld);//light bloom is OR'd in (could be done in post processing)
	return vec4(clamp(max(tcol,lightColor/max(lBloom*minLiteDist*minLiteDist,0.5))*SuperBloom,0.0,1.0),frl/maxDepth);
}

vec3 color(vec3 ro, vec3 rd){
return scene(ro,rd).rgb;
}

/* //for completeness here is an example stand alone main function (from another app)
void main() {//for slow accumulation (needs texture same size)
	vec4 clr=vec4(0.0);
	vec2 pxl=(-size+2.0*(gl_FragCoord.xy+rand2()))/size.y;//size is image resolution
	ro=eye;//eye=camera position, could move this for motion blur
	vec3 er = normalize( vec3( pxl.xy, fov ) );
	vec3 rd = er.x*uu + er.y*vv + er.z*ww;//uu,vv,ww are up right forward vectors
	vec3 go = blurAmount*focusDistance*vec3( -1.0 + 2.0*rand2(), 0.0 );
	vec3 gd = normalize( er*focusDistance - go );
	ro += go.x*uu + go.y*vv;
	rd += gd.x*uu + gd.y*vv;
	clr+=scene(ro,normalize(rd));
	clr.rgb = clamp(pow(clr.rgb,gammaCorrection),0.0,1.0);
	gl_FragColor = vec4(mix(texture2D(tex,gl_FragCoord.xy/vec2(size.xy)).rgb,clr.rgb,1.0/(iRay+1)),1.0);
	gl_FragDepth = clamp(clr.a,0.01,0.99);
}*/



#preset Defaut
FOV = 0.39216
Eye = -0.370982,-0.033395,-2.87777
Target = 1.19436,0.130941,6.95466
Up = -0.175745,0.981535,-0.0755186
EquiRectangular = false
FocalPlane = 2.0406
Aperture = 0.03235
Gamma = 1
ToneMapping = 3
Exposure = 1
Brightness = 1.5352
Contrast = 1.2
Saturation = 2
GaussianWeight = 1
AntiAliasScale = 3
RayBounces = 6
RaySteps = 250
FudgeFactor = 0.90122
HitDistance = 0.00014
SoftShadows = 1.88005
HitDistanceMultiplier = 1
Factor = 1
Suspicious = 1
GloubiGoulba = 0
RayDistFactor = 1
Flat = 0
UseEmissive = false
lightPosition = -0.0227,0.07956,-0.60112
lightColor2 = 0.788235,0.980392,1
lightColor = 0.807843,1,0.870588
lightSize = 0.03306
lightStrength = 29.7537
LightSoft = 0
lightBloom = 11.0278
UseEmissive2 = false
lightPosition2 = -1.3,0.9,0
lightSize2 = 0.1
UseBackgroundPicture = false
RotatePicXY = 0,0
BackgroundExposure = 1
texture = "Ditch-River_Env.hdr"
AmbientStrenght = 1
sunColor = 1,0.960784,0.866667
sunDirection = 0.86842,-0.08334,1
skyColor = 0.694118,0.65098,0.639216
UseOrbitSky = false
SunPower = 2
GradientSkyVignette = true
GradientSkyVertical = false
GradientSkyHorizontal = false
GradientSkyColor = 0,0,0
GradientSky = 0.5495
GradientSkyOffset = 0
fog = 5.5936
fogCorrector = 0
ambience = 0
SuperBloom = 1
FX = false
FXTone = 0.40358
mengerColor = 0.94902,0.94902,0.94902
mengerRefrRefl = 0.31746
mengerGloss = 0.60938
mengerSpec = 1.9697
mengerSpecExp = 9.14283
MakeEmit = false
EmitStrength = 1
IOR = 1.5
mengerfalloffColor = 0.870588,1,0.992157
mengerfalloffDiffuse = 0
mengerfalloffRefl = 0
EnableColorMultiplier = false
EnableStrenghMultiplier = false
OrbitMultiplier = 0
TreshMultiplier = 0.42073
BaseColor = 0.0941176,0.909804,1
OrbitStrength = 0.46667
OrbitStrength2 = 0
X = 0.0627451,0.960784,0,1
Y = 1,0.933333,0,1
Z = 0,0.45098,1,1
R = 1,0,0,-0.15482
CycleColors = true
Cycles = 22.1457
OrbitTrap = false
SPecialZ = false
LineArt2 = true
LineArt3 = false
LineArt4 = false
LineArt5 = false
LineArtPower = 5
LineArtShow = 0.97168
Scale = 2.57488
Offset = 1,0,0
Angle1 = 0
Rot1 = 1,1,1
Angle2 = 180
MengerScale = 2.16592
MengerRotVector = 0.7133,0.33945,0
MengerRotAngle = 180
MengerIterations = 6
MengerColorIterations = 5
MengerOffset = 0.95354,0.15487,0.30088
UseDupli = false
OffsetDupX = 2.38825,1.1625
OffsetDupY = 0,0
OffsetDupZ = 0,0
Iterations = 13
ColorIterations = 13
UseOrbit = 0
UseFloor = false
UseWall = false
UseCurvedFloor = true
floorColor = 0.784314,0.823529,0.835294
floorReflection = 0
floorGloss = 0.10233
floorSpec = 0.2894
floorSpecExp = 1
floorHeight = 0.52778
wallHeight = 1
floorfalloffColor = 0.815686,0.729412,0.776471
floorfalloffDiffuse = 0
floorfalloffRefl = 0
curvature = 3
width = 10
depth = 1.5796
CurvedSol_Placement = 5,0.9278
CurvedFloorRot = -10.5372,-28.9764,-8.7804
Sol_Placement = 0,1,0
Wall_Placement = 0.04,0,-1
Test = 0,0,1
Test2 = 0.04,0,-1
Test3 = 0
#endpreset



#preset KeyFrame.000
FOV = 0.39216
Eye = -0.370982,-0.033395,-2.87777
Target = 1.19436,0.130941,6.95466
Up = -0.175745,0.981535,-0.0755186
FocalPlane = 0.03
#endpreset


#preset KeyFrame.001
FOV = 0.39216
Eye = 0.377683,0.0569666,-1.68219
Target = -0.126643,0.174696,8.26194
Up = 0.000752256,0.999989,0.00472517
#endpreset

#preset KeyFrame.002
FOV = 0.39216
Eye = 0.957515,0.419498,-0.683321
Target = -6.26994,0.225331,6.16359
Up = -0.00157901,0.998715,0.0506566
#endpreset

#preset KeyFrame.003
FOV = 0.3585
Eye = 1.36389,0.132442,0.42941
Target = -7.8417,2.91244,-2.12318
Up = 1.34665,0.370559,0.197744
#endpreset

#preset KeyFrame.004
FOV = 0.62264
Eye = 0.401437,-0.248841,1.32858
Target = -3.81405,2.15617,-7.36622
Up = 1.03363,0.00130618,1.06698
#endpreset

#preset KeyFrame.005
FOV = 0.62264
Eye = -0.704337,0.209282,1.46743
Target = 0.662885,-0.581798,-8.36409
Up = 0,1,1
#endpreset

#preset KeyFrame.006
FOV = 0.3585
Eye = -1.88709,1.26596,0.989902
Target = 5.28316,-4.20772,-3.28647
Up = -0.780268,1.2576,0.646322
#endpreset


#preset KeyFrame.007
FOV = 0.39216
Eye = -0.370982,-0.033395,-2.87777
Target = 1.19436,0.130941,6.95466
Up = -0.175745,0.981535,-0.0755186
#endpreset