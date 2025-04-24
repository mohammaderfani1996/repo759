#ifndef SEQUENTIAL_COLLISION_DETECTOR_HPP
#define SEQUENTIAL_COLLISION_DETECTOR_HPP

#include "defs.hpp"
#include <vector>

class SequentialCollisionDetector : public CollisionDetector {
    public:
        SequentialCollisionDetector() {}
        ~SequentialCollisionDetector() {}
        void getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, double tolerance, std::vector<LikelyCollision> &collisions) override;
};

#endif