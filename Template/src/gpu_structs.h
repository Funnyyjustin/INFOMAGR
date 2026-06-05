#include <CL/cl.h>

struct alignas(64) SphereNew
{
    cl_float4 center;
    cl_float4 color;
    float radius;
};

struct alignas(16) RayNew
{
    cl_float4 origin;
    cl_float4 direction;
};