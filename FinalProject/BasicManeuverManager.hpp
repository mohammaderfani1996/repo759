#ifndef BASICMANEUVERMANAGER_HPP
#define BASICMANEUVERMANAGER_HPP

#include "defs.hpp"
#include <vector>

class BasicManeuverManager : public ManeuverManager {
    private:
        std::vector<CollinearManuver> maneuvers; // List of maneuvers to be performed
    public:
        BasicManeuverManager() {}
        ~BasicManeuverManager() {}
        double scheduleManeuvers(std::vector<LikelyCollision> &collisions) override;
        void getOngoingManeuvers(double startTime, double endTime, std::vector<CollinearManuver> &ongoingManeuvers) override;
};

#endif