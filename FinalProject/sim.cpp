#include <cstddef>
#include <cstdio>
#include <iostream>
#include <fstream>  
#include <cmath>
#include <cstdlib>  
#include <ctime>   
#include <chrono>
#include <random>
#include <omp.h>
#include <set>
#include <vector>
#include <algorithm>
#include "defs.hpp"
#include "utils.hpp"
#include "SequentialCollisionDetector.hpp"
#include "NaiveCPUParallelCollisionDetector.hpp"
#include "BasicManeuverManager.hpp"
#include "CPUSortBasedCollisionDetector.hpp"
#include "BatchCPUSortBasedCollisionDetector.hpp"
#include "GPUGridBasedCollisionDetector.cuh"
#include "GridCollisionDetector.hpp"
#include "GPUNaiveCollisionDetector.cuh"
#include "GPUSortBasedCollisionDetector.cuh"
#include "CPUBubbleSortBasedCollisionDetector.hpp"


using std::chrono::high_resolution_clock;
using std::chrono::duration;

// For debug: save positions to a CSV file
void savePositionsToCSV(const SphericalSatellite sat[], int N, double t, const std::string &filename) {
    std::ofstream file;
    
    // Open the file in append mode
    file.open(filename, std::ios_base::app);

    if (file.is_open()) {
        file << t << ",[";
        for (int i = 0; i < N; i++) {
            if (i != N - 1)
                file << "[" << sat[i].pos.x << "," << sat[i].pos.y << "," << sat[i].pos.z << "],";
            else
                file << "[" << sat[i].pos.x << "," << sat[i].pos.y << "," << sat[i].pos.z << "]";
        }
        file << "]\n";  // Newline for separation between steps
        file.close();
    } else {
        std::cerr << "Unable to open file!" << std::endl;
    }
}

