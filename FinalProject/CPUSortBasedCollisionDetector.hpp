#ifndef CPU_SORT_BASED_COLLISION_DETECTOR_HPP
#define CPU_SORT_BASED_COLLISION_DETECTOR_HPP

#include "defs.hpp"
#include <vector>

class CPUSortBasedCollisionDetector : public CollisionDetector {
    public:
        CPUSortBasedCollisionDetector() {}
        ~CPUSortBasedCollisionDetector() {}
        void getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, int _, double tolerance, std::vector<LikelyCollision> &collisions) override;
};

#endif
