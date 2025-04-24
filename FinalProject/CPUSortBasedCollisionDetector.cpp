#include "defs.hpp"
#include <vector>
#include <set>
#include <cmath>
#include <stdio.h>
#include <omp.h> 
#include "CPUSortBasedCollisionDetector.hpp"
#include "utils.hpp"



void CPUSortBasedCollisionDetector::getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t, double tolerance, std::vector<LikelyCollision> &collisions) {
    static double checkDistance;
    if(checkDistance == 0) {
        checkDistance = 3 * DISTANCE_STD_DEV;
        double checkProb = normalCDF(2 * satRadius - checkDistance, 0, DISTANCE_STD_DEV);
        while (checkProb > tolerance) {
            checkDistance += DISTANCE_STD_DEV;
            checkProb = normalCDF(2 * satRadius - checkDistance, 0, DISTANCE_STD_DEV);
        }
    }

    SphericalSatellite * sats2 = new SphericalSatellite[nSats];
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < nSats; i++) {
        sats2[i] = sats[i];
    }

    collisions.clear(); // Clear previous collisions

    // Init sats, array with satellite info and coordinates
    std::set<int> possibleCollidersSet;
    for (int i = 0; i < nPossibleColliders; i++) {
        possibleCollidersSet.insert(possibleColliders[i].id);
    }

    // Sort sats by x
    sortSatsByXValueCPUParallel(sats2, nSats, 100);


    // if(possibleColliders != sats) {
    //     #pragma omp parallel for schedule(static)
    //     for(int i = 0; i < nPossibleColliders; i++) {
    //         //binary search for the possible collider in the currentSortedSats array
    //         int left = 0;
    //         int right = nSats - 1;
    //         while (left <= right) {
    //             int mid = left + (right - left) / 2;
    //             if (sats2[mid].id == possibleColliders[i].id) {
    //                 break;
    //             }
    //             if (sats2[mid].id < possibleColliders[i].id) {
    //                 left = mid + 1;
    //             } else {
    //                 right = mid - 1;
    //             }
    //         }
    //         int j = left;
    //         while(j + 1 < nSats && sats2[j+1].pos.x - sats2[left].pos.x <= 2 * satRadius) j++;
    //         if(j == left) continue; // No possible collisions
    //         for (int k = left + 1; k <= j; k++) {
    //             SphericalSatellite &sat1 = sats[sats2[left].id];
    //             SphericalSatellite &sat2 = sats[sats2[k].id];
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

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < nSats - 1; i++) {
        if(possibleCollidersSet.count(sats2[i].id) == 0) {
            continue; // Skip if the satellite is not in the possible colliders set
        }
        int j = i;
        while(sats2[j+1].pos.x - sats2[i].pos.x <= checkDistance) j++;
        if(j == i) continue; // No possible collisions
        for (int k = i + 1; k <= j; k++) {
            CartesianCoordinates pos1 = sats2[i].pos;
            CartesianCoordinates pos2 = sats2[k].pos;
            double distance = sqrt(pow(pos1.x - pos2.x, 2) +
                                    pow(pos1.y - pos2.y, 2) +
                                    pow(pos1.z - pos2.z, 2));
            
            // Use same collision detectionas SequentialCollisionDetector
            double collisionProbability = normalCDF(2 * satRadius - distance, 0, DISTANCE_STD_DEV);

            if (collisionProbability > tolerance) {
                SphericalSatellite &sat1 = sats[sats2[i].id];
                SphericalSatellite &sat2 = sats[sats2[k].id];
                LikelyCollision collision = {sat1, sat2, collisionProbability, t};
                #pragma omp critical
                {
                    collisions.push_back(collision);
                }
            }
        }
    }
    delete[] sats2;
}
