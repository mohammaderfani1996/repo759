#include <cuda_runtime.h>
#include "defs.hpp"
#include <cmath>
#include <thrust/scan.h>
#include "GPUSortBasedCollisionDetector.cuh"
#include <stdio.h>
#include <set>
#include "utils.cuh"
#include "CollisionDetectionDevice.cuh"

#define THREADS_PER_BLOCK (32)

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true) {
    if (code != cudaSuccess) {
        fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
        if (abort) exit(code);
    }
}
#include <omp.h>
#define CUDA_CHECK(call)                                                    \
    do {                                                                    \
        cudaError_t err = call;                                             \
        if (err != cudaSuccess) {                                           \
            std::cerr << "CUDA error in " << __FILE__ << ":" << __LINE__    \
                      << " - " << cudaGetErrorString(err) << std::endl;     \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
    } while (0)

#define CHECK_ERROR() { \
  cudaDeviceSynchronize(); \
  cudaError_t error = cudaGetLastError(); \
  if(error != cudaSuccess) \
  { \
    gpuErrchk(error); \
    exit(-1); \
  } \
}

double approxErfinv(double x) {
    double a = 0.147; // magic constant
    double ln = std::log(1.0 - x * x);
    double tt1 = 2.0 / (M_PI * a) + ln / 2.0;
    double tt2 = ln / a;

    return (x >= 0 ? 1 : -1) * std::sqrt(std::sqrt(tt1 * tt1 - tt2) - tt1);
}

double inverseNormalCDF(double p, double mu, double sigma) {
    // Invert normal CDF using inverse erf approximation
    return mu + sigma * std::sqrt(2.0) * approxErfinv(2.0 * p - 1.0);
}

struct CompareByKey {
    __host__ __device__
    bool operator()(const SphericalSatellite &a, const SphericalSatellite &b) const {
        return a.pos.x < b.pos.x;
    }
};
 
__global__ void sortBasedCollisionDetectorKernel(SphericalSatellite * d_sats, 
        int nSats, /*SphericalSatellite * possibleColliders, int nPossibleColliders,*/
        int * d_hadCollision, double tolerance, LikelyCollisionByIdx * d_collisions, 
        double maxDistance, double t) {
    
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    
    d_hadCollision[idx] = 0;
    if (idx < nSats - 1)
    {
        CartesianCoordinates pos1 = d_sats[idx].pos;
        for (int j = idx + 1; j < nSats; j ++)
        {
            CartesianCoordinates pos2 = d_sats[j].pos;
            // Check: next satellite must be able to be a collision based on X coordinate alone
            if (pos1.x + satRadius + maxDistance < pos2.x - satRadius)
                break;

            double distance = sqrt(pow(pos1.x - pos2.x, 2) 
                    + pow(pos1.y - pos2.y, 2) + pow(pos1.z - pos2.z, 2));
            
            double collisionProbability = deviceNormalCDF(2 * satRadius - distance,
                    0, DISTANCE_STD_DEV);

            // Check if the two satellites are likely to collide
            if (collisionProbability > tolerance) {
                printf("Collision detected between satellite %d and satellite %d with probability %f at distance %f at time %f\n", 
                        idx, j, collisionProbability, distance, t);
                // Store the collision information
                LikelyCollisionByIdx collision = {idx, j, collisionProbability, t};
                d_collisions[idx] = collision;
                d_hadCollision[idx] = 1;
            }
        }
    }
}

GPUSortBasedCollisionDetector::GPUSortBasedCollisionDetector(size_t N) 
	: deviceMemory(N)
{
}

__global__ void condenseCollisionArrayA(int * d_hadCollision, int * d_hadCollisionScan, 
        LikelyCollisionByIdx * d_collisions, LikelyCollisionByIdx * d_collisionsDense, int nSats) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;

    if ((idx < nSats) && (d_hadCollision[idx]))
    {
        d_collisionsDense[d_hadCollisionScan[idx]] = d_collisions[idx];
    }
}

