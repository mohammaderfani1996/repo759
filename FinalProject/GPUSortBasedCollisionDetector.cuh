#ifndef GPU_SORT_COLLISION_DETECTOR_HPP
#define GPU_SORT_COLLISION_DETECTOR_HPP

#include "defs.hpp"
#include <vector>
#include "CollisionDetectionDevice.cuh"

class GPUSortBasedCollisionDetector : public CollisionDetector {
    private: 
	CollisionDetectionDevice deviceMemory;
    public:
        GPUSortBasedCollisionDetector() {}
        ~GPUSortBasedCollisionDetector() {}
        GPUSortBasedCollisionDetector(size_t N);
        void getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, int num_threads, double tolerance, std::vector<LikelyCollision> &collisions) override;
};


#endif
