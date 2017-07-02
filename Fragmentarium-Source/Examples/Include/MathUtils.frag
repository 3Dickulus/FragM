#donotrun

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
// required for Wang hash
#extension GL_ARB_shader_bit_encoding : enable
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_gpu_shader5 : enable
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
        return vec2(wang_hash(ix),wang_hash(iy)) / 4294967296.0;
}

vec3 wang_hash_fp(vec3 v)
{
        uint ix = floatBitsToUint(v.x);
        uint iy = floatBitsToUint(v.y);
        uint iz = floatBitsToUint(v.z);
        return vec3(wang_hash(ix),wang_hash(iy),wang_hash(iz)) / 4294967296.0;
}

#endif
