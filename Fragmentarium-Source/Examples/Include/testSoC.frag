//an example using the Sphere of Confusion renderer by eiffie
#include "SoC.frag"

uniform float time; //include this if you are animating

//accumulate color samples in the variable mcol by adding your color whenever bColoring==true
//mcol is a vec4 with mcol.a used for reflections (0..1)
float DE(vec3 z0){
	vec4 c = vec4(z0,1.0),z = c;
	float r = length(z.xyz),zr,zo,zi,p=8.0;
	vec4 trap=vec4(z.xyz,r);
	for (int n = 0; n < 9 && r<2.0; n++) {
		zo = asin(z.z / r) * p-time;
		zi = atan(z.y, z.x) * p;
		zr = pow(r, p-1.0);
		z=zr*vec4(r*vec3(cos(zo)*vec2(cos(zi),sin(zi)),sin(zo)),z.w*p)+c;
		r = length(z.xyz);
		if(bColoring && n<2)trap=vec4(z.xyz,r);
	}
	float d=min(0.5 * log(r) * r / z.w,z0.y+1.0);
	if(bColoring){
		if(abs(d-z0.y-1.0)<0.001)mcol+=vec4(0.3,0.4,0.5,0.125);
		else mcol+=vec4(0.9,0.5,0.3,0.0)*min(trap.w,1.0)+vec4(sin(trap.gbr)*0.2,0.0);
	}
	return d;
}


#preset default
FOV = 0.62536
Eye = 0.205,0.45,-2.252
Target = 0,0,0
Up = 0,1,0
FudgeFactor = 1
Specular = 1.6456
SpecularExp = 16.364
FocalPlane = 1.3068
Aperture = 0.02604
MaxRaySteps = 97
MaxShadowSteps = 32
DiffuseContrast = 0.7
ShadowContrast = 0.66102
ShadowCone = 0.05
LightColor = 1,0.9,0.4
LightDir = 0.9394,0.75758,-0.69696
BGColor = 0.5,0.5,0.5

#endpreset
