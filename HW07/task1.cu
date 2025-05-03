#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cuda.h>
#include "matmul.cuh"


 
float randFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

int randInt(int min, int max) {
    return min + static_cast<int>(rand()) / (static_cast<int>(RAND_MAX / (max - min)));
}

float randdouble(double min, double max) {
    return min + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (max - min)));
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " n\n";
        return 1;
    }

    unsigned int n = atoi(argv[1]);
    unsigned int block_dim = atoi(argv[2]); 
    srand(static_cast<unsigned int>(time(0)));

    // Allocate memory for host
    int *hAint=new int[n*n];
    int *hBint=new int[n*n];
    int *hCint=new int[n*n];

    float *hA = new float[n*n];
    float *hB = new float[n*n];
    float *hC = new float[n*n];

    double *hAdoub=new double[n*n];
    double *hBdoub=new double[n*n];
    double *hCdoub=new double[n*n];


    for (unsigned int i = 0; i < n*n; i++) {
        hAint[i] = randInt(-10, 10);
        hBint[i] = randInt(-10, 10);
    }

    for (unsigned int i = 0; i < n*n; i++) {
        hA[i] = randFloat(-10.0f, 10.0f);
        hB[i] = randFloat(-10.0f, 10.0f);
    }

    for (unsigned int i = 0; i < n*n; i++) {
        hAdoub[i] = randdouble(-10.0, 10.0);
        hBdoub[i] = randdouble(-10.0, 10.0);
    }

    // Allocate memory for device array
    float *dA,*dB,*dC;

    int *dAint,*dBint,*dCint;

    double *dAdoub,*dBdoub,*dCdoub;

    unsigned int size_float = n * n * sizeof(float);
    unsigned int size_int = n * n * sizeof(int);
    unsigned int size_doub = n * n * sizeof(double);

    cudaMalloc((void**)&dA, size_float);
    cudaMalloc((void**)&dB, size_float);
    cudaMalloc((void**)&dC, size_float);

    cudaMalloc((void**)&dAint, size_int);
    cudaMalloc((void**)&dBint, size_int);
    cudaMalloc((void**)&dCint, size_int);

    cudaMalloc((void**)&dAdoub, size_doub);
    cudaMalloc((void**)&dBdoub, size_doub);
    cudaMalloc((void**)&dCdoub, size_doub);

    cudaMemcpy(dA, hA, size_float, cudaMemcpyHostToDevice);
    cudaMemcpy(dB, hB, size_float, cudaMemcpyHostToDevice);

    cudaMemcpy(dAint, hAint, size_int, cudaMemcpyHostToDevice);
    cudaMemcpy(dBint, hBint, size_int, cudaMemcpyHostToDevice);

    cudaMemcpy(dAdoub, hAdoub, size_doub, cudaMemcpyHostToDevice);
    cudaMemcpy(dBdoub, hBdoub, size_doub, cudaMemcpyHostToDevice);
    
    

    cudaEvent_t startf, stopf;
    cudaEvent_t startd, stopd;
    cudaEvent_t starti, stopi;
    cudaEventCreate(&startf);
    cudaEventCreate(&stopf);
    cudaEventCreate(&startd);
    cudaEventCreate(&stopd);
    cudaEventCreate(&starti);
    cudaEventCreate(&stopi);


    cudaEventRecord(starti);
    matmul_1(dAint,dBint,dCint,n,block_dim);
    cudaEventRecord(stopi);
    cudaEventSynchronize(stopi);

    cudaEventRecord(startf);
    matmul_2(dA,dB,dC,n,block_dim);
    cudaEventRecord(stopf);
    cudaEventSynchronize(stopf);

    cudaEventRecord(startd);
    matmul_3(dAdoub,dBdoub,dCdoub,n,block_dim);
    cudaEventRecord(stopd);
    cudaEventSynchronize(stopd);
    
    float milliseconds_intmult = 0.0f;
    cudaEventElapsedTime(&milliseconds_intmult, starti, stopi);
   
    float milliseconds_floatmult = 0.0f;
    cudaEventElapsedTime(&milliseconds_floatmult, startf, stopf);

    float milliseconds_doubmult = 0.0f;
    cudaEventElapsedTime(&milliseconds_doubmult, startd, stopd);

    // Copy result back to host
    cudaMemcpy(hC, dC, size_float, cudaMemcpyDeviceToHost);
    cudaMemcpy(hCint, dCint, size_int, cudaMemcpyDeviceToHost);
    cudaMemcpy(hCdoub, dCdoub, size_doub, cudaMemcpyDeviceToHost);

    std::cout << hCint[0] << std::endl;
    std::cout << hCint[n*n - 1] << std::endl;
    std::cout << milliseconds_intmult << std::endl;

    std::cout << hC[0] << std::endl;
    std::cout << hC[n*n - 1] << std::endl;
    std::cout << milliseconds_floatmult << std::endl;
    
    std::cout << hCdoub[0] << std::endl;
    std::cout << hCdoub[n*n - 1] << std::endl;
    std::cout << milliseconds_doubmult << std::endl;
    

   
    delete[] hA;
    delete[] hB;
    delete[] hC;
    delete[] hAint;
    delete[] hBint;
    delete[] hCint;
    delete[] hAdoub;
    delete[] hBdoub;
    delete[] hCdoub;

    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaFree(dAint);
    cudaFree(dBint);
    cudaFree(dCint);
    cudaFree(dAdoub);
    cudaFree(dBdoub);
    cudaFree(dCdoub);

    cudaEventDestroy(startf);
    cudaEventDestroy(stopf);
    cudaEventDestroy(starti);
    cudaEventDestroy(stopi);
    cudaEventDestroy(startd);
    cudaEventDestroy(stopd);

    return 0;
}