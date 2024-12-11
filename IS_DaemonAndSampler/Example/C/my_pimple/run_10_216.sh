#!/bin/sh
#PBS -N damBr5k
#PBS -P damBr5kv10
#PBS -q sc8d
#PBS -l select=4:ncpus=40:mpiprocs=4:ompthreads=10
#PBS -l walltime=00:15:00
#PBS -j oe

. /etc/profile.d/modules.sh
module purge
#module load gnu/9.5.0 openmpi/4.1.4 paraview/5.9.0_osmesa vtk/7.1.1
module load gnu/9.5.0 openmpi/4.1.4 vtk/7.1.1
#. /home/g6/a214026/pbvr/OpenFOAM_sample_test/OpenFOAM-10/etc/bashrc
. /home/g6/a214026/OpenFOAM/env/OpenFOAM-v2206/etc/bashrc

cd ${PBS_O_WORKDIR}

. $WM_PROJECT_DIR/bin/tools/RunFunctions

#source ~/.bashrc

#CURRENT=`pwd` #ジョブスクリプトでは機能しない
echo $CURRENT 
#mkdir $CURRENT/particle_out
export TF_NAME=default
#export VIS_PARAM_DIR=/home/g6/a214026/pbvr/UNPBVR/IntegPBVRv300_dev/CS-IS-PBVR/IS_DaemonAndSampler/Example/C/cavity_OpenFOAM
#export  PARTICLE_DIR=/home/g6/a214026/pbvr/UNPBVR/IntegPBVRv300_dev/CS-IS-PBVR/IS_DaemonAndSampler/Example/C/cavity_OpenFOAM/particle_out
export VIS_PARAM_DIR=$CURRENT
export  PARTICLE_DIR=$CURRENT/particle_out

runApplication blockMesh
runApplication setFields
runApplication decomposePar
#/home/g6/a214026/OpenFOAM/env/OpenFOAM-v10/platforms/linux64GccDPInt32Opt/bin/icoFoam
#mpirun -n 2 /home/g6/a214026/OpenFOAM/env/OpenFOAM-v10/platforms/linux64GccDPInt32Opt/bin/icoFoam -parallel
#mpirun -n 16 $CURRENT/bin/icoFoam -parallel
mpirun -n 4 bin/pimpleFoam -parallel
#mpirun -n 2 -npernode 1 bin/icoFoam -parallel
#runApplication $(getApplication) 

#/usr/bin/time -p mpirun $(getApplication) -parallel >& log_v10_10M-216.mpi


