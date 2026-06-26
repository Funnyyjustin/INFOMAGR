#include <CL/cl.h>

struct MaterialNew
{
    cl_float4 info; // albedo, fuzz, refractive index
    cl_float4 type; // 0 = diffuse, 1 = metal, 2 = dielectic
};

struct SphereNew
{
    cl_float4 center;
    cl_float4 radius;
};

struct Configuration
{
    cl_float4 pixel00_loc;
    cl_float4 camera_center;
    cl_float4 pixel_delta_u;
    cl_float4 pixel_delta_v;
    cl_int sphere_count;
    cl_int screen_width;
    cl_int max_depth;
    cl_int num_samples;

    cl_int use_grid;
    cl_int padding[3]; // padding
    cl_float4 worldMin;
    cl_float4 worldMax;
    cl_float4 cellDimensions;
    cl_int boxesAlongX;
    cl_int boxesAlongY;
    cl_int boxesAlongZ;
    cl_int padding2; // padding
};