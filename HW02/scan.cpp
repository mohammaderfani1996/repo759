#include<iostream>

#include<string>
#include<algorithm>
#include<vector>
#include<cstdlib>
#include<cstring>
#include <cctype>
#include <fstream>
using namespace std;

#include "scan.h"

void scan(const float *arr, float *output, std::size_t n){
   
    for (std::size_t i=0; i<n; i++){
        float sum=0;
        for (std::size_t j=0; j<=i; j++){
            sum=sum+(arr[j]);
        }
        output[i]=sum;
    }
}
