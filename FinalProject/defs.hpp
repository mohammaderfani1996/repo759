#ifndef DEFS_HPP
#define DEFS_HPP

#include <cmath>
#include <vector>

/** Ring around the Earth defined by its distance from the center of the Earth and its inclination angle.
 */
 struct OrbitalRing {
    double r; // Distance from center of Earth in meters
    double inclination; // Inclination angle in radians, 0 = North Pole, pi/2 = Equator, pi=South Pole
};


// Constants
const double G = 6.67430e-11;          // Gravitational constant
const double M = 5.972e24;         // Mass of the Earth in kg
const double earthRadius = 6378000.0; // Radius of the Earth in meters
const double satRadius = 5.0;         // Radius of the satellite in meters
const double satMass = 100.0;         // Mass of the satellite in kg
const double satThrust = 10.0;        // Thrust of the satellite in N
const OrbitalRing minPos = {earthRadius+160000.0, M_PI_4}; // Minimum position
const OrbitalRing maxPos = {earthRadius+2000000.0, 3*M_PI_4}; // Maximum position
const double midRadius = (minPos.r + maxPos.r) / 2.0;
const double semiMajor = (minPos.r + maxPos.r) / 2.0;
const double maxSpeed = sqrt(G * M * (2/minPos.r - 1/semiMajor)); // Maximum speed of a satellite
const double minSpeed = sqrt(G * M * (2/maxPos.r - 1/semiMajor)); // Minimum speed of a satellite
const double DISTANCE_STD_DEV = 500.0; // Standard deviation for distance noise, in meters
const double dt = (satRadius + DISTANCE_STD_DEV) / maxSpeed;        // Time step


/** Orbit of a satellite defined by its semi-major and semi-minor axes, inclination angle, and phase offset.
 */
struct OrbitalCoordinates {
    double inclination;  // Inclination angle in radians, 0 = along the equator, pi/2 = along the North pole
    double semiMajorAxis; // Semi-major axis of the orbit in meters
    double semiMinorAxis; // Semi-minor axis of the orbit in meters
    double phaseOffset; // Phase offset of the orbit in radians
};

struct SphericalCoordinates {
    double r; // distance from center of Earth in meters
    double theta; // azimuthal angle, from west to east, in radians, zero is arbitrary
    double phi; // inclination angle, from south to north, in radians, zero is the equator, pi/2 is the north pole
};

struct CartesianCoordinates {
    double x; // x coordinate in meters
    double y; // y coordinate in meters
    double z; // z coordinate in meters
};

struct SphericalSatellite {
    int id; // ID of the satellite
    CartesianCoordinates pos; // Position of the satellite in Cartesian coordinates
    CartesianCoordinates vel; // Velocity of the satellite in Cartesian coordinates
};

inline bool operator<(const SphericalSatellite& lhs, const SphericalSatellite& rhs)
{
  return lhs.id < rhs.id;
}


struct LikelyCollision {
    SphericalSatellite &sat1;
    SphericalSatellite &sat2;
    double probability; // The probability of collision
    double t; // The time of the collision
};

struct LikelyCollisionByIdx {
    int idx1;
    int idx2;
    double probability; // The probability of collision
    double t; 
};

struct CollinearManuver {
    SphericalSatellite &sat; // The satellite to be maneuvered
    double startTime; // The time of the maneuver
    double endTime; // The end time of the maneuver
    bool forward; // true if thrusting forward, false if thrusting backward
    bool active; // true if the maneuver is active, false otherwise
};

class CollisionDetector {
    public:
        CollisionDetector() {}
        ~CollisionDetector() {}
        /**
            * @brief Detects collisions between the given objects.
            * @param sats The array of SphericalSatellite objects to check for collisions.
            * @param nSats The number of SphericalSatellite objects in the array.
            * @param possibleColliders only these satellites are checked for collisions with the sats array.
            * @param nPossibleColliders The number of SphericalSatellite objects in the possibleColliders array.
            * @param t The time at which to check for collisions.
            * @param tolerance The minimum probability of collision to output.
            * @param collisions The array of LikelyCollision objects to store the detected collisions.
            * @return number of collisions detected.
         */
        virtual void getLikelyCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double t,int num_threads, double tolerance, std::vector<LikelyCollision> &collisions) = 0;
    };

class BatchCollisionDetector {
    public:
        BatchCollisionDetector() {}
        ~BatchCollisionDetector() {}
        /**
            * @brief Detects collisions between the given objects.
            * @param sats The array of SphericalSatellite objects to check for collisions.
            * @param nSats The number of SphericalSatellite objects in the array.
            * @param t The time at which to check for collisions.
            * @param tolerance The minimum probability of collision to output.
            * @param collisions The array of LikelyCollision objects to store the detected collisions.
         */
        virtual void simAndGetCollisions(SphericalSatellite sats[], int nSats, SphericalSatellite possibleColliders[], int nPossibleColliders, double tStart, double tEnd, double tolerance, std::vector<LikelyCollision> &collisions) = 0;
    };

class ManeuverManager {
    public:
        ManeuverManager() {}
        ~ManeuverManager() {}
        /**
            * @brief Schedules manuevers to be done
            * @param sats The array of SphericalSatellite objects to schedule maneuvers for.
            * @param collisions The array of LikelyCollision objects representing the detected collisions.
            * @return The time that simulation needs to go back and replay from (earliest start time of a newly scheduled maneuver).
         */
        virtual double scheduleManeuvers(std::vector<LikelyCollision> &collisions) = 0;

        /**
            * @brief Gets the maneuvering satellites in the given time range.
            * @param startTime The start time of the range to check for maneuvers.
            * @param endTime The end time of the range to check for maneuvers.
            * @param ongoingManeuvers all maneuvers that overlap with the time range.
            * @return The number of maneuvering satellites in the given time range.
         */
        virtual void getOngoingManeuvers(double startTime, double endTime, std::vector<CollinearManuver> &ongoingManeuvers) = 0;
    };

#endif
