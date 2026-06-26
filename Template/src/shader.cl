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
	int max_depth;
	int num_samples;
	
	// Grid configs
	int use_grid;
	float4 worldMin;
	float4 worldMax;
	float4 cellDimensions;
	int boxesAlongX;
	int boxesAlongY;
	int boxesAlongZ;
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
	float eps = 1e-6f;
	if (fabs(v.x) < eps && fabs(v.y) < eps && fabs(v.z) < eps)
		return 1;
	return 0;
}

int contains(float x, float min, float max)
{
	if (x >= min && x <= max)
		return 1;

	return 0;
}

float4 set_normal(Ray r, float4 normal)
{
	float dotproduct = dot_product(r.dir, normal);

	if (dotproduct < 0)
		return normal;

	return -normal;
}

float4 sample_square(uint *seed)
{
	float r1 = RandomFloat(seed);
	float r2 = RandomFloat(seed);
	return (float4)(r1 - 0.5f, r2 - 0.5f, 0, 0);
}

float4 random_unit_vector(uint *seed)
{
	while (1)
	{
		float r1 = RandomFloat(seed);
		float r2 = RandomFloat(seed);
		float r3 = RandomFloat(seed);
		float4 v = (float4)(r1 * 2.0f - 1.0f, r2 * 2.0f - 1.0f, r3 * 2.0f - 1.0f, 0);
		if (dot_product(v, v) < 1.0f)
			return v;
	}
}

Ray get_ray(__global Configuration* conf, int x, int y, uint *seed)
{
	Ray r;

	float4 offset = sample_square(seed);
	float4 pixel_sample = conf->pixel00_loc + ((x + offset.x) * conf->pixel_delta_u) + ((y + offset.y) * conf->pixel_delta_v);
	r.origin = conf->camera_center;
	r.dir = normalize(pixel_sample - conf->camera_center);

	return r;
}

// Material struct and functions

typedef struct
{
	float4 info; // albedo, fuzz, refractive index
	float4 type; // 0 = diffuse, 1 = metal, 2 = dielectic
} Material;

typedef struct
{
	float4 att;
	Ray scat;
	int success;
} ScatReturn;

float4 reflect(float4 v, float4 n)
{
	return v - 2 * dot_product(v, n) * n;
}

float4 refract(float4 v, float4 n, float eps)
{
	float cos_theta = fmin(dot_product(-v, n), 1.0f);
	float4 r_out_perp = eps * (v + cos_theta * n);
	float4 r_out_par = -sqrt(fabs(1.0f - length_squared(r_out_perp))) * n;
	return r_out_perp + r_out_par;
}

