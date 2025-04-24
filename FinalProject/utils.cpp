#include "defs.hpp"
#include <cmath>
#include <stdio.h>
#include <omp.h>

SphericalCoordinates orbitalToSpherical(const OrbitalCoordinates &orbit, double t) {
    // Calculate the position of the satellite in spherical coordinates
    SphericalCoordinates pos;
    double period = sqrt(4 * M_PI * M_PI * pow(orbit.semiMajorAxis, 3) / (G * M));
    double eccentricity = sqrt(1 - pow(orbit.semiMinorAxis, 2) / pow(orbit.semiMajorAxis, 2));
    printf("eccentricity: %lf\n", eccentricity);
    double semiLatusRectum = pow(orbit.semiMinorAxis, 2) / orbit.semiMajorAxis; 
    pos.theta = orbit.phaseOffset + 2 * M_PI * t / period;
    pos.r = semiLatusRectum / (1 + eccentricity * cos(pos.theta));
    pos.phi = orbit.inclination;

    return pos;
}

CartesianCoordinates sphericalToCartesian(const SphericalCoordinates &sph) {
    // Convert spherical coordinates to Cartesian coordinates
    CartesianCoordinates cart;
    cart.x = sph.r * sin(sph.phi) * cos(sph.theta);
    cart.y = sph.r * sin(sph.phi) * sin(sph.theta);
    cart.z = sph.r * cos(sph.phi);

    return cart;
}

CartesianCoordinates cross(const CartesianCoordinates& a, const CartesianCoordinates& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double magnitude(const CartesianCoordinates& v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

CartesianCoordinates normalize(const CartesianCoordinates& v) {
    double mag = magnitude(v);
    return {v.x / mag, v.y / mag, v.z / mag};
}

double normalCDF(double x, double mu, double sigma) {
    return std::erfc(-(x - mu) / (sigma * std::sqrt(2))) / 2;
}


void singleManeuverHalfKick(CollinearManuver maneuver, double t, double dt) {
    if(maneuver.endTime < t || maneuver.startTime > t) {
        return;
    }
    if(!maneuver.active) {
        if(dt < 0) {
            return;
        } else {
            maneuver.active = true;
        }
    }
    SphericalSatellite &sat = maneuver.sat;
    double sign = 1.0;
    if(!maneuver.forward) {
        sign = -1.0;
    }
    double deltaSpeed = sign * satThrust / satMass * dt / 2.0; // change in velocity
    double currSpeed = magnitude(sat.vel);
    double newSpeed = currSpeed + deltaSpeed;
    double scaleBy  = newSpeed / currSpeed;

    sat.vel.x *= scaleBy;
    sat.vel.y *= scaleBy;
    sat.vel.z *= scaleBy;
}

void singleSatelliteGravityHalfKick(SphericalSatellite &sat, double dt) {
    double r = magnitude(sat.pos);
    double aMagnitude = (G*M) / (r * r);
    double deltaVMagnitude = aMagnitude * dt / 2.0; // change in velocity
    
    // determine direction of change
    double u_x = -(sat.pos.x / r);
    double u_y = -(sat.pos.y / r);
    double u_z = -(sat.pos.z / r);

    sat.vel.x += deltaVMagnitude * u_x;
    sat.vel.y += deltaVMagnitude * u_y;
    sat.vel.z += deltaVMagnitude * u_z;
}

void singleSatelliteDrift(SphericalSatellite &sat, double dt) {
    sat.pos.x += sat.vel.x * dt;
    sat.pos.y += sat.vel.y * dt;
    sat.pos.z += sat.vel.z * dt;
}


auto comp = [](const SphericalSatellite& a, const SphericalSatellite& b) {
    return a.pos.x < b.pos.x;
};

// Parallel Quick Sort from this article: https://mcbeukman.medium.com/parallel-quicksort-using-openmp-9d18d7468cac
int partition(int p, int r, SphericalSatellite* data) {
    SphericalSatellite pivot = data[r];
    int i = p - 1;
    for (int j = p; j < r; j++) {
        if (comp(data[j], pivot)) {
            i++;
            std::swap(data[i], data[j]);
        }
    }
    std::swap(data[i + 1], data[r]);
    return i + 1;
}

void seq_qsort(int p, int r, SphericalSatellite* data) {
    if (p < r) {
        int q = partition(p, r, data);
        seq_qsort(p, q - 1, data);
        seq_qsort(q + 1, r, data);
    }
}

void q_sort_tasks(int p, int r, SphericalSatellite* data, int low_limit) {
    if (p < r) {
        if (r - p < low_limit) {
            seq_qsort(p, r, data);
        } else {
            int q = partition(p, r, data);
            #pragma omp task shared(data)
            q_sort_tasks(p, q - 1, data, low_limit);
            #pragma omp task shared(data)
            q_sort_tasks(q + 1, r, data, low_limit);
        }
    }
}

void sortSatsByXValueCPUParallelForAlreadyMostlySorted(SphericalSatellite * sats, const std::size_t n) {
    static long calls = 0;
    static long iters = 0;
    calls++;
	std::size_t i=0, j=0;
	std::size_t first;
    bool stop;
	for( i = 0; i < n-1; i++ )
	{
        stop = true;
		first = i % 2; 
		#pragma omp parallel for default(none),shared(sats,first,n,comp),reduction(&&: stop)
		for( j = first; j < n-1; j += 1 )
		{
            if(!comp(sats[j], sats[j+1])) {
                SphericalSatellite temp = sats[j];
                sats[j] = sats[j+1];
                sats[j+1] = temp;
                stop = false;
            }
		}
        if(stop) {
            break;
        }
	}
    iters += i+1;
    // if(calls % 1000 == 0) {
    //     printf("Bubble sort avg iters: %ld\n", iters / calls);
    //     fflush(stdout);
    // }
}

void sortSatsByXValueCPUParallel(SphericalSatellite * sats, const std::size_t n, const std::size_t threshold) {
    if(n <= threshold) {
        // selection sort
        for (size_t i = 0; i < n; i++) {
            for (size_t j = i + 1; j < n; j++) {
                if (!comp(sats[i], sats[j])) {
                    // swap
                    SphericalSatellite temp = sats[i];
                    sats[i] = sats[j];
                    sats[j] = temp;
                }
            }
        }
    } else {
        // parallelize on divide
        size_t mid = n / 2;
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                sortSatsByXValueCPUParallel(sats, mid, threshold);
            }
            #pragma omp section
            {
                sortSatsByXValueCPUParallel(sats + mid, n - mid, threshold);
            }
        }
        // merge
        SphericalSatellite* temp = new SphericalSatellite[n];
        size_t i = 0;
        size_t j = mid;
        size_t k = 0;
        while (i < mid && j < n) {
            if (comp(sats[i], sats[j])) {
                temp[k++] = sats[i++];
            } else {
                temp[k++] = sats[j++];
            }
        }
        while (i < mid) {
            temp[k++] = sats[i++];
        }
        while (j < n) {
            temp[k++] = sats[j++];
        }
        for (size_t i = 0; i < n; i++) {
            sats[i] = temp[i];
        }
        delete[] temp;
    }
}

