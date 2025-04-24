#include "defs.hpp"
#include <cstdio>
#include <vector>
#include <set>
#include <cmath>
#include <stdio.h>
#include <omp.h> 
#include "CPUBubbleSortBasedCollisionDetector.hpp"
#include "utils.hpp"


void CPUBubbleSortBasedCollisionDetector::getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, double tolerance, std::vector<LikelyCollision> &collisions) {
    if(currentSortedSats == NULL) {
        currentSortedSats = new SphericalSatellite[nSats];
    } 
    if(t < currentSortedSatsTime) {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < nSats; i++) {
            currentSortedSats[i] = sats[i];
        }
        sortSatsByXValueCPUParallel(currentSortedSats, nSats, 100);
    } else {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < nSats; i++) {
            currentSortedSats[i] = sats[currentSortedSats[i].id];
        }
        sortSatsByXValueCPUParallelForAlreadyMostlySorted(currentSortedSats, nSats);
        for(int i = 0; i < nSats; i++) {
            
        }
    }
    currentSortedSatsTime = t;

    std::set<int> possibleCollidersSet;
    for (int i = 0; i < nPossibleColliders; i++) {
        possibleCollidersSet.insert(possibleColliders[i].id);
    }

    static double checkDistance;
    if(checkDistance == 0) {
        checkDistance = 3 * DISTANCE_STD_DEV;
        double checkProb = normalCDF(2 * satRadius - checkDistance, 0, DISTANCE_STD_DEV);
        while (checkProb > tolerance) {
            checkDistance += DISTANCE_STD_DEV;
            checkProb = normalCDF(2 * satRadius - checkDistance, 0, DISTANCE_STD_DEV);
        }
    }


    collisions.clear(); // Clear previous collisions

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < nSats - 1; i++) {
        if(possibleCollidersSet.count(currentSortedSats[i].id) == 0) {
            continue; // Skip if the satellite is not in the possible colliders set
        }
        int j = i;
        while(j + 1 < nSats && currentSortedSats[j+1].pos.x - currentSortedSats[i].pos.x <= checkDistance) j++;
        if(j == i) continue; // No possible collisions
        for (int k = i + 1; k <= j; k++) {
            SphericalSatellite &sat1 = sats[currentSortedSats[i].id];
            SphericalSatellite &sat2 = sats[currentSortedSats[k].id];

            CartesianCoordinates pos1 = sat1.pos;
            CartesianCoordinates pos2 = sat2.pos;
            double distance = sqrt(pow(pos1.x - pos2.x, 2) +
                                    pow(pos1.y - pos2.y, 2) +
                                    pow(pos1.z - pos2.z, 2));
            
            // Use same collision detectionas SequentialCollisionDetector
            double collisionProbability = normalCDF(2 * satRadius - distance, 0, DISTANCE_STD_DEV);

            if (collisionProbability > tolerance) {
                LikelyCollision collision = {sat1, sat2, collisionProbability, t};
                #pragma omp critical
                {
                    collisions.push_back(collision);
                }
            }
        }
    }
}
