#include <cuda_runtime.h>
#include <thrust/sort.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <vector>
#include <cmath>
#include <stdio.h>
#include "defs.hpp"
#include "GPUGridBasedCollisionDetector.cuh"
#include "utils.cuh"
#include <thrust/scan.h>


const double GRID_CELL_SIZE = 2 * satRadius + DISTANCE_STD_DEV;

__device__ int getGridHash(int3 pos, int3 dims) {
    return pos.z * dims.y * dims.x + pos.y * dims.x + pos.x;
}

__device__ int3 getGridPos(CartesianCoordinates pos, CartesianCoordinates origin, double cellSize) {
    return make_int3(
        (int)((pos.x - origin.x) / cellSize),
        (int)((pos.y - origin.y) / cellSize),
        (int)((pos.z - origin.z) / cellSize)
    );
}

struct GridEntry {
    int satIdx;
    int gridHash;
};


__global__ void assignSatellitesToGrid(const SphericalSatellite *d_sats, GridEntry *d_entries,
                                       CartesianCoordinates origin, double cellSize, int3 gridDims, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= n) return;

    CartesianCoordinates pos = d_sats[idx].pos;
    int3 gridPos = getGridPos(pos, origin, cellSize);
    int hash = getGridHash(gridPos, gridDims);

    d_entries[idx].satIdx = idx;
    d_entries[idx].gridHash = hash;
}

__global__ void fillCellBounds(int *d_cellStarts, int *d_cellEnds, const GridEntry *d_entries, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= n) return;

    int currHash = d_entries[idx].gridHash;
    int prevHash = (idx > 0) ? d_entries[idx - 1].gridHash : -1;
    int nextHash = (idx < n - 1) ? d_entries[idx + 1].gridHash : -1;

    if (currHash != prevHash) d_cellStarts[currHash] = idx;
    if (currHash != nextHash) d_cellEnds[currHash] = idx + 1;
}

__global__ void detectCollisions(const SphericalSatellite *d_sats, const GridEntry *d_entries, 
                                 const int *d_cellStarts, const int *d_cellEnds, 
                                 int3 gridDims, CartesianCoordinates origin, double cellSize, 
                                 double tolerance, LikelyCollisionByIdx *d_collisions, int *d_numCollisions, int maxCollisions, double t) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= gridDims.x * gridDims.y * gridDims.z) return;

    int start = d_cellStarts[tid];
    int end = d_cellEnds[tid];
    if (start == -1 || end == -1) return;

    for (int i = start; i < end; ++i) {
        int satIdx1 = d_entries[i].satIdx;
        CartesianCoordinates pos1 = d_sats[satIdx1].pos;

        for (int dz = -1; dz <= 1; ++dz) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int3 neighborGridPos = make_int3(tid % gridDims.x + dx,
                                                    (tid / gridDims.x) % gridDims.y + dy,
                                                    tid / (gridDims.x * gridDims.y) + dz);

                    if (neighborGridPos.x < 0 || neighborGridPos.x >= gridDims.x ||
                        neighborGridPos.y < 0 || neighborGridPos.y >= gridDims.y ||
                        neighborGridPos.z < 0 || neighborGridPos.z >= gridDims.z) continue;

                    int neighborHash = getGridHash(neighborGridPos, gridDims);
                    int nStart = d_cellStarts[neighborHash];
                    int nEnd = d_cellEnds[neighborHash];
                    if (nStart == -1 || nEnd == -1) continue;

                    for (int j = nStart; j < nEnd; ++j) {
                        int satIdx2 = d_entries[j].satIdx;
                        if (satIdx2 <= satIdx1) continue;

                        CartesianCoordinates pos2 = d_sats[satIdx2].pos;
                        double dx = pos1.x - pos2.x;
                        double dy = pos1.y - pos2.y;
                        double dz = pos1.z - pos2.z;
                        double distance = sqrt(dx * dx + dy * dy + dz * dz);

                        double collisionProb = deviceNormalCDF(2 * satRadius - distance, 0, DISTANCE_STD_DEV);
                        if (collisionProb > tolerance) {
                            int idx = atomicAdd(d_numCollisions, 1);
                            if (idx < maxCollisions) {
                                d_collisions[idx] = {satIdx1, satIdx2, collisionProb,t};
                            }
                        }
                    }
                }
            }
        }
    }
}

//GPUGridBasedCollisionDetector


