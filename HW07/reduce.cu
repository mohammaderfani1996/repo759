#include <cuda.h>
#include "reduce.cuh"

__global__ void reduce_kernel(float *g_idata, float *g_odata, unsigned int n) {
    extern __shared__ float sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int idx = blockIdx.x * blockDim.x * 2 + threadIdx.x;

    // Load 2 elements per thread and add them during load
    float val = 0.0f;
    if (idx < n)
        val += g_idata[idx];
    if (idx + blockDim.x < n)
        val += g_idata[idx + blockDim.x];
    sdata[tid] = val;

    __syncthreads();

    // Reduction in shared memory 
    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }

    // Write result  to global memory
    if (tid == 0) {
        g_odata[blockIdx.x] = sdata[0];
    }
}


__host__ void reduce(float **input, float **output, unsigned int N,
                     unsigned int threads_per_block) {
    unsigned int n = N;
    float **in = input;
    float **out = output;

    
    float *tmp;

    
    unsigned int blocks = (n + threads_per_block * 2 - 1) / (threads_per_block * 2);

    while (n > 1) {
        size_t shmem_size = threads_per_block * sizeof(float);

        reduce_kernel<<<blocks, threads_per_block, shmem_size>>>(*in, *out, n);
        cudaDeviceSynchronize(); 

       
        n = blocks;
        tmp = *in;
        *in = *out;
        *out = tmp;

        blocks = (n + threads_per_block * 2 - 1) / (threads_per_block * 2);
    }
}
