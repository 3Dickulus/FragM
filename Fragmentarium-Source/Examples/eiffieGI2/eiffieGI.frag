#version 120
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
#include "MathUtils.frag"
//this was built with 3D.frag as of v0.9.12b2
#include "3D.frag"
uniform float time;
#group Environment
uniform vec3 sunColor;color[1.0,1.0,0.5]
uniform vec3 sunDirection;slider[(-1.0,-1.0,-1.0),(0.55,1.0,-0.1),(1.0,1.0,1.0)]
uniform vec3 skyColor;color[0.3,0.6,1.0]
uniform float fog;slider[0.0,5.0,10.0]
uniform float ambience;slider[0.0,0.2,25.0]
uniform vec3 floorColor;color[0.45,0.64,0.53]
uniform float floorReflection;slider[0.0,0.1,1.0]
uniform float floorGloss;slider[0.0,0.9,1.0]
uniform float floorSpec;slider[0.0,0.1,2.0]
uniform float floorSpecExp;slider[1.0,4.0,20.0]
#group Menger
uniform vec3 mengerColor;color[0.85,0.9,0.95]
uniform float mengerRefrRefl;slider[-1.0,-1.0,1.0]
uniform float mengerGloss;slider[0.0,1.0,1.0]
uniform float mengerSpec;slider[0.0,1.0,1.0]
uniform float mengerSpecExp;slider[1.0,14.0,20.0]
#group Bulb
uniform vec3 bulbColor;color[0.75,0.6,0.5]
uniform vec3 bulbPosition;slider[(-1.0,-1.0,-1.0),(-1.0,0.0,1.7),(1.0,1.0,1.0)]
uniform float bulbRefrRefl;slider[-1.0,0.6,1.0]
uniform float bulbGloss;slider[0.0,0.75,1.0]
uniform float bulbSpec;slider[0.0,1.0,1.0]
uniform float bulbSpecExp;slider[1.0,7.0,20.0]
#group Emissive
uniform bool UseEmissive; checkbox[false]
uniform vec3 lightPosition;slider[(-2.0,-1.0,-2.0),(-1.3,0.9,0.0),(2.0,3.0,2.0)]
uniform vec3 lightColor;color[1.0,0.8,0.75]
uniform float lightStrength;slider[1.0,10.0,16.0]
uniform float lightBloom;slider[1.0,10.0,16.0]
#group Raytracer
uniform int RayBounces;slider[1,6,16]
uniform int RaySteps;slider[64,128,512]
uniform float FudgeFactor;slider[0.1,0.9,1.0]
uniform float HitDistance;slider[0.0001,0.001,0.001]
uniform float SoftShadows;slider[1.0,8.0,16.0]

const float maxDepth=10.0, OOMD=1.0/maxDepth;
const vec3 ior=vec3(1.0,1.52,1.0/1.52);
int iRay=subframe;//taking the user input and making it ready to use
vec3 sunDir=normalize(sunDirection);
float lBloom=pow(2.,17.-lightBloom)-1.999,lStrength=pow(1.05,17.-lightStrength)-1.04999;
float ShadowExp=pow(2.,17.0-SoftShadows);
float minLiteDist,side=1.0; //1.0=outside, -1.0=inside of object

struct material {vec3 color;float refrRefl,gloss,spec,specExp;};
material Mater0=material(mengerColor,mengerRefrRefl,mengerGloss,mengerSpec,pow(2.,mengerSpecExp));
material Mater1=material(bulbColor,bulbRefrRefl,bulbGloss,bulbSpec,pow(2.,bulbSpecExp));
material Mater2=material(floorColor,floorReflection,floorGloss,floorSpec,pow(2.,floorSpecExp));

//some simple distance estimate functions
float DESphere(in vec3 z, float radius){return length(z)-radius;}
float DEBox(in vec3 z, float hlen){return max(abs(z.x),max(abs(z.y),abs(z.z)))-hlen;}
float DECylinder(in vec3 z, float hlen, float radius){return max(length(z.zy)-radius,abs(z.x)-hlen);}
float DERRect(in vec3 z, vec4 radii){return length(max(abs(z)-radii.xyz,0.0))-radii.w;}

