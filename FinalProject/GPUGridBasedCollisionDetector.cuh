#ifndef GPU_GRID_BASED_COLLISION_DETECTOR_HPP
#define GPU_GRID_BASED_COLLISION_DETECTOR_HPP

#include "defs.hpp"
#include <vector>

class GPUGridBasedCollisionDetector : public CollisionDetector {
public:
    GPUGridBasedCollisionDetector() {}
    ~GPUGridBasedCollisionDetector() {}
    void getLikelyCollisions(SphericalSatellite sats[], int nSats,
                             SphericalSatellite possibleColliders[], int nPossibleColliders,
                             double t, double tolerance,
                             std::vector<LikelyCollision> &collisions) override;
};

#endif
