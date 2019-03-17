#version 120
//KISOCAHEDRON SCRIPT Higly modded IFS by Vinz
/*this code from eiffie is an example of simple global illumination
includes ideas from Inigo Quilez, Syntopia, rrrola and others
but don't blame them for this questionable code - it came mainly out of my head
the idea is to bounce rays according to the material specification
the materials have 2 components that define the probability a ray bounces a particular way
m.refrRefl = the refraction(-) and reflection(+) probabilty: 0.9 means reflect the ray 90% of the time
	-0.9 means refract for transparency. This has a built in 25% chance of then reflecting (seems about right)
m.gloss = probability of not scattering. Else gathers indirect light from any direction.
If the ray hasn't reflected or scattered and it has a good probability of being
directly lit it becomes a shadow test ray. This shadow ray acts differently. It "dies" if it hits
non-refracting/reflecting objects to create a shadow while allowing some caustics.
Any ray that happens to hit a light will light the pixel. Running
masks are kept for the color, spec and overall amount of light collected.
The material also has specular components:
m.spec = specularity and m.specExp = specular exponent
These should work about as you expect but the specular
exponent also controls how clear a glass or mirror surface will be.
It seems like you can get a lot of different materials with these simple controls.
There is also an emissive material that will let you create complex lighting.
That's a lot for such a small script!*/
#info global illum
//this was built with 3D.frag as of v0.9.12b2
#define providesInit
#include "MathUtils.frag"
#include "3D.frag"
#define PI  3.14159265358979323846264

#group Raytracer
uniform int RayBounces;slider[1,6,128]
uniform int RaySteps;slider[64,128,512]
uniform float FudgeFactor;slider[0.1,0.9,1.0]
uniform float HitDistance;slider[0.00001,0.0001,0.001]
uniform float HitDistanceMultiplier;slider[0.03,1,400]
uniform float SoftShadows;slider[1.0,8.0,16.0]
uniform float Suspicious;slider[1.0,1.0,16.0]
uniform float GloubiGoulba;slider[0.0,0.0,8]
uniform bool buildMask; checkbox[False]

#group Emissive
uniform bool UseEmissive; checkbox[false]
uniform vec3 lightPosition;slider[(-2.0,-1.0,-2.0),(-1.3,0.9,0.0),(4.0,3.0,2.0)]
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
uniform float ambience;slider[0.0,0.2,5.0]

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

#group Coloring
uniform bool EnableColorMultiplier; checkbox[false]
uniform bool EnableStrenghMultiplier; checkbox[false]
uniform float OrbitMultiplier; slider[0,1,15]
uniform float TreshMultiplier; slider[0.0,0.1,0.5]
// This is the pure color of object (in white light)
uniform vec3 BaseColor; color[1.0,1.0,1.0];
// Determines the mix between pure light coloring and pure orbit trap coloring
uniform float OrbitStrength; slider[0,0,1]
// Closest distance to YZ-plane during orbit
uniform vec4 X; color[-1,0.7,1,0.5,0.6,0.6];
// Closest distance to XZ-plane during orbit
uniform vec4 Y; color[-1,0.4,1,1.0,0.6,0.0];
// Closest distance to XY-plane during orbit
uniform vec4 Z; color[-1,0.5,1,0.8,0.78,1.0];
// Closest distance to  origin during orbit
uniform vec4 R; color[-1,0.12,1,0.4,0.7,1.0];
// Background color
//uniform vec3 BackgroundColor; color[0.6,0.6,0.45]
// Vignette background
//uniform float GradientBackground; slider[0.0,0.3,5.0]

uniform bool CycleColors; checkbox[false]
uniform float Cycles; slider[0.1,1.1,32.3]
//---------------------------------------------------------------------------------------------------------------------------
//KISOCA ADDS ----------------------------------------------------------------------------------------------------
#group Object Placement
//Rotate Object
uniform vec3 RotXYZ; slider[(-180,-180,-180),(0,0,0),(180,180,180)]
//Move Object
uniform vec3 MovXYZ; slider[(-5.0,-5.0,0.0),(0.0,0.0,0.0),(5.0,5.0,10.0)];
//Precise Move Object
uniform vec3 FineMove; slider[(-0.1,-0.1,-0.1),(0.0,0.0,0.0),(0.1,0.1,0.1)];
#group Kisocahedron
uniform bool Prefold; checkbox[True]
uniform bool Mode2; checkbox[False]
uniform bool Mode3; checkbox[False]
uniform float PhiMain; slider[-3,1.618,20]
uniform float PhiFine; slider[-1.0,0.0,1.0]
uniform float Scale; slider[0.00,2.0,4.00]

uniform vec3 Offset; slider[(0,0,0),(0.850650808,0.525731112,0),(1,1,1)]
uniform float Angle1; slider[-180,0,180]
uniform vec3 Rot1; slider[(-1,-1,-1),(1,1,1),(1,1,1)]
uniform float Angle2; slider[-180,0,180]
uniform vec3 Rot2; slider[(-1,-1,-1),(1,1,1),(1,1,1)]
// Number of fractal iterations.
uniform int Iterations;  slider[0,13,100]
//ColoringMode
uniform bool OrbitTrap; checkbox[True]
uniform bool SPecialZ; checkbox[False]
uniform bool LineArt2; checkbox[False]
uniform bool LineArt3; checkbox[False]
uniform bool LineArt4; checkbox[False]
uniform bool LineArt5; checkbox[False]

//LineArt Param
uniform float LineArtPower; slider[0,0.1,5]
uniform float LineArtShow; slider[0,0.3,1]
uniform int ColorIterations;  slider[0,3,100]
uniform bool Disolve; checkbox[False]
uniform vec3 DisolveR; slider[(-1,-1,-1),(0,1,1),(1,1,1)]
uniform bool Revert; checkbox[False]
uniform float RevertR; slider[-1.01,-1,-0.94]

#group Floor
uniform bool UseOrbit; checkbox[false]
uniform bool UseFloor; checkbox[True]
uniform vec3 floorColor;color[0.45,0.64,0.53]
uniform float floorReflection;slider[0.0,0.1,1.0]
uniform float floorGloss;slider[0.0,0.9,1.0]
uniform float floorSpec;slider[0.0,0.1,2.0]
uniform float floorSpecExp;slider[1.0,4.0,20.0]
uniform float floorHeight;slider[0.0,1.0,2.0]


