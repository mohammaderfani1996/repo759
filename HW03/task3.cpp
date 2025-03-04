#include "msort.h"
#include<iostream>
#include<vector>
#include <algorithm>
using namespace std;
#include <random>
#include <chrono>
#include <ratio>
#include <cmath>

using std::chrono::high_resolution_clock;
using std::chrono::duration;

int main(int argc, char *argv[]){
    if (argc != 4){
        cerr << "There is an error with an argument" << std::endl;
        return 1;
    }
    std::size_t n=std::atoi(argv[1]);
    int t=std::atoi(argv[2]);
    std::size_t ts=std::atoi(argv[3]);
    //std::printf("I can go w/ this many threads:%d\n", omp_get_max_threads());
    omp_set_num_threads(t);
    //int n=4;
    //int m=3;
    //high_resolution_clock::time_point start;
    //high_resolution_clock::time_point end;
    //duration<double, milli> duration_sec;
    
  
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(-1000, 1000);

  
    int* arr = new int[n];  
    for (std::size_t i = 0; i < n; i++) {
        arr[i] = dis(gen);
    }
    

    //auto start = high_resolution_clock::now();
    double start = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        {
            msort(arr, n, ts);
        }
    }
    double end = omp_get_wtime();
    //auto end = high_resolution_clock::now();
    double duration_msec=(end - start)*1000;
    //duration<double, std::milli> duration_sec = end - start;    

    
    cout<<arr[0]<< endl;
    cout<<arr[n-1]<<endl;
    cout << duration_msec << endl;
  
  
    return 0;


}