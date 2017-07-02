//return min(DE(p), surf(p));

#ifdef USE_TERRAIN

#include "Classic-Noise.frag"

#group Surface
//Rotate Object
uniform vec3 RotXYZ; slider[(-180,-180,-180),(0,0,0),(180,180,180)]
//Move Object
uniform vec3 MovXYZ; slider[(-15.0,-15.0,-15.0),(0.0,0.0,0.0),(15.0,15.0,15.0)];
//Precise Move Object
uniform vec3 FineMove; slider[(-0.1,-0.1,-0.1),(0.0,0.0,0.0),(0.1,0.1,0.1)];


uniform int TerrIter;  slider[0,10,40]
uniform float TerrSlope; slider[-1,-0.7,1]
uniform float TerrFreq; slider[1,2,10]
uniform float TerrOffset; slider[-1,0.4,1]
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
	KDist = terrain(pos);
	return min(KDist,1.);
}
#endif