//Variable-----------------------------------------------
float Phi = PhiMain + PhiFine;
vec3 n1 = normalize(vec3(-Phi,Phi-1.0,1.0));
vec3 n2 = normalize(vec3(1.0,-Phi,Phi+1.0));
vec3 n5 = normalize(vec3(-Phi,1.0,0.0));
vec3 n6 = normalize(vec3(Phi,0.0,-1.0));
vec3 nX= n1;

vec4 orbitTrap = vec4(10000.0);
vec3 emColor;
float multiStrength = OrbitStrength ;
float tgd = 1.0; //gradientvalue
uniform float time;

mat4 M;
mat3 fracRotation3;
mat4 RotatiX ;

const float maxDepth=20, OOMD=1.0/maxDepth;
const vec3 ior=vec3(1.0,1.52,1.0/1.52);
int iRay=subframe;//taking the user input and making it ready to use
vec3 sunDir=normalize(sunDirection);
float lBloom=pow(2.,17.-lightBloom)-1.999,lStrength=pow(1.05,17.-lightStrength)-1.04999;
float ShadowExp=pow(2.,17.0-SoftShadows);

float minLiteDist,side=1.0; //1.0=outside, -1.0=inside of object
struct material {vec3 color; float refrRefl, gloss, spec, specExp;};
material Mater1=material(floorColor,floorReflection,floorGloss,floorSpec,pow(2.,floorSpecExp));
material Mater0=material(mengerColor,mengerRefrRefl,mengerGloss,mengerSpec,pow(2.,mengerSpecExp));

//-------------------------------------------------------------------------------------------------------------------


void init() {
	mat4 fracRotation2 = rotationMatrix(normalize(Rot2), Angle2);
	mat4 fracRotation1 = rotationMatrix(normalize(Rot1), Angle1);
	fracRotation3 = rotationMatrix3(normalize(Rot1), Angle1);
	RotatiX = rotationMatrix(vec3(1.0,0.0,0.0), RotXYZ.x)*rotationMatrix(vec3(0.0,1.0,0.0), RotXYZ.y)*rotationMatrix(vec3(0.0,0.0,1.0), RotXYZ.z)*translate(MovXYZ+FineMove);
       M =  fracRotation2 *translate(Offset) * scale4(Scale) * translate(-Offset) * fracRotation1;
}

//-------------------------------------------------------------------------------------------------------------------



//some simple distance estimate functions
/*float DEBox(in vec3 z, float hlen){return max(abs(z.x),max(abs(z.y),abs(z.z)))-hlen;}
float DECylinder(in vec3 z, float hlen, float radius){return max(length(z.zy)-radius,abs(z.x)-hlen);}
float DERRect(in vec3 z, vec4 radii){return length(max(abs(z)-radii.xyz,0.0))-radii.w;}
float DEOcto(in vec3 z, float hlen){return max(abs(z.x+z.y+z.z),max(abs(-z.x-z.y+z.z),max(abs(-z.x+z.y-z.z),abs(z.x-z.y-z.z))))-hlen;}*/

float DEIcosa(in vec3 z, float hlen){return max(abs(z.x),max(abs(z.y),abs(z.z)))-hlen;}

//ICOSA FORMULA ----------------------------------------------------------------------------------------------
float DEIcosahedron(vec3 z)
{
	float t;
	z =( RotatiX*vec4(z,1.0)).xyz;

	// Prefolds.
		if ( Prefold ){
		z = abs(z);
		t =dot(z,n1); if (t>0.0) { z-=2.0*t*n1; }
		t =dot(z,n2); if (t>0.0) { z-=2.0*t*n2; }
		t =dot(z,n5); if (t>0.0) { z-=2.0*t*n5; }
		t =dot(z,n6); if (t>0.0) { z-=2.0*t*n6; }
	}
	// Iterate to compute the distance estimator.

	int n = 0;
	while (n < Iterations) {
		if ( Disolve ) nX = nX - DisolveR;
		if ( Revert ) nX.x *= RevertR;
		// Fold
		if ( Mode3 ) {z *= fracRotation3;}
		z = abs(z);
		t =dot(z,nX); if (t>0.0) { z-=2.0*t*nX; }
		if ( Mode2 ) {	t =dot(z,n6); if (t>0.0) { z-=2.0*t*n6; }}

		// Rotate, scale, rotate (we need to cast to a 4-component vector).
		z = (M*vec4(z,1.0)).xyz;

		n++;

		if (n < ColorIterations) {
			if (LineArt2) {orbitTrap =  vec4( cos(length(z)) );}
			if (LineArt3) {orbitTrap = vec4((cos(sin(length(z)))+abs(z.z))*LineArtPower+ LineArtShow);}
			if (LineArt4) {orbitTrap = vec4((cos(sin(length(z.z)))+tan(z.z)))*LineArtPower+ LineArtShow;}
			if (LineArt5) {orbitTrap = vec4((cos(length(z.z))*sin(length(z.z))))*LineArtPower+ LineArtShow;}
			if (SPecialZ) {orbitTrap = (min(length(z)-vec4(1.0) , dot(z,z)))*LineArtPower+ LineArtShow;}
			if (OrbitTrap) {orbitTrap = min(orbitTrap, abs(vec4(z,dot(z,z))));}
			}
	}

	//return DETetra (z,Scale*0.5)*psni;
	return DEIcosa(z,Scale*0.5)*pow(Scale,-float(Iterations));
}

//---------------------------------------------------------------------------------------------------------------------

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
	if(EnableStrenghMultiplier){ multiStrength = ( orbitColor.x < TreshMultiplier)? OrbitMultiplier:OrbitStrength; }
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
	vec2 glass=vec2(DEIcosahedron(z),(MakeEmit==false)?0.0:-2.0);

	//vec2 glass=vec2(DEMenger(z,2.0,vec3(1.,0.,0.),2),0.0);//start your object ids at zero
	//vec2 box=vec2(DEBulb(z-bulbPosition,4.0,1.0,4),1.0);
	vec2 flr=(UseFloor==true)?vec2(z.y+floorHeight,2.0):vec2(1,2.0);//add as many ids as you likee
	vec2 lit=vec2((UseEmissive)?mapL(z):1000.0,-2.0);//the id -2 is for emissive light
	minLiteDist=min(minLiteDist,lit.x);//save the closest distance to the light for bloom
	return min2(min2(glass,lit),flr);//add as many objects as you like this way
}

