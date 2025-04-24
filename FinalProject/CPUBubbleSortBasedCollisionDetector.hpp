#ifndef CPUBubbleSortBasedCollisionDetector_HPP
#define CPUBubbleSortBasedCollisionDetector_HPP

#include "defs.hpp"
#include <cfloat>
#include <vector>
#include <float.h>

class CPUBubbleSortBasedCollisionDetector : public CollisionDetector {
    private:
        SphericalSatellite * currentSortedSats = NULL;
        double currentSortedSatsTime = DBL_MAX;
    public:
        CPUBubbleSortBasedCollisionDetector() {}
        ~CPUBubbleSortBasedCollisionDetector() {}
        void getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, double tolerance, std::vector<LikelyCollision> &collisions) override;
};

#endif