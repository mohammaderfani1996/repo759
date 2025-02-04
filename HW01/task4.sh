#!/usr/bin/env zsh
#SBATCH --job-name=FirstSlurm
#SBATCH --partition=instruction
#SBATCH --time 00:00:10
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=2
#SBATCH --gpus-per-task=1
#SBATCH --output="FirstSlurm.output"
#SBATCH --error="FirstSlurm.err"
# log in the submission directory
cd $SLURM_SUBMIT_DIR

hostname