//DEMenger(z,3.0,vec3(1.0),2);
//DEMenger(z,2.0,vec3(1.,0.,0.),2);
float DEMenger(in vec3 z, float scale, vec3 offset, int iters)
{
	for (int n = 0; n < iters; n++) {
		z = abs(z);
		if (z.x<z.y)z.xy = z.yx;
		if (z.x<z.z)z.xz = z.zx;
		if (z.y<z.z)z.yz = z.zy;
		z = z*scale - offset*(scale-1.0);
		if(z.z<-0.5*offset.z*(scale-1.0))z.z+=offset.z*(scale-1.0);
	}
	return DERRect(z,vec4(vec3(0.5*(scale-1.0)),0.1))*pow(scale,-float(iters));
}

//DEBulb(z,8.0,1.0,5);
float DEBulb(vec3 z0, float p, float scale, int iters)
{
	vec3 c = z0*scale,z = c;
	float dr = scale,r = length(z),zr,zo,zi;
	for (int n = 0; n < iters && r<2.0; n++) {
		zo = asin(z.z / r) * p;//+time;
		zi = atan(z.y, z.x) * p;
		zr = pow(r, p-1.0);
		dr = dr * zr * p + 1.0;
		z=(r*zr)*vec3(cos(zo)*cos(zi),cos(zo)*sin(zi),sin(zo))+c;
		r = length(z);
	}
	return 0.5 * log(r) * r / dr;
}

float mapL(in vec3 z){//this is the DE for emissive light (use folding etc. to create multiple lights)
	z-=lightPosition;
	return (length(z)-0.2);
	//return DERRect(z,vec4(vec3(0.2),0.05));
	//return DEBulb(z,3.0,5.0,1);//a larger scale "5" makes it 1/5 scale
}

vec2 min2(vec2 d1, vec2 d2){return (d1.x<d2.x)?d1:d2;}//sorts vectors based on .x

vec2 map(in vec3 z)
{//return distance estimate and object id
	vec2 glass=vec2(DEMenger(z,2.0,vec3(1.,0.,0.),2),0.0);//start your object ids at zero
	vec2 box=vec2(DEBulb(z-bulbPosition,4.0,1.0,4),1.0);
	vec2 flr=vec2(z.y+1.0,2.0);//add as many ids as you like
	vec2 lit=vec2((UseEmissive)?mapL(z):1000.0,-2.0);//the id -2 is for emissive light
	minLiteDist=min(minLiteDist,lit.x);//save the closest distance to the light for bloom
	return min2(min2(min2(box,glass),lit),flr);//add as many objects as you like this way
}

material getMaterial( in vec3 z0, in vec3 nor, in float item )
{//get material properties (color,refr/refl,gloss,spec,specExp)
	return (item==0.0)?Mater0:(item==1.0)?Mater1:Mater2; //you can extend this with a texture lookup etc.
}

vec3 getBackground( in vec3 rd ){
	return skyColor+rd*0.1+sunColor*(max(0.0,dot(rd,sunDir))*0.2+pow(max(0.0,dot(rd,sunDir)),256.0)*2.0);
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
	for(int i=0;i<RaySteps && abs(d)>t*HitDistance;i++){
		t+=d=map(ro+t*rd).x*mult;
		if(t>=maxDepth)return vec2(t,-1.0);//-1.0 is id for "hit nothing"
	}
	vec2 h=map(ro+t*rd);//move close to the hit point without fudging
	h.x=t+h.x*side;
	return h;//returns distance, object id
}

vec3 ve=vec3(HitDistance*0.1,0.0,0.0);
vec3 getNormal( in vec3 pos, in float item )
{// get the normal to the surface at the hit point
	return side*normalize(vec3(-map(pos-ve.xyy).x+map(pos+ve.xyy).x,
		-map(pos-ve.yxy).x+map(pos+ve.yxy).x,-map(pos-ve.yyx).x+map(pos+ve.yyx).x));
}

vec4 getEmissiveDir( in vec3 pos)
{//get the direction to a DE based light
	vec2 vt=vec2(mapL(pos),0.0);//find emissive light dir by triangulating its nearest point
	return vec4(-normalize(vec3(-mapL(pos-vt.xyy)+mapL(pos+vt.xyy),
			-mapL(pos-vt.yxy)+mapL(pos+vt.yxy),-mapL(pos-vt.yyx)+mapL(pos+vt.yyx))),vt.x);
}