void GPUSortBasedCollisionDetector::getLikelyCollisions(SphericalSatellite sats[], int nSats, 
    SphericalSatellite possibleColliders[], int nPossibleColliders, double t, int num_threads, double tolerance, 
    std::vector<LikelyCollision> &collisions) {
    collisions.clear(); // Clear previous collisions
    
    // 0. Determine what distance of x is required for collision 
    // 1. Copy array in 
    // 2. Sort it (e.g., on x coordinate or x coordinate - radius)
    //  - Either use thrust implementation
    //  - Or implement insertion sort or similar?
    // 3. Check all sats
    //  - For each sat, sats[i]:
    //      - Check sats[i+1]...sats[i+k], s.t. sats[i+k].x - radius <= sats[i+1] + radius
    // 4. In CPU: Loop over array and add to 


    int nSats2 = nSats * nSats;

    // Define device objects
    SphericalSatellite * d_sats = deviceMemory.getDeviceSats();
    //SphericalSatellite * d_possibleColliders;
    LikelyCollisionByIdx * d_collisionIndices = deviceMemory.getDeviceCollisionIndices();
    LikelyCollisionByIdx * d_collisionIndicesDense = deviceMemory.getDeviceCollisionIndicesDense();
    int * d_hadCollision = deviceMemory.getDeviceHadCollision();
    int * d_hadCollisionScan = deviceMemory.getDeviceHadCollisionScan();
    // Host objects
    int h_numCollisions;
    std::vector<LikelyCollisionByIdx> h_collisionsIndices;

    CHECK_ERROR();
    // MALLOC
    /*cudaMalloc(&d_sats, sizeof(SphericalSatellite) * nSats);
    //cudaMalloc(&d_possibleColliders, sizeof(SphericalSatellite) * nPossibleColliders);
    cudaMalloc(&d_collisionIndices, sizeof(LikelyCollisionByIdx) * nSats2);
    // Can't have more than n^2 - 1 actual collisions
    cudaMalloc(&d_collisionIndicesDense, sizeof(LikelyCollisionByIdx) * nSats2 / 2); 
    cudaMalloc(&d_hadCollision, sizeof(int) * nSats2);
    cudaMalloc(&d_hadCollisionScan, sizeof(int) * nSats2);
    */
    // Sort host array directly
    thrust::sort(sats, sats + nSats, CompareByKey());

    CHECK_ERROR();
    // MEMCPY from host
    cudaMemcpy(d_sats, sats, sizeof(SphericalSatellite) * nSats, cudaMemcpyHostToDevice);
    CHECK_ERROR();

    // Divide tolerance by half to have some room for error
    double maxDistance = inverseNormalCDF(tolerance/2, 0, DISTANCE_STD_DEV);
    CHECK_ERROR();

    // Call detection kernel to set:
    //   - d_hadCollision (1/0)
    //   - d_collisions (sparse array)
    sortBasedCollisionDetectorKernel<<<(nSats + THREADS_PER_BLOCK-1)/THREADS_PER_BLOCK, THREADS_PER_BLOCK>>>(d_sats, nSats, d_hadCollision, 
            tolerance, d_collisionIndices, maxDistance, t);

    CHECK_ERROR();

    // Exclusive scan of d_hadCollision (note, this will give us # collisions as last index)
    thrust::exclusive_scan(thrust::device, d_hadCollision, d_hadCollision + nSats2, d_hadCollisionScan);

    CHECK_ERROR();
    // Condense the array: 
    //   - if d_hadCollision[i], store d_collisions[i] to dense array at index d_hadCollisionScan[i]
    condenseCollisionArrayA<<<(nSats + THREADS_PER_BLOCK-1)/THREADS_PER_BLOCK, THREADS_PER_BLOCK>>>(d_hadCollision, 
            d_hadCollisionScan, d_collisionIndices, d_collisionIndicesDense, nSats);

    CHECK_ERROR();
    // Copy stuff back
    cudaMemcpy(&h_numCollisions, d_hadCollisionScan + nSats2 - 1, sizeof(int), cudaMemcpyDeviceToHost);
    h_collisionsIndices.resize(h_numCollisions);
    cudaMemcpy(h_collisionsIndices.data(), d_collisionIndicesDense, 
            sizeof(LikelyCollisionByIdx) * h_numCollisions, cudaMemcpyDeviceToHost);
    CHECK_ERROR();

    // Convert to LikelyCollision (from i/j/probability)
    for (int i = 0; i < h_numCollisions; i ++)
    {
        LikelyCollisionByIdx x = h_collisionsIndices[i];
        SphericalSatellite &sat1 = sats[x.idx1];
        SphericalSatellite &sat2= sats[x.idx2];
        LikelyCollision collision = {sat1, sat2, x.probability, t};
        collisions.push_back(collision);
    }
    /*
    cudaFree(d_sats);
    //cudaFree(d_possibleColliders);
    cudaFree(d_collisionIndices);
    cudaFree(d_collisionIndicesDense);
    cudaFree(d_hadCollision);
    cudaFree(d_hadCollisionScan);*/
}
