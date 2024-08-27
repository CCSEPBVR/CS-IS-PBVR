#!/bin/bash
# 
# File:   test.bash
# Author: Martin
#
# Created on Nov 5, 2015, 6:02:50 PM
#
DROP_CACHE=0
if [ "$1" = --drop-cache ]; then
    if [ $(id -u) = 0 ]; then
		DROP_CACHE=1
	else
		DROP_CACHE=0
		echo --drop-cache requires root
		exit;
	fi
fi

function beginDataSet {
	echo
	echo DataSet samples/$1
	echo ============================================================
	INPUT_DIR=samples/$1
		if [ $DROP_CACHE = 1 ]; then
				echo Dropping sys vm cache before each run
		fi	
}

function runTest {
	
	echo Running test with params:/param_$1.txt
	mkdir -p output/$1/
	for i in `seq 1 3`;
	do
		if [ $DROP_CACHE = 1 ]; then
				echo 3 > /proc/sys/vm/drop_caches
				echo -n "*" 
		fi
		echo -n run $i:
		/usr/bin/time -f"Wall:%E Percent:%P  Kernel:%S  User:%U" ./filter $INPUT_DIR/param_$1.txt >output/$1.txt
	done
	echo ------------------------------------------------------------
}
	
beginDataSet gt5d
runTest gt5d_1block_1step
runTest gt5d_1block_4step

beginDataSet plot3d/ConvertedFromGt5d
runTest co3d_1block_1step
runTest co3d_1block_4step
runTest co3d_1block_1step
runTest co3d_4block_4step

echo done
