#!/usr/bin/env zsh
#SBATCH -p instruction
#SBATCH -c 8
#SBATCH -J sim
#SBATCH -o sim.out -e sim.err

module load nvidia/cuda/11.8.0

nvcc *.cu *.cpp -Xcompiler -fopenmp -Xcompiler -O3 -Xcompiler -Wall -Xptxas -O3 -std=c++17 -o sim 
./sim $@
