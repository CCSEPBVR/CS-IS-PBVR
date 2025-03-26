CURRENT=$(cd $(dirname $0);pwd)
rm bin/icoFoam
cd ../../../../InSituLib/unstruct/;make clean;  make -j
#make -j  -f /home/g6/a214026/pbvr/UNPBVR/ispbvr_4openfoam/CS-IS-PBVR/IS_DaemonAndSampler/InSituLib/unstruct/Makefile 
cd $CURRENT
wmake -j
