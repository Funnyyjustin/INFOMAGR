#include <CL/cl.h>

struct alignas(16) MaterialNew
{
    cl_int type; // 0 = diffuse, 1 = metal, 2 = dielectic
    cl_float4 info; // albedo, fuzz, refractive index
};

struct alignas(128) SphereNew
{
    cl_float4 center;
    cl_float4 color;
    cl_float radius;
};

struct alignas(16) RayNew
{
    cl_float4 origin;
    cl_float4 direction;
};

struct alignas(256) Configuration
{
    cl_float4 pixel00_loc;
    cl_float4 camera_center;
    cl_float4 pixel_delta_u;
    cl_float4 pixel_delta_v;
    cl_int sphere_count;
    cl_int screen_width;
};