#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cuda.h>
#include "matmul.cuh"


 
float randFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " n\n";
        return 1;
    }

    size_t n = atoi(argv[1]);
    unsigned int THREADS_PER_BLOCK = atoi(argv[2]); 
    srand(static_cast<unsigned int>(time(0)));

    // Allocate memory for host
    float *hA = new float[n*n];
    float *hB = new float[n*n];
    float *hC = new float[n*n];

    for (unsigned int i = 0; i < n*n; i++) {
        hA[i] = randFloat(-1.0f, 1.0f);
        hB[i] = randFloat(-1.0f, 1.0f);
    }

    // Allocate memory for device array
    float *dA,*dB,*dC;
    size_t size = n * n * sizeof(float);
    cudaMalloc((void**)&dA, size);
    cudaMalloc((void**)&dB, size);
    cudaMalloc((void**)&dC, size);

    cudaMemcpy(dA, hA, size, cudaMemcpyHostToDevice);
    cudaMemcpy(dB, hB, size, cudaMemcpyHostToDevice);

    
    

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    matmul(dA,dB,dC,n,THREADS_PER_BLOCK);
    

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    
    
   
    float milliseconds = 0.0f;
    cudaEventElapsedTime(&milliseconds, start, stop);

    // Copy result back to host
    cudaMemcpy(hC, dC, size, cudaMemcpyDeviceToHost);

    std::cout << hC[n*n - 1] << std::endl;
    std::cout << milliseconds << std::endl;
    

   
    delete[] hA;
    delete[] hB;
    delete[] hC;
    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    return 0;
}