#ifndef NAIVE_CPU_PARALLEL_COLLISION_DETECTOR_HPP
#define NAIVE_CPU_PARALLEL_COLLISION_DETECTOR_HPP

#include "defs.hpp"
#include <vector>

class NaiveCPUParallelCollisionDetector : public CollisionDetector {
    public:
        NaiveCPUParallelCollisionDetector() {}
        ~NaiveCPUParallelCollisionDetector() {}
        void getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, double tolerance, std::vector<LikelyCollision> &collisions) override;
};

#endif