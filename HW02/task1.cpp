#include "scan.h"
#include<iostream>
#include<vector>
#include <random>
using namespace std;

#include <chrono>
#include <ratio>
#include <cmath>

using std::chrono::high_resolution_clock;
using std::chrono::duration;


int main(int argc, char *argv[]){
    if (argc != 2){
        cerr << "There is an error with an argument" << endl;
        return 1;
    }
    std::size_t n=std::atoi(argv[1]);
    std::random_device rd;
    std::mt19937 gen(rd());

 
    std::uniform_real_distribution<> dis(-1.0, 1.0); 
    //int n=100;

    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, milli> duration_sec;

    float* arr=(float*)malloc(n*sizeof(float));
    float* output=(float*)malloc(n*sizeof(float));
    for (std::size_t i=0; i<n; i++){
        //arr[i]=(-1)+static_cast<float> (rand()) / (static_cast <float> (RAND_MAX/(1-(-1))));
        arr[i]=dis(gen);
    }


    start = high_resolution_clock::now();
    scan(arr,output,n);
    end = high_resolution_clock::now();

    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    

    cout << duration_sec.count() << endl;

    cout<<output[0]<< endl;
    cout<<output[n-1]<<endl;
    free(arr);
    free(output);
    return 0;
}
