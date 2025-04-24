#include <stdlib.h>
#include <stdio.h>
#include <climits>
#include <vector>
#include <set>
#include "defs.hpp"
#include "utils.hpp"
#include "BasicManeuverManager.hpp"


double BasicManeuverManager::scheduleManeuvers(std::vector<LikelyCollision> &collisions) {
    // Schedule maneuvers based on the detected collisions
    int earliestStartTime = INT_MAX;
    for (size_t i = 0; i < collisions.size(); i++) {
        SphericalSatellite &sat = collisions[i].sat1;
        
        double duration = 100.0 / (satThrust / satMass); //accelerate until speed changed by numerator m/s
        double startTime = collisions[i].t - 3600*3; // 3 hours before the collision
        if (startTime < earliestStartTime) {
            earliestStartTime = startTime;
        }
        double endTime = startTime + duration;

        bool forward = true;
        double speed = magnitude(sat.vel);
        double r = magnitude(sat.pos);
        double circOrbitSpeed = sqrt(G * M / r);
        if(speed > circOrbitSpeed) {
            forward = false; // Thrust in the direction of motion
        }

        bool active = false;

        // Create a maneuver object
        CollinearManuver maneuver = {sat, startTime, endTime, forward, active};

        // Store the maneuver
        maneuvers.push_back(maneuver);

        printf("Scheduled maneuver for satellite %d from %lf to %lf with thrust direction %d\n", sat.id, startTime, endTime, forward);
        fflush(stdout);
    }

    return earliestStartTime;
}


void BasicManeuverManager::getOngoingManeuvers(double startTime, double endTime, std::vector<CollinearManuver> &ongoingManeuvers) {
    ongoingManeuvers.clear();

    // Get the list of satellites that have ongoing maneuvers during any portion of the time range
    std::set<int> maneuveringSatellites;
    for (const auto &maneuver : maneuvers) {
        if (startTime >= startTime || endTime <= endTime) {
            if(maneuveringSatellites.count(maneuver.sat.id) == 0) {
                maneuveringSatellites.insert(maneuver.sat.id);
                ongoingManeuvers.push_back(maneuver);
            }
        }
    }
}
