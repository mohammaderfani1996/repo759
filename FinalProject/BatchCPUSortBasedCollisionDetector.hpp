#ifndef BATCH_CPU_SORT_BASED_COLLISION_DETECTOR_HPP
#define BATCH_CPU_SORT_BASED_COLLISION_DETECTOR_HPP

#include "defs.hpp"
#include <vector>
#include <float.h>

class BatchCPUSortBasedCollisionDetector : public CollisionDetector {
    private:
        SphericalSatellite * currentSortedSats = NULL;
        double currentSortedSatsStartTime = DBL_MAX;
    public:
        BatchCPUSortBasedCollisionDetector() {}
        ~BatchCPUSortBasedCollisionDetector() {}
        void getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, double tolerance, std::vector<LikelyCollision> &collisions) override;
};

#endif