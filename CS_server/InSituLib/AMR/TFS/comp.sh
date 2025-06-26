#mpiicpc -o test main.o Hydrogen.o Argument.o -L../../InSituLib -lInSituPBVR -L../../FunctionParser -lpbvrFunc -L../../KVS/Source/Core/Release -lkvsCore -O3 -axMIC-AVX512 -qopenmp -ipo -qopt-report=5 -L../../KMATH/random/c++/ -L../../KMATH/ntl-5.5.2/src  -lkm_random -lntl -L../../KVS/Source/Core/Release -lkvsCore

icpc -c -qopt-report=5 -axMIC-AVX512 -I../../KVS/Source TrilinearInterpolator.cpp
icpc -c -qopt-report=5 -axMIC-AVX512 -I../../KVS/Source main.cpp
icpc -o TrilinearInterp main.o TrilinearInterpolator.o -L../../KVS/Source/Core/Release -lkvsCore -qopt-report=5 -axMIC-AVX512 
#icpc -o TrilinearInterp main.o -L../../KVS/Source/Core/Release -lkvsCore -qopt-report -axMIC-AVX512 
