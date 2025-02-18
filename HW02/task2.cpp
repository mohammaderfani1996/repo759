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
    std::size_t m=std::atoi(argv[2]);
    //int n=4;
    //int m=3;
    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, milli> duration_sec;
 
    float* image=new float[n*n];
    float* mask=new float[m*m];
    float* output=new float[n*n];
    for (std::size_t i=0; i<n*n; i++){
        image[i]=(-10)+static_cast<float> (rand()) / (static_cast <float> (RAND_MAX/(10-(-10))));
    }
    for (std::size_t j=0; j<m*m; j++){
        mask[j]=(-1)+static_cast<float> (rand()) / (static_cast <float> (RAND_MAX/(1-(-1))));
    }
   


    start = high_resolution_clock::now();
    convolve(image,output,n,mask,m);
    end = high_resolution_clock::now();

    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    

    cout << duration_sec.count() << endl;
    cout<<output[0]<< endl;
    cout<<output[n*n-1]<<endl;
    delete[] image;
    delete[] mask;
    delete[] output;
    return 0;


}
