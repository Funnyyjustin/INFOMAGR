// Random functions

// These functions are shamelessly stolen from the INFOMOV-P2 template
// From: https://github.com/angeloschatzimparmpas/INFOMOV_Assignment_2_Template
uint WangHash(uint s) { s = (s ^ 61) ^ (s >> 16), s *= 9, s = s ^ (s >> 4), s *= 0x27d4eb2d, s = s ^ (s >> 15); return s; }
uint RandomInt(uint* s) { *s ^= *s << 13, * s ^= *s >> 17, * s ^= *s << 5; return *s; }
float RandomFloat(uint* s) { return RandomInt(s) * 2.3283064365387e-10f; } // [0..1)

// Configuration struct

typedef struct
{
	float4 pixel00_loc;
	float4 camera_center;
	float4 pixel_delta_u;
	float4 pixel_delta_v;
	int sphere_count;
	int screen_width;
} Configuration;

// Ray struct and functions

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

int near_zero(float4 v)
{
	return 0;
}

float4 sample_square(int index)
{
	uint seed = WangHash((index + 1) * 17);
	float r1 = RandomFloat(&seed);
	float r2 = RandomFloat(&seed);
	return (float4)(r1 - 0.5f, r2 - 0.5f, 0, 0);
}

Ray get_ray(__global Configuration* conf, int x, int y, int index)
{
	Ray r;

	float4 offset = sample_square(index);
	float4 pixel_sample = conf->pixel00_loc + ((x + offset.x) * conf->pixel_delta_u) + ((y + offset.y) * conf->pixel_delta_v);
	r.origin = conf->camera_center;
	r.dir = pixel_sample - conf->camera_center;

	return r;
}

// Material struct and functions

typedef struct
{
	int type; // 0 = diffuse, 1 = metal, 2 = dielectic
	float4 info; // albedo, fuzz, refractive index
} Material;

typedef struct
{
	float4 att;
	Ray scat;
	bool success;
} ScatReturn;

ScatReturn scatter(Ray r_in, Material m, float4 p, float4 normal)
{
	ScatReturn sr;

	// Scatter diffuse material
	if (m.type == 0)
	{
		float4 scat_dir = normal + (float4)(0, 0, 0, 0); // TODO: get random unit vector

		if (near_zero(scat_dir))
			scat_dir = normal;

		Ray r;
		r.origin = p;
		r.dir = scat_dir;

		sr.att = m.info;
		sr.scat = r;
		success = true;
	}

	return sr;
}

// Sphere struct and functions

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
		return 0;

	return 1;
}

// General functions

void set_color(__global float4* img, int index, float4 color)
{
	img[index].x = color.x;
	img[index].y = color.y;
	img[index].z = color.z;
	img[index].w = 0;
}

float4 traverse(Ray r, __global Sphere* spheres, int sphere_count, int depth)
{
	Ray ray = r;
	float4 color = (float4)(1, 1, 1, 0);

	for (int i = depth; i > 0; i--)
	{
		// Max depth reached
		if (depth <= 0)
			return color * float4(0, 0, 0, 0);

		// Check all objects in scene
		for (int i = 0; i < sphere_count; i++)
		{
			Sphere s = spheres[i];

			// There is a hit; bounce/scatter it based on the material
			if (hit(r, s) == 1)
			{
				// TODO: get intersection point and normal
				float4 p;
				float4 normal;

				// TODO: get material
				Material m;

				ScatReturn sr = scatter(r, m, p, normal);

				if (sr.success)
				{
					color *= sr.att;
					ray = sr.scat;
				}
			}
		}
	}

	// No hit has been found; return background gradient
	float4 unit_dir = normalize(r.dir);
	float a = 0.5f * (unit_dir.y + 1.0f);
	color = (1.0f - a) * (float4)(1.0f, 1.0f, 1.0f, 0) + a * (float4)(0.5f, 0.7f, 1.0f, 0);
	return color;
}

__kernel void get_color(__global float4* img, __global Configuration* conf, __global Sphere* spheres)
{
	int index = get_global_id(0);
	int x = index % conf->screen_width;
	int y = index / conf->screen_width;

	float4 color = (float4)(0, 0, 0, 0);
	int num_samples = 10;

	for (int i = 0; i < num_samples; i++)
	{
		Ray r = get_ray(conf, x, y, index);
		color += traverse(r, spheres, conf->sphere_count, 1);
	}

	color *= (float)(1.0 / (float)num_samples);
	set_color(img, index, color);
}