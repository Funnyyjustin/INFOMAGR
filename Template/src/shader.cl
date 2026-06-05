typedef struct
{
	float4 center;
	float4 color;
	float radius;
} Sphere;

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

int hit(Ray r, Sphere s)
{
	float4 oc = s.center - r.origin;
	float a = length_squared(r.dir);
	float h = dot_product(r.dir, oc);
	float c = length_squared(oc) - (s.radius * s.radius);
	float d = h * h - a * c;

	if (d < 0)
		return 0;

	return 1;
}

void set_color(__global float4* img, int index, float4 color)
{
	img[index].x = color.x;
	img[index].y = color.y;
	img[index].z = color.z;
	img[index].w = 0;
}

float4 traverse(Ray r, __global Sphere* spheres, int sphere_count, int depth)
{
	float4 color = (float4)(0, 0, 0, 0);

	if (depth <= 0)
		return color;

	for (int i = 0; i < sphere_count; i++)
	{
		Sphere s = spheres[i];
		if (hit(r, s) == 1)
			return s.color;
	}

	float4 unit_dir = normalize(r.dir);
	float a = 0.5f * (unit_dir.y + 1.0f);
	color = (1.0f - a) * (float4)(1.0f, 1.0f, 1.0f, 0) + a * (float4)(0.5f, 0.7f, 1.0f, 0);
	return color;
}

__kernel void get_color(__global float4* img, __global Ray* rays, __global Sphere* spheres, int sphere_count)
{
	int index = get_global_id(0);
	Ray r = rays[index];
	float4 color = traverse(r, spheres, sphere_count, 1);
	set_color(img, index, color);
}