#include<iostream>
#include<vector>
#include "msort.h"
#include <algorithm>
using namespace std;
#include <cstddef>
#include <omp.h>


void merge(int* arr, int* left, std::size_t leftSize, int* right, std::size_t rightSize) {
    std::size_t i = 0, j = 0, k = 0;
    while (i < leftSize && j < rightSize) {
        if (left[i] <= right[j]) {
            arr[k++] = left[i++];
        } else {
            arr[k++] = right[j++];
        }
    }
    while (i < leftSize) arr[k++] = left[i++];
    while (j < rightSize) arr[k++] = right[j++];
}

void msort(int* arr, const std::size_t n, const std::size_t threshold){
    if (n <= threshold) {
        std::sort(arr, arr + n);  
        return;
    }

    std::size_t mid = n / 2;
    std::vector<int> left(arr, arr + mid);
    std::vector<int> right(arr + mid, arr + n);

    // **Create OpenMP tasks for parallel sorting**
    #pragma omp task shared(left)
    msort(left.data(), mid, threshold);

    #pragma omp task shared(right)
    msort(right.data(), n - mid, threshold);

    #pragma omp taskwait // Ensure both halves are sorted before merging

    // **Merge the sorted halves**
    merge(arr, left.data(), mid, right.data(), n - mid);

    //int min_value;
    //vector<int> inter_arr(arr, arr + n);
    //vector<int> result_arr;
    //for(std::size_t i=0; i<n; i++){
    //    min_value=inter_arr[0];
    //    for(std::size_t j=1; j<size(inter_arr); j++){
    //        if(inter_arr[j]<=min_value){
    //            min_value=inter_arr[j];
    //        }
    //    }
    //    inter_arr.erase(find(inter_arr.begin(), inter_arr.end(), min_value));
    //    result_arr.push_back(min_value);
   //     arr[i]=min_value;
   // }
    
}
