#!/bin/sh
#PBS -q h432
#PBS -l select=1:ncpus=24:mpiprocs=6:ompthreads=4
#PBS -P pbvrInSituTest@PG18019
#PBS -l walltime=1:00:00
cd $PBS_O_WORKDIR
./etc/profile.d/modules.sh
module load intel/cur  mpt/cur
mpijob ./run > ./output.log 2>&1



