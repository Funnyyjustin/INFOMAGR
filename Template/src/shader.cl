typedef struct {
	float x;
	float y;
	float z;
	float w;
} Vec4;

__kernel void color(__global Vec4* arr)
{
	int index = get_global_id(0);
	arr[index].x = 1;
	arr[index].y = 0;
	arr[index].z = 0;
	arr[index].w = 0;
}