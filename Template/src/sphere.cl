typedef struct
{
	float4 center;
	float4 color;
	float radius;
} Sphere;

int hit(Ray r, Sphere s)
{
	float4 oc = s.center - r.origin;
	float a = length_squared(r.dir);
	float h = dot_product(r.dir, oc);
	float c = length_squared(oc) - (s.radius * s.radius);
	float d = h * h - a * c;

	if (d < 0)
		return -1.0f;

	return (h - sqrt(d)) / a;
}