vec3 powDir(vec3 nor, vec3  dir, float power)
{//creates a biased random sample without penetrating the surface
	float ddn=max(0.01,abs(dot(dir,nor)));
	vec2 r=rand2()*vec2(6.2831853,1.0);
	vec3 nr=(ddn<0.99)?nor:((abs(nor.x)<0.5)?vec3(1.0,0.0,0.0):vec3(0.0,1.0,0.0));
	vec3 sdir=normalize(cross(dir,nr));
	r.y=pow(r.y,1.0/power);
	vec3 ro= normalize(sqrt(1.0-r.y*r.y)*(cos(r.x)*sdir + sin(r.x)*cross(dir,sdir)*ddn) + r.y*dir);
	return (dot(ro,nor)<0.0)?reflect(ro,nor):ro;
}

vec4 scene(vec3 ro, vec3 rd)
{// find color and depth of scene
	vec3 tcol = vec3(0.0),acol=vec3(0.05*ambience),fcol = vec3(1.0),bcol=getBackground(rd),fgcl=bcol;//total color, ambient, mask, background, fog
	float drl=1.0,spec=1.0,frl=0.0,erl=0.0,fgrl=0.0,smld=0.0;//direct and specular masks, important ray lengths
	minLiteDist=1000.0;//for bloom (sketchy)
	bool bLightRay=false; //is this ray used as a shadow check
	vec2 hit=vec2(0.0);int i=0;
	for(i=0; i <RayBounces && dot(fcol,fcol)>0.001 && drl>0.01; i++ )
	{// create light paths iteratively
		hit = intersect( ro, rd ); //find the first object along the ray march
		if(i==0)frl=hit.x;else erl+=hit.x;//frl=depth, emissive light decays so track of the distance
        	if( hit.y >= 0.0 ){//hit something
			if(i>0)acol+=bcol*hit.x*OOMD*ambience; //calc some ambient lighting???
        		ro+= rd * hit.x;// advance ray position
        		vec3 nor = getNormal( ro, hit.y );// get the surface normal
			material m=getMaterial( ro, nor, hit.y );//and material
			if(bLightRay)drl*=abs(m.refrRefl);else fgrl+=hit.x;//if we are checking for shadows then decrease the light unless refl/refr
			//this rather complicated section is just choosing an appropriate but "random" ray direction
			vec3 refl=reflect(rd,nor),newRay=refl;//setting up for a new ray direction and defaulting to a reflection
			float se=m.specExp;//also defaulting to the sample bias for specular light
			vec2 rnd=rand2();//get 2 random numbers
			if(abs(m.refrRefl)>rnd.x){//do we reflect and/or refract
				if(m.refrRefl<0.0){//if the material refracts use the fresnel eq.
					vec3 refr=refract(rd,nor,(side>=0.0)?ior.z:ior.y);//calc the probabilty of reflecting instead
					vec2 ca=vec2(dot(nor,rd),dot(nor,refr)),n=(side>=0.0)?ior.xy:ior.yx,nn=vec2(n.x,-n.y);
					if(rnd.y>0.5*(pow(dot(nn,ca)/dot(n,ca),2.0)+pow(dot(nn,ca.yx)/dot(n,ca.yx),2.0))){newRay=refr;nor=-nor;}
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
			fcol*=mix(m.color,vec3(1.0),se);// modulate the frequency mask dependant on diffuse contribution
		}else{//hit a light so light up the pixel and bail out
			if(i==0)spec=0.0;//lights themselves don't have specularity
			if(hit.y==-1.0)tcol=fcol*bcol;//-1 = background lighting
			else tcol=lightColor/max(lStrength*erl*erl,0.5);//we hit a DE light so make it bright!
			tcol*=(fcol+vec3(spec))*drl;//this adds light (seems suspicious;) light color * (color mask + specular lighting) * light power remaining after bounces
			break;
		}
		bcol=getBackground(rd);//get the background color in this new direction
	}
	if(hit.y>=0.0 || i>0)tcol+=acol*fcol;//add ambient light where needed (not on lights)
	tcol=mix(tcol,fgcl,clamp(log(fgrl*OOMD*fog),0.0,1.0));//fog, still a bit broken :(
	minLiteDist=max(minLiteDist,smld);//light bloom is OR'd in (could be done in post processing)
	return vec4(clamp(max(tcol,lightColor/max(lBloom*minLiteDist*minLiteDist,0.5)),0.0,1.0),frl/maxDepth);
}

vec3 color(vec3 ro, vec3 rd){return scene(ro,rd).rgb;}

/* //for completeness here is an example stand alone main function
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

#preset default
FOV = 1.0
Eye = -0.5,0.5,-1.0
Target = -0.25,0.0,0.0
Up = 0.0,1.0,0.0
#endpreset
