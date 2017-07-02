#donotrun
#group KalisetTexture
uniform int KIterations;  slider[0,35,100]
uniform float KScale;  slider[0,1.3,3.0]
uniform float KZoom;  slider[0,1,3.0]
uniform vec3 KFold; slider[(0,0,0),(0,0,0),(1,1,1)]
uniform vec3 KJulia; slider[(-1,-1,-1),(-0.5,-0.5,-0.5),(0,0,0)]
uniform vec3 KPosition; slider[(-10,-10,-10),(0,0,0),(10,10,10)]
uniform float KBailout;  slider[0,0,50]
//uniform bool KHeightmap; checkbox[false]
uniform float KFinalScale;  slider[0,1,1]
uniform float KFinalPow;  slider[0,1,5]
uniform vec3 KRotVector; slider[(-1,-1,-1),(1,1,1),(1,1,1)]
uniform float KRotAngle; slider[-180,0,180]
uniform bool KExpSmoothing; checkbox[true]


float Kaliset(vec3 pos) {
	mat3 rt=rotationMatrix3(normalize(KRotVector),KRotAngle);
	vec3 p = (pos+KPosition)*KZoom, p0 = KJulia;  
	int i=0;
	float l=1.;
	float ln=0.;
	float lnprev=0.;
	float expsmooth=0.;
	for (i=0; i<KIterations; i++) {
		p.xyz=abs(p.xyz+KFold.xyz)-KFold.xyz;
		p*=rt;
		p=p/dot(p,p);
		p=p*KScale+p0;
		lnprev=ln;
		ln=length(p);
		if (ln>KBailout && KBailout>0.) break;
		expsmooth+=exp(-1./abs(lnprev-ln));
	}
	float fv=KExpSmoothing?expsmooth:ln;
	fv=pow(fv,KFinalPow)*KFinalScale;
	return fv;
}

