#ifndef GRID_COLLISION_DETECTOR_HPP
#define GRID_COLLISION_DETECTOR_HPP

#include "defs.hpp"
#include <vector>

class GridCollisionDetector : public CollisionDetector {
public:
    // Main required interface
    virtual void getLikelyCollisions(
        SphericalSatellite sats[], int nSats,
        SphericalSatellite possibleColliders[], int nPossibleColliders,
        double t, double tolerance,
        std::vector<LikelyCollision> &collisions
    );
};

#endif // GRID_COLLISION_DETECTOR_HPP
