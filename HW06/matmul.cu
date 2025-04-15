#include "matmul.cuh"
#include <cuda_runtime.h>
#include <iostream>
// Assumptions:
// - 1D kernel configuration
__global__ void matmul_kernel(const float* A, const float* B, float* C, size_t n){
   
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    if( idx>= n*n ) return;

    int row = idx / n;
    int col = idx % n;
    
    float Pvalue = 0.0f;
    for (size_t k = 0; k < n; ++k) {
       
        Pvalue +=  A[row * n + k] * B[k * n + col];
    }
// Write matrix to device memory; each thread one element
    C[row * n + col] = Pvalue;
}

// Makes one call to matmul_kernel with threads_per_block threads per block.
// You can consider following the kernel call with cudaDeviceSynchronize (but if you use 
// cudaEventSynchronize to time it, that call serves the same purpose as cudaDeviceSynchronize).
void matmul(const float* A, const float* B, float* C, size_t n, unsigned int threads_per_block){
  

    const int blocksPerGrid=(n*n+threads_per_block-1)/threads_per_block;
    matmul_kernel<<<blocksPerGrid, threads_per_block>>>(A,B,C,n);
    cudaDeviceSynchronize();


}