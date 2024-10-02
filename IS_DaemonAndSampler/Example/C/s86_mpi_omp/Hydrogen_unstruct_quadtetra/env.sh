#!/bin/sh

. /etc/profile.d/modules.sh
module purge
module load cuda/11.4 gnu/cur intel/cur  mpt/2.23-ga
export MPICC_CC=icc; export MPICXX_CXX=icpc

echo "end setting module & export"

#cd ../
cd ../../../InSituLib/unstruct

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

cd ../../Example/C/Hydrogen_unstruct_quadtetra
make clean
make -j

echo "finish make citylbm"

#if [ $1 == sub ]; then
#    echo "execute submission process"
#    cd run_pbvr
#    ./RESET_STATE.sh
#    qsub js
#else
#    echo "not execute submission process"
#fi
