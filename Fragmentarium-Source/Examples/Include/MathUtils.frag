#donotrun
#ifdef WANG_HASH
// required for Wang hash, placing this here means this file must be the first #included file
#extension GL_ARB_shader_bit_encoding : enable
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_gpu_shader5 : enable
#endif

#extension GL_ARB_gpu_shader_int64 : enable

// Standard matrices

// Return rotation matrix for rotating around vector v by angle
mat3  rotationMatrix3(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));
	
	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}

mat3 rotationMatrixXYZ(vec3 v) {
	return rotationMatrix3(vec3(1.0,0.0,0.0), v.x)*
	rotationMatrix3(vec3(0.0,1.0,0.0), v.y)*
	rotationMatrix3(vec3(0.0,0.0,1.0), v.z);
}

// Return rotation matrix for rotating around vector v by angle
mat4  rotationMatrix(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));
	
	return mat4(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y, 0.0,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x, 0.0,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z, 0.0,
		0.0, 0.0, 0.0, 1.0);
}

mat4 translate(vec3 v) {
	return mat4(1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0,
		v.x,v.y,v.z,1.0);
}

mat4 scale4(float s) {
	return mat4(s,0.0,0.0,0.0,
		0.0,s,0.0,0.0,
		0.0,0.0,s,0.0,
		0.0,0.0,0.0,1.0);
}

#ifdef WANG_HASH
// http://www.fractalforums.com/index.php?topic=22721.msg88910#msg88910
uint wang_hash(uint seed)
{
        seed = (seed ^ 61u) ^ (seed >> 16u);
        seed *= 9u;
        seed = seed ^ (seed >> 4u);
        seed *= 0x27d4eb2du;
        seed = seed ^ (seed >> 15u);
        return seed ;
}


// Wrapper for getting from float to ints. This certainly looses precision. I imagine we could do better here.
float wang_hash_fp(float v)
{
        uint ix = floatBitsToUint(v);
        return float(wang_hash(ix)) / 4294967296.0;
}

vec2 wang_hash_fp(vec2 v)
{
        uint ix = floatBitsToUint(v.x);
        uint iy = floatBitsToUint(v.y);
        // I use two hash calls to untangle pos.x and pos.y
        return vec2(float(wang_hash(wang_hash(ix)+iy)), float(wang_hash(wang_hash(iy)+ix))) / 4294967296.0;
}

vec3 wang_hash_fp(vec3 v)
{
        uint ix = floatBitsToUint(v.x);
        uint iy = floatBitsToUint(v.y);
        uint iz = floatBitsToUint(v.z);
        return vec3(float(wang_hash(wang_hash(wang_hash(ix)+iy)+iz)),float(wang_hash(wang_hash(wang_hash(iy)+ix)+iz)),float(wang_hash(wang_hash(wang_hash(iz)+ix)+iy))) / 4294967296.0;
}
// Wrapper for getting from float to ints. This certainly looses precision. I imagine we could do better here. //float wang_hash_fp(vec2 pos)
float rand(float v)
{
        uint ix = floatBitsToUint(v);
        return float(wang_hash(ix)) / 4294967296.0;
}

float rand(vec2 pos){
	uint ix = floatBitsToUint(pos.x);
	uint iy = floatBitsToUint(pos.y);
    // I use two hash calls to untangle pos.x and pos.y
	return float(wang_hash(wang_hash(ix)+iy)) / 4294967296.0;
}
//for iq-clouds
float rand(vec3 pos){
	uint ix = floatBitsToUint(pos.x);
	uint iy = floatBitsToUint(pos.y);
	uint iz = floatBitsToUint(pos.z);
	return float(wang_hash(wang_hash(wang_hash(ix)+iy)+iz)) / 4294967296.0;
}

vec2 rand2(vec2 pos){
	uint ix = floatBitsToUint(pos.x);
	uint iy = floatBitsToUint(pos.y);
    // I use two hash calls to untangle pos.x and pos.y
	return vec2(float(wang_hash(wang_hash(ix)+iy)) / 4294967296.0, float(wang_hash(wang_hash(iy)+ix)) / 4294967296.0);
}

#else
float rand(float v) {
// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return fract(sin(dot(v ,78.233)) * 43758.5453);
}

float rand(vec2 pos) {
// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
    return fract(sin(dot(pos.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

//for iq-clouds
float rand(vec3 co){
        // implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
        return fract(sin(dot(co*0.123,vec3(12.9898,78.233,112.166))) * 43758.5453);
        }

vec2 rand2(vec2 co){
	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return
	vec2(fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453),
		fract(cos(dot(co.xy ,vec2(4.898,7.23))) * 23421.631));
}

#endif