material getMaterial( in vec3 z0, in vec3 nor, in float item )
{//get material properties (color,refr/refl,gloss,spec,specExp)
	//Mater0.color=(OrbitTrap==true)?getColorOrbit():BaseColor;
	Mater0.color=getColorOrbit();
	if(UseOrbit)Mater1.color = Mater0.color;
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
	randv2+=vec2(1.0,1.0);
	return vec2(fract(sin(dot(randv2.xy ,vec2(12.9898,78.233))) * 43758.5453),
		fract(cos(dot(randv2.xy ,vec2(4.898,7.23))) * 23421.631));
}

vec2 intersect(in vec3 ro, in vec3 rd )
{//march the ray until you hit something or go out of bounds
	float t=HitDistance*10.0,d=1000.0;
	side=sign(map(ro+t*rd).x);//keep track of which side you are on
	float mult=side*FudgeFactor;
	//for(int i=0;i<RaySteps && abs(d)>HitDistance;i++){ //old
	for(int i=0;i<RaySteps && abs(d)>t*HitDistance;i++){
		t+=d=map(ro+t*rd).x*mult;
		if(t>=maxDepth)return vec2(t,-1.0);//-1.0 is id for "hit nothing"
	}
	//float Mask; //old
	//Mask = ( buildMask==true)?-5:0;//old
	vec2 h=map(ro+t*rd);//move close to the hit point without fudging
	//vec2 h=map(ro+t*rd)+ Mask;//old
	h.x=t+h.x*side;
	return h;//returns distance, object id
}
vec3 ve=vec3(HitDistance*0.1*HitDistanceMultiplier,0.0,0.0);
//vec3 ve=vec3(HitDistance,0.0,0.0);//old
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

//vec3 cosPowDir(vec3  dir, float power) //old
vec3 powDir(vec3 nor, vec3  dir, float power)
{//creates a biased random sample without penetrating the surface
	float ddn=max(0.01,abs(dot(dir,nor)));//new
	vec2 r=rand2()*vec2(6.2831853,1.0);
	/*vec3 sdir=cross(dir,((abs(dir.x)<0.5)?vec3(1.0,0.0,0.0):vec3(0.0,1.0,0.0)));
	vec3 tdir=cross(dir,sdir);*/ //old
	vec3 nr=(ddn<0.99)?nor:((abs(nor.x)<0.5)?vec3(1.0,0.0,0.0):vec3(0.0,1.0,0.0));
	vec3 sdir=normalize(cross(dir,nr));
	r.y=pow(r.y,1.0/power);
	vec3 ro= normalize(sqrt(1.0-r.y*r.y)*(cos(r.x)*sdir + sin(r.x)*cross(dir,sdir)*ddn) + r.y*dir);//new
	//float oneminus = sqrt(1.0-r.y*r.y);//old
	//return cos(r.x)*oneminus*sdir + sin(r.x)*oneminus*tdir + r.y*dir;//old
	return (dot(ro,nor)<0.0)?reflect(ro,nor):ro;
}

