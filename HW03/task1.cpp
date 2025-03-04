#include<iostream>
#include<vector>
using namespace std;
#include "matmul.h"
#include <chrono>
#include <ratio>
#include <cmath>
#include <random>
#include <omp.h>
using std::chrono::high_resolution_clock;
using std::chrono::duration;

int main(int argc, char *argv[]){
    if (argc != 3){
       cerr << "There is an error with an argument" << std::endl;
        return 1;
    }
    //std::size_t n=200;
    //int t=10;
    std::size_t n=std::atoi(argv[1]);
    int t=std::atoi(argv[2]);
    omp_set_num_threads(t);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0,1.0);
    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, milli> duration_sec_mul;
    

 
    float A[n*n];
    float B[n*n];
    float C[n*n];
    
    for (unsigned int i=0; i<n*n; i++){
        A[i]=dis(gen);
    }
    for (unsigned int j=0; j<n*n; j++){
        B[j]=dis(gen);
    }
   
    fill(C, C + n * n, 0.0);

    start = high_resolution_clock::now();
    mmul(A, B, C, n);
    end = high_resolution_clock::now();
    duration_sec_mul = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << C[0] << endl;
    cout<<C[n*n-1]<<endl;
    cout << duration_sec_mul.count() << endl;
    
    
    return 0;


}