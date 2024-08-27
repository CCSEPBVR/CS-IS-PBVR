#!/bin/bash
# 
# File:   test.bash
# Author: Martin
#
# Created on Nov 5, 2015, 6:02:50 PM
#

echo DataSet samples/p3d/steamTurbine
echo ============================================================
echo Running test with params:param_turbine_134blocks_1step.txt
mkdir -p output/turbine_134blocks_1step
/usr/bin/time -f"Wall:%E Percent:%P  Kernel:%S  User:%U" ./filter samples/p3d/steamTurbine/param_turbine_134blocks_1step.txt >output/turbine_134blocks_1step.txt
echo ------------------------------------------------------------
echo Running test with params:param_turbine_134blocks_2step.txt
mkdir -p output/turbine_134blocks_2step
/usr/bin/time -f"Wall:%E Percent:%P  Kernel:%S  User:%U" ./filter samples/p3d/steamTurbine/param_turbine_134blocks_2step.txt >output/turbine_134blocks_2step.txt
echo ============================================================
echo done