int main(int argc, char *argv[]) {


    // Check if correct number of arguments are provided
    if (argc < 6) {
        std::cerr << "Usage: " << argv[0] << "<number_of_satellites> <simulation_end_time> <num_threads> <random_seed> <frame_sampling_interval> flags" << std::endl;
        std::cerr << "Flags: --force-collision --p-kick-drift --p-naive-cpu --p-naive-gpu --p-cpu-sort --p-batch-cpu-sort --p-gpu-sort --p-gpu-grid --p-cpu-grid --p-cpu-bubble-sort --disable-maneuvers --disable-print-collisions" << std::endl;
        return 1;
    }

    // Read N and tEnd from command line
    int N = std::stoi(argv[1]);     // Number of satellites

    double tEnd = std::stod(argv[2]); // Time at which simulation ends
    int num_threads = std::stoi(argv[3]); // Number of threads

    int random_seed = std::stoi(argv[4]); // Random 

    double frame_sampling_interval = std::stod(argv[5]); // Frame sampling interval

    bool force_collision = false;
    bool disable_maneuvers = false;
    bool disable_print_collisions = false;
    bool parallel_kick_drift = false;
    bool naive_cpu_parallel_collision_detection = false;
    bool cpu_sort_based = false;
    bool cpu_bubble_sort_based = false;
    bool batch_cpu_sort_based = false;
    bool gpu_grid_based = false;
    bool grid_cpu_based = false;
    bool gpu_sort_based = false;
    bool gpu_naive = false;
    for(int i = 6; i < argc; i++) {
        if (std::string(argv[i]) == "--force-collision") {
            force_collision = true;
        }
        if (std::string(argv[i]) == "--disable-maneuvers") {
            disable_maneuvers = true;
        }
        if (std::string(argv[i]) == "--disable-print-collisions") {
            disable_print_collisions = true;
        }
        if (std::string(argv[i]) == "--p-kick-drift") {
            parallel_kick_drift = true;
        } else if (std::string(argv[i]) == "--p-naive-cpu") {
            naive_cpu_parallel_collision_detection = true;
        } else if (std::string(argv[i]) == "--p-cpu-sort") {
            cpu_sort_based = true;
        } else if( std::string(argv[i]) == "--p-cpu-bubble-sort") {
            cpu_bubble_sort_based = true;
        } else if (std::string(argv[i]) == "--p-batch-cpu-sort") {
            batch_cpu_sort_based = true;
        } else if (std::string(argv[i]) == "--p-gpu-grid") {
            gpu_grid_based = true;
        } else if (std::string(argv[i]) == "--p-cpu-grid") {
            grid_cpu_based = true;
        } else if (std::string(argv[i]) == "--p-gpu-sort") {
            gpu_sort_based = true;
        } else if (std::string(argv[i]) == "--p-naive-gpu") {
            gpu_naive = true;
        }
    }
    
    printf("maxSpeed: %lf\n", maxSpeed);
    printf("dt: %lf\n", dt);
    printf("tEnd: %lf\n", tEnd);
    printf("N: %d\n", N);
    printf("num_threads: %d\n", num_threads);
    printf("random_seed: %d\n", random_seed);
    printf("frame_sampling_interval: %lf\n", frame_sampling_interval);
    printf("force_collision: %d\n", force_collision);
    printf("parallel_kick_drift: %d\n", parallel_kick_drift);
    printf("naive_cpu_parallel_collision_detection: %d\n", naive_cpu_parallel_collision_detection);
    printf("cpu_sort_based_collision_detector: %d\n", cpu_sort_based);
    printf("gpu_sort_based_collision_detector: %d\n", gpu_sort_based);
    printf("gpu_naive_collision_detector: %d\n", gpu_naive);
    printf("gpu_sort_based_collision_detector: %d\n", gpu_grid_based);
    printf("batch_cpu_sort_based_collision_detector: %d\n", batch_cpu_sort_based);
    fflush(stdout);


    // Set number of threads
    omp_set_num_threads(num_threads);

    // File to save positions
    std::string filename = "positions.csv";

    // Clear the file before starting simulation (optional)
    std::ofstream file;
    file.open(filename, std::ofstream::out | std::ofstream::trunc);
    file.close();

    // Allocate dynamic arrays based on N
    SphericalSatellite *satellites = new SphericalSatellite[N];

    // Create a random number engine
    std::mt19937 generator(random_seed);

    // Create random distributions
    std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);
    std::normal_distribution<double> normal_dist(0.0, 1.0);

    // Simulation parameters
    double t = 0.0;

    // Set initial masses and random positions/velocities
    for (int i = 0; i < N; i++) {
        satellites[i].id = i;
        SphericalCoordinates sph;
        sph.r = minPos.r + (maxPos.r - minPos.r) * uniform_dist(generator);
        sph.phi = minPos.inclination + (maxPos.inclination - minPos.inclination) * uniform_dist(generator);
        sph.theta = 2 * M_PI * uniform_dist(generator);

        if(force_collision) {
            //force immediate collision by making initial pos of all satellites the same
            sph.r = midRadius;
            sph.phi = (maxPos.inclination + minPos.inclination) / 2.0;
            sph.theta = 0;
        }

        satellites[i].pos = sphericalToCartesian(sph);

        CartesianCoordinates r_hat = normalize(satellites[i].pos);

        // Create a unit vector in the direction of the orbit plane
        CartesianCoordinates orbit_normal = sphericalToCartesian({
            .r = 1.0,
            .theta = 0.0,
            .phi = sph.phi + M_PI/2.0 // Offset to get plane normal
        });

        // Velocity is perpendicular to both r_hat and orbit_normal
        CartesianCoordinates vdir = normalize(cross(orbit_normal, r_hat));

        double speed = sqrt(G * M / sph.r); // circular orbit speed at radius
        double speedNoise = normal_dist(generator) * 100.0; // Add some noise to the speed
        if(sph.r <= midRadius) {
            speed += speedNoise;
        } else {
            speed -= speedNoise;
        }
        satellites[i].vel = {
            speed * vdir.x,
            speed * vdir.y,
            speed * vdir.z
        };

        if(force_collision && N == 2 && i == 0) {
            satellites[i].pos.x = 2739495.699246;
            satellites[i].pos.y = 6988128.625091;
            satellites[i].pos.z = 0.0;
            satellites[i].vel.x = 6771.965681;
            satellites[i].vel.y = -2728.619715;
            satellites[i].vel.z = 0.0;
        }
        if(force_collision && N == 2 && i == 1) {
            satellites[i].pos.x = 2762465.737001;
            satellites[i].pos.y = 7072242.049592;
            satellites[i].pos.z = 0.0;
            satellites[i].vel.x = 6713.969935;
            satellites[i].vel.y = -2829.168149;
            satellites[i].vel.z = 0.0;
        }

    }
    CollisionDetector *collisionDetector;
    if(naive_cpu_parallel_collision_detection) {
        collisionDetector = new NaiveCPUParallelCollisionDetector();
    }else if(cpu_sort_based){
        collisionDetector = new CPUSortBasedCollisionDetector();
    } else if(cpu_bubble_sort_based) {
        collisionDetector = new CPUBubbleSortBasedCollisionDetector();
    } else if(batch_cpu_sort_based) {
        collisionDetector = new BatchCPUSortBasedCollisionDetector();
    } else if (grid_cpu_based) {
        collisionDetector = new GridCollisionDetector();
    } else if (gpu_grid_based) {
        collisionDetector = new GPUGridBasedCollisionDetector();
    } else if (gpu_sort_based) {
        collisionDetector = new GPUSortBasedCollisionDetector(N);
    } else if (gpu_naive) {
        collisionDetector = new GPUNaiveCollisionDetector(N);
    } else {
        collisionDetector = new SequentialCollisionDetector();
    }


    ManeuverManager * maneuverManager = new BasicManeuverManager();

    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, std::milli> duration_sec;
    start = high_resolution_clock::now();

    // Main simulation loop
    double lastBacktrackTime = 0;
    double goToTime = tEnd;
    double highestTimeReached = 0;
    double currDt = dt;
    int step = 0;
    while (t < tEnd) {
        //get ongoing maneuvers
        std::vector<CollinearManuver> ongoingManeuvers;
        maneuverManager->getOngoingManeuvers(lastBacktrackTime, t, ongoingManeuvers);

        //check for collisions if not backtracing, schedule maneuvers if there are any
        if(currDt > 0) {
            std::vector<LikelyCollision> collisions;
            SphericalSatellite *possibleColliders = satellites;
            int numPossibleColliders = N;
            if (t < highestTimeReached) {
                std::set<SphericalSatellite> colliderSet;
                for (size_t i = 0; i < ongoingManeuvers.size(); i++) {
                    colliderSet.insert(ongoingManeuvers[i].sat);
                }
                numPossibleColliders = colliderSet.size();
                possibleColliders = new SphericalSatellite[numPossibleColliders];
                std::copy(colliderSet.begin(), colliderSet.end(), possibleColliders);
            }
            collisionDetector->getLikelyCollisions(satellites, N, possibleColliders, numPossibleColliders, t,num_threads, 1e-6, collisions);
            if (t < highestTimeReached) {
                delete[] possibleColliders;
            }
            if (collisions.size() > 0) {
                // Print the detected collisions
                if(!disable_print_collisions) {
                    for (size_t i = 0; i < collisions.size(); i++) {
                        printf("Collision detected between satellite %d and satellite %d with probability %f at time %f\n", collisions[i].sat1.id, collisions[i].sat2.id, collisions[i].probability, collisions[i].t);
                    }
                    fflush(stdout);
                }
                if(!disable_maneuvers) {
                    goToTime = maneuverManager->scheduleManeuvers(collisions);
                    lastBacktrackTime = goToTime;
                }
            }
        }
        
        if(goToTime < t) {
            currDt = -dt;
        } else {
            goToTime = tEnd;
            currDt = dt;
        }

        // 1/2 kick based on maneuvers
        if(parallel_kick_drift) {
            #pragma omp parallel for
            for (size_t i = 0; i < ongoingManeuvers.size(); i++) {
                singleManeuverHalfKick(ongoingManeuvers[i], t, dt);
            }
        } else {
            for (size_t i = 0; i < ongoingManeuvers.size(); i++) {
                singleManeuverHalfKick(ongoingManeuvers[i], t, dt);
            }
        }

        // 1/2 kick based on Earth's gravity
        if(parallel_kick_drift) {
            #pragma omp parallel for
            for (int i = 0; i < N; i++) {
                singleSatelliteGravityHalfKick(satellites[i], currDt);
            }
        } else {
            for (int i = 0; i < N; i++) {
                singleSatelliteGravityHalfKick(satellites[i], currDt);
            }
        }

        // drift
        if(parallel_kick_drift) {
            #pragma omp parallel for
            for (int i = 0; i < N; i++) {
                singleSatelliteDrift(satellites[i], currDt);
            }
        } else {
            for (int i = 0; i < N; i++) {
                singleSatelliteDrift(satellites[i], currDt);
            }
        }

        // 1/2 kick based on maneuvers
        if(parallel_kick_drift) {
            #pragma omp parallel for
            for (size_t i = 0; i < ongoingManeuvers.size(); i++) {
                singleManeuverHalfKick(ongoingManeuvers[i], t, dt);
            }
        } else {
            for (size_t i = 0; i < ongoingManeuvers.size(); i++) {
                singleManeuverHalfKick(ongoingManeuvers[i], t, dt);
            }
        }
    
        // 1/2 kick based on Earth's gravity
        if(parallel_kick_drift) {
            #pragma omp parallel for
            for (int i = 0; i < N; i++) {
                singleSatelliteGravityHalfKick(satellites[i], currDt);
            }
        } else {
            for (int i = 0; i < N; i++) {
                singleSatelliteGravityHalfKick(satellites[i], currDt);
            }
        }


        // For debug and animation: save positions to CSV at each step
        if (step % (int)(frame_sampling_interval / dt) == 0) {
            savePositionsToCSV(satellites, N, t, filename);
        }

        t += currDt;
        step++;

        if(t > highestTimeReached) {
            highestTimeReached = t;
        }
    }

    /*
    Alternative algorithm:

    detect collisons at all time steps
    remove collisions after the first one for each satellite
    schedule maneuvers to avoid each collision
    repeat unless there were no collisions / under certain probability
    */ 


    // Clean up dynamically allocated memory
    delete[] satellites;

    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli> >(end - start);
    std::cout << "time: " << duration_sec.count() << "ms\n";
    fflush(stdout);

    return 0;
}
