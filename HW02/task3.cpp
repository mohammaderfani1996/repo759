#include<iostream>
#include<vector>
using namespace std;
#include "matmul.h"
#include <chrono>
#include <ratio>
#include <cmath>
#include <random>
using std::chrono::high_resolution_clock;
using std::chrono::duration;

int main(int argc, char *argv[]){
    if (argc != 1){
        cerr << "There is an error with an argument" << std::endl;
        return 1;
    }
    unsigned int n=1200;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0,1.0);
    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, milli> duration_sec_mul1;
    duration<double, milli> duration_sec_mul2;
    duration<double, milli> duration_sec_mul3;
    duration<double, milli> duration_sec_mul4;

 
    double A[n*n];
    double B[n*n];
    double C[n*n];
    
    for (unsigned int i=0; i<n*n; i++){
        A[i]=dis(gen);
    }
    for (unsigned int j=0; j<n*n; j++){
        B[j]=dis(gen);
    }
   
    fill(C, C + n * n, 0.0);

    start = high_resolution_clock::now();
    mmul1(A, B, C, n);
    end = high_resolution_clock::now();
    duration_sec_mul1 = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << n << endl;
    cout << duration_sec_mul1.count() << endl;
    cout<<C[n*n-1]<<endl;

    fill(C, C + n * n, 0.0);

    start = high_resolution_clock::now();
    mmul2(A, B, C, n);
    end = high_resolution_clock::now();
    duration_sec_mul2 = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << duration_sec_mul2.count() << endl;
    cout<<C[n*n-1]<<endl;

    fill(C, C + n * n, 0.0);
    start = high_resolution_clock::now();
    mmul3(A, B, C, n);
    end = high_resolution_clock::now();
    duration_sec_mul3 = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << duration_sec_mul3.count() << endl;
    cout<<C[n*n-1]<<endl;

    fill(C, C + n * n, 0.0);
    vector<double> B_vec(B, B + sizeof B / sizeof B[0]);
    vector<double> A_vec(A, A + sizeof A / sizeof A[0]);
    start = high_resolution_clock::now();
    mmul4(A_vec, B_vec, C, n);
    end = high_resolution_clock::now();
    duration_sec_mul4 = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    cout << duration_sec_mul4.count() << endl;
    cout<<C[n*n-1]<<endl;
    
    return 0;


}
