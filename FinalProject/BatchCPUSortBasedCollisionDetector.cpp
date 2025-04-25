#include "defs.hpp"
#include <cstdio>
#include <vector>
#include <set>
#include <cmath>
#include <stdio.h>
#include <omp.h> 
#include "BatchCPUSortBasedCollisionDetector.hpp"
#include "utils.hpp"

const double timeBatchSize = 1.0; // Time batch size in seconds

void BatchCPUSortBasedCollisionDetector::getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, int _, double tolerance, std::vector<LikelyCollision> &collisions) {
    if(currentSortedSats == NULL) {
        // printf("Allocating currentSortedSats\n");
        // fflush(stdout);
        currentSortedSats = new SphericalSatellite[nSats];
    }
    if(t > currentSortedSatsStartTime + timeBatchSize || t < currentSortedSatsStartTime) {
        // printf("New time batch detected, sorting\n");
        // fflush(stdout);
        currentSortedSatsStartTime = t;
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < nSats; i++) {
            currentSortedSats[i] = sats[i];
        }

        // Sort sats by x
        sortSatsByXValueCPUParallel(currentSortedSats, nSats, 100);
    }

    std::set<int> possibleCollidersSet;
    for (int i = 0; i < nPossibleColliders; i++) {
        possibleCollidersSet.insert(possibleColliders[i].id);
    }

    // if(possibleColliders != sats) {
    //     #pragma omp parallel for schedule(static)
    //     for(int i = 0; i < nPossibleColliders; i++) {
    //         //binary search for the possible collider in the currentSortedSats array
    //         int left = 0;
    //         int right = nSats - 1;
    //         while (left <= right) {
    //             int mid = left + (right - left) / 2;
    //             if (currentSortedSats[mid].id == possibleColliders[i].id) {
    //                 break;
    //             }
    //             if (currentSortedSats[mid].id < possibleColliders[i].id) {
    //                 left = mid + 1;
    //             } else {
    //                 right = mid - 1;
    //             }
    //         }
    //         int j = left;
    //         while(j + 1 < nSats && currentSortedSats[j+1].pos.x - currentSortedSats[left].pos.x <= 2 * satRadius) j++;
    //         if(j == left) continue; // No possible collisions
    //         for (int k = left + 1; k <= j; k++) {
    //             SphericalSatellite &sat1 = sats[currentSortedSats[left].id];
    //             SphericalSatellite &sat2 = sats[currentSortedSats[k].id];
    //             CartesianCoordinates pos1 = sat1.pos;
    //             CartesianCoordinates pos2 = sat2.pos;
    //             double distance = sqrt(pow(pos1.x - pos2.x, 2) +
    //                                     pow(pos1.y - pos2.y, 2) +
    //                                     pow(pos1.z - pos2.z, 2));
                
    //             // Use same collision detectionas SequentialCollisionDetector
    //             double collisionProbability = normalCDF(2 * satRadius - distance, 0, DISTANCE_STD_DEV);

    //             if (collisionProbability > tolerance) {
    //                 LikelyCollision collision = {sat1, sat2, collisionProbability, t};
    //                 #pragma omp critical
    //                 {
    //                     collisions.push_back(collision);
    //                 }
    //             }
    //         }
    //     }
    //     return;
    // }

    static double singleTimestepCheckDistance;
    if(singleTimestepCheckDistance == 0) {
        singleTimestepCheckDistance = 3 * DISTANCE_STD_DEV;
        double checkProb = normalCDF(2 * satRadius - singleTimestepCheckDistance, 0, DISTANCE_STD_DEV);
        while (checkProb > tolerance) {
            singleTimestepCheckDistance += DISTANCE_STD_DEV;
            checkProb = normalCDF(2 * satRadius - singleTimestepCheckDistance, 0, DISTANCE_STD_DEV);
        }
    }
    double maxTravelDistance = 2 * maxSpeed * (t - currentSortedSatsStartTime);
    double checkDistance = singleTimestepCheckDistance + maxTravelDistance;


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
