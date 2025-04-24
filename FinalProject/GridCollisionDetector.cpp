#include "GridCollisionDetector.hpp"
#include <omp.h>  // OpenMP 
#include <unordered_map>
#include "utils.hpp"


struct GridKey {
    int x, y, z;
    
    bool operator==(const GridKey& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

// Hash function for unordered_map
namespace std {
    template <>
    struct hash<GridKey> {
        std::size_t operator()(const GridKey& k) const {
            return ((51 + std::hash<int>()(k.x)) * 51 + std::hash<int>()(k.y)) * 51 + std::hash<int>()(k.z);
        }
    };
}

void GridCollisionDetector::getLikelyCollisions(
    SphericalSatellite sats[], int nSats,
    SphericalSatellite possibleColliders[], int nPossibleColliders,
    double t, double tolerance,
    std::vector<LikelyCollision>& collisions
) {
    collisions.clear();

    const double cellSize = 1e5;  // 100 km cubes
    std::unordered_map<GridKey, std::vector<SphericalSatellite>> grid;

    // Step 1: Insert satellites into grid
    for (int i = 0; i < nSats; ++i) {
        const CartesianCoordinates& p = sats[i].pos;
        GridKey key = {
            static_cast<int>(std::floor(p.x / cellSize)),
            static_cast<int>(std::floor(p.y / cellSize)),
            static_cast<int>(std::floor(p.z / cellSize))
        };
        grid[key].push_back(sats[i]);
    }

    // Step 2: Check for collisions
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < nSats; ++i) {
        const CartesianCoordinates& pos = sats[i].pos;
        GridKey key = {
            static_cast<int>(std::floor(pos.x / cellSize)),
            static_cast<int>(std::floor(pos.y / cellSize)),
            static_cast<int>(std::floor(pos.z / cellSize))
        };

        // Check 3×3×3 neighbors
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    GridKey neighborKey = {key.x + dx, key.y + dy, key.z + dz};

                    auto it = grid.find(neighborKey);
                    if (it != grid.end()) {
                        for (auto& other : it->second) {
                            if (sats[i].id >= other.id) continue;  // avoid double-counting
                        
                            double dist = std::sqrt(
                                std::pow(pos.x - other.pos.x, 2) +
                                std::pow(pos.y - other.pos.y, 2) +
                                std::pow(pos.z - other.pos.z, 2)
                            );
                        
                            double prob = normalCDF(2 * satRadius - dist, 0, DISTANCE_STD_DEV);
                        
                            if (prob > tolerance) {
                                #pragma omp critical
                                collisions.push_back(LikelyCollision{sats[i], other, prob, t});
                            }
                        }
                        
                    }
                }
            }
        }
    }
}

