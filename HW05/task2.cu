#include<cuda.h>
#include<iostream>
#include <random>
global void simpleKernel(int* data, int a)
{
    int indx=blockIdx.x * blockDim.x + threadIdx.x;
    data[indx] = a*threadIdx.x + blockIdx.x;
}
int main()
{
const int numElems = 8;
const int num_int=16;
int hA[num_int], *dA;

int some_seed = 759;
std::mt19937 generator(some_seed);
auto pseudorandom_value = generator();

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(0, 100);

int a = dis(gen);
 
cudaMalloc((void**)&dA, sizeof(int) * num_int);
cudaMemset(dA, 0, num_int * sizeof(int));
s
simpleKernel<<<2,numElems>>>(dA, a);
cudaDeviceSynchronize();

cudaMemcpy(&hA, dA, sizeof(int) * num_int, cudaMemcpyDeviceToHost);

for (int i = 0; i < num_int; i++)
std::cout << hA[i] <<" ";
cudaFree(dA);
return 0;
}