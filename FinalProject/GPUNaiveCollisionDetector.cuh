#ifndef GPU_NAIVE_COLLISION_DETECTOR_HPP
#define GPU_NAIVE_COLLISION_DETECTOR_HPP

#include "defs.hpp"
#include <vector>
#include "CollisionDetectionDevice.cuh"

class GPUNaiveCollisionDetector : public CollisionDetector {
    private: 
	CollisionDetectionDevice deviceMemory;
    public:
        GPUNaiveCollisionDetector() {}
        ~GPUNaiveCollisionDetector() {}
        GPUNaiveCollisionDetector(size_t N);
        void getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, double tolerance, std::vector<LikelyCollision> &collisions) override;
};


#endif
