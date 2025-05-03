#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cuda.h>
#include "matmul.cuh"


__global__ void Muld_float(const float* A,const float* B, float* C, unsigned int n, unsigned int block_dim)
{
    extern __shared__ float shared[];
    // Block index
    int bx = blockIdx.x; //the B (and C) matrix sub-block column index
    int by = blockIdx.y; //the A (and C) matrix sub-block row index
    // Thread index
    int tx = threadIdx.x; //the column index in the sub-block
    int ty = threadIdx.y; //the row index in the sub-block
    float* As=tile_mem;
    float* Bs=tile_mem + block_dim*block_dim;
    int row = by * block_dim + ty;
    int col = bx * block_dim + tx;

    float Csub = 0.0f;
    

    for (int t = 0; t < (n + block_dim - 1) / block_dim; ++t) {
    
        int aRow = row;
        int aCol = t * block_dim + tx;
        int bRow = t * block_dim + ty;
        int bCol = col;

        As[ty*block_dim+tx] = (aRow < n && aCol < n) ? A[aRow * n + aCol] : 0.0f;
        Bs[ty*block_dim+tx] = (bRow < n && bCol < n) ? B[bRow * n + bCol] : 0.0f;

        __syncthreads();
      
        for (int k = 0; k < block_dim; ++k)
            Csub += As[ty*block_dim+k] * Bs[k*block_dim+tx];
         
        __syncthreads();
    }

    if (row < n && col < n)
        C[row*n+col] = Csub;
}



__global__ void Muld_int(const int* A,const int* B, int* C, unsigned int n, unsigned int block_dim)
{
    extern __shared__ int shared[];
    // Block index
    int bx = blockIdx.x; //the B (and C) matrix sub-block column index
    int by = blockIdx.y; //the A (and C) matrix sub-block row index
    // Thread index
    int tx = threadIdx.x; //the column index in the sub-block
    int ty = threadIdx.y; //the row index in the sub-block
    int* As=tile_mem;
    int* Bs=tile_mem + block_dim*block_dim;
    int row = by * block_dim + ty;
    int col = bx * block_dim + tx;

    int Csub = 0;
    

    for (int t = 0; t < (n + block_dim - 1) / block_dim; ++t) {
    
        int aRow = row;
        int aCol = t * block_dim + tx;
        int bRow = t * block_dim + ty;
        int bCol = col;

        As[ty*block_dim+tx] = (aRow < n && aCol < n) ? A[aRow * n + aCol] : 0;
        Bs[ty*block_dim+tx] = (bRow < n && bCol < n) ? B[bRow * n + bCol] : 0;

        __syncthreads();
      
        for (int k = 0; k < block_dim; ++k)
            Csub += As[ty*block_dim+k] * Bs[k*block_dim+tx];
         
        __syncthreads();
    }

    if (row < n && col < n)
        C[row*n+col] = Csub;
}


__global__ void Muld_doub(const double* A,const double* B, double* C, unsigned int n, unsigned int block_dim)
{
    extern __shared__ double shared[];
    // Block index
    int bx = blockIdx.x; //the B (and C) matrix sub-block column index
    int by = blockIdx.y; //the A (and C) matrix sub-block row index
    // Thread index
    int tx = threadIdx.x; //the column index in the sub-block
    int ty = threadIdx.y; //the row index in the sub-block
    double* As=tile_mem;
    double* Bs=tile_mem + block_dim*block_dim;
    int row = by * block_dim + ty;
    int col = bx * block_dim + tx;

    double Csub = 0.0;
    

    for (int t = 0; t < (n + block_dim - 1) / block_dim; ++t) {
    
        int aRow = row;
        int aCol = t * block_dim + tx;
        int bRow = t * block_dim + ty;
        int bCol = col;

        As[ty*block_dim+tx] = (aRow < n && aCol < n) ? A[aRow * n + aCol] : 0.0;
        Bs[ty*block_dim+tx] = (bRow < n && bCol < n) ? B[bRow * n + bCol] : 0.0;

        __syncthreads();
      
        for (int k = 0; k < block_dim; ++k)
            Csub += As[ty*block_dim+k] * Bs[k*block_dim+tx];
         
        __syncthreads();
    }

    if (row < n && col < n)
        C[row*n+col] = Csub;
}

void matmul_1(const int *A, const int *B, int *C, unsigned int n, unsigned int block_dim){

    dim3 dimBlock(block_dim, block_dim);
    dim3 dimGrid( (n + block_dim - 1)/block_dim , (n + block_dim - 1)/block_dim );
    size_t sharedMemSize = 2 * block_dim * block_dim * sizeof(int);
    // Launch the device computation
    Muld_int<<<dimGrid, dimBlock, sharedMemSize>>>(A,B,C,n,block_dim);
    cudaDeviceSynchronize();


}


void matmul_2(const float *A, const float *B, float *C, unsigned int n, unsigned int block_dim){

    dim3 dimBlock(block_dim, block_dim);
    dim3 dimGrid( (n + block_dim - 1)/block_dim, (n + block_dim - 1)/block_dim );
    size_t sharedMemSize = 2 * block_dim * block_dim * sizeof(float);
    // Launch the device computation
    Muld_float<<<dimGrid, dimBlock, sharedMemSize>>>(A,B,C,n,block_dim);
    cudaDeviceSynchronize();

}



void matmul_3(const double *A, const double *B, double *C, unsigned int n, unsigned int block_dim){
    dim3 dimBlock(block_dim, block_dim);
    dim3 dimGrid( (n + block_dim - 1)/block_dim , (n + block_dim - 1)/block_dim );
    size_t sharedMemSize = 2 * block_dim * block_dim * sizeof(double);
    // Launch the device computation
    Muld_doub<<<dimGrid, dimBlock, sharedMemSize>>>(A,B,C,n,block_dim);
    cudaDeviceSynchronize();

}