vec4 scene(vec3 ro, vec3 rd)
{// find color and depth of scene
	//vec3 tcol = vec3(0.0),acol=vec3(0.0),fcol = vec3(1.0),bcol=vec3(0.),fbcol=vec3(0.);//total color, ambient, mask, background, first bg
	//float drl=1.0,spec=1.0,frl=0.0,erl=0.0,smld=0.0;//direct and specular masks, important ray lengths
	vec3 tcol = vec3(0.0),acol=vec3(0.05*ambience),fcol = vec3(1.0),bcol=getBackground(rd),fgcl=bcol;//total color, ambient, mask, background, fog
	float drl=1.0,spec=1.0,frl=0.0,erl=0.0,fgrl=0.0,smld=0.0;//direct and specular masks, important ray lengths
	minLiteDist=1000.0;//for bloom (sketchy)
	bool bLightRay=false; //is this ray used as a shadow check
	vec2 hit=vec2(0.0);int i=0; //vec2 hit=vec2(0.0); //old
	for(i=0; i <RayBounces && dot(fcol,fcol)>0.001 && drl>0.01; i++ )
	{// create light paths iteratively
		//bcol=getBackground(rd);//old
		hit = intersect( ro, rd ); //find the first object along the ray march
		//acol+=bcol*pow(hit.x,0.1)*OOMD*ambience; //calc some ambient lighting??? old
		//if(i==0){frl=hit.x;fbcol=bcol;}//save the very first length and backcolor for fog cheat
		//else erl+=hit.x;//since the emissive light decays keep track of the distance //old
		if(i==0)frl=hit.x;else erl+=hit.x;//frl=depth, emissive light decays so track of the distance
        	if( hit.y >= 0.0 ){//hit something
			if(i>0)acol+=bcol*hit.x*OOMD*ambience; //calc some ambient lighting??? //new
        		ro+= rd * hit.x;// advance ray position
        		vec3 nor = getNormal( ro, hit.y );// get the surface normal
			material m=getMaterial( ro, nor, hit.y );//and material
			//fcol*= m.color;
			 if(FX)fcol*= pow(m.color,vec3(FXTone));
			//if(bLightRay)drl*=abs(m.refrRefl)+GloubiGoulba*GloubiGoulba;//if we are checking for shadows then decrease the light unless refl/refr //old
			if(bLightRay)drl*=abs(m.refrRefl)+(GloubiGoulba*GloubiGoulba);else fgrl+=hit.x;//if we are checking for shadows then decrease the light unless refl/refr
			//this rather complicated section is just choosing an appropriate but "random" ray direction
			vec3 refl=reflect(rd,nor),newRay=refl;//setting up for a new ray direction and defaulting to a reflection
			float se=m.specExp;//also defaulting to the sample bias for specular light
			vec2 rnd=rand2();//get 2 random numbers
			if(abs(m.refrRefl)>rnd.x){//do we reflect and/or refract
				if(m.refrRefl<0.0){//if the material refracts use the fresnel eq.
					vec3 refr=refract(rd,nor,(side>=0.0)?ior.z:ior.y);//calc the probabilty of reflecting instead
					vec2 ca=vec2(dot(nor,rd),dot(nor,refr)),n=(side>=0.0)?ior.xy:ior.yx,nn=vec2(n.x,-n.y);
					//if(rnd.y>0.5*(pow(dot(nn,ca)/dot(n,ca),2.0)+pow(dot(nn,ca.yx)/dot(n,ca.yx),2.0)))newRay=refr; //old
					if(rnd.y>0.5*(pow(dot(nn,ca)/dot(n,ca),2.0)+pow(dot(nn,ca.yx)/dot(n,ca.yx),2.0))){newRay=refr;nor=-nor;}
					//if(rnd.y>1.05-dot(-nor,refr))newRay=refr;//above is real calc - this is a fake for more reflections
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
			//rd = cosPowDir(newRay,se);//finally redirect the ray //old
			rd = powDir(nor,newRay,se);//finally redirect the ray
			//spec*=pow(max(0.0,dot(rd,refl)),m.specExp)*m.spec;//how much does it contribute to specular lighting? //old
			spec*=se=pow(max(0.0,dot(rd,refl)),m.specExp)*m.spec;//how much does it contribute to specular lighting?
			fcol*=mix(m.color,vec3(1.0),se);// modulate the frequency mask dependant on diffuse contribution //new
		}else{//hit a light so light up the pixel and bail out
			if(i==0)spec=0.0;//lights themselves don't have specularity
			if(hit.y==-1.0)tcol=fcol*bcol;//-1 = background lighting
			else{ tcol=lightColor/max(lStrength*erl*erl,0.5);//we hit a DE light so make it bright!
				if(MakeEmit)tcol=getColorOrbit()*EmitStrength;}
			//tcol*=(fcol+spec)*drl*Suspicious;//this adds light (seems suspicious;)//old
			tcol*=(fcol+vec3(spec))*drl*Suspicious;//this adds light (seems suspicious;) light color * (color mask + specular lighting) * light power remaining after bounces
			break;
		}
		bcol=getBackground(rd);//get the background color in this new direction
	}
	//if(hit.y>=0.0)tcol+=acol*fcol; //old
	//tcol=mix(tcol,fbcol,clamp(log(frl*OOMD*fog),0.0,1.0));//fog & ambience // old
	if(hit.y>=0.0 || i>0)tcol+=acol*fcol;//add ambient light where needed (not on lights)
	tcol=mix(tcol,fgcl,clamp(log(fgrl*OOMD*fog),0.0,1.0));//fog, still a bit broken :(
	minLiteDist=max(minLiteDist,smld);//light bloom is OR'd in (could be done in post processing)
	return vec4(clamp(max(tcol,lightColor/max(lBloom*minLiteDist*minLiteDist,0.5)),0.0,1.0),frl/maxDepth);
}

vec3 color(vec3 ro, vec3 rd){return scene(ro,rd).rgb;}

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

#preset defaultCAM
FOV = 0.4
Eye = 0.0,0.0,-3.2
Target = 0.0,0.0,6.8
Up = 0.0,1.0,0.0
#endpreset

#preset ResetAll
FOV = 0.4
Eye = 0,0,-3.2
Target = 0,0,6.8
Up = 0,1,0
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
TestFine = 0
TestMed = 0
Test = 0
position3 = 0,0,0
position2 = 0,0
RayBounces = 6
RaySteps = 128
FudgeFactor = 0.9
HitDistance = 0.0001
HitDistanceMultiplier = 1
SoftShadows = 8
Suspicious = 1
GloubiGoulba = 0
buildMask = false
UseEmissive = false
lightPosition = -1.3,0.9,0
lightColor2 = 1,0.8,0.75
lightColor = 1,0.8,0.75
lightSize = 0.1
lightStrength = 10
LightSoft = 0
lightBloom = 10
UseEmissive2 = false
lightPosition2 = -1.3,0.9,0
lightSize2 = 0.1
UseBackgroundPicture = false
RotatePicXY = 0,0
BackgroundExposure = 1
texture = Ditch-River_Env.hdr
AmbientStrenght = 1
sunColor = 1,1,0.5
sunDirection = 0.55,1,-0.1
skyColor = 0.3,0.6,1
UseOrbitSky = false
SunPower = 1
GradientSkyVignette = false
GradientSkyVertical = false
GradientSkyHorizontal = false
GradientSkyColor = 0,0,0
GradientSky = 0.3
GradientSkyOffset = 0
fog = 5
ambience = 0.2
FX = false
FXTone = 0
mengerColor = 0.85,0.9,0.95
mengerRefrRefl = 0
mengerGloss = 1
mengerSpec = 1.5
mengerSpecExp = 14
MakeEmit = false
EmitStrength = 1
EnableColorMultiplier = false
EnableStrenghMultiplier = false
OrbitMultiplier = 1
TreshMultiplier = 0.1
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.7
Y = 1,0.6,0,0.4
Z = 0.8,0.78,1,0.5
R = 0.4,0.7,1,0.12
CycleColors = false
Cycles = 1.1
RotXYZ = 0,0,0
MovXYZ = 0,0,0
FineMove = 0,0,0
Prefold = true
Mode2 = false
Mode3 = false
PhiMain = 1.618
PhiFine = 0
Scale = 2
Offset = 0.850651,0.525731,0
Angle1 = 0
Rot1 = 1,1,1
Angle2 = 0
Rot2 = 1,1,1
Iterations = 13
OrbitTrap = true
SPecialZ = false
LineArt2 = false
LineArt3 = false
LineArt4 = false
LineArt5 = false
LineArtPower = 0.1
LineArtShow = 0.3
ColorIterations = 3
Disolve = false
DisolveR = 0,1,1
Revert = false
RevertR = -1
UseOrbit = false
UseFloor = true
floorColor = 0.45,0.64,0.53
floorReflection = 0.1
floorGloss = 0.9
floorSpec = 0.1
floorSpecExp = 4
floorHeight = 1
#endpreset


#preset Shape_Defaut
RotXYZ = 0,0,0
Scale = 2
Offset = 0.850651,0.525731,0
Angle1 = 0
Rot1 = 1,1,1
Angle2 = 0
Rot2 = 1,1,1
Iterations = 13
ColorIterations = 3
Disolve = false
DisolveR = 0,1,1
Revert = false
RevertR = -1
Prefold = true
Mode2 = false
Mode3 = false
#endpreset

#preset Shape_StarSqaure
FOV = 0.4
Eye = 0,0,-3.2
Target = 0,0,6.8
Up = 0,1,0
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
TestFine = 0
TestMed = 0
Test = 0
position3 = 0,0,0
position2 = 0,0
RayBounces = 6
RaySteps = 128
FudgeFactor = 0.9
HitDistance = 0.0001
HitDistanceMultiplier = 1
SoftShadows = 8
Suspicious = 1
GloubiGoulba = 0
buildMask = false
UseEmissive = false
lightPosition = -1.3,0.9,0
lightColor2 = 1,0.8,0.75
lightColor = 1,0.8,0.75
lightSize = 0.1
lightStrength = 10
LightSoft = 0
lightBloom = 10
UseEmissive2 = false
lightPosition2 = -1.3,0.9,0
lightSize2 = 0.1
UseBackgroundPicture = false
RotatePicXY = 0,0
BackgroundExposure = 1
texture = Ditch-River_Env.hdr
AmbientStrenght = 1
sunColor = 1,1,0.5
sunDirection = 0.55,1,-0.1
skyColor = 0.3,0.6,1
UseOrbitSky = false
SunPower = 1
GradientSkyVignette = false
GradientSkyVertical = false
GradientSkyHorizontal = false
GradientSkyColor = 0,0,0
GradientSky = 0.3
GradientSkyOffset = 0
fog = 5
ambience = 0.2
FX = false
FXTone = 0
mengerColor = 0.85,0.9,0.95
mengerRefrRefl = 0
mengerGloss = 1
mengerSpec = 1.5
mengerSpecExp = 14
MakeEmit = false
EmitStrength = 1
EnableColorMultiplier = true
EnableStrenghMultiplier = false
OrbitMultiplier = 15
TreshMultiplier = 0.13449
BaseColor = 1,1,1
OrbitStrength = 1
X = 0.5,0.6,0.6,1
Y = 1,0.6,0,1
Z = 0.8,0.78,1,1
R = 0.4,0.7,1,-0.15116
CycleColors = true
Cycles = 8.15
RotXYZ = 0,0,0
MovXYZ = 0,0,0
FineMove = 0,0,0
Prefold = false
Mode2 = false
Mode3 = false
PhiMain = 1.13635
PhiFine = -0.5
Scale = 2
Offset = 0.850651,0.525731,0
Angle1 = 0
Rot1 = 1,1,1
Angle2 = 0
Rot2 = 1,1,1
Iterations = 13
OrbitTrap = true
SPecialZ = false
LineArt2 = false
LineArt3 = false
LineArt4 = false
LineArt5 = false
LineArtPower = 0.1
LineArtShow = 0.3
ColorIterations = 3
Disolve = false
DisolveR = 0,1,1
Revert = false
RevertR = -1
UseOrbit = false
UseFloor = true
floorColor = 0.45,0.64,0.53
floorReflection = 0.1
floorGloss = 0.9
floorSpec = 0.1
floorSpecExp = 4
floorHeight = 1
#endpreset

#preset Mexplod
FOV = 0.59028
Eye = 0.0129237,0.103772,-10.0646
Target = 2.08485,6.71422,-2.85291
Up = 0.0111021,0.735534,-0.677397
EquiRectangular = false
FocalPlane = 0.3113
Aperture = 0.0008
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
BaseColor = 1,0.87451,0.745098
OrbitStrength = 0.45868
X = 0.5,0.6,0.6,1
Y = 1,0.6,0,-0.34328
Z = 0.8,0.78,1,0.95522
R = 0.4,0.7,1,1
CycleColors = true
Cycles = 27.5473
TestFine = 0
TestMed = 0
Test = 0
position3 = 0,0,0
position2 = 0.36016,-0.00768
RayBounces = 8
RaySteps = 294
FudgeFactor = 0.9
HitDistance = 0.0003
HitDistanceMultiplier = 1
SoftShadows = 8
Suspicious = 1
GloubiGoulba = 0
buildMask = false
UseEmissive = false
lightPosition = -1.3,0.9,0
lightColor2 = 1,0.8,0.75
lightColor = 1,0.8,0.75
lightSize = 0.1
lightStrength = 10
LightSoft = 0
lightBloom = 10
UseEmissive2 = false
lightPosition2 = -1.3,0.9,0
lightSize2 = 0.1
UseBackgroundPicture = true
BackgroundExposure = 6
texture = fielditalien_low.hdr
AmbientStrenght = 1
sunColor = 1,1,0.5
sunDirection = 0.55102,0.18368,-0.22448
skyColor = 0.85098,0.933333,1
UseOrbitSky = false
SunPower = 3.1102
GradientSkyColor = 0.603922,0.752941,0.909804
GradientSky = 0.8197
fog = 3.3916
ambience = 2.62355
FX = false
FXTone = 0
mengerColor = 0.85,0.9,0.95
mengerRefrRefl = 0
mengerGloss = 1
mengerSpec = 1.5
mengerSpecExp = 14
MakeEmit = false
EmitStrength = 1
EnableColorMultiplier = false
EnableStrenghMultiplier = false
OrbitMultiplier = 1
TreshMultiplier = 0.1
Prefold = true
Mode2 = false
Mode3 = true
Scale = 1.9788
Offset = 0.25271,0.22383,0.49097
Angle1 = 54.9036
Rot1 = 0.20284,1,-0.5089
Angle2 = -32.6628
Rot2 = 1,-1,-1
Iterations = 18
OrbitTrap = true
SPecialZ = false
LineArt2 = false
LineArt3 = false
LineArt4 = false
LineArt5 = true
LineArtPower = 0
LineArtShow = 0.34274
ColorIterations = 8
Disolve = false
DisolveR = 0,1,1
Revert = false
RevertR = -1
UseOrbit = false
UseFloor = true
floorColor = 0.45,0.64,0.53
floorReflection = 0.1
floorGloss = 0.9
floorSpec = 0.1
floorSpecExp = 4
floorHeight = 1
PhiMain = 16.2821
PhiFine = 0
RotXYZ = 13.3056,-23.112,45.5256
MovXYZ = 0,0,9.5455
FineMove = -0.03488,-0.0938,0.00853
RotatePicXY = -0.26016,-0.0813
GradientSkyVignette = false
GradientSkyVertical = true
GradientSkyHorizontal = false
GradientSkyOffset = 0.14652
#endpreset

#preset EntraNce
FOV = 0.41666
Eye = 0.0232016,-0.945966,-9.83279
Target = 0.157922,0.598273,0.0463423
Up = 6.39898e-05,0.988002,-0.154442
EquiRectangular = false
FocalPlane = 1.4008
Aperture = 0.0083
Gamma = 1
ToneMapping = 2
Exposure = 1.2
Brightness = 1.5
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
TestFine = 0
TestMed = 0
Test = 0
position3 = 0,0,0
position2 = 0,0
RayBounces = 8
RaySteps = 128
FudgeFactor = 1
HitDistance = 0.00058
HitDistanceMultiplier = 1
SoftShadows = 1
Suspicious = 14.4584
GloubiGoulba = 0
buildMask = false
UseEmissive = true
lightPosition = -0.17816,-0.4656,-0.18624
lightColor2 = 1,0.8,0.75
lightColor = 1,0.843137,0.572549
lightSize = 0.23136
lightStrength = 10
LightSoft = 0
lightBloom = 12.7519
UseEmissive2 = false
lightPosition2 = -1.3,0.9,0
lightSize2 = 0.1
UseBackgroundPicture = true
RotatePicXY = -0.187,-0.17074
BackgroundExposure = 6
texture = fielditalien_low.hdr
AmbientStrenght = 1
sunColor = 1,0.529412,0.294118
sunDirection = -1,0.03674,-0.6
skyColor = 0.0627451,0.0666667,0.054902
UseOrbitSky = false
SunPower = 8.5433
GradientSkyVignette = false
GradientSkyVertical = false
GradientSkyHorizontal = true
GradientSkyColor = 0.227451,0.25098,0.301961
GradientSky = 0.2049
GradientSkyOffset = -1.5
fog = 3.986
ambience = 0
FX = false
FXTone = 0
mengerColor = 0.85,0.9,0.95
mengerRefrRefl = 0.25438
mengerGloss = 0.58704
mengerSpec = 1.8273
mengerSpecExp = 16.2844
MakeEmit = false
EmitStrength = 1.21375
EnableColorMultiplier = false
EnableStrenghMultiplier = false
OrbitMultiplier = 15
TreshMultiplier = 0.02709
BaseColor = 0.905882,0.905882,0.905882
OrbitStrength = 0.99587
X = 0.6,0.6,0.6,1
Y = 1,1,1,1
Z = 0.686275,0.686275,0.686275,1
R = 0.623529,0.623529,0.623529,-0.1236
CycleColors = true
Cycles = 32.3
RotXYZ = 0,0,0
MovXYZ = 0,0,7.5758
FineMove = 0,0,0
Prefold = false
Mode2 = true
Mode3 = false
PhiMain = -3
PhiFine = 1
Scale = 2.04948
Offset = 1,1,1
Angle1 = -156.37
Rot1 = 1,1,1
Angle2 = -2.0844
Rot2 = 1,1,1
Iterations = 7
OrbitTrap = false
SPecialZ = false
LineArt2 = false
LineArt3 = false
LineArt4 = false
LineArt5 = true
LineArtPower = 0.7172
LineArtShow = 0.50403
ColorIterations = 8
Disolve = false
DisolveR = 0,1,1
Revert = false
RevertR = -1
UseOrbit = false
UseFloor = true
floorColor = 0.054902,0.0705882,0.0823529
floorReflection = 0.14706
floorGloss = 0.67816
floorSpec = 1.74904
floorSpecExp = 20
floorHeight = 1
#endpreset

#preset Plantule
FOV = 0.7077
Eye = 0.0426659,-0.553287,-9.58353
Target = -0.0650875,4.24256,-0.809241
Up = -0.1635,0.78805,-0.593502
EquiRectangular = false
FocalPlane = 0.2529
Aperture = 0.002
Gamma = 1
ToneMapping = 2
Exposure = 1.2
Brightness = 1.5
Contrast = 1
Saturation = 2.44185
GaussianWeight = 1
AntiAliasScale = 2
TestFine = 0
TestMed = 0
Test = 0
position3 = 0,0,0
position2 = 0,0
RayBounces = 11
RaySteps = 128
FudgeFactor = 1
HitDistance = 0.00025
HitDistanceMultiplier = 55.8378
SoftShadows = 3.7615
Suspicious = 1.8301
GloubiGoulba = 0
buildMask = false
UseEmissive = true
lightPosition = -0.17816,-0.4656,-0.18624
lightColor2 = 1,0.8,0.75
lightColor = 1,0.843137,0.572549
lightSize = 0.23136
lightStrength = 10
LightSoft = 0
lightBloom = 12.7519
UseEmissive2 = false
lightPosition2 = -1.3,0.9,0
lightSize2 = 0.1
UseBackgroundPicture = true
RotatePicXY = -0.187,-0.11382
BackgroundExposure = 2.24135
texture = fielditalien_low.hdr
AmbientStrenght = 1
sunColor = 1,0.976471,0.686275
sunDirection = -1,0.0204,-1
skyColor = 0.666667,0.784314,0.784314
UseOrbitSky = false
SunPower = 1.2598
GradientSkyVignette = false
GradientSkyVertical = false
GradientSkyHorizontal = true
GradientSkyColor = 0.615686,0.835294,0.882353
GradientSky = 0.7787
GradientSkyOffset = -1.43022
fog = 4.6843
ambience = 0
FX = false
FXTone = -3.14
mengerColor = 0.85,0.9,0.95
mengerRefrRefl = 0.12
mengerGloss = 0.72603
mengerSpec = 3.8755
mengerSpecExp = 18.8427
MakeEmit = false
EmitStrength = 1.21375
EnableColorMultiplier = false
EnableStrenghMultiplier = false
OrbitMultiplier = 6.80085
TreshMultiplier = 0.375
BaseColor = 0.992157,0.984314,0.92549
OrbitStrength = 0.78037
X = 0.6,0.27451,0.0117647,0.98508
Y = 1,0.262745,0.498039,1
Z = 0.996078,0.2,0,1
R = 0.623529,0.14902,0.00392157,-0.96652
CycleColors = true
Cycles = 8.44817
RotXYZ = 10.5048,-18.0792,4.9032
MovXYZ = 0,0.188,8.9394
FineMove = -0.02868,-0.04264,-0.1
Prefold = true
Mode2 = true
Mode3 = false
PhiMain = 13.5152
PhiFine = 1
Scale = 1.1175
Offset = 1,1,0.27437
Angle1 = 79.9236
Rot1 = 1,1,1
Angle2 = -70.1928
Rot2 = 0.93594,1,0.5943
Iterations = 60
OrbitTrap = true
SPecialZ = false
LineArt2 = false
LineArt3 = true
LineArt4 = false
LineArt5 = false
LineArtPower = 0.08195
LineArtShow = 0
ColorIterations = 30
Disolve = false
DisolveR = 0,1,1
Revert = false
RevertR = -1
UseOrbit = false
UseFloor = true
floorColor = 0.054902,0.0705882,0.0823529
floorReflection = 0.14706
floorGloss = 0.67816
floorSpec = 1.74904
floorSpecExp = 20
floorHeight = 0.64314
#endpreset

#preset Plantule 360
FOV = 0.7077
Eye = -0.243421,0.134444,-9.73446
Target = -0.351174,4.93029,-0.960174
Up = -0.13789,0.792932,-0.593502
EquiRectangular = true
FocalPlane = 0.2529
Aperture = 0
Gamma = 1
ToneMapping = 2
Exposure = 1.2
Brightness = 1.5
Contrast = 1
Saturation = 2.44185
GaussianWeight = 1
AntiAliasScale = 2
TestFine = 0
TestMed = 0
Test = 0
position3 = 0,0,0
position2 = 0,0
RayBounces = 11
RaySteps = 128
FudgeFactor = 1
HitDistance = 0.00025
HitDistanceMultiplier = 55.8378
SoftShadows = 3.7615
Suspicious = 1.8301
GloubiGoulba = 0
buildMask = false
UseEmissive = true
lightPosition = -0.17816,-0.4656,-0.18624
lightColor2 = 1,0.8,0.75
lightColor = 1,0.843137,0.572549
lightSize = 0.23136
lightStrength = 10
LightSoft = 0
lightBloom = 12.7519
UseEmissive2 = false
lightPosition2 = -1.3,0.9,0
lightSize2 = 0.1
UseBackgroundPicture = true
RotatePicXY = -0.3945,0
BackgroundExposure = 2.24135
texture = fielditalien_low.hdr
AmbientStrenght = 1
sunColor = 1,0.976471,0.686275
sunDirection = -1,0.0204,-1
skyColor = 0.666667,0.784314,0.784314
UseOrbitSky = false
SunPower = 1.2598
GradientSkyVignette = false
GradientSkyVertical = false
GradientSkyHorizontal = true
GradientSkyColor = 0.615686,0.835294,0.882353
GradientSky = 0.7787
GradientSkyOffset = -1.43022
fog = 4.6843
ambience = 0
FX = false
FXTone = -3.14
mengerColor = 0.85,0.9,0.95
mengerRefrRefl = 0.12
mengerGloss = 0.72603
mengerSpec = 3.8755
mengerSpecExp = 18.8427
MakeEmit = false
EmitStrength = 1.21375
EnableColorMultiplier = false
EnableStrenghMultiplier = false
OrbitMultiplier = 6.80085
TreshMultiplier = 0.375
BaseColor = 0.992157,0.984314,0.92549
OrbitStrength = 0.78037
X = 0.6,0.27451,0.0117647,0.98508
Y = 1,0.262745,0.498039,1
Z = 0.996078,0.2,0,1
R = 0.623529,0.14902,0.00392157,-0.96652
CycleColors = true
Cycles = 8.44817
RotXYZ = 10.5048,-18.0792,4.9032
MovXYZ = 0,0.188,9.2797
FineMove = 0.02087,-0.04957,0.05043
Prefold = true
Mode2 = true
Mode3 = false
PhiMain = 13.5152
PhiFine = 1
Scale = 1.1175
Offset = 1,1,0.27437
Angle1 = 79.9236
Rot1 = 1,1,1
Angle2 = -70.1928
Rot2 = 0.93594,1,0.5943
Iterations = 60
OrbitTrap = true
SPecialZ = false
LineArt2 = false
LineArt3 = true
LineArt4 = false
LineArt5 = false
LineArtPower = 0.08195
LineArtShow = 0
ColorIterations = 30
Disolve = false
DisolveR = 0,1,1
Revert = false
RevertR = -1
UseOrbit = false
UseFloor = true
floorColor = 0.054902,0.0705882,0.0823529
floorReflection = 0.14706
floorGloss = 0.67816
floorSpec = 1.74904
floorSpecExp = 20
floorHeight = 0.64314
#endpreset

#preset KisoBlex
FOV = 0.8077
Eye = 0.238774,-0.553082,-8.79376
Target = -2.39567,5.05937,-0.947739
Up = -0.15253,0.679852,-0.717312
EquiRectangular = false
FocalPlane = 0.74235
Aperture = 0.00084
Gamma = 1
ToneMapping = 2
Exposure = 1.2
Brightness = 1.5
Contrast = 1
Saturation = 2.44185
GaussianWeight = 1
AntiAliasScale = 2
TestFine = 0
TestMed = 0
Test = 0
position3 = 0,0,0
position2 = 0,0
RayBounces = 11
RaySteps = 128
FudgeFactor = 1
HitDistance = 0.00085
HitDistanceMultiplier = 1
SoftShadows = 1.4266
Suspicious = 2.06665
GloubiGoulba = 0
buildMask = false
UseEmissive = true
lightPosition = -0.17816,-0.4656,-0.18624
lightColor2 = 1,0.8,0.75
lightColor = 1,0.843137,0.572549
lightSize = 0.23136
lightStrength = 10
LightSoft = 0
lightBloom = 12.7519
UseEmissive2 = false
lightPosition2 = -1.3,0.9,0
lightSize2 = 0.1
UseBackgroundPicture = true
RotatePicXY = -0.187,-0.04588
BackgroundExposure = 1.32001
texture = fielditalien_low.hdr
AmbientStrenght = 1
sunColor = 1,0.976471,0.686275
sunDirection = 0.63134,0.7235,-1
skyColor = 0.784314,0.635294,0.701961
UseOrbitSky = false
SunPower = 2.8761
GradientSkyVignette = false
GradientSkyVertical = false
GradientSkyHorizontal = true
GradientSkyColor = 0.882353,0.74902,0.819608
GradientSky = 0.6019
GradientSkyOffset = -0.81015
fog = 10
ambience = 1
FX = false
FXTone = -3.14
mengerColor = 0.85,0.9,0.95
mengerRefrRefl = 0.12
mengerGloss = 0.72603
mengerSpec = 3.8755
mengerSpecExp = 18.8427
MakeEmit = false
EmitStrength = 1.21375
EnableColorMultiplier = false
EnableStrenghMultiplier = false
OrbitMultiplier = 6.80085
TreshMultiplier = 0.375
BaseColor = 0.992157,0.984314,0.92549
OrbitStrength = 0.81308
X = 0.54902,0.941176,0,-0.55834
Y = 0.960784,0.960784,0.960784,-0.70834
Z = 0.992157,0.909804,0.00392157,0.89166
R = 0.654902,0.207843,0.156863,-0.48118
CycleColors = true
Cycles = 20.9041
RotXYZ = 10.5048,-18.0792,4.9032
MovXYZ = 0,0.188,8.0932
FineMove = -0.02868,-0.04264,-0.06348
Prefold = true
Mode2 = true
Mode3 = false
PhiMain = 7.92753
PhiFine = -0.08444
Scale = 1.042
Offset = 1,1,0
Angle1 = -174.953
Rot1 = -0.01694,0.76272,-0.27118
Angle2 = 95.8896
Rot2 = 1,1,0.6017
Iterations = 60
OrbitTrap = false
SPecialZ = false
LineArt2 = false
LineArt3 = false
LineArt4 = false
LineArt5 = true
LineArtPower = 0.42715
LineArtShow = 0.133
ColorIterations = 67
Disolve = false
DisolveR = 0,1,1
Revert = false
RevertR = -1
UseOrbit = false
UseFloor = true
floorColor = 0.054902,0.0705882,0.0823529
floorReflection = 0.14706
floorGloss = 0.67816
floorSpec = 1.74904
floorSpecExp = 20
floorHeight = 0.64314
#endpreset

#preset Default
FOV = 0.4
Eye = 0,0,-3.2
Target = 0,0,6.8
Up = 0,1,0
EquiRectangular = false
AutoFocus = false
FocalPlane = 1
Aperture = 0
Gamma = 1
ToneMapping = 1
Exposure = 1
Brightness = 1
Contrast = 1
AvgLumin = 0.5,0.5,0.5
Saturation = 1
LumCoeff = 0.212500006,0.715399981,0.0720999986
Hue = 0
GaussianWeight = 1
AntiAliasScale = 2
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.600000024,0.600000024,0.7
Y = 1,0.600000024,0,0.4
Z = 0.800000012,0.779999971,1,0.5
R = 0.400000006,0.699999988,1,0.12
CycleColors = false
Cycles = 1.1
RayBounces = 6
RaySteps = 128
FudgeFactor = 0.9
HitDistance = 0.0001
HitDistanceMultiplier = 1
SoftShadows = 8
Suspicious = 1
GloubiGoulba = 0
buildMask = false
UseEmissive = false
lightPosition = -1.3,0.9,0
lightColor2 = 1,0.800000012,0.75
lightColor = 1,0.800000012,0.75
lightSize = 0.1
lightStrength = 10
LightSoft = 0
lightBloom = 10
UseEmissive2 = false
lightPosition2 = -1.3,0.9,0
lightSize2 = 0.1
UseBackgroundPicture = false
RotatePicXY = 0,0
BackgroundExposure = 1
texture = Ditch-River_Env.hdr
AmbientStrenght = 1
sunColor = 1,1,0.5
sunDirection = 0.55,1,-0.1
skyColor = 0.300000012,0.600000024,1
UseOrbitSky = false
SunPower = 1
GradientSkyVignette = false
GradientSkyVertical = false
GradientSkyHorizontal = false
GradientSkyColor = 0,0,0
GradientSky = 0.3
GradientSkyOffset = 0
fog = 5
ambience = 0.2
FX = false
FXTone = 0
mengerColor = 0.850000024,0.899999976,0.949999988
mengerRefrRefl = 0
mengerGloss = 1
mengerSpec = 1.5
mengerSpecExp = 14
MakeEmit = false
EmitStrength = 1
EnableColorMultiplier = false
EnableStrenghMultiplier = false
OrbitMultiplier = 1
TreshMultiplier = 0.1
RotXYZ = 0,0,0
MovXYZ = 0,0,0
FineMove = 0,0,0
Prefold = true
Mode2 = false
Mode3 = false
PhiMain = 1.618
PhiFine = 0
Scale = 2
Offset = 0.850651,0.525731,0
Angle1 = 0
Rot1 = 1,1,1
Angle2 = 0
Rot2 = 1,1,1
Iterations = 13
OrbitTrap = true
SPecialZ = false
LineArt2 = false
LineArt3 = false
LineArt4 = false
LineArt5 = false
LineArtPower = 0.1
LineArtShow = 0.3
ColorIterations = 3
Disolve = false
DisolveR = 0,1,1
Revert = false
RevertR = -1
UseOrbit = false
UseFloor = true
floorColor = 0.449999988,0.639999986,0.529999971
floorReflection = 0.1
floorGloss = 0.9
floorSpec = 0.1
floorSpecExp = 4
floorHeight = 1
#endpreset
