// Author: Ruochun and Nic Olsen

#include "stencil.cuh"
#include <cuda_runtime.h>
#include <iostream>

// Computes the convolution of image and mask, storing the result in output.
// Each thread should compute _one_ element of the output matrix.
// Shared memory should be allocated _dynamically_ only.
//
// image is an array of length n.
// mask is an array of length (2 * R + 1).
// output is an array of length n.
// All of them are in device memory
//
// Assumptions:
// - 1D configuration
// - blockDim.x >= 2 * R + 1
//
// The following should be stored/computed in shared memory:
// - The entire mask
// - The elements of image that are needed to compute the elements of output corresponding to the threads in the given block
// - The output image elements corresponding to the given block before it is written back to global memory
__global__ void stencil_kernel(const float* image, const float* mask, float* output, unsigned int n, unsigned int R){
    extern __shared__ float shared[];  // dynamic shared memory

    float* shared_mask = shared;                  
    float* shared_image = shared + (2 * R + 1);  

    unsigned int global_idx = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int local_idx  = threadIdx.x;

    // Load mask into shared memory
    if (local_idx < (2 * R + 1)) {
        shared_mask[local_idx] = mask[local_idx];
    }
     // Load image into shared memory
     int iter_idx = global_idx - R;
     if (iter_idx < n) {
         shared_image[local_idx + R] = (iter_idx >= 0) ? image[iter_idx] : 1.0f; // left halo
     } else {
         shared_image[local_idx + R] = 1.0f;
     }
      // Left padding
    if (local_idx < R) {
        int left_idx = global_idx - R + local_idx - R;
        shared_image[local_idx] = (left_idx >= 0 && left_idx < n) ? image[left_idx] : 1.0f;
    }
    if (local_idx >= blockDim.x - R) {
        int right_idx = global_idx - R + local_idx + R;
        if (right_idx < n)
            shared_image[local_idx + 2 * R] = image[right_idx];
        else
            shared_image[local_idx + 2 * R] = 1.0f;
    }
    __syncthreads();
    if (global_idx < n) {
        float result = 0.0f;
        for (int j = -R; j <= (int)R; ++j) {
            result += shared_image[local_idx + R + j] * shared_mask[j + R];
        }
        output[global_idx] = result;
    }
}

// Makes one call to stencil_kernel with threads_per_block threads per block.
// You can consider following the kernel call with cudaDeviceSynchronize (but if you use
// cudaEventSynchronize to time it, that call serves the same purpose as cudaDeviceSynchronize).
//
// Assumptions:
// - threads_per_block >= 2 * R + 1
__host__ void stencil(const float* image,const float* mask,float* output,unsigned int n,unsigned int R,unsigned int threads_per_block){


    const int blocksPerGrid=(n+threads_per_block-1)/threads_per_block;
    size_t shared_bytes = (2 * R + 1 + threads_per_block + 2 * R) * sizeof(float);

    stencil_kernel<<<blocksPerGrid, threads_per_block, shared_bytes>>>(image,mask,output,n,R);
    cudaDeviceSynchronize();
}




