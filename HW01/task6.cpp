#include <iostream>

int main(int argc, char *argv[]){
    if (argc != 2){
        std::cerr << "There is an error with an argument" << std::endl;
        return 1;
    }
    int N=std::atoi(argv[1]);
    for (int i=0; i<=N; i++){
        printf("%d ", i);
    }
    printf("\n");

    for (int j=N; j>=0; j--){
        std::cout << j << " ";
    }
    return 0;
    
}
