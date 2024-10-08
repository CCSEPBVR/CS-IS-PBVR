#!/bin/sh

. /etc/profile.d/modules.sh
module purge
#module load cuda/11.4 gnu/cur intel/cur  mpt/2.23-ga
module load gnu/9.5.0 openmpi/4.1.4 paraview/5.9.0_osmesa vtk/7.1.1
#export MPICC_CC=icc; export MPICXX_CXX=icpc
bash /home/g6/a214026/OpenFOAM/env/OpenFOAM-v10/etc/bashrc
#bash ../../../../OpenFOAM-v10/etc/bashrc

mkdir bin 
mkdir particle_out

CURRENT=$(cd $(dirname $0);pwd)
echo "end setting module & export"
rm bin/icoFoam
#cd ../
cd ../../../../InSituLib/unstruct

#file="citylbm"
#if [ -e $file ]; then
#    echo "citylbm exists. skip git cloning..."
#else
#    echo "citylbm not exists. start git cloning..."
#    git clone -b feature/citylbm_pbvr git@github.com:hasegawa-yuta-jaea/citylbm
#    echo "end git cloning..."
#fi

#cd ispbvr_daemon 
make clean
make -j

echo "finish make pbvr"

cd $CURRENT
#cd ../../Example/C/gcc_mpi_omp/cavity_OpenFOAM
wclean
wmake -j

echo "finish make citylbm"

#if [ $1 == sub ]; then
#    echo "execute submission process"
#    cd run_pbvr
#    ./RESET_STATE.sh
#    qsub js
#else
#    echo "not execute submission process"
#fi
