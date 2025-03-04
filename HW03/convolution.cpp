#include<iostream>
#include<string>
#include<algorithm>
#include<vector>
#include<cstdlib>
#include<cstring>
#include <cctype>
#include <fstream>
using namespace std;
#include <cstddef>
#include <omp.h>
#include "convolution.h"


void convolve(const float *image, float *output, std::size_t n, const float *mask, std::size_t m){
    std::size_t row_index;
    std::size_t col_index;
    #pragma omp parallel for
    for(std::size_t x=0; x<n; x++){
        for (std::size_t y=0; y<n; y++){
            float sum=0;
            for (std::size_t i=0; i< m; i++){
                for (std::size_t j=0; j<m;j++){
                    row_index=x+i-(m-1)/2;
                    col_index=y+j-(m-1)/2;
                    if((row_index>=n || row_index<0) || (col_index>=n || col_index<0)){
                        //#pragma omp atomic
                        sum=sum+mask[i*m+j]*1;
                    }
                    else if((row_index>=n || row_index<0) && (col_index>=n || col_index<0)){
                        //#pragma omp atomic
                        sum=sum+mask[i*m+j]*0;
                    }
                   else{
                    //#pragma omp atomic
                    sum=sum + mask[i*m+j]*image[row_index*n+col_index];
                   }
                }
            }
            output[x*n+y]=sum;
        }
    }
}
