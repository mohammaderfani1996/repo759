#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cuda.h>
#include "reduce.cuh"

float randFloatMinus1to1() {
    return -1.0f + 2.0f * static_cast<float>(rand()) / RAND_MAX;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " N threads_per_block\n";
        return 1;
    }

    unsigned int N = atoi(argv[1]);
    unsigned int threads_per_block = atoi(argv[2]);

    srand(static_cast<unsigned int>(time(0)));

    float *h_input = new float[N];
    for (unsigned int i = 0; i < N; ++i)
        h_input[i] = randFloatMinus1to1();


    float *d_input, *d_output;
    cudaMalloc(&d_input, N * sizeof(float));

    unsigned int blocks = (N + threads_per_block * 2 - 1) / (threads_per_block * 2);
    cudaMalloc(&d_output, blocks * sizeof(float));

    // Copy host to device
    cudaMemcpy(d_input, h_input, N * sizeof(float), cudaMemcpyHostToDevice);

    float *d_in_ptr = d_input;
    float *d_out_ptr = d_output;

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    reduce(&d_in_ptr, &d_out_ptr, N, threads_per_block);

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float milliseconds = 0.0f;
    cudaEventElapsedTime(&milliseconds, start, stop);

    float result;
    cudaMemcpy(&result, d_in_ptr, sizeof(float), cudaMemcpyDeviceToHost);

    std::cout << result << std::endl;
    std::cout << milliseconds << std::endl;

    // Cleanup
    delete[] h_input;
    cudaFree(d_input);
    cudaFree(d_output);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    return 0;
}
