typedef struct
{
	float4 origin;
	float4 dir;
} Ray;

float length_squared(float4 v)
{
	float res = v.x * v.x + v.y * v.y + v.z * v.z;
	return res;
}

float dot_product(float4 v1, float4 v2)
{
	float res = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return res;
}