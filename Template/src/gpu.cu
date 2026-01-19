#include "cuda.h"
#include "cuda_runtime.h"
#include <stdio.h>

#include "gpu.h"

__global__ void render_kernel()
{
	printf("Hello!\n");
}

void launch_kernels(int width, int height)
{
	dim3 threads(16, 16);
	dim3 grid((width + threads.x - 1) / threads.x, (height + threads.y - 1) / threads.y);

    // If there's a red underline here, do not be scared - that is normal if you use IntelliSense!
    // Just ignore it, and if there's a supposed error and the program doesn't compile,
    // then the problem lies elsewhere most likely! :)
    render_kernel<<<grid, threads>>>();

    // Check for launch errors
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("Kernel launch error: %s\n", cudaGetErrorString(err));
    }

    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        printf("CUDA sync error: %s\n", cudaGetErrorString(err));
    }
}