#include <iostream>
//#include <cuda_runtime.h>
#include "defs.hpp"
#include "CollisionDetectionDevice.cuh"

// Constructor: Allocates memory for all device objects
CollisionDetectionDevice::CollisionDetectionDevice(size_t N) {
	// Allocate memory on the device
	cudaError_t err;
	size_t nSats2 = N * N;

	err = cudaMalloc(&d_sats, sizeof(SphericalSatellite) * N);
	if (err != cudaSuccess) {
	    std::cerr << "CUDA malloc for d_sats failed: " << cudaGetErrorString(err) << std::endl;
	    exit(EXIT_FAILURE);
	}

	err = cudaMalloc(&d_collisionIndices, sizeof(LikelyCollisionByIdx) * nSats2);
	if (err != cudaSuccess) {
	    std::cerr << "CUDA malloc for d_collisionIndices failed: " << cudaGetErrorString(err) << std::endl;
	    exit(EXIT_FAILURE);
	}

	err = cudaMalloc(&d_collisionIndicesDense, sizeof(LikelyCollisionByIdx) * nSats2 / 2);
	if (err != cudaSuccess) {
	    std::cerr << "CUDA malloc for d_collisionIndicesDense failed: " << cudaGetErrorString(err) << std::endl;
	    exit(EXIT_FAILURE);
	}

	err = cudaMalloc(&d_hadCollision, sizeof(int) * nSats2);
	if (err != cudaSuccess) {
	    std::cerr << "CUDA malloc for d_hadCollision failed: " << cudaGetErrorString(err) << std::endl;
	    exit(EXIT_FAILURE);
	}

	err = cudaMalloc(&d_hadCollisionScan, sizeof(int) * nSats2);
	if (err != cudaSuccess) {
	    std::cerr << "CUDA malloc for d_hadCollisionScan failed: " << cudaGetErrorString(err) << std::endl;
	    exit(EXIT_FAILURE);
	}

	std::cerr << "Memory successfully allocated on device." << std::endl;
}

// Destructor: Frees the allocated device memory
CollisionDetectionDevice::~CollisionDetectionDevice() {
	cudaFree(d_sats);
	cudaFree(d_collisionIndices);
	cudaFree(d_collisionIndicesDense);
	cudaFree(d_hadCollision);
	cudaFree(d_hadCollisionScan);
	std::cerr << "Device memory successfully freed." << std::endl;
}

// Getter functions to access the device pointers
SphericalSatellite* CollisionDetectionDevice::getDeviceSats() const { return d_sats; }
LikelyCollisionByIdx* CollisionDetectionDevice::getDeviceCollisionIndices() const { return d_collisionIndices; }
LikelyCollisionByIdx* CollisionDetectionDevice::getDeviceCollisionIndicesDense() const { return d_collisionIndicesDense; }
int* CollisionDetectionDevice::getDeviceHadCollision() const { return d_hadCollision; }
int* CollisionDetectionDevice::getDeviceHadCollisionScan() const { return d_hadCollisionScan; }

// Function to check if device memory is valid (e.g., for error handling)
bool CollisionDetectionDevice::isValid() const {
return (d_sats != nullptr && d_collisionIndices != nullptr && d_collisionIndicesDense != nullptr &&
	d_hadCollision != nullptr && d_hadCollisionScan != nullptr);
}

