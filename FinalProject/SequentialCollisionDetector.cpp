#include "defs.hpp"
#include <cmath>
#include "SequentialCollisionDetector.hpp"
#include <stdio.h>
#include <set>
#include "utils.hpp"

void SequentialCollisionDetector::getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, double tolerance, std::vector<LikelyCollision> &collisions) {
    collisions.clear(); // Clear previous collisions

    std::set<int> possibleCollidersSet;
    for (int i = 0; i < nPossibleColliders; i++) {
        possibleCollidersSet.insert(possibleColliders[i].id);
    }

    // Loop through each pair of satellites
    for (int i = 0; i < nSats; i++) {
        for (int j = i+1; j < nSats; j++) {
            if(possibleCollidersSet.count(sats[i].id) == 0) {
                continue; // Skip if the first satellite is not in the possibleColliders set
            }
            CartesianCoordinates pos1 = sats[i].pos;
            CartesianCoordinates pos2 = sats[j].pos;
            double distance = sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2) + pow(pos1.z - pos2.z, 2));
            double collisionProbability = normalCDF(2 * satRadius - distance, 0, DISTANCE_STD_DEV);

            // Check if the two satellites are likely to collide
            if (collisionProbability > tolerance) {
                // Store the collision information
                SphericalSatellite &sat1 = sats[i];
                SphericalSatellite &sat2= sats[j];
                double probability = collisionProbability;
                LikelyCollision collision = {sat1, sat2, probability, t};
                collisions.push_back(collision);
            }
        }
    }
}