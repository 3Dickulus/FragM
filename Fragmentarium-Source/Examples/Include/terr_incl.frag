#donotrun
//return min(DE(p), surf(p));

#include "Classic-Noise.frag"

#group Surface
//mode of surface, 1 = none, 2 = kaliset, 3 = terrain
//uniform int SurfaceMode; slider[1,1,4]
//Rotate Object
uniform vec3 RotXYZ; slider[(-180,-180,-180),(0,0,0),(180,180,180)]
//Move Object
uniform vec3 MovXYZ; slider[(-5.0,-5.0,-5.0),(0.0,0.0,0.0),(5.0,5.0,5.0)];
//Precise Move Object
uniform vec3 FineMove; slider[(-0.1,-0.1,-0.1),(0.0,0.0,0.0),(0.1,0.1,0.1)];


uniform int TerrIter;  slider[0,10,40]
uniform float TerrSlope; slider[-1,-0.7,1]
uniform float TerrFreq; slider[1,2,10]
uniform float TerrOffset; slider[-10,0.4,10]
uniform float TerrAmp; slider[0,0.3,1]
// wave direction
uniform vec3 WaveDir; slider[(-1.0,-1.0,-1.0),(0.0,0.0,1.0),(1.0,1.0,1.0)]
// wave speed
uniform float WaveSpeed; slider[0.0,1.0,2.0]




float height(vec3 pos) {
	float A = 1.0;
	float B = 1.0;
	float r = 0.0;

        vec3 p = pos + vec3(WaveSpeed*(WaveDir*time));

	for (int j = 0; j < TerrIter; j++) {
		r+= B*cnoise(A*(p.xz)+TerrOffset);
		A*=TerrFreq;
		B*=TerrAmp;
	}
	return r;
}

float terrain(vec3 pos) {
	float dr = height(pos);
	return (pos.y-TerrSlope*dr);
}

float surf(vec3 pos) {
	float KDist = 0.0;
	mat4 RotatiX;
	RotatiX = rotationMatrix(vec3(1.0,0.0,0.0), RotXYZ.x)*rotationMatrix(vec3(0.0,1.0,0.0), RotXYZ.y)*rotationMatrix(vec3(0.0,0.0,1.0), RotXYZ.z)*translate(MovXYZ+FineMove);
	pos =(RotatiX*vec4(pos, 1.0)).xyz;
	float k;
	float sc;
	/*
if (SurfaceMode==2) {
		k = KalisetSurf(pos);
		vec3 kdir=normalize(s_Dir);
		if (KTex) orbitTrap=vec4(1,1,1,1)*(k*Color_Scale+Color_Offset);
		if (KHeightmap) sc=k*Strength; pos+=vec3(sc,sc,sc)*Strength2;
		KDist = abs(dot(kdir,pos)-sHeight);
	} if (SurfaceMode==3) {
		KDist = terrain(pos);
		//orbitTrap=vec4(TColor,0);
	} if (SurfaceMode==4) {
		float dr = height(pos);
		float k = KalisetSurf(pos);
		orbitTrap=vec4(1,1,1,1)*(k*Color_Scale+Color_Offset);
		dr += k;
		sc=k*Strength; pos+=vec3(sc,sc,sc)*Strength2;
		KDist = (pos.y-TerrSlope*dr);
	}
	if (SurfaceMode==1){KDist=1;}
*/
	KDist = terrain(pos);
	return min(KDist,1.);
}