void GPUGridBasedCollisionDetector::getLikelyCollisions(SphericalSatellite sats[], int nSats,
    SphericalSatellite[], int, double t, int threads_per_block, double tolerance, std::vector<LikelyCollision> &collisions) {

    //Calculate bounding box
    CartesianCoordinates min = sats[0].pos;
    CartesianCoordinates max = sats[0].pos;
    for (int i = 1; i < nSats; ++i) {
        min.x = std::min(min.x, sats[i].pos.x);
        min.y = std::min(min.y, sats[i].pos.y);
        min.z = std::min(min.z, sats[i].pos.z);
        max.x = std::max(max.x, sats[i].pos.x);
        max.y = std::max(max.y, sats[i].pos.y);
        max.z = std::max(max.z, sats[i].pos.z);
    }

    CartesianCoordinates origin = min;
    int3 gridDims = make_int3(
        ceil((max.x - min.x) / GRID_CELL_SIZE) + 1,
        ceil((max.y - min.y) / GRID_CELL_SIZE) + 1,
        ceil((max.z - min.z) / GRID_CELL_SIZE) + 1
    );

    //Copy satellites to GPU
    SphericalSatellite *d_sats;
    cudaMalloc(&d_sats, sizeof(SphericalSatellite) * nSats);
    cudaMemcpy(d_sats, sats, sizeof(SphericalSatellite) * nSats, cudaMemcpyHostToDevice);

    //Assign each satellites to grid
    GridEntry *d_entries;
    cudaMalloc(&d_entries, sizeof(GridEntry) * nSats);
    assignSatellitesToGrid<<<(nSats + threads_per_block -1)/threads_per_block, threads_per_block>>>(d_sats, d_entries, origin, GRID_CELL_SIZE, gridDims, nSats);
    cudaDeviceSynchronize();
    //Sort grid entries
    thrust::device_ptr<int> keys((int*)&d_entries[0].gridHash);
    thrust::device_ptr<int> values((int*)&d_entries[0].satIdx);
    thrust::sort_by_key(keys, keys + nSats, values);

    // Compute cell starts/ends based on hash calcualtion
    int gridSize = gridDims.x * gridDims.y * gridDims.z;
    int *d_cellStarts, *d_cellEnds;
    cudaMalloc(&d_cellStarts, sizeof(int) * gridSize);
    cudaMalloc(&d_cellEnds, sizeof(int) * gridSize);
    cudaMemset(d_cellStarts, -1, sizeof(int) * gridSize);
    cudaMemset(d_cellEnds, -1, sizeof(int) * gridSize);

    fillCellBounds<<<(nSats + threads_per_block -1)/threads_per_block, threads_per_block>>>(d_cellStarts, d_cellEnds, d_entries, nSats);
    cudaDeviceSynchronize();
    //Allocate memory for collisions
    int maxCollisions = nSats * 10;
    LikelyCollisionByIdx *d_collisions;
    int *d_numCollisions;
    cudaMalloc(&d_collisions, sizeof(LikelyCollisionByIdx) * maxCollisions);
    cudaMalloc(&d_numCollisions, sizeof(int));
    cudaMemset(d_numCollisions, 0, sizeof(int));

    // detection kernel
    detectCollisions<<<(gridSize + threads_per_block -1)/threads_per_block, threads_per_block>>>(d_sats, d_entries, d_cellStarts, d_cellEnds,
        gridDims, origin, GRID_CELL_SIZE, tolerance, d_collisions, d_numCollisions, maxCollisions, t);
    cudaDeviceSynchronize();

    // back collisions to the host
    int h_numCollisions;
    cudaMemcpy(&h_numCollisions, d_numCollisions, sizeof(int), cudaMemcpyDeviceToHost);
    printf("h_numCollisions %d", h_numCollisions);
    std::vector<LikelyCollisionByIdx> temp(h_numCollisions);
    cudaMemcpy(temp.data(), d_collisions, sizeof(LikelyCollisionByIdx) * h_numCollisions, cudaMemcpyDeviceToHost);

    for (const auto &c : temp) {
        collisions.push_back({sats[c.idx1], sats[c.idx2], c.probability, t});
    }

    cudaFree(d_sats);
    cudaFree(d_entries);
    cudaFree(d_cellStarts);
    cudaFree(d_cellEnds);
    cudaFree(d_collisions);
    cudaFree(d_numCollisions);
}
