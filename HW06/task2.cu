#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cuda.h>
#include "stencil.cuh"


 
float randFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}



int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " n\n";
        return 1;
    }

    unsigned int n = atoi(argv[1]);
    unsigned int R =atoi(argv[2]);
    unsigned int THREADS_PER_BLOCK = atoi(argv[3]); 
    srand(static_cast<unsigned int>(time(0)));

    // Allocate memory for host
    float *himage = new float[n];
    float *houtput = new float[n];
    float *hmask = new float[2*R+1];

    for (unsigned int i = 0; i < n; i++) {
        himage[i] = randFloat(-1.0f, 1.0f);
    }
    for (unsigned int i = 0; i < 2*R+1; i++) {
       
        hmask[i] = randFloat(-1.0f, 1.0f);
    }

    // Allocate memory for device array
    float *dimage,*doutput,*dmask;
    unsigned int size_image = n * sizeof(float);
    unsigned int size_mask= (2*R+1)*sizeof(float);
    cudaMalloc((void**)&dimage, size_image);
    cudaMalloc((void**)&doutput, size_image);
    cudaMalloc((void**)&dmask, size_mask);

    cudaMemcpy(dimage, himage, size_image, cudaMemcpyHostToDevice);
    cudaMemcpy(dmask, hmask, size_mask, cudaMemcpyHostToDevice);

    
    

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    stencil(dimage,dmask,doutput,n,R,THREADS_PER_BLOCK);
    

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    
    
   
    float milliseconds = 0.0f;
    cudaEventElapsedTime(&milliseconds, start, stop);

    // Copy result back to host
    cudaMemcpy(houtput, doutput, size_image, cudaMemcpyDeviceToHost);

    std::cout << houtput[n - 1] << std::endl;
    std::cout << milliseconds << std::endl;
    

   
    delete[] himage;
    delete[] houtput;
    delete[] hmask;
    cudaFree(dimage);
    cudaFree(doutput);
    cudaFree(dmask);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    return 0;
}