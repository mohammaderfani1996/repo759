#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cuda.h>
#include "vscale.cuh"

#define THREADS_PER_BLOCK 512

float randFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " n\n";
        return 1;
    }

    unsigned int n = atoi(argv[1]);
    srand(static_cast<unsigned int>(time(0)));

    // Allocate memory for host
    float *h_a = new float[n];
    float *h_b = new float[n];

    for (unsigned int i = 0; i < n; i++) {
        h_a[i] = randFloat(-10.0f, 10.0f);
        h_b[i] = randFloat(0.0f, 1.0f);
    }

    // Allocate memory for device array
    float *d_a, *d_b;
    cudaMalloc((void **)&d_a, n * sizeof(float));
    cudaMalloc((void **)&d_b, n * sizeof(float));

    // Copy data to device
    cudaMemcpy(d_a, h_a, n * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b, n * sizeof(float), cudaMemcpyHostToDevice);

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    unsigned int numBlocks = (n + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    vscale<<<numBlocks, THREADS_PER_BLOCK>>>(d_a, d_b, n);

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);

    // Copy result back to host
    cudaMemcpy(h_b, d_b, n * sizeof(float), cudaMemcpyDeviceToHost);

    
    std::cout << milliseconds << std::endl;
    std::cout << h_b[0] << std::endl;
    std::cout << h_b[n - 1] << std::endl;

   
    delete[] h_a;
    delete[] h_b;
    cudaFree(d_a);
    cudaFree(d_b);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    return 0;
}