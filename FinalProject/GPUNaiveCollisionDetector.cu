#include <cuda_runtime.h>
#include "defs.hpp"
#include <thrust/scan.h>
#include "GPUNaiveCollisionDetector.cuh"
#include <stdio.h>
#include "utils.cuh"
#include "CollisionDetectionDevice.cuh"

__global__ void naiveCollisionDetectorKernel(SphericalSatellite * d_sats, 
        int nSats, /*SphericalSatellite * possibleColliders, int nPossibleColliders,*/
        int * d_hadCollision, double tolerance, LikelyCollisionByIdx * d_collisions,
	double t) {
    
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    int i = idx / nSats;
    int j = idx % nSats;
    
    d_hadCollision[idx] = 0;
    if ((i < j) && (j < nSats))
    {
        CartesianCoordinates pos1 = d_sats[i].pos;
        CartesianCoordinates pos2 = d_sats[j].pos;
        double distance = sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2) + pow(pos1.z - pos2.z, 2));
        
        double collisionProbability = deviceNormalCDF(2 * satRadius - distance, 0, DISTANCE_STD_DEV);

        // Check if the two satellites are likely to collide
        if (collisionProbability > tolerance) {
            printf("Collision detected between satellite %d and satellite %d with probability %f at distance %f at time %f\n", i, j, collisionProbability, distance, t);
            // Store the collision information
            LikelyCollisionByIdx collision = {i, j, collisionProbability, t};
            d_collisions[idx] = collision;
            d_hadCollision[idx] = 1;
        }
    }
}

__global__ void condenseCollisionArrayB(int * d_hadCollision, int * d_hadCollisionScan, 
        LikelyCollisionByIdx * d_collisions, LikelyCollisionByIdx * d_collisionsDense, int nSats) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;

    if ((idx < nSats) && (d_hadCollision[idx]))
    {
        d_collisionsDense[d_hadCollisionScan[idx]] = d_collisions[idx];
    }
}

GPUNaiveCollisionDetector::GPUNaiveCollisionDetector(size_t N) 
	: deviceMemory(N)
{
}

void GPUNaiveCollisionDetector::getLikelyCollisions(SphericalSatellite sats[], int nSats, 
    SphericalSatellite possibleColliders[], int nPossibleColliders, double t, double tolerance, 
    std::vector<LikelyCollision> &collisions) {
    collisions.clear(); // Clear previous collisions
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

    // MALLOC
    /* cudaMalloc(&d_sats, sizeof(SphericalSatellite) * nSats);
    //cudaMalloc(&d_possibleColliders, sizeof(SphericalSatellite) * nPossibleColliders);
    cudaMalloc(&d_collisionIndices, sizeof(LikelyCollisionByIdx) * nSats2);
    cudaMalloc(&d_collisionIndicesDense, sizeof(LikelyCollisionByIdx) * nSats2 / 2); // Can't have more than this
    cudaMalloc(&d_hadCollision, sizeof(int) * nSats2);
    cudaMalloc(&d_hadCollisionScan, sizeof(int) * nSats2);
    */
    // MEMCPY from host
    cudaMemcpy(d_sats, sats, sizeof(SphericalSatellite) * nSats, cudaMemcpyHostToDevice);

    // Call detection kernel to set:
    //   - d_hadCollision (1/0)
    //   - d_collisions (sparse array)
    naiveCollisionDetectorKernel<<<(nSats + 127)/128, 128>>>(d_sats, nSats, d_hadCollision, 
            tolerance, d_collisionIndices, t);

    // Exclusive scan of d_hadCollision (note, this will give us # collisions as last index)
    thrust::exclusive_scan(d_hadCollision, d_hadCollision + nSats2, d_hadCollisionScan);

    // Condense the array: 
    //   - if d_hadCollision[i], store d_collisions[i] to dense array at index d_hadCollisionScan[i]
    condenseCollisionArrayB<<<(nSats + 127)/128, 128>>>(d_hadCollision, 
            d_hadCollisionScan, d_collisionIndices, d_collisionIndicesDense, nSats);

    // Copy stuff back
    cudaMemcpy(&h_numCollisions, d_hadCollisionScan + nSats2 - 1, sizeof(int), cudaMemcpyDeviceToHost);
    h_collisionsIndices.resize(h_numCollisions);
    cudaMemcpy(h_collisionsIndices.data(), d_collisionIndicesDense, 
            sizeof(LikelyCollisionByIdx) * h_numCollisions, cudaMemcpyDeviceToHost);

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
    cudaFree(d_hadCollisionScan);
    */
}
