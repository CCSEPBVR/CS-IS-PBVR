#!/bin/sh
#PBS -N damBr5k
#PBS -P damBr5kv10
#PBS -q sc8d
#PBS -l select=1:ncpus=2:mpiprocs=2:ompthreads=1
#PBS -l walltime=00:20:00
#PBS -j oe

. /etc/profile.d/modules.sh
module purge
module load gnu/9.5.0 openmpi/4.1.4 paraview/5.9.0_osmesa vtk/7.1.1
#module load cuda/11.4 gnu/cur intel/cur  openmpi/4.1.4 paraview/5.9.0_osmesa vtk/7.1.1
#. /home/app/OpenFOAM/OpenFOAM-10/etc/bashrc
. /home/g6/a214026/pbvr/OpenFOAM_sample_test/OpenFOAM-10/etc/bashrc

#CURRENT=$(cd $(dirname $0);pwd) #ジョブスクリプトでは機能しない
#echo $CURRENT 
#mkdir $CURRENT/particle_out
export TF_NAME=default
export VIS_PARAM_DIR=/home/g6/a214026/pbvr/UNPBVR/IntegPBVRv300_dev/CS-IS-PBVR/IS_DaemonAndSampler/Example/C/cavity_OpenFOAM
export  PARTICLE_DIR=/home/g6/a214026/pbvr/UNPBVR/IntegPBVRv300_dev/CS-IS-PBVR/IS_DaemonAndSampler/Example/C/cavity_OpenFOAM/particle_out
#export VIS_PARAM_DIR=$CURRENT
#export  PARTICLE_DIR=$CURRENT/particle_out

cd ${PBS_O_WORKDIR}

. $WM_PROJECT_DIR/bin/tools/RunFunctions

runApplication blockMesh
runApplication setFields
runApplication decomposePar
#runApplication $(getApplication)

mpirun -n 2 bin/icoFoam -parallel
#/usr/bin/time -p mpirun $(getApplication) -parallel >& log_v10_10M-216.mpi


