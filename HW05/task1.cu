#include <cuda.h>
#include <stdio.h>
__global__ void simpleKernel() { 
    
    int result=1;
    for (int i=1; i<=threadIdx.x; i++){
        result=result*i;
    }
    
    printf("%d!=%d\n",threadIdx,result); 

}
int main() {
const int numThreads = 8;
// invoke GPU kernel, with one block that has four threads
simpleKernel<<<1, numThreads>>>();
cudaDeviceSynchronize();
return 0;
}