float reflectance(float cosine, float ri)
{
	float r0 = (1 - ri) / (1 + ri);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

ScatReturn scatter(Ray r_in, Material m, float4 p, float4 normal, int front_face, uint *seed)
{
	ScatReturn sr;
	sr.success = 0;

	// Scatter diffuse material
	if (m.type.x == 0)
	{
		float4 scat_dir = normalize(normal + random_unit_vector(seed));

		if (near_zero(scat_dir) == 1)
			scat_dir = normal;

		Ray r;
		r.origin = p + normal * 0.001f;
		r.dir = scat_dir;

		sr.att = m.info;
		sr.scat = r;
		sr.success = 1;
	}
	// Scatter metal material
	else if (m.type.x == 1)
	{
		float4 reflected = normalize(reflect(r_in.dir, normal)) + (m.info.w * random_unit_vector(seed));
		
		Ray r;
		r.origin = p;
		r.dir = reflected;

		sr.att = (float4)(m.info.x, m.info.y, m.info.z, 0);
		sr.scat = r;

		if (dot_product(sr.scat.dir, normal) > 0)
			sr.success = 1;
		else sr.success = 0;
	}
	// Scatter dieletric material
	else if (m.type.x == 2)
	{
		sr.att = (float4)(1.0f, 1.0f, 1.0f, 0);

		float ri;

		if (front_face == 1)
			ri = 1.0f / m.info.x;
		else ri = m.info.x;

		float4 unit_dir = normalize(r_in.dir);
		
		float cos_theta = fmin(dot_product(-unit_dir, normal), 1.0f);
		float sin_theta = sqrt(1.0f - cos_theta * cos_theta);

		bool cannot_refract = ri * sin_theta > 1.0f;
		float4 dir;

		if (cannot_refract || reflectance(cos_theta, ri) > RandomFloat(seed))
			dir = reflect(unit_dir, normal);
		else
			dir = refract(unit_dir, normal, ri);

		Ray r;
		r.origin = p;
		r.dir = dir;

		sr.scat = r;
		sr.success = 1;
	}

	return sr;
}

// Sphere struct and functions

typedef struct
{
	float4 center;
	float4 radius;
} Sphere;

typedef struct
{
	float4 p; // location of intersection
	float4 normal; // normal of intersection
	int hit; // 0 is no hit, 1 is hit
	float t;
	int front_face;
} HitRecord;

HitRecord hit(Ray r, Sphere s)
{
	HitRecord hr;
	hr.hit = 0;
	hr.p = (float4)(0, 0, 0, 0);
	hr.normal = (float4)(0, 0, 0, 0);
	hr.front_face = 0;

	float4 oc = s.center - r.origin;
	float a = length_squared(r.dir);
	float h = dot_product(r.dir, oc);
	float c = length_squared(oc) - (s.radius.x * s.radius.x);
	float d = h * h - a * c;

	if (d < 0.0f)
	{	
		hr.hit = 0;
		hr.p = (float4)(0, 0, 0, 0);
		hr.normal = (float4)(0, 0, 0, 0);
		hr.front_face = 0;
		return hr;
	}

	float sd = sqrt(d);

	float t = (h - sd) / a;
	if (t <= 0.001f)
	{
		t = (h + sd) / a;
		if (t <= 0.001f)
			return hr;
	}

	hr.hit = 1;
	hr.t = t;
	hr.p = r.origin + t * r.dir;
	float4 outward_normal = (hr.p - s.center) / s.radius.x;
	float4 new_normal = set_normal(r, outward_normal);

	if (all(fabs(new_normal - outward_normal) < 0.001f))
		hr.front_face = 1;
	else hr.front_face = 0;

	hr.normal = new_normal;

	return hr;
}

void goTo(float* maxT, float delta, float entry_t)
{
	if (*maxT == INFINITY) return;
	if (*maxT < entry_t)
	{
		float d = entry_t - *maxT;
		float num_steps = ceil(d / delta);
		*maxT += num_steps * delta;
	}
}

float nextBoundaryT(int start, float r_origin, float r_dir, float min, float cellDim, int step)
{
	// ray will not hit current axis
    if (r_dir == 0) return INFINITY;

    int step_;
    if (step > 0) step_ = 1;
    else step_ = 0;

    float boundary = min + (start + step_) * cellDim;
    return (boundary - r_origin) / r_dir;
}

int3 getVoxelIndex(float4 p, float4 worldMin, float4 cellDims, int boxesAlongX, int boxesAlongY, int boxesAlongZ)
{
	float4 offset = p - worldMin;
	int3 idx;
	idx.x = (int)floor(offset.x / cellDims.x);
	idx.y = (int)floor(offset.y / cellDims.y);
	idx.z = (int)floor(offset.z / cellDims.z);

	idx.x = clamp(idx.x, 0, boxesAlongX - 1);
	idx.y = clamp(idx.y, 0, boxesAlongY - 1);
	idx.z = clamp(idx.z, 0, boxesAlongZ - 1);

	return idx;
}

int index3(int x, int y, int z, int boxesAlongX, int boxesAlongY)
{
	return x + boxesAlongX * (y + boxesAlongY * z);
}

// General functions

void set_color(__global float4* img, int index, float4 color)
{
	img[index].x = color.x;
	img[index].y = color.y;
	img[index].z = color.z;
	img[index].w = 0;
}

float4 gridTraverse(Ray r, __global Sphere* spheres, __global Material* materials, __global int2* voxels, 
	__global int* voxelIndices, __global Configuration* conf, uint* seed)
{
	Ray ray = r;
	float4 throughput = (float4)(1, 1, 1, 0);
	int depth = conf->max_depth;

	for (int bounce = 0; bounce <= depth; bounce++)
    {
        float entry_t = 0.001f;
        float exit_t = 1e30f;

        // Check if ray hits the grid bounding box at all (bug preserved: uses max instead of min for exit_t)
        float4 invDir = (float4)(1.0f / ray.dir.x, 1.0f / ray.dir.y, 1.0f / ray.dir.z, 0);
        float4 t0 = (conf->worldMin - ray.origin) * invDir;
        float4 t1 = (conf->worldMax - ray.origin) * invDir;
        float4 tmin4 = fmin(t0, t1);
        float4 tmax4 = fmax(t0, t1);
        float tenter = fmax(fmax(tmin4.x, tmin4.y), tmin4.z);
        float texit  = fmin(fmin(tmax4.x, tmax4.y), tmax4.z);

        if (tenter > texit || texit < 0.001f)
        {
            // Ray misses grid — return sky
            float4 unit_dir = normalize(ray.dir);
            float a = 0.5f * (unit_dir.y + 1.0f);
            float4 sky = (1.0f - a) * (float4)(1.0f, 1.0f, 1.0f, 0) + a * (float4)(0.5f, 0.7f, 1.0f, 0);
            return throughput * sky;
        }

        entry_t = max(0.001f, tenter);
        exit_t  = max(1e30f,  exit_t);  // bug preserved from C++

        float4 entryPoint = ray.origin + entry_t * ray.dir;

        int3 vi = getVoxelIndex(entryPoint, conf->worldMin, conf->cellDimensions,
                                conf->boxesAlongX, conf->boxesAlongY, conf->boxesAlongZ);

        int stepX = (ray.dir.x > 0) ? 1 : -1;
        int stepY = (ray.dir.y > 0) ? 1 : -1;
        int stepZ = (ray.dir.z > 0) ? 1 : -1;

        float deltaX = conf->cellDimensions.x / fabs(ray.dir.x);
        float deltaY = conf->cellDimensions.y / fabs(ray.dir.y);
        float deltaZ = conf->cellDimensions.z / fabs(ray.dir.z);

        float maxT_x = nextBoundaryT(vi.x, ray.origin.x, ray.dir.x, conf->worldMin.x, conf->cellDimensions.x, stepX);
        float maxT_y = nextBoundaryT(vi.y, ray.origin.y, ray.dir.y, conf->worldMin.y, conf->cellDimensions.y, stepY);
        float maxT_z = nextBoundaryT(vi.z, ray.origin.z, ray.dir.z, conf->worldMin.z, conf->cellDimensions.z, stepZ);

        goTo(&maxT_x, deltaX, entry_t);
        goTo(&maxT_y, deltaY, entry_t);
        goTo(&maxT_z, deltaZ, entry_t);

        int xi = vi.x, yi = vi.y, zi = vi.z;

        // DDA traversal (Amanatides & Woo)
        bool hit_anything = false;
        HitRecord best;
        int best_id = -1;
        float closest = 1e30f;

        while (true)
        {
            if (xi < 0 || xi >= conf->boxesAlongX ||
                yi < 0 || yi >= conf->boxesAlongY ||
                zi < 0 || zi >= conf->boxesAlongZ)
                break;

            int voxelIdx = index3(xi, yi, zi, conf->boxesAlongX, conf->boxesAlongY);
            int2 voxel = voxels[voxelIdx];
            int offset = voxel.x;
            int count  = voxel.y;

            for (int k = 0; k < count; k++)
            {
                int sphereIdx = voxelIndices[offset + k];
                HitRecord hr = hit(ray, spheres[sphereIdx]);
                if (hr.hit == 1 && hr.t < closest)
                {
                    closest   = hr.t;
                    best      = hr;
                    best_id   = sphereIdx;
                    hit_anything = true;
                }
            }

            if (hit_anything)
			{
				float nextBoundary = min(maxT_x, min(maxT_y, maxT_z));
				// break if hit is closer than the next voxel boundary
				if(closest <= nextBoundary)
					break;
			}
                

            // Step to next voxel
            if (maxT_x < maxT_y)
            {
                if (maxT_x < maxT_z) 
					{ xi += stepX; maxT_x += deltaX; }
                else                  
					{ zi += stepZ; maxT_z += deltaZ; }
            }
            else
            {
                if (maxT_y < maxT_z) 
					{ yi += stepY; maxT_y += deltaY; }
                else                  
					{ zi += stepZ; maxT_z += deltaZ; }
            }
        }

        if (hit_anything)
        {
            Material m = materials[best_id];
            ScatReturn sr = scatter(ray, m, best.p, best.normal, best.front_face, seed);
            if (sr.success == 1)
            {
                throughput *= sr.att;
                ray = sr.scat;
            }
            else
            {
                return throughput * (float4)(0, 0, 0, 0);
            }
        }
        else
        {
            float4 unit_dir = normalize(ray.dir);
            float a = 0.5f * (unit_dir.y + 1.0f);
            float4 sky = (1.0f - a) * (float4)(1.0f, 1.0f, 1.0f, 0) + a * (float4)(0.5f, 0.7f, 1.0f, 0);
            return throughput * sky;
        }
    }

    return throughput;
}

float4 traverse(Ray r, __global Sphere* spheres, __global Material* materials, int sphere_count, int depth, uint *seed)
{
	Ray ray = r;
	float4 throughput = (float4)(1, 1, 1, 0);

	for (int i = 0; i <= depth; i++)
	{
		int hit_anything = 0;

		float closest = 1e30f;
		int id = -1;
		HitRecord best;

		// Check all objects in scene for hit
		for (int i = 0; i < sphere_count; i++)
		{
			Sphere s = spheres[i];
			
			HitRecord hr = hit(ray, s);

			// There is a hit; save closest info
			if (hr.hit == 1)
			{
				hit_anything = 1;

				if (hr.t < closest)
				{
					closest = hr.t;
					best = hr;
					id = i;
				}
			}
		}

		// Hit has been found
		if (id >= 0)
		{
			Material m = materials[id];
			ScatReturn sr = scatter(ray, m, best.p, best.normal, best.front_face, seed);

			if (sr.success == 1)
			{
				throughput *= sr.att;
				ray = sr.scat;
			}
			else
			{
				throughput *= (float4)(0, 0, 0, 0);
				return throughput;
			}
		}
		// No hit has been found; return background gradient
		else
		{
			float4 unit_dir = normalize(ray.dir);
			float a = 0.5f * (unit_dir.y + 1.0f);
			float4 sky = (1.0f - a) * (float4)(1.0f, 1.0f, 1.0f, 0) + a * (float4)(0.5f, 0.7f, 1.0f, 0);
			return throughput * sky;
		}
	}
		
	return throughput;
}

__kernel void get_color(__global float4* img, __global Configuration* conf, __global Sphere* spheres, __global Material* materials, __global int2* voxels, __global int* voxelIndices)
{
	int index = get_global_id(0);
	uint seed = WangHash((index + 1) * 17);

	int x = index % conf->screen_width;
	int y = index / conf->screen_width;

	float4 color = (float4)(0, 0, 0, 0);
	int samples = conf->num_samples;

	for (int i = 0; i < samples; i++)
	{
		Ray r = get_ray(conf, x, y, &seed);
		if (conf->use_grid)
			color += gridTraverse(r, spheres, materials, voxels, voxelIndices, conf, &seed);
		else
			color += traverse(r, spheres, materials, conf->sphere_count, conf->max_depth, &seed);
	}

	color *= (float)(1.0 / (float)samples);
	set_color(img, index, color);
}