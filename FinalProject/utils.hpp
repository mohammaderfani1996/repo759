#ifndef CONVERSION_HPP
#define CONVERSION_HPP

#include "defs.hpp"

SphericalCoordinates orbitalToSpherical(const OrbitalCoordinates &orbit, double t);

CartesianCoordinates sphericalToCartesian(const SphericalCoordinates &sph);

CartesianCoordinates cross(const CartesianCoordinates& a, const CartesianCoordinates& b);\

double magnitude(const CartesianCoordinates& v);

CartesianCoordinates normalize(const CartesianCoordinates& v);

double normalCDF(double x, double mu, double sigma);

void singleManeuverHalfKick(CollinearManuver maneuver, double t, double dt);

void singleSatelliteGravityHalfKick(SphericalSatellite &sat, double dt);

void singleSatelliteDrift(SphericalSatellite &sat, double dt);

void sortSatsByXValueCPUParallel(SphericalSatellite * sats, const std::size_t n, const std::size_t threshold);

void sortSatsByXValueCPUParallelForAlreadyMostlySorted(SphericalSatellite * sats, const std::size_t n);

#endif