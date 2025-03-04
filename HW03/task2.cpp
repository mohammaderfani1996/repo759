#include "convolution.h"
#include<iostream>
#include<vector>
using namespace std;
#include <random>
#include <chrono>
#include <ratio>
#include <cmath>

using std::chrono::high_resolution_clock;
using std::chrono::duration;

int main(int argc, char *argv[]){
    if (argc != 3){
        cerr << "There is an error with an argument" << std::endl;
        return 1;
    }
    std::size_t n=std::atoi(argv[1]);
    int t=std::atoi(argv[2]);
    std::size_t m=3;
    //std::printf("I can go w/ this many threads:%d\n", omp_get_max_threads());
    omp_set_num_threads(t);
    //int n=4;
    //int m=3;
    //high_resolution_clock::time_point start;
    //high_resolution_clock::time_point end;
    //duration<double, milli> duration_sec;
 
    float* image=new float[n*n];
    float* mask=new float[m*m];
    float* output=new float[n*n];
    for (std::size_t i=0; i<n*n; i++){
        image[i]=(-10)+static_cast<float> (rand()) / (static_cast <float> (RAND_MAX/(10-(-10))));
    }
    for (std::size_t j=0; j<m*m; j++){
        mask[j]=(-1)+static_cast<float> (rand()) / (static_cast <float> (RAND_MAX/(1-(-1))));
    }
   


    //start = high_resolution_clock::now();
    double start = omp_get_wtime();
    convolve(image,output,n,mask,m);
    double end = omp_get_wtime();
    //end = high_resolution_clock::now();
    double duration_msec=(end - start)*1000;
    //duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    

    
    cout<<output[0]<< endl;
    cout<<output[n*n-1]<<endl;
    cout << duration_msec << endl;
    delete[] image;
    delete[] mask;
    delete[] output;
    return 0;


}
