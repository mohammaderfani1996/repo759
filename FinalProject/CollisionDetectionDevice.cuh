#ifndef COLLISION_DETECTION_DEVICE_H
#define COLLISION_DETECTION_DEVICE_H

#include <cuda_runtime.h>
#include "defs.hpp"  

class CollisionDetectionDevice {
private:
    SphericalSatellite* d_sats;
    LikelyCollisionByIdx* d_collisionIndices;
    LikelyCollisionByIdx* d_collisionIndicesDense;
    int* d_hadCollision;
    int* d_hadCollisionScan;

public:
    // Constructor: Allocates memory for all device objects
    CollisionDetectionDevice() {}
    CollisionDetectionDevice(size_t N);

    // Destructor: Frees the allocated device memory
    ~CollisionDetectionDevice();

    // Getter functions to access the device pointers
    SphericalSatellite* getDeviceSats() const;
    LikelyCollisionByIdx* getDeviceCollisionIndices() const;
    LikelyCollisionByIdx* getDeviceCollisionIndicesDense() const;
    int* getDeviceHadCollision() const;
    int* getDeviceHadCollisionScan() const;

    // Function to check if device memory is valid (e.g., for error handling)
    bool isValid() const;
};

#endif // COLLISION_DETECTION_DEVICE_H

